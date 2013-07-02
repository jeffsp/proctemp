/// @file proctempn.cc
/// @brief ncurses version of proctemp
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-06-30

#include "proctemp.h"
#include <cassert>
#include <iostream>
#include <sstream>
#include <ncurses.h>

using namespace std;
using namespace proctemp;

/// @brief display using ncurses
class display
{
    private:
    /// @brief screen dimensions
    int rows, cols;
    /// @brief flag for F/C
    bool fahrenheit;
    /// @brief flag for debugging
    bool debug;
    static const int WHITE = COLOR_PAIR(1);
    static const int GREEN = COLOR_PAIR(2);
    static const int YELLOW = COLOR_PAIR(3);
    static const int RED = COLOR_PAIR(4);
    public:
    /// @brief constructor
    display ()
        : fahrenheit (true)
        , debug (false)
    {
        init ();
        labels ();
    }
    /// @brief destructor
    ~display ()
    {
        release ();
    }
    /// @brief initialize ncurses stuff
    void init ()
    {
        initscr ();
        start_color ();
        raw ();
        keypad (stdscr, 1);
        noecho ();
        curs_set (0); // make cursor invisible
        getmaxyx (stdscr, rows, cols);
        init_pair (1, COLOR_WHITE, COLOR_BLACK);
        init_pair (2, COLOR_GREEN, COLOR_BLACK);
        init_pair (3, COLOR_YELLOW, COLOR_BLACK);
        init_pair (4, COLOR_RED, COLOR_BLACK);
        fahrenheit = true; // temperature scale
        timeout (500); // timeout in ms
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
        erase ();
        int row = 0;
        stringstream ss;
        ss.str ("");
        ss << "proctemp version " << proctemp::MAJOR_REVISION << '.' << proctemp::MINOR_REVISION;
        text ({}, rows - 1, 0, ss.str ().c_str ());
        if (debug)
        {
            ss.str ("");
            ss << "ncurses version " << NCURSES_VERSION_MAJOR << '.' << NCURSES_VERSION_MINOR;
            text ({}, row++, cols / 2, ss.str ().c_str ());
            ss.str ("");
            ss << "terminal dimensions " << rows << " X " << cols;
            text ({}, row++, cols / 2, ss.str ().c_str ());
        }
        ss.str ("");
        ss << "";
        text ({}, row++, cols / 2, ss.str ().c_str ());
        ss.str ("");
        ss << "T = toggle temperature scale";
        text ({A_BOLD}, row++, cols / 2, ss.str ().c_str ());
        ss.str ("");
        ss << "Q = quit";
        text ({A_BOLD}, row++, cols / 2, ss.str ().c_str ());
        if (debug)
        {
            ++row;
            ss.str ("");
            ss << "YOU ARE IN DEBUG MODE.";
            text ({A_BOLD,A_BLINK}, row++, cols / 2, ss.str ().c_str ());
            ss.str ("");
            ss << "PRESS '~' TO TURN OFF DEBUG MODE.";
            text ({A_BOLD,A_BLINK}, row++, cols / 2, ss.str ().c_str ());
        }
        refresh ();
    }
    /// @brief ncurses cleanup
    void release () const
    {
        endwin ();
    }
    /// @brief fahrenheit flag access
    bool get_fahrenheit () const
    {
        return fahrenheit;
    }
    /// @brief fahrenheit flag access
    void set_fahrenheit (bool f)
    {
        fahrenheit = f;
    }
    /// @brief debug flag access
    bool get_debug () const
    {
        return debug;
    }
    /// @brief debug flag access
    void set_debug (bool f)
    {
        debug = f;
    }
    /// @brief show temperatures for all CPUS/GPUS
    ///
    /// @tparam T temperature container type
    /// @tparam U name container type
    /// @param temps temperature container
    /// @param names name container
    template<typename T,typename U>
    void show_temps (const T &temps, const U &names) const
    {
        assert (temps.size () == names.size ());
        auto row = 0;
        for (size_t i = 0; i < names.size (); ++i)
        {
            text ({A_BOLD}, row++, 0, names[i].c_str ());
            for (size_t n = 0; n < temps[i].size (); ++n)
            {
                temp_bar (n, row++, 0, temps[i][n]);
            }
        }
    }
    /// @brief draw a temperature bar
    ///
    /// @tparam T temperature type
    /// @param n cpu number
    /// @param i row
    /// @param j col
    /// @param t temperature
    template<typename T>
    void temp_bar (size_t n, int i, int j, T t) const
    {
        if (debug && n == 0)
            t.current = (rand () % int (t.critical + 10 - t.high)) + t.high;
        stringstream ss;
        ss << n;
        text ({}, i, 0, ss.str ().c_str ());
        const int LEFT = 4;
        const int RIGHT = 5;
        const int LEN = cols / 2 - LEFT - RIGHT;
        text ({A_BOLD}, i, j + LEFT - 1, "[");
        text ({A_BOLD}, i, j + LEFT + 1 + LEN + 1, "]");
        ss.str ("");
        ss << int (fahrenheit ? ctof (t.current) : t.current) << (fahrenheit ? 'F' : 'C');
        const int MIN = 40;
        const int MAX = t.critical + 5;
        int current = t.current < MIN ? MIN : (t.current > MAX ? MAX : t.current);
        int len = LEN * (current - MIN) / (MAX - MIN);
        int color = GREEN;
        if (t.current >= t.high)
            color = YELLOW;
        if (t.current >= t.critical)
            color = RED;
        // print the bars
        int k = 0;
        for (; k < len; ++k)
            text ({A_BOLD, color}, i, j + LEFT + 1 + k, "|");
        // print spaces
        for (; k < LEN; ++k)
            text ({}, i, j + LEFT + 1 + k, " ");
        // print the numerical value
        text ({A_BOLD, color}, i, j + LEFT + 1 + LEN - ss.str ().size (), ss.str ().c_str ());
    }
    /// @brief event loop
    void update ()
    {
        if (is_term_resized (rows, cols))
        {
            release ();
            init ();
            labels ();
        }
        refresh ();
    }
};

template<typename S,typename D>
void check (const S &s, const D &d)
{
    vector<vector<temperature>> temps;
    vector<string> bus_names;
    for (auto chip : s.get_isa_chips ())
    {
        // append CPU temps
        temps.push_back (s.get_temperatures (chip));
        bus_names.push_back ("CPUs");
    }
    for (auto chip : s.get_pci_chips ())
    {
        // append GPU temps
        temps.push_back (s.get_temperatures (chip));
        bus_names.push_back ("GPUs");
    }
    d.show_temps (temps, bus_names);
}

int main (int argc, char *argv[])
{
    try
    {
        // init the sensors library
        sensors s;
        // init terminal window
        display d;
        while (true)
        {
            check (s, d);
            auto ch = getch ();
            switch (ch)
            {
                default:
                break;
                case 'q':
                case 'Q':
                    return 0;
                break;
                case 't':
                case 'T':
                    d.set_fahrenheit (!d.get_fahrenheit ());
                break;
                case '~':
                    d.set_debug (!d.get_debug ());
                    d.labels ();
                break;
            }
            d.update ();
        }
        return 0;
    }
    catch (const exception &e)
    {
        cerr << e.what () << endl;
        return -1;
    }
}
