#pragma once
#include <string>
#include <vector>

/**
 * Creates an HTML document with tables showing benchmark statistics
 */
class benchmark_results_html
{
    std::vector<std::tuple<std::string, std::string>> _sections{};

    std::string _build_base();

public:
    void add_section(const std::string& htmlTable, const std::string& sectionName);

    std::string get_html_page();
};
