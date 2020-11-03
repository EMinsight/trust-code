/****************************************************************************
* Copyright (c) 2019, CEA
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
// File:        Champ_P0_CoviMAC.h
// Directory:   $TRUST_ROOT/src/CoviMAC/Champs
// Version:     1
//
//////////////////////////////////////////////////////////////////////////////

#ifndef Champ_P0_CoviMAC_included
#define Champ_P0_CoviMAC_included

#include <Champ_Inc_P0_base.h>
#include <Ref_Zone_VF.h>
#include <Zone_CoviMAC.h>
#include <Operateur.h>

class Zone_CoviMAC;

/////////////////////////////////////////////////////////////////////////////
// .NAME        : Champ_P0_CoviMAC
// .DESCRIPTION : class Champ_P0_CoviMAC
//
// Champ correspondant a une inconnue scalaire (type temperature ou pression)
// Degres de libertes : valeur aux elements + flux aux faces
/////////////////////////////////////////////////////////////////////////////

class Champ_P0_CoviMAC : public Champ_Inc_P0_base
{
  Declare_instanciable(Champ_P0_CoviMAC);

public :

  const Zone_CoviMAC&        zone_CoviMAC() const;
  void                         associer_zone_dis_base(const Zone_dis_base&);
  virtual const Zone_dis_base& zone_dis_base() const;
  int                       imprime(Sortie& , int ) const;

  //tableaux utilitaires sur les CLs
  //types de CL : 0 -> pas de CL
  //              1 -> Echange_externe_impose
  //              2 -> Echange_global_impose
  //              3 -> Echange_contact_CoviMAC gere en monolithique
  //              4 -> Neumann_paroi
  //              5 -> Neumann_sortie_libre
  //              6 -> Dirichlet
  //              7 -> Dirichlet_homogene
  void init_cl() const;
  mutable IntTab icl; //icl(f, .) = (type de la CL, no de la CL, indice dans la CL)

protected :

  REF(Zone_VF) la_zone_VF;


};

#endif
