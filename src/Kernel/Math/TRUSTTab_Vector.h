/****************************************************************************
* Copyright (c) 2022, CEA
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
// File:        TRUSTTab_Vector.h
// Directory:   $TRUST_ROOT/src/Kernel/Math
// Version:     /main/10
//
//////////////////////////////////////////////////////////////////////////////

#ifndef TRUSTTab_Vector_included
#define TRUSTTab_Vector_included

#include <vect_impl.h>
#include <TRUSTTab.h>

// Elie Saikali : mai 2022
// J'ajoute cette classe temporairement (enfin j'espere)
// a virer le jour ou le macro VECT devient / ou accepte des templates
// Pourquoi ? car dans Const_DoubleTab_parts on a VECT(IntTab) et VECT(DoubleTab) .... boom, comment je peux faire sinon ?
template<typename _TYPE_>
class TRUSTTab_Vector: public vect_impl
{
protected:

  inline unsigned taille_memoire() const override { throw; }

  inline int duplique() const override
  {
    TRUSTTab_Vector *xxx = new TRUSTTab_Vector(*this);
    if (!xxx)
      {
        Cerr << "Not enough memory " << finl;
        Process::exit();
      }
    return xxx->numero();
  }

  Objet_U* cree_une_instance() const override
  {
    TRUSTTab<_TYPE_> *instan = new TRUSTTab<_TYPE_>();
    return instan;
  }

  Sortie& printOn(Sortie& s) const override { return vect_impl::printOn(s); }

  Entree& readOn(Entree& s) override { return vect_impl::readOn(s); }

public:
  TRUSTTab_Vector() : vect_impl() { }
  TRUSTTab_Vector(int i) { build_vect_impl(i); }
  TRUSTTab_Vector(const TRUSTTab_Vector& avect) : vect_impl(avect) { }

  /* Returns the ith VECT element */
  const TRUSTTab<_TYPE_>& operator[](int i) const { return static_cast<const TRUSTTab<_TYPE_>&>(vect_impl::operator[](i)); }
  TRUSTTab<_TYPE_>& operator[](int i) { return static_cast<TRUSTTab<_TYPE_>&>(vect_impl::operator[](i)); }

  const TRUSTTab<_TYPE_>& operator()(int i) const { return operator[](i); }
  TRUSTTab<_TYPE_>& operator()(int i) { return operator[](i); }

  TRUSTTab_Vector& operator=(const TRUSTTab_Vector& avect)
  {
    vect_impl::operator=(avect);
    return *this;
  }

  Entree& lit(Entree& s) { return vect_impl::lit(s); }

  TRUSTTab<_TYPE_>& add(const TRUSTTab<_TYPE_>& data_to_add)
  {
    vect_impl::add(data_to_add);
    return (*this)[size() - 1];
  }

  TRUSTTab<_TYPE_>& add() { return add(TRUSTTab<_TYPE_>()); }
  void add(const TRUSTTab_Vector& data_to_add) { vect_impl::add(data_to_add); }
};

// BYE BYE MACRO !! ahahaha
using Vect_DoubleTab = TRUSTTab_Vector<double>; // remplace VECT(DoubleTab)
using Vect_IntTab = TRUSTTab_Vector<int>; // remplace VECT(IntTab)

#endif /* TRUSTTab_Vector_included */
