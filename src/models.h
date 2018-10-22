#ifndef MODELS_H
#define MODELS_H

#include "objects.h"
#include "elasticity.h"
#include "ri_flow.h"
#include "visco_flow.h"
#include "general_flow.h"
#include "solvers.h"
#include "interpolate.h"
#include "creep.h"

#include <cstddef>
#include <memory>
#include <vector>
#include <cmath>
#include <iostream>

namespace neml {

/// NEML material model interface definitions
//  All material models inherit from this base class.  It defines interfaces
//  and provides the methods for reading in material parameters.
class NEMLModel: public NEMLObject {
  public:
   /// Total number of stored internal variables
   virtual size_t nstore() const = 0;
   /// Initialize the internal variables
   virtual int init_store(double * const store) const = 0;

   /// Small strain update interface
   virtual int update_sd(
       const double * const e_np1, const double * const e_n,
       double T_np1, double T_n,
       double t_np1, double t_n,
       double * const s_np1, const double * const s_n,
       double * const h_np1, const double * const h_n,
       double * const A_np1,
       double & u_np1, double u_n,
       double & p_np1, double p_n) = 0;
  
   /// Number of internal variables that are true material history
   virtual size_t nhist() const = 0;
   /// Initialize the history variables
   virtual int init_hist(double * const hist) const = 0;

   /// Instantaneous thermal expansion coefficient as a function of temperature
   virtual double alpha(double T) const = 0;
   /// Elastic strain for a given stress, temperature, and history state
   virtual int elastic_strains(const double * const s_np1,
                               double T_np1, const double * const h_np1,
                               double * const e_np1) const = 0;
   /// Model effective bulk modulus
   virtual double bulk(double T) const = 0;
   /// Model effective shear modulus
   virtual double shear(double T) const = 0;
};

/// Small deformation stress update
class NEMLModel_sd: public NEMLModel {
  public:
    /// All small strain models use small strain elasticity and CTE
    NEMLModel_sd(std::shared_ptr<LinearElasticModel> emodel,
                 std::shared_ptr<Interpolate> alpha);
    virtual ~NEMLModel_sd();

   /// The small strain stress update interface
   virtual int update_sd(
       const double * const e_np1, const double * const e_n,
       double T_np1, double T_n,
       double t_np1, double t_n,
       double * const s_np1, const double * const s_n,
       double * const h_np1, const double * const h_n,
       double * const A_np1,
       double & u_np1, double u_n,
       double & p_np1, double p_n) = 0;

   /// Number of stored variables
   virtual size_t nstore() const;
   /// Initialize stored variables
   virtual int init_store(double * const store) const;

   /// Number of stored variables that are true material history
   virtual size_t nhist() const = 0;
   /// Initialize the stored history
   virtual int init_hist(double * const hist) const = 0;

   /// Provide the instantaneous CTE
   virtual double alpha(double T) const;
   /// Returns the elasticity model, for sub-objects that want to use it
   const std::shared_ptr<const LinearElasticModel> elastic() const;

   /// Return the elastic strains
   virtual int elastic_strains(const double * const s_np1,
                               double T_np1, const double * const h_np1,
                               double * const e_np1) const;
   /// Return the model elastic bulk modulus
   virtual double bulk(double T) const;
   /// Return the model elastic shear modulus
   virtual double shear(double T) const;

   /// Used to override the linear elastic model to match another object's 
   virtual int set_elastic_model(std::shared_ptr<LinearElasticModel> emodel);

  protected:
   std::shared_ptr<LinearElasticModel> elastic_;

  private:
   std::shared_ptr<Interpolate> alpha_;

};

/// Small strain linear elasticity
//  This is generally only used as a basic test
class SmallStrainElasticity: public NEMLModel_sd {
 public:
  /// Parameters are the minimum: an elastic model and a thermal expansion 
  SmallStrainElasticity(std::shared_ptr<LinearElasticModel> elastic,
                        std::shared_ptr<Interpolate> alpha);
  
  /// Type for the object system
  static std::string type();
  /// Setup parameters for the object system
  static ParameterSet parameters();
  /// Initialize from a parameter set
  static std::unique_ptr<NEMLObject> initialize(ParameterSet & params);
  
