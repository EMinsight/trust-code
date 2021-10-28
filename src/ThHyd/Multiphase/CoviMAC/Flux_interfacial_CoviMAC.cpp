/****************************************************************************
* Copyright (c) 2021, CEA
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
// File:        Flux_interfacial_CoviMAC.cpp
// Directory:   $TRUST_ROOT/src/ThHyd/Multiphase/CoviMAC
// Version:     /main/13
//
//////////////////////////////////////////////////////////////////////////////

#include <Flux_interfacial_CoviMAC.h>
#include <Zone_CoviMAC.h>
#include <Champ_P0_CoviMAC.h>
#include <Matrix_tools.h>
#include <Pb_Multiphase.h>
#include <Champ_Uniforme.h>
#include <Flux_interfacial_base.h>
#include <Changement_phase_base.h>
#include <Milieu_composite.h>

Implemente_instanciable(Flux_interfacial_CoviMAC,"Flux_interfacial_P0_CoviMAC", Source_base);

Sortie& Flux_interfacial_CoviMAC::printOn(Sortie& os) const
{
  return os;
}

Entree& Flux_interfacial_CoviMAC::readOn(Entree& is)
{
  const Pb_Multiphase& pbm = ref_cast(Pb_Multiphase, equation().probleme());
  if (!pbm.has_correlation("flux_interfacial")) Process::exit(que_suis_je() + " : a flux_interfacial correlation must be defined in the global correlations { } block!");
  correlation_ = pbm.get_correlation("flux_interfacial");
  return is;
}

void Flux_interfacial_CoviMAC::dimensionner_blocs(matrices_t matrices, const tabs_t& semi_impl) const
{
  const Champ_P0_CoviMAC& ch = ref_cast(Champ_P0_CoviMAC, equation().inconnue().valeur());
  const Zone_CoviMAC& zone = ref_cast(Zone_CoviMAC, equation().zone_dis().valeur());
  const DoubleTab& inco = ch.valeurs();

  /* on doit pouvoir ajouter / soustraire les equations entre composantes */
  int i, j, e, n, N = inco.line_size();
  if (N == 1) return;
  std::set<int> idx;
  for (auto &&n_m : matrices) if (n_m.first.find("_") == std::string::npos) /* pour ignorer les inconnues venant d'autres problemes */
      {
        Matrice_Morse& mat = *n_m.second, mat2;
        const DoubleTab& dep = equation().probleme().get_champ(n_m.first.c_str()).valeurs();
        int m, nc = dep.dimension_tot(0), M = dep.line_size();
        IntTrav sten(0, 2);
        sten.set_smart_resize(1);
        if (n_m.first == "temperature" || n_m.first == "pression") /* temperature/pression: dependance locale */
          for (e = 0; e < zone.nb_elem(); e++) for (n = 0; n < N; n++) for (m = 0; m < M; m++) sten.append_line(N * e + n, M * e + m);
        else if (mat.nb_colonnes()) for (e = 0; e < zone.nb_elem(); e++) /* autres variables: on peut melanger les composantes*/
            {
              for (idx.clear(), n = 0, i = N * e; n < N; n++, i++) for (j = mat.get_tab1()(i) - 1; j < mat.get_tab1()(i + 1) - 1; j++)
                  idx.insert(mat.get_tab2()(j) - 1); //idx : ensemble des colonnes dont depend au moins une ligne des N composantes en e
              for (n = 0, i = N * e; n < N; n++, i++) for (auto &&x : idx) sten.append_line(i, x); //ajout de cette depedance a toutes les lignes
            }
        else continue;
        Matrix_tools::allocate_morse_matrix(N * zone.nb_elem_tot(), M * nc, sten, mat2);
        mat.nb_colonnes() ? mat += mat2 : mat = mat2;
      }

}

