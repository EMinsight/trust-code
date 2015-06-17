// Data exchange channel for trio
// TrioDEC.cxx
// version 0.0 06/06/2014

#include <TrioDEC.hxx>

#include <ICoCoMEDField.hxx>
#include <ICoCoTrioField.h>
#include <Convert_ICoCoTrioField.h>

using namespace ParaMEDMEM;
using namespace ICoCo;
 
TrioDEC::TrioDEC():_my_traduced_field(0)
{
}

TrioDEC::TrioDEC(ProcessorGroup& source_group, ProcessorGroup& target_group):InterpKernelDEC(source_group,target_group),_my_traduced_field(0)
{
}
void TrioDEC::attachLocalField(ICoCo::Field *field) 
{ 
  ICoCo::TrioField * triofield=dynamic_cast<ICoCo::TrioField *>(field);
  if (triofield)
    attachLocalField(triofield); 
  else
    {
      ICoCo::MEDField * medfield=dynamic_cast<ICoCo::MEDField *>(field);
      if (medfield)
	DisjointDEC::attachLocalField(medfield);
      else
	{
	  MEDCouplingFieldDouble * medfieldd=dynamic_cast<MEDCouplingFieldDouble *>(field);
	  if (medfieldd)
	    DisjointDEC::attachLocalField(medfieldd);
	  else
	    {
              std::cerr<<"error in TrioDEC::attachLocalField" <<std::endl;
	      throw INTERP_KERNEL::Exception("TrioDEC::attachLocalField ");
	    }

	}

    }
}

TrioDEC::TrioDEC(const std::set<int>& src_ids, const std::set<int>& trg_ids, const MPI_Comm& world_comm):InterpKernelDEC(src_ids,trg_ids,world_comm),_my_traduced_field(0)
{
}

void TrioDEC::attachLocalField(ICoCo::TrioField *field)
{
  if(!field)
    throw INTERP_KERNEL::Exception("TrioDEC::attachLocalField : The input trio Field is NULL !");
  releaseInternalPointer(); 
  _my_traduced_field=new MEDField(build_medfield(*field));
  DisjointDEC::attachLocalField(_my_traduced_field);
}

void TrioDEC::releaseInternalPointer()
{
  if(_my_traduced_field)
    delete _my_traduced_field;
  _my_traduced_field=0;
}

TrioDEC::~TrioDEC()
{
  releaseInternalPointer();
}
