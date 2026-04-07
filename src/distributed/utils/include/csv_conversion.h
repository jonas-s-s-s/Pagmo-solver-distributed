#pragma once
#include <string>

/**
 * Converts CSV to HTML representation.
 * Useful for solver_benchmark.
 * https://rosettacode.org/wiki/CSV_to_HTML_translation#C
 */
inline std::string csv_to_html(const std::string& input) {
    std::string html = "<table>\n";
    bool in_quotes = false;
    html += "<tr><td>";

    for (const char c : input) {
        if (c == '"') {
            in_quotes = !in_quotes;
        }
        else if (c == ',' && !in_quotes) {
            html += "</td><td>";
        }
        else if (c == '\n' && !in_quotes) {
            html += "</td></tr>\n<tr><td>";
        }
        else {
            switch(c) {
            case '<': html += "&lt;"; break;
            case '>': html += "&gt;"; break;
            case '&': html += "&amp;"; break;
            default:  html += c;
            }
        }
    }

    html += "</td></tr>\n</table>";
    return html;
}