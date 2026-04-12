#pragma once
#include <string>

/**
 * Converts CSV to HTML representation.
 * Useful for solver_benchmark.
 */
inline std::string csv_to_html(const std::string& input) {
    std::string html;
    html.reserve(input.size() * 2);
    html += "<table>\n<thead>\n";

    bool in_quotes = false;
    bool is_header = true;
    bool new_cell = true;
    bool new_row = true;

    for (size_t i = 0; i < input.size(); ++i) {
        char c = input[i];

        if (new_row) {
            html += "<tr>";
            new_row = false;
            new_cell = true;
        }

        if (c == '"') {
            if (in_quotes && i + 1 < input.size() && input[i + 1] == '"') {
                html += "&quot;";
                ++i;
            } else {
                in_quotes = !in_quotes;
            }
            continue;
        }

        if (c == ',' && !in_quotes) {
            if (!new_cell) html += is_header ? "</th>" : "</td>";
            new_cell = true;
            continue;
        }

        if ((c == '\n' || c == '\r') && !in_quotes) {
            if (c == '\r' && i + 1 < input.size() && input[i + 1] == '\n')
                ++i;

            if (!new_cell) html += is_header ? "</th>" : "</td>";
            html += "</tr>\n";

            if (is_header) html += "</thead>\n<tbody>\n";

            is_header = false;
            new_row = true;
            new_cell = true;
            continue;
        }

        if (new_cell) {
            html += is_header ? "<th>" : "<td>";
            new_cell = false;
        }

        switch (c) {
        case '<': html += "&lt;";   break;
        case '>': html += "&gt;";   break;
        case '&': html += "&amp;";  break;
        case '"': html += "&quot;"; break;
        default:  html += c;
        }
    }

    if (!new_row) {
        if (!new_cell) html += is_header ? "</th>" : "</td>";
        html += "</tr>\n";
        if (is_header) html += "</thead>\n<tbody>\n";
    }

    html += "</tbody>\n</table>";
    return html;
}