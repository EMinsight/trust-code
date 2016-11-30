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
// File:        Evaluateur_Source_VEF_Face.h
// Directory:   $TRUST_ROOT/src/VEF/Sources/Evaluateurs
// Version:     /main/10
//
//////////////////////////////////////////////////////////////////////////////

#ifndef Evaluateur_Source_VEF_Face_included
#define Evaluateur_Source_VEF_Face_included


#include <Evaluateur_Source_VEF.h>
#include <DoubleVect.h>
#include <IntTab.h>

//////////////////////////////////////////////////////////////////////////////
//
// CLASS: Evaluateur_Source_VEF_Face
//
//
//////////////////////////////////////////////////////////////////////////////

class Evaluateur_Source_VEF_Face : public Evaluateur_Source_VEF
{

public:

  Evaluateur_Source_VEF_Face();
  Evaluateur_Source_VEF_Face(const Evaluateur_Source_VEF_Face& );
  void completer();
  virtual double calculer_terme_source_standard(int ) const =0;
  virtual double calculer_terme_source_non_standard(int ) const =0;
  virtual void calculer_terme_source_standard(int , DoubleVect&  ) const =0;
  virtual void calculer_terme_source_non_standard(int , DoubleVect&  ) const =0;
  void changer_volumes_entrelaces_Cl(DoubleVect&);

protected:

  DoubleVect volumes_entrelaces;
  DoubleVect volumes_entrelaces_Cl;
  DoubleVect porosite_surf;
  IntTab face_voisins;

};

//
//   Fonctions inline de Evaluateur_Source_VEF_Face
//

inline Evaluateur_Source_VEF_Face::Evaluateur_Source_VEF_Face() {}

#endif
