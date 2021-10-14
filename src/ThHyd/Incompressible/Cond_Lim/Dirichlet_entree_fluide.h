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
// File:        Dirichlet_entree_fluide.h
// Directory:   $TRUST_ROOT/src/ThHyd/Incompressible/Cond_Lim
// Version:     /main/17
//
//////////////////////////////////////////////////////////////////////////////

#ifndef Dirichlet_entree_fluide_included
#define Dirichlet_entree_fluide_included

#include <Dirichlet.h>


//////////////////////////////////////////////////////////////////////////////
//
// .DESCRIPTION
//    classe Dirichlet_entree_fluide
//    Cette classe represente une condition aux limite imposant une grandeur
//    sur l'entree du fluide. Des classes derivees specialiseront la grandeur
//    imposee: vitesse, temperature, concentration, fraction_massique ...
// .SECTION voir aussi
//    Dirichlet Entree_fluide_vitesse_imposee Entree_fluide_vitesse_imposee_libre
//    Entree_fluide_temperature_imposee Entree_fluide_T_h_imposee
//    Entree_fluide_Fluctu_Temperature_imposee Entree_fluide_Flux_Chaleur_Turbulente_imposee
//    Entree_fluide_K_Eps_impose Entree_fluide_V2_impose
//    Entree_fluide_concentration_imposee Entree_fluide_fraction_massique_imposee
//////////////////////////////////////////////////////////////////////////////
class Dirichlet_entree_fluide  : public Dirichlet
{

  Declare_base(Dirichlet_entree_fluide);

};

#endif

