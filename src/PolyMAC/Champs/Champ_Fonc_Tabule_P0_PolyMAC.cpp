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
// File:        Champ_Fonc_Tabule_P0_PolyMAC.cpp
// Directory:   $TRUST_ROOT/src/PolyMAC/Champs
// Version:     1
//
//////////////////////////////////////////////////////////////////////////////

#include <Champ_Fonc_Tabule_P0_PolyMAC.h>
#include <Zone_PolyMAC.h>
#include <Champ_Inc.h>
#include <Table.h>

Implemente_instanciable(Champ_Fonc_Tabule_P0_PolyMAC,"Champ_Fonc_Tabule_P0_PolyMAC",Champ_Fonc_P0_PolyMAC);

// printOn


Sortie& Champ_Fonc_Tabule_P0_PolyMAC::printOn(Sortie& s) const
{
  return s << que_suis_je() << " " << le_nom();
}

// readOn

Entree& Champ_Fonc_Tabule_P0_PolyMAC::readOn(Entree& s)
{
  return s ;
}

void Champ_Fonc_Tabule_P0_PolyMAC::associer_param(const Champ_base& un_champ_inc,
                                                  const Table& une_table)
{
  le_champ_parametre = un_champ_inc;
  la_table = une_table;
}


void Champ_Fonc_Tabule_P0_PolyMAC::mettre_a_jour(double t)
{
  const Zone_PolyMAC& zone_PolyMAC =la_zone_PolyMAC.valeur();
  const Table& table = la_table.valeur();
  const DoubleTab& val_param = le_champ_parametre->valeurs();
  DoubleTab& mes_valeurs = valeurs();
  if (!(val_param.nb_dim() == mes_valeurs.nb_dim()))
    {
      Cerr << "Erreur a la mise a jour d'un Champ_Fonc_Tabule" << finl;
      Cerr << "Le champ parametre et le champ a initialiser ne sont pas compatibles" << finl;
      exit();
    }
  int nb_elem=zone_PolyMAC.nb_elem();
  int nb_elem_tot=zone_PolyMAC.nb_elem_tot();
  const int nbcomp=mes_valeurs.line_size();
  DoubleTab val_param_aux_elems(nb_elem_tot, mes_valeurs.dimension(1));
  const DoubleTab& centres_de_gravites=zone_PolyMAC.xp();
  IntVect les_polys(nb_elem_tot);
  for(int elem=0; elem<nb_elem_tot; elem++)
    les_polys(elem)=elem;

  // Estimate the field parameter on cells:
  le_champ_parametre.valeur().valeur_aux_elems(centres_de_gravites,les_polys,val_param_aux_elems);
  // Compute the field according to the parameter field
  if (table.isfonction() != 2)
    {
      for (int num_elem=0; num_elem<nb_elem; num_elem++)
        for (int ncomp=0; ncomp<nbcomp; ncomp++)
          mes_valeurs(num_elem,ncomp) = table.val(val_param_aux_elems(num_elem,ncomp), ncomp);
    }
  else
    table.valeurs(val_param_aux_elems,centres_de_gravites,t,mes_valeurs);

  Champ_Fonc_base::mettre_a_jour(t);
}

int Champ_Fonc_Tabule_P0_PolyMAC::initialiser(const double&)
{
  Champ_Fonc_Tabule_P0_PolyMAC::mettre_a_jour(temps());
  return 1;
}
