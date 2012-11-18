//
// BAGEL - Parallel electron correlation program.
// Filename: dfdist.cc
// Copyright (C) 2012 Toru Shiozaki
//
// Author: Toru Shiozaki <shiozaki@northwestern.edu>
// Maintainer: Shiozaki group
//
// This file is part of the BAGEL package.
//
// The BAGEL package is free software; you can redistribute it and\/or modify
// it under the terms of the GNU Library General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
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


#include <memory>
#include <src/util/taskqueue.h>
#include <src/util/constants.h>
#include <src/util/f77.h>
#include <src/df/dfdist.h>
#include <stdexcept>
#include <iostream>
#include <algorithm>
#include <cassert>
#include <chrono>
#include <iomanip>
#include <list>

using namespace std;
using namespace chrono;
using namespace bagel;



unique_ptr<double[]> ParallelDF::form_2index(shared_ptr<const ParallelDF> o, const double a, const bool swap) const {
  if (blocks_.size() != o->blocks_.size()) throw logic_error("illegal call of ParallelDF::form_2index");
  const size_t size = blocks_.front()->b2size()*o->blocks_.front()->b2size();
  unique_ptr<double[]> out(new double[size]);

  // loop over blocks
  for (auto i = blocks_.begin(), j = o->blocks_.begin(); i != blocks_.end(); ++i, ++j) { 
    unique_ptr<double[]> tmp = (!swap) ? (*i)->form_2index(*j, a) : (*j)->form_2index(*i, a);
    // accumulate
    daxpy_(size, 1.0, tmp, 1, out, 1);
  }
  return out;
}


unique_ptr<double[]> ParallelDF::form_4index(shared_ptr<const ParallelDF> o, const double a, const bool swap) const {
  if (blocks_.size() != o->blocks_.size()) throw logic_error("illegal call of ParallelDF::form_4index");
  const size_t size = blocks_.front()->b2size()*o->blocks_.front()->b2size() * blocks_.front()->b1size()*o->blocks_.front()->b1size();
  unique_ptr<double[]> out(new double[size]);

  // loop over blocks
  for (auto i = blocks_.begin(), j = o->blocks_.begin(); i != blocks_.end(); ++i, ++j) { 
    unique_ptr<double[]> tmp = (!swap) ? (*i)->form_4index(*j, a) : (*j)->form_4index(*i, a);
    // accumulate
    daxpy_(size, 1.0, tmp, 1, out, 1);
  }
  return out;
}


void DFDist::common_init(const vector<shared_ptr<const Atom> >& atoms0, const vector<shared_ptr<const Atom> >& atoms1,
                         const vector<shared_ptr<const Atom> >& aux_atoms, const double throverlap, const bool compute_inverse) {

#if 0
  // this will be distributed in the future.
  auto tp0 = high_resolution_clock::now();

  // 3index Integral is now made in DFBlock.
  vector<shared_ptr<const Shell> > ashell, b1shell, b2shell;
  for (auto& i : aux_atoms) ashell.insert(ashell.end(), i->shells().begin(), i->shells().end());
  for (auto& i : atoms1) b1shell.insert(b1shell.end(), i->shells().begin(), i->shells().end());
  for (auto& i : atoms0) b2shell.insert(b2shell.end(), i->shells().begin(), i->shells().end());

  // Decide how we distribute (dynamic distribution).
  // TODO we need a parallel queue server!
  data_ = shared_ptr<DFBlock>(new DFBlock(ashell, b1shell, b2shell, 0, 0, 0));

  // generates a task of integral evaluations
  vector<DFIntTask_OLD> tasks;
  data2_ = shared_ptr<Matrix>(new Matrix(naux_, naux_));

  int tmpa = 0;
  vector<int> aof;
  for (auto& i : ashell) { aof.push_back(tmpa); tmpa += i->nbasis(); }
  const shared_ptr<const Shell> b3(new Shell(atoms0.front()->shells().front()->spherical()));

  auto o0 = aof.begin();
  for (auto& b0 : ashell) {
    auto o1 = aof.begin();
    for (auto& b1 : ashell) {
      if (*o0 <= *o1)
        tasks.push_back(DFIntTask_OLD(array<shared_ptr<const Shell>,4>{{b1, b3, b0, b3}}, vector<int>{*o0, *o1}, this));
      ++o1;
    }
    ++o0;
  }

  // these shell loops will be distributed across threads
  TaskQueue<DFIntTask_OLD> tq(tasks);
  tq.compute(resources__->max_num_threads());
  auto tp1 = high_resolution_clock::now();
  cout << "       - time spent for integral evaluation  " << setprecision(2) << setw(10) << duration_cast<milliseconds>(tp1-tp0).count()*0.001 << endl;

  if (compute_inverse) data2_->inverse_half(throverlap);
  auto tp2 = high_resolution_clock::now();
  cout << "       - time spent for computing inverse    " << setprecision(2) << setw(10) << duration_cast<milliseconds>(tp2-tp1).count()*0.001 << endl;
#endif

}


