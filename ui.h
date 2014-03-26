/// @file ui.h
/// @brief user interface
/// @author Jeff Perry <jeffsp@gmail.com>
/// @date 2013-07-04

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

#ifndef UI_H
#define UI_H

#include "options.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <ncurses.h>
#include <sstream>

namespace therm
{

/// @brief draw ncurses text with attributes
///
/// @param attrs vector of attrs
/// @param rows total rows
/// @param r row
/// @param c col
/// @param s string
/// @param args print args
template<typename... T>
void text (const std::vector<int> &attrs, int rows, int r, int c, const char *s, T... args)
{
    if (r + 1 == rows)
        return;
    for (auto a : attrs)
        attron (a);
    mvprintw (r, c, s, args...);
    for (auto a : attrs)
        attroff (a);
}

/// @brief ncurses user interface
class ncurses_ui
{
    private:
    /// @brief screen dimensions
    int rows, cols;
    /// @brief configuration options
    options &opts;
    /// @brief event loop support
    bool done;
    /// @brief flag for debugging
    bool debug;
    static const int WHITE = COLOR_PAIR(1);
    static const int GREEN = COLOR_PAIR(2);
    static const int YELLOW = COLOR_PAIR(3);
    static const int RED = COLOR_PAIR(4);
    static const int BLUE = COLOR_PAIR(5);
    static const int GRAY_ON_CYAN = COLOR_PAIR(6);
    static const int RED_ON_CYAN = COLOR_PAIR(7);
    public:
    /// @brief constructor
    ncurses_ui (options &opts)
        : opts (opts)
        , done (false)
        , debug (false)
    {
        init ();
        labels ();
    }
    /// @brief destructor
    ~ncurses_ui ()
    {
        release ();
    }
    /// @brief initialize ncurses stuff
    void init ()
    {
        initscr ();
        start_color ();
        use_default_colors ();
        raw ();
        keypad (stdscr, 1);
        noecho ();
        curs_set (0); // make cursor invisible
        erase ();
        getmaxyx (stdscr, rows, cols);
        init_pair (1, COLOR_WHITE, -1);
        init_pair (2, COLOR_GREEN, -1);
        init_pair (3, COLOR_YELLOW, -1);
        init_pair (4, COLOR_RED, -1);
        init_pair (5, COLOR_BLUE, -1);
        init_pair (6, COLOR_WHITE, COLOR_CYAN);
        init_pair (7, COLOR_RED, COLOR_CYAN);
        timeout (1000); // timeout in ms
    }
    /// @brief ncurses cleanup
    void release () const
    {
        endwin ();
    }
    /// @brief event loop support
    ///
    /// @return true if done
    bool is_done () const
    {
        return done;
    }
    /// @brief event loop support
    void process (int ch, const std::string &config_fn)
    {
        switch (ch)
        {
            default:
            break;
            case 'q':
            case 'Q':
            done = true;
            break;
            case 's':
            case 'S':
            release ();
            write (opts, config_fn);
            init ();
            labels ();
            break;
            case 't':
            case 'T':
            opts.set_fahrenheit (!opts.get_fahrenheit ());
            break;
            case '!':
            debug = !debug;
            release ();
            init ();
            labels ();
            break;
        }
        if (is_term_resized (rows, cols))
        {
            release ();
            init ();
            labels ();
        }
        refresh ();
    }
    /// @brief display temps
    ///
    /// @param busses vector of busses
    void show_temps (const busses &bs) const
    {
        // get the width of the cpu number column
        size_t max_cpus = 0;
        for (auto bus : bs)
            for (auto chip : bus.chips)
                if (chip.temps.size () > max_cpus)
                    max_cpus = chip.temps.size ();
        std::stringstream ss;
        ss << max_cpus;
        // length of largest number plus a space
        const int indent1 = ss.str ().size () + 1;
        // assumes temps are 3 digits at most, plus the C or F, plus a space
        const int indent2 = indent1 + 5;
        const int indent3 = indent2 + 7;
        // print the temperatures
        auto row = 0;
        for (auto bus : bs)
        {
            text ({}, rows, row++, 0, "%s", bus.name.c_str ());
            size_t chipno = 0;
            for (auto chip : bus.chips)
            {
                if (bus.chips.size () > 1)
                    text ({}, rows, row++, 0, "%s %d", chip.name.c_str (), chipno++);
                else
                    text ({}, rows, row++, 0, "%s", chip.name.c_str ());
                size_t n = 0;
                for (auto t : chip.temps)
                {
                    // set default temps if none were given
                    if (t.high == -1)
                        t.high = 80;
                    if (t.critical == -1)
                        t.critical = 90;
                    if (debug && !(rand () % chip.temps.size ()))
                        t.current = (rand () % int (t.critical + 10 - t.high)) + t.high;
                    // print the cpu number
                    std::stringstream ss;
                    ss << n++;
                    text ({}, rows, row, 0, ss.str ().c_str ());
                    // print the numerical value
                    ss.str ("");
                    ss << round (opts.get_fahrenheit () ? ctof (t.current) : t.current) << (opts.get_fahrenheit () ? 'F' : 'C');
                    int color = GREEN;
                    if (t.current >= t.high)
                        color = YELLOW;
                    if (t.current >= t.critical)
                        color = RED;
                    text ({A_BOLD, color}, rows, row, indent1, "%4s", ss.str ().c_str ());
                    // print the bar
                    const int size = cols - indent2;
                    temp_bar (row++, indent2, size, t);
                }
                n = 0;
                for (auto f : chip.fan_speeds)
                {
                    std::stringstream ss;
                    ss << round (f.current);
                    if (n == 0)
                        text ({WHITE}, rows, row++, 0, "  FAN");
                    text ({A_BOLD, WHITE}, rows, row, 0, "  %d %4s RPM", n++, ss.str ().c_str ());
                    const int size = cols - indent3;
                    speed_bar (row++, indent3, size, f);
                }
                ++row;
            }
        }
    }
    private:
    /// @brief draw a temperature bar
    ///
    /// @tparam T temperature type
    /// @param i row
    /// @param j col
    /// @param size bar length
    /// @param t temperature
    template<typename T>
    void temp_bar (int i, int j, int size, T t) const
    {
        const float MIN = 40;
        const float MAX = t.critical + 5;
        float current = t.current < MIN ? MIN : (t.current > MAX ? MAX : t.current);
        float SZ = (MAX - MIN);
        float len = size * (current - MIN) / SZ;
        for (int k = 0; k < size; ++k)
        {
            int color;
            if (k <= (size * (t.high - MIN) / SZ))
                color = GREEN;
            else if (k <= (size * (t.critical - MIN) / SZ))
                color = YELLOW;
            else
                color = RED;
            if (k <= len)
                text ({A_BOLD, A_REVERSE, color}, rows, i, j + k, " ");
            else
                text ({A_BOLD, color}, rows, i, j + k, "-");
        }
        text ({A_BOLD}, rows, i, j, "[");
        text ({A_BOLD}, rows, i, j + size - 1, "]");
    }
    /// @brief draw a speed bar
    ///
    /// @tparam speed type
    /// @param i row
    /// @param j col
    /// @param size bar length
    /// @param s speed
    template<typename T>
    void speed_bar (int i, int j, int size, T t) const
    {
        text ({A_BOLD}, rows, i, j, "[");
        text ({A_BOLD}, rows, i, j + size - 1, "]");
        const int MIN = 20000;
        const int MAX = 90000;
        int current = t.current < MIN ? MIN : (t.current > MAX ? MAX : t.current);
        int len = size * (current - MIN) / (MAX - MIN);
        for (int k = 1; k + 1 < size; ++k)
        {
            int color = BLUE;
            if (k < len)
                text ({A_REVERSE, color}, rows, i, j + k, " ");
            else
                text ({A_BOLD, color}, rows, i, j + k, "-");
        }
    }
    /// @brief draw labels
    void labels () const
    {
        int col = 0;
        std::stringstream ss;
        ss.str ("");
        ss << "therm version " << therm::MAJOR_REVISION << '.' << therm::MINOR_REVISION;
        text ({A_BOLD, BLUE}, rows + 1, rows - 1, col, ss.str ().c_str ());
        col += ss.str ().size () + 1;
        ss.str ("");
        ss << "T";
        text ({}, rows + 1, rows - 1, col, ss.str ().c_str ());
        col += ss.str ().size ();
        ss.str ("");
        ss << "emp scale  ";
        text ({GRAY_ON_CYAN}, rows + 1, rows - 1, col, ss.str ().c_str ());
        col += ss.str ().size ();
        ss.str ("");
        ss << "S";
        text ({}, rows + 1, rows - 1, col, ss.str ().c_str ());
        col += ss.str ().size ();
        ss.str ("");
        ss << "ave config ";
        text ({GRAY_ON_CYAN}, rows + 1, rows - 1, col, ss.str ().c_str ());
        col += ss.str ().size ();
        ss.str ("");
        ss << "Q";
        text ({}, rows + 1, rows - 1, col, ss.str ().c_str ());
        col += ss.str ().size ();
        ss.str ("");
        ss << "uit        ";
        text ({GRAY_ON_CYAN}, rows + 1, rows - 1, col, ss.str ().c_str ());
        col += ss.str ().size ();
        if (debug)
        {
            ss.str ("");
            ss << "!";
            text ({}, rows + 1, rows - 1, col, ss.str ().c_str ());
            col += ss.str ().size ();
            ss.str ("");
        ss << "Debug OFF  ";
            text ({RED_ON_CYAN}, rows + 1, rows - 1, col, ss.str ().c_str ());
            col += ss.str ().size ();
        }
    }
};

/// @brief ncurses user interface
class debug_ui
{
    private:
    options &opts;
    int done;
    public:
    /// @brief constructor
    debug_ui (options &opts)
        : opts (opts)
        , done (0)
    {
        std::clog << "options" << std::endl;
        std::clog << "fahrenheit:\t" << opts.get_fahrenheit () << std::endl;
    }
    /// @brief initialize
    void init ()
    {
    }
    /// @brief cleanup
    void release () const
    {
    }
    /// @brief event loop support
    ///
    /// @return true if done
    bool is_done ()
    {
        return done++;
    }
    /// @brief event loop support
    void process (int , const std::string &)
    {
    }
    /// @brief display temps
    ///
    /// @param busses vector of busses
    void show_temps (const busses &bs) const
    {
        for (auto bus : bs)
        {
            std::clog << bus.name << std::endl;
            for (auto chip : bus.chips)
            {
                std::clog << "adapter " << chip.name << std::endl;
                for (auto t : chip.temps)
                {
                    std::clog
                        << round (opts.get_fahrenheit () ? ctof (t.current) : t.current)
                        << (opts.get_fahrenheit () ? 'F' : 'C')
                        << std::endl;
                }
            }
        }
    }
};

} // namespace therm

#endif
