//
// Author : Toru Shiozaki
// Date   : Dec 2011
//

#include <src/casscf/casscf.h>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <src/casscf/qvec.h>
#include <boost/lexical_cast.hpp>

using namespace std;


CASSCF::CASSCF(multimap<string, string> idat, const shared_ptr<Geometry> geom, const shared_ptr<Reference> scf)
  : idata_(idat), geom_(geom), ref_(scf) {

  common_init();

};


void CASSCF::common_init() {
  // at the moment I only care about C1 symmetry, with dynamics in mind
  if (geom_->nirrep() > 1) throw runtime_error("CASSCF: C1 only at the moment.");
  print_header();

  // get maxiter from the input
  max_iter_ = read_input<int>(idata_, "maxiter", 100);
  // get maxiter from the input
  max_micro_iter_ = read_input<int>(idata_, "maxiter_micro", 100);
  // get nstate from the input
  nstate_ = read_input<int>(idata_, "nstate", 1);
  // get thresh (for macro iteration) from the input
  thresh_ = read_input<double>(idata_, "thresh", 1.0e-8);
  // get thresh (for micro iteration) from the input
  thresh_micro_ = read_input<double>(idata_, "thresh_micro", thresh_);

  // nocc from the input. If not present, full valence active space is generated.
  nocc_ = read_input<int>(idata_, "nocc", 0);
  if (nocc_ < 0) {
    throw runtime_error("It appears that nocc < 0. Check nocc value.");
  } else if (nocc_ == 0) {
    cout << "    * full valence occupied space generated for nocc." << endl;
    nocc_ = geom_->num_count_full_valence_nocc();
  }

  // nclosed from the input. If not present, full core space is generated.
  nclosed_ = read_input<int>(idata_, "nclosed", -1);
  if (nclosed_ < -1) {
    throw runtime_error("It appears that nclosed < 0. Check nocc value.");
  } else if (nclosed_ == -1) {
    cout << "    * full core space generated for nclosed." << endl;
    nclosed_ = geom_->num_count_ncore() / 2;
  }

  nact_ = nocc_ - nclosed_;
  if (nact_ <= 0) throw runtime_error("It appears that nact <= 0. Check the nocc and nclosed values");
#if 0
  nbasis_ = geom_->nbasis();
#else
  nbasis_ = ref_->coeff()->mdim();
#endif
  nvirt_ = nbasis_ - nocc_;
  if (nvirt_ < 0) throw runtime_error("It appears that nvirt < 0. Check the nocc value");

  cout << "    * nstate   : " << setw(6) << nstate_ << endl;
  cout << "    * nclosed  : " << setw(6) << nclosed_ << endl;
  cout << "    * nact     : " << setw(6) << nact_ << endl;
  cout << "    * nvirt    : " << setw(6) << nvirt_ << endl;

  const int idel = geom_->nbasis() - nbasis_;
  if (idel)
    cout << "      Due to linear dependency, " << idel << (idel==1 ? " function is" : " functions are") << " omitted" << endl;


  // CASSCF methods should have FCI member. Inserting "ncore" and "norb" keyword for closed and total orbitals.
  mute_stdcout();
  {
    multimap<string, string> fullci_data = idata_;
    for (auto iter = fullci_data.equal_range("ncore").first; iter != fullci_data.equal_range("ncore").second; ++iter)
      fullci_data.erase(iter);
    fullci_data.insert(make_pair("ncore", boost::lexical_cast<string>(nclosed_)));
    for (auto iter = fullci_data.equal_range("norb").first; iter != fullci_data.equal_range("norb").second; ++iter)
      fullci_data.erase(iter);
    fullci_data.insert(make_pair("norb", boost::lexical_cast<string>(nact_)));
    shared_ptr<FCI> fci_tmp(new FCI(fullci_data, geom_, ref_));
    fci_ = fci_tmp;
  }
  resume_stdcout();

};


CASSCF::~CASSCF() {

};


