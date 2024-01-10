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
#include <Comm_Group_MPI.h>
#include <TRUST_2_CGNS.h>
#include <Option_CGNS.h>
#include <Param.h>

#ifdef HAS_CGNS
#define CGNS_STR_SIZE 32
#define CGNS_DOUBLE_TYPE Option_CGNS::SINGLE_PRECISION>0?CGNS_ENUMV(RealSingle):CGNS_ENUMV(RealDouble)
#endif

#ifdef MPI_
#ifdef INT_is_64_
#define MPI_ENTIER MPI_LONG
#else
#define MPI_ENTIER MPI_INT
#endif
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
      if (is_parallel())
        {
          cgp_mpi_comm(Comm_Group_MPI::get_trio_u_world()); // initialise MPI_COMM_WORLD

          /*
           * Elie Saikali XXX NOTA BENE XXX
           * We need sometimes to write planes, boundaries where some processors have no elements/vertices ...
           * This wont work because the default PIO mode of CGNS is COLLECTIVE. We want it to be INDEPENDENT !
           */
          if (cgp_pio_mode((CGNS_ENUMT(PIOmode_t)) CGP_INDEPENDENT) != CG_OK)
            Cerr << "Error Format_Post_CGNS::ecrire_entete : cgp_pio_mode !" << finl, cgp_error_exit();

          if (cgp_open(fn.c_str(), CG_MODE_WRITE, &fileId_) != CG_OK)
            Cerr << "Error Format_Post_CGNS::ecrire_entete : cgp_open !" << finl, cgp_error_exit();

          Cerr << "**** Parallel CGNS file " << fn << " opened !" << finl;
        }
      else
        {
          if (cg_open(fn.c_str(), CG_MODE_WRITE, &fileId_) != CG_OK)
            Cerr << "Error Format_Post_CGNS::ecrire_entete : cg_open !" << finl, cg_error_exit();

          Cerr << "**** CGNS file " << fn << " opened !" << finl;
        }
    }
#endif
  return 1;
}

int Format_Post_CGNS::ecrire_temps(const double t)
{
#ifdef HAS_CGNS
  time_post_.push_back(t); // add time_post
  flowId_elem_++, flowId_som_++; // increment
  fieldId_elem_ = 0, fieldId_som_ = 0; // reset
  solname_elem_written_ = false, solname_som_written_ = false; // reset
#endif
  return 1;
}

int Format_Post_CGNS::finir(const int est_le_dernier_post)
{
#ifdef HAS_CGNS
  if (est_le_dernier_post)
    {
      if (is_parallel()) finir_par_();
      else finir_();
    }
#endif
  return 1;
}

int Format_Post_CGNS::ecrire_domaine(const Domaine& domaine, const int est_le_premier_post)
{
#ifdef HAS_CGNS
  if (is_parallel())
    ecrire_domaine_par_(domaine, domaine.le_nom());
  else
    ecrire_domaine_(domaine, domaine.le_nom());

  // Si on a des frontieres domaine, on les ecrit egalement
  const LIST(REF(Domaine)) &bords = domaine.domaines_frontieres();
  for (auto &itr : bords) ecrire_domaine(itr.valeur(), est_le_premier_post);

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
              is_parallel() ? ecrire_domaine_par_(domaine, nom_dom) : ecrire_domaine_(domaine, nom_dom); // XXX Attention
              fld_loc_map_.insert( { LOC, nom_dom } );
            }
        }
    }

  /* 2 : on ecrit */
  const int nb_cmp = valeurs.dimension(1);

  if (is_parallel())
    for (int i = 0; i < nb_cmp; i++)
      ecrire_champ_par_(i /* compo */, temps, nb_cmp > 1 ? Motcle(noms_compo[i]) : id_du_champ, id_du_domaine, localisation, fld_loc_map_.at(LOC), valeurs);
  else
    for (int i = 0; i < nb_cmp; i++)
      ecrire_champ_(i /* compo */, temps, nb_cmp > 1 ? Motcle(noms_compo[i]) : id_du_champ, id_du_domaine, localisation, fld_loc_map_.at(LOC), valeurs);
#endif
  return 1;
}

/*
 * ****************************************
 * METHODES PRIVEES CLASSE Format_Post_CGNS
 * ****************************************
 */

#ifdef HAS_CGNS

int Format_Post_CGNS::get_index_nom_vector(const std::vector<Nom>& vect, const Nom& nom)
{
  int ind = -1;
  auto it = find(vect.begin(), vect.end(), nom);

  if (it != vect.end()) // element found
    ind = it - vect.begin(); // XXX sinon utilse std::distance ...

  return ind;
}

/*
 * *********************
 * Pour ecriture domaine
 * *********************
 */
void Format_Post_CGNS::ecrire_domaine_par_(const Domaine& domaine, const Nom& nom_dom)
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

  const int icelldim = domaine.les_sommets().dimension(1), iphysdim = Objet_U::dimension;
  const int nb_som = domaine.nb_som(), nb_elem = domaine.nb_elem();
  const int nb_procs = Process::nproc(), proc_me = Process::me();

  /* 3 : Base write */
  baseId_.push_back(-123); // pour chaque dom, on a une baseId
  char basename[CGNS_STR_SIZE];
  strcpy(basename, nom_dom.getChar()); // dom name

  if (cg_base_write(fileId_, basename, icelldim, iphysdim, &baseId_.back()) != CG_OK)
    Cerr << "Error Format_Post_CGNS::ecrire_domaine_par_ : cg_base_write !" << finl, cgp_error_exit();

  /* 4 : We need global nb_elems/nb_soms => MPI_Allgather. Thats the only information required ! */
  std::vector<int> global_nb_elem, global_nb_som;
  global_nb_elem.assign(nb_procs, -123 /* default */);
  global_nb_som.assign(nb_procs, -123 /* default */);

