/****************************************************************************
* Copyright (c) 2022, CEA
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
* 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*****************************************************************************/

#include <Echange_contact_VDF.h>
#include <Op_Diff_VDF_base.h>
#include <Champ_front_calc.h>
#include <Eval_Diff_VDF.h>
#include <Pb_Multiphase.h>
#include <TRUSTTrav.h>
#include <Champ_Don.h>
#include <Operateur.h>
#include <Motcle.h>

Implemente_base(Op_Diff_VDF_base, "Op_Diff_VDF_base", Operateur_Diff_base);

Sortie& Op_Diff_VDF_base::printOn(Sortie& s) const { return s << que_suis_je(); }
Entree& Op_Diff_VDF_base::readOn(Entree& s) { return s; }

void Op_Diff_VDF_base::completer()
{
  Operateur_base::completer();
  // Certains operateurs (Axi) n'ont pas d'iterateurs en VDF... Encore une anomalie dans la conception a corriger un jour !
  if (iter.non_nul())
    {
      iter->completer_();
      iter->associer_champ_convecte_ou_inc(equation().inconnue(), nullptr);
      iter->set_convective_op_pb_type(false /* diff op */, sub_type(Pb_Multiphase, equation().probleme()));
    }
}

int Op_Diff_VDF_base::impr(Sortie& os) const
{
  // Certains operateurs (Axi) n'ont pas d'iterateurs en VDF... Encore une anomalie dans la conception a corriger un jour !
  return (iter.non_nul()) ? iter->impr(os) : 0;
}

void Op_Diff_VDF_base::calculer_flux_bord(const DoubleTab& inco, const DoubleTab& val_b) const
{
  iter->calculer_flux_bord(inco, val_b);
}

/*! @brief on ajoute la contribution du second membre.
 *
 */
void Op_Diff_VDF_base::contribuer_au_second_membre(DoubleTab& resu) const
{
  iter->contribuer_au_second_membre(resu);
  if (equation().domaine_application() == Motcle("Hydraulique"))
    // On est dans le cas des equations de Navier_Stokes
    {
      // Ajout du terme du au terme supplementaire en V/(R*R)
      // dans le cas des coordonnees axisymetriques
      if (Objet_U::bidim_axi == 1)
        {
          const Zone_VDF& zvdf = iter->zone();
          const Zone_Cl_VDF& zclvdf = iter->zone_Cl();
          const DoubleTab& xv = zvdf.xv();
          const IntVect& ori = zvdf.orientation();
          const IntTab& face_voisins = zvdf.face_voisins();
          const DoubleVect& volumes_entrelaces = zvdf.volumes_entrelaces();
          //          int nb_faces=zvdf.nb_faces();
          DoubleTrav diffu_tot(zvdf.nb_elem_tot());
          double db_diffusivite;

          if (equation().que_suis_je() == "Navier_Stokes_standard")
            {
              const Eval_Diff_VDF& eval = dynamic_cast<const Eval_Diff_VDF&>(iter->evaluateur());
              const Champ_base& ch_diff = eval.get_diffusivite();
              const DoubleTab& tab_diffusivite = ch_diff.valeurs();
              if (tab_diffusivite.size() == 1)
                diffu_tot = tab_diffusivite(0, 0);
              else
                diffu_tot = tab_diffusivite;
            }
          else
            {
              Cerr << "Probleme dans Op_Diff_VDF_base::contribuer_au_second_membre  avec le type de l'equation" << finl;
              Cerr << "on n'a pas prevu d'autre cas que Navier_Stokes_std" << finl;
              exit();
            }

          // Boucle sur les conditions limites pour traiter les faces de bord

          int ndeb, nfin, num_face;

          for (int n_bord = 0; n_bord < zvdf.nb_front_Cl(); n_bord++)
            {
              const Cond_lim& la_cl = zclvdf.les_conditions_limites(n_bord);

              if (sub_type(Dirichlet, la_cl.valeur()))
                {
                  const Dirichlet& la_cl_diri = ref_cast(Dirichlet, la_cl.valeur());
                  const Front_VF& le_bord = ref_cast(Front_VF, la_cl.frontiere_dis());
                  ndeb = le_bord.num_premiere_face();
                  nfin = ndeb + le_bord.nb_faces();
                  double r_2, coef;
                  for (num_face = ndeb; num_face < nfin; num_face++)
                    {
                      if (ori(num_face) == 0)
                        {
                          int elem1 = face_voisins(num_face, 0);
                          int elem2 = face_voisins(num_face, 1);
                          if (elem1 == -1)
                            db_diffusivite = diffu_tot(elem2);
                          else
                            {
                              assert(elem2 == -1);
                              db_diffusivite = diffu_tot(elem1);
                            }
                          r_2 = xv(num_face, 0) * xv(num_face, 0);
                          coef = volumes_entrelaces(num_face) / r_2;
                          resu(num_face) -= (la_cl_diri.val_imp(num_face - ndeb) * coef) * db_diffusivite;
                        }
                    }
                }
            }
        }
    }
}

/*! @brief calcule la contribution de la diffusion, la range dans resu
 *
 */
DoubleTab& Op_Diff_VDF_base::calculer(const DoubleTab& inco, DoubleTab& resu) const
{
  resu = 0.;
  return ajouter(inco, resu);
}

void Op_Diff_VDF_base::init_op_ext() const
{
  const Zone_VDF& zvdf = iter->zone();
  const Zone_Cl_VDF& zclvdf = iter->zone_Cl();
  op_ext = { this };      //le premier op_ext est l'operateur local

  for (int n_bord = 0; n_bord < zvdf.nb_front_Cl(); n_bord++)
    {
      const Cond_lim& la_cl = zclvdf.les_conditions_limites(n_bord);
      if (sub_type(Echange_contact_VDF, la_cl.valeur()))
        {
          const Echange_contact_VDF& cl = ref_cast(Echange_contact_VDF, la_cl.valeur());
          const Champ_front_calc& ch = ref_cast(Champ_front_calc, cl.T_autre_pb().valeur());
          const Equation_base& o_eqn = ch.equation();
          const Op_Diff_VDF_base *o_diff = &ref_cast(Op_Diff_VDF_base, o_eqn.operateur(0).l_op_base());

          if (std::find(op_ext.begin(), op_ext.end(), o_diff) == op_ext.end())
            op_ext.push_back(o_diff);
        }
    }
  op_ext_init_ = 1;
}