  /// Small strain stress update
  virtual int update_sd(
      const double * const e_np1, const double * const e_n,
      double T_np1, double T_n,
      double t_np1, double t_n,
      double * const s_np1, const double * const s_n,
      double * const h_np1, const double * const h_n,
      double * const A_np1,
      double & u_np1, double u_n,
      double & p_np1, double p_n);
  /// Number of history variables (=0)
  virtual size_t nhist() const;
  /// Initialize history (none to setup)
  virtual int init_hist(double * const hist) const;
};

static Register<SmallStrainElasticity> regSmallStrainElasticity;

/// Small strain perfect plasticity trial state
//  Store data the solver needs and can be passed into solution interface
class SSPPTrialState : public TrialState {
 public:
  double ys, T;
  double ee_n[6];
  double s_n[6];
  double s_tr[6];
  double e_np1[6];
  double e_n[6];
  double S[36];
  double C[36];
};

/// Small strain rate independent plasticity trial state
class SSRIPTrialState : public TrialState {
 public:
  double ep_tr[6];
  double s_tr[6];
  double e_np1[6];
  double C[36];
  double T;
  std::vector<double> h_tr;
};

/// Small strain creep+plasticity trial state 
class SSCPTrialState : public TrialState {
 public:
  double ep_strain[6];

  double e_n[6], e_np1[6];
  double s_n[6];
  double T_n, T_np1, t_n, t_np1;
  std::vector<double> h_n;
};

/// General inelastic integrator trial state
class GITrialState : public TrialState {
 public:
  double e_dot[6];
  double s_n[6];
  double T, Tdot, dt;
  std::vector<double> h_n;

};

/// Small strain, associative, perfect plasticity
//    Algorithm is generalized closest point projection.
//    This degenerates to radial return for models where the gradient of
//    the yield surface is constant along lines from the origin to a point
//    in stress space outside the surface (i.e. J2).

class SmallStrainPerfectPlasticity: public NEMLModel_sd, public Solvable {
 public:
  /// Parameters: elastic model, yield surface, yield stress, CTE,
  /// integration tolerance, maximum number of iterations,
  /// verbosity flag, and the maximum number of adaptive subdivisions
  SmallStrainPerfectPlasticity(std::shared_ptr<LinearElasticModel> elastic,
                               std::shared_ptr<YieldSurface> surface,
                               std::shared_ptr<Interpolate> ys,
                               std::shared_ptr<Interpolate> alpha,
                               double tol, int miter,
                               bool verbose,
                               int max_divide);
  
  /// Type for the object system
  static std::string type();
  /// Parameters for the object system
  static ParameterSet parameters();
  /// Setup from a ParameterSet
  static std::unique_ptr<NEMLObject> initialize(ParameterSet & params);
  
  /// The small strain stress update
  virtual int update_sd(
      const double * const e_np1, const double * const e_n,
      double T_np1, double T_n,
      double t_np1, double t_n,
      double * const s_np1, const double * const s_n,
      double * const h_np1, const double * const h_n,
      double * const A_np1,
      double & u_np1, double u_n,
      double & p_np1, double p_n);
  /// Number of history variables (=0)
  virtual size_t nhist() const;
  /// Initialize history (nothing to do)
  virtual int init_hist(double * const hist) const;
  
  /// Number of nonlinear equations to solve in the integration
  virtual size_t nparams() const;
  /// Setup an initial guess for the nonlinear solution
  virtual int init_x(double * const x, TrialState * ts);
  /// Integration residual and jacobian equations
  virtual int RJ(const double * const x, TrialState * ts, double * const R,
                 double * const J);

  /// Helper to return the yield stress
  double ys(double T) const;

  /// Setup a trial state for the solver from the input information
  int make_trial_state(const double * const e_np1, const double * const e_n,
                       double T_np1, double T_n, double t_np1, double t_n,
                       const double * const s_n, const double * const h_n,
                       SSPPTrialState & ts);

 private:
  int update_substep_(
      const double * const e_np1, const double * const e_n,
      double T_np1, double T_n,
      double t_np1, double t_n,
      double * const s_np1, const double * const s_n,
      double * const h_np1, const double * const h_n,
      double * const A_np1,
      double & u_np1, double u_n,
      double & p_np1, double p_n);
  int calc_tangent_(SSPPTrialState ts, const double * const s_np1, double dg, 
                double * const A_np1);

