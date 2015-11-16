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
// File:        Convert_ICoCoTrioField.cpp
// Directory:   $TRUST_ROOT/src/Kernel/ICoCo
// Version:     1
//
//////////////////////////////////////////////////////////////////////////////

#include <Convert_ICoCoTrioField.h>
#include <ICoCoTrioField.h>

#include <ICoCoMEDField.hxx>
#include <Domaine.h>
#include <Champ_base.h>
#include <Zone_dis_base.h>


void affecte_double_avec_doubletab(double** p, const ArrOfDouble& trio)
{
  *p=new double[trio.size_array()];
  memcpy(*p,trio.addr(),trio.size_array()*sizeof(double));
}

void affecte_int_avec_inttab(int** p, const ArrOfInt& trio)
{
  *p=new int[trio.size_array()];
  memcpy(*p,trio.addr(),trio.size_array()*sizeof(int));
}

ICoCo::TrioField build_ICoCoField(const std::string& name,const Domaine& dom,  const DoubleTab& values,const int is_p1, const double& t1,const double& t2 )
{
  ICoCo::TrioField afield;
  afield.clear();
  afield._space_dim=dom.dimension;
  afield.setName(name);

  afield._mesh_dim=afield._space_dim;

  const DoubleTab& coord=dom.les_sommets();
  //ch.get_copy_coordinates(coord);
  affecte_double_avec_doubletab(&afield._coords,coord);

  afield._nbnodes=coord.dimension(0);
  Motcle type_elem_=dom.zone(0).type_elem()->que_suis_je();
  Motcle type_elem(type_elem_);
  type_elem.prefix("_AXI");
  if (type_elem!=Motcle(type_elem_))
    {
      if (type_elem=="QUADRILATERE_2D")
        type_elem="SEGMENT_2D";
      if (type_elem=="RECTANGLE_2D")
        type_elem="RECTANGLE";

    }
  if ((type_elem=="RECTANGLE") ||(type_elem=="QUADRANGLE")||(type_elem=="TRIANGLE")|| (type_elem=="TRIANGLE_3D")||(type_elem=="QUADRANGLE_3D"))
    afield._mesh_dim=2;
  else if ((type_elem=="HEXAEDRE")|| (type_elem=="HEXAEDRE_VEF")||(type_elem=="POLYEDRE")||(type_elem=="PRISME")||  (type_elem=="TETRAEDRE"))
    afield._mesh_dim=3;
  else if ((type_elem=="SEGMENT_2D")||(type_elem=="SEGMENT"))
    afield._mesh_dim=1;
  else
    {
      Cerr<<type_elem<< " not coded" <<finl;
      Process::exit();
    }

  const IntTab& les_elems=dom.zone(0).les_elems();
  //ch.get_copy_connectivity(ELEMENT,NODE,les_elems);
  affecte_int_avec_inttab(&afield._connectivity,les_elems);

  if (les_elems.dimension(1)==4)
    {
      for (int f=0; f<les_elems.dimension_tot(0); f++)
        {
          afield._connectivity[f*4+2]=les_elems(f,3);
          afield._connectivity[f*4+3]=les_elems(f,2);
        }
    }
  if (les_elems.dimension(1)==8)
    {
      for (int f=0; f<les_elems.dimension_tot(0); f++)
        {
          afield._connectivity[f*8+2]=les_elems(f,3);
          afield._connectivity[f*8+3]=les_elems(f,2);
          afield._connectivity[f*8+6]=les_elems(f,7);
          afield._connectivity[f*8+7]=les_elems(f,6);
        }
    }
  afield._nodes_per_elem=les_elems.dimension(1);
  afield._nb_elems=les_elems.dimension(0);

  afield._itnumber=0;
  afield._time1=t1;
  afield._time2=t2;



  afield._type=is_p1;

  afield._has_field_ownership=true;
  affecte_double_avec_doubletab(&afield._field,values);
  if (values.nb_dim()>1)
    afield._nb_field_components=values.dimension(1);
  else
    afield._nb_field_components=(1);
  return afield;
}
ICoCo::TrioField buildTrioField_from_champ_base(const Champ_base& ch)
{
  if (ch.a_une_zone_dis_base())
    {
      int is_p1= 0;
      double t1=ch.temps();
      double t2=t1;
      return build_ICoCoField(ch.le_nom().getString(),ch.zone_dis_base().zone().domaine(),  ch.valeurs(), is_p1,t1,t2 );

    }
  else
    {
      Cerr<<"In ICoCo::TrioField buildTrioField_from_champ_base "<<ch.le_nom()<<" has no zone_dis"<<finl;
      Process::exit();
    }
  // on arrive jamais la
  throw;
}




#ifndef NO_MEDFIELD
#include <MEDCouplingUMesh.hxx>
#include <MEDCouplingFieldDouble.hxx>
#include <MEDCouplingAutoRefCountObjectPtr.hxx>

#include <string.h>
#include <iostream>
#include <iomanip>

using namespace ICoCo;
using namespace std;


/*!
 * This method is non const only due to this->_field that can be modified (to point to the same zone than returned object).
 * So \b warning, to access to \a this->_field only when the returned object is alive.
 */
