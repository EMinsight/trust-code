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
// File:        Source_Quasi_Compressible_Chaleur.h
// Directory:   $TRUST_ROOT/src/ThHyd/Fluide_Dilatable/Quasi_Compressible/Sources
// Version:     /main/11
//
//////////////////////////////////////////////////////////////////////////////


#ifndef Source_Quasi_Compressible_Chaleur_included
#define Source_Quasi_Compressible_Chaleur_included

#include <Source_base.h>
#include <Ref_Fluide_Quasi_Compressible.h>

class Zone_dis;
class Zone_Cl_dis;

//////////////////////////////////////////////////////////////////////////////
//
// .DESCRIPTION class Source_Quasi_Compressible_Chaleur
//
// Cette classe represente un terme source supplementaire
// a prendre en compte dans les equations de la chaleur
//  dans le cas ou le fluide est quasi compressible
//
// .SECTION voir aussi
// Source_base Fluide_Quasi_Compressible
//
//////////////////////////////////////////////////////////////////////////////

class Source_Quasi_Compressible_Chaleur : public Source_base
{

  Declare_base(Source_Quasi_Compressible_Chaleur);

public:

  inline void associer_pb(const Probleme_base& ) {};

  DoubleTab& calculer(DoubleTab& ) const ;
  void completer();
  inline void mettre_a_jour(double) {};
  DoubleTab& ajouter(DoubleTab& ) const;
protected:
  DoubleVect volumes,porosites;
  virtual void associer_zones(const Zone_dis& ,const Zone_Cl_dis& ) =0;
  REF(Fluide_Quasi_Compressible) le_fluide;
};

#endif

