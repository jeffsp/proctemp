/// @file sensors.h
/// @brief sensors/sensors.h wrapper
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

#ifndef SENSORS_H
#define SENSORS_H

#include <iostream>
#include <sensors/sensors.h>
#include <stdexcept>
#include <string>
#include <vector>

namespace proctemp
{

/// @brief total number of busses to scan
const int MAX_BUSSES = SENSORS_BUS_TYPE_HID + 1;

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
            throw std::runtime_error ("could not initialize libsensors");
    }
    /// @brief destructor
    ~sensors ()
    {
        sensors_cleanup ();
    }
    /// @brief get sensors version number
    ///
    /// @return the version
    std::string get_version () const
    {
        return std::string (libsensors_version);
    }
    /// @brief opaque type for high level functions
    typedef const sensors_chip_name *chip;
    /// @brief collection of chips
    typedef std::vector<chip> chips;
    /// @brief get temperatures for all the cores on a chip
    ///
    /// @param c the chip
    ///
    /// @return collection of core temperatures
    std::vector<temperature> get_temperatures (chip c) const
    {
        std::vector<temperature> temps;
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
    std::vector<const sensors_chip_name *> get_chip_names () const
    {
        std::vector<const sensors_chip_name *> chip_names;
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
    std::vector<const sensors_feature *> get_features (const sensors_chip_name *name) const
    {
        std::vector<const sensors_feature *> features;
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
            throw std::runtime_error ("could not get value");
    }
};

} // namespace proctemp

#endif