#ifdef MPI_
//  grp.all_gather(&nb_elem, global_nb_elem.data(), 1); // Elie : pas MPI_CHAR desole
  MPI_Allgather(&nb_elem, 1, MPI_ENTIER, global_nb_elem.data(), 1, MPI_ENTIER, MPI_COMM_WORLD);
  MPI_Allgather(&nb_som, 1, MPI_ENTIER, global_nb_som.data(), 1, MPI_ENTIER, MPI_COMM_WORLD);
#endif

  global_nb_elem_.push_back(global_nb_elem); // XXX

  /* 5 : CREATION OF FILE STRUCTURE : zones, coords & sections
   *
   *  - All processors THAT HAVE nb_elem > 0 write the same information.
   *  - Only zone meta-data is written to the library at this stage ... So no worries ^^
   */
  std::vector<int> coordsIdx, coordsIdy, coordsIdz, sectionId, sectionId2, proc_non_zero_elem;
  std::string zonename;
  int gridId;

  const auto min_nb_elem = std::min_element(global_nb_elem.begin(), global_nb_elem.end());
  int nb_zones_to_write = nb_procs;

  if (*min_nb_elem <= 0) // not all procs will write !
    {
      // remplir proc_non_zero_elem avec le numero de proc si nb_elem > 0 !!
      for (int i = 0; i < static_cast<int>(global_nb_elem.size()); i++)
        if (global_nb_elem[i] > 0) proc_non_zero_elem.push_back(i);

      nb_zones_to_write = static_cast<int>(proc_non_zero_elem.size());
    }

  proc_non_zero_write_.push_back(proc_non_zero_elem); // XXX
  const bool all_write = proc_non_zero_elem.empty(); // all procs will write !
  const bool is_polyedre = (type_elem == "POLYEDRE" || type_elem == "PRISME" || type_elem == "PRISME_HEXAG");

  /* ces vecteurs restes vide sauf si poly */
  std::vector<cgsize_t> sf, sf_offset, ef, ef_offset;
  std::vector<int> global_nb_sf, global_nb_ef, global_nb_sf_offset, global_nb_ef_offset;
  int nb_sf = -123, nb_sf_offset = -123, nb_ef = -123, nb_ef_offset = -123;

  if (cgns_type_elem == CGNS_ENUMV(NGON_n)) // cas polyedre
    {
      nb_sf = TRUST2CGNS.convert_connectivity_ngon(sf, sf_offset, is_polyedre);
      nb_sf_offset = static_cast<int>(sf.size());

      global_nb_sf.assign(nb_procs, -123 /* default */);
      global_nb_sf_offset.assign(nb_procs, -123 /* default */);

#ifdef MPI_
//  grp.all_gather(&nb_elem, global_nb_elem.data(), 1); // Elie : pas MPI_CHAR desole
      MPI_Allgather(&nb_sf, 1, MPI_ENTIER, global_nb_sf.data(), 1, MPI_ENTIER, MPI_COMM_WORLD);
      MPI_Allgather(&nb_sf_offset, 1, MPI_ENTIER, global_nb_sf_offset.data(), 1, MPI_ENTIER, MPI_COMM_WORLD);
#endif

      if (is_polyedre) // Pas pour polygone
        {
          nb_ef = TRUST2CGNS.convert_connectivity_nface(ef, ef_offset);
          nb_ef_offset = static_cast<int>(ef.size());

          global_nb_ef.assign(nb_procs, -123 /* default */);
          global_nb_ef_offset.assign(nb_procs, -123 /* default */);

#ifdef MPI_
//  grp.all_gather(&nb_elem, global_nb_elem.data(), 1); // Elie : pas MPI_CHAR desole
          MPI_Allgather(&nb_ef, 1, MPI_ENTIER, global_nb_ef.data(), 1, MPI_ENTIER, MPI_COMM_WORLD);
          MPI_Allgather(&nb_ef_offset, 1, MPI_ENTIER, global_nb_ef_offset.data(), 1, MPI_ENTIER, MPI_COMM_WORLD);
#endif
        }
    }

  // on boucle seulement sur les procs qui n'ont pas des nb_elem 0
  zoneId_.clear(); // XXX commencons par ca
  for (int i = 0; i != nb_zones_to_write; i++)
    {
      const int indZ = all_write ? i : proc_non_zero_elem[i]; // procID
      const int ne_loc = global_nb_elem[indZ], ns_loc = global_nb_som[indZ]; /* nb_elem & nb_som local */
      assert (ne_loc > 0);

      cgsize_t start = 1, end = ne_loc;
      cgsize_t isize[3][1];
      isize[0][0] = ns_loc;
      isize[1][0] = end;
      isize[2][0] = 0; /* boundary vertex size (zero if elements not sorted) */

      zoneId_.push_back(-123);
      zonename = nom_dom.getString() + "_" + std::to_string(indZ);
      zonename.resize(CGNS_STR_SIZE, ' ');

      /* 5.1 : Create zone */
      if (cg_zone_write(fileId_, baseId_.back(), zonename.c_str(), isize[0], CGNS_ENUMV(Unstructured), &zoneId_.back()) != CG_OK)
        Cerr << "Error Format_Post_CGNS::ecrire_domaine_par_ : cg_zone_write !" << finl, cgp_error_exit();

      if (cg_grid_write(fileId_, baseId_.back(), zoneId_.back(), "GridCoordinates", &gridId) != CG_OK)
        Cerr << "Error Format_Post_CGNS::ecrire_domaine_par_ : cg_grid_write !" << finl, cgp_error_exit();

      /* 5.2 : Construct the grid coordinates nodes */
      coordsIdx.push_back(-123), coordsIdy.push_back(-123);
      if (cgp_coord_write(fileId_, baseId_.back(), zoneId_.back(), CGNS_DOUBLE_TYPE, "CoordinateX", &coordsIdx.back()) != CG_OK)
        Cerr << "Error Format_Post_CGNS::ecrire_domaine_par_ : cgp_coord_write - X !" << finl, cgp_error_exit();

      if (cgp_coord_write(fileId_, baseId_.back(), zoneId_.back(), CGNS_DOUBLE_TYPE, "CoordinateY", &coordsIdy.back()) != CG_OK)
        Cerr << "Error Format_Post_CGNS::ecrire_domaine_par_ : cgp_coord_write - Y !" << finl, cgp_error_exit();

      if (icelldim > 2)
        {
          coordsIdz.push_back(-123);
          if (cgp_coord_write(fileId_, baseId_.back(), zoneId_.back(), CGNS_DOUBLE_TYPE, "CoordinateZ", &coordsIdz.back()) != CG_OK)
            Cerr << "Error Format_Post_CGNS::ecrire_domaine_par_ : cgp_coord_write - Z !" << finl, cgp_error_exit();
        }

      /* 5.3 : Construct the sections to host connectivity later */
      sectionId.push_back(-123);

      if (cgns_type_elem == CGNS_ENUMV(NGON_n)) // cas polyedre
        {
          end = start + static_cast<cgsize_t>(global_nb_sf[indZ]) -1;
          cgsize_t maxoffset = static_cast<cgsize_t>(global_nb_sf_offset[indZ]);

          if (cgp_poly_section_write(fileId_, baseId_.back(), zoneId_.back(), "NGON_n", CGNS_ENUMV(NGON_n), start, end, maxoffset, 0, &sectionId.back()) != CG_OK)
            Cerr << "Error Format_Post_CGNS::ecrire_domaine_par_ : cgp_poly_section_write !" << finl, cgp_error_exit();

          if (is_polyedre) // Pas pour polygone
            {
              sectionId2.push_back(-123);
              start = end + 1;

              end = start + static_cast<cgsize_t>(global_nb_ef[indZ]) -1;
              maxoffset = static_cast<cgsize_t>(global_nb_ef_offset[indZ]);

              if (cgp_poly_section_write(fileId_, baseId_.back(), zoneId_.back(), "NFACE_n", CGNS_ENUMV(NFACE_n), start, end, maxoffset, 0, &sectionId2.back()) != CG_OK)
                Cerr << "Error Format_Post_CGNS::ecrire_domaine_par_ : cgp_poly_section_write !" << finl, cgp_error_exit();
            }
        }
      else
        {
          if (cgp_section_write(fileId_, baseId_.back(), zoneId_.back(), "Elem", cgns_type_elem, start, end, 0, &sectionId.back()) != CG_OK)
            Cerr << "Error Format_Post_CGNS::ecrire_domaine_par_ : cgp_section_write !" << finl, cgp_error_exit();
        }
    }

  zoneId_par_.push_back(zoneId_); // XXX : Dont touch

