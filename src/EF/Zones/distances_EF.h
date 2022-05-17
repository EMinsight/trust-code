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
// File:        distances_EF.h
// Directory:   $TRUST_ROOT/src/EF/Zones
// Version:     /main/14
//
//////////////////////////////////////////////////////////////////////////////

#ifndef distances_EF_inclus
#define distances_EF_inclus
#include <Zone_EF.h>
double norm_vit_lp_k(const DoubleTab& vit,int face,int face_b,const Zone_EF& zone,ArrOfDouble& val,int is_defilante);
double norm_vit_lp(const ArrOfDouble& vit,int face,const Zone_EF& zone,ArrOfDouble& val);
double norm_3D_vit1(const DoubleTab& vit,int fac,int num1,
                    const Zone_EF& zone,
                    double& val1,double& val2,double& val3);
double norm_2D_vit1_k(const DoubleTab& vit,int fac,int num1,
                      const Zone_EF& zone,
                      double& val1,double& val2);
double norm_3D_vit2(const DoubleTab& vit,int fac,int num1,
                    const Zone_EF& zone,
                    double& val1,double& val2,double& val3);
double norm_2D_vit2_k(const DoubleTab& vit,int fac,int num1,
                      const Zone_EF& zone,
                      double& val1,double& val2);
double distance_2D(int fac,int elem,const Zone_EF& zone);

double distance_3D(int fac,int elem,const Zone_EF& zone);
double distance_face_elem(int fac,int elem,const Zone_EF& zone);

// Fonctions inlinees pour optimisation
inline double vitesse_tangentielle(const double& v0,const double& v1,const double& r0,const double& r1)
{
  // On prend fabs car mathematiquement la valeur est >=0
  return sqrt(std::fabs(carre(v0)+carre(v1)-carre(v0*r0+v1*r1)));
}
inline double vitesse_tangentielle(const double& v0,const double& v1,const double& v2,const double& r0,const double& r1,const double& r2)
{
  // On prend fabs car mathematiquement la valeur est >=0
  return sqrt(std::fabs(carre(v0)+carre(v1)+carre(v2)-carre(v0*r0+v1*r1+v2*r2)));
}

inline void calcule_r0r1(const DoubleTab& face_normale, int& fac, double& r0, double& r1)
{
  r0=face_normale(fac,0);
  r1=face_normale(fac,1);
  double tmp=1/sqrt(r0*r0+r1*r1);
  r0*=tmp;
  r1*=tmp;
}

inline void calcule_r0r1r2(const DoubleTab& face_normale, int& fac, double& r0, double& r1, double& r2)
{
  r0=face_normale(fac,0);
  r1=face_normale(fac,1);
  r2=face_normale(fac,2);
  double tmp=1/sqrt(r0*r0+r1*r1+r2*r2);
  r0*=tmp;
  r1*=tmp;
  r2*=tmp;
}

inline double distance_face(int fac,int fac1,const Zone_EF& zone)
{
  int dimension=Objet_U::dimension;
  const DoubleTab& xv = zone.xv();    // centre de gravite des faces
  const DoubleTab& face_normale = zone.face_normales();
  double r0,r1;
  double x0=xv(fac,0);
  double y0=xv(fac,1);
  double x1=xv(fac1,0);
  double y1=xv(fac1,1);
  if (dimension==3)
    {
      double r2;
      double z0=xv(fac,2);
      double z1=xv(fac1,2);
      calcule_r0r1r2(face_normale,fac,r0,r1,r2);
      return std::fabs(r0*(x1-x0)+r1*(y1-y0)+r2*(z1-z0));
    }
  else
    {
      calcule_r0r1(face_normale,fac,r0,r1);
      return std::fabs(r0*(x1-x0)+r1*(y1-y0));
    }
}
#endif