void CASSCF::print_header() const {
  cout << "  ---------------------------" << endl;
  cout << "      CASSCF calculation     " << endl;
  cout << "  ---------------------------" << endl << endl;
}

static streambuf* backup_stream_;
static ofstream* ofs_;

void CASSCF::mute_stdcout() {
#if 1
  ofstream* ofs(new ofstream("casscf.log",(backup_stream_ ? ios::app : ios::trunc)));
  ofs_ = ofs;
  backup_stream_ = cout.rdbuf(ofs->rdbuf());
#endif
}


void CASSCF::resume_stdcout() {
#if 1
  cout.rdbuf(backup_stream_);
  delete ofs_;
#endif
}


shared_ptr<Matrix1e> CASSCF::ao_rdm1(shared_ptr<RDM<1> > rdm1, const bool inactive_only) const {
  // first make 1RDM in MO
  shared_ptr<Matrix1e> mo_rdm1(new Matrix1e(geom_));
  for (int i = 0; i != nclosed_; ++i) mo_rdm1->element(i,i) = 1.0;
  if (!inactive_only) {
    for (int i = 0; i != nact_; ++i) {
      for (int j = 0; j != nact_; ++j) {
        mo_rdm1->element(nclosed_+j, nclosed_+i) = rdm1->element(j,i)*0.5; // note the difference in notation between SCF and FCI...
      }
    }
  }
  // transform into AO basis
  const shared_ptr<Coeff> coeff = ref_->coeff();
  shared_ptr<Matrix1e> ao_rdm1(new Matrix1e(*coeff * *mo_rdm1 ^ *coeff)); // TODO make sure when overlap is truncated
  return ao_rdm1;
}



