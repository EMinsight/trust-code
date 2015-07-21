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
// File:        Pb_QC_base.h
// Directory:   $TRUST_ROOT/src/ThHyd/Quasi_Compressible
// Version:     /main/9
//
//////////////////////////////////////////////////////////////////////////////


#ifndef Pb_QC_base_included
#define Pb_QC_base_included

#include <Pb_qdm_fluide.h>


//////////////////////////////////////////////////////////////////////////////
//
// .DESCRIPTION
//    classe Pb_QC_base
//    Cette classe est censee factoriser ce qui est commun a l'ensemble
//    des problemes quasi-compressibles.
//    Il est suppose dans l'algorithme de iterateTimeStep que la
//     premiere equation est hydraulique et la deuxieme est thermique.
// .SECTION voir Pb_qdm_fluide
//////////////////////////////////////////////////////////////////////////////
class Pb_QC_base : public Pb_qdm_fluide
{

  Declare_base(Pb_QC_base);

public:

  virtual bool initTimeStep(double dt); // Appelle aussi sur le fluide
  virtual bool iterateTimeStep(bool& converged); // Schema de resolution particulier au QC

  virtual void preparer_calcul(); // Prepare aussi le fluide
  virtual void mettre_a_jour(double temps); // Ne met a jour que les postraitements
  virtual void associer_milieu_base(const Milieu_base& ); // Verifie le milieu
  virtual void associer_sch_tps_base(const Schema_Temps_base&); // Verifie le schema

protected:


};



#endif
