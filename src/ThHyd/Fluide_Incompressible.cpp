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
// File:        Fluide_Incompressible.cpp
// Directory:   $TRUST_ROOT/src/ThHyd
// Version:     /main/32
//
//////////////////////////////////////////////////////////////////////////////

#include <Fluide_Incompressible.h>
#include <Motcle.h>
#include <Champ.h>
#include <Champ_Uniforme.h>
#include <Champ_Fonc_Tabule.h>
#include <Probleme_base.h>
#include <Discretisation_base.h>
#include <Equation_base.h>
#include <Schema_Temps_base.h>
#include <Param.h>
#include <Champ_Fonc_MED.h>

Implemente_instanciable_sans_constructeur(Fluide_Incompressible,"Fluide_Incompressible",Milieu_base);

Fluide_Incompressible::Fluide_Incompressible()
{
  /*  Noms& nom=champs_compris_.liste_noms_compris();
      nom.dimensionner(2);
      nom[0]="viscosite_dynamique";
      nom[1]="viscosite_cinematique";
  */
}

// Description:
//    Ecrit les proprietes du fluide sur un flot de sortie.
// Precondition:
// Parametre: Sortie& os
//    Signification: un flot de sortie
//    Valeurs par defaut:
//    Contraintes:
//    Acces: entree/sortie
// Retour: Sortie&
//    Signification: le flot de sortie modifie
//    Contraintes:
// Exception:
// Effets de bord:
// Postcondition: la methode ne modifie pas l'objet
Sortie& Fluide_Incompressible::printOn(Sortie& os) const
{
  os << "{" << finl;
  os << "kappa "<<coeff_absorption_ << finl;
  os << "indice "<<indice_refraction_ << finl;
  os << "longueur_rayo "<<longueur_rayo_ << finl;
  os << "mu " << mu << finl;
  os << "beta_co " << beta_co << finl;
  Milieu_base::ecrire(os);
  os << "}" << finl;
  return os;
}


// Description:
//   Lit les caracteristiques du fluide a partir d'un flot
//   d'entree.
//   Format:
//     Fluide_Incompressible
//     {
//      Mu type_champ bloc de lecture de champ
//      Rho Champ_Uniforme 1 vrel
//      [Cp Champ_Uniforme 1 vrel]
//      [Lambda type_champ bloc de lecture de champ]
//      [Beta_th type_champ bloc de lecture de champ]
//      [Beta_co type_champ bloc de lecture de champ]
//     }
// cf Milieu_base::readOn
// Precondition:
// Parametre: Entree& is
//    Signification: un flot d'entree
//    Valeurs par defaut:
//    Contraintes:
//    Acces: entree/sortie
// Retour: Entree&
//    Signification: le flot d'entree modifie
//    Contraintes:
// Exception: accolade ouvrante attendue
// Effets de bord:
// Postcondition:
Entree& Fluide_Incompressible::readOn(Entree& is)
{
  Milieu_base::readOn(is);
  return is;
}

void Fluide_Incompressible::set_param(Param& param)
{
  Milieu_base::set_param(param);
  //La lecture de rho n est pas specifiee obligatoire ici car ce
  //champ ne doit pas etre lu pour un fluide quasi-compressible
  param.ajouter("mu",&mu,Param::REQUIRED);
  param.ajouter("beta_co",&beta_co);
  param.ajouter("kappa",&coeff_absorption_);
  param.ajouter("indice",&indice_refraction_);
}

void Fluide_Incompressible::creer_champs_non_lus()
{
  Milieu_base::creer_champs_non_lus();
  creer_nu();
}

