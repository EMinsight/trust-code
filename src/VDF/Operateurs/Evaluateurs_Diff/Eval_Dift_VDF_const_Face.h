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
// File:        Eval_Dift_VDF_const_Face.h
// Directory:   $TRUST_ROOT/src/VDF/Operateurs/Evaluateurs_Diff
// Version:     /main/19
//
//////////////////////////////////////////////////////////////////////////////


#ifndef Eval_Dift_VDF_const_Face_included
#define Eval_Dift_VDF_const_Face_included

#include <Eval_Dift_VDF_const2.h>
#include <Eval_Diff_VDF_Face.h>
#include <Ref_Turbulence_paroi_base.h>
#include <Mod_turb_hyd_base.h>
//
// .DESCRIPTION class Eval_Dift_VDF_const_Face
//
// Evaluateur VDF pour la diffusion totale (laminaire et turbulente)
// Le champ diffuse est un Champ_Face
// Le champ de diffusivite est constant.

//
// .SECTION voir aussi Eval_Dift_VDF_const

class Eval_Dift_VDF_const_Face : public Eval_Diff_VDF_Face<Eval_Dift_VDF_const_Face>,
  public Eval_Dift_VDF_const2
{

public:
  inline void associer_modele_turbulence(const Mod_turb_hyd_base& mod)
  {
    le_modele_turbulence= mod;
  }

  inline void mettre_a_jour()
  {
    Eval_Dift_VDF_const2::mettre_a_jour( ) ;
    if (le_modele_turbulence->loi_paroi().non_nul())
      {
        tau_tan_.ref(le_modele_turbulence->loi_paroi()->Cisaillement_paroi());
      }
  }

  // overload methods (see implementations in Eval_Diff_VDF_const2)
  inline double tau_tan_impl(int face,int k) const;
  inline bool uses_wall() const
  {
    return le_modele_turbulence.valeur().utiliser_loi_paroi();
  }

protected:
  REF(Mod_turb_hyd_base) le_modele_turbulence;
  DoubleTab tau_tan_;
};

// overload method
inline double Eval_Dift_VDF_const_Face::tau_tan_impl(int face, int k) const
{
  int nb_faces = la_zone.valeur().nb_faces();
  const ArrOfInt& ind_faces_virt_bord = la_zone.valeur().ind_faces_virt_bord();
  int f;
  if(face>=tau_tan_.dimension(0))
    f = ind_faces_virt_bord[face-nb_faces];
  else
    f=face;
  if(f>=tau_tan_.dimension_tot(0))
    {
      Cerr << "Erreur dans tau_tan " << finl;
      Cerr << "dimension : " << tau_tan_.dimension(0) << finl;
      Cerr << "dimension_tot : " << tau_tan_.dimension_tot(0) << finl;
      Cerr << "face : " << face << finl;
      Process::exit();
    }
  return tau_tan_(f,k);
}

/*
 * Class template spectializations for the class Eval_Dift_VDF_const_Face
 * This is a first version : All are here
 * TODO : FIXME : all these should be factorized and put in Eval_Diff_VDF_Face
 * BUT , NO COHERENT CODING ... TODO TODO TODO
 *
 */

//// calculer_arete_paroi_fluide
//
template<>
inline int Eval_Diff_VDF_Face<Eval_Dift_VDF_const_Face>::calculer_arete_paroi_fluide() const
{
  return 0;
}

////   calculer_fa7_sortie_libre
//
template<>
inline int Eval_Diff_VDF_Face<Eval_Dift_VDF_const_Face>::calculer_fa7_sortie_libre() const
{
  return 1;
}

