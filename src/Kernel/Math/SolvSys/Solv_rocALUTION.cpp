/****************************************************************************
* Copyright (c) 2022, CEA
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

#include <Solv_rocALUTION.h>
#include <Matrice_Morse_Sym.h>
#include <TRUSTVect.h>
#include <EChaine.h>
#include <Motcle.h>
#include <SFichier.h>

Implemente_instanciable_sans_constructeur_ni_destructeur(Solv_rocALUTION,"Solv_rocALUTION",SolveurSys_base);

// printOn
Sortie& Solv_rocALUTION::printOn(Sortie& s ) const
{
  s << chaine_lue_;
  return s;
}

// readOn
Entree& Solv_rocALUTION::readOn(Entree& is)
{
  create_solver(is);
  return is;
}

Solv_rocALUTION::Solv_rocALUTION()
{
  initialize();
}

Solv_rocALUTION::Solv_rocALUTION(const Solv_rocALUTION& org)
{
  initialize();
  // on relance la lecture ....
  EChaine recup(org.get_chaine_lue());
  readOn(recup);
}

Solv_rocALUTION::~Solv_rocALUTION()
{
#ifdef ROCALUTION_ROCALUTION_HPP_
  if (ls!=nullptr)
    {
      ls->Clear();
      delete ls;
    }
  if (sp_ls!=nullptr)
    {
      sp_ls->Clear();
      delete sp_ls;
    }
  if (p!=nullptr)
    {
      p->Clear();
      delete p;
    }
  if (sp_p!=nullptr)
    {
      sp_p->Clear();
      delete sp_p;
    }
#endif
}

void Solv_rocALUTION::initialize()
{
#ifdef ROCALUTION_ROCALUTION_HPP_
  ls = nullptr;
  sp_ls = nullptr;
  p = nullptr;
  sp_p = nullptr;
  write_system_ = false;
#else
  Process::exit("Sorry, rocALUTION solvers not available with this build.");
#endif
}

double precond_option(Entree& is, const Motcle& motcle)
{
  Motcle motlu;
  is >> motlu;
  Cerr << motlu << " ";
  if (motlu!="{") Process::exit("\nWe are waiting {");
  is >> motlu;
  Cerr << motlu << " ";
  if (motlu==motcle)
    {
      double val;
      is >> val;
      Cerr << val << " ";
      is >> motlu;
      Cerr << motlu << " ";
      return val;
    }
  if (motlu!="}") Process::exit("\nWe are waiting }");
  return -1;
}

// Fonction template pour la creation des precond simple ou double precision
#ifdef ROCALUTION_ROCALUTION_HPP_
template <typename T>
Solver<LocalMatrix<T>, LocalVector<T>, T>* create_rocALUTION_precond(EChaine& is)
{
  Solver<LocalMatrix<T>, LocalVector<T>, T>* p;
  Motcle precond;
  is >> precond;
  Cerr << precond << " ";
  // Preconditioner
  if (precond==(Motcle)"Jacobi" || precond==(Motcle)"diag") // OK en ~335 its
    {
      p = new Jacobi<LocalMatrix<T>, LocalVector<T>, T>();
      precond_option(is, "");
    }
  else if (precond==(Motcle)"ILUT") // OK en 78 its
    {
      p = new ILUT<LocalMatrix<T>, LocalVector<T>, T>();
      precond_option(is, "");
    }
  else if (precond==(Motcle)"SPAI") // Converge pas
    {
      p = new SPAI<LocalMatrix<T>, LocalVector<T>, T>();
      precond_option(is, "");
    }
  else if (precond==(Motcle)"ILU") // OK en ~123 its (level 0), 76 its (level 1)
    {
      p = new ILU<LocalMatrix<T>, LocalVector<T>, T>();
      int level = (int)precond_option(is, "level");
      if (level>=0) dynamic_cast<ILU<LocalMatrix<T>, LocalVector<T>, T> &>(*p).Set(level, true);
    }
  else if (precond==(Motcle)"MultiColoredSGS") // OK en ~150 its (mais 183 omega 1.6!) (semble equivalent a GCP/SSOR mais 3 fois plus lent)
    {
      p = new MultiColoredSGS<LocalMatrix<T>, LocalVector<T>, T>();
      double omega = precond_option(is, "omega");
      if (omega>=0) dynamic_cast<MultiColoredSGS<LocalMatrix<T>, LocalVector<T>, T> &>(*p).SetRelaxation(omega);
    }
  else if (precond==(Motcle)"GS") // Converge pas
    {
      p = new GS<LocalMatrix<T>, LocalVector<T>, T>();
      precond_option(is, "");
    }
  else if (precond==(Motcle)"MultiColoredGS") // Converge pas
    {
      p = new MultiColoredGS<LocalMatrix<T>, LocalVector<T>, T>();
      double omega = ::precond_option(is, "omega");
      if (omega>=0) dynamic_cast<MultiColoredGS<LocalMatrix<T>, LocalVector<T>, T> &>(*p).SetRelaxation(omega);
    }
  else if (precond==(Motcle)"SGS") // Converge pas
    {
      p = new SGS<LocalMatrix<T>, LocalVector<T>, T>();
      precond_option(is, "");
    }
  else if (precond==(Motcle)"BlockPreconditioner")   // ToDo: See page 67
    {
      p = new BlockPreconditioner<LocalMatrix<T>, LocalVector<T>, T>();
      precond_option(is, "");
    }
  else if (precond==(Motcle)"SAAMG" || precond==(Motcle)"SA-AMG")  // Converge en 100 its
    {
      p = new SAAMG<LocalMatrix<T>, LocalVector<T>, T>();
      precond_option(is, "");
    }
  else if (precond==(Motcle)"UAAMG" || precond==(Motcle)"UA-AMG")  // Converge en 120 its
    {
      p = new UAAMG<LocalMatrix<T>, LocalVector<T>, T>();
      precond_option(is, "");
      dynamic_cast<UAAMG<LocalMatrix<T>, LocalVector<T>, T> &>(*p).Verbose(0);
    }
  else if (precond==(Motcle)"PairwiseAMG")  // Converge pas
    {
      p = new PairwiseAMG<LocalMatrix<T>, LocalVector<T>, T>();
      precond_option(is, "");
      dynamic_cast<PairwiseAMG<LocalMatrix<T>, LocalVector<T>, T> &>(*p).Verbose(0);
    }
  else if (precond==(Motcle)"RugeStuebenAMG" || precond==(Motcle)"C-AMG")  // Converge en 57 its (equivalent a C-AMG ?)
    {
      p = new RugeStuebenAMG<LocalMatrix<T>, LocalVector<T>, T>();
      precond_option(is, "");
    }
  else
    {
      Cerr << "Error! Unknown rocALUTION preconditionner: " << precond << finl;
      Process::exit();
      return nullptr;
    }
  return p;
}

// Fonction template pour la creation des solveurs simple ou double precision
template <typename T>
IterativeLinearSolver<LocalMatrix<T>, LocalVector<T>, T>* create_rocALUTION_solver(const Motcle& solver)
{
  if (solver==(Motcle)"GCP")
    {
      return new CG<LocalMatrix<T>, LocalVector<T>, T>();
    }
  else if (solver==(Motcle)"GMRES")
    {
      return new GMRES<LocalMatrix<T>, LocalVector<T>, T>();
    }
  else
    {
      Cerr << solver << " solver not recognized for rocALUTION." << finl;
      Process::exit();
      return nullptr;
    }
}
#endif

// Lecture et creation du solveur
void Solv_rocALUTION::create_solver(Entree& entree)
{
#ifdef ROCALUTION_ROCALUTION_HPP_
  // Valeurs par defaut:
  atol_ = 1.e-12;
  rtol_ = 1.e-12;
  double div_tol = 1e3;
  bool mixed_precision = false;

  lecture(entree);
  EChaine is(get_chaine_lue());
  Motcle accolade_ouverte("{"), accolade_fermee("}");
  Motcle solver, motlu;
  EChaine precond;
  is >> solver;   // On lit le solveur en premier puis les options du solveur: PETSC ksp { ... }
  is >> motlu; // On lit l'accolade
  if (motlu != accolade_ouverte)
    {
      Cerr << "Error while reading the parameters of the solver " << solver << " { ... }" << finl;
      Cerr << "We expected " << accolade_ouverte << " instead of " << motlu << finl;
      exit();
    }
  // Lecture des parametres du solver:
  // precond name { [ omega double } [ level int ] } [impr] [seuil|atol double] [rtol double] }
  is >> motlu;
  while (motlu!=accolade_fermee)
    {
      if (motlu==(Motcle)"impr")
        {
          fixer_limpr(1);
        }
      else if (motlu==(Motcle)"seuil" || motlu==(Motcle)"atol")
        {
          is >> atol_;
        }
      else if (motlu==(Motcle)"rtol")
        {
          is >> rtol_;
        }
      else if (motlu==(Motcle)"precond")
        {
          Nom str("");
          is >> motlu;
          while (motlu!=accolade_fermee)
            {
              str+=motlu+" ";
              is >> motlu;
            }
          str+=motlu;
          precond.init(str);
        }
      else if (motlu==(Motcle)"save_matrix")
        {
          write_system_ = true;
        }
      else if (motlu==(Motcle)"mixed_precision")
        {
          // Mixed precision
          mixed_precision = true;
        }
      else
        {
          Cerr << motlu << " keyword not recognized for rocALUTION solver " << solver << finl;
          Process::exit();
        }
      is >> motlu;
    }

  // Creation des solveurs et precond rocALUTION
  if (mixed_precision)
    {
      ls = new MixedPrecisionDC<LocalMatrix<double>, LocalVector<double>, double, LocalMatrix<float>, LocalVector<float>, float>();
      sp_p = create_rocALUTION_precond<float>(precond);
      sp_ls = create_rocALUTION_solver<float>(solver);
      sp_ls->SetPreconditioner(*sp_p);
      sp_ls->InitTol(atol_, rtol_, div_tol); // ToDo "The stopping criteria for the inner solver has to be tuned well for good performance."
    }
  else
    {
      p = create_rocALUTION_precond<double>(precond);
      ls = create_rocALUTION_solver<double>(solver);
      ls->SetPreconditioner(*p);
    }
  ls->InitTol(atol_, rtol_, div_tol);
  //p->FlagPrecond();
  //ls->Clear();
  //ls->InitMaxIter(500);
  //ls->InitMinIter(20);
#endif
}

// ToDo remonter dans Matrice_Morse
// Urgent de faire une reconception d'une classe mere de Solv_Petsc et Solv_rocALUTION qui factoriser
// les conversions de matrice...
void MorseSymToMorseToMatrice_Morse(const Matrice_Morse_Sym& MS, Matrice_Morse& M)
{
  M = MS;
  Matrice_Morse mattmp(MS);
  M.transpose(mattmp);
  int ordre = M.ordre();
  for (int i=0; i<ordre; i++)
    if (M.nb_vois(i))
      M(i, i) = 0.;
  M = mattmp + M;
}

static int save=0;
void write_matrix(const Matrice_Base& a)
{
  // Save matrix
  if (Process::nproc() > 1) Process::exit("Error, matrix market format is not available yet in parallel.");
  Nom filename(Objet_U::nom_du_cas());
  filename += "_trust_matrix";
  filename += (Nom)save;
  filename += ".mtx";
  SFichier mtx(filename);
  mtx.precision(14);
  mtx.setf(ios::scientific);
  int rows = a.nb_lignes();
  mtx << "%%MatrixMarket matrix coordinate real " << (sub_type(Matrice_Morse_Sym, a) ? "symmetric" : "general") << finl;
  Cerr << "Matrix (" << rows << " lines) written into file: " << filename << finl;
  mtx << "%%matrix" << finl;
  Matrice_Morse csr = ref_cast(Matrice_Morse, a);
  mtx << rows << " " << rows << " " << csr.get_tab1()[rows] << finl;
  for (int row=0; row<rows; row++)
    for (int j=csr.get_tab1()[row]; j<csr.get_tab1()[row+1]; j++)
      mtx << row+1 << " " << csr.get_tab2()[j-1] << " " << csr.get_coeff()[j-1] << finl;
}
#ifdef ROCALUTION_ROCALUTION_HPP_
void write_vectors(const LocalVector<double>& rhs, const LocalVector<double>& sol)
{
  Nom filename(Objet_U::nom_du_cas());
  filename = Objet_U::nom_du_cas();
  filename += "_rocalution_rhs";
  filename += (Nom)save;
  filename += ".vec";
  Cout << "Write rhs into " << filename << finl;
  rhs.WriteFileASCII(filename.getString());
  filename = Objet_U::nom_du_cas();
  filename += "_rocalution_sol";
  filename += (Nom)save;
  filename += ".vec";
  Cout << "Write initial solution into " << filename << finl;
  sol.WriteFileASCII(filename.getString());
}
void write_matrix(const LocalMatrix<double>& mat)
{
  Nom filename(Objet_U::nom_du_cas());
  filename += "_rocalution_matrix";
  filename += (Nom)save;
  filename += ".mtx";
  Cout << "Writing rocALUTION matrix into " << filename << finl;
  mat.WriteFileMTX(filename.getString());
}

void check(const DoubleVect& t, LocalVector<double>& r, const Nom& nom)
{
  double norm_t = t.mp_norme_vect();
  double norm_r = r.Norm();
  double difference = std::abs((norm_t - norm_r)/(norm_t+norm_r+DMINFLOAT));
  if (difference>1.e-8)
    {
      Cerr << nom << " Trust " << norm_t << " rocALUTION " << norm_t << " difference " << difference << finl;
      Process::exit();
    }
}
double residual(const Matrice_Base& a, const DoubleVect& b, const DoubleVect& x)
{
  DoubleVect e(b);
  e *= -1;
  a.ajouter_multvect(x, e);
  return e.mp_norme_vect();
}
double residual_device(const LocalMatrix<double>& a, const LocalVector<double>& b, const LocalVector<double>& x, LocalVector<double>& e)
{
  a.Apply(x, &e);
  e.ScaleAdd(-1.0, b);
  return e.Norm();
}
#endif

int Solv_rocALUTION::resoudre_systeme(const Matrice_Base& a, const DoubleVect& b, DoubleVect& x)
{
#ifdef ROCALUTION_ROCALUTION_HPP_
  if (write_system_) save++;
  double tick;
  if (nouvelle_matrice())
    {
      // Build matrix
      tick = rocalution_time();

      // Conversion matrice stockage symetrique vers matrice stockage general:
      Matrice_Morse csr;
      MorseSymToMorseToMatrice_Morse(ref_cast(Matrice_Morse_Sym, a), csr);

      // Save TRUST matrix to check:
      if (write_system_) write_matrix(a);

      int N = csr.get_tab1().size_array() - 1;
      ArrOfInt tab1_c(N+1); // Passage Fortran->C
      for (int i = 0; i < N+1; i++)
        tab1_c(i) = csr.get_tab1()[i] - 1;

      int nnz = csr.get_coeff().size_array();
      ArrOfInt tab2_c(nnz); // Passage Fortran->C
      for (int i = 0; i < nnz; i++)
        tab2_c(i) = csr.get_tab2()[i] - 1;

      // Copie de la matrice TRUST dans une matrice rocALUTION
      Cout << "[rocALUTION] Build a matrix with N= " << N << " and nnz=" << nnz << finl;
      mat.AllocateCSR("a", nnz, N, N);
      mat.CopyFromCSR(tab1_c.addr(), tab2_c.addr(), csr.get_coeff().addr());
      mat.Sort();
      Cout << "[rocALUTION] Time to build matrix: " << (rocalution_time() - tick) / 1e6 << finl;
      mat.Info();
      tick = rocalution_time();
      mat.MoveToAccelerator(); // Important: move mat to device so after ls is built on device (best for performance)
      Cout << "[rocALUTION] Time to copy matrix on device: " << (rocalution_time() - tick) / 1e6 << finl;

      tick = rocalution_time();
      ls->SetOperator(mat);
      if (sp_ls!=nullptr)    // Important: Inner solver for mixed-precision solver set after SetOperator and before Build
        {
          auto& mp_ls = dynamic_cast<MixedPrecisionDC<LocalMatrix<double>, LocalVector<double>, double, LocalMatrix<float>, LocalVector<float>, float> &>(*ls);
          mp_ls.Set(*sp_ls);
        }
      // C-AMG:
      try
        {
          // ToDo: tuning C-AMG (pas efficace encore). On doit le faire apres le SetOperator(mat) pour avoir les differentes grilles:
          auto& mg = dynamic_cast<RugeStuebenAMG<LocalMatrix<double>, LocalVector<double>, double> &>(*p);
          //mg.Verbose(2);
          // Specify coarsest grid solver:
          bool ChangeDefaultCoarsestSolver = false; // Default ?
          if (ChangeDefaultCoarsestSolver)
            {
              mg.SetManualSolver(true);
              Solver<LocalMatrix<double>, LocalVector<double>, double> *cgs;
              cgs = new CG<LocalMatrix<double>, LocalVector<double>, double>();
              //cgs = new LU<LocalMatrix<T>, LocalVector<T>, T>();
              mg.SetSolver(*cgs);
            }
          bool ChangeDefaultSmoothers = false; // Default: FixedPoint/Jacobi ?
          if (ChangeDefaultSmoothers)
            {
              // Specify smoothers:
              mg.SetManualSmoothers(true);
              mg.SetOperator(mat);
              mg.BuildHierarchy();
              // Smoother for each level
              mg.SetSmootherPreIter(1);
              mg.SetSmootherPostIter(1);
              int levels = mg.GetNumLevels();
              auto **sm = new IterativeLinearSolver<LocalMatrix<double>, LocalVector<double>, double> *[levels - 1];
              auto **gs = new Preconditioner<LocalMatrix<double>, LocalVector<double>, double> *[levels - 1];
              for (int i = 0; i < levels - 1; ++i)
                {
                  // Smooth with (ToDo ameliorer car pas efficace...)
                  FixedPoint<LocalMatrix<double>, LocalVector<double>, double> *fp;
                  fp = new FixedPoint<LocalMatrix<double>, LocalVector<double>, double>;
                  sm[i] = fp;
                  gs[i] = new MultiColoredSGS<LocalMatrix<double>, LocalVector<double>, double>;
                  sm[i]->SetPreconditioner(*gs[i]);
                  sm[i]->Verbose(0);
                }
              mg.SetSmoother(sm);
            }
        }
      catch(const std::bad_cast& e)
        { };

      ls->Build();
      ls->Print();
      /*
      if (sp_ls!=nullptr) sp_ls->Print();
      if (p!=nullptr) p->Print();
      if (sp_p!=nullptr) sp_p->Print(); */
      Cout << "[rocALUTION] Time to build solver on device: " << (rocalution_time() - tick) / 1e6 << finl;

      // Save rocALUTION matrix to check
      if (write_system_) write_matrix(mat);
    }
  long N = mat.GetN();
  assert(N==b.size_array());
  assert(N==x.size_array());

  // Build rhs and initial solution:
  tick = rocalution_time();
  LocalVector<double> sol;
  LocalVector<double> rhs;
  LocalVector<double> e;
  sol.Allocate("a", N);
  sol.CopyFromData(x.addr());
  rhs.Allocate("rhs", N);
  rhs.CopyFromData(b.addr());
  Cout << "[rocALUTION] Time to build vectors: " << (rocalution_time() - tick) / 1e6 << finl;
  tick = rocalution_time();
  sol.MoveToAccelerator();
  rhs.MoveToAccelerator();
  e.MoveToAccelerator();
  e.Allocate("e", N);
  Cout << "[rocALUTION] Time to move vectors on device: " << (rocalution_time() - tick) / 1e6 << finl;
  if (write_system_) write_vectors(rhs, sol);
  sol.Info();

