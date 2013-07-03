/// @file proctempn.cc
/// @brief ncurses version of proctemp
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-06-30

#include "proctemp.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
#include <ncurses.h>
#include <sys/stat.h>

using namespace std;
using namespace proctemp;

/// @brief configuration options
class options
{
    private:
    /// @brief true if options have changed
    bool dirty;
    /// @brief temperature scale
    bool fahrenheit;
    public:
    /// @brief constructor
    options ()
        : dirty (false)
        , fahrenheit (true)
    {
    }
    /// @brief return true if options have changed
    bool is_dirty ()
    {
        return dirty;
    }
    /// @brief option access
    bool get_fahrenheit () const
    {
        return fahrenheit;
    }
    /// @brief option access
    void set_fahrenheit (bool f)
    {
        if (f == fahrenheit)
            return;
        dirty = true;
        fahrenheit = f;
    }
    /// @brief i/o helper
    friend std::ostream& operator<< (std::ostream &s, const options &opts)
    {
        s << opts.fahrenheit;
        return s;
    }
    /// @brief i/o helper
    friend std::istream& operator>> (std::istream &s, options &opts)
    {
        opts.dirty = false;
        s >> opts.fahrenheit;
        return s;
    }
};

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
            case 'd':
            case 'D':
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
        auto row = 0;
        for (size_t i = 0; i < names.size (); ++i)
        {
            text ({}, row++, 0, names[i].c_str ());
            for (size_t n = 0; n < temps[i].size (); ++n)
            {
                temp_bar (n, row++, 0, temps[i][n]);
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
    /// @param t temperature
    template<typename T>
    void temp_bar (size_t n, int i, int j, T t) const
    {
        if (debug && n == 0)
            t.current = (rand () % int (t.critical + 10 - t.high)) + t.high;
        stringstream ss;
        ss << n;
        text ({}, i, 0, ss.str ().c_str ());
        const int LEFT = 8;
        const int RIGHT = 5;
        const int SIZE = cols / 2 - LEFT - RIGHT;
        text ({A_BOLD}, i, j + LEFT , "[");
        text ({A_BOLD}, i, j + LEFT + SIZE + 1, "]");
        ss.str ("");
        ss << int (opts.get_fahrenheit () ? ctof (t.current) : t.current) << (opts.get_fahrenheit () ? 'F' : 'C');
        const int MIN = 40;
        const int MAX = t.critical + 5;
        int current = t.current < MIN ? MIN : (t.current > MAX ? MAX : t.current);
        int len = SIZE * (current - MIN) / (MAX - MIN);
        int color = GREEN;
        if (t.current >= t.high)
            color = YELLOW;
        if (t.current >= t.critical)
            color = RED;
        // print the numerical value
        text ({A_BOLD, color}, i, 3, ss.str ().c_str ());
        for (int k = 0; k < SIZE; ++k)
        {
            int color;
            if (k < SIZE * (t.high - MIN) / (MAX - MIN))
                color = GREEN;
            else if (k < SIZE * (t.critical - MIN) / (MAX - MIN))
                color = YELLOW;
            else
                color = RED;
            if (k < len)
                text ({A_BOLD, A_REVERSE, color}, i, j + LEFT + 1 + k, " ");
            else
                text ({A_BOLD, color}, i, j + LEFT + 1 + k, "-");
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
        text ({}, rows - 1, 0, ss.str ().c_str ());
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
            ss << "ncurses version " << NCURSES_VERSION_MAJOR << '.' << NCURSES_VERSION_MINOR;
            text ({}, row++, cols / 2, ss.str ().c_str ());
            ss.str ("");
            ss << "terminal dimensions " << rows << " X " << cols;
            text ({}, row++, cols / 2, ss.str ().c_str ());
            ++row;
            ss.str ("");
            ss << "YOU ARE IN DEBUG MODE.";
            text ({A_BOLD}, row++, cols / 2, ss.str ().c_str ());
            ss.str ("");
            ss << "PRESS 'D' TO TURN OFF DEBUG MODE.";
            text ({A_BOLD}, row++, cols / 2, ss.str ().c_str ());
        }
    }
};

/// @brief get sensor data
///
/// @tparam S sensors type
/// @tparam T temperature output type
/// @tparam U bus name output type
/// @param s sensors
/// @param temps temperature output
/// @param bus_names bus names output
template<typename S,typename T,typename U>
void get_temps (const S &s, T &temps, U &bus_names)
{
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
}

/// @brief helper
///
/// @param opts options
/// @param fn filename
void read (options &opts, const string &fn)
{
    ifstream ifs (fn.c_str ());
    if (!ifs)
        throw runtime_error ("could not open config file for reading");
    ifs >> opts;
}

/// @brief helper
///
/// @param opts options
/// @param fn filename
void write (const options &opts, const string &fn)
{
    ofstream ofs (fn.c_str ());
    if (!ofs)
        throw runtime_error ("could not open config file for writing");
    ofs << opts;
}

/// @brief get the name of the configuration file, creating the directory structure if needed
///
/// @return name of the config file
string get_config_filename ()
{
    string config_home;
    if (getenv ("XDG_CONFIG_HOME"))
        config_home = getenv ("XDG_CONFIG_HOME");
    else if (getenv ("HOME"))
        config_home = getenv ("HOME") + string ("/.config");
    else
        config_home = "~/.config";
    config_home += "/proctemp";
    struct stat sb;
    if (stat (config_home.c_str (), &sb) == -1)
    {
        clog << "creating config file directory " << config_home << endl;
        mkdir (config_home.c_str (), 0700);
    }
    if (stat (config_home.c_str (), &sb) == -1)
        throw runtime_error ("could not create config file directory");
    return config_home + "/proctemprc";
};

int main (int argc, char *argv[])
{
    try
    {
        // init the sensors library
        sensors s;
        // options get saved here
        string config_fn = get_config_filename ();
        // configurable options
        options opts;
        {
            // does the config file exist?
            ifstream ifs (config_fn.c_str ());
            if (!ifs) // no, write it
                write (opts, config_fn);
        }
        {
            // does the config file exist?
            ifstream ifs (config_fn.c_str ());
            if (!ifs) // if not, it's an error
                throw runtime_error ("can't create configuration file");
            // read in the current options
            read (opts, config_fn);
        }
        // init terminal window
        user_interface ui (opts);
        while (!ui.is_done ())
        {
            // get temperature data
            vector<vector<temperature>> temps;
            vector<string> bus_names;
            get_temps (s, temps, bus_names);
            // show it
            ui.show_temps (temps, bus_names);
            // interpret user input
            ui.process (getch ());
        }
        // save them if any changed
        if (opts.is_dirty ())
            write (opts, config_fn);
        return 0;
    }
    catch (const exception &e)
    {
        cerr << e.what () << endl;
        return -1;
    }
}
