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
//////////////////////////////////////////////////////////////////////////////
//
// File:        Op_Conv_EF_Stab_PolyMAC_Face.cpp
// Directory:   $TRUST_ROOT/src/PolyMAC/Operateurs
// Version:     1
//
//////////////////////////////////////////////////////////////////////////////

#include <Op_Conv_EF_Stab_PolyMAC_Face.h>
#include <Dirichlet_homogene.h>
#include <Champ_Face_PolyMAC.h>
#include <Schema_Temps_base.h>
#include <Zone_Cl_PolyMAC.h>
#include <Pb_Multiphase.h>
#include <Zone_Poly_base.h>
#include <Matrix_tools.h>
#include <Array_tools.h>
#include <TRUSTLists.h>
#include <Dirichlet.h>

#include <Param.h>
#include <cmath>
#include <Masse_ajoutee_base.h>

Implemente_instanciable( Op_Conv_EF_Stab_PolyMAC_Face, "Op_Conv_EF_Stab_PolyMAC_Face", Op_Conv_PolyMAC_base ) ;
Implemente_instanciable( Op_Conv_Amont_PolyMAC_Face, "Op_Conv_Amont_PolyMAC_Face", Op_Conv_EF_Stab_PolyMAC_Face ) ;
Implemente_instanciable( Op_Conv_Centre_PolyMAC_Face, "Op_Conv_Centre_PolyMAC_Face", Op_Conv_EF_Stab_PolyMAC_Face ) ;

// XD Op_Conv_EF_Stab_PolyMAC_Face interprete Op_Conv_EF_Stab_PolyMAC_Face 1 Class Op_Conv_EF_Stab_PolyMAC_Face
Sortie& Op_Conv_EF_Stab_PolyMAC_Face::printOn( Sortie& os ) const
{
  Op_Conv_PolyMAC_base::printOn( os );
  return os;
}

Sortie& Op_Conv_Amont_PolyMAC_Face::printOn( Sortie& os ) const
{
  Op_Conv_PolyMAC_base::printOn( os );
  return os;
}

Sortie& Op_Conv_Centre_PolyMAC_Face::printOn( Sortie& os ) const
{
  Op_Conv_PolyMAC_base::printOn( os );
  return os;
}

Entree& Op_Conv_EF_Stab_PolyMAC_Face::readOn( Entree& is )
{
  Op_Conv_PolyMAC_base::readOn( is );
  Param param(que_suis_je());
  param.ajouter("alpha", &alpha);            // XD_ADD_P double parametre ajustant la stabilisation de 0 (schema centre) a 1 (schema amont)
  param.lire_avec_accolades_depuis(is);
  return is;
}

Entree& Op_Conv_Amont_PolyMAC_Face::readOn( Entree& is )
{
  Op_Conv_PolyMAC_base::readOn( is );
  alpha = 1;
  return is;
}

Entree& Op_Conv_Centre_PolyMAC_Face::readOn( Entree& is )
{
  Op_Conv_PolyMAC_base::readOn( is );
  alpha = 0;
  return is;
}

void Op_Conv_EF_Stab_PolyMAC_Face::completer()
{
  Op_Conv_PolyMAC_base::completer();
  /* au cas ou... */
  const Zone_Poly_base& zone = la_zone_poly_.valeur();
  zone.init_equiv();

  if (zone.zone().nb_joints() && zone.zone().joint(0).epaisseur() < 2)
    Cerr << "Op_Conv_EF_Stab_PolyMAC_Face : largeur de joint insuffisante (minimum 2)!" << finl, Process::exit();
  porosite_f.ref(zone.porosite_face());
  porosite_e.ref(zone.porosite_elem());
}

