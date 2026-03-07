#pragma once
#include "pagmo/types.hpp"

class example_problem_single_objective
{
    // Implementation of the objective function.
    pagmo::vector_double fitness(const pagmo::vector_double &dv) const;

    // Number of equality constraints.
    pagmo::vector_double::size_type get_nec() const;

    // Number of inequality constraints.
    pagmo::vector_double::size_type get_nic() const;

    // Implementation of the box bounds.
    std::pair<pagmo::vector_double, pagmo::vector_double> get_bounds() const;
};
