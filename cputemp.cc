/// @file cputemp.cc
/// @brief get cpu temperature
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 0.1
/// @date 2011-03-14

#include "cputemp.h"
#include <getopt.h>

using namespace std;
using namespace cputemp;

const string usage = "usage: cputemp [-h|--help] [-f|--fahrenheit]";

int main (int argc, char **argv)
{
    try
    {
        // parse the options
        bool fahrenheit;
        static struct option options[] =
        {
            {"fahrenheit", 0, 0, 'f'},
            {"help", 0, 0, 'h'},
            {NULL, 0, NULL, 0}
        };
        int option_index;
        int arg;
        while ((arg = getopt_long (argc, argv, "fh", options, &option_index)) != -1)
        {
            switch (arg)
            {
                case 'f':
                fahrenheit = true;
                break;

                case 'h':
                clog << usage << endl;
                return 0;
            }
        };

        // init the sensors library
        sensors s;
        std::clog << "libsensors version " << s.get_version () << std::endl;

        // get CPUs and print current temperatures
        sensors::chips cpus = s.get_isa_chips ();
        std::clog << "CPUs" << std::endl;
        for (auto chip : cpus)
            for (auto temp : s.get_temperatures (chip))
                if (fahrenheit)
                    std::clog << ctof (temp) << std::endl;
                else
                    std::clog << temp << std::endl;

        // get GPUs and print current temperatures
        sensors::chips gpus = s.get_pci_chips ();
        std::clog << "GPUs" << std::endl;
        for (auto chip : gpus)
            for (auto temp : s.get_temperatures (chip))
                if (fahrenheit)
                    std::clog << ctof (temp) << std::endl;
                else
                    std::clog << temp << std::endl;

        return 0;
    }
    catch (const exception &e)
    {
        cerr << e.what () << endl;
        return -1;
    }
}
