/****************************************************************************
* Copyright (c) 2023, CEA
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

#include <Linear_algebra_tools_impl.h>
#include <Connectivite_som_elem.h>
#include <Champ_Fonc_reprise.h>
#include <Champ_Face_PolyMAC.h>
#include <Schema_Temps_base.h>
#include <Champ_Uniforme.h>
#include <TRUSTTab_parts.h>
#include <Probleme_base.h>
#include <Equation_base.h>
#include <Matrix_tools.h>
#include <Zone_PolyMAC.h>
#include <Symetrie.h>
#include <EChaine.h>
#include <Domaine.h>
#include <Zone_VF.h>
#include <array>
#include <cmath>
#include <Operateur.h>
#include <Op_Diff_PolyMAC_base.h>
#include <MD_Vector_base.h>

Implemente_instanciable(Champ_Face_PolyMAC,"Champ_Face_PolyMAC",Champ_Face_base) ;

Sortie& Champ_Face_PolyMAC::printOn(Sortie& os) const
{
  os << que_suis_je() << " " << le_nom();
  return os;
}

Entree& Champ_Face_PolyMAC::readOn(Entree& is) { return is; }

int Champ_Face_PolyMAC::fixer_nb_valeurs_nodales(int n)
{

  // j'utilise le meme genre de code que dans Champ_Fonc_P0_base
  // sauf que je recupere le nombre de faces au lieu du nombre d'elements
  //
  // je suis tout de meme etonne du code utilise dans
  // Champ_Fonc_P0_base::fixer_nb_valeurs_nodales()
  // pour recuperer la zone discrete...

  const Champ_Inc_base& self = ref_cast(Champ_Inc_base, *this);
  const Zone_PolyMAC& zone = ref_cast(Zone_PolyMAC,self.zone_dis_base());

  assert(n == zone.nb_faces() || n < 0); //on accepte a la fois les conventions VEF et VDF

  // Probleme: nb_comp vaut 2 mais on ne veut qu'une dimension !!!
  // HACK :
  const int old_nb_compo = nb_compo_;
  if(nb_compo_ >= dimension) nb_compo_ /= dimension;

  creer_tableau_distribue(zone.md_vector_faces());
  nb_compo_ = old_nb_compo;
  return n;

}

void Champ_Face_PolyMAC::init_auxiliary_variables()
{
  for (int n = 0; n < nb_valeurs_temporelles(); n++)
    if (futur(n).size_reelle_ok())
      {
        DoubleTab& vals = futur(n);
        vals.set_md_vector(MD_Vector()); //on enleve le MD_Vector...
        vals.resize_dim0(zone_PolyMAC().mdv_faces_aretes.valeur().get_nb_items_tot()); //...on dimensionne a la bonne taille...
        vals.set_md_vector(zone_PolyMAC().mdv_faces_aretes); //...et on remet le bon MD_Vector
      }
}

int Champ_Face_PolyMAC::reprendre(Entree& fich)
{
  const Zone_PolyMAC* zone = la_zone_VF.non_nul() ? &ref_cast( Zone_PolyMAC,la_zone_VF.valeur()) : NULL;
  valeurs().set_md_vector(MD_Vector()); //on enleve le MD_Vector...
  valeurs().resize(0);
  int ret = Champ_Inc_base::reprendre(fich);
  //et on met le bon
  if (zone) valeurs().set_md_vector(valeurs().dimension_tot(0) > zone->nb_faces_tot() ? zone->mdv_faces_aretes : zone->md_vector_faces());
  return ret;
}

Champ_base& Champ_Face_PolyMAC::affecter_(const Champ_base& ch)
{
  const DoubleTab& v = ch.valeurs();
  DoubleTab_parts parts(valeurs());
  DoubleTab& val = parts[0]; //partie vitesses
  const Zone_PolyMAC& zone = zone_PolyMAC();
  int nb_faces = zone.nb_faces();
  const DoubleVect& surface = zone.face_surfaces();
  const DoubleTab& normales = zone.face_normales();

  if (sub_type(Champ_Uniforme,ch))
    {
      for (int num_face=0; num_face<nb_faces; num_face++)
        {
          double vn=0;
          for (int dir=0; dir<dimension; dir++)
            vn+=v(0,dir)*normales(num_face,dir);

          vn/=surface(num_face);
          val(num_face) = vn;
        }
    }
  else if (sub_type(Champ_Fonc_reprise, ch))
    {
      for (int num_face=0; num_face<nb_faces; num_face++)
        val(num_face) = ch.valeurs()[num_face];
    }
  else
    {
      //      int ndeb_int = zone.premiere_face_int();
      //      const IntTab& face_voisins = zone.face_voisins();
      const DoubleTab& xv=zone.xv();
      DoubleTab eval(val.dimension_tot(0),dimension);
      ch.valeur_aux(xv,eval);
      for (int num_face=0; num_face<nb_faces; num_face++)
        {
          double vn=0;
          for (int dir=0; dir<dimension; dir++)
            vn+=eval(num_face,dir)*normales(num_face,dir);


          vn/=surface(num_face);
          val(num_face) = vn;
        }
    }
  return *this;
}


DoubleVect& Champ_Face_PolyMAC::valeur_a_elem(const DoubleVect& position, DoubleVect& result, int poly) const
{
  Cerr << "Champ_Face_PolyMAC::" <<__func__ << " is not coded !" << finl;
  throw;
  // return Champ_implementation_RT0::valeur_a_elem(position,result,poly);
}

double Champ_Face_PolyMAC::valeur_a_elem_compo(const DoubleVect& position, int poly, int ncomp) const
{
  Cerr << "Champ_Face_PolyMAC::" <<__func__ << " is not coded !" << finl;
  throw;
  //return Champ_implementation_RT0::valeur_a_elem_compo(position,poly,ncomp);
}

/* vitesse aux elements */
void Champ_Face_PolyMAC::interp_ve(const DoubleTab& inco, DoubleTab& val, bool is_vit) const
{
  const Zone_PolyMAC& zone = zone_PolyMAC();
  const DoubleTab& xv = zone.xv(), &xp = zone.xp();
  const DoubleVect& fs = zone.face_surfaces(), *ppf = mon_equation_non_nul() ? &equation().milieu().porosite_face() : NULL, *ppe = ppf ? &equation().milieu().porosite_elem() : NULL, &ve = zone.volumes();
  const IntTab& e_f = zone.elem_faces(), &f_e = zone.face_voisins();
  int e, f, j, r;

  val = 0;
  for (e = 0; e < val.dimension(0); e++)
    for (j = 0; j < e_f.dimension(1) && (f = e_f(e, j)) >= 0; j++)
      {
        const double coef = is_vit && ppf ? (*ppf)(f) / (*ppe)(e) : 1.0;
        for (r = 0; r < dimension; r++) val(e, r) += fs(f) / ve(e) * (xv(f, r) - xp(e, r)) * (e == f_e(f, 0) ? 1 : -1) * inco(f) * coef;
      }
}

