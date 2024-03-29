/*
  Copyright (C) 2005 Steven L. Scott

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
*/
#include "PosteriorSampler.hpp"
namespace BOOM{
  typedef PosteriorSampler PS;

  void intrusive_ptr_add_ref(PosteriorSampler *m){ m->up_count(); }
  void intrusive_ptr_release(PosteriorSampler *m){
    m->down_count(); if(m->ref_count()==0) delete m; }

  PS::PosteriorSampler()
      : rng_(seed_rng())
  {}

  PS::PosteriorSampler(const PS &rhs)
      : RefCounted(rhs)
  {
    rng_.seed(seed_rng(rhs.rng()));
  }

  void PosteriorSampler::set_seed(unsigned long s){
    rng_.seed(s);
  }
}
