#include "udp_dll_wrapper.h"

pagmo::vector_double udp_dll_wrapper::fitness(const pagmo::vector_double& dv) const
{
    return _problemPtr->fitness(dv);
}

std::pair<pagmo::vector_double, pagmo::vector_double> udp_dll_wrapper::get_bounds() const
{
    return _problemPtr->get_bounds();
}

pagmo::vector_double::size_type udp_dll_wrapper::get_nec() const
{
    return _problemPtr->get_nec();
}

pagmo::vector_double::size_type udp_dll_wrapper::get_nic() const
{
    return _problemPtr->get_nic();
}

udp_dll_wrapper::udp_dll_wrapper(const std::shared_ptr<udp_base>& problem_ptr,
    const std::string& lib_file_name): _problemPtr(problem_ptr),
                                       _libFileName(lib_file_name)
{
}

std::string udp_dll_wrapper::get_lib_file_name() const
{
    return _libFileName;
}