/* vitesse aux elements sur une liste d'elements */
void Champ_Face_PolyMAC::interp_ve(const DoubleTab& inco, const IntVect& les_polys, DoubleTab& val, bool is_vit) const
{
  const Zone_PolyMAC& zone = zone_PolyMAC();
  const DoubleTab& xv = zone.xv(), &xp = zone.xp();
  const DoubleVect& fs = zone.face_surfaces(), *ppf = mon_equation_non_nul() ? &equation().milieu().porosite_face() : NULL, *ppe = ppf ? &equation().milieu().porosite_elem() : NULL, &ve = zone.volumes();
  const IntTab& e_f = zone.elem_faces(), &f_e = zone.face_voisins();
  int e, f, j, d, D = dimension, n, N = inco.line_size();
  assert(ve.line_size() == N * D);

  for (int poly = 0; poly < les_polys.size(); poly++)
    if ((e = les_polys(poly)) != -1)
      {
        for (n = 0; n < N * D; n++) val(e, n) = 0;
        for (j = 0; j < e_f.dimension(1) && (f = e_f(e, j)) >= 0; j++)
          {
            const double coef = is_vit && ppf ? (*ppf)(f) / (*ppe)(e) : 1.0;
            for (d = 0; d < D; d++)
              for (n = 0; n < N; n++) val(e, N * d + n) += fs(f) / ve(e) * (xv(f, d) - xp(e, d)) * (e == f_e(f, 0) ? 1 : -1) * inco(f, n) * coef;
          }
      }
}

DoubleTab& Champ_Face_PolyMAC::valeur_aux_elems(const DoubleTab& positions, const IntVect& les_polys, DoubleTab& val_elem) const
{
  return valeur_aux_elems_(le_champ().valeurs(), positions, les_polys, val_elem);
}

DoubleTab& Champ_Face_PolyMAC::valeur_aux_elems_passe(const DoubleTab& positions, const IntVect& les_polys, DoubleTab& val_elem) const
{
  return valeur_aux_elems_(le_champ().passe(), positions, les_polys, val_elem);
}