void Fluide_Incompressible::discretiser(const Probleme_base& pb, const  Discretisation_base& dis)
{
  const Zone_dis_base& zone_dis=pb.equation(0).zone_dis();
  // mu rho nu  revoir
  double temps=pb.schema_temps().temps_courant();
  if (mu.non_nul())
    if (sub_type(Champ_Fonc_MED,mu.valeur()))
      {
        Cerr<<" on convertit le champ_fonc_med en champ_don"<<finl;
        Champ_Don mu_prov;
        dis.discretiser_champ("champ_elem",zone_dis,"neant","neant",1,temps,mu_prov);
        mu_prov.affecter_(mu.valeur());
        mu.detach();
        nu.detach();
        dis.discretiser_champ("champ_elem",zone_dis,"neant","neant",1,temps,mu);

        mu.valeur().valeurs()=mu_prov.valeur().valeurs();

      }
  if (!mu.non_nul())
    {
      dis.discretiser_champ("champ_elem",zone_dis,"neant","neant",1,temps,mu);
      dis.discretiser_champ("champ_elem",zone_dis,"neant","neant",1,temps,nu);
    }
  dis.nommer_completer_champ_physique(zone_dis,"viscosite_dynamique","kg/m/s",mu.valeur(),pb);
  champs_compris_.ajoute_champ(mu.valeur());
  if (sub_type(Champ_Fonc_Tabule,mu.valeur()))
    {
      dis.discretiser_champ("champ_elem",zone_dis,"neant","neant",1,temps,nu);
    }
  if (!nu.non_nul())
    dis.discretiser_champ("champ_elem",zone_dis,"neant","neant",1,temps,nu);

  if (nu.non_nul())
    {
      dis.nommer_completer_champ_physique(zone_dis,"viscosite_cinematique","m2/s",nu.valeur(),pb);
      champs_compris_.ajoute_champ(nu.valeur());
    }
  if (beta_co.non_nul())
    {
      dis.nommer_completer_champ_physique(zone_dis,"dilatabilite_solutale",".",beta_co.valeur(),pb);
      champs_compris_.ajoute_champ(beta_co.valeur());
    }
  Milieu_base::discretiser(pb,dis);

}
// Description:
//    Verifie que les champs lus l'ont ete correctement.
// Precondition:
// Parametre:
//    Signification:
//    Valeurs par defaut:
//    Contraintes:
//    Acces:
// Retour:
//    Signification:
//    Contraintes:
// Exception: la masse volumique (rho) n'est pas strictement positive
// Exception: la masse volumique (rho) n'est pas de type Champ_Uniforme
// Exception: la viscosite (mu) n'est pas strictement positive
// Exception: l'une des proprietes (rho ou mu) du fluide n'a pas ete definie
// Exception: la capacite calorifique (Cp) n'est pas strictement positive
// Exception: la capacite calorifique (Cp) n'est pas de type Champ_Uniforme
// Exception: la conductivite (lambda) n'est pas strictement positive
// Exception: toutes les proprietes du fluide anisotherme n'ont pas ete definies
// Effets de bord:
// Postcondition:
void Fluide_Incompressible::verifier_coherence_champs(int& err,Nom& msg)
{
  msg="";
  if (rho.non_nul())
    {
      if (sub_type(Champ_Uniforme,rho.valeur()))
        {
          if (rho(0,0) <= 0)
            {
              msg += "The density rho is not striclty positive. \n";
              err = 1;
            }
        }
      else
        {
          msg += "The density rho is not of type Champ_Uniforme. \n";
          err = 1;
        }
      if (sub_type(Champ_Uniforme,mu.valeur()))
        {
          if (mu(0,0) < 0)
            {
              msg += "The dynamical viscosity mu is not positive. \n";
              err = 1;
            }
        }
    }
  else
    {
      msg += "The density rho has not been specified. \n";
      err = 1;
    }

  if  (  (Cp.non_nul()) && ( (lambda.non_nul()) && (beta_th.non_nul()) ) ) // Fluide anisotherme
    {
      if (sub_type(Champ_Uniforme,Cp.valeur()))
        {
          if  (Cp(0,0) <= 0)
            {
              msg += "The heat capacity Cp is not striclty positive. \n";
              err = 1;
            }
        }
      else
        {
          msg += "The heat capacity Cp is not of type Champ_Uniforme. \n";
          err = 1;
        }
      if (sub_type(Champ_Uniforme,lambda.valeur()))
        {
          if (lambda(0,0) < 0)
            {
              msg += "The conductivity lambda is not positive. \n";
              err = 1;
            }
        }

    }
  if  ( ( (Cp.non_nul()) || (beta_th.non_nul()) ) && (!lambda.non_nul()) )
    {
      msg += " Physical properties for an anisotherm case : \n";
      msg += "the conductivity lambda has not been specified. \n";
      err = 2;
    }
  if  ( ( (lambda.non_nul()) || (beta_th.non_nul()) ) && (!Cp.non_nul()) )
    {
      msg += " Physical properties for an anisotherm case : \n";
      msg += "the heat capacity Cp has not been specified. \n";
      err = 2;
    }
  if ( ( (lambda.non_nul()) || (Cp.non_nul()) ) && (!beta_th.non_nul()) )
    {
      msg += " Physical properties for an anisotherm case : \n";
      msg += "the thermal expansion coefficient beta_th has not been specified. \n";
      err = 2;
    }

  // Test de la coherence des proprietees radiatives du fluide incompressible
  // (pour un milieu semi transparent
  if ( (coeff_absorption_.non_nul()) && (!indice_refraction_.non_nul()) )
    {
      msg += " Physical properties for semi tranparent radiation case : \n";
      msg += "Refraction index has not been specfied while it has been done for absorption coefficient. \n";
      err = 1;
    }
  if ( (!coeff_absorption_.non_nul()) && (indice_refraction_.non_nul()) )
    {
      msg += " Physical properties for semi tranparent radiation case : \n";
      msg += "Absorption coefficient has not been specfied while it has been done for refraction index. \n";
      err = 1;
    }

  if ( (coeff_absorption_.non_nul()) && indice_refraction_.non_nul() )
    {
      if (sub_type(Champ_Uniforme,coeff_absorption_.valeur()))
        {
          if (coeff_absorption_(0,0) <= 0)
            {
              msg += "The absorption coefficient kappa is not striclty positive. \n";
              err = 1;
            }
        }
    }
  Milieu_base::verifier_coherence_champs(err,msg);
}