MEDField build_medfield(TrioField& triofield)
{
  ParaMEDMEM::MEDCouplingAutoRefCountObjectPtr<ParaMEDMEM::MEDCouplingUMesh> mesh(ParaMEDMEM::MEDCouplingUMesh::New("",triofield._mesh_dim));
  ParaMEDMEM::MEDCouplingAutoRefCountObjectPtr<ParaMEDMEM::DataArrayDouble> coo(ParaMEDMEM::DataArrayDouble::New());
  coo->alloc(triofield._nbnodes,triofield._space_dim);
  mesh->setCoords(coo);
  double *ptr(coo->getPointer());
  std::copy(triofield._coords,triofield._coords+triofield._space_dim*triofield._nbnodes,ptr);
  mesh->allocateCells(triofield._nb_elems);
  INTERP_KERNEL::NormalizedCellType elemtype;
  switch(triofield._mesh_dim)
    {
    case 0 :
      {
        switch (triofield._nodes_per_elem)
          {
          case 0: // cas field vide
            elemtype=INTERP_KERNEL::NORM_SEG2; // pour eviter warning
            break;
          default:
            throw INTERP_KERNEL::Exception("incompatible Trio field - wrong nb of nodes per elem");
          }
        break;
      }
    case 1:
      {
        switch (triofield._nodes_per_elem)
          {
          case 2:
            elemtype=INTERP_KERNEL::NORM_SEG2;
            break;
          default:
            throw INTERP_KERNEL::Exception("incompatible Trio field - wrong nb of nodes per elem");
          }
        break;
      }
    case 2:
      {
        switch (triofield._nodes_per_elem)
          {
          case 3:
            elemtype=INTERP_KERNEL::NORM_TRI3;
            break;
          case 4 :
            elemtype=INTERP_KERNEL::NORM_QUAD4;
            break;
          default:
            throw INTERP_KERNEL::Exception("incompatible Trio field - wrong nb of nodes per elem");
          }
        break;
      }
    case 3:
      {
        switch (triofield._nodes_per_elem)
          {
          case 4:
            elemtype=INTERP_KERNEL::NORM_TETRA4;
            break;
          case 8 :
            elemtype=INTERP_KERNEL::NORM_HEXA8;
            break;
          default:
            throw INTERP_KERNEL::Exception("incompatible Trio field - wrong nb of nodes per elem");
          }
        break;
      default:
        throw INTERP_KERNEL::Exception("incompatible Trio field - wrong mesh dimension");
      }
    }
  //creating a connectivity table that complies to MED (1 indexing)
  //and passing it to _mesh
  ParaMEDMEM::MEDCouplingAutoRefCountObjectPtr<ParaMEDMEM::MEDCouplingFieldDouble> field;
  int *conn(new int[triofield._nodes_per_elem]);
  for (int i=0; i<triofield._nb_elems; i++)
    {
      for(int j=0; j<triofield._nodes_per_elem; j++)
        {
          conn[j]=triofield._connectivity[i*triofield._nodes_per_elem+j];
        }
      mesh->insertNextCell(elemtype,triofield._nodes_per_elem,conn);
    }
  delete [] conn;
  mesh->finishInsertingCells();
  //
  //field on the sending end
  int nb_case=triofield.nb_values();
  if (triofield._type==0)
    {
      field =  ParaMEDMEM::MEDCouplingFieldDouble::New(ParaMEDMEM::ON_CELLS,ParaMEDMEM::ONE_TIME);
    }
  else
    {
      field =  ParaMEDMEM::MEDCouplingFieldDouble::New(ParaMEDMEM::ON_NODES,ParaMEDMEM::ONE_TIME );
    }
  field->setMesh(mesh);
  field->setNature(ParaMEDMEM::ConservativeVolumic);
  ParaMEDMEM::MEDCouplingAutoRefCountObjectPtr<ParaMEDMEM::DataArrayDouble> fieldArr(ParaMEDMEM::DataArrayDouble::New());
  fieldArr->alloc(field->getNumberOfTuplesExpected(),triofield._nb_field_components);
  field->setName(triofield.getName().c_str());
  std::string meshName("SupportOf_");
  meshName+=triofield.getName();
  mesh->setName(meshName.c_str());
  field->setTime(triofield._time1,0,triofield._itnumber);
  if (triofield._field!=0)
    {
      for (int i =0; i<nb_case; i++)
        for (int j=0; j<triofield._nb_field_components; j++)
          {
            fieldArr->setIJ(i,j,triofield._field[i*triofield._nb_field_components+j]);
          }
    }
  //field on the receiving end
  else
    {
      // the trio field points to the pointer inside the MED field
      triofield._field=fieldArr->getPointer();
      for (int i=0; i<triofield._nb_field_components*nb_case; i++)
        triofield._field[i]=0.0;
    }
  field->setArray(fieldArr);
  return MEDField(field);
}
MEDField build_medfield_from_champ_base(const Champ_base& ch)
{
  TrioField fl =  buildTrioField_from_champ_base(ch);
  return build_medfield(fl);
}


#else
namespace ICoCo
{
class MEDField
{
};
}
ICoCo::MEDField build_medfield(ICoCo::TrioField& ch)
{
  Cerr<<"Version compiled without MEDCoupling"<<finl;
  Process::exit();
  throw;
}
ICoCo::MEDField build_medfield_from_champ_base(const Champ_base& ch)
{
  Cerr<<"Version compiled without MEDCoupling"<<finl;
  Process::exit();
  throw;
}
#endif
