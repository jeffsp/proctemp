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
        s << "proctemp " << MAJOR_REVISION << ' ' << MINOR_REVISION << endl;
        s << "fahrenheit " << opts.fahrenheit << endl;
        return s;
    }
    /// @brief i/o helper
    friend std::istream& operator>> (std::istream &s, options &opts)
    {
        try
        {
            string name;
            int major, minor;
            s >> name >> major >> minor;
            clog << name << ' ' << major << ' ' << minor << endl;
            if (name != "proctemp")
                throw runtime_error ("warning: error parsing revision number");
            clog << "parsing proctemp configure file with revision number " << major << "." << minor << endl;
            if (major != MAJOR_REVISION)
                throw runtime_error ("warning: configuration file major revision is not the same this programs's major revision number");
            if (minor > MINOR_REVISION)
                throw runtime_error ("warning: configuration file revision number is newer than this program's revision number");
            bool value;
            s >> name >> value;
            clog << name << ' ' << value << endl;
            if (name != "fahrenheit")
                throw runtime_error ("warning: error parsing fahrenheit option");
            opts.set_fahrenheit (value);
            // if it was read correctly, it won't need to get written back out
            opts.dirty = false;
        }
        catch (const exception &e)
        {
            cerr << e.what () << endl;
            clog << "warning: cannot parse configuration file" << endl;
            // set dirty so it will get written
            opts.dirty = true;
        }
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
                text ({}, row++, 0, "%s(%d)", names[bus].c_str (), i);
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
                    ss << int (opts.get_fahrenheit () ? ctof (t.current) : t.current) << (opts.get_fahrenheit () ? 'F' : 'C');
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
    // check two buses
    temps.resize (2);
    temps[0].clear ();
    temps[1].clear ();
    bus_names.resize (2);
    bus_names[0] = "CPU";
    bus_names[1] = "GPU";
    // each bus may have more than one chip
    for (auto chip : s.get_isa_chips ())
        // each chip may have more than one processor
        temps[0].push_back (s.get_temperatures (chip));
    // each bus may have more than one chip
    for (auto chip : s.get_pci_chips ())
        // each chip may have more than one processor
        temps[1].push_back (s.get_temperatures (chip));
}

/// @brief helper
///
/// @param opts options
/// @param fn filename
void read (options &opts, const string &fn)
{
    clog << "reading configuration file " << fn << endl;
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
    clog << "writing configuration file " << fn << endl;
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
            vector<vector<vector<temperature>>> temps;
            vector<string> bus_names;
            get_temps (s, temps, bus_names);
            // show it
            ui.show_temps (temps, bus_names);
            // interpret user input
            ui.process (getch ());
        }
        // close down console window
        ui.release ();
        // save options if any changed
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
