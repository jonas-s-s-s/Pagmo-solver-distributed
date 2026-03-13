#include "include/dll_problem_wrapper.h"

pagmo::vector_double dll_problem_wrapper::fitness(const pagmo::vector_double& dv) const
{
    return _problemPtr->fitness(dv);
}

std::pair<pagmo::vector_double, pagmo::vector_double> dll_problem_wrapper::get_bounds() const
{
    return _problemPtr->get_bounds();
}

pagmo::vector_double::size_type dll_problem_wrapper::get_nec() const
{
    return _problemPtr->get_nec();
}

pagmo::vector_double::size_type dll_problem_wrapper::get_nic() const
{
    return _problemPtr->get_nic();
}

dll_problem_wrapper::dll_problem_wrapper(const std::shared_ptr<base_problem>& problem_ptr,
    const std::string& lib_file_name): _problemPtr(problem_ptr),
                                       _libFileName(lib_file_name)
{
}

std::string dll_problem_wrapper::get_lib_file_name() const
{
    return _libFileName;
}
