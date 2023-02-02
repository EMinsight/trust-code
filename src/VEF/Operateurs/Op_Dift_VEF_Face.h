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


#ifndef Op_Dift_VEF_Face_included
#define Op_Dift_VEF_Face_included

#include <Op_Dift_VEF_base.h>
#include <Ref_Champ_P1NC.h>
#include <Matrice_Morse.h>
#include <Champ_base.h>

/*! @brief class Op_Dift_VEF_Face
 *
 *
 *
 */

class Op_Dift_VEF_Face : public Op_Dift_VEF_base
{
  Declare_instanciable(Op_Dift_VEF_Face);

public:

  void associer_diffusivite(const Champ_base&) override;
  inline const Champ_base& diffusivite() const override
  {
    return diffusivite_;
  };
  inline double diffusivite(int) const
  {
    return (diffusivite().valeurs().nb_dim()==1)?(diffusivite().valeurs())(0):(diffusivite().valeurs())(0,0);
  };
  DoubleTab& ajouter(const DoubleTab& ,  DoubleTab& ) const override;
  DoubleTab& calculer(const DoubleTab& , DoubleTab& ) const override;
  double calculer_dt_stab() const override;
  void calculer_pour_post(Champ& espace_stockage,const Nom& option,int comp) const override;
  Motcle get_localisation_pour_post(const Nom& option) const override;

  // Methodes pour l implicite.

  inline void dimensionner(Matrice_Morse& ) const override;
  inline void modifier_pour_Cl(Matrice_Morse&, DoubleTab&) const override;
  inline void contribuer_a_avec(const DoubleTab&, Matrice_Morse&) const override;
  inline void contribuer_au_second_membre(DoubleTab& ) const override;

  void contribue_au_second_membre(DoubleTab& ) const;
  void ajouter_contribution(const DoubleTab&, Matrice_Morse& ) const;
  void ajouter_contribution_cl(const DoubleTab&, Matrice_Morse&, const DoubleTab&, const DoubleTab&, const DoubleVect& ) const;
  void ajouter_contribution_multi_scalaire(const DoubleTab&, Matrice_Morse& ) const;

  void ajouter_cas_scalaire(const DoubleVect& inconnue,
                            DoubleVect& resu, DoubleTab& flux_bords,
                            const DoubleVect& nu,
                            const DoubleVect& nu_turb,
                            const Domaine_Cl_VEF& domaine_Cl_VEF,
                            const Domaine_VEF& domaine_VEF ) const;
  void ajouter_cas_vectoriel(const DoubleTab& inconnue,
                             DoubleTab& resu, DoubleTab& flux_bords,
                             const DoubleTab& nu,
                             const DoubleTab& nu_turb,
                             const Domaine_Cl_VEF& domaine_Cl_VEF,
                             const Domaine_VEF& domaine_VEF,
                             int nb_comp) const;
  void ajouter_cas_multi_scalaire(const DoubleTab& inconnue,
                                  DoubleTab& resu, DoubleTab& flux_bords,
                                  const DoubleTab& nu,
                                  const DoubleVect& nu_turb,
                                  const Domaine_Cl_VEF& domaine_Cl_VEF,
                                  const Domaine_VEF& domaine_VEF,
                                  int nb_comp) const;

protected :
  REF(Champ_base) diffusivite_;

  mutable DoubleTab grad_;  // grad
  //DoubleTab grad_transp_;

};
/*! @brief on dimensionne notre matrice.
 *
 */

inline  void Op_Dift_VEF_Face::dimensionner(Matrice_Morse& matrice) const
{
  Op_VEF_Face::dimensionner(le_dom_vef.valeur(), la_zcl_vef.valeur(), matrice);
}

inline void Op_Dift_VEF_Face::modifier_pour_Cl(Matrice_Morse& matrice, DoubleTab& secmem) const
{
  Op_VEF_Face::modifier_pour_Cl(le_dom_vef.valeur(),la_zcl_vef.valeur(), matrice, secmem);
}


/*! @brief on assemble la matrice.
 *
 */

inline void Op_Dift_VEF_Face::contribuer_a_avec(const DoubleTab& inco,
                                                Matrice_Morse& matrice) const
{
  const Champ_base& inconnue = equation().inconnue().valeur();
  const Nature_du_champ nature_champ = inconnue.nature_du_champ();
  if (nature_champ!=multi_scalaire)
    ajouter_contribution(inco, matrice);
  else
    ajouter_contribution_multi_scalaire(inco, matrice);
}

/*! @brief on ajoute la contribution du second membre.
 *
 */

inline void Op_Dift_VEF_Face::contribuer_au_second_membre(DoubleTab& resu) const
{
  contribue_au_second_membre(resu);
}

#endif