pair<const double*, shared_ptr<RysInt> > DFDist::compute_batch(array<shared_ptr<const Shell>,4>& input) {
#ifdef LIBINT_INTERFACE
  shared_ptr<Libint> eribatch(new Libint(input));
#else
  shared_ptr<ERIBatch> eribatch(new ERIBatch(input, 2.0));
#endif
  eribatch->compute();
  return make_pair(eribatch->data(), eribatch);
}


unique_ptr<double[]> DFDist::compute_Jop(const double* den) const {
  // first compute |E*) = d_rs (D|rs) J^{-1}_DE
  unique_ptr<double[]> tmp0 = compute_cd(den);
  unique_ptr<double[]> out(new double[nbasis0_*nbasis1_]);
  fill(out.get(), out.get()+nbasis0_*nbasis1_, 0.0);
  // then compute J operator J_{rs} = |E*) (E|rs)
  for (auto& i : blocks_)
    dgemv_("T", i->asize(), nbasis0_*nbasis1_, 1.0, data_->get(), i->asize(), tmp0.get()+i->astart(), 1, 1.0, out.get(), 1);
  return out;
}


unique_ptr<double[]> DFDist::compute_cd(const double* den) const {
  unique_ptr<double[]> tmp0(new double[naux_]);
  unique_ptr<double[]> tmp1(new double[naux_]);
  // D = (D|rs)*d_rs
  for (auto& i : blocks_)
    dgemv_("N", i->asize(), nbasis0_*nbasis1_, 1.0, data_->get(), i->asize(), den, 1, 0.0, tmp0.get()+i->astart(), 1);
  // C = S^-1_CD D 
  dgemv_("N", naux_, naux_, 1.0, data2_->data(), naux_, tmp0.get(), 1, 0.0, tmp1.get(), 1);
  dgemv_("N", naux_, naux_, 1.0, data2_->data(), naux_, tmp1.get(), 1, 0.0, tmp0.get(), 1);
  return tmp0;
}


