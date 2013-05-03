/// @file cputempx.cc
/// @brief xwindows version of cputemp
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-05-02

#include "cputemp.h"
#include <iostream>
#include <gtk/gtk.h>

using namespace std;

int main (int argc, char *argv[])
{
    try
    {
        GtkWidget *window;
        gtk_init (&argc, &argv);
        clog << "GTK+ version "
            << gtk_get_major_version () << '.'
            << gtk_get_minor_version () << '.'
            << gtk_get_micro_version ()
            << endl;
        window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
        g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
        gtk_widget_show (window);
        gtk_main ();
        return 0;
    }
    catch (const exception &e)
    {
        cerr << e.what () << endl;
        return -1;
    }
}
