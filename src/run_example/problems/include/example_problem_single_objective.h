#pragma once
#include "base_problem.h"
#include "pagmo/types.hpp"
#include "dll_visibility.h"
#include <pagmo/s11n.hpp>
#include <pagmo/detail/s11n_wrappers.hpp>
#include <boost/serialization/extended_type_info_no_rtti.hpp>

class DLL_PUBLIC example_problem_single_objective : public base_problem
{
public:
    // Implementation of the objective function.
    pagmo::vector_double fitness(const pagmo::vector_double& dv) const override;

    // Number of equality constraints.
    pagmo::vector_double::size_type get_nec() const override;

    // Number of inequality constraints.
    pagmo::vector_double::size_type get_nic() const override;

    // Implementation of the box bounds.
    std::pair<pagmo::vector_double, pagmo::vector_double> get_bounds() const override;

    std::string get_lib_file_name() override;

    ~example_problem_single_objective() override
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
        pagmo::detail::to_archive(ar);
    }

    template <typename Archive>
    void load(Archive& ar, unsigned)
    {
        try
        {
            pagmo::detail::from_archive(ar);
        }
        catch (...)
        {
            *this = example_problem_single_objective{};
            throw;
        }
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()

};


extern "C" DLL_PUBLIC void run_after_load();

extern "C" DLL_PUBLIC example_problem_single_objective* allocator();

extern "C" DLL_PUBLIC void deleter(example_problem_single_objective* ptr);


BOOST_CLASS_EXPORT_KEY(example_problem_single_objective)
BOOST_CLASS_TYPE_INFO(
    example_problem_single_objective,
    boost::serialization::extended_type_info_no_rtti<example_problem_single_objective>
)