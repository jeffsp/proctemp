/// @file proctempview.cc
/// @brief view processor temperatures
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-06-30

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

#include "ui.h"

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

template<typename U,typename S,typename T>
void main_loop (const S &s, T &opts, const string &config_fn)
{
    U ui (opts);
    while (!ui.is_done ())
    {
        // get temperature data
        vector<vector<vector<temperature>>> temps;
        vector<string> bus_names;
        get_temps (s, temps, bus_names);
        // show it
        ui.show_temps (temps, bus_names);
        // interpret user input
        ui.process (getch (), config_fn);
    }
    // close down window
    ui.release ();
}

int main (int argc, char *argv[])
{
    try
    {
        // init the sensors library
        sensors s;

        // options get saved here
        string config_fn = get_config_dir () + "/proctempviewrc";

        // configurable options
        options opts;

        // if the config file does not exist, write one
        {
            ifstream ifs (config_fn.c_str ());
            if (!ifs)
                write (opts, config_fn);
        }

        // read in the config file
        {
            ifstream ifs (config_fn.c_str ());
            if (!ifs) // if it's not there, notify the user
                clog << "warning: could not read configuration options" << endl;
            else
                read (opts, config_fn);
        }

        // run the main loop
        main_loop<ncurses_ui> (s, opts, config_fn);

        return 0;
    }
    catch (const exception &e)
    {
        cerr << e.what () << endl;
        return -1;
    }
}
