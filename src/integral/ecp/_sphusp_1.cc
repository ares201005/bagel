//
// BAGEL - Parallel electron correlation program.
// Filename: _sphusp_1.cc
// Copyright (C) 2014 Toru Shiozaki
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

#include <algorithm>
#include <cassert>
#include <src/integral/ecp/sphusplist.h>

using namespace std;
using namespace bagel;

vector<double> SphUSPList::sphusp_1(const int m) {

  vector<double> c;
  constexpr double coeff[9] = {   0.000000000000000e+00,   4.886025119029199e-01,   0.000000000000000e+00,   0.000000000000000e+00,   0.000000000000000e+00,   4.886025119029199e-01,   4.886025119029199e-01,   0.000000000000000e+00,   0.000000000000000e+00};

  assert(abs(m) <= 1);
  const int size_c = (1 + 1) * (1 + 2) / 2;
  const int mu = m + 1;
  const int i0 = mu * size_c;
  for (int i = i0; i != i0 + size_c; ++i) c.push_back(coeff[i]);
  return c;

}
