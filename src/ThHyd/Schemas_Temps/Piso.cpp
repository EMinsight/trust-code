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

#include <Piso.h>
#include <Source_PDF_base.h>

#include <EChaine.h>
#include <Debog.h>
#include <Matrice_Bloc.h>
#include <Assembleur_base.h>

#include <Schema_Temps_base.h>
#include <TRUSTTrav.h>
#include <Fluide_Dilatable_base.h>

#include <Probleme_base.h>

#include <MD_Vector_std.h>
#include <MD_Vector_composite.h>
#include <MD_Vector_tools.h>
#include <TRUSTTab_parts.h>
#include <Discretisation_base.h>

Implemente_instanciable_sans_constructeur(Piso,"Piso",Simpler);

Implemente_instanciable_sans_constructeur(Implicite,"Implicite",Piso);

Piso::Piso()
{
  nb_corrections_max_ = 21;
  avancement_crank_ = 0;
}

Implicite::Implicite()
{
  avancement_crank_ = 1;
}

Sortie& Piso::printOn(Sortie& os ) const
{
  return Simpler::printOn(os);
}

Entree& Piso::readOn(Entree& is )
{
  Simpler::readOn(is);
  return is;
}

Sortie& Implicite::printOn(Sortie& os ) const
{
  return Piso::printOn(os);
}

Entree& Implicite::readOn(Entree& is )
{
  Piso::readOn(is);
  return is;
}


Entree& Piso::lire(const Motcle& motlu,Entree& is)
{
  Motcles les_mots(1);
  {
    les_mots[0] = "nb_corrections_max";
  }

  int rang = les_mots.search(motlu);
  switch(rang)
    {
    case 0:
      {
        is >> nb_corrections_max_;
        if (nb_corrections_max_ < 2)
          {
            Cerr<<"There must be at least two corrections steps for the PISO algorithm."<<finl;
            exit();
          }
        break;
      }

    default :
      {
        Cerr << "Keyword : " << motlu << " is not understood in " << que_suis_je() << finl;
        exit();
      }
    }
  return is;
}



void test_imposer_cond_lim(Equation_base& eqn,DoubleTab& current2,const char * msg,int flag)
{
  return;
  /*
  DoubleTab& present = eqn.inconnue().futur();
  DoubleTab sauv(present);
  const Schema_Temps_base& sch = eqn.probleme().schema_temps();
  eqn.zone_Cl_dis()->imposer_cond_lim(eqn.inconnue(),sch.temps_courant()+sch.pas_de_temps());
  present -= sauv;
  // BM, je remplace max_abs par mp_pax_abs: du coup la methode doit etre appelee simultanement par tous les procs.
  double ecart_max=mp_max_abs_vect(present);
  Cout<<msg <<" "<<ecart_max<<finl;

  if ((ecart_max>1e-10))
    abort();
  present = sauv;
   */
}

//Entree Un ; Pn
//Sortie Un+1 = U***_k ; Pn+1 = P**_k
//n designe une etape temporelle

