#include "udp_dll_wrapper.h"

#include <pagmo/problem.hpp>

pagmo::vector_double udp_dll_wrapper::fitness(const pagmo::vector_double& dv) const
{
    return _udpPtr->fitness(dv);
}

std::pair<pagmo::vector_double, pagmo::vector_double> udp_dll_wrapper::get_bounds() const
{
    return _udpPtr->get_bounds();
}

pagmo::vector_double::size_type udp_dll_wrapper::get_nec() const
{
    return _udpPtr->get_nec();
}

pagmo::vector_double::size_type udp_dll_wrapper::get_nic() const
{
    return _udpPtr->get_nic();
}

udp_dll_wrapper::udp_dll_wrapper(const std::string& lib_file_name) : _libFileName(lib_file_name)
{
    // This is called when the UDP is constructed manually.
    // Make sure to register an "udp provider" in udp_registry even on the controller side.

    // construct_udp throws std::runtime_error if not found
    _udpPtr = udp_registry::get().construct_udp(_libFileName);
}

std::string udp_dll_wrapper::get_lib_file_name() const
{
    return _libFileName;
}

udp_dll_wrapper::udp_dll_wrapper(const udp_dll_wrapper& other) : _libFileName(other._libFileName)
{
    // Possibly allow an empty object to exist (we expose the default constructor)
    if (!_libFileName.empty())
        _udpPtr = udp_registry::get().clone_udp(other._udpPtr);
}

udp_dll_wrapper& udp_dll_wrapper::operator=(const udp_dll_wrapper& other)
{
    if (this == &other)
        return *this;

    _libFileName = other._libFileName;

    if (!_libFileName.empty())
        _udpPtr = udp_registry::get().clone_udp(other._udpPtr);

    return *this;
}

BOOST_CLASS_EXPORT(udp_dll_wrapper)

// Without this it's impossible to serialize this class when used inside pagmo objects
BOOST_CLASS_EXPORT(pagmo::detail::prob_inner<udp_dll_wrapper>)
