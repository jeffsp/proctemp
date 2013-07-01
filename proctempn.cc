/// @file proctempn.cc
/// @brief ncurses version of proctemp
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-06-30

#include "proctemp.h"
#include <iostream>
#include <ncurses.h>
#include <signal.h>

using namespace std;
using namespace proctemp;

class ncurses
{
    private:
    int rows, cols;
    vector<string> buses;
    public:
    ncurses ()
    {
        initscr ();
        raw ();
        keypad (stdscr, 1);
        noecho ();
    }
    ~ncurses ()
    {
        endwin ();
    }
    void set_buses (vector<string> &b)
    {
        buses = b;
        redraw ();
    }
    void temp (int i, int j, double t, double high, double critical) const
    {
        mvprintw (j + 1, i * cols / 2, "%.0f %.0f %.0f", t, high, critical);
    }
    void redraw ()
    {
        erase ();
        for (size_t i = 0; i < buses.size (); ++i)
            mvprintw (0, i * cols / 2, buses[i].c_str());
        getmaxyx (stdscr, rows, cols);
        //mvprintw (rows - 1, 1, "libsensors version %s\n", s.get_version ().c_str ());
        mvprintw (rows - 1, 0, "press 'q' to quit");
        //mvprintw (rows - 2, 0, "ncurses version %d.%d\n", NCURSES_VERSION_MAJOR,  NCURSES_VERSION_MINOR);
        refresh ();
    }
} n;

void resize (int)
{
    n.redraw ();
}

template<typename C,typename N,typename T>
void check (int bus, const C &chips, const N &n, const T &s, bool fahrenheit)
{
    auto i = 0;
    for (auto chip : chips)
    {
        for (auto temp : s.get_temperatures (chip))
        {
            double t = fahrenheit ? ctof (temp.current) : temp.current;
            double h = fahrenheit ? ctof (temp.high) : temp.high;
            double c = fahrenheit ? ctof (temp.critical) : temp.critical;
            n.temp (bus, i++, t, h, c);
        }
    }
}

template<typename N,typename T>
void check (const N &n, const T &s, bool fahrenheit)
{
    {
    auto chips = s.get_isa_chips ();
    check (0, chips, n, s, fahrenheit);
    }
    {
    auto chips = s.get_pci_chips ();
    check (1, chips, n, s, fahrenheit);
    }
}

int main (int argc, char *argv[])
{
    try
    {
        // install signal handler
        signal (SIGWINCH, resize);
        // label buses
        vector<string> b {"CPU", "GPU"};
        n.set_buses (b);
        // init the sensors library
        sensors s;
        bool fahrenheit = true;
        timeout (500); // timeout in ms
        while (true)
        {
            check (n, s, fahrenheit);
            auto ch = getch ();
            switch (ch)
            {
                default:
                break;
                case 'q':
                    return 0;
                break;
                case 'f':
                    fahrenheit = !fahrenheit;
                break;
            }
            refresh ();
        }
        return 0;
    }
    catch (const exception &e)
    {
        cerr << e.what () << endl;
        return -1;
    }
}
