/****************************************************************************
* Copyright (c) 2023, CEA
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

#include <Op_Div_PolyMAC_P0.h>
#include <Domaine_Cl_PolyMAC.h>
#include <Champ_Face_PolyMAC_P0.h>
#include <Probleme_base.h>
#include <Navier_Stokes_std.h>
#include <Schema_Temps_base.h>

#include <EcrFicPartage.h>
#include <Matrice_Morse.h>
#include <Matrix_tools.h>
#include <Array_tools.h>


Implemente_instanciable(Op_Div_PolyMAC_P0,"Op_Div_PolyMAC_P0",Operateur_Div_base);


//// printOn
//

Sortie& Op_Div_PolyMAC_P0::printOn(Sortie& s) const
{
  return s << que_suis_je() ;
}

//// readOn
//

Entree& Op_Div_PolyMAC_P0::readOn(Entree& s)
{
  return s ;
}



/*! @brief
 *
 */
void Op_Div_PolyMAC_P0::associer(const Domaine_dis& domaine_dis,
                                 const Domaine_Cl_dis& domaine_Cl_dis,
                                 const Champ_Inc&)
{
  const Domaine_PolyMAC_P0& zPolyMAC_P0 = ref_cast(Domaine_PolyMAC_P0, domaine_dis.valeur());
  const Domaine_Cl_PolyMAC& zclPolyMAC_P0 = ref_cast(Domaine_Cl_PolyMAC, domaine_Cl_dis.valeur());
  le_dom_PolyMAC_P0 = zPolyMAC_P0;
  la_zcl_PolyMAC_P0 = zclPolyMAC_P0;
}

void Op_Div_PolyMAC_P0::dimensionner(Matrice_Morse& matrice) const
{
  const Domaine_PolyMAC_P0& domaine = le_dom_PolyMAC_P0.valeur();
  const IntTab& f_e = domaine.face_voisins();
  int i, e, f, n, ne_tot = domaine.nb_elem_tot(), nf_tot = domaine.nb_faces_tot(), N = equation().inconnue().valeurs().line_size(), D = dimension;

  IntTab stencil(0,2);
  stencil.set_smart_resize(1);
  for (f = 0; f < domaine.nb_faces(); f++)
    for (i = 0; i < 2 && (e = f_e(f, i)) >= 0; i++)
      for (n = 0; n < N; n++) stencil.append_line(N * e + n, N * f + n);

  tableau_trier_retirer_doublons(stencil);
  Matrix_tools::allocate_morse_matrix(N * ne_tot, N * (nf_tot + D * ne_tot), stencil, matrice);
}

DoubleTab& Op_Div_PolyMAC_P0::ajouter(const DoubleTab& vit, DoubleTab& div) const
{
  const Domaine_PolyMAC_P0& domaine = le_dom_PolyMAC_P0.valeur();
  const DoubleVect& fs = domaine.face_surfaces(), &pf = equation().milieu().porosite_face();
  const IntTab& f_e = domaine.face_voisins();
  int i, e, f, n, N = vit.line_size();
  assert(div.line_size() == N);

  DoubleTab& tab_flux_bords = flux_bords_;
  tab_flux_bords.resize(domaine.nb_faces_bord(),1);
  tab_flux_bords=0;

  for (f = 0; f < domaine.nb_faces(); f++)
    {
      for (i = 0; i < 2 && (e = f_e(f, i)) >= 0; i++)
        for (n = 0; n < N; n++)
          div(e, n) += (i ? -1 : 1) * fs(f) * pf(f) * vit(f, n);
      if (f < domaine.premiere_face_int())
        for (n = 0; n < N; n++) tab_flux_bords(f, n) = fs(f) * pf(f) * vit(f, n);
    }

  div.echange_espace_virtuel();

  return div;
}
void Op_Div_PolyMAC_P0::contribuer_a_avec(const DoubleTab&,Matrice_Morse& mat) const
{
  const Domaine_PolyMAC_P0& domaine = le_dom_PolyMAC_P0.valeur();
  const Champ_Face_PolyMAC_P0& ch = ref_cast(Champ_Face_PolyMAC_P0, equation().inconnue().valeur());
  const DoubleVect& fs = domaine.face_surfaces(), &pf = equation().milieu().porosite_face();
  const IntTab& f_e = domaine.face_voisins();
  int i, e, f, n, N = ch.valeurs().line_size();

  for (f = 0; f < domaine.nb_faces(); f++)
    for (i = 0; i < 2 && (e = f_e(f, i)) >= 0; i++)
      for (n = 0; n < N; n++) mat(N * e + n, N * f + n) += (i ? 1 : -1) * fs(f) * pf(f);
}