double Op_Conv_EF_Stab_PolyMAC_Face::calculer_dt_stab() const
{
  double dt = 1e10;
  const Zone_Poly_base& zone = la_zone_poly_.valeur();
  const Champ_Face_PolyMAC& ch = ref_cast(Champ_Face_PolyMAC, equation().inconnue().valeur());
  const DoubleVect& fs = zone.face_surfaces(), &pf = zone.porosite_face(), &ve = zone.volumes(), &pe = zone.porosite_elem(), &vf = zone.volumes_entrelaces();
  const DoubleTab& vit = vitesse_->valeurs(), &vfd = zone.volumes_entrelaces_dir(),
                   *alp = sub_type(Pb_Multiphase, equation().probleme()) ? &ref_cast(Pb_Multiphase, equation().probleme()).eq_masse.inconnue().passe() : NULL;
  const IntTab& e_f = zone.elem_faces(), &f_e = zone.face_voisins(), &fcl = ch.fcl();
  int i, e, f, n, N = vit.line_size();
  DoubleTrav flux(N), vol(N); //somme des flux pf * |f| * vf, volume minimal des mailles d'elements/faces affectes par ce flux

  for (e = 0; e < zone.nb_elem(); e++)
    {
      for (vol = pe(e) * ve(e), flux = 0, i = 0; i < e_f.dimension(1) && (f = e_f(e, i)) >= 0; i++) for (n = 0; n < N; n++)
          {
            flux(n) += pf(f) * fs(f) * std::max((e == f_e(f, 1) ? 1 : -1) * vit(f, n), 0.); //seul le flux entrant dans e compte
            if (0 && fcl(f, 0) < 2) vol(n) = std::min(vol(n), pf(f) * vf(f) * vf(f) / vfd(f, e != f_e(f, 0))); //prise en compte de la contribution aux faces
          }
      for (n = 0; n < N; n++) if ((!alp || (*alp)(e, n) > 1e-3) && flux(n)) dt = std::min(dt, vol(n) / flux(n));
    }

  return Process::mp_min(dt);
}

void Op_Conv_EF_Stab_PolyMAC_Face::dimensionner_blocs(matrices_t matrices, const tabs_t& semi_impl) const
{
  const Zone_Poly_base& zone = la_zone_poly_.valeur();
  const Champ_Face_PolyMAC& ch = ref_cast(Champ_Face_PolyMAC, equation().inconnue().valeur());
  const IntTab& f_e = zone.face_voisins(), &e_f = zone.elem_faces(), &fcl = ch.fcl();
  const DoubleTab& nf = zone.face_normales(), &inco = ch.valeurs(), &xp = zone.xp(), &xv = zone.xv();
  const DoubleVect& fs = zone.face_surfaces(), &ve = zone.volumes();

  const std::string& nom_inco = ch.le_nom().getString();
  if (!matrices.count(nom_inco) || semi_impl.count(nom_inco)) return; //pas de bloc diagonal ou semi-implicite -> rien a faire
  const Pb_Multiphase *pbm = sub_type(Pb_Multiphase, equation().probleme()) ? &ref_cast(Pb_Multiphase, equation().probleme()) : NULL;
  const Masse_ajoutee_base *corr = pbm && pbm->has_correlation("masse_ajoutee") ? &ref_cast(Masse_ajoutee_base, pbm->get_correlation("masse_ajoutee").valeur()) : NULL;
  Matrice_Morse& mat = *matrices.at(nom_inco), mat2;

  int i, j, k, l, e, eb, f, fb, fc, m, n, N = equation().inconnue().valeurs().line_size();

  IntTab stencil(0, 2);
  stencil.set_smart_resize(1);

  /* agit uniquement aux elements; diagonale omise */
  for (f = 0; f < zone.nb_faces_tot(); f++) if (f_e(f, 0) >= 0 && (f_e(f, 1) >= 0 || fcl(f, 0) == 3))
      for (i = 0; i < 2 && (e = f_e(f, i)) >= 0; i++) for (j = 0; j < 2 && (eb = f_e(f, j)) >= 0; j++)
          {
            for (k = 0; k < e_f.dimension(1) && (fb = e_f(e, k)) >= 0; k++) if (fb < zone.nb_faces())
                {
                  if ((fc = zone.equiv(f, i, k)) >= 0) //equivalence : face -> face
                    for (n = 0; n < N; n++) for (m = (corr ? 0 : n); m < (corr ? N : n + 1); m++) stencil.append_line(N * fb + n, N * fc + m);
                  else if (f_e(f, 1) >= 0) for (l = 0; l < e_f.dimension(1) && (fc = e_f(eb, l)) >= 0; l++) //pas d'equivalence : faces de l'elem -> face
                      if(std::abs(fs(fc) * zone.dot(&xv(fc, 0), &nf(fb, 0), &xp(eb, 0))) > 1e-6 * ve(eb) * fs(fb)) for (n = 0; n < N; n++) for (m = (corr ? 0 : n); m < (corr ? N : n + 1); m++)
                            stencil.append_line(N * fb + n, N * fc + m);
                }
          }

  tableau_trier_retirer_doublons(stencil);
  Matrix_tools::allocate_morse_matrix(inco.size_totale(), inco.size_totale(), stencil, mat2);
  mat.nb_colonnes() ? mat += mat2 : mat = mat2;
}

