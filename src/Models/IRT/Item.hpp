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
#ifndef IRT_ITEM_HPP
#define IRT_ITEM_HPP

#include "IRT.hpp"
#include <Models/ModelTypes.hpp>
#include <Models/Policies/IID_DataPolicy.hpp>

namespace BOOM{
  class GlmCoefs;
  namespace IRT{

    class Item
      : public IID_DataPolicy<Subject>,
	public LoglikeModel
    {
    public:
      friend class Subject;
      friend class IrtModel;

      typedef std::vector<string> StringVec;
      Item(const string &Id, uint Maxscore, uint one_subscale,
	   uint nscales, const string &Name="");
      Item(const string &Id, uint Maxscore, std::vector<bool> subscales,
	   const string &Name="");
      Item(const Item &rhs);

      virtual Item * clone()const=0;

      uint nscales_this()const;    // number of subscales assessed by this item
      uint Nscales()const;    // total number of subscales
      const Indicators & subscales()const;
      uint maxscore()const;     // maximum score possible on the item
      uint nlevels()const;      // number of possible repsonses: maxscore+1
      bool assigned_to_subject(Ptr<Subject> s)const;

      //--- data managment over-rides for Model base class ---
      virtual void add_subject(Ptr<Subject> s);
      virtual void remove_subject(Ptr<Subject> s);
      virtual void add_data(Ptr<Data>);
      virtual void add_data(Ptr<Subject>);
      virtual void clear_data();

      const SubjectSet &subjects()const;
      uint Nsubjects()const;

      const string & id()const;
      const string & name()const;

      // create/get responses
      Response make_response(const string &s)const;
      Response make_response(uint m)const;
      Response response(Ptr<Subject>);
      const Response response(Ptr<Subject>)const;

      void set_response_names(const StringVec &levels);
      const StringVec &possible_responses()const;

      void report(ostream &, uint namewidth=0)const;
      Vec response_histogram()const;

      ostream & display(ostream &)const;
      virtual ostream & display_item_params(ostream &,
					    bool decorate=true)const=0;

      Response simulate_response(const Vec &Theta)const;

      virtual const Vec & beta()const=0;

      virtual double pdf(Ptr<Data>, bool logsc)const;
      virtual double pdf(Ptr<Subject>, bool logsc)const;

      virtual double response_prob(Response r, const Vec &Theta,
				   bool logscale)const=0;
      virtual double response_prob(uint r, const Vec &Theta,
				   bool logscale)const=0;

      virtual double loglike()const;
    private:
      Indicators subscales_;      // which subscales does this item assess
      string id_;                 // internal id, like "17"
      string name_;               // external id, like "Toy Story"
      Ptr<CatKey> possible_responses_;  // "0", "1"... "Poor","Fair","Good"...
      void increment_hist(Ptr<Subject>, Vec &)const;
      void increment_loglike(Ptr<Subject>)const;
      mutable double loglike_ans;
    };

    //======================================================================
    //A "NullItem" is used by Subjects and IrtModels to help them
    //navigate their ItemSets

    class NullItem : public Item{
    public:
      NullItem() : Item("Null", 1, 0, 1, "Null"){}
      NullItem * clone()const{return new NullItem(*this);}
      ostream & display_item_params(ostream &out, bool=true)const{
	return out;}
      const Vec & beta()const{ return b;}
      double response_prob(Response, const Vec &, bool)const{return 0.0;}
      double response_prob(uint, const Vec &, bool)const{return 0.0;}
      double pdf(Ptr<Data>, bool)const{return 0.0;}
      double pdf(Ptr<Subject>, bool)const{return 0.0;}
      ParamVec t(){return ParamVec() ;}
      const ParamVec t()const{return ParamVec() ;}
      void initialize_params(){}
      void add_data(Ptr<Data>){}
      void add_data(Ptr<Subject>){}
      void clear_data(){}
      void sample_posterior(){}
      double logpri() const{return 0.0;}
      void set_method(Ptr<PosteriorSampler>){}
    private:
      Vec b;
    };

  }
}
#endif// IRT_ITEM_HPP
