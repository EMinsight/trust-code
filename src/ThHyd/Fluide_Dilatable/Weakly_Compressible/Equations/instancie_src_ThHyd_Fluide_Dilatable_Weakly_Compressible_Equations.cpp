//
// Warning : DO NOT EDIT !
// To update this file, run: make depend
//
#include <verifie_pere.h>
#include <Convection_Diffusion_Chaleur_WC.h>
#include <Convection_Diffusion_fraction_massique_MB_WC.h>
#include <Navier_Stokes_WC.h>
void instancie_src_ThHyd_Fluide_Dilatable_Weakly_Compressible_Equations() {
Cerr << "src_ThHyd_Fluide_Dilatable_Weakly_Compressible_Equations" << finl;
Convection_Diffusion_Chaleur_WC inst1;verifie_pere(inst1);
Convection_Diffusion_fraction_massique_MB_WC inst2;verifie_pere(inst2);
Navier_Stokes_WC inst3;verifie_pere(inst3);
}