// ajoute la contribution de la convection au second membre resu
// renvoie resu
void Op_Conv_EF_Stab_PolyMAC_Face::ajouter_blocs(matrices_t matrices, DoubleTab& secmem, const tabs_t& semi_impl) const
{
  const Zone_Poly_base& zone = la_zone_poly_.valeur();
  const Champ_Face_PolyMAC& ch = ref_cast(Champ_Face_PolyMAC, equation().inconnue().valeur());
  const Conds_lim& cls = la_zcl_poly_.valeur().les_conditions_limites();
  const IntTab& f_e = zone.face_voisins(), &e_f = zone.elem_faces(), &fcl = ch.fcl();
  const DoubleTab& vit = ch.passe(), &nf = zone.face_normales(), &vfd = zone.volumes_entrelaces_dir(), &xp = zone.xp(), &xv = zone.xv();
  const DoubleVect& fs = zone.face_surfaces(), &pe = porosite_e, &pf = porosite_f, &ve = zone.volumes();

  /* a_r : produit alpha_rho si Pb_Multiphase -> par semi_implicite, ou en recuperant le champ_conserve de l'equation de masse */
  const std::string& nom_inco = ch.le_nom().getString();
  const Pb_Multiphase *pbm = sub_type(Pb_Multiphase, equation().probleme()) ? &ref_cast(Pb_Multiphase, equation().probleme()) : NULL;
  const Masse_ajoutee_base *corr = pbm && pbm->has_correlation("masse_ajoutee") ? &ref_cast(Masse_ajoutee_base, pbm->get_correlation("masse_ajoutee").valeur()) : NULL;
  const DoubleTab& inco = semi_impl.count(nom_inco) ? semi_impl.at(nom_inco) : ch.valeurs(),
                   *a_r = !pbm ? NULL : semi_impl.count("alpha_rho") ? &semi_impl.at("alpha_rho") : &pbm->eq_masse.champ_conserve().valeurs(),
                    *alp = pbm ? &pbm->eq_masse.inconnue().passe() : NULL, &rho = equation().milieu().masse_volumique().passe();
  Matrice_Morse *mat = matrices.count(nom_inco) && !semi_impl.count(nom_inco) ? matrices.at(nom_inco) : NULL;

  int i, j, k, l, e, eb, f, fb, fc, fd, m, n, N = inco.line_size(), d, D = dimension, comp = !incompressible_;
  double mult;

  DoubleTrav dfac(2, N, N), masse(N, N);
  for (f = 0; f < zone.nb_faces_tot(); f++) if (f_e(f, 0) >= 0 && (f_e(f, 1) >= 0 || fcl(f, 0) == 1 || fcl(f, 0) == 3))
      {
        for (i = 0, dfac = 0; i < 2; i++)
          {
            //masse : diagonale + masse ajoutee si correlation
            for (masse = 0, e = f_e(f, f_e(f, i) >= 0 ? i : 0), n = 0; n < N; n++) masse(n, n) = a_r ? (*a_r)(e, n) : 1;
            if (corr) corr->ajouter(&(*alp)(e, 0), &rho(e, 0), masse);
            //contribution a dfac
            for (e = f_e(f, i), eb = f_e(f, i), n = 0; n < N; n++) for (m = 0; m < N; m++)
                dfac(fcl(f, 0) == 1 ? 0 : i, n, m) += fs(f) * vit(f, m) * pe(eb >= 0 ? eb : f_e(f, 0)) * masse(n, m)
                                                      * (1. + (vit(f, m) * (i ? -1 : 1) >= 0 ? 1. : vit(f, m) ? -1. : 0.) * alpha) / 2;
          }
        for (i = 0; i < 2 && (e = f_e(f, i)) >= 0; i++)
          {
            for (k = 0; k < e_f.dimension(1) && (fb = e_f(e, k)) >= 0; k++) if (fb < zone.nb_faces())
                {
                  if ((fc = zone.equiv(f, i, k)) >= 0 || f_e(f, 1) < 0) for (j = 0; j < 2; j++) //equivalence : face fd -> face fb
                      {
                        eb = f_e(f, j), fd = (j == i ? fb : fc); //element/face sources
                        mult = (fd < 0 || zone.dot(&nf(fb, 0), &nf(fd, 0)) > 0 ? 1 : -1) * (fd >= 0 ? pf(fd) / pe(eb) : 1); //multiplicateur pour passer de vf a ve
                        for (n = 0; n < N; n++) for (m = 0; m < N; m++) if (dfac(j, n, m))
                              {
                                double fac = (i ? -1 : 1) * vfd(fb, e != f_e(fb, 0)) * dfac(j, n, m) / ve(e);
                                if (fd >= 0) secmem(fb, n) -= fac * mult * inco(fd, m); //autre face calculee
                                else for (d = 0; d < D; d++)  //CL de Dirichlet
                                    secmem(fb, n) -= fac * nf(fb, d) / fs(fb) * ref_cast(Dirichlet, cls[fcl(f, 1)].valeur()).val_imp(fcl(f, 2), N * d + m);
                                if (comp) secmem(fb, n) += fac * inco(fb, m); //partie v div(alpha rho v)
                                if (!mat) continue;
                                if (fd >= 0) (*mat)(N * fb + n, N * fd + m) += fac * mult;
                                if (comp) (*mat)(N * fb + n, N * fb + m) -= fac;
                              }
                      }
                  else for (j = 0; j < 2; j++)  //pas d'equivalence : n_f * operateur aux elements
                      {
                        for (eb = f_e(f, j), l = 0; l < e_f.dimension(1) && (fc = e_f(eb, l)) >= 0; l++)
                          if (std::abs(fs(fc) * zone.dot(&xv(fc, 0), &nf(fb, 0), &xp(eb, 0))) > 1e-6 * ve(eb) * fs(fb)) for (n = 0; n < N; n++) for (m = 0; m < N; m++) if (dfac(j, n, m))
                                  {
                                    double fac = (i ? -1 : 1) * vfd(fb, e != f_e(fb, 0)) * dfac(j, n, m) * (eb == f_e(fc, 0) ? 1 : -1) * fs(fc) * zone.dot(&xv(fc, 0), &nf(fb, 0), &xp(eb, 0)) / (ve(e) * ve(eb) * fs(fb));
                                    secmem(fb, n) -= fac * inco(fc, m);
                                    if (mat && fac) (*mat)(N * fb + n, N * fc + m) += fac;
                                  }
                        if (comp) for (l = 0; l < e_f.dimension(1) && (fc = e_f(e, l)) >= 0; l++)
                            if (std::abs(fs(fc) * zone.dot(&xv(fc, 0), &nf(fb, 0), &xp(e, 0))) > 1e-6 * ve(e) * fs(fb)) for (n = 0; n < N; n++) for (m = 0; m < N; m++) if (dfac(j, n, m))
                                    {
                                      double fac = (i ? -1 : 1) * vfd(fb, e != f_e(fb, 0)) * dfac(j, n, m) * (e == f_e(fc, 0) ? 1 : -1) * fs(fc) * zone.dot(&xv(fc, 0), &nf(fb, 0), &xp(e, 0)) / (ve(e) * ve(e) * fs(fb));
                                      secmem(fb, n) += fac * inco(fc, m);
                                      if (mat && fac) (*mat)(N * fb + n, N * fc + m) -= fac;
                                    }
                      }
                }
          }
      }
}
