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
// File:        Masse_CoviMAC_Face.h
// Directory:   $TRUST_ROOT/src/CoviMAC/Solveurs
// Version:     /main/2
//
//////////////////////////////////////////////////////////////////////////////

#ifndef Masse_CoviMAC_Face_included
#define Masse_CoviMAC_Face_included


#include <Solveur_Masse.h>
#include <Ref_Zone_CoviMAC.h>
#include <Ref_Zone_Cl_CoviMAC.h>
#include <DoubleTab.h>

//////////////////////////////////////////////////////////////////////////////
//
// CLASS: Masse_CoviMAC_Face
//
//////////////////////////////////////////////////////////////////////////////

class Masse_CoviMAC_Face : public Solveur_Masse_base
{

  Declare_instanciable(Masse_CoviMAC_Face);

public:

  void associer_zone_dis_base(const Zone_dis_base& );
  void associer_zone_cl_dis_base(const Zone_Cl_dis_base& );

  DoubleTab& appliquer_impl(DoubleTab& ) const;

  void completer();
  /* calcule les permeabilites dv_e = dt W_e (grap p)_e et les facteurs v_f = mu_f v_{f,am} + (1-mu_f) v_{f,av} */
  /* les utilise pour remplir les lignes aux faces de la matrice */
  virtual Matrice_Base& ajouter_masse(double dt, Matrice_Base& matrice, int penalisation = 1) const;
  /* doit etre appele apres le ajouter_masse matriciel */
  DoubleTab& ajouter_masse(double dt, DoubleTab& x, const DoubleTab& y, int penalisation = 1) const;
  virtual DoubleTab& corriger_solution(DoubleTab& x, const DoubleTab& y) const;

  /* sensibilites dv_e = - ds W_e d(grad p_e) et ponderations v_f = mu_f v_{f,am} + (1-mu_f) v_{f,av} */
  mutable DoubleTab W_e; //W_e(e, n_comp, i, j)
  mutable DoubleTab mu_f; //mu_f(f, n_comp)

private:

  REF(Zone_CoviMAC) la_zone_CoviMAC;
  REF(Zone_Cl_CoviMAC) la_zone_Cl_CoviMAC;
};

#endif





