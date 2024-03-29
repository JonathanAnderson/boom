/*
  Copyright (C) 2005-2011 Steven L. Scott

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
#ifndef BOOM_NULL_DATA_POLICY_HPP
#define BOOM_NULL_DATA_POLICY_HPP
#include <Models/ModelTypes.hpp>
#include <Models/Policies/DataInfoPolicy.hpp>
#include <fstream>
#include <list>

namespace BOOM{
  // A class for models that don't have data (for whatever reason)
  class NullDataPolicy : virtual public Model{
   public:
    typedef NullDataPolicy DataPolicy;
   public:
    NullDataPolicy(){}
    virtual void add_data(Ptr<Data>){}
    virtual void clear_data(){}
    virtual void combine_data(const Model &, bool=true){}
   private:
  };
}
#endif //BOOM_NULL_DATA_POLICY_HPP
