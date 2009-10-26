/*
 * cabs.cc
 *
 *  Created on: Oct 22, 2009
 *      Author: shiozaki
 */

#include <src/pmp2/pmp2.h>
#include <src/pscf/poverlap.h>
#include <src/pscf/ptildex.h>
#include <src/pscf/phcore.h>

using namespace std;
using namespace boost;

typedef shared_ptr<PMatrix1e> RefMatrix;
typedef shared_ptr<PGeometry> RefGeom;
typedef shared_ptr<PHcore> RefHcore;
typedef shared_ptr<PMOFile<std::complex<double> > > RefPMOFile;

pair<RefMatrix, RefMatrix> PMP2::generate_CABS() {

  // Form RI space which is a union of OBS and CABS.
  RefGeom newgeom(new PGeometry(*geom_));
  union_geom_ = newgeom;
  union_geom_->merge_obs_cabs();

  shared_ptr<POverlap> union_overlap(new POverlap(union_geom_));
  shared_ptr<PTildeX> ri_coeff(new PTildeX(union_overlap));
  RefMatrix ri_reshaped(new PMatrix1e(coeff_, ri_coeff->ndim(), coeff_->mdim()));

  // SVD to project out OBS component. Note singular values are all 1 as OBS is a subset of RI space.
  RefMatrix tmp(new PMatrix1e(*ri_coeff % union_overlap->ft() * *ri_reshaped));
  RefMatrix U(new PMatrix1e(geom_, tmp->ndim(), tmp->ndim()));
  RefMatrix V(new PMatrix1e(geom_, tmp->mdim(), tmp->mdim()));
  tmp->svd(U, V);

  RefMatrix Ured(new PMatrix1e(U, tmp->mdim()));
  RefMatrix cabs_coeff(new PMatrix1e(*ri_coeff * *Ured));

  pair<RefMatrix, RefMatrix> cabs_coeff_spl = cabs_coeff->split(geom_->nbasis(), geom_->ncabs());

  return cabs_coeff_spl;
}


// Hartree-weighted (i.e., Fock except exchange) index space to be used in B intermediate evaluator.
pair<RefMatrix, RefMatrix> PMP2::generate_hJ_iA() {

  // Computes hcore in k-space.
  RefHcore union_hcore(new PHcore(union_geom_));
  RefMatrix hcore(new PMatrix1e(union_hcore->ft()));

  // "false" means it returns the k-space density matrix (-K <= k <= K).
  RefMatrix density(new PMatrix1e(coeff_->form_density_rhf(false)));

  RefMatrix coulomb = eri_ii_iA_->contract12(density);


  pair<RefMatrix, RefMatrix> out;
  return out;
}