DoubleTab& Champ_Face_PolyMAC::valeur_aux_elems_(const DoubleTab& val_face ,const DoubleTab& positions, const IntVect& les_polys, DoubleTab& val_elem) const
{
  const Champ_base& cha=le_champ();
  int nb_compo=cha.nb_comp(), N = val_face.line_size(), D = dimension;
  assert(val_elem.line_size() == nb_compo * N);
  // XXX : TODO Check this assert (positions and not val)
  assert((positions.dimension(0) == les_polys.size())||(positions.dimension_tot(0) == les_polys.size()));
  // assert((val.dimension(0) == les_polys.size())||(val.dimension_tot(0) == les_polys.size()));

  if (val_elem.nb_dim() > 2)
    {
      Cerr << "Erreur TRUST dans Champ_Face_PolyMAC::valeur_aux_elems()" << finl;
      Cerr << "Le DoubleTab val a plus de 2 entrees" << finl;
      Process::exit();
    }

  if (nb_compo == 1)
    {
      Cerr<<"Champ_Face_PolyMAC::valeur_aux_elems"<<finl;
      Cerr <<"A scalar field cannot be of Champ_Face type." << finl;
      Process::exit();
    }

  //on interpole ve sur tous les elements, puis on se restreint a les_polys
  DoubleTrav ve(0, N * D);
  zone_PolyMAC().zone().domaine().creer_tableau_elements(ve);
  bool is_vit = cha.le_nom().debute_par("vitesse") && !cha.le_nom().debute_par("vitesse_debitante");
  interp_ve(val_face, ve, is_vit);
  for (int p = 0; p < les_polys.size(); p++)
    for (int r = 0, e = les_polys(p); e < zone_PolyMAC().nb_elem() && r < N * D; r++) val_elem(p, r) = (e==-1) ? 0. : ve(e, r);
  return val_elem;
}

DoubleVect& Champ_Face_PolyMAC::valeur_aux_elems_compo(const DoubleTab& positions, const IntVect& polys, DoubleVect& val, int ncomp) const
{
  const Champ_base& cha=le_champ();
  assert(val.size() == polys.size());

  //on interpole ve sur tous les elements, puis on se restreint a les_polys
  DoubleTrav ve(0, dimension * cha.valeurs().line_size());
  zone_PolyMAC().zone().domaine().creer_tableau_elements(ve);
  interp_ve(cha.valeurs(), ve);

  for (int p = 0; p < polys.size(); p++) val(p) = (polys(p) == -1) ? 0. : ve(polys(p), ncomp);

  return val;
}

DoubleTab& Champ_Face_PolyMAC::remplir_coord_noeuds(DoubleTab& positions) const
{

  throw;
  // return Champ_implementation_RT0::remplir_coord_noeuds(positions);
}

int Champ_Face_PolyMAC::remplir_coord_noeuds_et_polys(DoubleTab& positions, IntVect& polys) const
{

  throw;
  //  return Champ_implementation_RT0::remplir_coord_noeuds_et_polys(positions,polys);
}

DoubleTab& Champ_Face_PolyMAC::valeur_aux_faces(DoubleTab& val) const
{
  const Champ_base& cha=le_champ();
  int nb_compo=cha.nb_comp(), n, N = cha.valeurs().line_size(), d, D = dimension;

  if (nb_compo == 1)
    {
      Cerr<<"Champ_Face_PolyMAC::valeur_aux_faces"<<finl;
      Cerr <<"A scalar field cannot be of Champ_Face type." << finl;
      Process::exit();
    }

  const Zone_PolyMAC& zone = zone_PolyMAC();
  val.resize(zone.nb_faces(), N * D);

  for (int f = 0; f < zone.nb_faces(); f++)
    for (d = 0; d < D; d++)
      for (n = 0; n < N; n++) val(f, N * d + n) = cha.valeurs()(f, n) * zone.face_normales(f, d) / zone.face_surfaces(f);
  return val;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//Methode qui renvoie SMA_barre aux elements a partir de la vitesse aux faces
//SMA_barre = Sij*Sij (sommation sur les indices i et j)
////////////////////////////////////////////////////////////////////////////////////////////////////

DoubleTab& Champ_Face_PolyMAC::trace(const Frontiere_dis_base& fr, DoubleTab& x, double t, int distant) const
{
  assert(distant==0);
  const bool vectoriel = (le_champ().nb_comp() > 1);
  const DoubleTab& val = valeurs(t);
  int n, N = val.line_size(), d, D = dimension, dim = vectoriel ? D : 1;
  const Front_VF& fr_vf = ref_cast(Front_VF, fr);
  const IntTab& face_voisins = zone_PolyMAC().face_voisins();
  DoubleTrav ve(0, N * D);
  if (vectoriel)
    {
      zone_PolyMAC().zone().domaine().creer_tableau_elements(ve);
      interp_ve(val, ve);
    }

  for (int i = 0; i < fr_vf.nb_faces(); i++)
    {
      const int face = fr_vf.num_premiere_face() + i;
      for (int dir = 0; dir < 2; dir++)
        {
          const int elem = face_voisins(face, dir);
          if (elem != -1)
            {
              for (d = 0; d < dim; d++)
                for (n = 0; n < N; n++) x(i, N * d + n) = vectoriel ? ve(elem, N * d + n) : val(face, n);
            }
        }
    }

  return x;
}

int Champ_Face_PolyMAC::nb_valeurs_nodales() const
{
  return zone_PolyMAC().nb_faces(); //on ignore les variables auxiliaires
}
