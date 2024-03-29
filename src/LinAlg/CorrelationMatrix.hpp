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

#ifndef NEW_LA_CORRELATION_MATRIX_H
#define NEW_LA_CORRELATION_MATRIX_H
#include "SpdMatrix.hpp"

namespace BOOM{

    class CorrelationMatrix : public SpdMatrix{
      // symmetric, positive definite Matrix with unit diagonal
    public:

      // need all the constructors from TNT
      CorrelationMatrix();
      CorrelationMatrix(int dim);
      CorrelationMatrix(int dim, double *m, bool ColMajor=true);
      template <class FwdIt>
      CorrelationMatrix(FwdIt Beg, FwdIt End);
      CorrelationMatrix(const Matrix &m);
      CorrelationMatrix(const CorrelationMatrix &sm);

      CorrelationMatrix & operator=(const CorrelationMatrix &x);
      CorrelationMatrix & operator=(const Matrix &x);

      virtual Vector vectorize(bool minimal=true)const;
      virtual void unvectorize(const Vector &v, bool minimal=true);
      virtual Vector::const_iterator unvectorize
      (Vector::const_iterator &b, bool minimal=true);

      virtual uint nelem()const;   // number of potentially distinct elements
      bool operator==(const CorrelationMatrix &rhs)const;
      bool operator!=(const CorrelationMatrix &rhs)const;
    };

    template <class FwdIt>
    CorrelationMatrix::CorrelationMatrix(FwdIt Beg, FwdIt End)
      : SpdMatrix(Beg,End)
    {}

    CorrelationMatrix var2cor(const SpdMatrix &v);
    SpdMatrix cor2var(const CorrelationMatrix &cor, const Vector &sd);
    CorrelationMatrix random_cor(uint n);
}
#endif // NEW_LA_CORRELATION_MATRIX_H
