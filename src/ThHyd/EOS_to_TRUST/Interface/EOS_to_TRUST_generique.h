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

#ifndef EOS_to_TRUST_generique_included
#define EOS_to_TRUST_generique_included

#include <EOS_to_TRUST.h>

class EOS_to_TRUST_generique : public EOS_to_TRUST
{
public :
  void set_EOS_generique(const char *const model_name, const char *const fluid_name);

  // en temperature
  void eos_get_rho_pT(const SpanD P, const SpanD T, SpanD R, int ncomp = 1, int id = 0) const override;
  void eos_get_rho_dp_pT(const SpanD P, const SpanD T, SpanD R, int ncomp = 1, int id = 0) const override;
  void eos_get_rho_dT_pT(const SpanD P, const SpanD T, SpanD R, int ncomp = 1, int id = 0) const override;
  void eos_get_h_pT(const SpanD P, const SpanD T, SpanD R, int ncomp = 1, int id = 0) const override;
  void eos_get_h_dp_pT(const SpanD P, const SpanD T, SpanD R, int ncomp = 1, int id = 0) const override;
  void eos_get_h_dT_pT(const SpanD P, const SpanD T, SpanD R, int ncomp = 1, int id = 0) const override;
  void eos_get_cp_pT(const SpanD P, const SpanD T, SpanD R, int ncomp = 1, int id = 0) const override;
  void eos_get_beta_pT(const SpanD P, const SpanD T, SpanD R, int ncomp = 1, int id = 0) const override;
  void eos_get_mu_pT(const SpanD P, const SpanD T, SpanD R, int ncomp = 1, int id = 0) const override;
  void eos_get_lambda_pT(const SpanD P, const SpanD T, SpanD R, int ncomp = 1, int id = 0) const override;

  // en enthalpie
  void eos_get_rho_ph(const SpanD P, const SpanD H, SpanD R, int ncomp = 1, int id = 0) const override;
  void eos_get_rho_dp_ph(const SpanD P, const SpanD H, SpanD R, int ncomp = 1, int id = 0) const override;
  void eos_get_rho_dh_ph(const SpanD P, const SpanD H, SpanD R, int ncomp = 1, int id = 0) const override;
  void eos_get_T_ph(const SpanD P, const SpanD H, SpanD R, int ncomp = 1, int id = 0) const override;
  void eos_get_T_dp_ph(const SpanD P, const SpanD H, SpanD R, int ncomp = 1, int id = 0) const override;
  void eos_get_T_dh_ph(const SpanD P, const SpanD H, SpanD R, int ncomp = 1, int id = 0) const override;
  void eos_get_cp_ph(const SpanD P, const SpanD H, SpanD R, int ncomp = 1, int id = 0) const override;
  void eos_get_cp_dp_ph(const SpanD P, const SpanD H, SpanD R, int ncomp = 1, int id = 0) const override;
  void eos_get_cp_dh_ph(const SpanD P, const SpanD H, SpanD R, int ncomp = 1, int id = 0) const override;
  void eos_get_mu_ph(const SpanD P, const SpanD H, SpanD R, int ncomp = 1, int id = 0) const override;
  void eos_get_mu_dp_ph(const SpanD P, const SpanD H, SpanD R, int ncomp = 1, int id = 0) const override;
  void eos_get_mu_dh_ph(const SpanD P, const SpanD H, SpanD R, int ncomp = 1, int id = 0) const override;
  void eos_get_lambda_ph(const SpanD P, const SpanD H, SpanD R, int ncomp = 1, int id = 0) const override;
  void eos_get_lambda_dp_ph(const SpanD P, const SpanD H, SpanD R, int ncomp = 1, int id = 0) const override;
  void eos_get_lambda_dh_ph(const SpanD P, const SpanD H, SpanD R, int ncomp = 1, int id = 0) const override;
  void eos_get_sigma_ph(const SpanD P, const SpanD H, SpanD R, int ncomp = 1, int id = 0) const override;
  void eos_get_d_sigma_d_p_ph(const SpanD P, const SpanD H, SpanD R, int ncomp = 1, int id = 0) const override;

  void eos_get_cp_mu_lambda_beta_pT(const SpanD P, const SpanD T, MSpanD prop, int ncomp = 1, int id = 0) const override;
  void eos_get_all_pT(MSpanD inter, MSpanD bord, int ncomp = 1, int id = 0) const override;
  void eos_get_all_loi_F5(MSpanD sats, int ncomp = 1, int id = 0, bool is_liq = true) const override;
};

#endif /* EOS_to_TRUST_generique_included */