//// flux_arete_interne
//
template<>
inline double Eval_Diff_VDF_Face<Eval_Dift_VDF_const_Face>::flux_arete_interne(const DoubleTab& inco,
                                                                               int fac1,int fac2, int fac3, int fac4) const
{
  double flux;
  int ori1 = orientation(fac1);
  int ori3 = orientation(fac3);
  int elem1 = elem_(fac3,0);
  int elem2 = elem_(fac3,1);
  int elem3 = elem_(fac4,0);
  int elem4 = elem_(fac4,1);

  double visc_lam = nu_3(0);
  double visc_turb = nu_2(elem1,elem2,elem3,elem4);
  double tau = (inco[fac4]-inco[fac3])/dist_face(fac3,fac4,ori1);
  double tau_tr = (inco[fac2]-inco[fac1])/dist_face(fac1,fac2,ori3);
  double reyn = (tau + tau_tr)*visc_turb;
  flux = 0.25*(reyn + visc_lam*tau)*(surface(fac1)+surface(fac2))
         *(porosite(fac1)+porosite(fac2));
  return flux;
}

//// coeffs_arete_interne
//
template<>
inline void Eval_Diff_VDF_Face<Eval_Dift_VDF_const_Face>::coeffs_arete_interne(int fac1, int fac2,
                                                                               int fac3, int fac4,double& aii, double& ajj) const
{
  int ori1 = orientation(fac1);
  int ori3 = orientation(fac3);
  int elem1 = elem_(fac3,0);
  int elem2 = elem_(fac3,1);
  int elem3 = elem_(fac4,0);
  int elem4 = elem_(fac4,1);

  double visc_lam = nu_3(0);
  double visc_turb = nu_2(elem1,elem2,elem3,elem4);
  double tau = 1/dist_face(fac3,fac4,ori1);
  double tau_tr = 1/dist_face(fac1,fac2,ori3);
  double reyn = (tau + tau_tr)*visc_turb;

  aii = ajj = 0.25*(reyn + visc_lam*tau)*(surface(fac1)+surface(fac2))
              *(porosite(fac1)+porosite(fac2));
}

//// flux_arete_mixte
//

// Sur les aretes mixtes les termes croises du tenseur de Reynolds
// sont nuls: il ne reste donc que la diffusion laminaire
template<>
inline double Eval_Diff_VDF_Face<Eval_Dift_VDF_const_Face>::flux_arete_mixte(const DoubleTab& inco,
                                                                             int fac1,int fac2, int fac3, int fac4) const
{
  double flux=0;
  if (inco[fac4]*inco[fac3] != 0)
    {
      int ori=orientation(fac1);
      double tau = (inco[fac4]-inco[fac3])/dist_face(fac3,fac4,ori);
      flux = 0.25*tau*(surface(fac1)+surface(fac2))*
             nu_3(0)*(porosite(fac1)+porosite(fac2));
    }
  return flux;
}

//// coeffs_arete_mixte
//
template<>
inline void Eval_Diff_VDF_Face<Eval_Dift_VDF_const_Face>::coeffs_arete_mixte(int fac1, int fac2,
                                                                             int fac3, int fac4,double& aii, double& ajj) const
{
  if (inconnue->valeurs()[fac4]*inconnue->valeurs()[fac3] != 0)
    {
      int ori=orientation(fac1);
      aii = ajj= 0.25*(surface(fac1)+surface(fac2))*nu_3(0)*
                 (porosite(fac1)+porosite(fac2))/dist_face(fac3,fac4,ori);
    }
  else
    {
      aii=ajj=0;
    }
}

