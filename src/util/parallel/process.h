//
// BAGEL - Parallel electron correlation program.
// Filename: process.h
// Copyright (C) 2012 Toru Shiozaki
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

#ifndef __SRC_PARALLEL_PROCESS_H
#define __SRC_PARALLEL_PROCESS_H

#include <iostream>
#include <sstream>

namespace bagel {

class Process {
  protected:
    // original stream
    std::streambuf* cout_orig;
    // dummy stream
    std::stringstream ss_;

    int print_level_;

  public:
    Process();
    ~Process();

    void cout_on()  const;
    void cout_off() const;

    int print_level() const { return print_level_; }
    void set_print_level(const int i) { print_level_ = i; }

};

}

#endif