//  Process::barrier();

  /* 6 : Write grid coordinates & set connectivity */
  if (nb_elem > 0) // this proc will write !
    {
      cgsize_t min = 1, max = nb_som;
      int indx = -123;
      if (all_write) indx = proc_me;
      else
        for (int i = 0; i < nb_zones_to_write; i++)
          if (proc_non_zero_elem[i] == proc_me)
            {
              indx = i;
              break;
            }

      /* 6.1 : Write grid coordinates */
      if (cgp_coord_write_data(fileId_, baseId_.back(), zoneId_par_.back()[indx], coordsIdx[indx], &min, &max, xCoords.data()) != CG_OK)
        Cerr << "Error Format_Post_CGNS::ecrire_domaine_par_ : cgp_coord_write_data - X !" << finl, cgp_error_exit();

      if (cgp_coord_write_data(fileId_, baseId_.back(), zoneId_par_.back()[indx], coordsIdy[indx], &min, &max, yCoords.data()) != CG_OK)
        Cerr << "Error Format_Post_CGNS::ecrire_domaine_par_ : cgp_coord_write_data - Y !" << finl, cgp_error_exit();

      if (icelldim > 2)
        if (cgp_coord_write_data(fileId_, baseId_.back(), zoneId_par_.back()[indx], coordsIdz[indx], &min, &max, zCoords.data()) != CG_OK)
          Cerr << "Error Format_Post_CGNS::ecrire_domaine_par_ : cgp_coord_write_data - Z !" << finl, cgp_error_exit();

      /* 6.2 : Set element connectivity */
      if (cgns_type_elem == CGNS_ENUMV(NGON_n)) // cas polyedre
        {
          max = min + nb_sf -1;

          if (cgp_poly_elements_write_data(fileId_, baseId_.back(), zoneId_par_.back()[indx], sectionId[indx], min, max, sf.data(), sf_offset.data()) != CG_OK)
            Cerr << "Error Format_Post_CGNS::ecrire_domaine_par_ : cgp_poly_elements_write_data !" << finl, cgp_error_exit();

          if (is_polyedre) // Pas pour polygone
            {
              min = max + 1;
              max = min + nb_ef -1;

              if (cgp_poly_elements_write_data(fileId_, baseId_.back(), zoneId_par_.back()[indx], sectionId2[indx], min, max, ef.data(), ef_offset.data()) != CG_OK)
                Cerr << "Error Format_Post_CGNS::ecrire_domaine_par_ : cgp_poly_elements_write_data !" << finl, cgp_error_exit();
            }
        }
      else
        {
          std::vector<cgsize_t> elems;
          TRUST2CGNS.convert_connectivity(cgns_type_elem, elems);

          max = nb_elem; /* now we need local elem */
          if (cgp_elements_write_data(fileId_, baseId_.back(), zoneId_par_.back()[indx], sectionId[indx], min, max, elems.data()) != CG_OK)
            Cerr << "Error Format_Post_CGNS::ecrire_domaine_par_ : cgp_elements_write_data !" << finl, cgp_error_exit();
        }
    }
}

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

  const int icelldim = domaine.les_sommets().dimension(1), iphysdim = Objet_U::dimension, nb_som = domaine.nb_som(), nb_elem = domaine.nb_elem();
  int coordsId;

  /* 3 : Base write */
  baseId_.push_back(-123); // pour chaque dom, on a une baseId
  char basename[CGNS_STR_SIZE];
  strcpy(basename, nom_dom.getChar()); // dom name

  if (cg_base_write(fileId_, basename, icelldim, iphysdim, &baseId_.back()) != CG_OK)
    Cerr << "Error Format_Post_CGNS::ecrire_domaine_ : cg_base_write !" << finl, cg_error_exit();

  /* 4 : Vertex, cell & boundary vertex sizes */
  cgsize_t isize[3][1];
  isize[0][0] = nb_som;
  isize[1][0] = nb_elem;
  isize[2][0] = 0; /* boundary vertex size (zero if elements not sorted) */

  /* 5 : Create zone */
  zoneId_.push_back(-123);
  if (cg_zone_write(fileId_, baseId_.back(), basename /* Dom name */, isize[0], CGNS_ENUMV(Unstructured), &zoneId_.back()) != CG_OK)
    Cerr << "Error Format_Post_CGNS::ecrire_domaine_ : cg_zone_write !" << finl, cg_error_exit();

  /* 6 : Write grid coordinates */
  if (cg_coord_write(fileId_, baseId_.back(), zoneId_.back(), CGNS_DOUBLE_TYPE, "CoordinateX", xCoords.data(), &coordsId) != CG_OK)
    Cerr << "Error Format_Post_CGNS::ecrire_domaine_ : cg_coord_write - X !" << finl, cg_error_exit();

  if (cg_coord_write(fileId_, baseId_.back(), zoneId_.back(), CGNS_DOUBLE_TYPE, "CoordinateY", yCoords.data(), &coordsId) != CG_OK)
    Cerr << "Error Format_Post_CGNS::ecrire_domaine_ : cg_coord_write - Y !" << finl, cg_error_exit();

  if (icelldim > 2)
    if (cg_coord_write(fileId_, baseId_.back(), zoneId_.back(), CGNS_DOUBLE_TYPE, "CoordinateZ", zCoords.data(), &coordsId) != CG_OK)
      Cerr << "Error Format_Post_CGNS::ecrire_domaine_ : cg_coord_write - Z !" << finl, cg_error_exit();

  /* 7 : Set element connectivity */
  int sectionId;
  cgsize_t start = 1, end;

  if (cgns_type_elem == CGNS_ENUMV(NGON_n)) // cas polyedre
    {
      const bool is_polyedre = (type_elem == "POLYEDRE" || type_elem == "PRISME" || type_elem == "PRISME_HEXAG");
      std::vector<cgsize_t> sf, sf_offset;

      end = start + static_cast<cgsize_t>(TRUST2CGNS.convert_connectivity_ngon(sf, sf_offset, is_polyedre)) -1;

      if (cg_poly_section_write(fileId_, baseId_.back(), zoneId_.back(), "NGON_n", CGNS_ENUMV(NGON_n), start, end, 0, sf.data(), sf_offset.data(), &sectionId))
        cg_error_exit();

      if (is_polyedre) // Pas pour polygone
        {
          std::vector<cgsize_t> ef, ef_offset;

          start = end + 1;
          end = start + static_cast<cgsize_t>(TRUST2CGNS.convert_connectivity_nface(ef, ef_offset)) -1;

          if (cg_poly_section_write(fileId_, baseId_.back(), zoneId_.back(), "NFACE_n", CGNS_ENUMV(NFACE_n), start, end, 0, ef.data(), ef_offset.data(), &sectionId))
            cg_error_exit();
        }
    }
  else
    {
      std::vector<cgsize_t> elems;

      int nsom = TRUST2CGNS.convert_connectivity(cgns_type_elem, elems);
      end = start + static_cast<cgsize_t>(elems.size()) / nsom - 1;

      if (cg_section_write(fileId_, baseId_.back(), zoneId_.back(), "Elem", cgns_type_elem, start, end, 0, elems.data(), &sectionId) != CG_OK)
        Cerr << "Error Format_Post_CGNS::ecrire_domaine_ : cg_section_write !" << finl, cg_error_exit();
    }
}

