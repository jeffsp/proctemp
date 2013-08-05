/// @file thermalert.cc
/// @brief get processor temperature and optionally send an alert if it's too high
/// @author Jeff Perry <jeffsp@gmail.com>
/// @date 2013-04-30

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

#include "therm.h"
#include <cmath>
#include <getopt.h>

using namespace std;
using namespace therm;

const string usage = "usage: thermalert [-h '...'|--high_cmd='...'] [-c '...'|--critical_cmd='...'] [-b#|--bus_id=#] [-d#|--debug=#] [-?|--help]";

int check (const busses &b, unsigned bus_id)
{
    int status = 0;
    for (size_t i = 0; i < b.size (); ++i)
    {
        // skip the bus if specified
        if (bus_id != ~0u && bus_id != b[i].id)
            continue;
        // print bus id
        clog << "[" << b[i].id << "] " << b[i].name << endl;
        // print temps
        for (auto chip : b[i].chips)
        {
            for (auto t : chip.temps)
            {
                clog
                    << "    " << t.current
                    << " " << t.high
                    << " " << t.critical
                    << endl;
                if (t.critical > 0 && t.current > t.critical)
                    status = max (status, 2);
                else if (t.high > 0 && t.current > t.high)
                    status = max (status, 1);
            }
        }
    }
    return status;
}

void execute (const string &cmd)
{
    clog << "executing '" << cmd << "'" << endl;
    if (system (cmd.c_str ()) == -1)
        throw runtime_error ("could not execute command");
}

int main (int argc, char **argv)
{
    try
    {
        // parse the options
        int debug = 0;
        string high_cmd;
        string critical_cmd;
        unsigned bus_id = ~0u;
        static struct option options[] =
        {
            {"help", 0, 0, 'h'},
            {"debug", 1, 0, 'd'},
            {"high_cmd", 1, 0, 'i'},
            {"critical_cmd", 1, 0, 'c'},
            {"bus", 1, 0, 'b'},
            {NULL, 0, NULL, 0}
        };
        int option_index;
        int arg;
        while ((arg = getopt_long (argc, argv, "hd:i:c:b:", options, &option_index)) != -1)
        {
            switch (arg)
            {
                default:
                    throw runtime_error ("unknown option specified");
                case 'h':
                clog << usage << endl;
                return 0;
                case 'd':
                debug = atoi (optarg);
                break;
                case 'i':
                high_cmd = string (optarg);
                break;
                case 'c':
                critical_cmd = string (optarg);
                break;
                case 'b':
                bus_id = atoi (optarg);
                break;
            }
        };

        // print version info
        clog << "therm version " << MAJOR_REVISION << '.' << MINOR_REVISION << endl;

        // print the options
        clog << "debug=" << debug << endl;
        clog << "high_cmd=\"" << high_cmd << "\"" << endl;
        clog << "critical_cmd=\"" << critical_cmd << "\"" << endl;
        clog << "bus_id=" << bus_id << endl;

        // init the sensors library
        sensors s;
        busses b = scan (s);

        // return code
        int status;

        // don't check if you are debugging
        if (debug)
            status = debug;
        else
        {
            clog << "libsensors version " << s.get_version () << endl;
            clog << "checking temperatures" <<  endl;
            status = check (b, bus_id);
        }

        switch (status)
        {
            default:
            case 0:
            clog << "temperatures are normal" << endl;
            break;
            case 1:
            clog << "temperatures are high" << endl;
            execute (high_cmd);
            break;
            case 2:
            clog << "temperatures are critical" << endl;
            execute (critical_cmd);
            break;
        }

        return status;
    }
    catch (const exception &e)
    {
        cerr << e.what () << endl;
        return -1;
    }
}
