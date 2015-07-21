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
// File:        Eval_Puiss_Th_VEF_Face.cpp
// Directory:   $TRUST_ROOT/src/VEF/Sources/Evaluateurs
// Version:     /main/7
//
//////////////////////////////////////////////////////////////////////////////

#include <Eval_Puiss_Th_VEF_Face.h>
#include <Zone_VEF.h>

void Eval_Puiss_Th_VEF_Face::completer()
{
  Evaluateur_Source_VEF_Face::completer();
  face_voisins.ref(la_zone->face_voisins());
  volumes.ref(la_zone->volumes());
  nb_faces_elem=la_zone->zone().nb_faces_elem();
}

void Eval_Puiss_Th_VEF_Face::associer_champs(const Champ_Don& rho,
                                             const Champ_Don& capa,
                                             const Champ_Don& Q)
{
  rho_ref = rho;
  rho_ref_ = rho(0,0);
  Cp = capa;
  Cp_ = capa(0,0);
  la_puissance = Q;
  puissance.ref(Q.valeurs());
}

void Eval_Puiss_Th_VEF_Face::mettre_a_jour( )
{

}


