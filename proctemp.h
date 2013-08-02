/// @file proctemp.h
/// @brief proctemp
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

#ifndef PROCTEMP_H
#define PROCTEMP_H

#include <iostream>
#include <sensors/sensors.h>
#include <stdexcept>
#include <string>
#include <vector>

namespace proctemp
{

using namespace std;

/// @brief version info
const int MAJOR_REVISION = 0;
const int MINOR_REVISION = 2;

/// @brief sensors definitions
const int MAX_BUSES = 7;

/// @brief convert from fahrenheit to celsius
///
/// @param c temperature in celsius
///
/// @return temperature in fahrenheit
double ctof (const double c)
{
    return c * 9.0 / 5.0 + 32.0;
}
/// @brief temperature reading from a chip
struct temperature
{
    double current;
    double high;
    double critical;
};
/// @brief wrapper for sensors/sensors.h functionality
class sensors
{
    public:
    /// @brief constructor
    sensors ()
    {
        if (sensors_init (0))
            throw runtime_error ("could not initialize libsensors");
    }
    /// @brief destructor
    ~sensors ()
    {
        sensors_cleanup ();
    }
    /// @brief get sensors version number
    ///
    /// @return the version
    string get_version () const
    {
        return string (libsensors_version);
    }
    /// @brief opaque type for high level functions
    typedef const sensors_chip_name *chip;
    /// @brief collection of chips
    typedef vector<chip> chips;
    /// @brief get chips on the ISA bus
    ///
    /// @return chips
    chips get_isa_chips () const
    {
        return get_chips (SENSORS_BUS_TYPE_ISA);
    }
    /// @brief get chips on the PCI bus
    ///
    /// @return chips
    chips get_pci_chips () const
    {
        return get_chips (SENSORS_BUS_TYPE_PCI);
    }
    /// @brief get temperatures for all the cores on a chip
    ///
    /// @param c the chip
    ///
    /// @return collection of core temperatures
    vector<temperature> get_temperatures (chip c) const
    {
        vector<temperature> temps;
        for (auto feature : get_features (c))
        {
            if (feature->type != SENSORS_FEATURE_TEMP)
                continue;
            const sensors_subfeature *input = get_subfeature (c, feature, SENSORS_SUBFEATURE_TEMP_INPUT);
            const sensors_subfeature *max = get_subfeature (c, feature, SENSORS_SUBFEATURE_TEMP_MAX);
            const sensors_subfeature *crit = get_subfeature (c, feature, SENSORS_SUBFEATURE_TEMP_CRIT);
            temperature t { -1, -1, -1 };
            if (input)
                t.current = get_value (c, input);
            if (max)
                t.high = get_value (c, max);
            if (crit)
                t.critical = get_value (c, crit);
            temps.push_back (t);
        }
        return temps;
    }
    /// @brief get collection of chips of a specific type
    ///
    /// @param type chip type
    ///
    /// @return chips
    chips get_chips (const short type) const
    {
        chips c;
        for (auto name : get_chip_names ())
        if (name->bus.type == type)
            c.push_back (name);
        return c;
    }
    private:
    /// @brief get sensors chip names
    ///
    /// @return collection of chip names
    vector<const sensors_chip_name *> get_chip_names () const
    {
        vector<const sensors_chip_name *> chip_names;
        int chip_num = 0;
        const sensors_chip_name *name;
        while ((name = sensors_get_detected_chips (0, &chip_num)))
            chip_names.push_back (name);
        return chip_names;
    }
    /// @brief get sensors features
    ///
    /// @param name chip name
    ///
    /// @return collection of sensors features
    vector<const sensors_feature *> get_features (const sensors_chip_name *name) const
    {
        vector<const sensors_feature *> features;
        const sensors_feature *feature;
        int feature_num = 0;
        while ((feature = sensors_get_features (name, &feature_num)))
            features.push_back (feature);
        return features;
    }
    /// @brief get a sensors subfeature
    ///
    /// @param name chip name
    /// @param feature feature
    /// @param type type of subfeature
    ///
    /// @return subfeature
    const sensors_subfeature *get_subfeature (const sensors_chip_name *name, const sensors_feature *feature, sensors_subfeature_type type) const
    {
        const sensors_subfeature *subfeature;
        if ((subfeature = sensors_get_subfeature (name, feature, type)))
            return subfeature;
        else
            return nullptr;
    }
    /// @brief get the value of a subfeature
    ///
    /// @param name chip name
    /// @param subfeature
    ///
    /// @return subfeature value
    double get_value (const sensors_chip_name *name, const sensors_subfeature *subfeature) const
    {
        double value;
        if (!sensors_get_value (name, subfeature->number, &value))
            return value;
        else
            throw runtime_error ("could not get value");
    }
};

} // namespace proctemp

#endif
