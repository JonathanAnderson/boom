#ifndef BOOM_R_LIST_IO_HPP_
#define BOOM_R_LIST_IO_HPP_

#include <string>
#include <LinAlg/Vector.hpp>
#include <LinAlg/VectorView.hpp>
#include <LinAlg/Matrix.hpp>
#include <LinAlg/SubMatrix.hpp>
#include <LinAlg/Array.hpp>
#include <LinAlg/Types.hpp>

#include <Models/ParamTypes.hpp>
#include <Models/SpdParams.hpp>

#include <Interfaces/R/boom_r_tools.hpp>

//======================================================================
// Note that the functions listed here throw exceptions.  Code that
// uses them should be wrapped in a try-block where the catch
// statement catches the exception and calls Rf_error() with an
// appropriate error message.  The functions handle_exception(), and
// handle_unknown_exception (in handle_exception.hpp), are suitable
// defaults.  These try-blocks should be present in any code called
// directly from R by .Call.
//======================================================================

namespace BOOM{
  class RListIoElement;

  // An RListIoManager manages an R list that is used to store the
  // output of an MCMC run in BOOM, and to read it back in again for
  // doing posterior computations.  The class maintains a vector of
  // RListIoElement's each of which is responsible for managing a
  // single list element.
  //
  // The basic idiom for output is
  // RListIoManager io_manager;
  // io_manager.add_list_element(new VectorListElement(...));
  // io_manager.add_list_element(new PartialSpdListElement(...));
  // int niter = 1000;
  // SEXP ans;
  // PROTECT(ans = io_manager.prepare_to_write(niter));
  // for(int i = 0; i < niter; ++i){
  //   do_an_mcmc_iteration();
  //   io_manager.write();
  // }
  // UNPROTECT(1);
  // return ans;

  // The basic idiom for streaming through an already populated list is
  // RListIoManager io_manager;
  // io_manager.add_list_element(new VectorListElement(...));
  // io_manager.add_list_element(new SpdListElement(...));
  // io_manager.prepare_to_stream();
  // int niter = 1000;
  // io_manager.advance(100);  // discard some burn-in
  // for(int i = 0; i < niter; ++i){
  //   io_manager.stream();
  //   do_something_with_the_current_value();
  // }

  class RListIoManager {
   public:
    // The class takes over ownership of 'element', and deletes it
    // when *this goes out of scope.
    void add_list_element(RListIoElement *element);

    // Returns a list with the necessary names and storage for keeping
    // track of 'niter' parameters worth of output.
    SEXP prepare_to_write(int niter);

    // Takes an existing list as an argument, and gets each component
    // in elements_ ready to stream from it.
    void prepare_to_stream(SEXP object);

    // Each managed parameter writes its value to the appropriate
    // portion of the list, and then increments its position to get
    // ready for the next write.
    void write();

    // Each managed parameter reads its next value from the list, then
    // increments its position to get ready for the next read.
    void stream();

    // Each element moves forward n steps.  This is useful for
    // discarding the first 'n' elements in an MCMC sample.
    void advance(int n);
   private:
    std::vector<boost::shared_ptr<RListIoElement> > elements_;
  };

  //======================================================================
  // An RListIoelement takes care of allocating space, recording to,
  // and streaming parameters from an R list.  One instance is
  // required for each distinct parameter in the model output list.
  class RListIoElement {
   public:
    RListIoElement(const string &name);
    virtual ~RListIoElement();

    // Allocates and returns the R object (usually a vector, matrix,
    // or array), to be stored in the list.  It is the caller's
    // responsibility to PROTECT this object, if needed.  The 'caller'
    // will almost always be the RListIoManager, which has the
    // PROTECT/UNPROTECT stuff safely hidden away so the caller won't
    // need to worry about protecting the individual list elements.
    virtual SEXP prepare_to_write(int niter);

    // Takes the list as an argument.  Finds the list element that
    // this object is supposed to manage in the given object.  Set the
    // input buffers
    virtual void prepare_to_stream(SEXP object);

    // Leaf classes keep track of the position in the output buffer,
    // and increment it whenever write() is called.
    virtual void write() = 0;

    // Leaf classes keep track of the position in the input buffer,
    // and increment it whenever stream() is called.
    virtual void stream() = 0;

    // Return the name of the component in the list.
    const string &name()const;

    // Move position in stream forward by n places.
    void advance(int n);
   protected:
    // StoreBuffer must be called in derived classes to pass the SEXP
    // that manages the parameter to this base class.
    void StoreBuffer(SEXP buffer);
    SEXP rbuffer(){return rbuffer_;}

    // Calling next_position() returns the current position and
    // advances the counter.  If you need it more than once, be sure
    // to store it.
    int next_position();
    double *data();
   private:
    // Prohibit copy and assign
    RListIoElement(const RListIoElement &rhs);
    void operator=(const RListIoElement &rhs);

