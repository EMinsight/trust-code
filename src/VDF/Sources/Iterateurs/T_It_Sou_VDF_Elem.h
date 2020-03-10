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
// File:        T_It_Sou_VDF_Elem.h
// Directory:   $TRUST_ROOT/src/VDF/Sources/Iterateurs
// Version:     /main/13
//
//////////////////////////////////////////////////////////////////////////////

#ifndef T_It_Sou_VDF_Elem_included
#define T_It_Sou_VDF_Elem_included

#include <Iterateur_Source_VDF_base.h>
#include <Zone_VDF.h>
#include <Champ_Uniforme.h>

//////////////////////////////////////////////////////////////////////////////
//
// CLASS Iterateur_Source_VDF_Elem
//
//////////////////////////////////////////////////////////////////////////////

template <class _TYPE_>
class T_It_Sou_VDF_Elem : public Iterateur_Source_VDF_base
{
  //Declare_instanciable(T_It_Sou_VDF_Elem(_TYPE_));
  inline int duplique() const
  {
    T_It_Sou_VDF_Elem* xxx = new  T_It_Sou_VDF_Elem(*this);
    if(!xxx)
      {
        Cerr << "Not enough memory " << finl;
        exit();
      }
    return xxx->numero();
  };
  inline unsigned taille_memoire() const
  {
    throw;
  };
public:

  inline Evaluateur_Source_VDF& evaluateur();

  DoubleTab& calculer(DoubleTab& ) const;
  DoubleTab& ajouter(DoubleTab& ) const;
  inline void completer_();

protected:

  _TYPE_ evaluateur_source_elem;

  DoubleTab& ajouter_(DoubleTab& ) const;
  DoubleTab& ajouter_(DoubleTab& , int ) const;

  int nb_elem_;

};

template <class _TYPE_>
inline void T_It_Sou_VDF_Elem<_TYPE_>::completer_()
{
  nb_elem_=la_zone->nb_elem();
}
template <class _TYPE_>  inline Evaluateur_Source_VDF& T_It_Sou_VDF_Elem<_TYPE_>::evaluateur()
{
  Evaluateur_Source_VDF& eval = (Evaluateur_Source_VDF&) evaluateur_source_elem;
  return eval;
}


//Implemente_instanciable(T_It_Sou_VDF_Elem<_TYPE_>,"Iterateur_Source_VDF_Elem",Iterateur_Source_VDF_base);
/*
 template <class _TYPE_>  Sortie& T_It_Sou_VDF_Elem<_TYPE_>::printOn(Sortie& s ) const
{
  return s << que_suis_je();
}

 template <class _TYPE_>  Entree& T_It_Sou_VDF_Elem<_TYPE_>::readOn(Entree& s )
{
  return s ;
}
*/
template <class _TYPE_>  DoubleTab& T_It_Sou_VDF_Elem<_TYPE_>::ajouter(DoubleTab& resu) const
{
  ((_TYPE_&) (evaluateur_source_elem)).mettre_a_jour( );
  assert(resu.nb_dim() < 3);
  int ncomp=1;
  if (resu.nb_dim() == 2)
    ncomp=resu.dimension(1);
  DoubleVect& bilan = so_base->bilan();
  bilan=0;
  if( ncomp == 1)
    ajouter_(resu) ;
  else
    ajouter_(resu, ncomp) ;
  return resu;
}
template <class _TYPE_>  DoubleTab& T_It_Sou_VDF_Elem<_TYPE_>::ajouter_(DoubleTab& resu) const
{
  DoubleVect& bilan = so_base->bilan();
  for (int num_elem=0; num_elem<nb_elem_; num_elem++)
    {
      double source = evaluateur_source_elem.calculer_terme_source(num_elem);
      resu[num_elem] += source;
      bilan(0) += source;
    }
  return resu;
}
template <class _TYPE_>  DoubleTab& T_It_Sou_VDF_Elem<_TYPE_>::ajouter_(DoubleTab& resu,int ncomp) const
{
  DoubleVect source(ncomp);
  DoubleVect& bilan = so_base->bilan();
  for (int num_elem=0; num_elem<nb_elem_; num_elem++)
    {
      evaluateur_source_elem.calculer_terme_source(num_elem, source);
      for (int k=0; k<ncomp; k++)
        {
          resu(num_elem,k) += source(k);
          bilan(k) += source(k);
        }
    }
  return resu;
}
template <class _TYPE_>  DoubleTab& T_It_Sou_VDF_Elem<_TYPE_>::calculer(DoubleTab& resu) const
{
  return resu;
}
#endif
