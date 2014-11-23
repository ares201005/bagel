//
// BAGEL - Parallel electron correlation program.
// Filename: caspt2grad.cc
// Copyright (C) 2013 Toru Shiozaki
//
// Author: Toru Shiozaki <shiozaki@northwestern.edu>
// Maintainer: Shiozaki group
//
// This file is part of the BAGEL package.
//
// The BAGEL package is free software; you can redistribute it and/or modify
// it under the terms of the GNU Library General Public License as published by
// the Free Software Foundation; either version 3, or (at your option)
// any later version.
//
// The BAGEL package is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public License
// along with the BAGEL package; see COPYING.  If not, write to
// the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//


#include <src/smith/caspt2grad.h>
#include <src/casscf/casbfgs.h>
#include <src/casscf/qvec.h>
#include <src/smith/smith.h>
#include <src/grad/gradeval.h>
#include <src/math/algo.h>


using namespace std;
using namespace bagel;

CASPT2Grad::CASPT2Grad(shared_ptr<const PTree> inp, shared_ptr<const Geometry> geom, shared_ptr<const Reference> ref)
  : Method(inp, geom, ref) {

  // compute CASSCF first
  auto cas = make_shared<SuperCI>(inp, geom, ref);
  cas->compute();

  cout << endl << "  === DF-CASPT2Grad calculation ===" << endl << endl;
  if (geom->df() == nullptr) throw logic_error("CASPT2Grad is only implemented with DF");

  // update reference
  ref_ = cas->conv_to_ref();
  fci_ = cas->fci();
  // TODO
  thresh_ = 1.0e-10; //cas->thresh();
  ref_energy_ = cas->energy();
}


// compute smith and set rdms and ci deriv to a member
void CASPT2Grad::compute() {
  const int nclosed = ref_->nclosed();
  const int nact = ref_->nact();
  {
    // construct SMITH here
    shared_ptr<const PTree> smithinput = idata_->get_child("smith");
    auto smith = make_shared<Smith>(smithinput, ref_->geom(), ref_);
    smith->compute();

    // use coefficients from smith (closed and virtual parts have been rotated in smith to make them canonical).
    coeff_ = smith->coeff();

    if (nact) {
      cideriv_ = smith->cideriv()->copy();
    }
    target_ = smith->algo()->ref()->target();
    ncore_  = smith->algo()->ref()->ncore();

    // save correlated density matrices d(1), d(2), and ci derivatives
    auto d1tmp = make_shared<Matrix>(*smith->dm1());
    auto d11tmp = make_shared<Matrix>(*smith->dm11());
    // d_1^(2) -= <1|1><0|E_mn|0>     [Celani-Werner Eq. (A6)]
    if (nact) {
      const double wf1norm = smith->wf1norm();
      shared_ptr<const Matrix> d0 = ref_->rdm1_mat(target_);
      for (int i = nclosed; i != nclosed+nact; ++i)
        for (int j = nclosed; j != nclosed+nact; ++j)
          d1tmp->element(j-ncore_, i-ncore_) -=  wf1norm * d0->element(j, i);
    }
    if (!ncore_) {
      d1_ = d1tmp;
      d11_ = d11tmp;
    } else {
      auto d1tmp2 = make_shared<Matrix>(coeff_->mdim(), coeff_->mdim());
      d1tmp2->copy_block(ncore_, ncore_, coeff_->mdim()-ncore_, coeff_->mdim()-ncore_, d1tmp);
      d1_ = d1tmp2->copy();
      d1tmp2->copy_block(ncore_, ncore_, coeff_->mdim()-ncore_, coeff_->mdim()-ncore_, d11tmp);
      d11_ = d1tmp2;
    }

    // correct cideriv for fock derivative [Celani-Werner Eq. (C1), some terms in first and second lines]
    // y_I += (g[d^(2)]_ij - Nf_ij) <I|E_ij|0>
    // -> y_I += [(h+g[d^(0)+d^(2)]) - (1+N)F] <I|E_ij|0>
    if (nact) {
      const int nmobasis = coeff_->mdim();
      auto d0 = ref_->rdm1_mat(target_)->resize(nmobasis,nmobasis);
      // TODO we should be able to avoid this Fock build
      auto d0ao = make_shared<Matrix>(*coeff_* *d0  ^ *coeff_);
      auto d1ao = make_shared<Matrix>((*coeff_* *d1_ ^ *coeff_) + *d0ao); // sum of d0 + d1 (to make it positive definite)
      auto fock  = make_shared<Fock<1>>(geom_, ref_->hcore(), d0ao);
      auto fock1 = make_shared<Fock<1>>(geom_, ref_->hcore(), d1ao);
      *fock1 -= *fock * (1.0+smith->wf1norm()); // g[d^(2)]

      auto acoeff = coeff_->slice(nclosed, nclosed+nact);
      auto fock1mo = make_shared<Matrix>(acoeff % *fock1 * acoeff);
      shared_ptr<const Dvec> deriv = ref_->rdm1deriv(target_);

      for (int i = 0; i != nact; ++i)
        for (int j = 0; j != nact; ++j)
          cideriv_->ax_plus_y(fock1mo->element(j,i), deriv->data(j+i*nact));
    }

    d2_ = smith->dm2();
    energy_ = smith->algo()->energy() + ref_energy_[target_];
  }
}


