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

#ifndef Vect_impl_included
#define Vect_impl_included

#include <DerOu_.h>
#include <assert.h>
#include <Nom.h>

class Vect_impl : public Objet_U
{
  Declare_instanciable_sans_constructeur_ni_destructeur(Vect_impl);

public :
  Vect_impl();
  inline int size() const;
  void build_vect_impl(int );
  inline const Objet_U& operator[](int) const;
  inline Objet_U& operator[](int);
  Vect_impl(const Vect_impl&);
  ~Vect_impl() override;
  Vect_impl& operator=(const Vect_impl& );
  void dimensionner(int) ;
  void dimensionner_force(int);
  void reset();
  Vect_impl& add(const Objet_U&);
  Vect_impl& add(const Vect_impl&);
  Entree& lit(Entree& );

protected :
  virtual Objet_U* cree_une_instance() const;
  int sz;
  DerObjU* data;

};
/* Size of the VECT */
inline int Vect_impl::size() const
{
  return sz;
}
/* Returns the ith VECT element */
inline const Objet_U& Vect_impl::operator[](int i) const
{
  assert( (i>=0) && (i<sz) );
  assert(sz>=0);
  if(sz) assert(data!=0);
  return (const Objet_U&)data[i].valeur();
}
/* Returns the ith VECT element */
inline Objet_U& Vect_impl::operator[](int i)
{
  assert( (i>=0) && (i<sz) );
  assert(sz>=0);
  if(sz) assert(data!=0);
  return (Objet_U&)data[i].valeur();
}

#endif /* Vect_impl_included */
