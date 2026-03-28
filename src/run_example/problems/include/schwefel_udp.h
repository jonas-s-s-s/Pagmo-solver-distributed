#pragma once
#include "udp_base.h"
#include "pagmo/types.hpp"
#include "dll_visibility.h"
#include <pagmo/s11n.hpp>

#include <pagmo/problems/schwefel.hpp>

class DLL_PUBLIC schwefel_udp : public udp_base
{
private:
    pagmo::schwefel _schwefel;

public:
    pagmo::vector_double fitness(const pagmo::vector_double& dv) const override;

    std::pair<pagmo::vector_double, pagmo::vector_double> get_bounds() const override;

    std::string get_lib_file_name() override;

    schwefel_udp() : _schwefel(1)
    {
    }

    explicit schwefel_udp(const std::any& dim)
    {
        unsigned dimCasted = 1;
        if (dim.has_value())
        {
            try
            {
                dimCasted = std::any_cast<unsigned>(dim);
            }
            catch (...)
            {
                throw std::runtime_error("Schwefel UDP only accepts dim (unsigned) as its parameter.");
            }
        }

        _schwefel = pagmo::schwefel{dimCasted};
    }

    ~schwefel_udp() override
    {
    };

private:
    //####################################
    //# BOOST SERIALIZE
    //####################################

    friend class boost::serialization::access;

    template <typename Archive>
    void save(Archive& ar, unsigned) const
    {
        boost::serialization::void_cast_register<schwefel_udp, udp_base>();
        // Cannot serialize schwefel directly due to linker error?
        unsigned dim = _schwefel.m_dim;
        ar << dim;
    }

    template <typename Archive>
    void load(Archive& ar, unsigned)
    {
        boost::serialization::void_cast_register<schwefel_udp, udp_base>();

        try
        {
            unsigned dim;
            ar >> dim;
            _schwefel = pagmo::schwefel{dim};
        }
        catch (...)
        {
            *this = schwefel_udp{};
            throw;
        }
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
};

extern "C" DLL_PUBLIC void run_after_load();

extern "C" DLL_PUBLIC schwefel_udp* allocator(const std::any& params);

extern "C" DLL_PUBLIC void deleter(schwefel_udp* ptr);

extern "C" DLL_PUBLIC schwefel_udp* cloner(const schwefel_udp* other);

BOOST_CLASS_EXPORT_KEY(schwefel_udp)