shared_ptr<DFHalfDist> DFDist::compute_half_transform(const double* c, const size_t nocc) const {
  shared_ptr<DFHalfDist> out(new DFHalfDist(shared_from_this(), nocc));
  for (auto& i : blocks_)
    out->add_block(i->transform_second(c, nocc));
  return out;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


shared_ptr<DFFullDist> DFHalfDist::compute_second_transform(const double* c, const size_t nocc) const {
  shared_ptr<DFFullDist> out(new DFFullDist(df_, nocc_, nocc));
  for (auto& i : blocks_)
    out->add_block(i->transform_third(c, nocc));
  return out;
}


shared_ptr<DFHalfDist> DFHalfDist::copy() const {
  shared_ptr<DFHalfDist> out(new DFHalfDist(df_, nocc_));
  for (auto& i : blocks_)
    out->add_block(i->copy());
  return out;
}


shared_ptr<DFHalfDist> DFHalfDist::clone() const {
  shared_ptr<DFHalfDist> out(new DFHalfDist(df_, nocc_));
  for (auto& i : blocks_)
    out->add_block(i->clone());
  return out;
}


shared_ptr<DFDist> DFHalfDist::back_transform(const double* c) const{
  shared_ptr<DFDist> out(new DFDist(df_));
  for (auto& i : blocks_)
    out->add_block(i->transform_second(c, df_->nbasis1(), true));
  return out;
}


void DFHalfDist::rotate_occ(const double* d) {
  for (auto& i : blocks_)
    i = i->transform_second(d, nocc_);
}


shared_ptr<DFHalfDist> DFHalfDist::apply_density(const double* den) const {
  shared_ptr<DFHalfDist> out(new DFHalfDist(df_, nocc_)); 
  for (auto& i : blocks_)
    out->add_block(i->transform_third(den, nbasis_));
  return out;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


shared_ptr<DFFullDist> DFFullDist::copy() const {
  shared_ptr<DFFullDist> out(new DFFullDist(df_, nocc1_, nocc2_));
  for (auto& i : blocks_)
    out->add_block(i->copy());
  return out;
}


shared_ptr<DFFullDist> DFFullDist::clone() const {
  shared_ptr<DFFullDist> out(new DFFullDist(df_, nocc1_, nocc2_));
  for (auto& i : blocks_)
    out->add_block(i->clone());
  return out;
}


void DFFullDist::daxpy(const double a, const DFFullDist& o) {
  if (blocks_.size() != o.blocks_.size()) throw logic_error("illegal call of DFFullDist::daxpy");
  auto ob = o.blocks_.begin();
  for (auto& i : blocks_) {
    i->daxpy(a, *ob);
    ++ob;
  }
}


void DFFullDist::daxpy(const double a, const DFHalfDist& o) {
  if (blocks_.size() != o.blocks_.size()) throw logic_error("illegal call of DFFullDist::daxpy");
  auto ob = o.blocks_.begin();
  for (auto& i : blocks_) {
    i->daxpy(a, *ob);
    ++ob;
  }
}


void DFFullDist::scale(const double a) {
  for (auto& i : blocks_)
    i->scale(a);
}


void DFFullDist::symmetrize() {
  for (auto& i : blocks_)
    i->symmetrize();
}


// AO back transformation (q|rs)[CCdag]_rt [CCdag]_su
shared_ptr<DFHalfDist> DFFullDist::back_transform(const double* c) const {
  shared_ptr<DFHalfDist> out(new DFHalfDist(df_, nocc1_));
  for (auto& i : blocks_)
    out->add_block(i->transform_third(c, df_->nbasis0(), true));
  return out;
}


// 2RDM contractions
shared_ptr<DFFullDist> DFFullDist::apply_closed_2RDM() const {
  shared_ptr<DFFullDist> out(new DFFullDist(df_, nocc1_, nocc2_));
  for (auto& i : blocks_)
    out->add_block(i->apply_rhf_2RDM());
  return out;
}


shared_ptr<DFFullDist> DFFullDist::apply_uhf_2RDM(const double* amat, const double* bmat) const {
  shared_ptr<DFFullDist> out(new DFFullDist(df_, nocc1_, nocc2_));
  for (auto& i : blocks_)
    out->add_block(i->apply_uhf_2RDM(amat, bmat));
  return out;
}


shared_ptr<DFFullDist> DFFullDist::apply_2rdm(const double* rdm, const double* rdm1, const int nclosed, const int nact) const {
  shared_ptr<DFFullDist> out(new DFFullDist(df_, nocc1_, nocc2_));
  for (auto& i : blocks_)
    out->add_block(i->apply_2RDM(rdm, rdm1, nclosed, nact));
  return out;
}


shared_ptr<DFFullDist> DFFullDist::apply_2rdm(const double* rdm) const {
  shared_ptr<DFFullDist> out(new DFFullDist(df_, nocc1_, nocc2_));
  for (auto& i : blocks_)
    out->add_block(i->apply_2RDM(rdm));
  return out;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if 0


shared_ptr<DF_Half> DF_Half::apply_J(shared_ptr<const DensityFit> d) const {
  if (!d->has_2index()) throw logic_error("apply_J called from an object without a 2 index integral (DF_Half)");
  unique_ptr<double[]> out(new double[nocc_*naux_*nbasis_]);
  dgemm_("N", "N", naux_, nocc_*nbasis_, naux_, 1.0, d->data2_->data(), naux_, data_->get(), naux_, 0.0, out.get(), naux_);
  return shared_ptr<DF_Half>(new DF_Half(df_, nocc_, out));
}


shared_ptr<DF_Half> DF_Half::apply_JJ(shared_ptr<const DensityFit> d) const {
  if (!d->has_2index()) throw logic_error("apply_J called from an object without a 2 index integral (DF_Half)");
  unique_ptr<double[]> jj(new double[naux_*naux_]);
  dgemm_("N", "N", naux_, naux_, naux_, 1.0, d->data2_->data(), naux_, d->data2_->data(), naux_, 0.0, jj.get(), naux_);

  unique_ptr<double[]> out(new double[nocc_*naux_*nbasis_]);
  dgemm_("N", "N", naux_, nocc_*nbasis_, naux_, 1.0, jj.get(), naux_, data_->get(), naux_, 0.0, out.get(), naux_);
  return shared_ptr<DF_Half>(new DF_Half(df_, nocc_, out));
}


unique_ptr<double[]> DF_Half::compute_Kop_1occ(const double* den) const {
  return apply_density(den)->form_2index(df_);
}


shared_ptr<DF_Full> DF_Full::apply_J(shared_ptr<const DensityFit> d) const {
  if (!d->has_2index()) throw logic_error("apply_J called from an object without a 2 index integral (DF_Full)");
  unique_ptr<double[]> out(new double[nocc1_*nocc2_*naux_]);
  dgemm_("N", "N", naux_, nocc1_*nocc2_, naux_, 1.0, d->data2_->data(), naux_, data_->get(), naux_, 0.0, out.get(), naux_);
  return shared_ptr<DF_Full>(new DF_Full(df_, nocc1_, nocc2_, out));
}


shared_ptr<DF_Full> DF_Full::apply_JJ(shared_ptr<const DensityFit> d) const {
  if (!d->has_2index()) throw logic_error("apply_J called from an object without a 2 index integral (DF_Full)");
  unique_ptr<double[]> jj(new double[naux_*naux_]);
  dgemm_("N", "N", naux_, naux_, naux_, 1.0, d->data2_->data(), naux_, d->data2_->data(), naux_, 0.0, jj.get(), naux_);

  unique_ptr<double[]> out(new double[nocc1_*nocc2_*naux_]);
  dgemm_("N", "N", naux_, nocc1_*nocc2_, naux_, 1.0, jj.get(), naux_, data_->get(), naux_, 0.0, out.get(), naux_);
  return shared_ptr<DF_Full>(new DF_Full(df_, nocc1_, nocc2_, out));
}


// for MP2-like quantities
void DF_Full::form_4index(unique_ptr<double[]>& target, const shared_ptr<const DF_Full> o, const size_t n) const {
  const int dim = nocc1_ * nocc2_;
  const int odim = o->nocc1_; // o->nocc2_ is fixed at n;
  const int naux = df_->naux();
  dgemm_("T", "N", dim, odim, naux, 1.0, data_->get(), naux, o->data_->get()+naux*odim*n, naux, 0.0, target.get(), dim);
}

unique_ptr<double[]> DF_Full::form_4index(const shared_ptr<const DF_Full> o, const size_t n) const {
  unique_ptr<double[]> target(new double[o->nocc1_*nocc1_*nocc2_]);
  form_4index(target, o, n);
  return move(target);
}


void DF_Full::set_product(const shared_ptr<const DF_Full> o, const unique_ptr<double[]>& c, const int jdim, const size_t off) {
  dgemm_("N", "N", naux(), jdim, nocc1()*nocc2(), 1.0, o->data_->get(), naux(), c.get(), nocc1()*nocc2(), 0.0, data_->get()+off, naux());
}


#endif