#ifndef NDEBUG
  // Check before solves:
  rhs.Check();
  sol.Check();
  mat.Check();
  check(x, sol, "Before ||x||");
  check(b, rhs, "Before ||b||");
#endif
  // Petit bug rocALUTION (division par 0 si rhs nul, on contourne en mettant la verbosity a 0)
  ls->Verbose(limpr() && rhs.Norm()>0 ? 2 : 0);
  if (sp_ls!=nullptr) sp_ls->Verbose(limpr() && rhs.Norm()>0 ? 2 : 0);

  // Solve A x = rhs
  tick = rocalution_time();
  // Calcul des residus sur le host la premiere fois (plus sur) puis sur device ensuite (plus rapide)
  double res_initial = first_solve_ ? residual(a, b, x) : residual_device(mat, rhs, sol, e);
  ls->Solve(rhs, &sol);
  if (ls->GetSolverStatus()==3) Process::exit("Divergence for solver.");
  if (ls->GetSolverStatus()==4) Cout << "Maximum number of iterations reached." << finl;
  Cout << "[rocALUTION] Time to solve: " << (rocalution_time() - tick) / 1e6 << finl;

  int nb_iter = ls->GetIterationCount();
  double res_final = ls->GetCurrentResidual();

  // Recupere la solution
  sol.MoveToHost();
  sol.CopyToData(x.addr());
  if (first_solve_) res_final = residual(a, b, x); // Securite a la premiere resolution
#ifndef NDEBUG
  check(x, sol, "After ||x||");
  check(b, rhs, "After ||b||");
#endif

  // Check residual e=||Ax-rhs||:
  if (res_final>atol_)
    {
      Cerr << "Solution not correct ! ||Ax-b|| = " << res_final << finl;
      Process::exit();
    }
  if (limpr()>-1)
    {
      double residu_relatif=(res_initial>0?res_final/res_initial:res_final);
      Cout << finl << "Final residue: " << res_final << " ( " << residu_relatif << " )"<<finl;
    }
  fixer_nouvelle_matrice(0);
  sol.Clear();
  rhs.Clear();
  if (nb_iter>1) first_solve_ = false;
  return nb_iter;
#else
  return -1;
#endif
}
