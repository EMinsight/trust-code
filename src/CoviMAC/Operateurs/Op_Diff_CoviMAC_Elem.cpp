/****************************************************************************
* Copyright (c) 2019, CEA
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
// File:        Op_Diff_CoviMAC_Elem.cpp
// Directory:   $TRUST_ROOT/src/CoviMAC/Operateurs
// Version:     1
//
//////////////////////////////////////////////////////////////////////////////
#include <Op_Diff_CoviMAC_Elem.h>
#include <Probleme_base.h>
#include <Schema_Temps_base.h>
#include <Zone_CoviMAC.h>
#include <Zone_Cl_CoviMAC.h>
#include <IntLists.h>
#include <DoubleLists.h>
#include <Dirichlet.h>
#include <Dirichlet_homogene.h>
#include <Neumann_paroi.h>
#include <Echange_contact_CoviMAC.h>
#include <Echange_externe_impose.h>
#include <Array_tools.h>
#include <Matrix_tools.h>
#include <Champ_P0_CoviMAC.h>
#include <Champ_front_calc.h>
#include <Modele_turbulence_scal_base.h>
#include <Synonyme_info.h>
#include <communications.h>
#include <MD_Vector_base.h>
#include <cmath>

Implemente_instanciable( Op_Diff_CoviMAC_Elem          , "Op_Diff_CoviMAC_Elem|Op_Diff_CoviMAC_var_Elem"                                , Op_Diff_CoviMAC_base ) ;
Implemente_instanciable( Op_Dift_CoviMAC_Elem          , "Op_Dift_CoviMAC_P0_CoviMAC|Op_Dift_CoviMAC_var_P0_CoviMAC"                    , Op_Diff_CoviMAC_Elem ) ;
Implemente_instanciable( Op_Diff_Nonlinear_CoviMAC_Elem, "Op_Diff_nonlinear_CoviMAC_Elem|Op_Diff_nonlinear_CoviMAC_var_Elem"            , Op_Diff_CoviMAC_Elem ) ;
Implemente_instanciable( Op_Dift_Nonlinear_CoviMAC_Elem, "Op_Dift_CoviMAC_P0_CoviMAC_nonlinear|Op_Dift_CoviMAC_var_P0_CoviMAC_nonlinear", Op_Diff_CoviMAC_Elem ) ;

Sortie& Op_Diff_CoviMAC_Elem::printOn( Sortie& os ) const
{
  return Op_Diff_CoviMAC_base::printOn( os );
}

Sortie& Op_Dift_CoviMAC_Elem::printOn( Sortie& os ) const
{
  return Op_Diff_CoviMAC_base::printOn( os );
}

Sortie& Op_Diff_Nonlinear_CoviMAC_Elem::printOn( Sortie& os ) const
{
  return Op_Diff_CoviMAC_base::printOn( os );
}

Sortie& Op_Dift_Nonlinear_CoviMAC_Elem::printOn( Sortie& os ) const
{
  return Op_Diff_CoviMAC_base::printOn( os );
}

Entree& Op_Diff_CoviMAC_Elem::readOn( Entree& is )
{
  return Op_Diff_CoviMAC_base::readOn( is );
}

Entree& Op_Diff_Nonlinear_CoviMAC_Elem::readOn( Entree& is )
{
  return Op_Diff_CoviMAC_base::readOn( is );
}

Entree& Op_Dift_CoviMAC_Elem::readOn( Entree& is )
{
  return Op_Diff_CoviMAC_base::readOn( is );
}

Entree& Op_Dift_Nonlinear_CoviMAC_Elem::readOn( Entree& is )
{
  return Op_Diff_CoviMAC_base::readOn( is );
}

void Op_Diff_CoviMAC_Elem::completer()
{
  Op_Diff_CoviMAC_base::completer();
  const Champ_P0_CoviMAC& ch = ref_cast(Champ_P0_CoviMAC, equation().inconnue().valeur());
  const Zone_CoviMAC& zone = la_zone_poly_.valeur();
  if (zone.zone().nb_joints() && zone.zone().joint(0).epaisseur() < 1)
    Cerr << "Op_Diff_CoviMAC_Elem : largeur de joint insuffisante (minimum 1)!" << finl, Process::exit();
  ch.init_cl();
  int nb_comp = (equation().que_suis_je() == "Transport_K_Eps") ? 2 : 1;
  flux_bords_.resize(zone.premiere_face_int(), nb_comp);

  if (!que_suis_je().debute_par("Op_Dift")) return;
  const RefObjU& modele_turbulence = equation().get_modele(TURBULENCE);
  const Modele_turbulence_scal_base& mod_turb = ref_cast(Modele_turbulence_scal_base,modele_turbulence.valeur());
  const Champ_Fonc& lambda_t = mod_turb.conductivite_turbulente();
  associer_diffusivite_turbulente(lambda_t);
}

void Op_Diff_CoviMAC_Elem::dimensionner(Matrice_Morse& mat) const
{
  const Champ_P0_CoviMAC& ch = ref_cast(Champ_P0_CoviMAC, equation().inconnue().valeur());
  const Zone_CoviMAC& zone = la_zone_poly_.valeur();
  const IntTab& f_e = zone.face_voisins(), &f_s = zone.face_sommets();
  const DoubleTab& nf = zone.face_normales();
  const DoubleVect& fs = zone.face_surfaces();
  update_nu();
  int i, ib, j, k, e, eb, f, s, n, N = ch.valeurs().line_size();

  IntTab stencil(0, 2);
  stencil.set_smart_resize(1);

  Cerr << "Op_Diff_CoviMAC_Elem::dimensionner() : ";

  /*  element e -> contribution au flux a la face f vers l'element eb -> depend de la valeur a la face fb -> hybride ou interpolee */
  DoubleTrav nu_f_s(N);
  for (f = 0; f < zone.nb_faces(); f++) if (!ch.icl(f, 0) || ch.icl(f, 0) > 5) //pas de CL ou CL de Dirichlet
      {
        //dependance directe : obligatoire
        for (i = 0; i < 2 && (e = f_e(f, i)) >= 0; i++) if (i < zone.nb_elem()) for (j = 0; j < 2 && (eb = f_e(f, j)) >= 0; j++)
              for (n = 0; n < N; n++) stencil.append_line(N * e + n, N * eb + n);
        //dependance par rapport aux sommets : si nf.nu_e.S_es = 0, on peut s'en passer
        for (i = 0, ib = zone.phifs_d(f); ib < zone.phifs_d(f + 1); i++, ib++)
          {
            if (nu_constant_) for (j = 0, nu_f_s = 0; j < 2 && (e = f_e(f, j)) >= 0; j++)
                for (n = 0; n < N; n++) nu_f_s(n) += nu_prod(e, n, N, &nf(f, 0), &zone.phifs_v(ib, j, 0));
            else nu_f_s = 1;
            //si le flux depend de l'interp au sommet s, alors il depend de tous les elements voisins de s
            for (n = 0, s = f_s(f, i); n < N; n++) if (dabs(nu_f_s(n)) > 1e-6 * fs(f) && bfs_d(s, 2) == bfs_d(s + 1, 2))
                for (j = 0; j < 2 && (e = f_e(f, j)) >= 0; j++) for (k = zone.sef_d(s, 0); k < zone.sef_d(s + 1, 0); k++)
                    stencil.append_line(N * e + n, N * zone.sef_e(k) + n);
          }
      }

  tableau_trier_retirer_doublons(stencil);
  Cerr << "width " << Process::mp_sum(stencil.dimension(0)) * 1. / (N * zone.zone().md_vector_elements().valeur().nb_items_seq_tot()) << finl;
  Matrix_tools::allocate_morse_matrix(N * zone.nb_elem_tot(), N * zone.nb_elem_tot(), stencil, mat);
}

