/****************************************************************************
* Copyright (c) 2015, CEA
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
// File:        Tayl_Green.cpp
// Directory:   $TRUST_ROOT/src/Kernel/Champs
// Version:     /main/7
//
//////////////////////////////////////////////////////////////////////////////

#include <Tayl_Green.h>

Implemente_instanciable_sans_constructeur(Tayl_Green,"Tayl_Green",Champ_Don_base);

Sortie& Tayl_Green::printOn(Sortie& os) const
{
  return os;
}

Entree& Tayl_Green::readOn(Entree& is)
{
  is >> kwave;
  Cerr << "kwave=" << kwave << finl;
  return is;
}

Tayl_Green::Tayl_Green()
{
  dimensionner(1,2);
}

Champ_base& Tayl_Green::affecter(const Champ_base& )
{
  return *this;
}

DoubleVect& Tayl_Green::valeur_a(const DoubleVect& ,
                                 DoubleVect& tab_valeurs) const
{
  assert(0);
  return tab_valeurs;
}

DoubleVect& Tayl_Green::valeur_a_elem(const DoubleVect& ,
                                      DoubleVect& tab_valeurs,
                                      int ) const
{
  assert(0);
  return tab_valeurs;
}

double Tayl_Green::valeur_a_compo(const DoubleVect& ,
                                  int ) const
{
  assert(0);
  return 0;
}

double Tayl_Green::valeur_a_elem_compo(const DoubleVect& ,
                                       int ,int ) const
{
  assert(0);
  return 0;
}

DoubleTab& Tayl_Green::valeur_aux(const DoubleTab& positions,
                                  DoubleTab& tab_valeurs) const
{
  assert(tab_valeurs.size_totale() == positions.size());
  for(int i=0; i<tab_valeurs.dimension(0); i++)
    {
      tab_valeurs(i,0)=-sin(kwave*positions(i,0))*cos(kwave*positions(i,1));
      tab_valeurs(i,1)= cos(kwave*positions(i,0))*sin(kwave*positions(i,1));
      if (dimension == 3)
        tab_valeurs(i,2)= 0.;
    }
  return tab_valeurs;
}

DoubleVect& Tayl_Green::valeur_aux_compo(const DoubleTab& positions,
                                         DoubleVect& tab_valeurs, int ncomp) const
{
  assert(tab_valeurs.size_totale()==positions.dimension(0));
  if(ncomp==0)
    for(int i=0; i<tab_valeurs.size(); i++)
      {
        tab_valeurs(i)=-sin(kwave*positions(i,0))*cos(kwave*positions(i,1));
      }
  else if(ncomp==1)
    for(int i=0; i<tab_valeurs.size(); i++)
      {
        tab_valeurs(i)=cos(kwave*positions(i,0))*sin(kwave*positions(i,1));
      }
  else if(ncomp==2)
    for(int i=0; i<tab_valeurs.size(); i++)
      {
        tab_valeurs(i)=0;
      }
  return tab_valeurs;
}

DoubleTab& Tayl_Green::valeur_aux_elems(const DoubleTab& positions,
                                        const IntVect& ,
                                        DoubleTab& tab_valeurs) const
{
  return valeur_aux(positions, tab_valeurs);
}

DoubleVect& Tayl_Green::valeur_aux_elems_compo(const DoubleTab& positions,
                                               const IntVect& ,
                                               DoubleVect& tab_valeurs,
                                               int ncomp) const
{
  return valeur_aux_compo(positions, tab_valeurs, ncomp);
}

