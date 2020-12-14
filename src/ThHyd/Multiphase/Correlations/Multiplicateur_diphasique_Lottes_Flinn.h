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
// File:        Multiplicateur_diphasique_Lottes_Flinn.h
// Directory:   $TRUST_ROOT/src/ThHyd/Multiphase/Correlations
// Version:     /main/18
//
//////////////////////////////////////////////////////////////////////////////

#ifndef Multiplicateur_diphasique_Lottes_Flinn_included
#define Multiplicateur_diphasique_Lottes_Flinn_included
#include <Multiplicateur_diphasique_base.h>

//////////////////////////////////////////////////////////////////////////////
//
// .DESCRIPTION
//    classe Multiplicateur_diphasique_Lottes_Flinn
//      coefficients de frottement interfacial d'un ecoulement a Lottes_Flinn
//      la phase 0 (liquide) est la seule a frotter jusqu'a alpha_min
//      puis bascule vers la phase gaz sur [alpha_min, alpha_max]
//      a partir de alpha_max, seule
//      parametres :
//       - alpha_min -> debut de la transition
//       - alpha_fin -> debut de la transition
//////////////////////////////////////////////////////////////////////////////

class Multiplicateur_diphasique_Lottes_Flinn : public Multiplicateur_diphasique_base
{
  Declare_instanciable(Multiplicateur_diphasique_Lottes_Flinn);
public:
  virtual void coefficient(const double *alpha, const DoubleTab& Fk, double Fm,
                           DoubleTab& coeff) const;
protected:
  double alpha_min_, alpha_max_;
};

#endif
