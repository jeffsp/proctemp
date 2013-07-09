/// @file proctempalert.cc
/// @brief get processor temperature and optionally send an alert if it's too high
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 0.1
/// @date 2013-04-30

#include "proctemp.h"
#include <cmath>
#include <getopt.h>

using namespace std;
using namespace proctemp;

const string usage = "usage: proctempalert [-h '...'|--high_cmd='...'] [-c '...'|--critical_cmd='...'] [-d#|--debug=#] [-?|--help] [-g|--gpus]";

template<typename T,typename U>
int check (const T &s, const U &chips)
{
    int status = 0;
    for (auto chip : chips)
    {
        for (auto temp : s.get_temperatures (chip))
        {
            if (temp.current > temp.critical)
                status = max (status, 2);
            else if (temp.current > temp.high)
                status = max (status, 1);
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
        bool gpus = false;
        int debug = 0;
        string high_cmd;
        string critical_cmd;
        static struct option options[] =
        {
            {"help", 0, 0, '?'},
            {"gpus", 0, 0, 'g'},
            {"debug", 1, 0, 'd'},
            {"high_cmd", 1, 0, 'h'},
            {"critical_cmd", 1, 0, 'c'},
            {NULL, 0, NULL, 0}
        };
        int option_index;
        int arg;
        while ((arg = getopt_long (argc, argv, "?gd:h:c:", options, &option_index)) != -1)
        {
            switch (arg)
            {
                case '?':
                clog << usage << endl;
                return 0;
                case 'g':
                gpus = true;
                break;
                case 'd':
                debug = atoi (optarg);
                break;
                case 'h':
                high_cmd = string (optarg);
                break;
                case 'c':
                critical_cmd = string (optarg);
                break;
            }
        };

        // print version info
        clog << "proctemp version " << MAJOR_REVISION << '.' << MINOR_REVISION << endl;

        // print the options
        clog << "gpus " << gpus << endl;
        clog << "debug " << debug << endl;
        clog << "high_cmd " << high_cmd << endl;
        clog << "critical_cmd " << critical_cmd << endl;

        // return code
        int status;

        // don't check if you are debugging
        if (debug)
            status = debug;
        else
        {
            // init the sensors library
            sensors s;
            clog << "libsensors version " << s.get_version () << endl;

            clog << "checking " << (gpus ? "GPUs" : "CPUs") <<  endl;
            status = gpus ?
                check (s, s.get_pci_chips ()) :
                check (s, s.get_isa_chips ());
        }

        switch (status)
        {
            default:
            case 0:
            break;
            case 1:
            execute (high_cmd);
            break;
            case 2:
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
