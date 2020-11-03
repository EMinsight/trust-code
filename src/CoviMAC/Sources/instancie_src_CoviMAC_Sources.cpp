//
// Warning : DO NOT EDIT !
// To update this file, run: make depend
//
#include <verifie_pere.h>
#include <Iterateur_Source_CoviMAC.h>
#include <Perte_Charge_Anisotrope_CoviMAC_Face.h>
#include <Perte_Charge_Circulaire_CoviMAC_Face.h>
#include <Perte_Charge_Directionnelle_CoviMAC_Face.h>
#include <Perte_Charge_Isotrope_CoviMAC_Face.h>
#include <Perte_Charge_Singuliere_CoviMAC_Face.h>
#include <Terme_Boussinesq_CoviMAC_Face.h>
#include <Terme_Puissance_Thermique_CoviMAC_Elem.h>
#include <Terme_Source_Constituant_CoviMAC_Elem.h>
#include <Terme_Source_Qdm_Face_CoviMAC.h>
void instancie_src_CoviMAC_Sources() {
Cerr << "src_CoviMAC_Sources" << finl;
Iterateur_Source_CoviMAC inst1;verifie_pere(inst1);
Perte_Charge_Anisotrope_CoviMAC_Face inst2;verifie_pere(inst2);
Perte_Charge_Circulaire_CoviMAC_Face inst3;verifie_pere(inst3);
Perte_Charge_Directionnelle_CoviMAC_Face inst4;verifie_pere(inst4);
Perte_Charge_Isotrope_CoviMAC_Face inst5;verifie_pere(inst5);
Perte_Charge_Singuliere_CoviMAC_Face inst6;verifie_pere(inst6);
Terme_Boussinesq_CoviMAC_Face inst7;verifie_pere(inst7);
Terme_Puissance_Thermique_CoviMAC_Elem inst8;verifie_pere(inst8);
Terme_Source_Constituant_CoviMAC_Elem inst9;verifie_pere(inst9);
Terme_Source_Qdm_Face_CoviMAC inst10;verifie_pere(inst10);
}