/*
 * ********************
 * Pour ecriture champs
 * ********************
 */
void Format_Post_CGNS::ecrire_champ_par_(const int comp, const double temps, const Nom& id_du_champ, const Nom& id_du_domaine, const Nom& localisation, const Nom& nom_dom, const DoubleTab& valeurs)
{
  std::string LOC = Motcle(localisation).getString();
  if (LOC == "FACES")
    {
      Cerr << "FACES FIELDS ARE NOT YET TREATED ... " << finl;
//      throw;
      return;
    }

  Motcle id_du_champ_modifie(id_du_champ), iddomaine(id_du_domaine);

  if (LOC == "SOM")
    {
      id_du_champ_modifie.prefix(id_du_domaine);
      id_du_champ_modifie.prefix(iddomaine);
      id_du_champ_modifie.prefix("_SOM_");
    }
  else if (LOC == "ELEM")
    {
      id_du_champ_modifie.prefix(id_du_domaine);
      id_du_champ_modifie.prefix(iddomaine);
      id_du_champ_modifie.prefix("_ELEM_");
    }

  Nom& id_champ = id_du_champ_modifie;

  /* 1 : Increment fieldIds */
  if (LOC == "SOM") fieldId_som_++;
  else // ELEM // TODO FIXME FACES
    fieldId_elem_++;

  /* 2 : Get corresponding domain index */
  const int ind = get_index_nom_vector(doms_written_, nom_dom);
  assert(ind > -1);

  const int nb_procs = Process::nproc(), proc_me = Process::me(), nb_vals = valeurs.dimension(0);

  /* 3 : CREATION OF FILE STRUCTURE
   *
   *  - All processors THAT HAVE nb_vals > 0 write the same information.
   *  - Only field meta-data is written to the library at this stage ... So no worries ^^
   *  - And just once per dt !
   */
  const auto min_nb_elem = std::min_element(global_nb_elem_[ind].begin(), global_nb_elem_[ind].end());
  int nb_zones_to_write = (*min_nb_elem <= 0) ? static_cast<int>(proc_non_zero_write_[ind].size()) : nb_procs;
  const bool all_write = proc_non_zero_write_[ind].empty(); // all procs will write !

  if (!solname_som_written_ && LOC == "SOM")
    {
      std::string solname = "FlowSolution" + std::to_string(temps) + "_" + LOC;
      solname.resize(CGNS_STR_SIZE, ' ');
      solname_som_ += solname;

      // on boucle seulement sur les procs qui n'ont pas des nb_elem 0
      for (int ii = 0; ii != nb_zones_to_write; ii++)
        {
          if (cg_sol_write(fileId_, baseId_[ind], zoneId_par_[ind][ii], solname.c_str(), CGNS_ENUMV(Vertex), &flowId_som_) != CG_OK)
            Cerr << "Error Format_Post_CGNS::ecrire_champ_par_ : cg_sol_write !" << finl, cgp_error_exit();

          if (cg_goto(fileId_, baseId_[ind], "Zone_t", zoneId_par_[ind][ii], "FlowSolution_t", flowId_som_, "end") != CG_OK)
            Cerr << "Error Format_Post_CGNS::ecrire_champ_par_ : cg_goto !" << finl, cgp_error_exit();
        }

      solname_som_written_ = true;
    }

  if (!solname_elem_written_ && LOC == "ELEM")
    {
      std::string solname = "FlowSolution" + std::to_string(temps) + "_" + LOC;
      solname.resize(CGNS_STR_SIZE, ' ');
      solname_elem_ += solname;

      // on boucle seulement sur les procs qui n'ont pas des nb_elem 0
      for (int ii = 0; ii != nb_zones_to_write; ii++)
        {
          if (cg_sol_write(fileId_, baseId_[ind], zoneId_par_[ind][ii], solname.c_str(), CGNS_ENUMV(CellCenter), &flowId_elem_) != CG_OK)
            Cerr << "Error Format_Post_CGNS::ecrire_champ_par_ : cg_sol_write !" << finl, cgp_error_exit();

          if (cg_goto(fileId_, baseId_[ind], "Zone_t", zoneId_par_[ind][ii], "FlowSolution_t", flowId_elem_, "end") != CG_OK)
            Cerr << "Error Format_Post_CGNS::ecrire_champ_par_ : cg_goto !" << finl, cgp_error_exit();
        }

      solname_elem_written_ = true;
    }

  for (int ii = 0; ii != nb_zones_to_write; ii++)
    {
      if (LOC == "SOM")
        {
          if (cgp_field_write(fileId_, baseId_[ind], zoneId_par_[ind][ii], flowId_som_, CGNS_DOUBLE_TYPE, id_champ.getChar(), &fieldId_som_) != CG_OK)
            Cerr << "Error Format_Post_CGNS::ecrire_champ_par_ : cgp_field_write !" << finl, cgp_error_exit();
        }
      else if (LOC == "ELEM")
        {
          if (cgp_field_write(fileId_, baseId_[ind], zoneId_par_[ind][ii], flowId_elem_, CGNS_DOUBLE_TYPE, id_champ.getChar(), &fieldId_elem_) != CG_OK)
            Cerr << "Error Format_Post_CGNS::ecrire_champ_par_ : cgp_field_write !" << finl, cgp_error_exit();
        }
    }

  /* 4 : Fill field values & dump to cgns file */
  if (nb_vals > 0) // this proc will write !
    {
      cgsize_t min = 1, max = nb_vals;
      int indx = -123;
      if (all_write) indx = proc_me;
      else
        for (int i = 0; i < nb_zones_to_write; i++)
          if (proc_non_zero_write_[ind][i] == proc_me)
            {
              indx = i;
              break;
            }

      if (valeurs.dimension(1) == 1) /* No stride ! */
        {
          if (LOC == "SOM")
            {
              if (cgp_field_write_data(fileId_, baseId_[ind], zoneId_par_[ind][indx], flowId_som_, fieldId_som_, &min, &max, valeurs.addr()) != CG_OK)
                Cerr << "Error Format_Post_CGNS::ecrire_champ_par_ : cgp_field_write_data !" << finl, cgp_error_exit();

            }
          else // ELEM // TODO FIXME FACES
            {
              if (cgp_field_write_data(fileId_, baseId_[ind], zoneId_par_[ind][indx], flowId_elem_, fieldId_elem_, &min, &max, valeurs.addr()) != CG_OK)
                Cerr << "Error Format_Post_CGNS::ecrire_champ_par_ : cgp_field_write_data !" << finl, cgp_error_exit();
            }
        }
      else
        {
          std::vector<double> field_cgns; /* XXX TODO Elie Saikali : try DoubleTrav with addr() later ... mais je pense pas :p */
          for (int i = 0; i < nb_vals; i++)
            field_cgns.push_back(valeurs(i, comp));

          if (LOC == "SOM")
            {
              if (cgp_field_write_data(fileId_, baseId_[ind], zoneId_par_[ind][indx], flowId_som_, fieldId_som_, &min, &max, field_cgns.data()) != CG_OK)
                Cerr << "Error Format_Post_CGNS::ecrire_champ_par_ : cgp_field_write_data !" << finl, cgp_error_exit();
            }
          else // ELEM // TODO FIXME FACES
            {
              if (cgp_field_write_data(fileId_, baseId_[ind], zoneId_par_[ind][indx], flowId_elem_, fieldId_elem_, &min, &max, field_cgns.data()) != CG_OK)
                Cerr << "Error Format_Post_CGNS::ecrire_champ_par_ : cgp_field_write_data !" << finl, cgp_error_exit();
            }
        }
    }
}

