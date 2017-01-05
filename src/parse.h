#ifndef PARSE_H
#define PARSE_H

#include "neml.h"

#include <string>
#include <vector>
#include <algorithm>

#include <libxml++/libxml++.h>

namespace neml {

/// Main entry to parse model from xml file
std::shared_ptr<NEMLModel> parse_xml(std::string fname, std::string mname, 
                                     int & ier);

/// Setup a model from a root node
std::shared_ptr<NEMLModel> make_from_node(const xmlpp::Element * node, int & ier);

/// Setup a small strain model
std::shared_ptr<NEMLModel> process_smallstrain(const xmlpp::Element * node, int & ier);

/// Setup a linear elasticity model
std::shared_ptr<LinearElasticModel> process_linearelastic(const xmlpp::Element * node, int & ier);

/// Setup an isotropic linear elastic model
std::shared_ptr<LinearElasticModel> process_isotropiclinearelastic(const xmlpp::Element * node, int & ier);

/// Setup shear modulus
std::shared_ptr<ShearModulus> process_shearmodulus(const xmlpp::Element * node, int & ier);

/// Constant shear
std::shared_ptr<ShearModulus> process_constantshearmodulus(const xmlpp::Element * node, int & ier);

/// Polynomial shear
std::shared_ptr<ShearModulus> process_polynomialshearmodulus(const xmlpp::Element * node, int & ier);

/// Setup bulk modulus
std::shared_ptr<BulkModulus> process_bulkmodulus(const xmlpp::Element * node, int & ier);

/// Constant bulk
std::shared_ptr<BulkModulus> process_constantbulkmodulus(const xmlpp::Element * node, int & ier);

/// Polynomial bulk
std::shared_ptr<BulkModulus> process_polynomialbulkmodulus(const xmlpp::Element * node, int & ier);

/// Rate independent plasticity processing
std::shared_ptr<RateIndependentFlowRule> process_independent(const xmlpp::Element * node, int & ier);

/// Specific RI model
std::shared_ptr<RateIndependentFlowRule> process_rirule(const xmlpp::Element * node, int & ier);

/// Associative RI
std::shared_ptr<RateIndependentFlowRule> process_riassociative(const xmlpp::Element * node, int & ier);

/// Yield surfaces
std::shared_ptr<YieldSurface> process_surface(const xmlpp::Element * node, int & ier);

/// Isotropic J2
std::shared_ptr<YieldSurface> process_isoj2(const xmlpp::Element * node, int & ier);

/// Kinematic J2
std::shared_ptr<YieldSurface> process_isokinj2(const xmlpp::Element * node, int & ier);

/// Hardening rules
std::shared_ptr<HardeningRule> process_hardening(const xmlpp::Element * node, int & ier);

/// Isotropic hardening
std::shared_ptr<HardeningRule> process_isotropic(const xmlpp::Element * node, int & ier);

/// The actual tag (rather than type)
std::shared_ptr<HardeningRule> process_isotropictag(const xmlpp::Element * node, int & ier);

/// Linear isotropic hardening
std::shared_ptr<HardeningRule> process_linearisotropic(const xmlpp::Element * node, int & ier);

/// Voce isotropic hardening
std::shared_ptr<HardeningRule> process_voceisotropic(const xmlpp::Element * node, int & ier);

/// Kinematic hardening
std::shared_ptr<HardeningRule> process_kinematic(const xmlpp::Element * node, int & ier);

/// The actual tag (rather than type)
std::shared_ptr<HardeningRule> process_kinematictag(const xmlpp::Element * node, int & ier);

/// Linear kinematic hardening
std::shared_ptr<HardeningRule> process_linearkinematic(const xmlpp::Element * node, int & ier);

/// Combined hardening
std::shared_ptr<HardeningRule> process_combined(const xmlpp::Element * node, int & ier);

/// Nonassociative RI
std::shared_ptr<RateIndependentFlowRule> process_rinonassociative_hardening(const xmlpp::Element * node, int & ier);

/// Nonassociative hardening rule
std::shared_ptr<NonAssociativeHardening> process_nonass_hardening(const xmlpp::Element * node, int & ier);

/// Chaboche model
std::shared_ptr<NonAssociativeHardening> process_nonass_chaboche(const xmlpp::Element * node, int & ier);

/// Gamma models for Chaboche
std::shared_ptr<GammaModel> process_gammamodel(const xmlpp::Element * node, int & ier);

/// Constant gamma model
std::shared_ptr<GammaModel> process_constant_gamma(const xmlpp::Element * node, int & ier);

/// Saturating gamma model
std::shared_ptr<GammaModel> process_saturating_gamma(const xmlpp::Element * node, int & ier);

/// Rate dependent plasticity processing
std::shared_ptr<ViscoPlasticFlowRule> process_dependent(const xmlpp::Element * node, int & ier);

/// Specific RD model
std::shared_ptr<ViscoPlasticFlowRule> process_rdrule(const xmlpp::Element * node, int & ier);

/// Yaguchi Gr 91 (rate dependent) model processing
std::shared_ptr<ViscoPlasticFlowRule> process_rd_yaguchigr91(const xmlpp::Element * node, int & ier);

/// Chaboche (rate dependent) model processing
std::shared_ptr<ViscoPlasticFlowRule> process_rd_chaboche(const xmlpp::Element * node, int & ier);

/// Chaboche (rd) fluidity models
std::shared_ptr<FluidityModel> process_fluidity(const xmlpp::Element * node, int & ier);

/// Constant fluidity
std::shared_ptr<FluidityModel> process_constant_fluidity(const xmlpp::Element * node, int & ier);

/// Associative viscoplasticity processing
std::shared_ptr<ViscoPlasticFlowRule> process_rd_associative(const xmlpp::Element * node, int & ier);

/// G model dispatcher
std::shared_ptr<GFlow> process_gmodel(const xmlpp::Element * node, int & ier);

/// Power law G model
std::shared_ptr<GFlow> process_gmodel_power_law(const xmlpp::Element * node, int & ier);

// Begin helpers

/// Return a single child node with given name, if not return relevant error
bool one_child(const xmlpp::Node * node, std::string name,
               xmlpp::Element * & child, int & ier);

/// Return the string value of an attribute with a given name, if not error
bool one_attribute(const xmlpp::Element * node, std::string name,
                   std::string & value, int & ier);

/// Return the scalar value of a named child node
double scalar_param(const xmlpp::Node * node, std::string name,
                    int & ier);

/// Return the vector value of a named child node
std::vector<double> vector_param(const xmlpp::Node * node, std::string name,
                            int & ier);


/* Templates */

/// Dispatch to a function based on a node attribute
template <typename T>
std::shared_ptr<T> dispatch_attribute(const xmlpp::Node * node,
                                      std::string aname,
                                      std::vector<std::string> names,
                                      std::vector<std::shared_ptr<T> (*)(const xmlpp::Element*, int &)> fns,
                                      int & ier)
{
  const xmlpp::Element * enode = dynamic_cast<const xmlpp::Element*>(node);
  std::string ft;
  if (not one_attribute(enode, aname, ft, ier)) {
    return std::shared_ptr<T>(nullptr);
  }

  auto it = std::find(names.begin(), names.end(), ft);
  if (it == names.end()) {
    ier = UNKNOWN_TYPE;
    std::cerr << "Invalid type for attribute " << aname << " of node "
        << node->get_name() << " near line " << node->get_line() << std::endl;
    std::cerr << "Actual type was " << ft << std::endl;
    std::cerr << "Valid types are: ";
    for (auto it = names.begin(); it != names.end(); ++it) {
      std::cerr << *it << " ";
    }
    std::cerr << std::endl;
    return std::shared_ptr<T>(nullptr);
  }

  ptrdiff_t pos = it - names.begin();

  return fns[pos](enode, ier);
}

/// Dispatch a function over a child node with given name
template <typename T>
std::shared_ptr<T> dispatch_node(const xmlpp::Node * node,
                                 std::string nname,
                                 std::shared_ptr<T> (*fptr)(const xmlpp::Element *, int &),
                                 int & ier)
{
  xmlpp::Element * subnode;
  if (not one_child(node, nname, subnode, ier)) {
    return std::shared_ptr<T>(nullptr);
  }
  return fptr(subnode, ier);
}

/// Find node with name of given type with attribute of a type matching string
template <typename T>
std::shared_ptr<T> find_and_dispatch(const xmlpp::Node * node,
                                     std::string node_name,
                                     std::string attrib_name,
                                     std::string attrib_value,
                                     std::shared_ptr<T> (*fptr)(const xmlpp::Element *, int &),
                                     int & ier)
{
  std::stringstream ss;
  ss << node_name << "[@" << attrib_name << "='" << attrib_value << "']";
  auto fset = node->find(ss.str());

  // Determine if we found anything
  if (fset.size() < 1) {
    ier = NODE_NOT_FOUND;
    std::cerr << "Node of type " << node_name << " with attribute " << attrib_name
        << " matching '" << attrib_value << "' not found near line " 
        << node->get_line() << std::endl;
    return std::shared_ptr<NEMLModel>(nullptr);
  }
  else if (fset.size() > 1) {
    ier = TOO_MANY_NODES;
    std::cerr << "Multiple nodes of type " << node_name << " with attribute " << attrib_name
        << " matching '" << attrib_value << "' found near line " 
        << node->get_line() << std::endl;
    return std::shared_ptr<NEMLModel>(nullptr);
  }
  else {
    return make_from_node(dynamic_cast<const xmlpp::Element*>(fset[0]), ier);
  }
}

/// Collect a series of models into a single vector
template <typename T>
std::vector<std::shared_ptr<T>> dispatch_vector_models(
    const xmlpp::Node * node, std::string collection_name,
    std::string entry_name, std::shared_ptr<T> (*fptr)(const xmlpp::Element *, int &),
    int & ier)
{
  // Get the collection node
  xmlpp::Element * mnode;
  if (not one_child(node, collection_name, mnode, ier)) {
    return {};
  }

  // For each subnode
  auto nodelist = mnode->get_children(entry_name);

  if (nodelist.size() < 1) {
    ier = NODE_NOT_FOUND;
    std::cerr << "No subnodes of type " << entry_name
        << " found near line " 
        << node->get_line() << std::endl;
    return {};
  }
  
  // Create the vector
  std::vector<std::shared_ptr<T>> res;
  
  for (auto it = nodelist.begin(); it != nodelist.end(); ++it) {
    res.push_back(fptr(dynamic_cast<const xmlpp::Element *>(*it), ier));
  }

  return res;
}

} // namespace neml

#endif // PARSE_H