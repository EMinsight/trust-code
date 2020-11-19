/****************************************************************************
* Copyright (c) 2020, CEA
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
// File:        Matrice_Petsc.h
// Directory:   $TRUST_ROOT/src/Kernel/Math/Matrices
// Version:     /main/41
//
//////////////////////////////////////////////////////////////////////////////

#ifndef Matrice_Petsc_included
#define Matrice_Petsc_included

#include <petsc_for_kernel.h>
#include <Matrice_Base.h>
class Matrice_Petsc : public Matrice_Base
{
  Declare_instanciable(Matrice_Petsc);
public:
  virtual int ordre() const
  {
#ifdef PETSCKSP_H
    PetscInt m,n;
    MatGetSize(Mat_, &m, &n);
    return m == n ? m : 0;
#else
    Cerr << "Matrice_Petsc non disponible." << finl;
    Process::exit();
    return 0;
#endif
  };
  virtual int nb_lignes() const
  {
#ifdef PETSCKSP_H
    PetscInt m,n;
    MatGetSize(Mat_, &m, &n);
    return m;
#else
    Cerr << "Matrice_Petsc non disponible." << finl;
    Process::exit();
    return 0;
#endif
  }
  virtual int nb_colonnes() const
  {
#ifdef PETSCKSP_H
    PetscInt m,n;
    MatGetSize(Mat_, &m, &n);
    return n;
#else
    Cerr << "Matrice_Petsc non disponible." << finl;
    Process::exit();
    return 0;
#endif
  }
  virtual void scale(const double& x)
  {
    Cerr << "ToDo" << finl;
    Process::exit(-1);
  };

  virtual DoubleVect& ajouter_multvect_(const DoubleVect& x, DoubleVect& r) const
  {
    Cerr << "ToDo" << finl;
    Process::exit(-1);
    return r;
  }
  virtual DoubleVect& ajouter_multvectT_(const DoubleVect& x, DoubleVect& r) const
  {
    Cerr << "ToDo" << finl;
    Process::exit(-1);
    return r;
  }
  virtual DoubleTab& ajouter_multTab_(const DoubleTab& x, DoubleTab& r) const
  {
    Cerr << "ToDo" << finl;
    Process::exit(-1);
    return r;
  }
#ifdef PETSCKSP_H
  inline const Mat& getMat() const
  {
    return Mat_;
  }

private :
  Mat Mat_;
#endif
};
#endif
