#include "nemlerror.h"

#include <iostream>

namespace neml {

void py_error(int ier)
{
  switch(ier) {
    case SUCCESS: return;
    case INCOMPATIBLE_MODELS: throw std::runtime_error("Incompatible submodels");
    case LINALG_FAILURE: throw LinalgError("Generic linear algebra failure");
    case MAX_ITERATIONS: throw std::runtime_error("Maximum iteration count exceeded");
    case KT_VIOLATION: throw std::runtime_error("Integration of rate-independent model resulted in a violation of the Kuhn-Tucker conditions");
    case NODE_NOT_FOUND: throw std::runtime_error("XML node not found");
    case TOO_MANY_NODES: throw std::runtime_error("More than one XML node found");
    case ATTRIBUTE_NOT_FOUND: throw std::runtime_error("XML attribute not found");
    case UNKNOWN_TYPE: throw std::runtime_error("Unknown model type");

    default: throw std::runtime_error("Unknown error!");
  }
}

std::string string_error(int ier)
{
  switch(ier) {
    case SUCCESS: return "Success";
    case INCOMPATIBLE_MODELS: return "Incompatible submodels";
    case LINALG_FAILURE: return "Linear algebra call failed";
    case MAX_ITERATIONS: return "Maximum iteration count exceeded";
    case KT_VIOLATION: return "Integration of rate-independent model resulted in a violation of the Kuhn-Tucker conditions";
    case NODE_NOT_FOUND: return "XML node not found";
    case TOO_MANY_NODES: return "More than  one XML node found";
    case ATTRIBUTE_NOT_FOUND: return "XML attribute not found";
    case UNKNOWN_TYPE: return "Unknown model type";

    default: return "Unknown error";
  }
}

LinalgError::LinalgError(const char * m) :
    std::runtime_error(m)
{

}


} // namespace neml