template<>
shared_ptr<GradFile> GradEval<CASPT2Grad>::compute() {
  shared_ptr<const Reference> ref = task_->ref();
  shared_ptr<FCI> fci = task_->fci();

  const int nclosed = ref->nclosed();
  const int nact = ref->nact();

  // state-averaged density matrices
  shared_ptr<const RDM<1>> rdm1_av = nact ? ref->rdm1_av() : nullptr;
  shared_ptr<const RDM<2>> rdm2_av = nact ? ref->rdm2_av() : nullptr;

  // second order density matrix
  shared_ptr<const Matrix> d1 = task_->d1();
  // first order density matrices
  shared_ptr<const Matrix> d11 = task_->d11();
  shared_ptr<const Matrix> d2 = task_->d2();
  shared_ptr<const Civec> cider = nact ? task_->cideriv() : nullptr;

  shared_ptr<const Matrix> coeff = task_->coeff();

  const int ncore = task_->ncore();
  const int nocc  = ref->nocc();
  const int nmobasis = coeff->mdim();

  // d0 including core
  shared_ptr<const Matrix> d0;
  if (nact) {
    d0 = ref->rdm1_mat(task_->target())->resize(nmobasis,nmobasis);
  } else {
    auto tmp = make_shared<Matrix>(nmobasis,nmobasis);
    for (int i = 0; i != nclosed; ++i) tmp->element(i,i) = 2.0;
    d0 = tmp;
  }
  const MatView ocoeff = coeff->slice(0, nocc);

  {
    auto dtotao = make_shared<Matrix>(*coeff * (*d0 + *d11 + *d1) ^ *coeff);
    Dipole dipole(geom_, dtotao, "CASPT2 unrelaxed");
    dipole.compute();
  }

  // compute Yrs
  shared_ptr<const DFHalfDist> half   = ref->geom()->df()->compute_half_transform(ocoeff);
  shared_ptr<const DFHalfDist> halfj  = half->apply_J();
  shared_ptr<const DFHalfDist> halfjj = halfj->apply_J();
  shared_ptr<Matrix> yrs;
  shared_ptr<const DFFullDist> fulld1; // (gamma| ir) D(ir,js)
  tie(yrs, fulld1) = task_->compute_Y(d1, d11, d2, half, halfj, halfjj);

  // solve CPCASSCF
  auto g0 = yrs;
  auto g1 = nact ? make_shared<Dvec>(cider, ref->nstate())
                 : make_shared<Dvec>(make_shared<Determinants>(), 1); // FIXME this is wrong for nstate > 1 in CASSCF
  auto grad = make_shared<PairFile<Matrix, Dvec>>(g0, g1);

  shared_ptr<const Dvec> civector;
  if (nact) {
    civector = ref->ciwfn()->civectors();
  } else {
    auto civec = make_shared<Dvec>(make_shared<Determinants>(), 1);
    civec->data(0)->element(0,0) = 1.0;
    civector = civec;
  }
  auto cp = make_shared<CPCASSCF>(grad, civector, half, halfjj, ref, fci, ncore, coeff);
  shared_ptr<const Matrix> zmat, xmat, smallz;
  shared_ptr<const Dvec> zvec;
  tie(zmat, zvec, xmat, smallz) = cp->solve(task_->thresh());

  // form relaxed 1RDM
  // form Zd + dZ^+
  shared_ptr<const Matrix> dsa = nact ? rdm1_av->rdm1_mat(nclosed)->resize(nmobasis, nmobasis) : d0;
  auto dm = make_shared<Matrix>(*zmat * *dsa + (*dsa ^ *zmat));

  shared_ptr<Matrix> dtot = d0->copy();
  dtot->ax_plus_y(1.0, dm);
  dtot->ax_plus_y(1.0, d1);
  dtot->ax_plus_y(1.0, d11);
  if (smallz)
    dtot->add_block(1.0, 0, 0, nocc, nocc, smallz);

  // form zdensity
  shared_ptr<const RDM<1>> zrdm1;
  shared_ptr<const RDM<2>> zrdm2;
  if (nact) {
    auto detex = make_shared<Determinants>(nact, fci->nelea(), fci->neleb(), false, /*mute=*/true);
    tie(zrdm1, zrdm2) = fci->compute_rdm12_av_from_dvec(ref->ciwfn()->civectors(), zvec, detex);

    shared_ptr<Matrix> zrdm1_mat = zrdm1->rdm1_mat(nclosed, false)->resize(nmobasis, nmobasis);
    zrdm1_mat->symmetrize();
    dtot->ax_plus_y(1.0, zrdm1_mat);
  }

  // compute relaxed dipole to check
  auto dtotao = make_shared<Matrix>(*coeff * *dtot ^ *coeff);
  {
    Dipole dipole(geom_, dtotao, "CASPT2 relaxed");
    dipole.compute();
  }

  // xmat in the AO basis
  auto xmatao = make_shared<Matrix>(*coeff * *xmat ^ *coeff);

  // two-body part
  // first make occ-occ part (copy-and-paste from src/casscf/supercigrad.cc)
  shared_ptr<const DFFullDist> qij  = halfjj->compute_second_transform(ocoeff);
  shared_ptr<DFHalfDist> qri;
  {
    shared_ptr<const Matrix> ztrans = make_shared<Matrix>(*coeff * zmat->slice(0,nocc));
    if (nact) {
      const RDM<2> D(*ref->rdm2(task_->target())+*zrdm2);
      const RDM<1> dd(*ref->rdm1(task_->target())+*zrdm1);

      shared_ptr<DFFullDist> qijd = qij->apply_2rdm(D, dd, nclosed, nact);
      qijd->ax_plus_y(2.0, halfjj->compute_second_transform(ztrans)->apply_2rdm(*rdm2_av, *rdm1_av, nclosed, nact));
      qri = qijd->back_transform(ocoeff);

      shared_ptr<const DFFullDist> qijd2 = qij->apply_2rdm(*rdm2_av, *rdm1_av, nclosed, nact);
      qri->ax_plus_y(2.0, qijd2->back_transform(ztrans));

    } else {
      shared_ptr<DFFullDist> qijd = qij->apply_closed_2RDM();
      qijd->ax_plus_y(2.0, halfjj->compute_second_transform(ztrans)->apply_closed_2RDM());
      qri = qijd->back_transform(ocoeff);

      shared_ptr<const DFFullDist> qijd2 = qij->apply_closed_2RDM();
      qri->ax_plus_y(2.0, qijd2->back_transform(ztrans));
    }
  }

  // D1 part. 2.0 seems to come from the difference between smith and bagel (?)
  qri->ax_plus_y(2.0, fulld1->apply_J()->back_transform(coeff));

  // contributions from non-separable part
  shared_ptr<Matrix> qq  = qri->form_aux_2index(halfjj, 1.0);
  shared_ptr<DFDist> qrs = qri->back_transform(ocoeff);

  // separable part
  auto separable_pair = [&,this](shared_ptr<const Matrix> d0occ, shared_ptr<const Matrix> d1bas) {
    shared_ptr<const Matrix> d0ao = make_shared<Matrix>(ocoeff * *d0occ ^ ocoeff);
    shared_ptr<const Matrix> d1ao = make_shared<Matrix>(*coeff * *d1bas ^ *coeff);
    shared_ptr<const VectorB> cd0 = geom_->df()->compute_cd(d0ao);
    shared_ptr<const VectorB> cd1 = geom_->df()->compute_cd(d1ao);

    // three-index derivatives (seperable part)...
    vector<shared_ptr<const VectorB>> cd {cd0, cd1};
    vector<shared_ptr<const Matrix>> dd {d1ao, d0ao};

    shared_ptr<DFHalfDist> sepd = halfjj->apply_density(d1ao);
    sepd->rotate_occ(d0occ);

    qrs->ax_plus_y(-1.0, sepd->back_transform(ocoeff)); // TODO this back transformation can be done together
    qrs->add_direct_product(cd, dd, 1.0);

    *qq += (*cd0 ^ *cd1) * 2.0;
    *qq += *halfjj->form_aux_2index(sepd, -1.0);
  };

  separable_pair(nact ? ref->rdm1_mat(task_->target()) : d0->get_submatrix(0,0,nocc,nocc), d1);

  if (ncore)
    separable_pair(smallz, dsa);

  // compute gradients
  shared_ptr<GradFile> gradient = contract_gradient(dtotao, xmatao, qrs, qq);
  gradient->print();
  // set energy
  energy_ = task_->energy();
  return gradient;
}


