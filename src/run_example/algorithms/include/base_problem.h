#pragma once
#include <string>

#include "pagmo/types.hpp"
#include <pagmo/s11n.hpp>

class base_problem
{
public:
    // Objective function
    virtual pagmo::vector_double fitness(const pagmo::vector_double& dv) const = 0;

    // Number of equality constraints
    virtual pagmo::vector_double::size_type get_nec() const = 0;

    // Number of inequality constraints
    virtual pagmo::vector_double::size_type get_nic() const = 0;

    // Box bounds
    virtual std::pair<pagmo::vector_double, pagmo::vector_double> get_bounds() const = 0;

    // Name of the library, used to identify it in the filesystem
    virtual std::string get_lib_file_name() = 0;

    virtual ~base_problem() = default;
};