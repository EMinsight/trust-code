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
// File:        Loi_Etat_Binaire_GP_QC.h
// Directory:   $TRUST_ROOT/src/ThHyd/Fluide_Dilatable/Quasi_Compressible/Loi_Etat
// Version:     /main/11
//
//////////////////////////////////////////////////////////////////////////////

#ifndef Loi_Etat_Binaire_GP_QC_included
#define Loi_Etat_Binaire_GP_QC_included

#include <Loi_Etat_Binaire_GP_base.h>
#include <Champ_Inc_base.h>
#include <Ref_Champ_Inc_base.h>

//////////////////////////////////////////////////////////////////////////////
//
// .DESCRIPTION
//     classe Loi_Etat_Binaire_GP_QC
//     Cette classe represente la loi d'etat pour les melanges binaires.
//     Associe a un fluide incompressible, elle definit un fluide binaire quasi compressible
//     dont la loi d'eata est :
//        Pth = rho*R*T*(Y1/M1+Y2/M2)
// .SECTION voir aussi
//     Loi_Etat_Binaire_GP_base
//////////////////////////////////////////////////////////////////////////////

class Loi_Etat_Binaire_GP_QC : public Loi_Etat_Binaire_GP_base
{
  Declare_instanciable_sans_constructeur(Loi_Etat_Binaire_GP_QC);

public :
  Loi_Etat_Binaire_GP_QC();
  void associer_fluide(const Fluide_Dilatable_base& fl);
  void calculer_lambda();
  void calculer_mu();
  void calculer_mu_wilke();
  void calculer_alpha();
  void calculer_mu_sur_Sc(); // returns rho * D
  void calculer_nu_sur_Sc(); // returns D
  void calculer_masse_volumique();

  const Nom type_fluide() const;
  void calculer_Cp();
  double calculer_masse_volumique(double P,double Y1) const;
  double inverser_Pth(double,double);

protected :
  double massmol1_,massmol2_,mu1_,mu2_,tempr_,diff_coeff_;

private :
  static constexpr double R_GAS = 8.314472;
};

#endif /* Loi_Etat_Binaire_GP_QC_included */