void Op_Diff_CoviMAC_Elem::calculer_flux_bord(const DoubleTab& inco) const
{
  abort();
}

// Description:
// ajoute la contribution de la diffusion au second membre resu
// renvoie resu
DoubleTab& Op_Diff_CoviMAC_Elem::ajouter(const DoubleTab& inco,  DoubleTab& resu) const
{
  update_nu();
  const Champ_P0_CoviMAC& ch = ref_cast(Champ_P0_CoviMAC, equation().inconnue().valeur());
  const Zone_CoviMAC& zone = la_zone_poly_.valeur();
  const Conds_lim& cls = la_zcl_poly_->les_conditions_limites();
  const IntTab& f_e = zone.face_voisins(), &f_s = zone.face_sommets();
  const DoubleVect& fs = zone.face_surfaces();
  const DoubleTab& nf = zone.face_normales();
  int i, ib, j, jb, e, f, s, n, N = inco.line_size(), nu_unif = ts_e.dimension(1) < N;

  /* 1. interpolation des valeurs aux sommets */
  DoubleTrav val_s(zone.zone().domaine().nb_som(), N), den(N);
  for (s = 0; s < val_s.dimension(0); s++)
    {
      //faces de Dirichlet : moyenne simple
      for (i = bfs_d(s, 2), den = bfs_d(s + 1, 2) - bfs_d(s, 2); i < bfs_d(s + 1, 2); i++) for (f = bfs_fd(i), n = 0; n < N; n++)
          val_s(s, n) += ref_cast(Dirichlet, cls[ch.icl(f, 1)].valeur()).val_imp(ch.icl(f, 2), n) / den(n);
      if (den(0)) continue; //plus rien a faire

      //partie "elements"
      for (i = ts_d(s, 0), ib = zone.sef_d(s, 0); i < ts_d(s + 1, 0); i++, ib++) for (n = 0; n < N; n++)
          val_s(s, n) += ts_e(i, nu_unif ? 0 : n) * inco.addr()[N * zone.sef_e(ib) + n];
      //partie "faces de bord"
      for (i = ts_d(s, 1), ib = zone.sef_d(s, 1), den = 1; i < ts_d(s + 1, 1); i++, ib++)
        if (ch.icl(f = zone.sef_f(ib)) < 3) for (n = 0; n < N; n++) //Echange_impose_base
            {
              val_s(s, n) -= ts_f(i, nu_unif ? 0 : n) * ref_cast(Echange_impose_base, cls[ch.icl(f, 1)].valeur()).h_imp(ch.icl(f, 2), n)
                             * ref_cast(Echange_impose_base, cls[ch.icl(f, 1)].valeur()).T_ext(ch.icl(f, 2), n);
              //Echange_externe_impose -> on implicte Ts, Echange_global_impose -> depend de Te
              if (ch.icl(f, 0) == 1) den(n) -= ts_f(i, nu_unif ? 0 : n) * ref_cast(Echange_impose_base, cls[ch.icl(f, 1)].valeur()).h_imp(ch.icl(f, 2), n);
              else val_s(s, n) += ts_f(i, nu_unif ? 0 : n) * ref_cast(Echange_impose_base, cls[ch.icl(f, 1)].valeur()).h_imp(ch.icl(f, 2), n) * inco.addr()[N * f_e(f, 0) + n];
            }
        else for (n = 0; n < N; n++) //Neumann
            val_s(s, n) += ts_f(i, nu_unif ? 0 : n) * ref_cast(Neumann, cls[ch.icl(f, 1)].valeur()).flux_impose(ch.icl(f, 2), n);
      //prise en compte de den
      for (n = 0; n < N; n++) val_s(s, n) /= den(n);
    }

  /* 2. flux aux faces (x surface) */
  DoubleTrav phi(N);
  for (f = 0, phi = 0; f < zone.nb_faces(); f++, phi = 0)
    {
      if (!ch.icl(f, 0) || ch.icl(f, 0) > 5) //interne ou Dirichlet
        {
          for (i = 0, phi = 0; i < 2 && (e = f_e(f, i)) >= 0; i++)
            {
              //contribution de l'element
              for (n = 0; n < N; n++)  phi(n) += nu_prod(e, n, N, &nf(f, 0), &zone.phife(f, i, 0)) * inco.addr()[N * e + n];
              //contribution des sommets
              for (j = 0, jb = zone.phifs_d(f); jb < zone.phifs_d(f + 1); j++, jb++) for (n = 0, s = f_s(f, j); n < N; n++)
                  phi(n) += nu_prod(e, n, N, &nf(f, 0), &zone.phifs_v(jb, i, 0)) * val_s(s, n);
            }
          phi *= nu_fac_(f); //prise en compte de nu_fac
        }
      else if (ch.icl(f, 0) < 3) for (n = 0; n < N; n++) //Echange_impose_base -> flux a deux points
          {
            double invh = 1. / ref_cast(Echange_impose_base, cls[ch.icl(f, 1)].valeur()).h_imp(ch.icl(f, 2), n);
            if (ch.icl(f, 0) == 1) invh += zone.dist_norm_bord(f) * fs(f) * fs(f) / nu_prod(f_e(f, 0), n, N, &nf(f, 0), &nf(f, 0));
            phi(n) = fs(f) * (inco.addr()[N * f_e(f, 0) + n] - ref_cast(Echange_impose_base, cls[ch.icl(f, 1)].valeur()).T_ext(ch.icl(f, 2), n)) / invh;
          }
      else for (n = 0; n < N; n++) phi(n) = ref_cast(Neumann, cls[ch.icl(f, 1)].valeur()).flux_impose(ch.icl(f, 2), n); //Neumann -> facile

      phi *= -1; //phi = - nu grad T
      if (f < zone.premiere_face_int()) for (n = 0; n < N; n++) flux_bords_(f, n) = phi(n);
      //contributions
      for (i = 0; i < 2 && (e = f_e(f, i)) >= 0; i++) for (n = 0; n < N; n++)
          resu.addr()[N * e + n] -= (i ? -1 : 1) * phi(n);
    }
  return resu;
}

