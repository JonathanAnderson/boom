/*
  Copyright (C) 2008 Steven L. Scott

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
#include <iostream>
#include <iomanip>
#include <cpputil/string_utils.hpp>

namespace BOOM{
using namespace std;

std::ostream & print_columns(std::ostream &out, const std::vector<Svec> &columns, unsigned pad){
  unsigned nc = columns.size();
  std::vector<unsigned> widths;
  unsigned nr = 0;
  for(unsigned i=0; i<nc; ++i){
    unsigned w = 0;
    unsigned ni = columns[i].size();
    nr = std::max<unsigned>(nr, ni);
    for(unsigned j=0; j<ni; ++j) w = std::max<unsigned>(w, columns[i][j].size());
    w+= pad;
    widths.push_back(w);
  }

  for(unsigned i=0; i<nr; ++i){
    for(unsigned j=0; j<nc; ++j){
      out << setw(widths[j]);
      if(i<columns[j].size()) out << columns[j][i];
      else out << string(widths[j], ' ');
    }
    out << endl;
  }
  return out;
}

ostream & print_two_columns(ostream & out, const Svec & left, const Svec &right, uint pad){
  std::vector<Svec> cols;
  cols.push_back(left);
  cols.push_back(right);
  return print_columns(out, cols, pad);
}

}