// Description:
//    Si l'objet reference par nu et du type Champ_Uniforme
//     type nu en "Champ_Uniforme" et le remplit
//     avec mu(0,0)/rho(0,0).
//    Sinon n efait rien.
// Precondition:
// Parametre:
//    Signification:
//    Valeurs par defaut:
//    Contraintes:
//    Acces:
// Retour:
//    Signification:
//    Contraintes:
// Exception:
// Effets de bord:
// Postcondition:
void Fluide_Incompressible::creer_nu()
{
  assert(mu.non_nul());
  assert(rho.non_nul());
  nu=mu;
  nu->nommer("nu");;
}

void Fluide_Incompressible::calculer_nu()
{
  Champ rhobis;
  rhobis=mu.valeur();
  rhobis->affecter(rho);

  DoubleTab& tabnu=nu.valeurs();
  tabnu=mu.valeurs();
  const DoubleTab& tabrho=rhobis.valeurs();
  int sz=tabnu.size();
  assert(sz==tabrho.size());
  for(int i=0; i < sz; i++)
    tabnu.addr()[i]/=(tabrho.addr()[i]);
}


// Description:
//    Effectue une mise a jour en temps du milieu,
//    et donc de ses parametres caracteristiques.
//    Les champs uniformes sont recalcules pour le
//    nouveau temps specifie, les autres sont mis a
//    par un appel a CLASSE_DU_CHAMP::mettre_a_jour(double temps).
// Precondition:
// Parametre: double temps
//    Signification: le temps de mise a jour
//    Valeurs par defaut:
//    Contraintes:
//    Acces:
// Retour:
//    Signification:
//    Contraintes:
// Exception:
// Effets de bord:
// Postcondition:
void Fluide_Incompressible::mettre_a_jour(double temps)
{
  Milieu_base::mettre_a_jour(temps);
  if (beta_co.non_nul())
    beta_co.mettre_a_jour(temps);
  mu.mettre_a_jour(temps);
  calculer_nu();
  nu.valeur().changer_temps(temps);
  nu->valeurs().echange_espace_virtuel();

  // Mise a jour des proprietes radiatives du fluide incompressible
  // (Pour un fluide incompressible semi transparent).

  if (coeff_absorption_.non_nul() && indice_refraction_.non_nul())
    {
      coeff_absorption_.mettre_a_jour(temps);
      indice_refraction_.mettre_a_jour(temps);

      // Mise a jour de longueur_rayo
      longueur_rayo_.mettre_a_jour(temps);

      if (sub_type(Champ_Uniforme,kappa().valeur()))
        {
          longueur_rayo()->valeurs()(0,0)=1/(3*kappa()(0,0));
          longueur_rayo()->valeurs().echange_espace_virtuel();
        }
      else
        {
          DoubleTab& l_rayo = longueur_rayo_.valeurs();
          const DoubleTab& K = kappa().valeurs();
          for (int i=0; i<kappa().nb_valeurs_nodales(); i++)
            l_rayo[i] = 1/(3*K[i]);
          l_rayo.echange_espace_virtuel();
        }
    }
}