DoubleTab& Op_Div_PolyMAC_P0::calculer(const DoubleTab& vit, DoubleTab& div) const
{
  div = 0;
  return ajouter(vit,div);

}

int Op_Div_PolyMAC_P0::impr(Sortie& os) const
{

  const int impr_bord=(le_dom_PolyMAC_P0->domaine().bords_a_imprimer().est_vide() ? 0:1);
  //SFichier Flux_div;
  if (!Flux_div.is_open()) ouvrir_fichier(Flux_div,"",je_suis_maitre());
  EcrFicPartage Flux_face;
  ouvrir_fichier_partage(Flux_face,"",impr_bord);
  const Schema_Temps_base& sch = equation().probleme().schema_temps();
  double temps = sch.temps_courant();
  if (je_suis_maitre()) Flux_div.add_col(temps);

  int nb_compo=flux_bords_.dimension(1);
  // On parcours les frontieres pour sommer les flux par frontiere dans le tableau flux_bord
  DoubleVect flux_bord(nb_compo);
  DoubleVect bilan(nb_compo);
  bilan = 0;
  for (int num_cl=0; num_cl<le_dom_PolyMAC_P0->nb_front_Cl(); num_cl++)
    {
      flux_bord=0;
      const Cond_lim& la_cl = la_zcl_PolyMAC_P0->les_conditions_limites(num_cl);
      const Front_VF& frontiere_dis = ref_cast(Front_VF,la_cl.frontiere_dis());
      int ndeb = frontiere_dis.num_premiere_face();
      int nfin = ndeb + frontiere_dis.nb_faces();
      for (int face=ndeb; face<nfin; face++)
        for(int k=0; k<nb_compo; k++)
          flux_bord(k)+=flux_bords_(face, k);
      for(int k=0; k<nb_compo; k++)
        flux_bord(k)=Process::mp_sum(flux_bord(k));

      if(je_suis_maitre())
        {
          for(int k=0; k<nb_compo; k++)
            {
              //Ajout pour impression sur fichiers separes
              Flux_div.add_col(flux_bord(k));
              bilan(k)+=flux_bord(k);
            }
        }
    }

  if(je_suis_maitre())
    {
      for(int k=0; k<nb_compo; k++)
        {
          Flux_div.add_col(bilan(k));
        }
      Flux_div << finl;
    }

  for (int num_cl=0; num_cl<le_dom_PolyMAC_P0->nb_front_Cl(); num_cl++)
    {
      const Frontiere_dis_base& la_fr = la_zcl_PolyMAC_P0->les_conditions_limites(num_cl).frontiere_dis();
      const Cond_lim& la_cl = la_zcl_PolyMAC_P0->les_conditions_limites(num_cl);
      const Front_VF& frontiere_dis = ref_cast(Front_VF,la_cl.frontiere_dis());
      int ndeb = frontiere_dis.num_premiere_face();
      int nfin = ndeb + frontiere_dis.nb_faces();
      if (le_dom_PolyMAC_P0->domaine().bords_a_imprimer().contient(la_fr.le_nom()))
        {
          Flux_face << "# Flux par face sur " << la_fr.le_nom() << " au temps " << temps << " : " << finl;
          for (int face=ndeb; face<nfin; face++)
            {
              if (dimension==2)
                Flux_face << "# Face a x= " << le_dom_PolyMAC_P0->xv(face,0) << " y= " << le_dom_PolyMAC_P0->xv(face,1) << " flux=" ;
              else if (dimension==3)
                Flux_face << "# Face a x= " << le_dom_PolyMAC_P0->xv(face,0) << " y= " << le_dom_PolyMAC_P0->xv(face,1) << " z= " << le_dom_PolyMAC_P0->xv(face,2) << " flux=" ;
              for(int k=0; k<nb_compo; k++)
                Flux_face << " " << flux_bords_(face, k);
              Flux_face << finl;
            }
          Flux_face.syncfile();
        }
    }

  return 1;
}


void Op_Div_PolyMAC_P0::volumique(DoubleTab& div) const
{
  const Domaine_PolyMAC_P0& domaine_PolyMAC_P0 = le_dom_PolyMAC_P0.valeur();
  const DoubleVect& vol = domaine_PolyMAC_P0.volumes();
  int nb_elem=domaine_PolyMAC_P0.domaine().nb_elem_tot();
  int num_elem;

  for(num_elem=0; num_elem<nb_elem; num_elem++)
    div(num_elem)/=vol(num_elem);
}