void Piso::iterer_NS(Equation_base& eqn,DoubleTab& current,DoubleTab& pression,
                     double dt,Matrice_Morse& matrice,double seuil_resol,DoubleTrav& secmem,int nb_ite,int& converge, int& ok)
{
  converge = 1;
  if (nb_ite>1) return;
  Navier_Stokes_std& eqnNS = ref_cast(Navier_Stokes_std,eqn);
  Parametre_implicite& param_eqn = get_and_set_parametre_implicite(eqn);
  SolveurSys& le_solveur_ = param_eqn.solveur();

  DoubleTrav gradP(current);
  DoubleTrav correction_en_pression(pression);
  DoubleTrav resu(current);
  int is_dilat = eqn.probleme().is_dilatable();

  double vitesse_norme,vitesse_norme_old ;
  double pression_norme,pression_norme_old ;
  vitesse_norme_old = 1.e10 ;
  pression_norme_old = 1.e10 ;

  // int deux_entrees = 0;
  //if (current.nb_dim()==2) deux_entrees = 1;
  Operateur_Grad& gradient = eqnNS.operateur_gradient();
  Operateur_Div& divergence = eqnNS.operateur_divergence();

  //int nb_comp = 1;
  //int nb_dim = current.nb_dim();

  //  if(nb_dim==2)    nb_comp = current.dimension(1);

  //Construction de matrice et resu
  //matrice = A[Un] = M/delta_t + CONV +DIFF
  //resu =  A[Un]Un -(A[Un]Un-Ss) + Sv -BtPn

  // <IBM> Taking into account penality term for Immersed Boundary Method
  const int i_source_PDF = eqnNS.get_i_source_pdf();
  if (i_source_PDF != -1)
    {
      Source_PDF_base& src = dynamic_cast<Source_PDF_base&>((eqnNS.sources())[i_source_PDF].valeur());
      Cerr<<"Immersed Interface: Dirichlet velocity in momentum equation for PDF (if any)."<<finl;
      // Terme PDF IB value : -rho/delta_t  ksi_gamma/epsilon U_gamma
      DoubleTab secmem_pdf(resu);
      src.calculer_pdf(secmem_pdf);
      resu -= secmem_pdf;
      resu.echange_espace_virtuel();

      // Terme en temps : -rho/delta_t ksi_gamma Un
      int i_traitement_special = 101;
      if (eqnNS.nombre_d_operateurs() > 1)
        {
          if (eqnNS.vitesse_pour_transport().le_nom()=="rho_u") i_traitement_special = 1;
        }
      DoubleTrav secmem_pdf_time(resu);
      src.calculer(secmem_pdf_time, i_traitement_special);
      secmem_pdf += secmem_pdf_time;

      // Sauvegarde de secmem_pdf
      secmem_pdf.echange_espace_virtuel();
      src.set_sec_mem_pdf(secmem_pdf);

      DoubleTab coeff;
      coeff = eqnNS.get_champ_coeff_pdf_som();
      if (eqnNS.get_gradient_pression_qdm_modifie()==1)
        {
          Cerr<<"(IBM) Immersed Interface: modified pressure gradient in momentum equation."<<finl;
          gradP/=coeff;
        }
      gradP.echange_espace_virtuel();
    }
  // </IBM>

  gradient.valeur().calculer(pression,gradP);
  if (eqnNS.has_interface_blocs()) //si l'interface blocs est disponible, on l'utilise
    {
      eqnNS.assembler_blocs_avec_inertie({{ "vitesse", &matrice }}, resu);
      if (eqnNS.discretisation().que_suis_je().debute_par("PolyMAC"))
        matrice.ajouter_multvect(current, resu);  //pour ne pas etre en increment
    }
  else //sinon, on passe par ajouter/contribuer
    {
      resu -= gradP;
      first_special_treatment( eqn, eqnNS, current, dt, resu );
      eqnNS.assembler_avec_inertie(matrice,current,resu);
    }

  le_solveur_.valeur().reinit();

  //sometimes we need a second special treatement like for ALE for example
  second_special_treatment( eqn, current, resu, matrice );

  //Construction de matrice_en_pression_2 = BD-1Bt[Un]
  //Assemblage reeffectue seulement pour algorithme Piso (avancement_crank_==0)
  Matrice& matrice_en_pression_2 = eqnNS.matrice_pression();
  SolveurSys& solveur_pression_ = eqnNS.solveur_pression();

  if (avancement_crank_==0)
    {
      assembler_matrice_pression_implicite(eqnNS,matrice,matrice_en_pression_2);
      solveur_pression_.valeur().reinit();
    }

  //Etape predicteur
  //Resolution du systeme A[Un]U* = -BtPn + Sv + Ss
  //current = U*
  le_solveur_.resoudre_systeme(matrice,resu,current);

  test_imposer_cond_lim(eqn,current,"apres resolution ",0);
  current.echange_espace_virtuel();
  Debog::verifier("Piso::iterer_NS current apres solveur",current);

  //Calcul de secmem = BU* (en incompressible) BU* -drho/dt (en quasi-compressible)
  if (is_dilat)
    {
      if (with_d_rho_dt_)
        {
          Fluide_Dilatable_base& fluide_dil = ref_cast(Fluide_Dilatable_base,eqn.milieu());
          fluide_dil.secmembre_divU_Z(secmem);
          secmem *= -1;
        }
      else   secmem = 0;
      divergence.ajouter(current,secmem);
    }
  else
    divergence.calculer(current,secmem);
  secmem *= -1;
  // <IBM> Taking into account zero velocity divergence for Immersed Boundary Method
  if ((i_source_PDF != -1) && (eqnNS.get_correction_matrice_pression()==1))
    {
      Cerr<<"(IBM) Immersed Interface: velocity divergence is zero for crossed elements."<<finl;
      DoubleTrav coeff;
      coeff = eqnNS.get_champ_coeff_pdf_som();
      Source_PDF_base& src = dynamic_cast<Source_PDF_base&>((eqnNS.sources())[i_source_PDF].valeur());
      src.correct_incr_pressure(coeff, secmem);
    }
  // <IBM>
  secmem.echange_espace_virtuel();
  Debog::verifier("Piso::iterer_NS secmem",secmem);

  // GF il ne faut pas modifier le scd membre le terme en du/dt au bord a deja ete pris en compte dans la resolution precedente
  //  eqnNS.assembleur_pression().valeur().modifier_secmem(secmem);

  //Etape de correction 1
  Cout << "Solving mass equation :" << finl;
  //Description du cas implicite
  //Resolution du systeme (BD-1Bt)P' = Bu* (D-1 = M-1 pour le cas implicite)
  //correction_en_pression = P' pour Piso et correction_en_pression = delta_t*P' pour implicite
  eqnNS.assembleur_pression()->modifier_secmem_pour_incr_p(pression, 1. / dt, secmem);
  solveur_pression_.resoudre_systeme(matrice_en_pression_2.valeur(),
                                     secmem,correction_en_pression);
  correction_en_pression.echange_espace_virtuel();
  Debog::verifier("Piso::iterer_NS apres correction_pression",correction_en_pression);

  if (avancement_crank_==1)
    {
      //calcul de Un+1
      //Calcul de Bt(delta_t*delta_P)
      gradient.valeur().multvect(correction_en_pression,gradP);
      eqn.solv_masse().appliquer(gradP);
      if ((i_source_PDF != -1) && (eqnNS.get_correction_vitesse_modifie()==1))
        {
          Cerr<<"(IBM) Immersed Interface: modified velocity correction."<<finl;
          DoubleTab coeff;
          coeff = eqnNS.get_champ_coeff_pdf_som();
          gradP/=coeff;
          gradP.echange_espace_virtuel();
        }

      //Calcul de Un+1 = U* -delta_t*delta_P
      current -= gradP;
      eqn.solv_masse().corriger_solution(current, current); //pour PolyMAC_P0 : sert a corriger ve
      current.echange_espace_virtuel();
      divergence.calculer(current,secmem);

      //Calcul de Pn+1 = Pn + (delta_t*delta_P)/delat_t
      Debog::verifier("Piso::iterer_NS correction avant dt",correction_en_pression);
      correction_en_pression /= dt;

      // <IBM> Immersed Interface: modified pressure correction.
      if ((i_source_PDF != -1) && (eqnNS.get_correction_pression_modifie()==1))
        {
          Cerr<<"Immersed Interface: modified pressure correction."<<finl;
          DoubleTab coeff;
          coeff = eqnNS.get_champ_coeff_pdf_som();
          const Source_PDF_base& src = dynamic_cast<Source_PDF_base&>((eqnNS.sources())[i_source_PDF].valeur());
          src.correct_pressure(coeff,pression,correction_en_pression);
        }
      else
        {
          pression += correction_en_pression;
        }
      // </IBM>
      eqnNS.assembleur_pression().valeur().modifier_solution(pression);
      pression.echange_espace_virtuel();
      Debog::verifier("Piso::iterer_NS pression finale",pression);
      Debog::verifier("Piso::iterer_NS current final",current);
      if (is_dilat)
        {
          // on redivise par rho_np_1 avant de sortir
          diviser_par_rho_np1_face(eqn,current);
        }
      return;
    }

  // calcul de la correction en vitesse premiere etape (DU' =-Bt P)

  //Calcul de P* = Pn + P'
  pression += correction_en_pression;
  eqnNS.assembleur_pression().valeur().modifier_solution(pression);
  //Resolution du systeme D[Un]U' = -BtP'
  DoubleTrav correction_en_vitesse(current);
  calculer_correction_en_vitesse(correction_en_pression,gradP,correction_en_vitesse,matrice,gradient);

  //Calcul de U** = U* + U'
  current += correction_en_vitesse;
  test_imposer_cond_lim(eqn,current,"apres premiere correction ",0);
  Debog::verifier("Piso::iterer_NS arpes cor pression",pression);
  Debog::verifier("Piso::iterer_NS arpes cor vitesse",current);

  //Etape correcteur 2
  for (int compt=0; compt<nb_corrections_max_-1; compt++)
    {
      correction_en_vitesse.echange_espace_virtuel();
      //Resolution du systeme D resu = EU' + (resu2=0) pour stocker resu = D-1EU'
      DoubleTrav resu2(resu);
      int status = inverser_par_diagonale(matrice,resu2,correction_en_vitesse,resu);

      if (status!=0) exit();
      // calcul de P''  BD-1Bt P''= -div(D-1EU')

      resu.echange_espace_virtuel();
      //Calcul de B(D-1EU')
      divergence.calculer(resu,secmem);
      secmem *= -1;
      secmem.echange_espace_virtuel();

      //Resolution du systeme (BD-1Bt)P'' = (BD-1E)U'
      //correction_en_pression = P''
      correction_en_pression = 0;
      solveur_pression_.resoudre_systeme(matrice_en_pression_2.valeur(),
                                         secmem,correction_en_pression);

#ifdef DEBUG
      // Pour verifier que le calcul du gradient ne modifie pas la pression
      DoubleTrav correction_en_pression_mod(pression);
      correction_en_pression_mod = correction_en_pression;
#endif
      //Resolution du systeme D[Un]U'' = -BtP''
      //correction_en_vitesse = U''
      calculer_correction_en_vitesse(correction_en_pression,gradP,correction_en_vitesse,matrice,gradient);

#ifdef DEBUG
      correction_en_pression_mod -= correction_en_pression;
      assert(max_abs(correction_en_pression_mod)==0);
#endif

      //Calcul de U'' = U'' + D-1EU'
      correction_en_vitesse += resu;
      // ajout des increments

      vitesse_norme = mp_norme_vect(correction_en_vitesse);
      pression_norme = mp_norme_vect(correction_en_pression);

      if ( (vitesse_norme>vitesse_norme_old) || (pression_norme>pression_norme_old) )
        {
          Cout <<"PISO : "<< compt+1 <<" corrections to perform the projection."<< finl;
          if (is_dilat)
            {
              // on redivise par rho_np_1 avant de sortir
              diviser_par_rho_np1_face(eqn,current);
            }
          return ;
        }

      vitesse_norme_old = vitesse_norme;
      pression_norme_old = pression_norme;
      //Calcul de P** = P* + P''
      pression += correction_en_pression;
      eqnNS.assembleur_pression().valeur().modifier_solution(pression);

      //Calcul de U*** = U** + U''
      current += correction_en_vitesse;
      test_imposer_cond_lim(eqn,current,"apres correction (int)__LINE__",0);

      Debog::verifier("Piso::iterer_NS apres correction pression",pression);
      Debog::verifier("Piso::iterer_NS apres correction vitesse",current);
    }
  if (is_dilat)
    {
      diviser_par_rho_np1_face(eqn,current);
      //ref_cast(Navier_Stokes_QC,eqn).impr_impl(eqnNS, Cout);
    }
  current.echange_espace_virtuel();
  // divergence.calculer(current, secmem); Cerr<<" ici DIVU  "<<mp_max_abs_vect(secmem)<<finl;;
  Cout <<"PISO : "<<nb_corrections_max_<<" corrections to perform the projection."<< finl;
}

void Piso::first_special_treatment(Equation_base& eqn, Navier_Stokes_std& eqnNS, DoubleTab& current, double dt, DoubleTrav& resu)
{
  //nothing to do
}

void Piso::second_special_treatment(Equation_base& eqn,DoubleTab& current, DoubleTrav& resu, Matrice_Morse& matrice)
{
  //nothing to do
}

void Implicite::first_special_treatment(Equation_base& eqn, Navier_Stokes_std& eqnNS, DoubleTab& current, double dt, DoubleTrav& resu)
{
  //nothing to do
}

void Implicite::second_special_treatment(Equation_base& eqn,DoubleTab& current, DoubleTrav& resu, Matrice_Morse& matrice)
{
  //nothing to do
}
