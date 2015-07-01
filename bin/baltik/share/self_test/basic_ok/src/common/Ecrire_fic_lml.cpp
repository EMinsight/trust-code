////////////////////////////////////////////////////////////
//
// File:	Ecrire_fic_lml.cpp
// Directory:	$TRUST_ROOT/Geometrie/Decoupeur
//
////////////////////////////////////////////////////////////

#include <Ecrire_fic_lml.h>
#include <Domaine.h>
#include <EcrFicPartage.h>
#include <Format_Post_Lml.h>
Implemente_instanciable(Ecrire_fic_lml,"Ecrire_fic_lml",Interprete);

  // Description: 
  //    Simple appel a: Interprete::printOn(Sortie&)
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
  Sortie& Ecrire_fic_lml::printOn(Sortie& os) const
{
  return Interprete::printOn(os);
}

// Description: 
//    Simple appel a: Interprete::readOn(Entree&)
// Precondition: 
// Parametre: Entree& is
//    Signification: un flot d'entree
//    Valeurs par defaut: 
//    Contraintes: 
//    Acces: entree/sortie
// Retour: Entree& 
//    Signification: le flot d'entree modifie
//    Contraintes: 
// Exception: 
// Effets de bord: 
// Postcondition: 
Entree& Ecrire_fic_lml::readOn(Entree& is)
{
  return Interprete::readOn(is);
}

// Description:
//    Fonction principale de l'interprete.
// Precondition: 
// Parametre: Entree& is
//    Signification: un flot d'entree
//    Valeurs par defaut: 
//    Contraintes: 
//    Acces: entree/sortie
// Retour: Entree& 
//    Signification: le flot d'entree
//    Contraintes: 
// Exception: 
// Effets de bord: 
// Postcondition: 
Entree& Ecrire_fic_lml::interpreter(Entree& is)
{ 
  Nom nom_dom,nom_fic;
  is >> nom_dom ;
   
  if(! sub_type(Domaine, objet(nom_dom)))
    {
      Cerr << nom_dom << " est du type " << objet(nom_dom).que_suis_je() << finl;
      Cerr << "On attendait un objet de type Domaine" << finl;
      exit();
    }
   
  Domaine& dom=ref_cast(Domaine, objet(nom_dom));
  nom_fic=nom_du_cas();
 // nom_fic+=".lml";
  Format_Post_Lml post;
  post.initialize_by_default(nom_fic);
  int est_le_premie_post=1;
  post.ecrire_entete(0.,0, est_le_premie_post);
  post.ecrire_domaine(dom, est_le_premie_post);
  post.finir(est_le_premie_post);
/*
  EcrFicPartage fic(nom_fic);
  dom.postraiter_lml(fic);  
  fic.syncfile();
  if (je_suis_maitre()) 
    fic<<"FIN"<<finl;
  fic.syncfile();
  fic.close();
*/
  return is;     
}


