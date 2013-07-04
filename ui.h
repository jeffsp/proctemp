/// @file ui.h
/// @brief user interface
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-07-04

#ifndef UI_H
#define UI_H

#include <cassert>
#include <cmath>
#include <iostream>
#include <ncurses.h>
#include <sstream>

namespace proctemp
{

using namespace std;

/// @brief ncurses user_interface
class user_interface
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
    public:
    /// @brief constructor
    user_interface (options &opts)
        : opts (opts)
        , done (false)
        , debug (false)
    {
        init ();
        labels ();
    }
    /// @brief destructor
    ~user_interface ()
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
    void process (int ch)
    {
        switch (ch)
        {
            default:
            break;
            case 'q':
            case 'Q':
            done = true;
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
    /// @tparam T vector of temps type
    /// @tparam U vector of names type
    /// @param temps vector of temps
    /// @param names vector of names
    template<typename T,typename U>
    void show_temps (const T &temps, const U &names) const
    {
        assert (temps.size () == names.size ());
        // get the width of the cpu number column
        size_t max_cpus = 0;
        for (size_t bus = 0; bus < temps.size (); ++bus)
            for (size_t i = 0; i < temps[bus].size (); ++i)
                if (temps[bus][i].size () > max_cpus)
                    max_cpus = temps[bus][i].size ();
        stringstream ss;
        ss << max_cpus;
        // length of largest number plus a space
        const int indent1 = ss.str ().size () + 1;
        // assumes temps are 3 digits at most, plus the C or F, plus a space
        const int indent2 = indent1 + 5;
        // print the temperatures
        auto row = 0;
        for (size_t bus = 0; bus < temps.size (); ++bus)
        {
            for (size_t i = 0; i < temps[bus].size (); ++i)
            {
                // don't print on last line
                if (row + 1 == rows)
                    continue;
                text ({}, row++, 0, "%s%d", names[bus].c_str (), i);
                for (size_t n = 0; n < temps[bus][i].size (); ++n)
                {
                    // don't print on last line
                    if (row + 1 == rows)
                        continue;
                    temperature t = temps[bus][i][n];
                    // print the cpu number
                    if (debug && !(rand () % temps[bus][i].size ()))
                        t.current = (rand () % int (t.critical + 10 - t.high)) + t.high;
                    stringstream ss;
                    ss << n;
                    text ({}, row, 0, ss.str ().c_str ());
                    // print the numerical value
                    ss.str ("");
                    ss << round (opts.get_fahrenheit () ? ctof (t.current) : t.current) << (opts.get_fahrenheit () ? 'F' : 'C');
                    int color = GREEN;
                    if (t.current >= t.high)
                        color = YELLOW;
                    if (t.current >= t.critical)
                        color = RED;
                    text ({A_BOLD, color}, row, indent1, "%4s", ss.str ().c_str ());
                    // print the bar
                    const int size = cols / 2 - indent2 - 5;
                    temp_bar (n, row++, indent2, size, t);
                }
            }
        }
    }
    private:
    /// @brief draw a temperature bar
    ///
    /// @tparam T temperature type
    /// @param n cpu number
    /// @param i row
    /// @param j col
    /// @param size bar length
    /// @param t temperature
    template<typename T>
    void temp_bar (size_t n, int i, int j, int size, T t) const
    {
        text ({A_BOLD}, i, j, "[");
        text ({A_BOLD}, i, j + size - 1, "]");
        const int MIN = 40;
        const int MAX = t.critical + 5;
        int current = t.current < MIN ? MIN : (t.current > MAX ? MAX : t.current);
        int len = size * (current - MIN) / (MAX - MIN);
        for (int k = 1; k + 1 < size; ++k)
        {
            int color;
            if (k < size * (t.high - MIN) / (MAX - MIN))
                color = GREEN;
            else if (k < size * (t.critical - MIN) / (MAX - MIN))
                color = YELLOW;
            else
                color = RED;
            if (k < len)
                text ({A_BOLD, A_REVERSE, color}, i, j + k, " ");
            else
                text ({A_BOLD, color}, i, j + k, "-");
        }
    }
    /// @brief draw normal style text
    ///
    /// @param attrs vector of attrs
    /// @param r row
    /// @param c col
    /// @param s string
    /// @param args print args
    template<typename... T>
    void text (const vector<int> &attrs, int r, int c, const char *s, T... args) const
    {
        for (auto a : attrs)
            attron (a);
        mvprintw (r, c, s, args...);
        for (auto a : attrs)
            attroff (a);
    }
    /// @brief draw labels
    void labels () const
    {
        int row = 0;
        stringstream ss;
        ss.str ("");
        ss << "proctemp version " << proctemp::MAJOR_REVISION << '.' << proctemp::MINOR_REVISION;
        text ({A_BOLD, BLUE}, rows - 1, 0, ss.str ().c_str ());
        ss.str ("");
        ss << "T = change Temperature scale";
        text ({}, row++, cols / 2, ss.str ().c_str ());
        ss.str ("");
        ss << "Q = Quit";
        text ({}, row++, cols / 2, ss.str ().c_str ());
        if (debug)
        {
            ++row;
            ss.str ("");
            ss << "ncurses version " << NCURSES_VERSION_MAJOR << '.' << NCURSES_VERSION_MINOR;
            text ({}, row++, cols / 2, ss.str ().c_str ());
            ss.str ("");
            ss << "terminal dimensions " << rows << " X " << cols;
            text ({}, row++, cols / 2, ss.str ().c_str ());
            ++row;
            ss.str ("");
            ss << "YOU ARE IN DEBUG MODE.";
            text ({}, row++, cols / 2, ss.str ().c_str ());
            ss.str ("");
            ss << "PRESS '!' TO TURN OFF DEBUG MODE.";
            text ({}, row++, cols / 2, ss.str ().c_str ());
        }
    }
};

} // namespace proctemp

#endif