    string name_;
    SEXP rbuffer_;  // The R object holding the BOOM output
    int position_;  // Current position in the rbuffer
    double *data_;  // Pointer to the first element in the rbuffer
  };

  //----------------------------------------------------------------------
  // For tracking an individual diagonal element of a variance matrix.
  class PartialSpdListElement : public RListIoElement {
   public:
    PartialSpdListElement(Ptr<SpdParams> prm,
                          const string &param_name,
                          int which,
                          bool report_sd);
    virtual void write();
    virtual void stream();
   private:
    void CheckSize();
    Ptr<SpdParams> prm_;
    int which_;
    bool report_sd_;
  };

  //----------------------------------------------------------------------
  // For managing UnivariateParams, stored in an R vector.
  class UnivariateListElement : public RListIoElement {
   public:
    UnivariateListElement(Ptr<UnivParams>, const string &name);
    virtual void write();
    virtual void stream();
   private:
    Ptr<UnivParams> prm_;
  };

  //----------------------------------------------------------------------
  // A callback interface class for managing scalar quantities that
  // are not stored in a BOOM::Params object.  The purpose of a
  // ScalarIoCallback is to supply values for a
  // NativeUnivariateListElement to write to the vector that it
  // maintains.
  class ScalarIoCallback {
   public:
    virtual ~ScalarIoCallback(){}
    virtual double get_value()const=0;
  };

  // For managing scalar (double) output that is not stored in a
  // UnivParams.
  class NativeUnivariateListElement : public RListIoElement {
   public:
    // Args:
    //   callback: A pointer to the callback object responsible for
    //     obtaining values for the list element to write().  This can
    //     be NULL if the object is being created just for streaming.
    //     The object takes ownership of the pointer, so it should not
    //     be deleted manually.
    //   name:  The name of the component in the R list.
    //   streaming_buffer: A pointer to a double that can hold the
    //     streamed value.  If streaming_buffer is NULL then this
    //     component of the list will not be streamed.
    NativeUnivariateListElement(ScalarIoCallback *callback,
                                const string &name,
                                double *streaming_buffer = NULL);
    virtual SEXP prepare_to_write(int niter);
    virtual void prepare_to_stream(SEXP object);
    virtual void write();
    virtual void stream();
   private:
    boost::shared_ptr<ScalarIoCallback> callback_;
    double *streaming_buffer_;
    BOOM::VectorView vector_view_;
  };

  //----------------------------------------------------------------------
  // Use this class when BOOM stores a variance, but you want to
  // report a standard deviation.
  class StandardDeviationListElement : public RListIoElement {
   public:
    StandardDeviationListElement(
        Ptr<UnivParams>, const string &name);
    virtual void write();
    virtual void stream();
   private:
    Ptr<UnivParams> variance_;
  };

  //----------------------------------------------------------------------
  // For managing VectorParams, stored in an R matrix.
  class VectorListElement : public RListIoElement {
   public:
    VectorListElement(Ptr<VectorParams> m,
                      const string &param_name);
    // Allocate a matrix
    virtual SEXP prepare_to_write(int niter);
    virtual void prepare_to_stream(SEXP object);
    virtual void write();
    virtual void stream();
   private:
    void CheckSize();
    Ptr<VectorParams> prm_;
    SubMatrix matrix_view_;
  };

  //----------------------------------------------------------------------
  // For vectors with named components, such as factor levels or
  // variable names.
  class NamedVectorListElement : public VectorListElement {
   public:
    NamedVectorListElement(Ptr<VectorParams> m,
                           const string &param_name,
                           const std::vector<string> &element_names);
    virtual SEXP prepare_to_write(int niter);
   private:
    const std::vector<string> element_names_;
  };
  //----------------------------------------------------------------------
  // For reporting a vector of standard deviations when the model
  // stores a vector of variances.
  class SdVectorListElement : public RListIoElement {
   public:
    SdVectorListElement(Ptr<VectorParams> v,
                        const string &param_name);
    virtual SEXP prepare_to_write(int niter);
    virtual void prepare_to_stream(SEXP object);
    virtual void write();
    virtual void stream();
   private:
    void CheckSize();
    Ptr<VectorParams> prm_;
    SubMatrix matrix_view_;
  };

  //----------------------------------------------------------------------
  // A mix-in class for handling row and column names for list
  // elements that store MCMC draws of matrices.
  class MatrixListElementBase : public RListIoElement {
   public:
    MatrixListElementBase(const string &param_name)
        : RListIoElement(param_name) {}
    virtual int nrow() const = 0;
    virtual int ncol() const = 0;
    const std::vector<std::string> & row_names()const;
    const std::vector<std::string> & col_names()const;
    void set_row_names(const std::vector<std::string> &row_names);
    void set_col_names(const std::vector<std::string> &row_names);
   protected:
    // Children of this class should call set_buffer_dimnames(buffer)
    // when they prepare_to_write().
    void set_buffer_dimnames(SEXP buffer);
   private:
    std::vector<std::string> row_names_;
    std::vector<std::string> col_names_;
  };

