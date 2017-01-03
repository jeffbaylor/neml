#ifndef PARSE_H
#define PARSE_H

#include "neml.h"

#include <string>

#include <libxml++/libxml++.h>

namespace neml {

/// Main entry to parse model from xml file
std::shared_ptr<NEMLModel> parse_xml(std::string fname, std::string mname, 
                                     int & ier);

/// Setup a model from a root node
std::shared_ptr<NEMLModel> make_from_node(const xmlpp::Element * node, int & ier);

/// Setup a small strain model
std::shared_ptr<NEMLModel> process_smallstrain(const xmlpp::Element * node, int & ier);

} // namespace neml

#endif // PARSE_H
