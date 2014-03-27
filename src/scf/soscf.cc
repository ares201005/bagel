//
// BAGEL - Parallel electron correlation program.
// Filename: soscf.cc
// Copyright (C) 2013 Toru Shiozaki
//
// Author: Hai-Anh Le <anh@u.northwestern.edu> 
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

#include <src/scf/soscf.h>
#include <src/scf/sofock.h>
#include <src/math/diis.h>
#include <src/scf/atomicdensities.h>

#include <iostream>
#include <src/integral/rys/r0batch.h>
#include <src/integral/rys/r1batch.h>
#include <src/integral/rys/r2batch.h>
#include <src/integral/rys/naibatch.h>
#include <src/integral/os/overlapbatch.h>
#include <src/molecule/mixedbasis.h>


using namespace std;
using namespace bagel;

BOOST_CLASS_EXPORT_IMPLEMENT(SOSCF)

SOSCF::SOSCF(const shared_ptr<const PTree> idata, const shared_ptr<const Geometry> geom, const shared_ptr<const Reference> re)
 : SCF_base(idata, geom, re) {
  cout << indent << "*** Two-component ECP-SCF ***" << endl << endl;
  soeig_ = unique_ptr<double[]> (new double[geom_->nbasis() * 2]);
  sohcore_base_ = make_shared<const SOHcore_base>(geom);
  sohcore_ = make_shared<SOHcore>(geom_, sohcore_base_);

  /**** Test Projection Integrals ****/

  auto mol = make_shared<const Molecule>(geom_->aux_atoms(), geom_->aux_atoms());
  vector<shared_ptr<const Atom>> auxatom;
  auxatom.push_back(mol->atoms(0));
  auto auxmol = make_shared<const Molecule>(auxatom, auxatom);
  //auxmol->print_atoms();

  vector<shared_ptr<const Atom>> refatom;
  refatom.push_back(geom_->atoms(1));
  auto refgeom = make_shared<const Geometry>(refatom, make_shared<const PTree>());
  refgeom->print_atoms();

  vector<shared_ptr<const Atom>> refatom2;
  refatom2.push_back(geom_->atoms(2));
  auto refgeom2 = make_shared<const Geometry>(refatom2, make_shared<const PTree>());
  refgeom2->print_atoms();

  auto auxgeom = make_shared<const Geometry>(auxatom, make_shared<const PTree>());
  MixedBasis<OverlapBatch> mixedSra(auxgeom, refgeom);
  mixedSra.print("< r | a >", 21);
  cout << "ndim = " << mixedSra.ndim() << " mdim = " << mixedSra.mdim() << endl;
  MixedBasis<OverlapBatch> mixedSsa(refgeom2, auxgeom);
  mixedSsa.print("< s | a >", 21);
  cout << "ndim = " << mixedSsa.ndim() << " mdim = " << mixedSsa.mdim() << endl;
  MixedBasis<OverlapBatch> S(auxgeom, auxgeom);
//S.print("< a | a >", 21);
//cout << "ndim = " << S.ndim() << " mdim = " << S.mdim() << endl;
  S.inverse();
#if 1
  (mixedSra * S * mixedSsa).print(" < r | a > S^{-1} < a | s > ", 21);
#endif

  MixedBasis<OverlapBatch> overlap(refgeom, refgeom2);
  overlap.print(" < r | s > ", 21);

}

void SOSCF::initial_guess() {
  sooverlap_ = SOSCF::sooverlap();
  sotildex_ = SOSCF::sotildex();

  shared_ptr<const Matrix> sofock = sohcore_;
  shared_ptr<Matrix> intermediate = make_shared<Matrix>(*sotildex_ % *sofock * *sotildex_);
  intermediate->diagonalize(soeig_.get());
  socoeff_ = make_shared<Coeff>(*sotildex_ * *intermediate);
  aodensity_ = socoeff_->form_density_rhf(nocc_ * 2);
}

void SOSCF::compute() {
  initial_guess();

  DIIS<Matrix> diis(diis_size_);

  for (int iter = 0; iter != max_iter_; ++iter) {
    shared_ptr<const Matrix> sofock = make_shared<const SOFock> (geom_, sohcore_, socoeff_->slice(0, nocc_ * 2));
    energy_ = 0.25 * ((*sohcore_ + *sofock) * *aodensity_).trace() + geom_->nuclear_repulsion();
    auto error_vector = make_shared<const Matrix>(*sofock * *aodensity_ * *sooverlap_ - 
                                                  *sooverlap_ * *aodensity_ * *sofock);
    const double error = error_vector->rms();

    cout << indent << setw(5) << iter << setw(20) << fixed << setprecision(8) << energy_ << endl;
    if (error < thresh_scf_) {
      cout << indent << endl << indent << "  * SOSCF iteration converged." << endl << endl;
      break;
    } else if (iter == max_iter_-1) {
      cout << indent << endl << indent << "  * Max iteration reached in SOSCF." << endl << endl;
      break;
    }

    if (iter >= diis_start_) {
      sofock = diis.extrapolate(make_pair(sofock, error_vector));
    }

    shared_ptr<Matrix> intermediate = make_shared<Matrix>(*sotildex_ % *sofock * *sotildex_);
    intermediate->diagonalize(soeig_.get());
    socoeff_ = make_shared<Coeff>(*sotildex_ * *intermediate);
    aodensity_ = socoeff_->form_density_rhf(nocc_ * 2);
  }
}

shared_ptr<const Matrix> SOSCF::sotildex() {
  shared_ptr<Matrix> out = make_shared<Matrix>(2 * tildex_->ndim(), 2 * tildex_->mdim());
  out->zero();
  out->add_block(1.0, 0, 0, tildex_->ndim(), tildex_->mdim(), tildex_);
  out->add_block(1.0, tildex_->ndim(), tildex_->mdim(), tildex_->ndim(), tildex_->mdim(), tildex_);
  return out;
}

shared_ptr<const Matrix> SOSCF::sooverlap() {
  shared_ptr<Matrix> out = make_shared<Matrix>(2 * overlap_->ndim(), 2 * overlap_->mdim());
  out->zero();
  out->add_block(1.0, 0, 0, overlap_->ndim(), overlap_->mdim(), overlap_);
  out->add_block(1.0, overlap_->ndim(), overlap_->mdim(), overlap_->ndim(), overlap_->mdim(), overlap_);
  return out;
}

