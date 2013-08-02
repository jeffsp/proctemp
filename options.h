/// @file options.h
/// @brief configuration options
/// @author Jeff Perry <jeffsp@gmail.com>
/// @date 2013-07-04

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

#ifndef OPTIONS_H
#define OPTIONS_H

#include "proctemp.h"
#include <fstream>
#include <string>
#include <sys/stat.h>

namespace proctemp
{

using namespace std;

/// @brief configuration option
///
/// @tparam T option type
template<typename T>
struct option
{
    /// @brief constructor
    ///
    /// @param value option value
    /// @param name option name
    option (const T &value, const string &name)
        : value (value), name (name) { }
    T value;
    string name;
    /// @brief read an option
    ///
    /// @tparam S stream type
    /// @param s stream
    template<typename S>
    void parse (S &s)
    {
        //clog << "parsing " << name << " option" << endl;
        string tmp_name;
        T tmp_value;
        s >> tmp_name >> tmp_value;
        //clog << tmp_name << ' ' << tmp_value << endl;
        if (name != tmp_name)
            throw runtime_error ("warning: didn't get expected option");
        value = tmp_value;
    }
};

/// @brief configuration options
class options
{
    private:
    /// @brief version information
    option<int> major_revision;
    /// @brief version information
    option<int> minor_revision;
    /// @brief temperature scale
    option<bool> fahrenheit;
    public:
    /// @brief constructor
    options ()
        : major_revision (MAJOR_REVISION, "major_revision")
        , minor_revision (MINOR_REVISION, "minor_revision")
        , fahrenheit (true, "fahrenheit")
    {
    }
    /// @brief option access
    bool get_fahrenheit () const
    {
        return fahrenheit.value;
    }
    /// @brief option access
    void set_fahrenheit (bool f)
    {
        if (f == fahrenheit.value)
            return;
        fahrenheit.value = f;
    }
    /// @brief i/o helper
    friend ostream& operator<< (ostream &s, const options &opts)
    {
        s << opts.major_revision.name << " " << opts.major_revision.value << endl;
        s << opts.minor_revision.name << " " << opts.minor_revision.value << endl;
        s << opts.fahrenheit.name << " " << opts.fahrenheit.value << endl;
        return s;
    }
    /// @brief i/o helper
    friend istream& operator>> (istream &s, options &opts)
    {
        try
        {
            // get version information
            opts.major_revision.parse (s);
            opts.minor_revision.parse (s);
            if (opts.major_revision.value != MAJOR_REVISION)
                throw runtime_error ("warning: configuration file major revision is not the same as this programs's major revision number");
            if (opts.minor_revision.value > MINOR_REVISION)
                throw runtime_error ("warning: configuration file revision number is newer than this program's revision number");
            opts.fahrenheit.parse (s);
        }
        catch (const exception &e)
        {
            cerr << e.what () << endl;
            clog << "cannot parse configuration file" << endl;
            clog << "setting options to their defaults" << endl;
            // set to default
            opts = options ();
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

/// @brief get the directory of the configuration file, creating the directory if needed
///
/// @return name of the config directory
string get_config_dir ()
{
    string config_dir;
    if (getenv ("XDG_CONFIG_HOME"))
        config_dir = getenv ("XDG_CONFIG_HOME");
    else if (getenv ("HOME"))
        config_dir = getenv ("HOME") + string ("/.config");
    else
        config_dir = "~/.config";
    config_dir += "/proctemp";
    struct stat sb;
    if (stat (config_dir.c_str (), &sb) == -1)
    {
        clog << "creating config file directory " << config_dir << endl;
        mkdir (config_dir.c_str (), 0700);
    }
    if (stat (config_dir.c_str (), &sb) == -1)
        throw runtime_error ("could not create config file directory");
    return config_dir;
};

} // namespace proctemp

#endif
