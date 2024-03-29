#ifndef CLICKSTREAM_MODEL_HPP
#define CLICKSTREAM_MODEL_HPP

#include <LinAlg/Selector.hpp>
#include <LinAlg/SubMatrix.hpp>
#include <Models/MarkovModel.hpp>
#include <Models/Policies/CompositeParamPolicy.hpp>
#include <Models/Policies/IID_DataPolicy.hpp>
#include <Models/Policies/PriorPolicy.hpp>
#include <Models/PosteriorSamplers/MarkovConjSampler.hpp>
#include <Models/PosteriorSamplers/MarkovConjShrinkageSampler.hpp>
#include <distributions/rng.hpp>

#include "Stream.hpp"

namespace BOOM {

  class NestedHmm
      : public CompositeParamPolicy,
        public IID_DataPolicy<Clickstream::Stream>,
        public PriorPolicy
  {
   public:
    typedef Clickstream::Event Event;
    typedef Clickstream::Session Session;
    typedef Clickstream::Stream Stream;

    NestedHmm(const std::vector<Ptr<Stream> > & streams, int S2, int S1);
    NestedHmm(int S2, int S1, int S0);
    virtual NestedHmm * clone()const;

    // The mixture component for state H, h.
    Ptr<MarkovModel> mix(int H, int h);
    const Ptr<MarkovModel> mix(int H, int h)const;

    // The model for latent state transitions between events.
    Ptr<MarkovModel> event_model(int H);
    const Ptr<MarkovModel> event_model(int H)const;

    // The model for latent state transitions between sessions.
    Ptr<MarkovModel> session_model();
    const Ptr<MarkovModel> session_model()const;

    // Session level latent state dimension.
    int S2()const;

    // Event level latent state dimension.
    int S1()const;

    // Number of levels in the observed sequence of events, inluding
    // the end of session indicator
    int S0()const;

    int Nstreams()const;
    Ptr<Stream> stream(int i);

    // Encode_state and decode state map between the state spaces H* =
    // (H,h) and S* = (0, ... , S1xS2-1).  encode_state moves from H* to
    // S*, and decode_state moves back.
    int encode_state(int H, int h)const;
    void decode_state(int state, int &H, int &h)const;

    virtual double pdf(Ptr<Data>, bool)const;  // no more overload needed

    double loglike();
    double last_loglike()const;
    double last_logpost()const;
    double logpri()const;

    void set_loglike(double);
    void set_logpost(double);

    // Fit the model (find the MLE) using an EM algorithm.
    double EM(double epsilon, bool bayes=true, bool trace=false);

    ostream & write_suf(ostream &)const;

    // Sets the number of threads to use for data imputation.
    void set_threads(int n);

    double impute_latent_data();

    virtual std::vector<Ptr<Sufstat> > suf_vec()const;

    double fwd_bkwd(bool bayes=false, bool find_mode=true);
    double fwd(Ptr<Stream>)const;
    void bkwd_sampling(Ptr<Stream>);
    void bkwd_smoothing(Ptr<Stream>);

    virtual void complete_data_mode(bool bayes);
    virtual double logp(Ptr<Event>, int H, int h)const;
    virtual void update(int H, int h, Ptr<Event> event);
    virtual void update_mixture(int H, int h, Ptr<Event> event, double prob);
    virtual void randomize_starting_values();

    // computes the probability that a conversion occurs before the end
    // of a session.  conv_state[0..S0-1] is the observed state defining
    // a conversion.  return value is an S2 vector of S1*(S0-2) X S1*2
    // matrices.  Element [H][(h0,y0)][(h1,y1)] is the conditional
    // probability of being absorbed into state (h1,y1) given session
    // type H and initial state (h0,y0).
    //  std::vector<Mat> conditional_conversion_probs(const BOOM::include &abs)const;

    // initial distribution and transition matrix in (h,y) space
    Mat augmented_Q(int H)const;
    Vec augmented_pi0(int H)const;

    void print_params(ostream &out)const; // for debugging
    void print_event(ostream &out,
                     const char *msg,
                     Ptr<Stream> u,
                     Ptr<Session> session,
                     Ptr<Event> event,
                     int event_number) const;
    void print_filter(ostream &out, int j)const;
    //------------------------------------------------------------
   private:
    const int S0_;  // observed data size, including the EOS marker
    const int S1_;  // number of event types
    const int S2_;  // number of session types

    Ptr<MarkovModel> session_model_;
    std::vector<Ptr<MarkovModel> > event_model_;
    std::vector<std::vector<Ptr<MarkovModel> > > mix_;

    Ptr<UnivParams> loglike_;
    Ptr<UnivParams> logpost_;

    // stuff for the filter
    mutable std::vector<Mat> P;
    mutable  Vec pi_;
    mutable  Vec logpi0_;
    mutable  Vec logd_;
    const Vec one_;       // A vector of 1's
    mutable  Mat logQ1_;  // for the first obs in a session
    mutable  Mat logQ2_;  // for the subsequent observations

    RNG rng_;

    std::vector<Ptr<NestedHmm> > workers_;
    void setup();
    void pass_params_to_workers();
    void fill_logd(Ptr<Event>)const;
    void fill_big_Q()const;
    void start_thread_imputation();
    void start_thread_em();
    double initialize(Ptr<Event>)const;
    void check_filter_size(int n)const;
    ConstVectorView get_hinit(const Vec &pi, int H)const;
    Vec get_Hinit(const Vec &pi)const;
    ConstSubMatrix get_htrans(const Mat &P, int H)const;
    ConstSubMatrix get_block(const Mat &P, int H1, int H2)const;
    Mat get_Htrans(const Mat &P)const;
    double fwd_bkwd_with_threads(bool bayes=false, bool find_mode=true);
    double impute_latent_data_with_threads();

    double collect_threads();
    void clear_client_data();
    void allocate_data_to_workers();
    void add_worker(Ptr<NestedHmm> w);
    void clear_workers();
    RNG & rng(){return rng_;}
  };

  class NestedHmmDataImputer
      : public BOOM::PosteriorSampler{
   public:
    NestedHmmDataImputer(NestedHmm *mod) : m(mod) {}
    void draw(){m->impute_latent_data();}
    double logpri()const{return 0;}
   private:
    NestedHmm *m;
  };

}  // namespace BOOM

#endif// CLICKSTREAM_MODEL_HPP
