#include "udp_dll_wrapper.h"

#include "udp_registry.h"
#include <pagmo/problem.hpp>
void udp_dll_wrapper::_initialize_udp()
{
    // construct_udp throws std::runtime_error if not found
    _problemPtr = udp_registry::get().construct_udp(_libFileName);
}

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

udp_dll_wrapper::udp_dll_wrapper(const std::string& lib_file_name) : _libFileName(lib_file_name)
{
    _initialize_udp();
}

std::string udp_dll_wrapper::get_lib_file_name() const
{
    return _libFileName;
}

BOOST_CLASS_EXPORT(udp_dll_wrapper)

// Export the concrete instantiation
BOOST_CLASS_EXPORT(pagmo::detail::prob_inner<udp_dll_wrapper>)