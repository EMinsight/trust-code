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
// File:        Flux_parietal_base.h
// Directory:   $TRUST_ROOT/src/ThHyd/Multiphase/Correlations
// Version:     /main/18
//
//////////////////////////////////////////////////////////////////////////////

#ifndef Flux_parietal_base_included
#define Flux_parietal_base_included
#include <DoubleTab.h>
#include <Correlation_base.h>

//////////////////////////////////////////////////////////////////////////////
//
// .DESCRIPTION
//    classe Flux_parietal_base
//      correlations de flux parietal de la forme
//        flux de chaleur sensible : q_{p}(k)     = F(alpha_f, p, T_f, T_p, v_f, D_h, D_ch)
//        flux de chaleur latente  : q_{pi}(k, l) = F(alpha_f, p, T_f, T_p, v_f, D_h, D_ch)
//          (par ex ebullition nucleee : Gamma_{kl} = q_{pi}(k, l) / Lvap)
//      cette classe definit deux fonctions q_pk, q_pi avec :
//    entrees :
//        N : nombre de phases
//        D_h, D_ch -> diametre hyd, diametre hyd chauffant
//        alpha[n]  -> taux de presence de la phase n
//        T[n]      -> temperature de la phase n
//        p         -> pression
//        v[n]      -> norme de la vitesse de la phase n
//        Tp        -> temperature de la paroi (une seule!)
//        lambda[n], mu[n], rho[n], Cp[n] -> diverses proprietes physiques de la phase n
//
//    sorties :
//           qpk[n]         -> flux de chaleur vers la phase n
//        da_qpk[N * n + m] -> derivee par rapport a alpha_m
//        dp_qpk[n]         -> derivee par rapport a p
//        dv_qpk[N * n + m] -> derivee par rapport a v[m]
//       dTf_qpk[N * n + m] -> derivee par rapport a T[m]
//       dTp_qpk[n]         -> derivee par rapport a Tp
//           qpi[N * k + l]           -> flux de masse de la phase k vers la phase l (a remplir pour k < l)
//        da_qpi[N * (N * k + l) + m] -> derivee par rapport a alpha_m
//        dp_qpi[N * k + l]           -> derivee par rapport a p
//        dv_qpi[N * k + l]           -> derivee par rapport a v[m]
//       dTf_qpi[N * (N * k + l) + m] -> derivee par rapport a T[m]
//       dTp_qpi[N * k + l]           -> derivee par rapport a Tp
//////////////////////////////////////////////////////////////////////////////

class Flux_parietal_base : public Correlation_base
{
  Declare_base(Flux_parietal_base);
public:
  virtual void q_pk(int N, double D_h, double D_ch,
                    const double *alpha, const double *T, const double p, const double *v, const double Tp,
                    const double *lambda, const double *mu, const double *rho, const double *Cp,
                    double *qpk, double *da_qpk, double *dp_qpk, double *dv_qpk, double *dTf_qpk, double *dTp_qpk) const = 0;
  virtual void q_pi(int N, double D_h, double D_ch,
                    const double *alpha, const double *T, const double p, const double *v, const double Tp,
                    const double *lambda, const double *mu, const double *rho, const double *Cp,
                    double *qpi, double *da_qpi, double *dp_qpi, double *dv_qpi, double *dTf_qpi, double *dTp_qpi) const = 0;
};

#endif
