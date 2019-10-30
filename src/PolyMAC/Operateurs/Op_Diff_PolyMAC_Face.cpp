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
// File:        Op_Diff_PolyMAC_Face.cpp
// Directory:   $TRUST_ROOT/src/PolyMAC/Operateurs
// Version:     1
//
//////////////////////////////////////////////////////////////////////////////

#include <Op_Diff_PolyMAC_Face.h>
#include <Probleme_base.h>
#include <Schema_Temps_base.h>
#include <Zone_PolyMAC.h>
#include <Zone_Cl_PolyMAC.h>
#include <IntLists.h>
#include <DoubleLists.h>
#include <Dirichlet.h>
#include <Dirichlet_homogene.h>
#include <Symetrie.h>
#include <Champ_Face_PolyMAC.h>
#include <Array_tools.h>
#include <Matrix_tools.h>
#include <Mod_turb_hyd_base.h>

Implemente_instanciable( Op_Diff_PolyMAC_Face, "Op_Diff_PolyMAC_Face|Op_Dift_PolyMAC_Face_PolyMAC", Op_Diff_PolyMAC_base ) ;

Sortie& Op_Diff_PolyMAC_Face::printOn( Sortie& os ) const
{
  Op_Diff_PolyMAC_base::printOn( os );
  return os;
}

Entree& Op_Diff_PolyMAC_Face::readOn( Entree& is )
{
  Op_Diff_PolyMAC_base::readOn( is );
  return is;
}

void Op_Diff_PolyMAC_Face::completer()
{
  Op_Diff_PolyMAC_base::completer();
  const Zone_PolyMAC& zone = la_zone_poly_.valeur();
  const Champ_Face_PolyMAC& ch = ref_cast(Champ_Face_PolyMAC, equation().inconnue().valeur());
  if (zone.zone().nb_joints() && zone.zone().joint(0).epaisseur() < 1)
    Cerr << "Op_Diff_PolyMAC_Face : largeur de joint insuffisante (minimum 1)!" << finl, Process::exit();
  ch.init_ra(), zone.init_rf(), zone.init_m1(), zone.init_m2();

  if (que_suis_je() == "Op_Diff_PolyMAC_Face") return;
  const RefObjU& modele_turbulence = equation().get_modele(TURBULENCE);
  const Mod_turb_hyd_base& mod_turb = ref_cast(Mod_turb_hyd_base,modele_turbulence.valeur());
  const Champ_Fonc& alpha_t = mod_turb.viscosite_turbulente();
  associer_diffusivite_turbulente(alpha_t);
}

void Op_Diff_PolyMAC_Face::dimensionner(Matrice_Morse& mat) const
{
  const Zone_PolyMAC& zone = la_zone_poly_.valeur();
  const Champ_Face_PolyMAC& ch = ref_cast(Champ_Face_PolyMAC, equation().inconnue().valeur());
  const IntTab& f_e = zone.face_voisins(), &e_f = zone.elem_faces();
  int i, j, a, e, f, fb, nf_tot = zone.nb_faces_tot(), na_tot = dimension < 3 ? zone.zone().nb_som_tot() : zone.zone().nb_aretes_tot();

  zone.init_m2();

  IntTab stencil(0, 2);
  stencil.set_smart_resize(1);
  //partie vitesses : nu grad(div) - m2 Rf
  for (f = 0; f < zone.nb_faces(); f++) if (ch.icl(f, 0) < 2)
      {
        //nu grad(div)
        if (0 && f_e(f, 1) >= 0) for (i = 0; i < 2 && (e = f_e(f, i)) >= 0; i++) for (j = 0; j < e_f.dimension(1) && (fb = e_f(e, j)) >= 0; j++)
              if (ch.icl(fb, 0) < 2) stencil.append_line(f, fb);
        // m2 Rf
        for (i = zone.m2deb(f); i < zone.m2deb(f + 1); i++)
          for (fb = zone.m2ji(i, 0), j = zone.rfdeb(fb); j < zone.rfdeb(fb + 1); j++)
            stencil.append_line(f, nf_tot + zone.rfji(j));
      }

  //partie vorticites : Ra m2 - m1 / nu
  for (a = 0; a < (dimension < 3 ? zone.nb_som() : zone.zone().nb_aretes()); a++)
    {
      for (i = ch.radeb(a, 0); i < ch.radeb(a + 1, 0); i++) stencil.append_line(nf_tot + a, ch.raji(i));
      for (i = zone.m1deb(a); i < zone.m1deb(a + 1); i++) stencil.append_line(nf_tot + a, nf_tot + zone.m1ji(i, 0));
    }

  tableau_trier_retirer_doublons(stencil);
  Matrix_tools::allocate_morse_matrix(nf_tot + na_tot, nf_tot + na_tot, stencil, mat);
}

