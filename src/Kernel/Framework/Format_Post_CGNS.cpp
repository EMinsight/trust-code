/****************************************************************************
* Copyright (c) 2024, CEA
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

#include <Format_Post_CGNS.h>
#include <TRUST_2_CGNS.h>
#include <Param.h>

#ifdef HAS_CGNS
#define CGNS_STR_SIZE 32
#endif

Implemente_instanciable_sans_constructeur(Format_Post_CGNS, "Format_Post_CGNS", Format_Post_base);

Format_Post_CGNS::Format_Post_CGNS()
{
  reset();
}

void Format_Post_CGNS::reset()
{
  cgns_basename_ = "??";
}

Sortie& Format_Post_CGNS::printOn(Sortie& os) const
{
  Process::exit("Format_Post_CGNS::printOn - Should not be called ! ");
  return os;
}

Entree& Format_Post_CGNS::readOn(Entree& is)
{
  verify_if_cgns(__func__);
  return Format_Post_base::readOn(is);
}

void Format_Post_CGNS::set_param(Param& param)
{
  verify_if_cgns(__func__);
  param.ajouter("nom_fichier", &cgns_basename_, Param::REQUIRED);
}

int Format_Post_CGNS::initialize_by_default(const Nom& file_basename)
{
  verify_if_cgns(__func__);
  cgns_basename_ = file_basename;
  return 1;
}

int Format_Post_CGNS::initialize(const Nom& file_basename, const int format, const Nom& option_para)
{
  verify_if_cgns(__func__);
  cgns_basename_ = file_basename;
  return 1;
}

int Format_Post_CGNS::ecrire_entete(const double temps_courant,const int reprise,const int est_le_premier_post)
{
  verify_if_cgns(__func__);
#ifdef HAS_CGNS
  if (est_le_premier_post)
    {
      std::string fn = cgns_basename_.getString() + ".cgns"; // file name
      if (cg_open(fn.c_str(), CG_MODE_WRITE, &fileId_)) cg_error_exit();
      Cerr << "**** CGNS file " << fn << " opened !" << finl;
    }
#endif
  return 1;
}

int Format_Post_CGNS::ecrire_temps(const double t)
{
#ifdef HAS_CGNS
  time_post_.push_back(t);
  flowId_elem_++, flowId_som_++;
  fieldId_elem_ = 0, fieldId_som_ = 0; // reset
  solname_elem_written_ = false, solname_som_written_ = false;
#endif
  return 1;
}

int Format_Post_CGNS::finir(const int est_le_dernier_post)
{
#ifdef HAS_CGNS
  if (est_le_dernier_post)
    {
      assert(static_cast<int>(baseId_.size()) == static_cast<int>(zoneId_.size()));
      const int nsteps = static_cast<int>(time_post_.size());

      std::vector<int> ind_doms_dumped;

      /* 1 : on iter juste sur le map fld_loc_map_; ie: pas domaine dis ... */
      for (auto& itr : fld_loc_map_)
        {
          const std::string& LOC = itr.first;
          const Nom& nom_dom = itr.second;
          const int ind = get_index_nom_vector(doms_written_, nom_dom);
          ind_doms_dumped.push_back(ind);
          assert (ind > -1);

          /* create BaseIterativeData */
          cg_biter_write(fileId_, baseId_[ind], "TimeIterValues", nsteps);

          /* go to BaseIterativeData level and write time values */
          cg_goto(fileId_, baseId_[ind], "BaseIterativeData_t", 1, "end");

          cgsize_t nuse = static_cast<cgsize_t>(nsteps);
          cg_array_write("TimeValues", CGNS_ENUMV(RealDouble), 1, &nuse, time_post_.data());

          /* create ZoneIterativeData */
          cg_ziter_write(fileId_, baseId_[ind], zoneId_[ind], "ZoneIterativeData");
          cg_goto(fileId_, baseId_[ind], "Zone_t", zoneId_[ind], "ZoneIterativeData_t", 1, "end");

          cgsize_t idata[2];
          idata[0] = CGNS_STR_SIZE;
          idata[1] = nsteps;
          if (LOC == "SOM")
            cg_array_write("FlowSolutionPointers", CGNS_ENUMV(Character), 2, idata, solname_som_.c_str());
          else
            cg_array_write("FlowSolutionPointers", CGNS_ENUMV(Character), 2, idata, solname_elem_.c_str());

          cg_simulation_type_write(fileId_, baseId_[ind], CGNS_ENUMV(TimeAccurate));
        }

      /* 2 : on iter sur les autres domaines; ie: domaine dis */
      for (int i = 0; i < static_cast<int>(doms_written_.size()); i++)
        {
          if (std::find(ind_doms_dumped.begin(), ind_doms_dumped.end(), i) == ind_doms_dumped.end()) // indice pas dans ind_doms_dumped
            {
              const Nom& nom_dom = doms_written_[i];
              const int ind = get_index_nom_vector(doms_written_, nom_dom);
              assert (ind > -1);

              /* create BaseIterativeData */
              cg_biter_write(fileId_, baseId_[ind], "TimeIterValues", nsteps);

              /* go to BaseIterativeData level and write time values */
              cg_goto(fileId_, baseId_[ind], "BaseIterativeData_t", 1, "end");

              cgsize_t nuse = static_cast<cgsize_t>(nsteps);
              cg_array_write("TimeValues", CGNS_ENUMV(RealDouble), 1, &nuse, time_post_.data());

              /* create ZoneIterativeData */
              cg_ziter_write(fileId_, baseId_[ind], zoneId_[ind], "ZoneIterativeData");
              cg_goto(fileId_, baseId_[ind], "Zone_t", zoneId_[ind], "ZoneIterativeData_t", 1, "end");

              cg_simulation_type_write(fileId_, baseId_[ind], CGNS_ENUMV(TimeAccurate));
            }
          else { /* Do Nothing */ }
        }

      /* 3 : close cgns file */
      std::string fn = cgns_basename_.getString() + ".cgns"; // file name
      cg_close (fileId_);
      Cerr << "**** CGNS file " << fn << " closed !" << finl;
    }
