#pragma once
#include <string>
#include <pagmo/s11n.hpp>

#include "UUID.h"

struct worker_settings
{
    // ID of the worker - used as an "address" in the ZeroMQ Router-Dealer model
    std::string workerId;

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
        ar & BOOST_SERIALIZATION_NVP(workerId);
    }
};