//// flux_arete_fluide
//
template<>
inline void Eval_Diff_VDF_Face<Eval_Dift_VDF_const_Face>::flux_arete_fluide(const DoubleTab& inco,
                                                                            int fac1,int fac2, int fac3, int signe,
                                                                            double& flux3, double& flux1_2) const
{
  int rang1 = (fac1-premiere_face_bord);
  int rang2 = (fac2-premiere_face_bord);
  int elem1 = elem_(fac3,0);
  int elem2 = elem_(fac3,1);
  double visc_lam = nu_3(0);
  double visc_turb = nu_1(elem1,elem2);
  int ori= orientation(fac3);
  double vit_imp = 0.5*(Champ_Face_get_val_imp_face_bord(inconnue->temps(),rang1,ori,la_zcl)+
                        Champ_Face_get_val_imp_face_bord(inconnue->temps(),rang2,ori,la_zcl));

  double dist = dist_norm_bord(fac1);
  double tau = signe * (vit_imp - inco[fac3])/dist;
  double tau_tr = (inco[fac2] - inco[fac1])/dist_face(fac1,fac2,ori);
  double surf = 0.5*(surface(fac1)+surface(fac2));
  double poros = 0.5*(porosite(fac1)+porosite(fac2));
  double reyn = (tau + tau_tr)*visc_turb;
  double coef = (tau*visc_lam + reyn);
  flux3 = coef*surf*poros;
  flux1_2 = (tau_tr*visc_lam + reyn)*surface(fac3)*porosite(fac3);
}

//// coeffs_arete_fluide
//
template<>
inline void Eval_Diff_VDF_Face<Eval_Dift_VDF_const_Face>::coeffs_arete_fluide(int fac1, int fac2,
                                                                              int fac3, int signe,double& aii1_2, double& aii3_4,double& ajj1_2) const
{
  int elem1 = elem_(fac3,0);
  int elem2 = elem_(fac3,1);
  double visc_lam = nu_3(0);
  double visc_turb = nu_1(elem1,elem2);
  int ori= orientation(fac3);
  double dist = dist_norm_bord(fac1);
  double tau = signe/dist;
  double tau_tr = 1/dist_face(fac1,fac2,ori);
  double surf = 0.5*(surface(fac1)+surface(fac2));
  double poros = 0.5*(porosite(fac1)+porosite(fac2));
  double reyn = (tau + tau_tr)*visc_turb;
  double coef = (tau*visc_lam + reyn);

  // Calcul de aii3_4
  aii3_4 = coef*surf*poros;

  // Calcul de aii1_2 et ajj1_2
  aii1_2 = ajj1_2  = (tau_tr*visc_lam + reyn)*surface(fac3)*porosite(fac3);
}

//// secmem_arete_fluide
//
template<>
inline void Eval_Diff_VDF_Face<Eval_Dift_VDF_const_Face>::secmem_arete_fluide(int fac1, int fac2,
                                                                              int fac3, int signe,double& flux3, double& flux1_2) const
{
  int rang1 = (fac1-premiere_face_bord);
  int rang2 = (fac2-premiere_face_bord);
  int elem1 = elem_(fac3,0);
  int elem2 = elem_(fac3,1);
  double visc_lam = nu_3(0);
  double visc_turb = nu_1(elem1,elem2);
  int ori= orientation(fac3);
  double vit_imp = 0.5*(Champ_Face_get_val_imp_face_bord(inconnue->temps(),rang1,ori,la_zcl)+
                        Champ_Face_get_val_imp_face_bord(inconnue->temps(),rang2,ori,la_zcl));

  double dist = dist_norm_bord(fac1);
  double tau = signe*vit_imp/dist;
  double surf = 0.5*(surface(fac1)+surface(fac2));
  double poros = 0.5*(porosite(fac1)+porosite(fac2));
  double reyn = tau*visc_turb;
  double coef = (tau*visc_lam + reyn);

  flux3 = coef*surf*poros;
  flux1_2 = 0;
}