void Flux_interfacial_CoviMAC::ajouter_blocs(matrices_t matrices, DoubleTab& secmem, const tabs_t& semi_impl) const
{
  const Pb_Multiphase& pbm = ref_cast(Pb_Multiphase, equation().probleme());
  const Champ_P0_CoviMAC& ch = ref_cast(Champ_P0_CoviMAC, equation().inconnue().valeur());
  // Matrice_Morse *mat = matrices.count(ch.le_nom().getString()) ? matrices.at(ch.le_nom().getString()) : NULL;
  const Milieu_composite& milc = ref_cast(Milieu_composite, equation().milieu());
  const Zone_CoviMAC& zone = ref_cast(Zone_CoviMAC, equation().zone_dis().valeur());
  const DoubleVect& pe = zone.porosite_elem(), &ve = zone.volumes();
  const tabs_t& der_h = ref_cast(Champ_Inc_base, milc.enthalpie()).derivees();
  const Champ_Inc_base& ch_alpha = pbm.eq_masse.inconnue().valeur(), &ch_a_r = pbm.eq_masse.champ_conserve(), &ch_vit = pbm.eq_qdm.inconnue().valeur(),
                        &ch_temp = pbm.eq_energie.inconnue().valeur(), &ch_p = pbm.eq_qdm.pression().valeur();
  const DoubleTab& inco = ch.valeurs(), &alpha = ch_alpha.valeurs(), &press = ch_p.valeurs(), &temp  = ch_temp.valeurs(), &pvit = ch_vit.passe(),
                   &h = milc.enthalpie().valeurs(), *dP_h = der_h.count("pression") ? &der_h.at("pression") : NULL, *dT_h = der_h.count("temperature") ? &der_h.at("temperature") : NULL,
                    &lambda = milc.conductivite().passe(), &mu = milc.viscosite_dynamique().passe(), &rho = milc.masse_volumique().passe(), &Cp = milc.capacite_calorifique().passe(),
                     &p_ar = ch_a_r.passe();
  Matrice_Morse *Mp = matrices.count("pression")    ? matrices.at("pression")    : NULL,
                 *Mt = matrices.count("temperature") ? matrices.at("temperature") : NULL,
                  *Ma = matrices.count("alpha") ? matrices.at("alpha") : NULL;
  int i, j, c, e, d, D = dimension, k, l, n, N = inco.line_size(), nf_tot = zone.nb_faces_tot(),
                     cL = lambda.dimension_tot(0) == 1, cM = mu.dimension_tot(0) == 1, cR = rho.dimension_tot(0) == 1, cCp = Cp.dimension_tot(0) == 1, is_therm;
  const Flux_interfacial_base& correlation_fi = ref_cast(Flux_interfacial_base, correlation_.valeur().valeur());
  const Changement_phase_base *correlation_G = pbm.has_correlation("changement_phase") ? &ref_cast(Changement_phase_base, pbm.get_correlation("changement_phase").valeur()) : NULL;
  double dt = equation().schema_temps().pas_de_temps();

  /* limiteur de changement de phase : on limite gamma pour eviter d'avoir alpha_k < 0 dans une phase */
  /* pour cela, on assemble l'equation de masse sans changement de phase */
  DoubleTrav sec_m(alpha); //residus
  std::map<std::string, Matrice_Morse> mat_m_stockees;
  matrices_t mat_m; //derivees
  for (auto &&n_m : matrices) if (n_m.first.find("_") == std::string::npos) /* pour ignorer les inconnues venant d'autres problemes */
      {
        Matrice_Morse& dst = mat_m_stockees[n_m.first], &src = *n_m.second;
        dst.get_set_tab1().ref_array(src.get_set_tab1()); // memes stencils que celui de l'equation courante
        dst.get_set_tab2().ref_array(src.get_set_tab2());
        dst.set_nb_columns(src.nb_colonnes());
        dst.get_set_coeff().resize(src.get_set_coeff().size()); //coeffs nuls
        mat_m[n_m.first] = &dst;
      }
  const Masse_Multiphase& eq_m = pbm.eq_masse;
  for (i = 0; i < eq_m.nombre_d_operateurs(); i++) /* tous les operateurs */
    eq_m.operateur(i).l_op_base().ajouter_blocs(mat_m, sec_m, semi_impl);
  for (i = 0; i < eq_m.sources().size(); i++) if (!sub_type(Flux_interfacial_CoviMAC, eq_m.sources()(i).valeur())) /* toutes les sources sauf le flux interfacial */
      eq_m.sources()(i).valeur().ajouter_blocs(mat_m, sec_m, semi_impl);
  std::vector<std::array<Matrice_Morse *, 2>> vec_m; //vecteur "matrice source, matrice de destination"
  for (auto &&n_m : matrices) if (n_m.first.find("_") == std::string::npos && mat_m[n_m.first]->get_tab1().size() > 1) vec_m.push_back({{ mat_m[n_m.first], n_m.second }});

  /* elements */
  //coefficients et plein de derivees...
  DoubleTrav hi(N, N), dT_hi(N, N, N), da_hi(N, N, N), dP_hi(N, N), dT_phi(N), da_phi(N), dT_G(N), da_G(N), nv(N);
  for (e = 0; e < zone.nb_elem(); e++)
    {
      double vol = pe(e) * ve(e), x, G, dP_G, dv_min = 0.1, dh = zone.diametre_hydraulique_elem()(e, 0);
      for (nv = 0, d = 0; d < D; d++) for (n = 0; n < N; n++) nv(n) += std::pow(pvit(nf_tot + D * e + d, n), 2);
      for (n = 0; n < N; n++) nv(n) = max(sqrt(nv(n)), dv_min);
      //coeffs d'echange vers l'interface (explicites)
      correlation_fi.coeffs(dh, &alpha(e, 0), &temp(e, 0), press(e, 0), &nv(0),
                            &lambda(!cL * e, 0), &mu(!cM * e, 0), &rho(!cR * e, 0), &Cp(!cCp * e, 0), hi, dT_hi, da_hi, dP_hi);

      for (k = 0; k < N; k++) for (l = k + 1; l < N; l++) if (milc.has_saturation(k, l)) //flux phase k <-> phase l si saturation
            {
              Saturation_base& sat = milc.get_saturation(k, l);
              if (correlation_G) /* taux de changement de phase par une correlation */
                G = correlation_G->calculer(k, l, dh, &alpha(e, 0), &temp(e, 0), press(e, 0), &nv(0), &lambda(!cL * e, 0), &mu(!cM * e, 0), &rho(!cM * e, 0), &Cp(!cCp * e, 0),
                                            sat, dT_G, da_G, dP_G);
              /* limite thermique */
              double Tk = temp(e, k), Tl = temp(e, l), p = press(e, 0), Ts = sat.Tsat(p), dP_Ts = sat.dP_Tsat(p), //temperature de chaque cote + Tsat + derivees
                     phi = hi(k, l) * (Tk - Ts) + hi(l, k) * (Tl - Ts), L = (phi < 0 ? h(e, l) : sat.Hvs(p)) - (phi > 0 ? h(e, k) : sat.Hls(p));
              if ((is_therm = !correlation_G || dabs(G) > dabs(phi / L))) G = phi / L;
              /* enthalpies des phases (dans le sens correspondant au mode choisi pour G) */
              double dTk_hk = G > 0 && dT_h ? (*dT_h)(e, k) : 0, dP_hk = G > 0 ? (dP_h ? (*dP_h)(e, k) : 0) : sat.dP_Hls(p),
                       hl = G < 0 ? h(e, l) : sat.Hvs(p), dTl_hl = G < 0 && dT_h ? (*dT_h)(e, l) : 0, dP_hl = G < 0 ? (dP_h ? (*dP_h)(e, l) : 0) : sat.dP_Hvs(p);
              if (is_therm) /* derivees de G en limite thermique */
              {
                  double dP_phi = dP_hi(k, l) * (Tk - Ts) + dP_hi(l, k) * (Tl - Ts) - (hi(k, l) + hi(l, k)) * dP_Ts, dTk_L = -dTk_hk, dTl_L = dTl_hl, dP_L = dP_hl - dP_hk;
                  dP_G = (dP_phi - G * dP_L) / L;
                  for (n = 0; n < N; n++) dT_G(n) = ((n == k) * (hi(k, l) - G * dTk_L) + (n == l) * (hi(l, k) - G * dTl_L) + dT_hi(k, l, n) * (Tk - Ts) + dT_hi(l, k, n) * (Tl - Ts)) / L;
                  for (n = 0; n < N; n++) da_G(n) = (da_hi(k, l, n) * (Tk - Ts) + da_hi(l, k, n) * (Tl - Ts)) / L;
                }

              /* G est-il limite par l'evanescence cote k ou l ? */
              int n_lim = G > 0 ? k : l, sgn = G > 0 ? 1 : -1; //phase sortante
              double Glim = sec_m(e, n_lim) / vol + p_ar(e, n_lim) / dt; //changement de phase max acceptable par cette phase
              if (dabs(G) < Glim) n_lim = -2;       //G ne rend pas la phase evanescente -> pas de limitation (n_lim = -2)
              else if (Glim < 0) G = 0, n_lim = -1; //la phase serait evanescente meme sans G -> on le met a 0 (n_lim = -1)
              else G = (G > 0 ? 1 : -1) * Glim;//la phase serait evanescente a cause de G -> on le bloque a G_lim (n_lim = k / l)

              if (sub_type(Masse_Multiphase, equation())) //eq de masse -> changement de phase
                {
                  for (i = 0; i < 2; i++) secmem(e, i ? l : k) -= vol * (i ? -1 : 1) * G;
                  if (n_lim < -1) /* G par limite thermique */
                    {
                      if (Ma) for (i = 0; i < 2; i++) for (n = 0; n < N; n++)  //derivees en alpha
                            (*Ma)(N * e + (i ? l : k), N * e + n) += vol * (i ? -1 : 1) * da_G(n);
                      if (Mt) for (i = 0; i < 2; i++) for (n = 0; n < N; n++)  //derivees en T
                            (*Mt)(N * e + (i ? l : k), N * e + n) += vol * (i ? -1 : 1) * dT_G(n);
                      if (Mp) for (i = 0; i < 2; i++) //derivees en p
                          (*Mp)(N * e + (i ? l : k), e) += vol * (i ? -1 : 1) * dP_G;
                    }
                  else if (n_lim >= 0) for (auto &s_d : vec_m) /* G par evanescence */
                      for (j = s_d[0]->get_tab1()(N * e + n_lim) - 1; j < s_d[0]->get_tab1()(N * e + n_lim + 1) - 1; j++)
                        for (c = s_d[0]->get_tab2()(j) - 1, x = -s_d[0]->get_coeff()(j), i = 0; i < 2; i++)
                          (*s_d[1])(N * e + (i ? l : k), c) += (i ? -1 : 1) * sgn * x;
                }
              else if (sub_type(Energie_Multiphase, equation())) //eq d'energie -> transfert de chaleur
                {
                  //on suppose que la limite thermique s'applique toujours cote vapeur
                  for (i = 0; i < 2; i++) secmem(e, i ? l : k) -= vol * (i ? 1 : -1) * (hi(l, k) * (Tl - Ts) - G * hl);
                  /* derivees (y compris celles en G, sauf dans le cas limite)*/
                  if (Ma) for (i = 0; i < 2; i++) for (n = 0; n < N; n++) //derivees en alpha
                        (*Ma)(N * e + (i ? l : k), N * e + n) += vol * (i ? 1 : -1) * (da_hi(l, k, n) * (Tl - Ts) - (n_lim < -1) * da_G(n) * hl);
                  if (Mt) for (i = 0; i < 2; i++) for (n = 0; n < N; n++) //derivees en T
                        (*Mt)(N * e + (i ? l : k), N * e + n) += vol * (i ? 1 : -1) * (dT_hi(l, k, n) * (Tl - Ts) + (n == l) * (hi(l, k) - G * dTl_hl) - (n_lim < -1) * dT_G(n) * hl);
                  if (Mp) for (i = 0; i < 2; i++) //derivees en p
                      (*Mp)(N * e + (i ? l : k), e) += vol * (i ? 1 : -1) * (dP_hi(l, k) * (Tl - Ts) - hi(l, k) * dP_Ts - G * dP_hl - (n_lim < -1) * dP_G * hl);
                  if (n_lim >= 0) for (auto &s_d : vec_m) /* derivees de G dans le cas evanexcent */
                      for (j = s_d[0]->get_tab1()(N * e + n_lim) - 1; j < s_d[0]->get_tab1()(N * e + n_lim + 1) - 1; j++)
                        for (c = s_d[0]->get_tab2()(j) - 1, x = -s_d[0]->get_coeff()(j), i = 0; i < 2; i++)
                          (*s_d[1])(N * e + (i ? l : k), c) += (i ? -1 : 1) * hl * sgn * x;
                }
            }
          else if (sub_type(Energie_Multiphase, equation())) /* pas de saturation : echanges d'energie seulement */
            {
              //flux sortant de la phase k : hm * (Tk - Tl)
              double hm = 1. / (1. / hi(k, l) + 1. / hi(l, k)), Tk = temp(e, k), Tl = temp(e, l);
              for (i = 0; i < 2; i++) secmem(e, i ? l : k) -= vol * (i ? -1 : 1) * hm * (Tk - Tl);
              if (Mt) for (i = 0; i < 2; i++) //juste des derivees en T
                  {
                    (*Mt)(N * e + (i ? l : k), N * e + k) += vol * (i ? -1 : 1) * hm;
                    (*Mt)(N * e + (i ? l : k), N * e + l) += vol * (i ? 1 : -1) * hm;
                  }
            }
    }
}
