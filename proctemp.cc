/// @file proctemp.cc
/// @brief get proc temperature
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 0.1
/// @date 2011-03-14

#include "proctemp.h"
#include <getopt.h>

using namespace std;
using namespace proctemp;

const string usage = "usage: proctemp [-h|--help] [-f|--fahrenheit] [-g|--gpus]";

template<typename T,typename U>
int check (const T &s, const U &chips, bool fahrenheit)
{
    int status = 0;
    for (auto chip : chips)
    {
        for (auto temp : s.get_temperatures (chip))
        {
            double t = fahrenheit ? ctof (temp.current) : temp.current;
            double h = fahrenheit ? ctof (temp.high) : temp.high;
            double c = fahrenheit ? ctof (temp.critical) : temp.critical;
            clog << ' ' << t;
            if (t > c)
            {
                status = max (status, 2);
                clog << ">" << c << "!!!";
            }
            else if (t > h)
            {
                status = max (status, 1);
                clog << ">" << h;
            }
        }
        clog << endl;
    }
    return status;
}

int main (int argc, char **argv)
{
    try
    {
        // parse the options
        bool fahrenheit = false;
        bool gpus = false;
        static struct option options[] =
        {
            {"help", 0, 0, 'h'},
            {"fahrenheit", 0, 0, 'f'},
            {"gpus", 0, 0, 'g'},
            {NULL, 0, NULL, 0}
        };
        int option_index;
        int arg;
        while ((arg = getopt_long (argc, argv, "hfgd:", options, &option_index)) != -1)
        {
            switch (arg)
            {
                case 'h':
                clog << usage << endl;
                return 0;
                case 'f':
                fahrenheit = true;
                break;
                case 'g':
                gpus = true;
                break;
            }
        };
        // print the options
        clog << "fahrenheit " << fahrenheit << endl;
        clog << "gpus " << gpus << endl;

        // init the sensors library
        sensors s;
        clog << "libsensors version " << s.get_version () << endl;

        clog << "checking " << (gpus ? "GPUs" : "CPUs") <<  endl;
        int status = gpus ?
            check (s, s.get_pci_chips (), fahrenheit) :
            check (s, s.get_isa_chips (), fahrenheit);

        return status;
    }
    catch (const exception &e)
    {
        cerr << e.what () << endl;
        return -1;
    }
}
