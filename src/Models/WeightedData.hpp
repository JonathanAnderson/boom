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
#ifndef BOOM_WEIGHED_DATA_HPP
#define BOOM_WEIGHED_DATA_HPP
#include "DataTypes.hpp"

namespace BOOM{
  template <class DAT, class WGT=DoubleData>
  class WeightedData : virtual public Data{
    Ptr<DAT> dat_;
    Ptr<WGT> w_;
  public:
    typedef typename DAT::value_type value_type;
    typedef typename WGT::value_type weight_type;
    //    WeightedData(const value_type &x);
    WeightedData(Ptr<DAT> d, const weight_type & W);
    WeightedData(Ptr<DAT> d, Ptr<WGT> W);
    WeightedData(const WeightedData &rhs);
    WeightedData * clone()const{return new WeightedData(*this);}

    virtual uint size(bool minimal=true)const;
    virtual ostream & display(ostream &out)const;
    //    virtual istream & read(istream &in);

    virtual void set_weight(const weight_type & w);
    const weight_type & weight()const;
    virtual void set(const value_type &v){dat_->set(v);}
    virtual const value_type &value()const{return dat_->value();}
  };

  typedef WeightedData<VectorData> WeightedVectorData;
  typedef WeightedData<DoubleData> WeightedDoubleData;

  //------------------------------------------------------------

//   template<class D, class W>
//   WeightedData<D>::WeightedData(const value_type &x)
//     : dat_ (new D(x)),
//       w_ (new WGT)
//   {}

  template <class D, class W>
  WeightedData<D,W>::WeightedData(Ptr<D> d, const weight_type &w)
    : dat_(d),
      w_(new W(w))
    {}

  template <class D, class W>
  WeightedData<D,W>::WeightedData(Ptr<D> d, Ptr<W> w )
    : dat_(d),
      w_(w)
    {}

  template <class D, class W>
  WeightedData<D,W>::WeightedData(const WeightedData &rhs)
    : Data(rhs),
      dat_(rhs.dat_->clone()),
      w_(rhs.w_->clone())
  {}

  template <class D, class W>
  uint WeightedData<D,W>::size(bool minimal)const{
    return dat_->size(minimal)+ w_->size(minimal);}

  template <class D, class W>
  ostream & WeightedData<D,W>::display(ostream &out)const{
    w_->display(out);
    out << " ";
    dat_->display(out);
    return out;
  }

//   template <class D, class W>
//   istream & WeightedData<D,W>::read(istream &in){
//     w_->read(in);
//     dat_->read(in);
//     return in;
//   }

  template <class D, class W>
  void WeightedData<D,W>::set_weight(const weight_type& w){w_->set(w);}

  template <class D, class W>
  const typename W::value_type & WeightedData<D,W>::weight()const{
    return w_->value();}

}
#endif// BOOM_WEIGHED_DATA_HPP
