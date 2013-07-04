/// @file proctempn.cc
/// @brief ncurses version of proctemp
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-06-30

#include "options.h"
#include "ui.h"
#include <cassert>

using namespace std;
using namespace proctemp;

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
