#include "benchmark_results_html.h"

#include <format>

constexpr std::string_view HTML_HEAD = R"(
       <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <meta http-equiv="X-UA-Compatible" content="ie=edge">
        <title>Distributed benchmark results</title>
      </head>
)";

std::string benchmark_results_html::_build_base()
{
    std::string sectionsHtml;
    for (const auto& [tableHtml, sectionName] : _sections)
    {
        sectionsHtml += "<section>";
        sectionsHtml += "<h1>" + sectionName + "</h1>\n";
        sectionsHtml += tableHtml + "\n";
        sectionsHtml += "</section>";
    }

    return std::format(R"(
    <!DOCTYPE html>
    <html lang="en">
      {}
      <body>
      {}
      </body>
    </html>
    )", HTML_HEAD, sectionsHtml);
}

void benchmark_results_html::add_section(const std::string& htmlTable, const std::string& sectionName)
{
    _sections.emplace_back(htmlTable, sectionName);
}

std::string benchmark_results_html::get_html_page()
{
    return _build_base();
}
