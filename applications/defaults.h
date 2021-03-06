/*
 Copyright 2019 Claroworks

 written by Mike Wilson mail4mikew@gmail.com

 This file is part of an application designed to work with VESC firmware,
 and is intended for use with special use vehicles.

 This firmware is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This firmware is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef APPLICATIONS_DIVEX_DEFAULTS_H_
#define APPLICATIONS_DIVEX_DEFAULTS_H_

#define USE_SAFETY_SPEED 1

// trigger timing values used for click timing
#define TRIG_ON_TOUT_MS 400			// click timeout when trigger is on
#define TRIG_OFF_TOUT_MS 500		// click timeout when trigger is off

// speed when user goes on trigger the first time
#define SPEED_DEFAULT 3

// ERPM PROGRAMMED SPEED
#define SPEEDS1 2079
#define SPEEDS2 2601
#define SPEEDS3 3123
#define SPEEDS4 3645
#define SPEEDS5 4167
#define SPEEDS6 4690
#define SPEEDS7 5212
#define SPEEDS8 5735

// CURRENT LIMIT PER PROGRAMMED SPEED
#define LIMITS1 1.344
#define LIMITS2 2.631
#define LIMITS3 4.554
#define LIMITS4 7.241
#define LIMITS5 10.823
#define LIMITS6 15.427
#define LIMITS7 21.182
#define LIMITS8 28.219

// BATTERY DISPLAY (PER DISPLAYED BAR)
#define DISP_BATT_VOLT1	34.0
#define DISP_BATT_VOLT2	36.0
#define DISP_BATT_VOLT3	38.0

#define SPEED_RAMPING_RATE 2400 // RPMS per second - approximately 1/4 second per speed increase

#define MIGRATE_SPEED_MILLISECONDS 5000 // time period to change speeds during off-trigger toward SPEED_DEFAULT

// SAFETY SPEED SETTINGS
#define SAFETY_SPEED_GUARD_HIGH 3.5 // Amps. Currents above this indicate an obstruction
#define SAFETY_SPEED_GUARD_LOW  1.7 // Amps. Currents below this indicate running out-of-water
#define SAFETY_SPEED_BI_LIMIT   0.30 // Amps. Battery current limit while in guard mode.
#define SAFETY_SPEED_ERPM       900  // running ERPM while in guard mode.
#define SAFETY_SPEED_MAX_ERPM   1500 // maximum ERPM in guard mode, when it tries to catch up as it becomes unblocked

#define RUNNING_SAFE_OK_CT 	20		// count at 20Hz at which we will confirm running in water with no obstructions
#define SAFETY_FILTER_ALPHA 0.2

// DISPLAY SETTINGS
#define DISP_BRIGHTNESS 6 // 0 to 15 (Max)
#define DISP_ROTATION 0 // 0 to 3 - for fixing differences in display hardware
#define DISP_POWER_ON_OFFTIME 10000	// after power on - milliseconds until display stops
#define DISP_OFF_TRIGGER_BEG_MS 3000 // after OFF-TRIGGER, begin display - time for battery to settle
#define DISP_OFF_TRIG_DURATION_MS 6000 // after OFF-TRIGGER, leave the display on for this time period
#define DISP_ON_TRIGGER_SPEED_MS 3500 // after ON-TRIGGER, time that speed is shown

// BATTERY IMBALANCE
#define BATTERY_MAX_IMBALANCE 2.0 // Volts that batteries are allowed to be different, to disallow overdischarge
#define BATTERY2_SENSE_RATIO 14 // ratio of voltage divider resistors. Normally 141K to 10K.

// LOGGING
#define LOGGING_OFF 0

#endif /* APPLICATIONS_DIVEX_DEFAULTS_H_ */
