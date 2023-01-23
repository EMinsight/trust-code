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

#ifndef Eval_Diff_VDF_Elem_leaves_included
#define Eval_Diff_VDF_Elem_leaves_included

#include <Eval_Diff_VDF_Multi_inco_const.h>
#include <Eval_Diff_VDF_Multi_inco_var.h>
#include <Eval_Diff_VDF_var_aniso.h>
#include <Eval_Diff_VDF_const.h>
#include <Eval_Diff_VDF_Elem.h>
#include <Eval_Diff_VDF_var.h>


/// \cond DO_NOT_DOCUMENT
class Eval_Diff_VDF_Elem_leaves
{};
/// \endcond

/*
 * ******************************
 * CAS SCALAIRE - const/var/aniso
 * ******************************
 */

/*! @brief class Eval_Diff_VDF_const_Elem_Axi Evaluateur VDF pour la diffusion en coordonnees cylindriques
 *
 *  Le champ diffuse est scalaire (Champ_P0_VDF). Le champ de diffusivite est constant.
 *
 * @sa Eval_Diff_VDF_const
 */
class Eval_Diff_VDF_const_Elem_Axi : public Eval_Diff_VDF_Elem<Eval_Diff_VDF_const_Elem_Axi>, public Eval_Diff_VDF_const
{
public:
  static constexpr bool IS_AXI = true;
};

/*! @brief class Eval_Diff_VDF_const_Elem Evaluateur VDF pour la diffusion
 *
 *  Le champ diffuse est scalaire (Champ_P0_VDF). Le champ de diffusivite est constant.
 *
 * @sa Eval_Diff_VDF_const
 */
class Eval_Diff_VDF_const_Elem : public Eval_Diff_VDF_Elem<Eval_Diff_VDF_const_Elem>, public Eval_Diff_VDF_const {};


/*! @brief class Eval_Diff_VDF_var_Elem_Axi Evaluateur VDF pour la diffusion en coordonnees cylindriques
 *
 *  Le champ diffuse est scalaire (Champ_P0_VDF). Le champ de diffusivite n'est pas constant.
 *
 * @sa Eval_Diff_VDF_var
 */
class Eval_Diff_VDF_var_Elem_Axi :public Eval_Diff_VDF_Elem<Eval_Diff_VDF_var_Elem_Axi>, public Eval_Diff_VDF_var
{
public:
  static constexpr bool IS_AXI = true;
};

/*! @brief class Eval_Diff_VDF_var_Elem Evaluateur VDF pour la diffusion
 *
 *  Le champ diffuse est scalaire (Champ_P0_VDF). Le champ de diffusivite n'est pas constant.
 *
 * @sa Eval_Diff_VDF_var
 */
class Eval_Diff_VDF_var_Elem : public Eval_Diff_VDF_Elem<Eval_Diff_VDF_var_Elem>, public Eval_Diff_VDF_var {};

/*! @brief class Eval_Diff_VDF_var_Elem_aniso Evaluateur VDF pour la diffusion
 *
 *  Le champ diffuse est scalaire (Champ_P0_VDF). Le champ de diffusivite n'est pas constant.
 *
 * @sa Eval_Diff_VDF_var_aniso
 */
class Eval_Diff_VDF_var_Elem_aniso : public Eval_Diff_VDF_Elem<Eval_Diff_VDF_var_Elem_aniso>, public Eval_Diff_VDF_var_aniso
{
public:
  static constexpr bool IS_ANISO= true;
};

/*
 * ******************************
 * CAS VECTORIEL - const/var
 * ******************************
 */

/*! @brief class Eval_Diff_VDF_Multi_inco_const_Elem_Axi Evaluateur VDF pour la diffusion en coordonnees cylindriques
 *
 *  Le champ diffuse est scalaire (Champ_P0_VDF) avec plusieurs inconnues
 *  Il y a une diffusivite par inconnue. Le champ de diffusivite associe a chaque inconnue est constant.
 *
 * @sa Eval_Diff_VDF_Multi_inco_const
 */
class Eval_Diff_VDF_Multi_inco_const_Elem_Axi : public Eval_Diff_VDF_Elem<Eval_Diff_VDF_Multi_inco_const_Elem_Axi>, public Eval_Diff_VDF_Multi_inco_const
{
public:
  static constexpr bool IS_MULTD = false, IS_AXI = true;
  void mettre_a_jour() override { }
};

/*! @brief class Eval_Diff_VDF_Multi_inco_const_Elem Evaluateur VDF pour la diffusion
 *
 *  Le champ diffuse est scalaire (Champ_P0_VDF) avec plusieurs inconnues
 *  Il y a une diffusivite par inconnue. Le champ de diffusivite associe a chaque inconnue est constant.
 *
 * @sa Eval_Diff_VDF_Multi_inco_const
 */
class Eval_Diff_VDF_Multi_inco_const_Elem : public Eval_Diff_VDF_Elem<Eval_Diff_VDF_Multi_inco_const_Elem>, public Eval_Diff_VDF_Multi_inco_const
{
public:
  static constexpr bool IS_MULTD = false;
  void mettre_a_jour() override { }
};

/*! @brief class Eval_Diff_VDF_Multi_inco_var_Elem_Axi Evaluateur VDF pour la diffusion en coordonnees cylindriques
 *
 *  Le champ diffuse est scalaire (Champ_P0_VDF) avec plusieurs inconnues
 *  Il y a une diffusivite par inconnue. Le champ de diffusivite associe a chaque inconnue n'est pas constant.
 *
 * @sa Eval_Diff_VDF_Multi_inco_var
 */
class Eval_Diff_VDF_Multi_inco_var_Elem_Axi : public Eval_Diff_VDF_Elem<Eval_Diff_VDF_Multi_inco_var_Elem_Axi>, public Eval_Diff_VDF_Multi_inco_var
{
public:
  static constexpr bool IS_MULTD = false, IS_AXI = true;
  void mettre_a_jour() override { }
};

/*! @brief class Eval_Diff_VDF_Multi_inco_var_Elem Evaluateur VDF pour la diffusion
 *
 *  Le champ diffuse est scalaire (Champ_P0_VDF) avec plusieurs inconnues
 *  Il y a une diffusivite par inconnue. Le champ de diffusivite associe a chaque inconnue n'est pas constant.
 *
 * @sa Eval_Diff_VDF_Multi_inco_var
 */
class Eval_Diff_VDF_Multi_inco_var_Elem : public Eval_Diff_VDF_Elem<Eval_Diff_VDF_Multi_inco_var_Elem>, public Eval_Diff_VDF_Multi_inco_var
{
public:
  static constexpr bool IS_MULTD = false;
  void mettre_a_jour() override { }
};

#endif /* Eval_Diff_VDF_Elem_leaves_included */
