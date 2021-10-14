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
// File:        Loi_Etat_GP_WC.cpp
// Directory:   $TRUST_ROOT/src/ThHyd/Fluide_Dilatable/Weakly_Compressible/Loi_Etat
// Version:     /main/25
//
//////////////////////////////////////////////////////////////////////////////

#include <Loi_Etat_GP_WC.h>
#include <Motcle.h>
#include <Fluide_Dilatable_base.h>
#include <Fluide_Weakly_Compressible.h>
#include <Champ_Uniforme.h>
#include <Zone_VF.h>
#include <Champ_Fonc_Tabule.h>

Implemente_instanciable(Loi_Etat_GP_WC,"Loi_Etat_Gaz_Parfait_WC",Loi_Etat_Mono_GP_base);

// Description:
//    Imprime la loi sur un flot de sortie.
// Precondition:
// Parametre: Sortie& os
//    Signification: le flot de sortie pour l'impression
//    Valeurs par defaut:
//    Contraintes:
//    Acces: sortie
// Retour: Sortie&
//    Signification: le flot de sortie modifie
//    Contraintes:
// Exception:
// Effets de bord: le flot de sortie est modifie
// Postcondition: la methode ne modifie pas l'objet
Sortie& Loi_Etat_GP_WC::printOn(Sortie& os) const
{
  os <<que_suis_je()<< finl;
  return os;
}

// Description:
//    Lecture d'une loi sur un flot d'entree.
// Precondition:
// Parametre: Entree& is
//    Signification: le flot d'entree pour la lecture des parametres
//    Valeurs par defaut:
//    Contraintes:
//    Acces: entree/sortie
// Retour: Entree&
//    Signification: le flot d'entree modifie
//    Contraintes:
// Exception: accolade ouvrante attendue
// Effets de bord:
// Postcondition: l'objet est construit avec les parametres lus
Entree& Loi_Etat_GP_WC::readOn(Entree& is)
{
  double Cv_ = -1;
  double gamma_ = -1;

  Motcle accferme="}";
  Motcle accouverte="{";

  Motcle motlu;
  is >> motlu;
  Cerr<<"Lecture de la loi d'etat Gaz Parfait"<<finl;
  if (motlu != accouverte)
    {
      Cerr<<" On attendait "<<accouverte<<" au lieu de "<<motlu<<finl;
      abort();
    }
  Motcles les_mots(6);
  {
    les_mots[0] = "Cp";
    les_mots[1] = "capacite_calorifique_pression_constante";
    les_mots[2] = "Cv";
    les_mots[3] = "capacite_calorifique_volume_constant";
    les_mots[4] = "gamma";
    les_mots[5] = "Prandtl";
  }
  is >> motlu;
  while(motlu != accferme )
    {
      int rang=les_mots.search(motlu);
      switch(rang)
        {
        case 0 :
        case 1 :
          {
            is>>Cp_;
            break;
          }
        case 2 :
        case 3 :
          {
            is>>Cv_;
            break;
          }
        case 4 :
          {
            is>>gamma_;
            if (gamma_<0)
              {
                gamma_ = -gamma_;
                debug=1;
              }
            else
              debug=0;
            break;
          }
        case 5 :
          {
            is>>Pr_;
            break;
          }
        default :
          {
            Cerr<<"Une loi d'etat "<<que_suis_je()<<" n'a pas la propriete "<<motlu<<finl;
            Cerr<<"On attendait un mot dans :"<<finl<<les_mots<<finl;
            abort();
          }
        }
      is >> motlu;
    }

  if (Pr_==-1)
    {
      Cerr<<"ERREUR : on attendait la definition du nombre de Prandtl (constante)"<<finl;
      abort();
    }
  if (Cp_==-1)
    {
      Cerr<<"ERREUR : on attendait la definition du Cp (constante en gaz parfaits)"<<finl;
      abort();
    }
  if (Cv_!=-1)
    {
      R_ = Cp_ - Cv_;
    }
  else if (gamma_!=-1)
    {
      R_ = Cp_ *(1.-1./gamma_);
    }
  else
    {
      Cerr<<"ERREUR : on attendait la definition du Cv (constante en gaz parfaits)"<<finl;
      Cerr<<"ou de la constante gamma (constante en gaz parfaits)"<<finl;
      abort();
    }
  return is;
}


// Description:
//    Recalcule la masse volumique
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
double Loi_Etat_GP_WC::calculer_masse_volumique(double P, double T) const
{
  return Loi_Etat_Mono_GP_base::calculer_masse_volumique(P,T);
}

// Description:
//    Calcule la pression avec la temperature et la masse volumique
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
double Loi_Etat_GP_WC::inverser_Pth(double T, double rho)
{
  Cerr << "Should not enter in the method Loi_Etat_GP_WC::inverser_Pth !" << finl;
  Process::exit();
  return -1.e10;
}

// Description:
//    Calcule la masse volumique
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
void Loi_Etat_GP_WC::calculer_masse_volumique()
{
  const DoubleTab& tab_ICh = le_fluide->inco_chaleur().valeurs();
  Fluide_Weakly_Compressible& FWC = ref_cast(Fluide_Weakly_Compressible,le_fluide.valeur());
  DoubleTab& tab_rho = le_fluide->masse_volumique().valeurs();
  const DoubleTab& pres = FWC.pression_th_tab();
  const int n=tab_rho.size();

  for (int som=0 ; som<n ; som++)
    {
      tab_rho_np1(som) = calculer_masse_volumique(pres(som,0),tab_ICh(som,0));
      tab_rho(som,0) = 0.5 * ( tab_rho_n(som) + tab_rho_np1(som) );
    }
  tab_rho.echange_espace_virtuel();
  tab_rho_np1.echange_espace_virtuel();
  le_fluide->calculer_rho_face(tab_rho_np1);
}