#endif

  return 1;
}

int Format_Post_CGNS::ecrire_domaine(const Domaine& domaine, const int est_le_premier_post)
{
#ifdef HAS_CGNS
  ecrire_domaine_(domaine, domaine.le_nom());

  // Si on a des frontieres domaine, on les ecrit egalement
  const LIST(REF(Domaine))& bords = domaine.domaines_frontieres();
  for (auto& itr : bords) ecrire_domaine(itr.valeur(), est_le_premier_post);
#endif
  return 1;
}

int Format_Post_CGNS::ecrire_champ(const Domaine& domaine, const Noms& unite_, const Noms& noms_compo, int ncomp, double temps,
                                   const Nom& id_du_champ, const Nom& id_du_domaine, const Nom& localisation,
                                   const Nom& nature, const DoubleTab& valeurs)
{
#ifdef HAS_CGNS
  const std::string LOC = Motcle(localisation).getString();
  /* 1 : if first time called ... build different supports for mixed locations */
  if (static_cast<int>(time_post_.size()) == 1)
    {
      if (static_cast<int>(fld_loc_map_.size()) == 0)
        fld_loc_map_.insert( { LOC , domaine.le_nom() });/* ici on utilise le 1er support */
      else
        {
          const bool in_map = (fld_loc_map_.count(LOC) != 0);
          if (!in_map) // XXX here we need a new support ... sorry
            {
              Nom nom_dom = domaine.le_nom();
              nom_dom += "_";
              nom_dom += LOC;
              Cerr << "Building new CGNS zone to host the field located at : " << LOC << " !" << finl;
              ecrire_domaine_(domaine, nom_dom);
              fld_loc_map_.insert( { LOC, nom_dom } );
            }
        }
    }

  /* 2 : on ecrit */
  const int nb_cmp = valeurs.dimension(1);
  for (int i = 0; i < nb_cmp; i++)
    ecrire_champ_(i /* compo */, temps, nb_cmp > 1 ? Motcle(noms_compo[i]) : id_du_champ, localisation, fld_loc_map_.at(LOC),  valeurs);
#endif
  return 1;
}

#ifdef HAS_CGNS
void Format_Post_CGNS::ecrire_domaine_(const Domaine& domaine, const Nom& nom_dom)
{
  /* 1 : Instance of TRUST_2_CGNS */
  TRUST_2_CGNS TRUST2CGNS;
  TRUST2CGNS.associer_domaine_TRUST(domaine);
  doms_written_.push_back(nom_dom);

  Motcle type_elem = domaine.type_elem().valeur().que_suis_je();
  CGNS_TYPE cgns_type_elem = TRUST2CGNS.convert_elem_type(type_elem);

  /* 2 : Fill coords */
  std::vector<double> xCoords, yCoords, zCoords;
  TRUST2CGNS.fill_coords(xCoords, yCoords, zCoords);

  const int dim = domaine.les_sommets().dimension(1), nb_som = domaine.nb_som(), nb_elem = domaine.nb_elem();
  const int icelldim = dim, iphysdim = Objet_U::dimension;
  int coordsId;

  /* 3 : Base write */
  baseId_.push_back(-123); // pour chaque dom, on a une baseId
  char basename[CGNS_STR_SIZE];
  strcpy(basename, nom_dom.getChar()); // dom name
  cg_base_write(fileId_, basename, icelldim, iphysdim, &baseId_.back());

  /* 4 : Vertex, cell & boundary vertex sizes */
  cgsize_t isize[3][1];
  isize[0][0] = nb_som;
  isize[1][0] = nb_elem;
  isize[2][0] = 0; /* boundary vertex size (zero if elements not sorted) */

  /* 5 : Create zone */
  zoneId_.push_back(-123);
  cg_zone_write(fileId_, baseId_.back(), basename /* Dom name */, isize[0], CGNS_ENUMV(Unstructured), &zoneId_.back());

  /* 6 : Write grid coordinates */
  cg_coord_write(fileId_, baseId_.back(), zoneId_.back(), CGNS_ENUMV(RealDouble), "CoordinateX", xCoords.data(), &coordsId);
  cg_coord_write(fileId_, baseId_.back(), zoneId_.back(), CGNS_ENUMV(RealDouble), "CoordinateY", yCoords.data(), &coordsId);
  if (dim > 2) cg_coord_write(fileId_, baseId_.back(), zoneId_.back(), CGNS_ENUMV(RealDouble), "CoordinateZ", zCoords.data(), &coordsId);

  /* 7 : Set element connectivity */
  std::vector<cgsize_t> elems;
  cgsize_t start = 1, end;
  int nsom = TRUST2CGNS.convert_connectivity(cgns_type_elem, elems);
  end = start + static_cast<cgsize_t>(elems.size()) / nsom - 1;

  /* 8 : Write domaine */
  int sectionId;
  cg_section_write(fileId_, baseId_.back(), zoneId_.back(), "Elem", cgns_type_elem, start, end, 0, elems.data(), &sectionId);
}

