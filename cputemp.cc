/// @file cputemp.cc
/// @brief get cpu temperature
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 0.1
/// @date 2011-03-14

#include "cputemp.h"
#include <getopt.h>

using namespace std;
using namespace cputemp;

const string usage = "usage: cputemp [-h|--help] [-f|--fahrenheit] [-c|--cpus] [-g|--gpus]";

template<typename T,typename U>
void print (const T &s, const U &chips, bool fahrenheit)
{
    for (auto chip : chips)
    {
        for (auto temp : s.get_temperatures (chip))
        {
            double t = fahrenheit ? ctof (temp.current) : temp.current;
            clog << ' ' << t;
            if (temp.current > temp.critical)
                clog << '!';
            else if (temp.current > temp.high)
                clog << '^';
        }
        clog << endl;
    }
}

int main (int argc, char **argv)
{
    try
    {
        // parse the options
        bool fahrenheit = false;
        bool cpus = true;
        bool gpus = false;
        static struct option options[] =
        {
            {"help", 0, 0, 'h'},
            {"fahrenheit", 0, 0, 'f'},
            {"cpus", 0, 0, 'c'},
            {"gpus", 0, 0, 'g'},
            {NULL, 0, NULL, 0}
        };
        int option_index;
        int arg;
        while ((arg = getopt_long (argc, argv, "hfcgd:", options, &option_index)) != -1)
        {
            switch (arg)
            {
                case 'h':
                clog << usage << endl;
                return 0;
                case 'f':
                fahrenheit = true;
                break;
                case 'c':
                cpus = !cpus;
                break;
                case 'g':
                gpus = !gpus;
                break;
            }
        };
        // print the options
        clog << "fahrenheit " << fahrenheit << endl;
        clog << "cpus " << cpus << endl;
        clog << "gpus " << gpus << endl;

        // you must report on at least one chip type
        if (!cpus && !gpus)
            throw runtime_error ("nothing to do!");

        // init the sensors library
        sensors s;
        clog << "libsensors version " << s.get_version () << endl;

        if (cpus)
            print (s, s.get_isa_chips (), fahrenheit);

        if (gpus)
            print (s, s.get_pci_chips (), fahrenheit);

        return 0;
    }
    catch (const exception &e)
    {
        cerr << e.what () << endl;
        return -1;
    }
}
