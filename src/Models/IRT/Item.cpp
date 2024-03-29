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
#include "Item.hpp"
#include "Subject.hpp"
#include <iomanip>
#include <sstream>
#include <distributions.hpp>
#include <Models/Glm/Glm.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>

namespace BOOM{
  namespace IRT{


    typedef std::vector<string> StringVec;

    inline Ptr<CatKey> make_resp(uint Mscore){
      StringVec ans(Mscore+1);
      for(uint i=0; i<=Mscore; ++i){
	ostringstream s;
	s << i;
	(ans)[i] = s.str();
	//	cout << "ans[" << i << "] = " << ans[i] << endl;
      }
      return new CatKey(ans);
    }

    Item::Item(const string &Id, uint Mscore, uint one_subscale, uint nscales, const string &Name)
      : subscales_(nscales, false),
	id_(Id),
	name_(Name),
	possible_responses_(make_resp(Mscore))
    {
      //      cout << "__________________" << Id << "__________________" <<endl;
      subscales_.add(one_subscale);
      if(Name=="") name_=id_;
    }


    Item::Item(const string &Id, uint Mscore, std::vector<bool> subscales, const string &Name)
      : subscales_(subscales),
	id_(Id),
	name_(Name),
	possible_responses_(make_resp(Mscore))
    {
      if(Name=="") name_=id_;
    }


    Item::Item(const Item &rhs)
      : Model(rhs),
        MLE_Model(rhs),
	DataPolicy(rhs),
	LoglikeModel(rhs),
	subscales_(rhs.subscales_),
	id_(rhs.id_),
	possible_responses_(rhs.possible_responses_)
    {}

    //    Item * Item::clone()const{return new Item(*this);}

    void Item::report(ostream &out, uint namewidth)const{
      string plural = nscales_this()==1 ? " subscale:  " : " subscales: ";
      string name_paren = " (" + name().substr(0,namewidth) + ")";
      out << "Item " << std::setw(4) << id();
      if (namewidth>0) out << std::setw(namewidth + 3);
      out << name_paren << "  assesses " << nscales_this() << plural;
      for(uint i=0; i<nscales_this(); ++i) out << subscales_.indx(i) << " ";
      out << endl;
    }

    void Item::increment_hist(Ptr<Subject> s, Vec &ans)const{
      Response r = s->response(const_cast<Item*>(this));
      ++ans[r->value()];
    }

    Vec Item::response_histogram()const{
      // 0.. maxscore are valid indices
      Vec ans(maxscore()+1, 0.0);
      typedef SubjectSet::const_iterator It;

      for(It it = subjects().begin();
          it!= subjects().end();
          ++it){
        increment_hist(*it, ans);
      }
      return ans;
    }

    uint Item::nscales_this()const{return subscales_.nvars();}
    uint Item::Nscales()const{return subscales_.nvars_possible();}
    const Selector & Item::subscales()const{ return subscales_;}
    uint Item::maxscore()const{
      return possible_responses().size()-1; //  0 is a potential value
    }
    uint Item::nlevels()const{return possible_responses().size();}

    bool Item::assigned_to_subject(Ptr<Subject> s)const{
      SubjectLess sl;
      const SubjectSet & Sub(subjects());
      return std::binary_search(Sub.begin(), Sub.end(), s, sl);}

    void Item::add_subject(Ptr<Subject> s){
      add_data(s); }

    void Item::add_data(Ptr<Data> dp){
      add_data(DAT(dp));}

    void Item::add_data(Ptr<Subject> s){
      BOOM::IRT::add_subject(dat(),s);
    }

    void Item::clear_data(){
      DataPolicy::clear_data();}

    void Item::remove_subject(Ptr<Subject> s){
      DatasetType::iterator it =
	std::lower_bound(dat().begin(), dat().end(), s);
      if(it==dat().end() || *it!=s) return;
      dat().erase(it);
    }

    const SubjectSet & Item::subjects()const{
      return DataPolicy::dat();}

    uint Item::Nsubjects()const{
      return dat().size();}

    const string & Item::id()const{return id_;}
    const string & Item::name()const{return name_;}

    Response Item::make_response(const string &s)const{
      return new OrdinalData(s, possible_responses_);
    }

    Response Item::make_response(uint m)const{
      return new OrdinalData(m, possible_responses_);
    }

    Response Item::response(Ptr<Subject> s){
      return s->response(this);}
    const Response Item::response(Ptr<Subject> s)const{
      const Ptr<Item> that(const_cast<Item *>(this));
      return s->response(that);}

    void Item::set_response_names(const StringVec &levels){
      possible_responses_->relabel(levels);}

    const StringVec & Item::possible_responses()const{
      return possible_responses_->labels(); }


    Response Item::simulate_response(const Vec &Theta)const{
      uint M = maxscore();
      Vec probs(M+1);  // 0.. max score
      for(uint m=0; m<=M; ++m) probs[m] = response_prob(m, Theta, false);
      uint m = rmulti(probs);
      return make_response(m);
    }

    double Item::pdf(Ptr<Data> dp, bool logsc)const{
      return pdf(DAT(dp), logsc); }

    double Item::pdf(Ptr<Subject> s, bool logsc)const{
      const Vec &Theta(s->Theta());
      Ptr<Item> it(const_cast<Item *>(this));
      Response r = s->response(it);
      return response_prob(r->value(), Theta, logsc);   }

    void Item::increment_loglike(Ptr<Subject> s)const{
      loglike_ans+= this->pdf(s, true); }

    double Item::loglike()const{
      const SubjectSet &subjects(this->subjects());
      loglike_ans=0.0;
      typedef SubjectSet::const_iterator It;
      for(It it = subjects.begin(); it!=subjects.end(); ++it){
        increment_loglike(*it);
      }
      return loglike_ans;
    }

    ostream & Item::display(ostream & out)const{
      out << id() << "\t" << name() << "\t";
      for(uint i=0; i<subscales_.nvars_possible(); ++i){
	out << subscales_[i] << "\t";
      }
      display_item_params(out);
      out << endl;
      return out;
    }

  }
}