//// flux_arete_paroi
//
template<>
inline double Eval_Diff_VDF_Face<Eval_Dift_VDF_const_Face>::flux_arete_paroi(const DoubleTab& inco,
                                                                             int fac1,int fac2, int fac3, int signe ) const
{
  double flux;
  int rang1 = (fac1-premiere_face_bord);
  int rang2 = (fac2-premiere_face_bord);
  int ori = orientation(fac3);
  double vit = inco(fac3);
  double vit_imp = 0.5*(Champ_Face_get_val_imp_face_bord(inconnue->temps(),rang1,ori,la_zcl)+
                        Champ_Face_get_val_imp_face_bord(inconnue->temps(),rang2,ori,la_zcl));
  if ( !uses_wall_law() )
    {
      double dist = dist_norm_bord(fac1);
      double tau  = signe*(vit_imp - inco[fac3])/dist;
      double surf = 0.5*(surface(fac1)+surface(fac2));
      flux = tau*surf*nu_3(0);
    }
  else
    {
      // On calcule u_star*u_star*surf sur chaque partie de la facette de Qdm
      int signe_terme;
      if ( vit < vit_imp )
        signe_terme = -1;
      else
        signe_terme = 1;

      //30/09/2003  YB : influence de signe terme eliminee, signe pris en compte dans la loi de paroi
      signe_terme = 1;

      double tau1 = tau_tan(rang1,ori)*0.5*surface(fac1);
      double tau2 = tau_tan(rang2,ori)*0.5*surface(fac2);
      double coef = tau1+tau2;
      flux = signe_terme*coef;
    }
  return flux;

}

//// coeffs_arete_paroi
//
template<>
inline void Eval_Diff_VDF_Face<Eval_Dift_VDF_const_Face>::coeffs_arete_paroi(int fac1, int fac2,
                                                                             int fac3, int signe, double& aii1_2, double& aii3_4, double& ajj1_2) const
{
  aii3_4 = 0;
  aii1_2 = 0;
  ajj1_2 = 0;
}

//// secmem_arete_paroi
//
template<>
inline double Eval_Diff_VDF_Face<Eval_Dift_VDF_const_Face>::secmem_arete_paroi(int fac1, int fac2,
                                                                               int fac3, int signe) const
{
  double flux;
  int rang1 = (fac1-premiere_face_bord);
  int rang2 = (fac2-premiere_face_bord);
  int ori = orientation(fac3);
  const DoubleTab& inco = inconnue->valeurs();
  double vit = inco(fac3);
  double vit_imp = 0.5*(Champ_Face_get_val_imp_face_bord(inconnue->temps(),rang1,ori,la_zcl)+
                        Champ_Face_get_val_imp_face_bord(inconnue->temps(),rang2,ori,la_zcl));
  if ( !uses_wall_law())
    {
      double dist = dist_norm_bord(fac1);
      double tau  = signe*(vit_imp - inco[fac3])/dist;
      double surf = 0.5*(surface(fac1)+surface(fac2));
      flux = tau*surf*nu_3(0);
    }
  else
    {
      // On calcule u_star*u_star*surf sur chaque partie de la facette de Qdm
      int signe_terme;
      if ( vit < vit_imp )
        signe_terme = -1;
      else
        signe_terme = 1;

      //30/09/2003  YB : influence de signe terme eliminee, signe pris en compte dans la loi de paroi
      signe_terme = 1;

      double tau1 = tau_tan(rang1,ori)*0.5*surface(fac1);
      double tau2 = tau_tan(rang2,ori)*0.5*surface(fac2);
      double coef = tau1+tau2;
      flux = signe_terme*coef;
    }
  return flux;
}

//// flux_arete_paroi_fluide
//
template<>
inline void Eval_Diff_VDF_Face<Eval_Dift_VDF_const_Face>::flux_arete_paroi_fluide(const DoubleTab& inco ,
                                                                                  int fac1,int fac2 , int fac3, int signe,
                                                                                  double& flux3, double& flux1_2 ) const
{
  int rang1 = (fac1-premiere_face_bord);
  int rang2 = (fac2-premiere_face_bord);
  int elem1 = elem_(fac3,0);
  int elem2 = elem_(fac3,1);
  double visc_lam = nu_3(0);
  double visc_turb = nu_1(elem1,elem2);
  int ori= orientation(fac3);

  // On ne sait pas qui de fac1 ou de fac2 est la face de paroi
  double vit_imp;
  if (est_egal(inco[fac1],0)) // fac1 est la face de paroi
    vit_imp = Champ_Face_get_val_imp_face_bord(inconnue->temps(),rang2,ori,la_zcl);
  else  // fac2 est la face de paroi
    vit_imp = Champ_Face_get_val_imp_face_bord(inconnue->temps(),rang1,ori,la_zcl);

  double dist = dist_norm_bord(fac1);
  double tau = signe * (vit_imp - inco[fac3])/dist;
  double tau_tr = (inco[fac2] - inco[fac1])/dist_face(fac1,fac2,ori);
  double surf = 0.5*(surface(fac1)+surface(fac2));
  double poros = 0.5*(porosite(fac1)+porosite(fac2));
  double reyn = (tau + tau_tr)*visc_turb;
  double coef = (tau*visc_lam + reyn);
  flux3 = coef*surf*poros;
  flux1_2 = (tau_tr*visc_lam + reyn)*surface(fac3)*porosite(fac3);
}

