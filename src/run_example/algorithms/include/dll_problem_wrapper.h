#pragma once
#include <memory>

#include "../../problems/include/base_problem.h"
#include "pagmo/types.hpp"
#include <pagmo/s11n.hpp>
#include <pagmo/detail/s11n_wrappers.hpp>

class dll_problem_wrapper
{
    std::shared_ptr<base_problem> _problemPtr;
    std::string _libFileName;

public:
    pagmo::vector_double fitness(const pagmo::vector_double& dv) const;

    std::pair<pagmo::vector_double, pagmo::vector_double> get_bounds() const;

    pagmo::vector_double::size_type get_nec() const;

    pagmo::vector_double::size_type get_nic() const;

    /**
     * This constructor needs to exist or this class won't be recognized as UDP by Pagmo
     */
    dll_problem_wrapper() = default;

    dll_problem_wrapper(const std::shared_ptr<base_problem>& problem_ptr, const std::string& lib_file_name);

    [[nodiscard]] virtual std::string get_lib_file_name() const;

private:
    //####################################
    //# BOOST SERIALIZE
    //####################################

    friend class boost::serialization::access;

    template <typename Archive>
    void save(Archive& ar, unsigned) const
    {
        pagmo::detail::to_archive(ar, _problemPtr, _libFileName);
    }

    template <typename Archive>
    void load(Archive& ar, unsigned)
    {
        try
        {
            pagmo::detail::from_archive(ar, _problemPtr, _libFileName);
        }
        catch (...)
        {
            *this = dll_problem_wrapper{};
            throw;
        }
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
};