tuple<shared_ptr<Matrix>, shared_ptr<const DFFullDist>>
  CASPT2Grad::compute_Y(shared_ptr<const Matrix> dm1, shared_ptr<const Matrix> dm11, shared_ptr<const Matrix> dm2,
                        shared_ptr<const DFHalfDist> half, shared_ptr<const DFHalfDist> halfj, shared_ptr<const DFHalfDist> halfjj) {
  const int nclosed = ref_->nclosed();
  const int nact = ref_->nact();
  const int nocc = ref_->nocc();
  const int nvirt = ref_->nvirt();
  const int nall = nocc + nvirt;
  const int nmobasis = coeff_->mdim();
  assert(nall == nmobasis);

  const MatView ocmat = coeff_->slice(0, nocc);

  shared_ptr<const DFFullDist> full = halfj->compute_second_transform(coeff_);
  shared_ptr<const DFFullDist> fullo = halfj->compute_second_transform(ocmat);

  // Y_rs = 2[Y1 + Y2 + Y3(ri) + Y4 + Y5(ri)]
  shared_ptr<Matrix> out = make_shared<Matrix>(nmobasis, nmobasis);
  const MatView ocoeff = coeff_->slice(0, nocc);

  {
    // 2 Y1 = h(d0 + d1 + d2) * 2
    // one-electron contributions
    auto hmo = make_shared<const Matrix>(*coeff_ % *ref_->hcore() * *coeff_);
    shared_ptr<Matrix> d0;
    if (nact) {
      d0 = ref_->rdm1_mat(target_)->resize(nmobasis,nmobasis);
    } else {
      d0 = make_shared<Matrix>(nmobasis,nmobasis);
      for (int i = 0; i != nclosed; ++i)
        d0->element(i,i) = 2.0;
    }
    *out += *hmo * (*dm1 + *dm11 + *d0) * 2.0;
  }

  {
    // Y2 = Y2_rs = Y2_ri + Y2_ra, so making both at once
    shared_ptr<Matrix> dkl;
    if (nact) {
      dkl = ref_->rdm1_mat(target_);
      dkl->sqrt();
      dkl->scale(1.0/sqrt(2.0));
    }
    Fock<1> fock(geom_, ref_->hcore()->clone(), nullptr, dkl ? ocoeff * *dkl : Matrix(ocoeff), /*grad*/false, /*rhf*/true);
    *out += *coeff_ % fock * *coeff_ * *dm1 * 2.0;
  }

  {
    // 2 Y3 = 2 Y3_ri*dm0_ji
    // coulomb
    auto dmrao = make_shared<Matrix>(*coeff_ * *dm1 ^ *coeff_);
    auto jop = geom_->df()->compute_Jop(dmrao);
    // exchange
    auto kopi = halfjj->compute_Kop_1occ(dmrao, -0.5)->transpose();

    auto tmp = make_shared<Matrix>(*coeff_ % (*jop * ocoeff + *kopi));
    if (nact) {
      *tmp *= *ref_->rdm1_mat(target_);
    } else {
      *tmp *= 2.0;
    }
    out->add_block(2.0, 0, 0, nmobasis, nocc, tmp);
  }

  // TODO D1 must be parallelised as it is very big.
  // construct D1 to be used in Y4 and Y5
  auto D1 = make_shared<btas::Tensor4<double>>(nocc,nall,nocc,nall);
  fill(D1->begin(), D1->end(), 0.0);
  {
    // resizing dm2_(le,kf) to dm2_(lt,ks). no resort necessary.
    for (int s = 0; s != nall; ++s) // extend
      for (int k = ncore_; k != nocc; ++k)
        for (int t = 0; t != nall; ++t) // extend
          for (int l = ncore_; l != nocc; ++l) {
            if (t >= nclosed && s >= nclosed) {
              (*D1)(l, t, k, s) = dm2->element(l-ncore_+(nocc-ncore_)*(t-nclosed), k-ncore_+(nocc-ncore_)*(s-nclosed));
              // TODO this is not correct for general cases. Need some generic logic
              if ((k >= nclosed) ^ (l >= nclosed)) {
                (*D1)(l, t, k, s) += dm2->element(k-ncore_+(nocc-ncore_)*(s-nclosed), l-ncore_+(nocc-ncore_)*(t-nclosed));
              }
            }
          }
  }

  // fullks will be reused for gradient contraction
  shared_ptr<const DFFullDist> fullks;
  {
    // 2 Y4 =  2 K^{kl}_{rt} D^{lk}_{ts} = 2 (kr|lj) D0_(lj,ki) +  2 (kr|lt) D1_(lt,ks)
    // construct stepwise, D1 part
    fullks = full->apply_2rdm(*D1);
    *out += *full->form_2index(fullks, 2.0);
    // D0 part
    shared_ptr<const DFFullDist> fulld = nact ? fullo->apply_2rdm(*ref_->rdm2(target_), *ref_->rdm1(target_), nclosed, nact)
                                              : fullo->apply_closed_2RDM();
    out->add_block(2.0, 0, 0, nmobasis, nocc, full->form_2index(fulld, 1.0));
  }

  {
    // 2 Y5 = 2 Y5_ri = 2 Ybar (Gyorffy)  = 2 (rs|tj) D^ij_st = 2 (rl|jk) D0_(il,jk) + 2 (rs|tj) D1_(is,jt)]
    // construct stepwise, D1 part
    shared_ptr<const DFHalfDist> dfback = fullks->apply_J()->back_transform(coeff_);
    auto y5ri_ao = ref_->geom()->df()->form_2index(dfback, 1.0);
    out->add_block(2.0, 0, 0, nmobasis, nocc, *coeff_ % *y5ri_ao);
  }

  return make_tuple(out, fullks);
}

