#include "schwefel_udp.h"

//#####################################################################################
//# PAGMO UDP public functions
//#####################################################################################

pagmo::vector_double schwefel_udp::fitness(const pagmo::vector_double& dv) const
{
    return _schwefel.fitness(dv);
}

std::pair<pagmo::vector_double, pagmo::vector_double> schwefel_udp::get_bounds() const
{
    return _schwefel.get_bounds();
}

//#####################################################################################
//# Other
//#####################################################################################

std::string schwefel_udp::get_lib_file_name()
{
    return "schwefel_udp";
}

void run_after_load()
{
}

schwefel_udp* allocator(const std::any& params)
{
    return new schwefel_udp(params);
}

void deleter(schwefel_udp* ptr)
{
    delete ptr;
}

schwefel_udp* cloner(const schwefel_udp* other)
{
    if (!other)
        return nullptr;
    return new schwefel_udp(*other);
}

BOOST_CLASS_EXPORT_IMPLEMENT(schwefel_udp)