//// coeffs_arete_paroi_fluide
//
template<>
inline void Eval_Diff_VDF_Face<Eval_Dift_VDF_const_Face>::coeffs_arete_paroi_fluide(int fac1,
                                                                                    int fac2, int fac3, int signe,double& aii1_2, double& aii3_4,
                                                                                    double& ajj1_2) const
{
  double dist;
  int ori= orientation(fac3);
  int elem1 = elem_(fac3,0);
  int elem2 = elem_(fac3,1);
  double visc_lam = nu_3(0);
  double visc_turb = nu_1(elem1,elem2);

  //Calcul des aii et ajj 3_4
  // On ne sait pas qui de fac1 ou de fac2 est la face de paroi
  dist = dist_norm_bord(fac1);
  double tau = signe/dist;
  double tau_tr = 1/dist_face(fac1,fac2,ori);
  double surf = 0.5*(surface(fac1)+surface(fac2));
  double poros = 0.5*(porosite(fac1)+porosite(fac2));
  double reyn = (tau + tau_tr)*visc_turb;
  double coef = (tau*visc_lam + reyn);

  aii3_4 = coef*surf*poros;
  aii1_2 = ajj1_2 =(tau_tr*visc_lam + reyn)*surface(fac3)*porosite(fac3);
}

//// secmem_arete_paroi_fluide
//
template<>
inline void Eval_Diff_VDF_Face<Eval_Dift_VDF_const_Face>::secmem_arete_paroi_fluide(int fac1,
                                                                                    int fac2, int fac3, int signe,double& flux3, double& flux1_2) const
{
  int rang1 = (fac1-premiere_face_bord);
  int rang2 = (fac2-premiere_face_bord);
  int elem1 = elem_(fac3,0);
  int elem2 = elem_(fac3,1);
  double visc_lam = nu_3(0);
  double visc_turb = nu_1(elem1,elem2);
  int ori= orientation(fac3);

  // On ne sait pas qui de fac1 ou de fac2 est la face de paroi

  double vit_imp;
  if (est_egal(inconnue->valeurs()[fac1],0)) // fac1 est la face de paroi
    vit_imp = Champ_Face_get_val_imp_face_bord(inconnue->temps(),rang2,ori,la_zcl);
  else  // fac2 est la face de paroi
    vit_imp = Champ_Face_get_val_imp_face_bord(inconnue->temps(),rang1,ori,la_zcl);

  double dist = dist_norm_bord(fac1);
  double tau = signe*vit_imp/dist;
  double surf = 0.5*(surface(fac1)+surface(fac2));
  double poros = 0.5*(porosite(fac1)+porosite(fac2));
  double reyn = tau*visc_turb;
  double coef = (tau*visc_lam + reyn);

  flux3 = coef*surf*poros;
  flux1_2 = 0;
}

