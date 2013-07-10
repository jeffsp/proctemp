/// @file html.h
/// @brief html helper functions
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-07-09

// Copyright (C) 2013 Jeffrey S. Perry
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef HTML_H
#define HTML_H

#include <cassert>
#include <cmath>
#include <sstream>

namespace proctemp
{

using namespace std;

/// @brief wrap with <html></html>
///
/// @tparam S ostream type
template<typename S>
struct html_tag
{
    S &s;
    html_tag (S &s) : s (s) { s << "<html>\n"; }
    ~html_tag () { s << "</html>\n"; }
};

/// @brief wrap with <head></head>
///
/// @tparam S ostream type
template<typename S>
struct html_head_tag
{
    S &s;
    html_head_tag (S &s) : s (s) { s << "<head>\n"; }
    ~html_head_tag () { s << "</head>\n"; }
};

/// @brief dump some head markup
///
/// @tparam S ostream type
/// @param s ostream
template<typename S>
void html_head1 (S &s)
{
    s <<
        "<META HTTP-EQUIV=\"refresh\" CONTENT=\"2\">\n"
        "<script type='text/javascript' src='https://www.google.com/jsapi'></script>\n";
}

/// @brief wrap with javascript tags
///
/// @tparam S ostream type
template<typename S>
struct html_javascript_tag
{
    S &s;
    html_javascript_tag (S &s) : s (s) { s << "<script type='text/javascript'>\n"; }
    ~html_javascript_tag () { s << "</script>\n"; }
};

/// @brief dump some chart markup
///
/// @tparam S ostream type
/// @param s ostream
template<typename S>
void html_draw_charts (S &s, size_t n)
{
    s <<
        "google.load('visualization', '1', {packages:['gauge']});\n"
        "google.setOnLoadCallback(drawCharts);\n"
        "function drawCharts() {\n";
    for (size_t i = 0; i < n; ++i)
        s << "    drawChart" << i << "();\n";
    s << "}\n";
};

/// @brief draw a google chart
///
/// @tparam S stream type
/// @tparam T temperatures type
/// @param s stream
/// @param t temperatures
/// @param chart_num chart num
/// @param f fahrenheit flag
template<typename S,typename T>
void html_draw_chart (S &s, const T &t, size_t chart_num, bool f)
{
    const int MIN = 40;
    int max = 0;
    int high = 0;
    int critical = 0;
    s <<
        "function drawChart" << chart_num << "() {\n"
        "   var data" << chart_num << " = google.visualization.arrayToDataTable([\n"
        "   ['Label', 'Value'],\n";
    for (size_t n = 0; n < t.size (); ++n)
    {
        if (t[n].high > high)
            high = t[n].high;
        if (t[n].critical > critical)
            critical = t[n].critical;
        if (t[n].critical + 5 > max)
            max = t[n].critical + 5;
        const int C = t[n].current < MIN ? MIN : (t[n].current > max ? max : t[n].current);
        s << "   ['" << n << "', " << round (f ? ctof (C) : C) << "]";
        if (n + 1 < t.size ())
            s << ",\n";
        else
            s << "]);\n";
    }
    s <<
        "   var options" << chart_num << " = { min : " << round (f ? ctof (MIN) : MIN)
                << ", " << "max : " << round (f ? ctof (max) : max)
                << ", " << "yellowFrom : " << round (f ? ctof (high) : high)
                << ", " << "yellowTo : " << round (f ? ctof (critical) : critical)
                << ", " << "redFrom : " << round (f ? ctof (critical) : critical)
                << ", " << "redTo : " << round (f ? ctof (max) : max)
                << ", " << "animation : { duration : 1000, easing : 'linear' }"
                << ", " << "minorTicks : 0"
                << ", " << "height : 100"
                << "};\n"
        "   var chart" << chart_num << " = new google.visualization.Gauge(document.getElementById('chart_div" << chart_num << "'));\n"
        "   chart" << chart_num << ".draw(data" << chart_num << ", options" << chart_num << ");\n"
        "}\n";
};

/// @brief html helper
///
/// @tparam S ostream type
/// @tparam T titles type
/// @param s ostream
/// @param titles titles
template<typename S,typename T>
void html_body (S &s, const T &titles)
{
    s << "<body>\n";
    for (size_t i = 0; i < titles.size (); ++i)
        s << titles[i] << "<br>" << "<div id='chart_div" << i << "'>" << "</div><br>\n";
    s << "</body>\n";
}

/// @brief draw a google charts api chart
///
/// @tparam S ostream type
/// @tparam T temperatures type
/// @tparam U bus names type
/// @param s ostream
/// @param temps temperatures
/// @param names bus names
/// @param fahrenheit fahrenheit flag
template<typename S,typename T,typename U>
void draw_charts (S &s, const T &temps, const U &names, bool fahrenheit)
{
    assert (temps.size () == names.size ());
    size_t n = 0;
    vector<string> titles;
    // dump html output to a stream
    html_tag<S> html (s);
    {
        html_head_tag<S> head (s);
        html_head1 (s);
        html_javascript_tag<S> javascript (s);
        for (size_t i = 0; i < temps.size (); ++i)
            n += temps[i].size ();
        html_draw_charts (s, n);
        size_t chart_num = 0;
        for (size_t bus = 0; bus < temps.size (); ++bus)
        {
            for (size_t i = 0; i < temps[bus].size (); ++i)
            {
                html_draw_chart (s, temps[bus][i], chart_num++, fahrenheit);
                stringstream ss;
                ss << names[bus] << i;
                titles.push_back (ss.str ());
            }
        }
        assert (chart_num == n);
    }
    html_body (s, titles);
}

}

#endif