void Format_Post_CGNS::ecrire_champ_(const int comp, const double temps, const Nom& id_du_champ, const Nom& id_du_domaine, const Nom& localisation, const Nom& nom_dom, const DoubleTab& valeurs)
{
  std::string LOC = Motcle(localisation).getString();

  if (LOC == "FACES")
    {
      Cerr << "FACES FIELDS ARE NOT YET TREATED ... " << finl;
//      throw;
      return;
    }

  Motcle id_du_champ_modifie(id_du_champ), iddomaine(id_du_domaine);

  if (LOC == "SOM")
    {
      id_du_champ_modifie.prefix(id_du_domaine);
      id_du_champ_modifie.prefix(iddomaine);
      id_du_champ_modifie.prefix("_SOM_");
    }
  else if (LOC == "ELEM")
    {
      id_du_champ_modifie.prefix(id_du_domaine);
      id_du_champ_modifie.prefix(iddomaine);
      id_du_champ_modifie.prefix("_ELEM_");
    }

  Nom& id_champ = id_du_champ_modifie;

  /* 1 : Increment fieldIds */
  if (LOC == "SOM") fieldId_som_++;
  else // ELEM // TODO FIXME FACES
    fieldId_elem_++;

  /* 2 : Get corresponding domain index */
  const int ind = get_index_nom_vector(doms_written_, nom_dom);
  assert(ind > -1);

  /* 3 : Write solution names for iterative data later */
  if (!solname_som_written_ && LOC == "SOM")
    {
      std::string solname = "FlowSolution" + std::to_string(temps) + "_" + LOC;
      solname.resize(CGNS_STR_SIZE, ' ');
      solname_som_ += solname;

      if (cg_sol_write(fileId_, baseId_[ind], zoneId_[ind], solname.c_str(), CGNS_ENUMV(Vertex), &flowId_som_) != CG_OK)
        Cerr << "Error Format_Post_CGNS::ecrire_champ_ : cg_sol_write !" << finl, cg_error_exit();

      solname_som_written_ = true;
    }

  if (!solname_elem_written_ && LOC == "ELEM")
    {
      std::string solname = "FlowSolution" + std::to_string(temps) + "_" + LOC;
      solname.resize(CGNS_STR_SIZE, ' ');
      solname_elem_ += solname;

      if (cg_sol_write(fileId_, baseId_[ind], zoneId_[ind], solname.c_str(), CGNS_ENUMV(CellCenter), &flowId_elem_) != CG_OK)
        Cerr << "Error Format_Post_CGNS::ecrire_champ_ : cg_sol_write !" << finl, cg_error_exit();

      solname_elem_written_ = true;
    }

  /* 4 : Fill field values & dump to cgns file */
  if (valeurs.dimension(1) == 1) /* No stride ! */
    {
      if (LOC == "SOM")
        {
          if (cg_field_write(fileId_, baseId_[ind], zoneId_[ind], flowId_som_, CGNS_DOUBLE_TYPE, id_champ.getChar(), valeurs.addr(), &fieldId_som_) != CG_OK)
            Cerr << "Error Format_Post_CGNS::ecrire_champ_ : cg_field_write !" << finl, cg_error_exit();
        }
      else // ELEM // TODO FIXME FACES
        {
          if (cg_field_write(fileId_, baseId_[ind], zoneId_[ind], flowId_elem_, CGNS_DOUBLE_TYPE, id_champ.getChar(), valeurs.addr(), &fieldId_elem_) != CG_OK)
            Cerr << "Error Format_Post_CGNS::ecrire_champ_ : cg_field_write !" << finl, cg_error_exit();
        }
    }
  else
    {
      std::vector<double> field_cgns; /* XXX TODO Elie Saikali : try DoubleTrav with addr() later ... mais je pense pas :p */
      for (int i = 0; i < valeurs.dimension(0); i++)
        field_cgns.push_back(valeurs(i, comp));

      if (LOC == "SOM")
        {
          if (cg_field_write(fileId_, baseId_[ind], zoneId_[ind], flowId_som_, CGNS_DOUBLE_TYPE, id_champ.getChar(), field_cgns.data(), &fieldId_som_) != CG_OK)
            Cerr << "Error Format_Post_CGNS::ecrire_champ_ : cg_field_write !" << finl, cg_error_exit();
        }
      else // ELEM // TODO FIXME FACES
        {
          if (cg_field_write(fileId_, baseId_[ind], zoneId_[ind], flowId_elem_, CGNS_DOUBLE_TYPE, id_champ.getChar(), field_cgns.data(), &fieldId_elem_) != CG_OK)
            Cerr << "Error Format_Post_CGNS::ecrire_champ_ : cg_field_write !" << finl, cg_error_exit();
        }
    }
}