//// flux_arete_periodicite
//
template<>
inline void Eval_Diff_VDF_Face<Eval_Dift_VDF_const_Face>::flux_arete_periodicite(const DoubleTab& inco,
                                                                                 int fac1, int fac2, int fac3, int fac4,
                                                                                 double& flux3_4, double& flux1_2) const
{
  double flux;
  int ori1 = orientation(fac1);
  int ori3 = orientation(fac3);
  int elem1 = elem_(fac3,0);
  int elem2 = elem_(fac3,1);
  int elem3 = elem_(fac4,0);
  int elem4 = elem_(fac4,1);
  double dist3_4 = dist_face_period(fac3,fac4,ori1);
  double dist1_2 = dist_face_period(fac1,fac2,ori3);
  double visc_lam = nu_3(0);
  double visc_turb = nu_2(elem1,elem2,elem3,elem4);
  double tau = (inco[fac4]-inco[fac3])/dist3_4;
  double tau_tr = (inco[fac2]-inco[fac1])/dist1_2;
  double reyn = (tau + tau_tr)*visc_turb;

  flux = 0.25*(reyn + visc_lam*tau)*(surface(fac1)+surface(fac2))
         *(porosite(fac1)+porosite(fac2));

  flux3_4 = flux;

  flux = 0.25*(reyn + visc_lam*tau_tr)*(surface(fac3)+surface(fac4))
         *(porosite(fac3)+porosite(fac4));

  flux1_2 = flux;
}

//// coeffs_arete_periodicite
//
template<>
inline void Eval_Diff_VDF_Face<Eval_Dift_VDF_const_Face>::coeffs_arete_periodicite(int fac1, int fac2,
                                                                                   int fac3, int fac4,double& aii, double& ajj) const
{
  int ori1 = orientation(fac1);
  int ori3 = orientation(fac3);
  int elem1 = elem_(fac3,0);
  int elem2 = elem_(fac3,1);
  int elem3 = elem_(fac4,0);
  int elem4 = elem_(fac4,1);
  double dist3_4 = dist_face_period(fac3,fac4,ori1);
  double dist1_2 = dist_face_period(fac1,fac2,ori3);
  double visc_lam = nu_3(0);
  double visc_turb = nu_2(elem1,elem2,elem3,elem4);
  double tau = 1/dist3_4;
  double tau_tr = 1/dist1_2;
  double reyn = (tau + tau_tr)*visc_turb;

  aii = ajj =0.25*(reyn + visc_lam*tau)*(surface(fac1)+surface(fac2))*(porosite(fac1)+porosite(fac2));
}

//// flux_fa7_elem
//
template<>
inline double Eval_Diff_VDF_Face<Eval_Dift_VDF_const_Face>::flux_fa7_elem(const DoubleTab& inco,
                                                                          int elem,int fac1, int fac2) const
{
  double flux;
  int ori=orientation(fac1);
  double tau = (inco[fac2]-inco[fac1])/dist_face(fac1,fac2,ori);
  double surf = 0.5*(surface(fac1)*porosite(fac1)+surface(fac2)*porosite(fac2));
  double reyn =  - 2.*nu_t(elem)*tau ;// On considere que l energie est dans la pression
  // On verifie que les termes diagonaux du tenseur de reynolds sont bien positifs
  // Sinon on annulle :
  //  if (reyn < 0) reyn=0. ;

  // E Saikali : Les commentaires précédentes ....  On comprend rien du tout !
  flux = (-reyn + nu_3(0)*tau) * surf ;
  return flux;
}

//// coeffs_fa7_elem
//
template<>
inline void Eval_Diff_VDF_Face<Eval_Dift_VDF_const_Face>::coeffs_fa7_elem(int elem ,int fac1,
                                                                          int fac2, double& aii, double& ajj) const
{
  int ori=orientation(fac1);
  double tau = 1/dist_face(fac1,fac2,ori);
  double surf = 0.5*(surface(fac1)*porosite(fac1)+surface(fac2)*porosite(fac2));
  double reyn =  - 2.*nu_t(elem)*tau ;  // On considere que l energie est dans la pression
  //  double reyn = 2./3.*k_elem - 2.*dv_diffusivite_turbulente(elem)*tau ;
  // On verifie que les termes diagonaux du tenseur de reynolds sont bien positifs
  // Sinon on annulle :
  //  if (reyn < 0) reyn=0. ;

  aii = ajj =(-reyn + nu_3(0)*tau) * surf;
}

