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
// File:        Solveur_Implicite_Base.h
// Directory:   $TRUST_ROOT/src/Kernel/Framework
// Version:     /main/17
//
//////////////////////////////////////////////////////////////////////////////


#ifndef Solveur_Implicite_Base_included
#define Solveur_Implicite_Base_included



//
// .DESCRIPTION class Solveur_Implicite_Base
//

#include <Objet_U.h>
#include <List_Ref_Equation_base.h>

class Equation_base;
class DoubleTab;

class Solveur_Implicite_Base : public  Objet_U
{
  Declare_base(Solveur_Implicite_Base);

public :
  virtual bool iterer_eqn(Equation_base& equation, const DoubleTab& inconnue, DoubleTab& result, double dt, int numero_iteration) =0;
  virtual bool iterer_eqs(LIST(REF(Equation_base)) eqs, int n, bool test_convergence);

protected :

};

#endif

