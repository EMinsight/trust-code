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
// File:        Eval_Conv_VDF_tools.h
// Directory:   $TRUST_ROOT/src/VDF/Operateurs/Evaluateurs_Conv
// Version:     /main/15
//
//////////////////////////////////////////////////////////////////////////////

#ifndef Eval_Conv_VDF_tools_included
#define Eval_Conv_VDF_tools_included

class Eval_Conv_VDF_tools
{
public:
  virtual ~Eval_Conv_VDF_tools() {}
  // DANGER !!!! FAUT JAMAIS ENTRER
  virtual int amont_amont(int face, int i) const { throw; }
  virtual double quick_fram(const double&, const int, const int, const int, const int, const int, const DoubleTab& ) const { throw; }
  virtual void quick_fram(const double&, const int, const int, const int, const int, const int, const DoubleTab&, ArrOfDouble& ) const { throw; }
  virtual double qcentre(const double&, const int, const int, const int, const int, const int, const DoubleTab& ) const { throw; }
  virtual void qcentre(const double&, const int, const int, const int, const int, const int, const DoubleTab&, ArrOfDouble& ) const { throw; }
};

#endif /* Eval_Conv_VDF_tools_included */
