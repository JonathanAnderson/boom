/*
  Copyright (C) 2007 Steven L. Scott

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
#ifndef BOOM_SCALAR_LOGPOST_TF_HPP
#define BOOM_SCALAR_LOGPOST_TF_HPP
#include <cpputil/Ptr.hpp>
#include <numopt.hpp>

namespace BOOM{
  class LoglikeModel;
  class DoubleModel;

  class ScalarLogpostTF{
  public:
    ScalarLogpostTF(LoglikeModel * m, Ptr<DoubleModel> pri);
    double operator()(const Vec &z)const;  // z is really a scalar
    double operator()(double z)const;
  protected:
  private:
    Target loglike;
    Ptr<DoubleModel> pri;
  };
}

#endif// BOOM_SCALAR_LOGPOST_TF_HPP
