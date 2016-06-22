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
// File:        Champ_Generique_Moyenne.h
// Directory:   $TRUST_ROOT/src/Kernel/Champs
// Version:     /main/14
//
//////////////////////////////////////////////////////////////////////////////


#ifndef Champ_Generique_Moyenne_included
#define Champ_Generique_Moyenne_included

#include <Champ_Generique_Statistiques_base.h>
#include <Op_Moyenne.h>

class DoubleTab;
class Postraitement_base;

//
// .DESCRIPTION class Champ_Generique_Moyenne
// Champ destine a post-traiter une moyenne d un champ generique
// La classe porte un operateur statistique "Op_Moyenne"

//// Syntaxe a respecter pour jdd
//
// "nom_champ" Moyenne { t_deb "val_tdeb" t_fin "val_tfin"
//                source "type_champ_gen" { ...source ref_Champ { Pb_champ "nom_pb" "nom_champ_discret" } }
//               }
// "nom_champ" fixe par utilisateur sera le nom du champ generique
// "val_tdeb" et "val_tfin" valeur de debut et fin des statistiques pour ce champ
// "type_champ_gen" type d'un champ generique

class Champ_Generique_Moyenne : public Champ_Generique_Statistiques_base
{
  Declare_instanciable(Champ_Generique_Moyenne);

public:

  void set_param(Param& param);
  int lire_motcle_non_standard(const Motcle&, Entree&);
  virtual const Noms get_property(const Motcle& query) const;

  inline double temps() const
  {
    return Op_Moyenne_.integrale().temps();
  };
  inline const Integrale_tps_Champ& integrale() const
  {
    return Op_Moyenne_.integrale();
  };

  inline const Operateur_Statistique_tps_base& Operateur_Statistique() const;
  inline Operateur_Statistique_tps_base& Operateur_Statistique();
  void completer(const Postraitement_base& post);

  virtual const Champ_base&  get_champ(Champ& espace_stockage) const;
  void nommer_source();

  const Champ_Generique_base& get_champ_post(const Motcle& nom) const;
  int comprend_champ_post(const Motcle& identifiant) const;

protected:

  Op_Moyenne Op_Moyenne_;

};


inline const Operateur_Statistique_tps_base& Champ_Generique_Moyenne::Operateur_Statistique() const
{
  return Op_Moyenne_;
}

inline Operateur_Statistique_tps_base& Champ_Generique_Moyenne::Operateur_Statistique()
{
  return Op_Moyenne_;
}
#endif

