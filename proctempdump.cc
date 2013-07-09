/// @file proctempdump.cc
/// @brief dump processor temperatures to the console
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 0.1
/// @date 2013-07-09

#include "proctemp.h"
#include <cmath>
#include <getopt.h>

using namespace std;
using namespace proctemp;

const string usage = "usage: proctempdump [-?|--help] [-f|--fahrenheit]";

template<typename T,typename U>
void dump (const T &s, const U &chips, bool fahrenheit)
{
    for (auto chip : chips)
    {
        for (auto temp : s.get_temperatures (chip))
        {
            double t = fahrenheit ? ctof (temp.current) : temp.current;
            double h = fahrenheit ? ctof (temp.high) : temp.high;
            double c = fahrenheit ? ctof (temp.critical) : temp.critical;
            cout << ' ' << round (t);
            if (t > c)
                cout << ">" << c << "!!!";
            else if (t > h)
                cout << ">" << h;
        }
        cout << endl;
    }
}


int main (int argc, char **argv)
{
    try
    {
        // parse the options
        bool fahrenheit = false;
        static struct option options[] =
        {
            {"help", 0, 0, '?'},
            {"fahrenheit", 0, 0, 'f'},
            {NULL, 0, NULL, 0}
        };
        int option_index;
        int arg;
        while ((arg = getopt_long (argc, argv, "?f", options, &option_index)) != -1)
        {
            switch (arg)
            {
                case '?':
                clog << usage << endl;
                return 0;
                case 'f':
                fahrenheit = true;
                break;
            }
        };

        // print version info
        cout << "proctemp version " << MAJOR_REVISION << '.' << MINOR_REVISION << endl;

        // init the sensors library
        sensors s;
        cout << "libsensors version " << s.get_version () << endl;

        cout << "CPUs" << endl;
        dump (s, s.get_isa_chips (), fahrenheit);
        cout << "GPUs" << endl;
        dump (s, s.get_pci_chips (), fahrenheit);

        return 0;
    }
    catch (const exception &e)
    {
        cerr << e.what () << endl;
        return -1;
    }
}