//// flux_arete_symetrie_fluide
//
template<>
inline void Eval_Diff_VDF_Face<Eval_Dift_VDF_const_Face>::flux_arete_symetrie_fluide(const DoubleTab& inco,
                                                                                     int fac1,int fac2, int fac3, int signe,
                                                                                     double& flux3, double& flux1_2) const
{
  int rang1 = (fac1-premiere_face_bord);
  int rang2 = (fac2-premiere_face_bord);
  int elem1 = elem_(fac3,0);
  int elem2 = elem_(fac3,1);
  double visc_lam = nu_3(0);
  double visc_turb = nu_1(elem1,elem2);
  int ori= orientation(fac3);
  double vit_imp = 0.5*(Champ_Face_get_val_imp_face_bord_sym(inco,inconnue->temps(),rang1,ori,la_zcl)+
                        Champ_Face_get_val_imp_face_bord_sym(inco,inconnue->temps(),rang2,ori,la_zcl));

  double dist = dist_norm_bord(fac1);
  double tau = signe * (vit_imp - inco[fac3])/dist;
  double tau_tr = (inco[fac2] - inco[fac1])/dist_face(fac1,fac2,ori);
  double surf = 0.5*(surface(fac1)+surface(fac2));
  double poros = 0.5*(porosite(fac1)+porosite(fac2));
  double reyn = (tau + tau_tr)*visc_turb;
  double coef = (tau*visc_lam + reyn);

  flux3 = coef*surf*poros;
  flux1_2 = (tau_tr*visc_lam + reyn)*surface(fac3)*porosite(fac3);
}

//// coeffs_arete_symetrie_fluide
//
template<>
inline void Eval_Diff_VDF_Face<Eval_Dift_VDF_const_Face>::coeffs_arete_symetrie_fluide(int fac1, int fac2,
                                                                                       int fac3, int signe,double& aii1_2,
                                                                                       double& aii3_4,double& ajj1_2) const
{
  int elem1 = elem_(fac3,0);
  int elem2 = elem_(fac3,1);
  double visc_lam = nu_3(0);
  double visc_turb = nu_1(elem1,elem2);
  int ori= orientation(fac3);
  double dist = dist_norm_bord(fac1);
  double tau = signe/dist;
  double tau_tr = 1/dist_face(fac1,fac2,ori);
  double surf = 0.5*(surface(fac1)+surface(fac2));
  double poros = 0.5*(porosite(fac1)+porosite(fac2));
  double reyn = (tau + tau_tr)*visc_turb;
  double coef = (tau*visc_lam + reyn);

  // Calcul de aii3_4
  aii3_4 = coef*surf*poros;

  // Calcul de aii1_2 et ajj1_2
  aii1_2 = ajj1_2  = (tau_tr*visc_lam + reyn)*surface(fac3)*porosite(fac3);
}

//// secmem_arete_symetrie_fluide
//
template<>
inline void Eval_Diff_VDF_Face<Eval_Dift_VDF_const_Face>::secmem_arete_symetrie_fluide(int fac1, int fac2,
                                                                                       int fac3, int signe,double& flux3, double& flux1_2) const
{
  int rang1 = (fac1-premiere_face_bord);
  int rang2 = (fac2-premiere_face_bord);
  int elem1 = elem_(fac3,0);
  int elem2 = elem_(fac3,1);
  double visc_lam = nu_3(0);
  double visc_turb = nu_1(elem1,elem2);
  int ori= orientation(fac3);
  double vit_imp = 0.5*(Champ_Face_get_val_imp_face_bord(inconnue->temps(),rang1,ori,la_zcl)+
                        Champ_Face_get_val_imp_face_bord(inconnue->temps(),rang2,ori,la_zcl));

  double dist = dist_norm_bord(fac1);
  double tau = signe*vit_imp/dist;
  double surf = 0.5*(surface(fac1)+surface(fac2));
  double poros = 0.5*(porosite(fac1)+porosite(fac2));
  double reyn = tau*visc_turb;
  double coef = (tau*visc_lam + reyn);

  flux3 = coef*surf*poros;
  flux1_2 = 0;
}