/*
 * *******************
 * Pour ecriture iters
 * *******************
 */
void Format_Post_CGNS::finir_par_()
{
//  assert(static_cast<int>(baseId_.size()) == static_cast<int>(zoneId_.size())); // XXX No not for // !!!
  const int nsteps = static_cast<int>(time_post_.size()), nb_procs = Process::nproc();
  std::vector<int> ind_doms_dumped;

  /* 1 : on iter juste sur le map fld_loc_map_; ie: pas domaine dis ... */
  for (auto &itr : fld_loc_map_)
    {
      const std::string& LOC = itr.first;
      const Nom& nom_dom = itr.second;
      const int ind = get_index_nom_vector(doms_written_, nom_dom);
      ind_doms_dumped.push_back(ind);
      assert(ind > -1);

      const auto min_nb_elem = std::min_element(global_nb_elem_[ind].begin(), global_nb_elem_[ind].end());
      int nb_zones_to_write = (*min_nb_elem <= 0) ? static_cast<int>(proc_non_zero_write_[ind].size()) : nb_procs;

      /* create BaseIterativeData */
      if (cg_biter_write(fileId_, baseId_[ind], "TimeIterValues", nsteps) != CG_OK)
        Cerr << "Error Format_Post_CGNS::finir_par_ : cg_biter_write !" << finl, cgp_error_exit();

      /* go to BaseIterativeData level and write time values */
      if (cg_goto(fileId_, baseId_[ind], "BaseIterativeData_t", 1, "end") != CG_OK)
        Cerr << "Error Format_Post_CGNS::finir_par_ : cg_goto !" << finl, cgp_error_exit();

      cgsize_t nuse = static_cast<cgsize_t>(nsteps);
      if (cg_array_write("TimeValues", CGNS_DOUBLE_TYPE, 1, &nuse, time_post_.data()) != CG_OK)
        Cerr << "Error Format_Post_CGNS::finir_par_ : cg_array_write !" << finl, cgp_error_exit();

      cgsize_t idata[2];
      idata[0] = CGNS_STR_SIZE;
      idata[1] = nsteps;

      // on boucle seulement sur les procs qui n'ont pas des nb_elem 0
      for (int ii = 0; ii != nb_zones_to_write; ii++)
        {
          /* create ZoneIterativeData */
          if (cg_ziter_write(fileId_, baseId_[ind], zoneId_par_[ind][ii], "ZoneIterativeData") != CG_OK)
            Cerr << "Error Format_Post_CGNS::finir_par_ : cg_ziter_write !" << finl, cgp_error_exit();

          if (cg_goto(fileId_, baseId_[ind], "Zone_t", zoneId_par_[ind][ii], "ZoneIterativeData_t", 1, "end") != CG_OK)
            Cerr << "Error Format_Post_CGNS::finir_par_ : cg_goto !" << finl, cgp_error_exit();

          if (LOC == "SOM")
            {
              if (cg_array_write("FlowSolutionPointers", CGNS_ENUMV(Character), 2, idata, solname_som_.c_str()) != CG_OK)
                Cerr << "Error Format_Post_CGNS::finir_par_ : cg_array_write !" << finl, cgp_error_exit();
            }
          else if (LOC == "ELEM")
            {
              if (cg_array_write("FlowSolutionPointers", CGNS_ENUMV(Character), 2, idata, solname_elem_.c_str()) != CG_OK)
                Cerr << "Error Format_Post_CGNS::finir_par_ : cg_array_write !" << finl, cgp_error_exit();
            }
        }

      if (cg_simulation_type_write(fileId_, baseId_[ind], CGNS_ENUMV(TimeAccurate)) != CG_OK)
        Cerr << "Error Format_Post_CGNS::finir_par_ : cg_simulation_type_write !" << finl, cgp_error_exit();
    }

  /* 2 : on iter sur les autres domaines; ie: domaine dis */
  for (int i = 0; i < static_cast<int>(doms_written_.size()); i++)
    {
      if (std::find(ind_doms_dumped.begin(), ind_doms_dumped.end(), i) == ind_doms_dumped.end()) // indice pas dans ind_doms_dumped
        {
          const Nom& nom_dom = doms_written_[i];
          const int ind = get_index_nom_vector(doms_written_, nom_dom);
          assert(ind > -1);

          const auto min_nb_elem = std::min_element(global_nb_elem_[ind].begin(), global_nb_elem_[ind].end());
          int nb_zones_to_write = (*min_nb_elem <= 0) ? static_cast<int>(proc_non_zero_write_[ind].size()) : nb_procs;

          /* create BaseIterativeData */
          if (cg_biter_write(fileId_, baseId_[ind], "TimeIterValues", nsteps) != CG_OK)
            Cerr << "Error Format_Post_CGNS::finir_ : cg_biter_write !" << finl, cg_error_exit();

          /* go to BaseIterativeData level and write time values */
          if (cg_goto(fileId_, baseId_[ind], "BaseIterativeData_t", 1, "end") != CG_OK)
            Cerr << "Error Format_Post_CGNS::finir_ : cg_goto !" << finl, cg_error_exit();

          cgsize_t nuse = static_cast<cgsize_t>(nsteps);
          if (cg_array_write("TimeValues", CGNS_DOUBLE_TYPE, 1, &nuse, time_post_.data()) != CG_OK)
            Cerr << "Error Format_Post_CGNS::finir_ : cg_array_write !" << finl, cg_error_exit();

          // on boucle seulement sur les procs qui n'ont pas des nb_elem 0
          for (int ii = 0; ii != nb_zones_to_write; ii++)
            {
              /* create ZoneIterativeData */
              if (cg_ziter_write(fileId_, baseId_[ind], zoneId_par_[ind][ii], "ZoneIterativeData") != CG_OK)
                Cerr << "Error Format_Post_CGNS::finir_ : cg_ziter_write !" << finl, cg_error_exit();

              if (cg_goto(fileId_, baseId_[ind], "Zone_t", zoneId_par_[ind][ii], "ZoneIterativeData_t", 1, "end") != CG_OK)
                Cerr << "Error Format_Post_CGNS::finir_ : cg_goto !" << finl, cg_error_exit();
            }

          if (cg_simulation_type_write(fileId_, baseId_[ind], CGNS_ENUMV(TimeAccurate)) != CG_OK)
            Cerr << "Error Format_Post_CGNS::finir_ : cg_simulation_type_write !" << finl, cg_error_exit();
        }
      else { /* Do Nothing */ }
    }

//  Process::barrier();
  std::string fn = cgns_basename_.getString() + ".cgns"; // file name
  if (cgp_close (fileId_) != CG_OK)
    Cerr << "Error Format_Post_CGNS::ecrire_domaine_par_ : cgp_close !" << finl, cgp_error_exit();

  Cerr << "**** Parallel CGNS file " << fn << " closed !" << finl;
}

