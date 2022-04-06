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
// File:        Masse_CoviMAC_Elem.cpp
// Directory:   $TRUST_ROOT/src/CoviMAC/Solveurs
// Version:     /main/1
//
//////////////////////////////////////////////////////////////////////////////

#include <Op_Diff_negligeable.h>
#include <Echange_impose_base.h>
#include <Masse_CoviMAC_Elem.h>
#include <Zone_Cl_CoviMAC.h>
#include <TRUSTTab_parts.h>
#include <Probleme_base.h>
#include <Equation_base.h>
#include <Zone_CoviMAC.h>
#include <Matrix_tools.h>
#include <Operateur.h>

Implemente_instanciable(Masse_CoviMAC_Elem,"Masse_CoviMAC_Elem",Solveur_Masse_base);


//     printOn()
/////

Sortie& Masse_CoviMAC_Elem::printOn(Sortie& s) const
{
  return s << que_suis_je() << " " << le_nom();
}

Entree& Masse_CoviMAC_Elem::readOn(Entree& s)
{
  return s ;
}


////////////////////////////////////////////////////////////////
//
//  Implementation des fonctions de la classe Masse_CoviMAC_Elem
//
////////////////////////////////////////////////////////////////

void Masse_CoviMAC_Elem::preparer_calcul()
{
  equation().init_champ_conserve();
}

DoubleTab& Masse_CoviMAC_Elem::appliquer_impl(DoubleTab& sm) const
{
  const Zone_CoviMAC& zone = la_zone_CoviMAC.valeur();
  const DoubleVect& ve = zone.volumes(), &pe = zone.porosite_elem();
  const DoubleTab& der = equation().champ_conserve().derivees().at(equation().inconnue().le_nom().getString());

  int e, ne_tot = zone.nb_elem_tot(), n, N = sm.line_size();
  assert(sm.dimension_tot(0) == ne_tot && N == der.line_size());


  for (e = 0; e < ne_tot; e++) for (n = 0; n < N; n++)
      if (der(e, n) > 1e-10) sm(e, n) /= pe(e) * ve(e) * der(e, n);
      else sm(e, n) = 0; //cas d'une evanescence

  return sm;
}

void Masse_CoviMAC_Elem::dimensionner_blocs(matrices_t matrices, const tabs_t& semi_impl) const
{
  /* une diagonale par derivee de champ_conserve_ presente dans matrices */
  const Zone_CoviMAC& zone = la_zone_CoviMAC.valeur();
  const Champ_Inc_base& cc = equation().champ_conserve();
  int e, ne = zone.nb_elem(), ne_tot = zone.nb_elem_tot(), n, N = cc.valeurs().line_size();

  for (auto &&i_m : matrices) if (cc.derivees().count(i_m.first))
      {
        /* nombre de composantes de la variable : autant que le champ par defaut, mais peut etre different pour la pression */
        int m, M = equation().probleme().get_champ(i_m.first.c_str()).valeurs().line_size();

        IntTrav stencil(0, 2);
        stencil.set_smart_resize(1);

        for (e = 0; e < ne; e++) for (n = 0, m = 0; n < N; n++, m += (M > 1)) stencil.append_line(N * e + n, M * e + m);
        Matrice_Morse mat;
        Matrix_tools::allocate_morse_matrix(N * ne_tot, M * ne_tot, stencil, mat);
        i_m.second->nb_colonnes() ? *i_m.second += mat : *i_m.second = mat;
      }
}

void Masse_CoviMAC_Elem::ajouter_blocs(matrices_t matrices, DoubleTab& secmem, double dt, const tabs_t& semi_impl, int resoudre_en_increments) const
{
  const Zone_CoviMAC& zone = la_zone_CoviMAC.valeur();
  const Champ_Inc_base& cc = equation().champ_conserve();
  const DoubleTab& present = cc.valeurs(), &passe = cc.passe();
  const DoubleVect& ve = zone.volumes(), &pe = zone.porosite_elem();
  int e, n, N = cc.valeurs().line_size(), ne = zone.nb_elem();

  /* second membre : avec ou sans resolution en increments*/
  for (e = 0; e < ne; e++) for (n = 0; n < N; n++)
      secmem(e, n) += pe(e) * ve(e) * (passe(e, n) - resoudre_en_increments * present(e, n)) / dt;

  /* matrices */
  for (auto &&i_m : matrices) if (cc.derivees().count(i_m.first))
      {
        int m, M = equation().probleme().get_champ(i_m.first.c_str()).valeurs().line_size();
        const DoubleTab& der = cc.derivees().at(i_m.first);
        for (e = 0; e < ne; e++) for (n = 0, m = 0; n < N; n++, m += (M > 1))
            (*i_m.second)(N * e + n, M * e + m) += pe(e) * ve(e) * der(e, n) / dt;
      }
}

void Masse_CoviMAC_Elem::associer_zone_dis_base(const Zone_dis_base& la_zone_dis_base)
{
  la_zone_CoviMAC = ref_cast(Zone_CoviMAC, la_zone_dis_base);
}

void Masse_CoviMAC_Elem::associer_zone_cl_dis_base(const Zone_Cl_dis_base& la_zone_Cl_dis_base)
{
  la_zone_Cl_CoviMAC = ref_cast(Zone_Cl_CoviMAC, la_zone_Cl_dis_base);
}
