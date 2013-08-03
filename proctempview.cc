/// @file proctempview.cc
/// @brief view processor temperatures
/// @author Jeff Perry <jeffsp@gmail.com>
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

template<typename U>
void main_loop (const sensors &s, options &opts, const string &config_fn)
{
    U ui (opts);
    while (!ui.is_done ())
    {
        // get temps
        busses b = scan (s);
        // show them
        ui.show_temps (b);
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
        //main_loop<debug_ui> (s, opts, config_fn);

        return 0;
    }
    catch (const exception &e)
    {
        cerr << e.what () << endl;
        return -1;
    }
}
