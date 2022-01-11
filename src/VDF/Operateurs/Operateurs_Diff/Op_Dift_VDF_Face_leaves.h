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
// File:        Op_Dift_VDF_Face_leaves.h
// Directory:   $TRUST_ROOT/src/VDF/Operateurs/Operateurs_Diff
// Version:     /main/13
//
//////////////////////////////////////////////////////////////////////////////

#ifndef Op_Dift_VDF_Face_leaves_included
#define Op_Dift_VDF_Face_leaves_included

#include <Eval_Dift_VDF_Face_leaves.h>
#include <Op_Dift_VDF_Face_base.h>
#include <Op_Dift_VDF.h>

#ifdef DOXYGEN_SHOULD_SKIP_THIS // seulement un truc inutile pour check_source ...
class Op_Dift_VDF_Face_leaves
{ };
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

class Mod_turb_hyd_base;
class Champ_Fonc;

//////////////// CONST /////////////////

declare_It_VDF_Face(Eval_Dift_VDF_const_Face)
class Op_Dift_VDF_Face : public Op_Dift_VDF_Face_base, public Op_Dift_VDF<Op_Dift_VDF_Face>
{
  Declare_instanciable_sans_constructeur(Op_Dift_VDF_Face);
public:
  Op_Dift_VDF_Face();
  inline void completer() { completer_impl<Type_Operateur::Op_FACE,Eval_Dift_VDF_const_Face>(); }
  inline void associer(const Zone_dis& zd, const Zone_Cl_dis& zcd, const Champ_Inc& ch) { associer_impl<Type_Operateur::Op_FACE,Eval_Dift_VDF_const_Face>(zd,zcd,ch); }
  inline void associer_diffusivite(const Champ_base& ch) { associer_diffusivite_impl<Eval_Dift_VDF_const_Face>(ch); }
  inline void associer_diffusivite_turbulente(const Champ_Fonc& ch) { associer_diffusivite_turbulente_impl<Eval_Dift_VDF_const_Face>(ch); }
  inline const Champ_base& diffusivite() const { return diffusivite_impl<Eval_Dift_VDF_const_Face>(); }
};

//////////////// VAR /////////////////

declare_It_VDF_Face(Eval_Dift_VDF_var_Face)
class Op_Dift_VDF_var_Face : public Op_Dift_VDF_Face_base, public Op_Dift_VDF<Op_Dift_VDF_var_Face>
{
  Declare_instanciable_sans_constructeur(Op_Dift_VDF_var_Face);

public:
  Op_Dift_VDF_var_Face();
  inline void completer() { completer_impl<Type_Operateur::Op_FACE,Eval_Dift_VDF_var_Face>(); }
  inline void associer(const Zone_dis& zd, const Zone_Cl_dis& zcd, const Champ_Inc& ch) { associer_impl<Type_Operateur::Op_FACE,Eval_Dift_VDF_var_Face>(zd,zcd,ch); }
  inline void associer_diffusivite(const Champ_base& ch) { associer_diffusivite_impl<Eval_Dift_VDF_var_Face>(ch); }
  inline void associer_diffusivite_turbulente(const Champ_Fonc& ch) { associer_diffusivite_turbulente_impl<Eval_Dift_VDF_var_Face>(ch); }
  inline const Champ_base& diffusivite() const { return diffusivite_impl<Eval_Dift_VDF_var_Face>(); }
};

#endif /* Op_Dift_VDF_Face_leaves_included */
