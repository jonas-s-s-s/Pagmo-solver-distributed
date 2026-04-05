#pragma once
#include <string>
#include <pagmo/s11n.hpp>

#include "UUID.h"

struct controller_settings
{
    std::string id;

    /**
     * Sets initial values of the settings fields
     */
    void initialize();

private:
    //####################################
    //# BOOST SERIALIZE
    //####################################

    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive& ar, unsigned)
    {
        ar & BOOST_SERIALIZATION_NVP(id);
    }
};