void CASSCF::one_body_operators(shared_ptr<Matrix1e>& f, shared_ptr<QFile>& fact, shared_ptr<QFile>& factp, shared_ptr<QFile>& gaa,
                                shared_ptr<RotFile>& d, const bool superci) {

  shared_ptr<Matrix1e> finact;
  shared_ptr<Coeff> coeff = ref_->coeff();

  // get quantity Q_xr = 2(xs|tu)P_rs,tu (x=general)
  // note: this should be after natorb transformation.
  shared_ptr<Qvec> qxr(new Qvec(geom_->nbasis(), nact_, geom_->df(), ref_->coeff(), nclosed_, fci_));

  {
    // Fock operators
    if (nclosed_) {
      shared_ptr<Matrix1e> deninact = ao_rdm1(fci_->rdm1_av(), true); // true means inactive_only
      shared_ptr<Matrix1e> f_inactao(new Matrix1e(geom_));
      dcopy_(f_inactao->size(), fci_->jop()->core_fock_ptr(), 1, f_inactao->data(), 1);
      shared_ptr<Matrix1e> ft(new Matrix1e(*coeff % *f_inactao * *coeff));
      finact = ft;

      shared_ptr<Matrix1e> denall = ao_rdm1(fci_->rdm1_av());
      shared_ptr<Matrix1e> denact(new Matrix1e(*denall-*deninact));
      shared_ptr<Fock<1> > fact_ao(new Fock<1>(geom_, hcore_, denact, ref_->schwarz()));
      shared_ptr<Matrix1e> fact(new Matrix1e(*coeff % (*fact_ao-*hcore_) * *coeff));
      shared_ptr<Matrix1e> fs(new Matrix1e(*finact+*fact));
      f = fs;
    } else {
      shared_ptr<Matrix1e> denall = ao_rdm1(fci_->rdm1_av());
      shared_ptr<Fock<1> > f_ao(new Fock<1>(geom_, hcore_, denall, ref_->schwarz()));
      shared_ptr<Matrix1e> ft(new Matrix1e(*coeff % *f_ao * *coeff)); f = ft;

      finact = hcore_;
    }
  }
  {
    // active-x Fock operator Dts finact_sx + Qtx
    shared_ptr<QFile> ft(new QFile(*qxr)); fact = ft; // nbasis_ runs first
    if (nclosed_) {
      for (int i = 0; i != nact_; ++i)
        daxpy_(nbasis_, occup_[i], finact->element_ptr(0,nclosed_+i), 1, fact->data()+i*nbasis_, 1);
    }
  }
  {
    // active Fock' operator (Fts+Fst) / (ns+nt)
    shared_ptr<QFile> ft(new QFile(nact_, nact_)); factp = ft;
    for (int i = 0; i != nact_; ++i)
      for (int j = 0; j != nact_; ++j)
        factp->element(j,i) = (fact->element(j+nclosed_,i)+fact->element(i+nclosed_,j)) / (occup_[i]+occup_[j]);
  }

  // G matrix (active-active) 2Drs,tu Factp_tu - delta_rs nr sum_v Factp_vv
  shared_ptr<QFile> ft4(new QFile(nact_, nact_)); gaa = ft4;
  dgemv_("N", nact_*nact_, nact_*nact_, 1.0, fci_->rdm2_av()->data(), nact_*nact_, factp->data(), 1, 0.0, gaa->data(), 1);
  double p = 0.0;
  for (int i = 0; i != nact_; ++i) p += occup_[i] * factp->element(i,i);
  for (int i = 0; i != nact_; ++i) gaa->element(i,i) -= occup_[i] * p;

  // denominator
  shared_ptr<RotFile> denom(new RotFile(nclosed_, nact_, nvirt_));
  fill(denom->data(), denom->data()+denom->size(), 1.0e100);

  double* target = denom->ptr_va();
  if (superci) {
    for (int i = 0; i != nact_; ++i)
      for (int j = 0; j != nvirt_; ++j, ++target)
        *target = gaa->element(i,i) / occup_[i] + f->element(j+nocc_, j+nocc_);

    target = denom->ptr_vc();
    for (int i = 0; i != nclosed_; ++i)
      for (int j = 0; j != nvirt_; ++j, ++target)
        *target = f->element(j+nocc_, j+nocc_) - f->element(i, i);

    target = denom->ptr_ca();
    for (int i = 0; i != nact_; ++i) {
      const double fac = -((2.0 - 2.0*occup_[i]) * factp->element(i, i) - gaa->element(i, i)) / (2.0 - occup_[i]);
      for (int j = 0; j != nclosed_; ++j, ++target)
        *target = fac - f->element(j, j);
    }
  } else {
    for (int i = 0; i != nact_; ++i)
      for (int j = 0; j != nvirt_; ++j, ++target)
        *target = gaa->element(i,i) + occup_[i] * f->element(j+nocc_, j+nocc_);

    target = denom->ptr_vc();
    for (int i = 0; i != nclosed_; ++i)
      for (int j = 0; j != nvirt_; ++j, ++target)
        *target = 2.0 * f->element(j+nocc_, j+nocc_) - 2.0 * f->element(i, i);

    target = denom->ptr_ca();
    for (int i = 0; i != nact_; ++i) {
      const double fac = -((2.0 - 2.0*occup_[i]) * factp->element(i, i) - gaa->element(i, i));
      for (int j = 0; j != nclosed_; ++j, ++target)
        *target = fac - f->element(j, j)*(2.0 - occup_[i]);
    }
  }
  d = denom;
}


shared_ptr<Coeff> CASSCF::update_coeff(const shared_ptr<Coeff> cold, vector<double> mat) const {
  shared_ptr<Matrix1e> cnew(new Matrix1e(*dynamic_cast<Matrix1e*>(cold.get())));
  int nbas = geom_->nbasis();
  dgemm_("N", "N", nbas, nact_, nact_, 1.0, cold->data()+nbas*nclosed_, nbas, &(mat[0]), nact_,
                   0.0, cnew->data()+nbas*nclosed_, nbas);
  shared_ptr<Coeff> cnew1(new Coeff(*cnew));
  return cnew1;
}


