//
// Warning : DO NOT EDIT !
// To update this file, run: make depend
//
#include <verifie_pere.h>
#include <Op_Conv_Amont_VDF_Elem.h>
#include <Op_Conv_Quick_VDF_Elem.h>
#include <Op_Conv_centre4_VDF_Elem.h>
#include <Op_Conv_centre_VDF_Elem.h>
void instancie_src_VDF_Operateurs_Operateurs_Conv() {
Cerr << "src_VDF_Operateurs_Operateurs_Conv" << finl;
Op_Conv_Amont_VDF_Elem inst1;verifie_pere(inst1);
Op_Conv_Quick_VDF_Elem inst2;verifie_pere(inst2);
Op_Conv_centre4_VDF_Elem inst3;verifie_pere(inst3);
Op_Conv_centre_VDF_Elem inst4;verifie_pere(inst4);
}