// ajoute la contribution de la convection au second membre resu
// renvoie resu
inline DoubleTab& Op_Diff_PolyMAC_Face::ajouter(const DoubleTab& inco, DoubleTab& resu) const
{
  const Zone_PolyMAC& zone = la_zone_poly_.valeur();
  const Champ_Face_PolyMAC& ch = ref_cast(Champ_Face_PolyMAC, equation().inconnue().valeur());
  const Conds_lim& cls = la_zcl_poly_.valeur().les_conditions_limites();
  const DoubleVect& pe = zone.porosite_elem();
  int i, j, k, f, fb, a, nf_tot = zone.nb_faces_tot();

  remplir_nu(nu_);
  //partie vitesses : m2 Rf
  for (f = 0; f < zone.nb_faces(); f++) if (ch.icl(f, 0) < 2)
      {
        for (i = zone.m2deb(f); i < zone.m2deb(f + 1); i++) for (fb = zone.m2ji(i, 0), j = zone.rfdeb(fb); j < zone.rfdeb(fb + 1); j++)
            resu(f) -= zone.m2ci(i) * pe(zone.m2ji(i, 1)) * zone.rfci(j) * inco(nf_tot + zone.rfji(j));
      }

  //partie vorticites : Ra m2 - m1 / nu
  if (resu.dimension_tot(0) == nf_tot) return resu; //resu ne contient que la partie "faces"
  for (a = 0; a < (dimension < 3 ? zone.nb_som() : zone.zone().nb_aretes()); a++)
    {
      //rotationnel : vitesses internes
      for (i = ch.radeb(a, 0); i < ch.radeb(a + 1, 0); i++)
        resu(nf_tot + a) -= ch.raci(i) * inco(ch.raji(i));
      //rotationnel : vitesses aux bords
      for (i = ch.radeb(a, 1); i < ch.radeb(a + 1, 1); i++) for (k = 0; k < dimension; k++)
          resu(nf_tot + a) -= ch.racf(i, k) * ref_cast(Dirichlet, cls[ch.icl(ch.rajf(i), 1)].valeur()).val_imp(ch.icl(ch.rajf(i), 2), k);
      // -m1 / nu
      for (i = zone.m1deb(a); i < zone.m1deb(a + 1); i++)
        resu(nf_tot + a) += zone.m1ci(i) / (pe(zone.m1ji(i, 1)) * nu_(zone.m1ji(i, 1))) * inco(nf_tot + zone.m1ji(i, 0));
    }
  return resu;
}

//Description:
//on assemble la matrice.

inline void Op_Diff_PolyMAC_Face::contribuer_a_avec(const DoubleTab& inco, Matrice_Morse& matrice) const
{
  const Zone_PolyMAC& zone = la_zone_poly_.valeur();
  const Champ_Face_PolyMAC& ch = ref_cast(Champ_Face_PolyMAC, equation().inconnue().valeur());
  const DoubleVect& pe = zone.porosite_elem();
  int i, j, f, fb, a, nf_tot = zone.nb_faces_tot();

  remplir_nu(nu_);
  //partie vitesses : m2 Rf
  for (f = 0; f < zone.nb_faces(); f++) if (ch.icl(f, 0) < 2)
      for (i = zone.m2deb(f); i < zone.m2deb(f + 1); i++) for (fb = zone.m2ji(i, 0), j = zone.rfdeb(fb); j < zone.rfdeb(fb + 1); j++)
          matrice(f, nf_tot + zone.rfji(j)) += zone.m2ci(i) * pe(zone.m2ji(i, 1)) * zone.rfci(j);

  //partie vorticites : Ra m2 - m1 / nu
  for (a = 0; a < (dimension < 3 ? zone.nb_som() : zone.zone().nb_aretes()); a++)
    {
      //rotationnel : vitesses internes
      for (i = ch.radeb(a, 0); i < ch.radeb(a + 1, 0); i++) if (ch.icl(f = ch.raji(i), 0) < 2)
          matrice(nf_tot + a, f) += ch.raci(i);
      // -m1 / nu
      for (i = zone.m1deb(a); i < zone.m1deb(a + 1); i++)
        matrice(nf_tot + a, nf_tot + zone.m1ji(i, 0)) -= zone.m1ci(i) / (pe(zone.m1ji(i, 1)) * nu_(zone.m1ji(i, 1)));
    }
}

//Description:
//on ajoute la contribution du second membre.

void Op_Diff_PolyMAC_Face::contribuer_au_second_membre(DoubleTab& resu) const
{
  abort();

}