  std::shared_ptr<YieldSurface> surface_;
  std::shared_ptr<Interpolate> ys_;
  const double tol_;
  const int miter_;
  const bool verbose_;
  const int max_divide_;
};

static Register<SmallStrainPerfectPlasticity> regSmallStrainPerfectPlasticity;

/// Small strain, rate-independent plasticity
//    The algorithm used here is generalized closest point projection
//    for associative flow models.  For non-associative models the algorithm
//    may theoretically fail the discrete Kuhn-Tucker conditions, even
//    putting aside convergence issues on the nonlinear solver.
//
//    The class does check for Kuhn-Tucker violations when it returns, 
//    reporting an error if the conditions are violated.
class SmallStrainRateIndependentPlasticity: public NEMLModel_sd, public Solvable {
 public:
  SmallStrainRateIndependentPlasticity(std::shared_ptr<LinearElasticModel> elastic,
                                       std::shared_ptr<RateIndependentFlowRule> flow,
                                       std::shared_ptr<Interpolate> alpha,
                                       double tol, int miter, bool verbose,double kttol,
                                       bool check_kt);

  static std::string type();
  static ParameterSet parameters();
  static std::unique_ptr<NEMLObject> initialize(ParameterSet & params);

  virtual int update_sd(
      const double * const e_np1, const double * const e_n,
      double T_np1, double T_n,
      double t_np1, double t_n,
      double * const s_np1, const double * const s_n,
      double * const h_np1, const double * const h_n,
      double * const A_np1,
      double & u_np1, double u_n,
      double & p_np1, double p_n);

  virtual size_t nhist() const;
  virtual int init_hist(double * const hist) const;

  virtual size_t nparams() const;
  virtual int init_x(double * const x, TrialState * ts);
  virtual int RJ(const double * const x, TrialState * ts, double * const R,
                 double * const J);
  
  // Getters
  const std::shared_ptr<const LinearElasticModel> elastic() const;

  // Make this public for ease of testing
  int make_trial_state(const double * const e_np1, const double * const e_n,
                       double T_np1, double T_n, double t_np1, double t_n,
                       const double * const s_n, const double * const h_n,
                       SSRIPTrialState & ts);

 private:
  int calc_tangent_(const double * const x, TrialState * ts, const double * const s_np1,
                    const double * const h_np1, double dg, double * const A_np1);
  int check_K_T_(const double * const s_np1, const double * const h_np1, double T_np1, double dg);

  std::shared_ptr<RateIndependentFlowRule> flow_;

  double tol_, kttol_;
  int miter_;
  bool verbose_, check_kt_;
};

static Register<SmallStrainRateIndependentPlasticity> regSmallStrainRateIndependentPlasticity;

/// Small strain, rate-independent plasticity + creep
//  Uses a combined iteration of a rate independent plastic + creep model
//  to solver overall update
class SmallStrainCreepPlasticity: public NEMLModel_sd, public Solvable {
 public:
  SmallStrainCreepPlasticity(
                             std::shared_ptr<LinearElasticModel> elastic,
                             std::shared_ptr<NEMLModel_sd> plastic,
                             std::shared_ptr<CreepModel> creep,
                             std::shared_ptr<Interpolate> alpha,
                             double tol, int miter,
                             bool verbose, double sf);

  static std::string type();
  static ParameterSet parameters();
  static std::unique_ptr<NEMLObject> initialize(ParameterSet & params);

  virtual int update_sd(
      const double * const e_np1, const double * const e_n,
      double T_np1, double T_n,
      double t_np1, double t_n,
      double * const s_np1, const double * const s_n,
      double * const h_np1, const double * const h_n,
      double * const A_np1,
      double & u_np1, double u_n,
      double & p_np1, double p_n);

  virtual size_t nhist() const;
  virtual int init_hist(double * const hist) const;

  virtual size_t nparams() const;
  virtual int init_x(double * const x, TrialState * ts);
  virtual int RJ(const double * const x, TrialState * ts, double * const R,
                 double * const J);
  
  // Make this public for ease of testing
  int make_trial_state(const double * const e_np1, const double * const e_n,
                       double T_np1, double T_n, double t_np1, double t_n,
                       const double * const s_n, const double * const h_n,
                       SSCPTrialState & ts);