//// flux_arete_symetrie_paroi
//
template<>
inline double Eval_Diff_VDF_Face<Eval_Dift_VDF_const_Face>::flux_arete_symetrie_paroi(const DoubleTab& inco,
                                                                                      int fac1,int fac2, int fac3, int signe ) const
{
  double flux;
  int rang1 = (fac1-premiere_face_bord);
  int rang2 = (fac2-premiere_face_bord);
  int ori = orientation(fac3);
  //  double vit = inco(fac3);
  double vit_imp = 0.5*(Champ_Face_get_val_imp_face_bord_sym(inco,inconnue->temps(),rang1,ori,la_zcl)+
                        Champ_Face_get_val_imp_face_bord_sym(inco,inconnue->temps(),rang2,ori,la_zcl));
  if ( !uses_wall_law() )
    {
      double dist = dist_norm_bord(fac1);
      double tau  = signe*(vit_imp - inco[fac3])/dist;
      double surf = 0.5*(surface(fac1)+surface(fac2));
      flux = tau*surf*nu_3(0);
    }
  else
    {
      // solution retenue pour le calcul du frottement sachant que sur l'une des faces
      // tau_tan vaut 0 car c'est une face qui porte une condition de symetrie
      // On calcule u_star*u_star*surf sur chaque partie de la facette de Qdm
      double tau1 = tau_tan(rang1,ori)*0.5*surface(fac1);
      double tau2 = tau_tan(rang2,ori)*0.5*surface(fac2);
      double coef = tau1+tau2;
      flux = coef;
    }
  return flux;
}

//// coeffs_arete_symetrie_paroi
//
template<>
inline void Eval_Diff_VDF_Face<Eval_Dift_VDF_const_Face>::coeffs_arete_symetrie_paroi(int fac1, int fac2,
                                                                                      int fac3, int signe,double& aii1_2,
                                                                                      double& aii3_4, double& ajj1_2) const
{
  aii3_4 = 0;
  aii1_2 = 0;
  ajj1_2 = 0;
}

//// secmem_arete_symetrie_paroi
//
template<>
inline double Eval_Diff_VDF_Face<Eval_Dift_VDF_const_Face>::secmem_arete_symetrie_paroi(int fac1,
                                                                                        int fac2, int fac3, int signe) const
{
  double flux;
  int rang1 = (fac1-premiere_face_bord);
  int rang2 = (fac2-premiere_face_bord);
  int ori = orientation(fac3);
  double vit_imp = 0.5*(Champ_Face_get_val_imp_face_bord(inconnue->temps(),rang1,ori,la_zcl)+
                        Champ_Face_get_val_imp_face_bord(inconnue->temps(),rang2,ori,la_zcl));
  if ( !uses_wall_law())
    {
      const DoubleTab& inco = inconnue->valeurs();
      double dist = dist_norm_bord(fac1);
      double tau  = signe*(vit_imp - inco[fac3])/dist;
      double surf = 0.5*(surface(fac1)+surface(fac2));
      flux = tau*surf*nu_3(0);
    }
  else
    {
      // solution retenue pour le calcul du frottement sachant que sur l'une des faces
      // tau_tan vaut 0 car c'est une face qui porte une condition de symetrie
      // On calcule u_star*u_star*surf sur chaque partie de la facette de Qdm
      double tau1 = tau_tan(rang1,ori)*0.5*surface(fac1);
      double tau2 = tau_tan(rang2,ori)*0.5*surface(fac2);
      double coef = tau1+tau2;
      flux = coef;
    }
  return flux;
}

#endif /* Eval_Dift_VDF_const_Face_included */
