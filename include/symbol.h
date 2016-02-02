#ifndef MXNETCPP_SYMBOL_H
#define MXNETCPP_SYMBOL_H

#include "base.h"
#include "mxnet.h"
#include "ndarray.h"

namespace mxnet {
namespace cpp {

class Executor;

/*!
* \brief struct to store SymbolHandle
*/
struct SymBlob {
public:
  /*!
  * \brief default constructor
  */
  SymBlob() : handle_(nullptr) {}
  /*!
  * \brief construct with SymbolHandle to store
  */
  explicit SymBlob(SymbolHandle handle) : handle_(handle) {}
  /*!
  * \brief destructor, free the SymbolHandle
  */
  ~SymBlob() { MXSymbolFree(handle_); }
  /*!
  * \brief the SymbolHandle to store
  */
  SymbolHandle handle_;

private:
  SymBlob(const SymBlob &);
  SymBlob &operator=(const SymBlob &);
};

/*!
* \brief Symbol interface
*/
class Symbol {
public:
  // TODO(zhangcheng-qinyinghua)
  // add more input in a single operator
  /*!
  * \brief construct a Symbol with SymbolHandle
  * \param handle the given SymbolHandle
  */
  explicit Symbol(SymbolHandle handle);
  /*!
  * \brief construct a variable Symbol
  * \param name the name of the variable
  */
  explicit Symbol(const std::string &name);
  // TODO(zhangcheng-qinyinghua)
  // implement all the operators
  Symbol operator+(const Symbol &rhs);
  Symbol operator-(const Symbol &rhs);
  Symbol operator*(const Symbol &rhs);
  Symbol operator/(const Symbol &rhs);
  Symbol Copy() const;
  /*!
  * \brief construct a variable Symbol
  * \param name the name of the variable
  */
  static Symbol Variable(const std::string &name = "");
  /*!
  * \return the SymbolHandle
  */
  SymbolHandle GetHandle() const { return blob_ptr_->handle_; }
  /*!
  * \brief construct an operator Symbol, with given input Symbol and config
  * \param name the name of the Symbol
  * \param input_keys the vector of keys of the input
  * \param input_values the vector of the intput Symbols
  * \param config_keys the vector of keys of the config
  * \param config_values the vecotr of values of the config
  */
  Symbol(const std::string &operator_name, const std::string &name,
    std::vector<const char *> input_keys,
    std::vector<SymbolHandle> input_values,
    std::vector<const char *> config_keys,
    std::vector<const char *> config_values);
  /*!
  * \brief infer the shapes by providing shapes of known argument shapes.
  * \param arg_shapes map of argument name to shape of arguments with known shapes.
  * \param in_shapes used to store infered shapes of input arguments.
  * \param out_shapes used to store infered shapes of outputs.
  * \param aux_shapes use to store the infered shapes of auxiliary states
  */
  void InferShape(
    const std::map<std::string, std::vector<mx_uint> > &arg_shapes,
    std::vector<std::vector<mx_uint> > *in_shape,
    std::vector<std::vector<mx_uint> > *aux_shape,
    std::vector<std::vector<mx_uint> > *out_shape) const;
  /*!
  * \brief List the arguments names.
  *
  * The position of the returned list also corresponds to calling position in operator()
  * \return the arguments list of this symbol, they can be either named or unnamed (empty string).
  */
  std::vector<std::string> ListArguments() const;
  /*! \return get the descriptions of outputs for this symbol */
  std::vector<std::string> ListOutputs() const;
  /*! \return get the descriptions of auxiliary data for this symbol */
  std::vector<std::string> ListAuxiliaryStates() const;
  /*!
  * \brief infer and construct all the arrays to bind to executor by providing some known arrays.
  * \param context the context of all the infered arrays
  * \param arg_arrays infered input arguments arrays.
  * \param arad_arrays infered arrays to store the gradient output of the input arguments.
  * \param aux_arrays infered arrays that is used as internal state in op.
  * \param args_map map of some given arguments arrays.
  * \param args_grad_store map of some gradient given store arrays.
  * \param args_req_type map of some given type of gradient saving. Can only be in {kNullOp, kAddTo, kWriteTo}.
  */
  void InferExecutorArrays(
    const Context &context, std::vector<NDArray> *arg_arrays,
    std::vector<NDArray> *grad_arrays, std::vector<OpReqType> *grad_reqs,
    std::vector<NDArray> *aux_arrays,
    const std::map<std::string, NDArray> &args_map,
    const std::map<std::string, NDArray> &arg_grad_store =
    std::map<std::string, NDArray>(),
    const std::map<std::string, OpReqType> &grad_req_type =
    std::map<std::string, OpReqType>()) const;
  /*!
  * \brief infer and construct all the input arguments arrays to bind to executor by providing some known arguments arrays.
  * \param context the context of all the infered arrays.
  * \param args_map map of all the infered input arguments arrays.
  * \param known_args map of some given arguments arrays.
  */
  void InferArgsMap(const Context &context,
    std::map<std::string, NDArray> *args_map,
    const std::map<std::string, NDArray> &known_args) const;
  /*!
  * \brief Create an executor by bind symbol with context and arguments.
  *  If user do not want to compute the gradients of i-th argument, grad_req_type[i] can be kNullOp.
  *  The input arrays in the given maps should have the same name with the input symbol.
  *  Only need some of the necessary arrays, and the other arrays can be infered automatically.
  *
  * \param context the context of binding.
  * \param args_map the NDArray that stores the input arguments to the symbol.
  * \param arg_grad_store NDArray that is used to store the gradient output of the input arguments.
  * \param grad_req_type requirment type of gradient saving. Can only be in {kNullOp, kAddTo, kWriteTo}.
  * \return a new executor, which need to be free manually.
  */
  Executor *SimpleBind(const Context &context,
    const std::map<std::string, NDArray> &args_map,
    const std::map<std::string, NDArray> &arg_grad_store =
    std::map<std::string, NDArray>(),
    const std::map<std::string, OpReqType> &grad_req_type =
    std::map<std::string, OpReqType>());
  /*!
  * \brief Create an executor by bind symbol with context and arguments.
  *  If user do not want to compute the gradients of i-th argument, grad_req_type[i] can be kNullOp.
  *
  * \param context the context of binding.
  * \param arg_arrays the NDArray that stores the input arguments to the symbol.
  * \param grad_arrays NDArray that is used to store the gradient output of the input arguments.
  * \param grad_reqs requirment type of gradient saving. Can only be in {kNullOp, kAddTo, kWriteTo}.
  * \param aux_arrays NDArray that is used as internal state in op
  * \return a new executor, which need to be free manually.
  */
  Executor *Bind(const Context &context, const std::vector<NDArray> &arg_arrays,
    const std::vector<NDArray> &grad_arrays,
    const std::vector<OpReqType> &grad_reqs,
    const std::vector<NDArray> &aux_arrays);

private:
  std::shared_ptr<SymBlob> blob_ptr_;
  static Mxnet* MxNet_;
};

}
}
#endif // MXNETCPP_SYMBOL_H