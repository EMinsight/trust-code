/****************************************************************************
* Copyright (c) 2015 - 2016, CEA
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
// File:        Entree_fluide_T_h_imposee.cpp
// Directory:   $TRUST_ROOT/src/ThHyd
// Version:     /main/17
//
//////////////////////////////////////////////////////////////////////////////

#include <Entree_fluide_T_h_imposee.h>
#include <Motcle.h>
#include <Equation_base.h>

Implemente_instanciable(Entree_fluide_T_h_imposee,"Frontiere_ouverte_T_h_imposee",Dirichlet_entree_fluide);


// Description:
//    Ecrit le type de l'objet sur un flot de sortie.
// Precondition:
// Parametre: Sortie& s
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
Sortie& Entree_fluide_T_h_imposee::printOn(Sortie& s ) const
{
  return s << que_suis_je() << "\n";
}

// Description:
//    Type le_champ_front en "Champ_front_uniforme".
//    Lit les valeurs du champ exterieur si les conditions
//    aux limites sont specifiees: "T_ext", "C_ext" ou "K_Eps_ext"
//    Produit une erreur sinon.
// Precondition:
// Parametre: Entree& s
//    Signification: un flot d'entree
//    Valeurs par defaut:
//    Contraintes:
//    Acces: entree/sortie
// Retour: Entree& s
//    Signification: le flot d'entree modifie
//    Contraintes:
// Exception: type de champ exterieur non reconnu,
//            les types reconnus sont: "T_ext", "C_ext" ou "K_Eps_ext"
// Effets de bord:
// Postcondition:
Entree& Entree_fluide_T_h_imposee::readOn(Entree& s )
{
  le_champ_front.typer("Champ_front_uniforme");
  le_champ_Text.typer("Champ_front_uniforme");
  le_champ_hext.typer("Champ_front_uniforme");
  Motcle motlu;
  Motcles les_motcles(4);
  {
    les_motcles[0] = "T_ext";
    les_motcles[1] = "C_ext";
    les_motcles[2] = "K_Eps_ext";
    les_motcles[3] = "H_ext";
  }

  Motcle accfermee="}";
  Motcle accouverte="{";
  s >> motlu;
  int rang;
  if (motlu != accouverte)
    {
      Cerr << "On attendait une { dans la lecture de l'interface " << finl;
      exit();
    }
  s >> motlu;
  while(motlu != accfermee)
    {
      rang = les_motcles.search(motlu);
      switch(rang)
        {
        case 0:
          {
            s >> le_champ_Text;
            break;
          }
        case 1:
        case 2:
        case 3:
          {
            s >> le_champ_hext;
            break;
          }
        default:
          {
            Cerr << "Erreur a la lecture de la condition aux limites de type: " << finl;
            Cerr << que_suis_je() << finl;
            Cerr << "On attendait " << les_motcles << " a la place de " <<  motlu << finl;
            exit();
          }
        }
      s >> motlu;
    }

  return s;
}

// Description:
//    Renvoie un booleen indiquant la compatibilite des conditions
//    aux limites avec l'equation specifiee en parametre.
//    Des CL de type Entree_fluide_temperature_imposee sont compatibles
//    avec une equation dont le domaine est la Thermique
//    ou bien indetermine.
// Precondition:
// Parametre: Equation_base& eqn
//    Signification: l'equation avec laquelle il faut verifier la compatibilite
//    Valeurs par defaut:
//    Contraintes: reference constante
//    Acces: entree
// Retour: int
//    Signification: valeur booleenne,
//                   1 si les CL sont compatibles avec l'equation
//                   0 sinon
//    Contraintes:
// Exception:
// Effets de bord:
// Postcondition: la methode ne modifie pas l'objet
int Entree_fluide_T_h_imposee::compatible_avec_eqn(const Equation_base& eqn) const
{
  Motcle dom_app=eqn.domaine_application();
  Motcle Thermique="Thermique";
  Motcle indetermine="indetermine";
  Cerr<<"Entree_fluide_T_h_imposee::compatible_avec_eqn"<<finl;
  if ( (dom_app==Thermique) || (dom_app==indetermine) )
    return 1;
  else
    {
      err_pas_compatible(eqn);
      return 0;
    }
}

// Description:
//    Renvoie la valeur de la i-eme composante
//    du champ impose a l'exterieur de la frontiere.
// Precondition:
// Parametre: int i
//    Signification: indice suivant la premiere dimension du champ
//    Valeurs par defaut:
//    Contraintes:
//    Acces: entree
// Retour: double
//    Signification: la valeur imposee sur la composante du champ specifiee
//    Contraintes:
// Exception: deuxieme dimension du champ de frontiere superieur a 1
// Effets de bord:
// Postcondition: la methode ne modifie pas l'objet
double Entree_fluide_T_h_imposee::val_imp(int i) const
{
  if (type_cond_lim == 0)
    {
      //      Cerr<<"Entree_fluide_T_h_imposee Condition limite en temperature"<<finl;
      if (le_champ_Text.valeurs().size()==1)
        return le_champ_Text(0,0);
      else if (le_champ_Text.valeurs().dimension(1)==1)
        return le_champ_Text(i,0);
      else
        Cerr << "Entree_fluide_T_h_imposee::val_ext" << finl;
      exit();
      return 0.;
    }
  else
    {
      //      Cerr<<"Entree_fluide_T_h_imposee Condition limite en enthalpie"<<finl;
      if (le_champ_hext.valeurs().size()==1)
        return le_champ_hext(0,0);
      else if (le_champ_hext.valeurs().dimension(1)==1)
        return le_champ_hext(i,0);
      else
        Cerr << "Entree_fluide_T_h_imposee::val_ext" << finl;
      exit();
      return 0.;
    }
}

// Description:
//    Renvoie la valeur de la (i,j)-eme composante
//    du champ impose a l'exterieur de la frontiere.
// Precondition:
// Parametre: int i
//    Signification: indice suivant la premiere dimension du champ
//    Valeurs par defaut:
//    Contraintes:
//    Acces: entree
// Parametre: int j
//    Signification: indice suivant la deuxieme dimension du champ
//    Valeurs par defaut:
//    Contraintes:
//    Acces: entree
// Retour: double
//    Signification: la valeur imposee sur la composante du champ specifiee
//    Contraintes:
// Exception:
// Effets de bord:
// Postcondition: la methode ne modifie pas l'objet
double Entree_fluide_T_h_imposee::val_imp(int i,int j) const
{
  if (type_cond_lim == 0)
    {
      // Condition limite en temperature
      //     Cerr<<"Entree_fluide_T_h_imposee Condition limite en temperature2 "<<finl;
      if (le_champ_Text.valeurs().dimension(0)==1)
        return le_champ_Text(0,j);
      else
        return le_champ_Text(i,j);
    }
  else
    {
      // Condition limite en enthalpie
      //      Cerr<<"Entree_fluide_T_h_imposee Condition limite en enthalpie2 "<<finl;
      if (le_champ_hext.valeurs().dimension(0)==1)
        return le_champ_hext(0,j);
      else
        return le_champ_hext(i,j);
    }
}