// Description:
//    Initialise les parametres du fluide.
// Precondition:
// Parametre:
//    Signification:
//    Valeurs par defaut:
//    Contraintes:
//    Acces:
// Retour:
//    Signification:
//    Contraintes:
// Exception:
// Effets de bord:
// Postcondition: les parametres du fluide sont initialises
int Fluide_Incompressible::initialiser(const double& temps)
{
  Cerr << "Fluide_Incompressible::initialiser()" << finl;
  Milieu_base::initialiser(temps);
  mu.initialiser(temps);

  if (beta_co.non_nul())
    beta_co.initialiser(temps);
  calculer_nu();
  nu->valeurs().echange_espace_virtuel();
  nu->changer_temps(temps);

  // Initialisation des proprietes radiatives du fluide incompressible
  // (Pour un fluide incompressible semi transparent).

  if (coeff_absorption_.non_nul() && indice_refraction_.non_nul())
    {
      Cerr<<"Semi transparent fluid properties initialization."<<finl;
      coeff_absorption_.initialiser(temps);
      indice_refraction_.initialiser(temps);

      // Initialisation de longueur_rayo
      longueur_rayo_.initialiser(temps);
      if (sub_type(Champ_Uniforme,kappa().valeur()))
        {
          longueur_rayo()->valeurs()(0,0)=1/(3*kappa()(0,0));
        }
      else
        {
          DoubleTab& l_rayo = longueur_rayo_.valeurs();
          const DoubleTab& K = kappa().valeurs();
          for (int i=0; i<kappa().nb_valeurs_nodales(); i++)
            l_rayo[i] = 1/(3*K[i]);
        }
    }
  return 1;
}

int Fluide_Incompressible::is_rayo_semi_transp() const
{
  if (indic_rayo_==SEMITRANSP)
    {
      return 1;
    }
  else
    {
      return 0;
    }
}

int Fluide_Incompressible::is_rayo_transp() const
{
  if (indic_rayo_==TRANSP)
    {
      return 1;
    }
  else
    {
      return 0;
    }
}

void Fluide_Incompressible::reset_type_rayo()
{
  indic_rayo_=NONRAYO;
}
void Fluide_Incompressible::fixer_type_rayo()
{
  if ((coeff_absorption_.non_nul()) && (indice_refraction_.non_nul()))
    indic_rayo_ = SEMITRANSP;
  else
    indic_rayo_ = TRANSP;

}

int Fluide_Incompressible::longueur_rayo_is_discretised()
{
  return longueur_rayo_.non_nul();
}