void Format_Post_CGNS::finir_()
{
  assert(static_cast<int>(baseId_.size()) == static_cast<int>(zoneId_.size()));
  const int nsteps = static_cast<int>(time_post_.size());
  std::vector<int> ind_doms_dumped;

  /* 1 : on iter juste sur le map fld_loc_map_; ie: pas domaine dis ... */
  for (auto &itr : fld_loc_map_)
    {
      const std::string& LOC = itr.first;
      const Nom& nom_dom = itr.second;
      const int ind = get_index_nom_vector(doms_written_, nom_dom);
      ind_doms_dumped.push_back(ind);
      assert(ind > -1);

      /* create BaseIterativeData */
      if (cg_biter_write(fileId_, baseId_[ind], "TimeIterValues", nsteps) != CG_OK)
        Cerr << "Error Format_Post_CGNS::finir_ : cg_biter_write !" << finl, cg_error_exit();

      /* go to BaseIterativeData level and write time values */
      if (cg_goto(fileId_, baseId_[ind], "BaseIterativeData_t", 1, "end") != CG_OK)
        Cerr << "Error Format_Post_CGNS::finir_ : cg_goto !" << finl, cg_error_exit();

      cgsize_t nuse = static_cast<cgsize_t>(nsteps);
      if (cg_array_write("TimeValues", CGNS_DOUBLE_TYPE, 1, &nuse, time_post_.data()) != CG_OK)
        Cerr << "Error Format_Post_CGNS::finir_ : cg_array_write !" << finl, cg_error_exit();

      /* create ZoneIterativeData */
      if (cg_ziter_write(fileId_, baseId_[ind], zoneId_[ind], "ZoneIterativeData") != CG_OK)
        Cerr << "Error Format_Post_CGNS::finir_ : cg_ziter_write !" << finl, cg_error_exit();

      if (cg_goto(fileId_, baseId_[ind], "Zone_t", zoneId_[ind], "ZoneIterativeData_t", 1, "end") != CG_OK)
        Cerr << "Error Format_Post_CGNS::finir_ : cg_goto !" << finl, cg_error_exit();

      cgsize_t idata[2];
      idata[0] = CGNS_STR_SIZE;
      idata[1] = nsteps;

      if (LOC == "SOM")
        {
          if (cg_array_write("FlowSolutionPointers", CGNS_ENUMV(Character), 2, idata, solname_som_.c_str()) != CG_OK)
            Cerr << "Error Format_Post_CGNS::finir_ : cg_array_write !" << finl, cg_error_exit();
        }
      else if (LOC == "ELEM")
        {
          if (cg_array_write("FlowSolutionPointers", CGNS_ENUMV(Character), 2, idata, solname_elem_.c_str()) != CG_OK)
            Cerr << "Error Format_Post_CGNS::finir_ : cg_array_write !" << finl, cg_error_exit();
        }

      if (cg_simulation_type_write(fileId_, baseId_[ind], CGNS_ENUMV(TimeAccurate)) != CG_OK)
        Cerr << "Error Format_Post_CGNS::finir_ : cg_simulation_type_write !" << finl, cg_error_exit();
    }

  /* 2 : on iter sur les autres domaines; ie: domaine dis */
  for (int i = 0; i < static_cast<int>(doms_written_.size()); i++)
    {
      if (std::find(ind_doms_dumped.begin(), ind_doms_dumped.end(), i) == ind_doms_dumped.end()) // indice pas dans ind_doms_dumped
        {
          const Nom& nom_dom = doms_written_[i];
          const int ind = get_index_nom_vector(doms_written_, nom_dom);
          assert(ind > -1);

          /* create BaseIterativeData */
          if (cg_biter_write(fileId_, baseId_[ind], "TimeIterValues", nsteps) != CG_OK)
            Cerr << "Error Format_Post_CGNS::finir_ : cg_biter_write !" << finl, cg_error_exit();

          /* go to BaseIterativeData level and write time values */
          if (cg_goto(fileId_, baseId_[ind], "BaseIterativeData_t", 1, "end") != CG_OK)
            Cerr << "Error Format_Post_CGNS::finir_ : cg_goto !" << finl, cg_error_exit();

          cgsize_t nuse = static_cast<cgsize_t>(nsteps);
          if (cg_array_write("TimeValues", CGNS_DOUBLE_TYPE, 1, &nuse, time_post_.data()) != CG_OK)
            Cerr << "Error Format_Post_CGNS::finir_ : cg_array_write !" << finl, cg_error_exit();

          /* create ZoneIterativeData */
          if (cg_ziter_write(fileId_, baseId_[ind], zoneId_[ind], "ZoneIterativeData") != CG_OK)
            Cerr << "Error Format_Post_CGNS::finir_ : cg_ziter_write !" << finl, cg_error_exit();

          if (cg_goto(fileId_, baseId_[ind], "Zone_t", zoneId_[ind], "ZoneIterativeData_t", 1, "end") != CG_OK)
            Cerr << "Error Format_Post_CGNS::finir_ : cg_goto !" << finl, cg_error_exit();

          if (cg_simulation_type_write(fileId_, baseId_[ind], CGNS_ENUMV(TimeAccurate)) != CG_OK)
            Cerr << "Error Format_Post_CGNS::finir_ : cg_simulation_type_write !" << finl, cg_error_exit();
        }
      else { /* Do Nothing */ }
    }

  /* 3 : close cgns file */
  std::string fn = cgns_basename_.getString() + ".cgns"; // file name
  if (cg_close(fileId_) != CG_OK)
    Cerr << "Error Format_Post_CGNS::finir : cg_close !" << finl, cg_error_exit();

  Cerr << "**** CGNS file " << fn << " closed !" << finl;
}

#endif