//Description:
//on assemble la matrice.
void Op_Diff_CoviMAC_Elem::contribuer_a_avec(const DoubleTab& inco, Matrice_Morse& matrice) const
{
  update_nu();
  const Champ_P0_CoviMAC& ch = ref_cast(Champ_P0_CoviMAC, equation().inconnue().valeur());
  const Zone_CoviMAC& zone = la_zone_poly_.valeur();
  const Conds_lim& cls = la_zcl_poly_->les_conditions_limites();
  const IntTab& f_e = zone.face_voisins(), &f_s = zone.face_sommets();
  const DoubleVect& fs = zone.face_surfaces();
  const DoubleTab& nf = zone.face_normales();
  int i, ib, j, jb, k, e, eb, f, fb, s, n, N = inco.line_size(), nu_unif = ts_e.dimension(1) < N;


  DoubleTrav nu_f_s(N), den(N);
  for (f = 0; f < zone.nb_faces(); f++)
    if (!ch.icl(f, 0) || ch.icl(f, 0) > 5) //interne ou Dirichlet
      {
        //partie "elements"
        for (i = 0; i < 2 && (e = f_e(f, i)) >= 0; i++) if (e < zone.nb_elem()) for (j = 0; j < 2 && (eb = f_e(f, j)) >= 0; j++)
              for (n = 0; n < N; n++) matrice(N * e + n, N * eb + n) += (i ? 1 : -1) * nu_prod(eb, n, N, &nf(f, 0), &zone.phife(f, j, 0));
        //partie "sommets"
        for (i = 0, ib = zone.phifs_d(f); ib < zone.phifs_d(f + 1); i++, ib++) if (bfs_d(f_s(f, i), 2) == bfs_d(f_s(f, i) + 1, 2)) //sauf ceux de Dirichlet
            {
              for (j = 0, s = f_s(f, i), nu_f_s = 0; j < 2 && (e = f_e(f, j)) >= 0; j++) for (n = 0; n < N; n++)
                  nu_f_s(n) += nu_prod(e, n, N, &nf(f, 0), &zone.phifs_v(ib, j, 0));

              //implicitation de Echange_externe_impose
              for (j = ts_d(s, 1), jb = zone.sef_d(s, 1), den = 1; j < ts_d(s + 1, 1); j++, jb++) if (ch.icl(fb = zone.sef_f(jb), 0) == 1)
                  for (n = 0; n < N; n++) den(n) -= ts_f(j, nu_unif ? 0 : n) * ref_cast(Echange_impose_base, cls[ch.icl(fb, 1)].valeur()).h_imp(ch.icl(fb, 2), n);

              //partie "elements"
              for (j = ts_d(s, 0), jb = zone.sef_d(s, 0); j < ts_d(s + 1, 0); j++, jb++) for (n = 0, eb = zone.sef_e(jb); n < N; n++) if (dabs(nu_f_s(n)) > 1e-6 * fs(f))
                    for (k = 0; k < 2 && (e = f_e(f, k)) >= 0; k++) if (e < zone.nb_elem())
                        matrice(N * e + n, N * eb + n) += (k ? 1 : -1) * nu_f_s(n) * nu_fac_(f) * ts_e(j, nu_unif ? 0 : n) / den(n);

              //partie "faces de bord" : seulement Echange_global_impose
              for (j = ts_d(s, 1), jb = zone.sef_d(s, 1); j < ts_d(s + 1, 1); j++, jb++) if (ch.icl(fb = zone.sef_f(jb), 0) == 2)
                  for (n = 0; n < N; n++) if (dabs(nu_f_s(n)) > 1e-6 * fs(f)) for (k = 0; k < 2 && (e = f_e(f, k)) >= 0; k++) if (e < zone.nb_elem())
                          matrice(N * e + n, N * f_e(fb, 0) + n) += (k ? 1 : -1) * nu_f_s(n) * nu_fac_(f) * ts_f(j, nu_unif ? 0 : n)
                                                                    * ref_cast(Echange_impose_base, cls[ch.icl(fb, 1)].valeur()).h_imp(ch.icl(fb, 2), n) / den(n);
            }
      }
    else if (ch.icl(f, 0) < 3) for (n = 0; n < N; n++) //Echange_impose_base -> flux a deux points
        {
          double invh = 1. / ref_cast(Echange_impose_base, cls[ch.icl(f, 1)].valeur()).h_imp(ch.icl(f, 2), n);
          if (ch.icl(f, 0) == 1) invh += zone.dist_norm_bord(f) * fs(f) * fs(f) / nu_prod(f_e(f, 0), n, N, &nf(f, 0), &nf(f, 0));
          matrice(N * e + n, N * e + n) += fs(f) / invh;
        }
  i++;
}

void Op_Diff_CoviMAC_Elem::modifier_pour_Cl(Matrice_Morse& la_matrice, DoubleTab& secmem) const
{
  //en principe, rien a faire!
  return;
}

//Description:
//on ajoute la contribution du second membre.
void Op_Diff_CoviMAC_Elem::contribuer_au_second_membre(DoubleTab& resu) const
{
  abort();
}
