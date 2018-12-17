#pragma once

#include <eigen/Eigen/Dense>
#include "integrals.h"
#include "rdm.h"

using namespace Eigen;

class Optimization {
 public:
  Optimization(RDM* rdm_ptr, Integrals* integrals_ptr) {
    rdm_p = rdm_ptr;
    integrals_p = integrals_ptr;
    n_orbs = integrals_ptr->n_orbs;
  }

  ~Optimization() {
    rdm_p = nullptr;
    integrals_p = nullptr;
  }

  void generate_natorb_integrals();

  void generate_optorb_integrals_from_newton();

  void generate_optorb_integrals_from_approximate_newton(const double& descent_param);

  void generate_optorb_integrals_from_grad_descent();

  std::vector<std::vector<double>> generate_optorb_integrals_from_adadelta(
      std::vector<std::vector<double>>& history);

  void dump_integrals(const char* file_name) const;

  void rewrite_integrals();

 private:
  RDM* rdm_p;

  Integrals* integrals_p;

  unsigned n_orbs;

  typedef std::vector<std::vector<std::vector<std::vector<double>>>> Integrals_array;

  typedef std::pair<unsigned, unsigned> index_t;

  Integrals_array new_integrals;

  void rotate_integrals(const MatrixXd& rot);

  void dump_integrals(const Integrals_array& new_integrals, const char* file_name) const;

  std::vector<index_t> parameter_indices() const;

  void fill_rot_matrix_with_parameters(
      MatrixXd& rot, const VectorXd& parameters, const std::vector<index_t>& parameter_indices);

  VectorXd gradient(const std::vector<std::pair<unsigned, unsigned>>&) const;

  double generalized_Fock(unsigned m, unsigned n) const;

  MatrixXd hessian(const std::vector<std::pair<unsigned, unsigned>>&) const;

  VectorXd hessian_diagonal(const std::vector<std::pair<unsigned, unsigned>>&) const;

  double Y_matrix(unsigned p, unsigned q, unsigned r, unsigned s) const;

  double hessian_part(unsigned p, unsigned q, unsigned r, unsigned s) const;
};