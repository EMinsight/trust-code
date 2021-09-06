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
// File:        SETS.h
// Directory:   $TRUST_ROOT/src/ThHyd/Multiphase/Schemas_Temps
// Version:     /main/13
//
//////////////////////////////////////////////////////////////////////////////


#ifndef SETS_included
#define SETS_included

#include <Simpler.h>
#include <QDM_Multiphase.h>
#include <Interface_blocs.h>
#include <utility>
#include <set>
#include <vector>

//Description

// SETS (semi-implicite + etapes de stabilisation, a la TRACE)

class SETS : public Simpler
{

  Declare_instanciable_sans_constructeur(SETS);

public :

  SETS();
  virtual Entree& lire(const Motcle&, Entree&); /* mot-cle "criteres_convergence" */

  virtual void iterer_NS(Equation_base&, DoubleTab& current, DoubleTab& pression, double, Matrice_Morse&, double, DoubleTrav&,int nb_iter,int& converge);


  /* elimination par blocs d'un systeme lineaire */
  // entree : ordre -> groupes de (variables, indice de bloc) a eliminer successivement : par ex. { { {"vitesse", 0 } }, { {"alpha", 0 }, {"temperature", 0 } } }
  //          inco_p -> nom de l'inconnue principale (ex. : "pression")
  //          mats, sec : matrices / seconds membres du systeme lineaire mats.d{incos} = {secs}
  //
  //
  // sortie : A_p / b_p t.q. d{inco} = A_p.d{inco_p} + b_p
  //
  // contraintes :  - les inconnues du meme bloc doivent partager un meme MD_Vector
  //                - pour chaque bloc { i_1, i_2 }, la matrice { mats[i_j][i_k] } doit etre diagonale par blocs par rapport a ce MD_Vector
  //                - hors cette diagonale, les inconnues d'un blocs ne peuvent dependre que des blocs precedents et de inco_p

  static void eliminer(const std::vector<std::set<std::pair<std::string, int>>> ordre, const std::string inco_p, const std::map<std::string, matrices_t>& mats, const tabs_t& sec,
                       std::map<std::string, Matrice_Morse>& A_p, tabs_t& b_p);

  /* assemblage d'un systeme en inco_p a partir des expressions d.{inco} : A_p[inco].d{inco_p} + b_p[inco] */
  //entree : - inco_p -> l'inconnue principale
  //         - extra_eq -> noms d'equations supplementaires a utiliser, en plus de l'equation sum_k alpha_k = 1 (utilisee par defaut)
  //         - A_p, b_p -> expressions des autres inconnues calculees par eliminer()
  //         - mats, sec -> systeme lineaire contenant les equations de extra_eqs
  //         - inco_a -> valeur actuelle de l'inconnue "alpha" (necessaire pour assembler l'esuation sum_k a_k = 1)
  //
  //sortie : systeme matrice.d{inco_p} = secmem
  //
  //contraintes : toutes les autres inconnues doivent etre exprimees dans A_p / b_p

  static void assembler(const std::string inco_p, const std::vector<std::string> extra_eq, const std::map<std::string, Matrice_Morse>& A_p, const tabs_t& b_p,
                        const std::map<std::string, matrices_t>& mats, const tabs_t& sec, const DoubleTab& inco_a, Matrice_Morse& matrice, DoubleTab& secmem, int p_degen);

  int iteration;  //numero de l'iteration en cours (pour les oeprateurs d'evanescence)
  int p_degen;    //1 si la pression est degeneree (milieu incompressible + pas de CLs de pression imposee)

protected :

  int sets_;      // 1 si on fait l'etape de prediction des vitesses
  int first_call_;//au tout premier appel, P peut etre tres mauvais -> on ne predit pas v en SETS

  /* criteres de convergences par inconnue (en norme Linf), modifiables par le mot-cle "criteres_convergence" */
  std::map<std::string, double> crit_conv;

  /* matrices de la resolution semi-implicite */
  std::map<std::string, matrices_t> mats; // matrices : mats[nom de l'inconnue de l'equation][nom de l'autre inconnue] = matrice
  Matrice_Bloc mat_semi_impl; //pour stocker les matrices de mats
  std::map<std::string, Matrice_Morse> mat_pred; //matrices de prediction

  /* elimination en pression dv = A_p[nom de la variable]. dp + b_p[nom de la variable] */
  std::map<std::string, Matrice_Morse> A_p;

  /* matrice en pression */
  Matrice_Morse matrice_pression;
};

//Description

// semi-implicte ICE, a la CATHARE 3D

class ICE : public SETS
{
  Declare_instanciable(ICE);
};

#endif