   virtual int set_elastic_model(std::shared_ptr<LinearElasticModel> emodel);

 private:
  int form_tangent_(double * const A, double * const B,
                    double * const A_np1);

 private:
  std::shared_ptr<NEMLModel_sd> plastic_;
  std::shared_ptr<CreepModel> creep_;

  double tol_, sf_;
  int miter_;
  bool verbose_;
};

static Register<SmallStrainCreepPlasticity> regSmallStrainCreepPlasticity;

/// Small strain general integrator
//    General NR one some stress rate + history evolution rate
//
class GeneralIntegrator: public NEMLModel_sd, public Solvable {
 public:
  GeneralIntegrator(std::shared_ptr<LinearElasticModel> elastic,
                    std::shared_ptr<GeneralFlowRule> rule,
                    std::shared_ptr<Interpolate> alpha,
                    double tol, int miter,
                    bool verbose, int max_divide);

  static std::string type();
  static ParameterSet parameters();
  static std::unique_ptr<NEMLObject> initialize(ParameterSet & params);

  virtual int update_sd(
      const double * const e_np1, const double * const e_n,
      double T_np1, double T_n,
      double t_np1, double t_n,
      double * const s_np1, const double * const s_n,
      double * const h_np1, const double * const h_n,
      double * const A_np1,
      double & u_np1, double u_n,
      double & p_np1, double p_n);
  virtual size_t nhist() const;
  virtual int init_hist(double * const hist) const;

  virtual size_t nparams() const;
  virtual int init_x(double * const x, TrialState * ts);
  virtual int RJ(const double * const x, TrialState * ts,
                 double * const R, double * const J);

  // Make this public for ease of testing
  int make_trial_state(const double * const e_np1, const double * const e_n,
                       double T_np1, double T_n, double t_np1, double t_n,
                       const double * const s_n, const double * const h_n,
                       GITrialState & ts);

  virtual int set_elastic_model(std::shared_ptr<LinearElasticModel> emodel);

 private:
  int calc_tangent_(const double * const x, TrialState * ts, double * const A_np1);

  std::shared_ptr<GeneralFlowRule> rule_;

  double tol_;
  int miter_, max_divide_;
  bool verbose_;
};

static Register<GeneralIntegrator> regGeneralIntegrator;

/// Combines multiple small strain integrators based on regimes of
/// rate-dependent behavior.
//
//  This model uses the idea from Kocks & Mecking of a normalized activation
//  energy to call different integrators depending on the combination of
//  temperature and strain rate.
//
//  A typical use case would be switching from rate-independent to rate
//  dependent behavior based on a critical activation energy cutoff point
//
//  A user provides a vector of models (length n) and a corresponding vector
//  of normalized activation energies (length n-1) dividing the response into
//  segments.  All the models must have compatible hardening -- the history
//  is just going to be blindly passed between the models.
//
class KMRegimeModel: public NEMLModel_sd {
 public:
  KMRegimeModel(std::shared_ptr<LinearElasticModel> emodel,
                std::vector<std::shared_ptr<NEMLModel_sd>> models,
                std::vector<double> gs, 
                double kboltz, double b, double eps0,
                std::shared_ptr<Interpolate> alpha);

  static std::string type();
  static ParameterSet parameters();
  static std::unique_ptr<NEMLObject> initialize(ParameterSet & params);

  virtual int update_sd(
      const double * const e_np1, const double * const e_n,
      double T_np1, double T_n,
      double t_np1, double t_n,
      double * const s_np1, const double * const s_n,
      double * const h_np1, const double * const h_n,
      double * const A_np1,
      double & u_np1, double u_n,
      double & p_np1, double p_n);
  virtual size_t nhist() const;
  virtual int init_hist(double * const hist) const;

  virtual int set_elastic_model(std::shared_ptr<LinearElasticModel> emodel);

 private:
  double activation_energy_(const double * const e_np1, 
                            const double * const e_n,
                            double T_np1,
                            double t_np1, double t_n);

 private:
  std::vector<std::shared_ptr<NEMLModel_sd>> models_;
  std::vector<double> gs_;
  double kboltz_, b_, eps0_;
};

static Register<KMRegimeModel> regKMRegimeModel;

} // namespace neml
#endif // MODELS_H