int Format_Post_CGNS::get_index_nom_vector(const std::vector<Nom>& vect, const Nom& nom)
{
  int ind = -1;
  auto it = find(vect.begin(), vect.end(), nom);

  if (it != vect.end()) // element found
    ind = it - vect.begin(); // XXX sinon utilse std::distance ...

  return ind;
}

void Format_Post_CGNS::ecrire_champ_(const int comp, const double temps, const Nom& id_du_champ, const Nom& localisation, const Nom& nom_dom,const DoubleTab& valeurs)
{
  std::string LOC = Motcle(localisation).getString();

  if (LOC == "FACES")
    {
      Cerr << "FACES FIELDS ARE NOT YET TREATED ... " << finl;
//      throw;
      return;
    }

  /* 1 : Increment fieldIds */
  if (LOC == "SOM")
    fieldId_som_++;
  else // ELEM // TODO FIXME FACES
    fieldId_elem_++;

  /* 2 : Get corresponding domain index */
  const int ind = get_index_nom_vector(doms_written_, nom_dom);
  assert (ind > -1);

  /* 3 : Write solution names for iterative data later */
  if (!solname_som_written_ && LOC == "SOM")
    {
      std::string solname = "FlowSolution" + std::to_string(temps) + "_" + LOC;
      solname.resize(CGNS_STR_SIZE, ' ');
      solname_som_ += solname;
      cg_sol_write(fileId_, baseId_[ind], zoneId_[ind], solname.c_str(), CGNS_ENUMV(Vertex), &flowId_som_);
      solname_som_written_ = true;
    }

  if (!solname_elem_written_ && LOC == "ELEM")
    {
      std::string solname = "FlowSolution" + std::to_string(temps) + "_" + LOC;
      solname.resize(CGNS_STR_SIZE, ' ');
      solname_elem_ += solname;
      cg_sol_write(fileId_, baseId_[ind], zoneId_[ind], solname.c_str(), CGNS_ENUMV(CellCenter), &flowId_elem_);
      solname_elem_written_ = true;
    }

  /* 4 : Fill field values & dump to cgns file */
  if (valeurs.dimension(1) == 1) /* No stride ! */
    {
      if (LOC == "SOM")
        cg_field_write(fileId_, baseId_[ind], zoneId_[ind], flowId_som_, CGNS_ENUMV(RealDouble), id_du_champ.getChar(), valeurs.addr(), &fieldId_som_);
      else // ELEM // TODO FIXME FACES
        cg_field_write(fileId_, baseId_[ind], zoneId_[ind], flowId_elem_, CGNS_ENUMV(RealDouble), id_du_champ.getChar(), valeurs.addr(), &fieldId_elem_);
    }
  else
    {
      std::vector<double> field_cgns; /* XXX TODO Elie Saikali : try DoubleTrav with addr() later ... mais je pense pas :p */
      for (int i = 0; i < valeurs.dimension(0); i++)
        field_cgns.push_back(valeurs(i, comp));

      if (LOC == "SOM")
        cg_field_write(fileId_, baseId_[ind], zoneId_[ind], flowId_som_, CGNS_ENUMV(RealDouble), id_du_champ.getChar(), field_cgns.data(), &fieldId_som_);
      else // ELEM // TODO FIXME FACES
        cg_field_write(fileId_, baseId_[ind], zoneId_[ind], flowId_elem_, CGNS_ENUMV(RealDouble), id_du_champ.getChar(), field_cgns.data(), &fieldId_elem_);
    }
}
#endif
