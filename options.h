/// @file options.h
/// @brief configuration options
/// @author Jeff Perry <jeffsp@gmail.com>
/// @version 1.0
/// @date 2013-07-04

#ifndef OPTIONS_H
#define OPTIONS_H

#include "proctemp.h"
#include <fstream>
#include <string>
#include <sys/stat.h>

namespace proctemp
{

using namespace std;

/// @brief configuration options
class options
{
    private:
    /// @brief true if options have changed
    bool dirty;
    /// @brief temperature scale
    bool fahrenheit;
    /// @brief html filename
    string html_filename;
    public:
    /// @brief constructor
    options ()
        : dirty (false)
        , fahrenheit (true)
        , html_filename ("/var/www/proctemp.html")
    {
    }
    /// @brief return true if options have changed
    bool is_dirty ()
    {
        return dirty;
    }
    /// @brief option access
    bool get_fahrenheit () const
    {
        return fahrenheit;
    }
    /// @brief option access
    void set_fahrenheit (bool f)
    {
        if (f == fahrenheit)
            return;
        dirty = true;
        fahrenheit = f;
    }
    /// @brief option access
    string get_html_filename () const
    {
        return html_filename;
    }
    /// @brief option access
    void set_html_filename (const string &s)
    {
        if (s == html_filename)
            return;
        dirty = true;
        html_filename = s;
    }
    /// @brief i/o helper
    friend ostream& operator<< (ostream &s, const options &opts)
    {
        s << "proctemp " << MAJOR_REVISION << ' ' << MINOR_REVISION << endl;
        s << "fahrenheit " << opts.fahrenheit << endl;
        s << "html_filename " << opts.html_filename << endl;
        return s;
    }
    /// @brief i/o helper
    friend istream& operator>> (istream &s, options &opts)
    {
        try
        {
            // get version information
            string name;
            int major, minor;
            s >> name >> major >> minor;
            clog << name << ' ' << major << ' ' << minor << endl;
            if (name != "proctemp")
                throw runtime_error ("warning: error parsing revision number");
            clog << "parsing proctemp configure file with revision number " << major << "." << minor << endl;
            if (major != MAJOR_REVISION)
                throw runtime_error ("warning: configuration file major revision is not the same as this programs's major revision number");
            if (minor > MINOR_REVISION)
                throw runtime_error ("warning: configuration file revision number is newer than this program's revision number");
            // get fahrenheit option
            bool value;
            s >> name >> value;
            clog << name << ' ' << value << endl;
            if (name != "fahrenheit")
                throw runtime_error ("warning: error parsing fahrenheit option");
            opts.set_fahrenheit (value);
            // get html_filename option
            string fn;
            s >> name >> fn;
            clog << name << ' ' << fn << endl;
            if (name != "html_filename")
                throw runtime_error ("warning: error parsing html_filename option");
            opts.set_html_filename (fn);
            // if it was read correctly, it won't need to get written back out
            opts.dirty = false;
        }
        catch (const exception &e)
        {
            cerr << e.what () << endl;
            clog << "warning: cannot parse configuration file" << endl;
            // set dirty so it will get written
            opts.dirty = true;
        }
        return s;
    }
};

/// @brief helper
///
/// @param opts options
/// @param fn filename
void read (options &opts, const string &fn)
{
    clog << "reading configuration file " << fn << endl;
    ifstream ifs (fn.c_str ());
    if (!ifs)
        throw runtime_error ("could not open config file for reading");
    ifs >> opts;
}

/// @brief helper
///
/// @param opts options
/// @param fn filename
void write (const options &opts, const string &fn)
{
    clog << "writing configuration file " << fn << endl;
    ofstream ofs (fn.c_str ());
    if (!ofs)
        throw runtime_error ("could not open config file for writing");
    ofs << opts;
}

/// @brief get the name of the configuration file, creating the directory structure if needed
///
/// @return name of the config file
string get_config_filename ()
{
    string config_home;
    if (getenv ("XDG_CONFIG_HOME"))
        config_home = getenv ("XDG_CONFIG_HOME");
    else if (getenv ("HOME"))
        config_home = getenv ("HOME") + string ("/.config");
    else
        config_home = "~/.config";
    config_home += "/proctemp";
    struct stat sb;
    if (stat (config_home.c_str (), &sb) == -1)
    {
        clog << "creating config file directory " << config_home << endl;
        mkdir (config_home.c_str (), 0700);
    }
    if (stat (config_home.c_str (), &sb) == -1)
        throw runtime_error ("could not create config file directory");
    return config_home + "/proctemprc";
};

} // namespace proctemp

#endif
