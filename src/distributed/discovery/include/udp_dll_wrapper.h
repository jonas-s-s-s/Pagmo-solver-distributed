#pragma once
#include <iostream>
#include <memory>

#include "udp_base.h"
#include "pagmo/types.hpp"
#include <pagmo/s11n.hpp>
#include <pagmo/detail/s11n_wrappers.hpp>

class udp_dll_wrapper
{
    // This shared_ptr is COPIED when the object is copy constructed.
    // We assume that udp_base is thread-safe, because it has no internal state,
    // so it doesn't matter that multiple threads share the same instance.
    // As long as we don't have UDPs with internal state this won't be an issue.
    std::shared_ptr<udp_base> _udpPtr{};

    std::string _libFileName;

    /**
     * Attempts to initialize the UDP via udp_registry, if UDP is not found, an exception is thrown
     */
    void _initialize_udp();

public:
    pagmo::vector_double fitness(const pagmo::vector_double& dv) const;

    std::pair<pagmo::vector_double, pagmo::vector_double> get_bounds() const;

    pagmo::vector_double::size_type get_nec() const;

    pagmo::vector_double::size_type get_nic() const;

    /**
     * This constructor needs to exist or this class won't be recognized as UDP by Pagmo
     */
    udp_dll_wrapper() = default;

    explicit udp_dll_wrapper(const std::string& lib_file_name);

    [[nodiscard]] virtual std::string get_lib_file_name() const;

private:
    //####################################
    //# BOOST SERIALIZE
    //####################################

    friend class boost::serialization::access;

    template <typename Archive>
    void save(Archive& ar, unsigned) const
    {
        pagmo::detail::to_archive(ar, _libFileName);
        std::cout << "udp_dll_wrapper successfully saved" << std::endl;
    }

    template <typename Archive>
    void load(Archive& ar, unsigned)
    {
        try
        {
            pagmo::detail::from_archive(ar, _libFileName);
            // When deserializing we load the UDP via udp_registry
            _initialize_udp();
            std::cout << "udp_dll_wrapper successfully loaded" << std::endl;
        }
        catch (...)
        {
            *this = udp_dll_wrapper{};
            throw;
        }
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
};
