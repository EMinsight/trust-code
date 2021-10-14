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
// File:        Loi_Etat_base.h
// Directory:   $TRUST_ROOT/src/ThHyd/Fluide_Dilatable
// Version:     /main/18
//
//////////////////////////////////////////////////////////////////////////////

#ifndef Loi_Etat_base_included
#define Loi_Etat_base_included

#include <Ref_Fluide_Quasi_Compressible.h>
#include <Champ_Don.h>
#include <Champs_compris.h>
#include <Champs_compris_interface.h>

class Fluide_Quasi_Comp;

//////////////////////////////////////////////////////////////////////////////
//
// .DESCRIPTION
//     classe Loi_Etat_base
//     Cette classe est la base de la hierarchie des lois d'etat.
//     Associe a un fluide incompressible, elle definit un fluide quasi compressible
// .SECTION voir aussi
//     Fluide_Quasi_Compressible
//     Classe abstraite dont toutes les lois d'etat doivent deriver.
//     Methodes abstraites:
//       void calculer_coeff_T()
//       void calculer_masse_volumique()
//////////////////////////////////////////////////////////////////////////////

class Loi_Etat_base : public Objet_U, public Champs_compris_interface
{
  Declare_base_sans_constructeur(Loi_Etat_base);

public :

  Loi_Etat_base();

  virtual void associer_fluide(const Fluide_Quasi_Compressible&);
  virtual void initialiser() =0;
  const virtual Nom type_fluide() const =0;
  virtual void preparer_calcul();
  void mettre_a_jour(double);
  virtual void abortTimeStep();

  virtual void initialiser_inco_ch();
  virtual double inverser_Pth(double,double) =0;
  virtual void remplir_T() =0;
  virtual void calculer_Cp() =0;
  virtual void calculer_mu();
  virtual void calculer_lambda();
  void calculer_nu();
  virtual void calculer_alpha();
  virtual void calculer_mu_sur_Sc();
  virtual void calculer_nu_sur_Sc();

  virtual void calculer_masse_volumique();
  virtual double calculer_masse_volumique(double,double) const =0;
  virtual double calculer_H(double,double) const;

  Champ_Don& ch_temperature();
  const Champ_Don& ch_temperature() const;
  inline const DoubleTab& temperature() const;
  inline const DoubleTab& rho_n() const;
  inline const DoubleTab& rho_np1() const;
  inline double Prandt() const;

  virtual double Drho_DP(double,double) const ;
  virtual double Drho_DT(double,double) const ;
  virtual double De_DP(double,double) const ;
  virtual double De_DT(double,double) const ;

  //Pour pouvoir acceder a champs_compris_ dans la classe de discretisation
  inline Champs_compris& champs_compris();
  //Methodes de l interface des champs postraitables
  /////////////////////////////////////////////////////
  virtual void creer_champ(const Motcle& motlu);
  virtual const Champ_base& get_champ(const Motcle& nom) const;
  virtual void get_noms_champs_postraitables(Noms& nom,Option opt=NONE) const;
  /////////////////////////////////////////////////////

protected :
  REF(Fluide_Quasi_Compressible) le_fluide;
  Champ_Don temperature_;
  DoubleTab tab_rho_n;    //rho a l'etape precedente
  DoubleTab tab_rho_np1;    //rho a l'etape suivante
  double Pr_;
  int debug;

private :

  Champs_compris champs_compris_;
};

inline double Loi_Etat_base::Prandt() const
{
  return Pr_;
}

inline const DoubleTab& Loi_Etat_base::temperature() const
{
  return ch_temperature().valeurs();
}


inline const DoubleTab& Loi_Etat_base::rho_n() const
{
  return tab_rho_n;
}
inline const DoubleTab& Loi_Etat_base::rho_np1() const
{
  return tab_rho_np1;
}

inline Champs_compris& Loi_Etat_base::champs_compris()
{
  return champs_compris_;
}
#endif
