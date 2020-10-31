/****************************************************************************
* Copyright (c) 2020, CEA
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
// File:        Eval_Dift_VDF_var_Face.cpp
// Directory:   $TRUST_ROOT/src/VDF/Operateurs/Evaluateurs
// Version:     /main/16
//
//////////////////////////////////////////////////////////////////////////////

#include <Eval_Dift_VDF_var_Face.h>
#include <Turbulence_paroi_base.h>
#include <Mod_turb_hyd_base.h>


void Eval_Dift_VDF_var_Face::associer_modele_turbulence(const Mod_turb_hyd_base& mod)
{
  le_modele_turbulence    = mod;
}

//// mettre_a_jour
//

void Eval_Dift_VDF_var_Face::mettre_a_jour( )
{
  Eval_Dift_VDF_var::mettre_a_jour();
  /*
  if (sub_type(Mod_turb_hyd_RANS,le_modele_turbulence.valeur()))
    {
      const Mod_turb_hyd_RANS& mod_K_eps = ref_cast(Mod_turb_hyd_RANS,le_modele_turbulence.valeur());
      k_.ref(mod_K_eps.eqn_transp_K_Eps().inconnue().valeurs());
  } */
  if (le_modele_turbulence->loi_paroi().non_nul())
    {
      // Modif E. Saikali : on fait le ref seulement si le tableau a ete initialise, sinon pointeur nulle
      const DoubleTab& tab = le_modele_turbulence->loi_paroi().valeur().Cisaillement_paroi();
      if (tab.size_array() > 0)
        tau_tan_.ref(tab);
      //tau_tan_.ref(loipar->Cisaillement_paroi());
    }
}
