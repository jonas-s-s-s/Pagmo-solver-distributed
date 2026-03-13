
#include "example_problem_single_objective.h"

pagmo::vector_double example_problem_single_objective::fitness(const pagmo::vector_double& dv) const
{
    return {
        dv[0] * dv[3] * (dv[0] + dv[1] + dv[2]) + dv[2], // objfun
        dv[0] * dv[0] + dv[1] * dv[1] + dv[2] * dv[2] + dv[3] * dv[3] - 40., // equality con.
        25. - dv[0] * dv[1] * dv[2] * dv[3] // inequality con.
    };
}

pagmo::vector_double::size_type example_problem_single_objective::get_nec() const
{
    return 1;
}

pagmo::vector_double::size_type example_problem_single_objective::get_nic() const
{
    return 1;
}

std::pair<pagmo::vector_double, pagmo::vector_double> example_problem_single_objective::get_bounds() const
{
    return {{1., 1., 1., 1.}, {5., 5., 5., 5.}};
}

std::string example_problem_single_objective::get_lib_file_name()
{
    return "problems";
}

// Function called by lib_loader after this dynamic library is loaded
void run_after_load()
{
    boost::serialization::void_cast_register<example_problem_single_objective, base_problem>();
}

example_problem_single_objective* allocator()
{
    return new example_problem_single_objective();
}

void deleter(example_problem_single_objective* ptr)
{
    delete ptr;
}

BOOST_CLASS_EXPORT_IMPLEMENT(example_problem_single_objective)