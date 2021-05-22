/****************************************************************************
* Copyright (c) 2015 - 2016, CEA
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

#ifndef Eval_Diff_VDF_var_Elem_included
#define Eval_Diff_VDF_var_Elem_included

#include <Eval_Diff_VDF_var2.h>
#include <Eval_Diff_VDF_Elem.h>

//
// .DESCRIPTION class Eval_Diff_VDF_var_Elem
//
// Evaluateur VDF pour la diffusion
// Le champ diffuse est scalaire (Champ_P0_VDF)
// Le champ de diffusivite n'est pas constant.

//
// .SECTION voir aussi Eval_Diff_VDF_var

class Eval_Diff_VDF_var_Elem : public Eval_Diff_VDF_Elem<Eval_Diff_VDF_var_Elem>,
                               public Eval_Diff_VDF_var2
{
};

//// DEBUT DES DEFINES
//#define CLASSNAME Eval_Diff_VDF_var_Elem
//#define f_heq_old(d0,i,d1,j,k) heq=1./(d0/dt_diffusivite(i,k) + d1/dt_diffusivite(j,k))
//#define nu_1(i) dv_diffusivite(i)
//#define nu_2(i) dv_diffusivite(i)
//#define f_heq(d0,i,d1,j) heq=1./(d0/nu_2(i) + d1/nu_2(j))
//#define MULTD
//#undef D_AXI
//// FIN DES DEFINES
//#include <Cal_std.h>
//#include <Scal_corps_base.h>
//#include <Vect_corps_base_inut.h>
//

#endif
