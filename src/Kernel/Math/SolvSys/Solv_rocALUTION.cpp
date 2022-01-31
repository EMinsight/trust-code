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
// File:        Solv_rocALUTION.cpp
// Directory:   $TRUST_ROOT/src/Kernel/Math/SolvSys
// Version:     /main/0
//
//////////////////////////////////////////////////////////////////////////////

#include <Solv_rocALUTION.h>
#include <Matrice_Morse.h>
#include <DoubleVect.h>

Implemente_instanciable_sans_constructeur_ni_destructeur(Solv_rocALUTION,"Solv_rocALUTION",SolveurSys_base);

// printOn
Sortie& Solv_rocALUTION::printOn(Sortie& s ) const
{
  return s;
}

// readOn
Entree& Solv_rocALUTION::readOn(Entree& is)
{
  return is;
}

Solv_rocALUTION::Solv_rocALUTION()
{
}
Solv_rocALUTION::~Solv_rocALUTION()
{
}

int Solv_rocALUTION::resoudre_systeme(const Matrice_Base& a, const DoubleVect& b, DoubleVect& x)
{
#ifdef ROCALUTION_ROCALUTION_HPP_

  // Build matrix
  const Matrice_Morse& csr = ref_cast(Matrice_Morse, a);
  ArrOfInt tab1_(csr.get_tab1()), tab2_(csr.get_tab2());
  ArrOfDouble coeff_(csr.get_coeff());
  mat.SetDataPtrCSR((int**)tab1_.addr(), (int**)tab2_.addr(), (double**)coeff_.addr(), "a", coeff_.size_array(), tab1_.size_array(), tab1_.size_array());

  // Move objects to accelerator
  mat.MoveToAccelerator();
  sol.MoveToAccelerator();
  rhs.MoveToAccelerator();
  res.MoveToAccelerator();

  // Allocate vectors
  sol.Allocate("sol", mat.GetN());
  rhs.Allocate("rhs", mat.GetM());
  res.Allocate("res", mat.GetN());

  // Linear Solver
  CG<LocalMatrix<double>, LocalVector<double>, double> ls;

  // Preconditioner
  MultiColoredSGS<LocalMatrix<double>, LocalVector<double>, double> p;
  p.SetRelaxation(1.6); // ToDo omega

  // Build rhs and initial solution:
  assert(mat.GetN()==b.size());
  assert(mat.GetN()==x.size());
  DoubleVect b_(b);
  rhs.SetDataPtr((double**)b_.addr(), "rhs", b.size_array());
  DoubleVect x_(x);
  sol.SetDataPtr((double**)x_.addr(), "sol", x.size_array());

  // Set solver operator
  ls.SetOperator(mat);
  ls.SetPreconditioner(p);
  ls.Build();
  ls.Verbose(2); // Verbosity output
  //ls.InitMinIter(20);

  // Print matrix info
  mat.Info();

  // Start time measurement
  double tick, tack;
  tick = rocalution_time();

  // Solve A x = rhs
  ls.Solve(rhs, &sol);

  // Stop time measurement
  tack = rocalution_time();
  Cout << "Solver execution:" << (tack - tick) / 1e6 << " sec" << finl;

  // Clear solver ?
  ls.Clear();

  // Check residual again e=||Ax-rhs||
  mat.Apply(sol, &res);
  res.ScaleAdd(-1.0, rhs);
  Cout << "||Ax - rhs||_2 = " << res.Norm() << finl;

#endif
  return 1;
}
