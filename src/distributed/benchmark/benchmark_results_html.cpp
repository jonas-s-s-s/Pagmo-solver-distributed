#include "benchmark_results_html.h"

#include <format>

constexpr std::string_view HTML_HEAD = R"(
       <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <meta http-equiv="X-UA-Compatible" content="ie=edge">
        <style>
        table {
            width: 100%;
            border-collapse: collapse;
            margin: 1em 0;
            font-family: Arial, sans-serif;
            font-size: 14px;
            border: 1px solid #ddd;
        }
        th {
            background-color: #f4f4f4;
            text-align: left;
            padding: 10px;
            border-bottom: 2px solid #ccc;
        }
        td {
            padding: 10px;
            border-bottom: 1px solid #ddd;
        }
        tr:nth-child(even) {
            background-color: #fafafa;
        }
        tr:hover {
            background-color: #f1f7ff;
        }
        th {
            position: sticky;
            top: 0;
            z-index: 1;
        }
        table {
            border-radius: 6px;
            overflow: hidden;
        }
        </style>
        <title>Distributed benchmark results</title>
      </head>
)";


constexpr std::string_view TABLE_SORTING_SCRIPT = R"(
    <script>
    (function () {
        let sortState = new WeakMap();

        function getCellValue(row, index) {
            return row.cells[index].innerText.trim();
        }

        function isNumeric(value) {
            return !isNaN(parseFloat(value)) && isFinite(value);
        }

        function sortTable(table, columnIndex) {
            const tbody = table.tBodies[0];
            if (!tbody) return;

            const rows = Array.from(tbody.rows);

            let state = sortState.get(table) || {};
            state[columnIndex] = !state[columnIndex];
            sortState.set(table, state);

            const asc = state[columnIndex];

            rows.sort((a, b) => {
                let x = getCellValue(a, columnIndex);
                let y = getCellValue(b, columnIndex);

                if (isNumeric(x) && isNumeric(y)) {
                    return asc ? x - y : y - x;
                }

                return asc
                    ? x.localeCompare(y)
                    : y.localeCompare(x);
            });

            rows.forEach(row => tbody.appendChild(row));
        }

        function init() {
            document.querySelectorAll("table").forEach(table => {
                const headers = table.querySelectorAll("th");

                headers.forEach((th, index) => {
                    th.style.cursor = "pointer";

                    th.addEventListener("click", () => {
                        sortTable(table, index);
                    });
                });
            });
        }

        if (document.readyState === "loading") {
            document.addEventListener("DOMContentLoaded", init);
        } else {
            init();
        }
    })();
    </script>
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
      {}
      </body>
    </html>
    )", HTML_HEAD, sectionsHtml, TABLE_SORTING_SCRIPT);
}

void benchmark_results_html::add_section(const std::string& htmlTable, const std::string& sectionName)
{
    _sections.emplace_back(htmlTable, sectionName);
}

std::string benchmark_results_html::get_html_page()
{
    return _build_base();
}