  //----------------------------------------------------------------------
  // For managing MatrixParams, stored in an R 3-way array.
  class MatrixListElement : public MatrixListElementBase {
   public:
    MatrixListElement(Ptr<MatrixParams> m,
                      const string &param_name);

    // Allocate an array to hold the matrix draws.
    virtual SEXP prepare_to_write(int niter);
    virtual void prepare_to_stream(SEXP object);
    virtual void write();
    virtual void stream();

    virtual int nrow()const;
    virtual int ncol()const;
   private:
    void CheckSize();
    Ptr<MatrixParams> prm_;
    ArrayView array_view_;
  };

  //----------------------------------------------------------------------
  // For managing SpdParams, stored in an R 3-way array.
  class SpdListElement : public MatrixListElementBase {
   public:
    SpdListElement(Ptr<SpdParams> m,
                      const string &param_name);

    // Allocate an array to hold the matrix draws.
    virtual SEXP prepare_to_write(int niter);
    virtual void prepare_to_stream(SEXP object);
    virtual void write();
    virtual void stream();

    virtual int nrow()const;
    virtual int ncol()const;
   private:
    void CheckSize();
    Ptr<SpdParams> prm_;
    ArrayView array_view_;
  };

  //----------------------------------------------------------------------
  // A VectorIoCallback is a base class for managing native BOOM Vec
  // objects that are not part of VectorParams.  To use it, define a
  // local class that inherits from VectorIoCallback.  The class
  // should store a pointer to the object you really care about, and
  // which can supply the two necessary member functions.  Then put
  // the callback into a NativeVectorListElement, described below.
  class VectorIoCallback{
   public:
    virtual ~VectorIoCallback(){}
    virtual int dim()const=0;
    virtual Vec get_vector()const=0;
  };

  // A NativeVectorListElement manages a native BOOM Vec that is not
  // stored in a VectorParams.
  class NativeVectorListElement : public RListIoElement{
   public:
    // Args:
    //   callback: supplied access to the vectors that need to be
    //     recorded.  This can be NULL if the object is being created
    //     for streaming.  If it is non-NULL then this class takes
    //     ownership and deletes the callback on destruction.
    //   name:  the name of the entry in the R list.
    //   streaming_buffer: A pointer to a BOOM Vector/Vec that will
    //     receive the contents of the R list when streaming.  This
    //     can be NULL if streaming is not desired.
    NativeVectorListElement(VectorIoCallback *callback,
                         const string &name,
                         Vec *streaming_buffer);
    virtual SEXP prepare_to_write(int niter);
    virtual void prepare_to_stream(SEXP object);
    virtual void write();
    virtual void stream();
   private:
    boost::shared_ptr<VectorIoCallback> callback_;
    Vec *streaming_buffer_;
    SubMatrix matrix_view_;
  };

  //----------------------------------------------------------------------
  // Please see the comments to VectorIoCallback, above.
  class MatrixIoCallback{
   public:
    virtual ~MatrixIoCallback(){}
    virtual int nrow()const = 0;
    virtual int ncol()const = 0;
    virtual Mat get_matrix()const=0;
  };

  // A NativeMatrixListElement manages a BOOM Mat/Matrix that is not
  // stored in a MatrixParams.
  class NativeMatrixListElement : public MatrixListElementBase{
   public:
    // Args:
    //   callback: supplies access to the matrices that need
    //     recording.  This can be NULL if the object is being created
    //     simply for streaming.  If it is non-NULL, this class takes
    //     ownership and deletes the callback on destruction.
    //   name: the name of the component in the list.
    //   streaming_buffer: A pointer to a BOOM matrix that will
    //     receive the contents of the R list when streaming.  This
    //     can be NULL if streaming is not desired.
    //
    // Note that it is pointless to create this object if both
    // callback and streaming_buffer are NULL.
    NativeMatrixListElement(MatrixIoCallback *callback,
                            const string &name,
                            Mat *streaming_buffer);
    virtual SEXP prepare_to_write(int niter);
    virtual void prepare_to_stream(SEXP object);
    virtual void write();
    virtual void stream();

    virtual int nrow()const;
    virtual int ncol()const;
   private:
    boost::shared_ptr<MatrixIoCallback> callback_;
    Mat *streaming_buffer_;
    ArrayView array_view_;
  };

}  // namespace BOOM

#endif  // BOOM_R_LIST_IO_HPP_
