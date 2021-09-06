/****************************************************************************
* Copyright (c) 2021, CEA
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
// File:        Champ_front_debit_massique.cpp
// Directory:   $TRUST_ROOT/src/Kernel/Champs
// Version:     /main/5
//
//////////////////////////////////////////////////////////////////////////////


#include <Champ_front_debit_massique.h>
#include <Champ_Don.h>
#include <Champ_Inc_base.h>
#include <Champ_Uniforme.h>
#include <Equation_base.h>
#include <Milieu_base.h>
#include <Zone_VF.h>
#include <DoubleTrav.h>

Implemente_instanciable(Champ_front_debit_massique,"Champ_front_debit_massique",Champ_front_debit);
// XD champ_front_debit_massique front_field_base champ_front_debit_massique 0 This field is used to define a flow rate field using the density
// XD attr ch front_field_base ch 0 uniform field in space to define the flow rate. It could be, for example, champ_front_uniforme, ch_front_input_uniform or champ_front_fonc_txyz that depends only on time.
Sortie& Champ_front_debit_massique::printOn(Sortie& os) const
{
  return Champ_front_debit::printOn(os);
}

Entree& Champ_front_debit_massique::readOn(Entree& is)
{
  return Champ_front_debit::readOn(is);
}

void Champ_front_debit_massique::initialiser_coefficient(const Champ_Inc_base& inco, double temps)
{
  Champ_front_debit::initialiser_coefficient(inco, temps);
  int i, fb, n, N = coeff_.line_size();
  const Champ_base& masse_volumique = inco.equation().milieu().masse_volumique();
  const Front_VF& le_bord= ref_cast(Front_VF,frontiere_dis());
  const Zone_VF& zone = ref_cast(Zone_VF, inco.equation().zone_dis().valeur());
  DoubleTab rho_bord = masse_volumique.valeur_aux_bords(); /* valeur a toutes les faces de bord */
  for(rho_.resize(le_bord.nb_faces_tot(), N), i = 0; i < le_bord.nb_faces_tot(); i++)
    for (fb = zone.fbord(le_bord.num_face(i)), n = 0; n < N; ++n)
      rho_(i, n) = rho_bord(fb, n);

  assert(flow_rate_.valeur().valeurs().line_size() == rho_.line_size());
  const int crho = sub_type(Champ_Uniforme, masse_volumique);
  update_coeff_ = !crho;
  update_coeff(temps);
}

void Champ_front_debit_massique::update_coeff(double temps)
{
  int i, N = coeff_.line_size();
  const Front_VF& le_bord= ref_cast(Front_VF,frontiere_dis());
  for(i = 0; i < le_bord.nb_faces_tot(); i++) for (int n = 0; n < N; ++n)
      coeff_(i, n) = 1. / rho_(i, n);
}
