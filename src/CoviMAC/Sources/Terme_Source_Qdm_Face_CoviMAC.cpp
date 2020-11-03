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
// File:        Terme_Source_Qdm_Face_CoviMAC.cpp
// Directory:   $TRUST_ROOT/src/CoviMAC/Sources
// Version:     /main/16
//
//////////////////////////////////////////////////////////////////////////////

#include <Terme_Source_Qdm_Face_CoviMAC.h>
#include <Champ_Uniforme.h>
#include <Champ_Don_Fonc_xyz.h>
#include <Champ_Don_Fonc_txyz.h>
#include <Zone_Cl_dis.h>
#include <Zone_CoviMAC.h>
#include <Zone_Cl_CoviMAC.h>
#include <Champ_Face_CoviMAC.h>
#include <Masse_CoviMAC_Face.h>
#include <Equation_base.h>

Implemente_instanciable(Terme_Source_Qdm_Face_CoviMAC,"Source_Qdm_face_CoviMAC",Source_base);



//// printOn
//

Sortie& Terme_Source_Qdm_Face_CoviMAC::printOn(Sortie& s ) const
{
  return s << que_suis_je() ;
}


//// readOn
//

Entree& Terme_Source_Qdm_Face_CoviMAC::readOn(Entree& s )
{
  s >> la_source;
  if (la_source->nb_comp() != dimension)
    {
      Cerr << "Erreur a la lecture du terme source de type " << que_suis_je() << finl;
      Cerr << "le champ source doit avoir " << dimension << " composantes" << finl;
      exit();
    }
  return s ;
}

void Terme_Source_Qdm_Face_CoviMAC::associer_pb(const Probleme_base& )
{
  ;
}

void Terme_Source_Qdm_Face_CoviMAC::associer_zones(const Zone_dis& zone_dis,
                                                   const Zone_Cl_dis& zone_Cl_dis)
{
  la_zone_CoviMAC = ref_cast(Zone_CoviMAC, zone_dis.valeur());
  la_zone_Cl_CoviMAC = ref_cast(Zone_Cl_CoviMAC, zone_Cl_dis.valeur());
}


DoubleTab& Terme_Source_Qdm_Face_CoviMAC::ajouter(DoubleTab& resu) const
{
  const Zone_CoviMAC& zone = la_zone_CoviMAC.valeur();
  const Champ_Face_CoviMAC& ch = ref_cast(Champ_Face_CoviMAC, equation().inconnue().valeur());
  const DoubleTab& vals = la_source->valeurs(), &mu_f = ref_cast(Masse_CoviMAC_Face, equation().solv_masse().valeur()).mu_f, &nf = zone.face_normales();
  const DoubleVect& pe = zone.porosite_elem(), &ve = zone.volumes(), &vf = zone.volumes_entrelaces(), &fs = zone.face_surfaces();
  const IntTab& f_e = zone.face_voisins();
  int e, f, i, unif = sub_type(Champ_Uniforme, la_source.valeur()), nf_tot = zone.nb_faces_tot(), n, N = equation().inconnue().valeurs().line_size(), d, D = dimension;
  ch.init_cl();

  /* contributions aux faces (par chaque voisin), aux elems */
  for (f = 0; f < zone.nb_faces(); f++) if (ch.fcl(f, 0) < 2) for (i = 0; i < 2 && (e = f_e(f, i)) >= 0; i++) for (d = 0; d < D; d++) for (n = 0; n < N; n++)
            resu.addr()[N * f + n] += vf(f) * mu_f(f, n, i) * nf(f, d) / fs(f) * pe(e) * vals(unif ? 0 : e, N * d + n);
  for (e = 0; e < zone.nb_elem(); e++) for (d = 0; d < D; d++) for (n = 0; n < N; n++)
        resu.addr()[N * (nf_tot + D * e + d) + n] += pe(e) * ve(e) * vals(unif ? 0 : e, N * d + n);

  return resu;
}

DoubleTab& Terme_Source_Qdm_Face_CoviMAC::calculer(DoubleTab& resu) const
{
  resu = 0;
  return ajouter(resu);
}

void Terme_Source_Qdm_Face_CoviMAC::mettre_a_jour(double temps)
{
  la_source->mettre_a_jour(temps);
}
