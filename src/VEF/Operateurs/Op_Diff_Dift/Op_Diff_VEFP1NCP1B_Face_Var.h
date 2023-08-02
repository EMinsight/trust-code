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


#ifndef Op_Diff_VEFP1NCP1B_Face_Var_included
#define Op_Diff_VEFP1NCP1B_Face_Var_included

#include <Op_Diff_VEFP1NCP1B_Face.h>

/*! @brief class Op_Diff_VEFP1NCP1B_Face_Var Cette classe represente l'operateur de diffusion
 *
 *   La discretisation est VEF
 *   Le champ diffuse est scalaire
 *   Le champ de diffusivite est fonction de T
 *   Operateur de diffusion qui permet l'utilisation de variables changeant au
 *   cours du temps. Il s'oppose a l'operateur constant Op_Diff_VEF_Face en VEF.
 *   Il existe le meme en version VDF.
 *
 *
 *
 */

class Op_Diff_VEFP1NCP1B_Face_Var : public Op_Diff_VEFP1NCP1B_Face
{
  Declare_instanciable_sans_constructeur(Op_Diff_VEFP1NCP1B_Face_Var);
public:
  Op_Diff_VEFP1NCP1B_Face_Var();
};
#endif

