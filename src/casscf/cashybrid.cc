//
// BAGEL - Parallel electron correlation program.
// Filename: casbfgs.h
// Copyright (C) 2014 Toru Shiozaki
//
// Author: Jefferson Bates <jefferson.bates@northwestern.edu>
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


#include <src/scf/scf.h>
#include <src/casscf/casscf.h>
#include <src/casscf/superci.h>
#include <src/casscf/casbfgs.h>
#include <src/casscf/cashybrid.h>

 using namespace std;
 using namespace bagel;

void CASHYBRID::compute() {

  shared_ptr<Method> active_method;
  // construct and compute SuperCI
  {
    auto idata = make_shared<PTree>(*idata_);
    idata->erase("maxiter");
    idata->erase("thresh");
    idata->put("maxiter", maxiter_sci_); 
    idata->put("thresh",  thresh_switch_); 
    active_method = make_shared<SuperCI>(idata, geom_, ref_);
    active_method->compute();
    refout_ = active_method->conv_to_ref();
  }

  // construct and compute step-restricted BFGS
  {
    active_method = make_shared<CASBFGS>(idata_, geom_, refout_);
    active_method->compute();
    refout_ = active_method->conv_to_ref();
  }

}


shared_ptr<const Reference> CASHYBRID::conv_to_ref() const {
  assert(refout_);
  return refout_; 
}
