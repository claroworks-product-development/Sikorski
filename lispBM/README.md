# LispBM

This is the VESC-integration of [lispBM](https://github.com/svenssonjoel/lispBM) written by Joel Svensson. It allows the VESC to run lisp-programs in a sandboxed environment.

### Feature Overview

* Development and testing in VESC Tool with variable live monitoring and plotting as well as CPU and memory monitoring.
* Sandboxed environment, meaning that the Lisp code (hopefully) cannot freeze or crash the rest of the VESC code when it gets stuck or runs out of heap or stack memory.
* The application runs on the VESC itself without the need for having VESC Tool connected and is stored in flash memory.
* When a lisp-application is written to the VESC it is automatically started on each boot.

## Documentation

Basics about LispBM are documented [here](http://svenssonjoel.github.io/lbmdoc/html/lbmref.html). The VESC-specific extensions are documented in this section. Note that VESC Tool includes a collection of examples that can be used as a starting point for using lisp on the VESC.

### Various Commands

#### print

```clj
(print arg1 ... argN)
```

Print to the VESC Tool Lisp console. Example:

```clj
(print "Hello World")
```

Should work for all types.

#### timeout-reset

```clj
(timeout-reset)
```

Reset the timeout that stops the motor. This has to be run on at least every second to keep the motor running. The timeout time can be configured in App Settings->General. The [Motor Set Commands](#motor-set-commands) will also reset the timeout when they are called.

#### get-ppm

```clj
(get-ppm)
```

Read the decoded value on the PPM input and returns 0.0 to 1.0. Note that the PPM app has to be configured and running. Example:

```clj
(print (list "PPM Value: " (get-ppm)))
```

Note that control type can be set to Off in the PPM app to get the input without running the motor automatically, which is useful when running the motor from lisp.

#### get-encoder

```clj
(get-encoder)
```

Get angle from selected encoder in degrees.

#### set-servo

```clj
(set-servo value)
```

Set servo output to value. Range 0 to 1. Note that the servo output has to be enabled in App Settings -> General.

#### get-vin

```clj
(get-vin)
```

Get input voltage.

#### select-motor

```clj
(select-motor motor)
```

Select which motor to control on dual-motor hardware. Options are 1 for motor 1 and 2 for motor 2.

#### get-selected-motor

```clj
(get-selected-motor)
```

Get currently selected motor on dual motor hardware.

#### get-bms-val

```clj
(get-bms-val val optValArg)
```

Get value from BMS. Examples:

```clj
(get-bms-val 'bms-v-tot) ; Total voltage
(get-bms-val 'bms-v-charge) ; Charge input voltage
(get-bms-val 'bms-i-in-ic) ; Measured current (negative means charging)
(get-bms-val 'bms-ah-cnt) ; Amp hour counter
(get-bms-val 'bms-wh-cnt) ; Watt hour counter
(get-bms-val 'bms-cell-num) ; Number of cells in series
(get-bms-val 'bms-v-cell 2) ; Cell 3 voltage (index starts from 0)
(get-bms-val 'bms-bal-state 2) ; Cell 3 balancing state. 0: not balancing, 1: balancing
(get-bms-val 'bms-temp-adc-num) ; Temperature sensor count
(get-bms-val 'bms-temps-adc 2) ; Get sensor 3 temperature (index starts from 0)
(get-bms-val 'bms-temp-ic) ; Balance IC temperature
(get-bms-val 'bms-temp-hum) ; Humidity sensor temperature
(get-bms-val 'bms-hum) ; Humidity
(get-bms-val 'bms-temp-cell-max) ; Maximum cell temperature
(get-bms-val 'bms-soc) ; State of charge (0.0 to 1.0)
(get-bms-val 'bms-can-id) ; CAN ID of BMS
(get-bms-val 'bms-ah-cnt-chg-total) ; Total ah charged
(get-bms-val 'bms-wh-cnt-chg-total) ; Total wh charged
(get-bms-val 'bms-ah-cnt-dis-total) ; Total ah discharged
(get-bms-val 'bms-wh-cnt-dis-total) ; Total wh discharged
(get-bms-val 'bms-msg-age) ; Age of last message from BMS in seconds
```

#### get-adc

```clj
(get-adc ch)
```

Get ADC voltage on channel ch (0, 1 or 2).

#### systime

```clj
(systime)
```

Get system time in ticks since boot. Every tick is 0.1 ms.

#### secs-since

```clj
(secs-since timestamp)
```

Get seconds elapsed since systime timestamp.

#### set-aux

```clj
(set-aux ch state)
```

Set AUX output ch (1 or 2) to state. Example:

```clj
(set-aux 1 1) ; Set AUX1 to ON.
```

Note: The AUX output mode must be set to Unused in Motor Settings->General->Advanced. Otherwise the firmware will change the AUX state directly after it is set using this function.

#### get-imu-rpy
```clj
(get-imu-rpy)
```
Get roll, pitch and yaw from the IMU in radians.

The function (ix list ind) can be used to get an element from the list. Example:
```clj
(ix (get-imu-rpy) 0) ; Get roll (index 0)
```

#### get-imu-quat
```clj
(get-imu-quat)
```
Get a list of quaternions from the IMU (q0, q1, q2 and q3).

#### get-imu-acc
```clj
(get-imu-acc)
```
Get a list of the x, y and z acceleration from the IMU in G.

#### get-imu-gyro
```clj
(get-imu-gyro)
```
Get a list of the x, y and z angular rate from the IMU in degrees/s.

#### get-imu-mag
```clj
(get-imu-mag)
```
Get a list of the x, y and z magnetic field strength from the IMU in uT. Note that most IMUs do not have a magnetometer.

#### get-imu-acc-derot

Same as get-imu-acc, but derotates the result first. This means that the acceleration will be relative to the horizon and not the IMU chip.

#### get-imu-gyro-derot

Same as get-imu-gyro, but derotates the result first. This means that the angular rates will be relative to the horizon and not the IMU chip.

#### send-data
```clj
(send-data dataList)
```
Send a list of custom app data to VESC Tool. This can be read from a Qml script for example.

Example of sending the numbers 1, 2, 3 and 4:

```clj
(send-data (list 1 2 3 4))
```

*dataList* can be a list or a [byte array](#byte-arrays).

#### sleep

```clj
(sleep seconds)
```

Sleep for *seconds* seconds. Example:

```clj
(sleep 0.05) ; Sleep for 0.05 seconds (50 ms)
```

### Motor Set Commands

#### set-current

```clj
(set-current current)
```

Set motor current in amperes.

#### set-current-rel

```clj
(set-current-rel current)
```

Set motor current relative to the maximum current. Range -1 to 1. For example, if the maximum current is set to 50A, (set-current-rel 0.5) will set the current to 25A.

#### set-duty
```clj
(set-duty dutycycle)
```

Set duty cycle. Range -1.0 to 1.0.

#### set-brake
```clj
(set-brake current)
```

Set braking current.

#### set-brake-rel
```clj
(set-brake-rel current)
```

Set braking current relative to the maximum current, range 0.0 to 1.0.

#### set-handbrake
```clj
(set-handbrake current)
```

Set handbrake current. This sets an open loop current that allows to hold the motor still even at 0 speed at the cost of efficient.

#### set-handbrake-rel
```clj
(set-handbrake-rel current)
```

Same as set-handbrake, but with a current relative to the maximum current in the range 0.0 to 1.0.

#### set-rpm
```clj
(set-rpm rpm)
```

Set RPM speed control.

#### set-pos
```clj
(set-pos pos)
```

Position control. Set motor position in degrees, range 0.0 to 360.0.

### Motor Get Commands

#### get-current
```clj
(get-current)
```

Get motor current. Positive means that current is flowing into the motor and negative means that current is flowing out of the motor (regenerative braking).

#### get-current-dir
```clj
(get-current-dir)
```

Get directional current. Positive for torque in the forward direction and negative for torque in the reverse direction.

#### get-current-in
```clj
(get-current-in)
```

Get input current. Will always be lower than the motor current. The closer the motor spins to full speed the closer the input current is to the motor current.

#### get-duty
```clj
(get-duty)
```

Get duty cycle. Range -1.0 to 1.0.

#### get-rpm
```clj
(get-rpm)
```

Get motor RPM. Negative values mean that the motor spins in the reverse direction.

#### get-temp-fet
```clj
(get-temp-fet)
```

Get MOSFET temperature.

#### get-temp-motor
```clj
(get-temp-motor)
```

Get motor temperature.

#### get-speed
```clj
(get-speed)
```

Get speed in meters per second. Requires that the number of motor poles, wheel diameter and gear ratio are set up correctly.

#### get-dist
```clj
(get-dist)
```

Get the distance traveled since start in meters. As with (get-speed) this requires that the number of motor poles, wheel diameter and gear ratio are set up correctly.

#### get-batt
```clj
(get-batt)
```

Get the battery level, range 0.0 to 1.0. Requires that the battery type and number of cells is set up correctly.

#### get-fault
```clj
(get-fault)
```

Get fault code.

### CAN-Commands

Notice that all canget-commands rely on the status messages being active on the VESCs on the CAN-bus. That can be done from App Settings->General->Can status message mode.

#### canset-current
```clj
(canset-current id current)
```

Set current over CAN-bus on VESC with id. Example for setting 25A on VESC with id 115:

```clj
(canset-current 115 25)
```

#### canset-current-rel
```clj
(canset-current-rel id current)
```

Same as above, but relative current in the range -1.0 to 1.0. See (set-current) for details on what relative current means.

#### canset-duty
```clj
(canset-duty id duty)
```

Set duty cycle over CAN-bus on VESC with id. Range -1.0 to 1.0.

#### canset-brake
```clj
(canset-brake id current)
```

Set braking current over CAN-bus.

#### canset-brake-rel
```clj
(canset-brake-rel id current)
```

Set relative braking current over CAN-bus. Range 0.0 to 1.0.

#### canset-rpm
```clj
(canset-rpm id rpm)
```

Set rpm over CAN-bus.

#### canset-pos
```clj
(canset-pos id pos)
```

Set position control in degrees over CAN-bus. Range 0.0 to 1.0.

#### canget-current
```clj
(canget-current id)
```

Get current over CAN-bus on VESC with id.

#### canget-current-dir
```clj
(canget-current-dir id)
```

Get directional current over CAN-bus on VESC with id. See (get-current-dir) for what directional means.

#### canget-current-in
```clj
(canget-current-in id)
```

Get input current over CAN-bus on VESC with id.

#### canget-duty
```clj
(canget-duty id)
```

Get duty cycle over CAN-bus on VESC with id.

#### canget-rpm
```clj
(canget-rpm id)
```

Get RPM over CAN-bus on VESC with id.

#### canget-temp-fet
```clj
(canget-temp-fet id)
```

Get MOSFET temperature over CAN-bus on VESC with id.

#### canget-temp-motor
```clj
(canget-temp-motor id)
```

Get motor temperature over CAN-bus on VESC with id.

#### canget-speed
```clj
(canget-speed id)
```

Get speed in meters per second over CAN-bus on VESC with id. The gearing, wheel diameter and number of motor poles from the local configuration will be used for converting the RPM to meters per second.

#### canget-dist
```clj
(canget-dist id)
```

Get distance traveled in meters over CAN-bus on VESC with id. As with (canget-speed id), the local configuration will be used to convert the tachometer value to meters.

#### can-list-devs
```clj
(can-list-devs)
```

List CAN-devices that have been heard on the CAN-bus since boot. This function is fast as it does not actively scan the CAN-bus, but it relies on the devices sending status message 1.

#### can-scan
```clj
(can-scan)
```

Actively scan the CAN-bus and return a list with devices that responded. This function takes several seconds to run, but also finds devices that do not actively send messages and only respond to a ping message.

#### can-send-sid
```clj
(can-send-sid id data)
```

Send standard ID CAN-frame with id and data. Data is a list with bytes, and the length of the list (max 8) decides how many data bytes are sent. Example:

```clj
(can-send-sid 0x11FF11 (list 0xAA 0x11 0x15))
```

*data* can be a list or a [byte array](#byte-arrays).

#### can-send-eid
```clj
(can-send-eid id data)
```

Same as (can-send-sid), but sends extended ID frame.

### Math Functions

#### sin
```clj
(sin angle)
```

Get the sine of angle. Unit: Radians.

#### cos
```clj
(cos angle)
```

Get the cosine of angle. Unit: Radians.

#### tan
```clj
(tan angle)
```

Get the tangent of angle. Unit: Radians.

#### asin
```clj
(asin x)
```

Get the arc sine of x. Unit: Radians.

#### acos
```clj
(acos x)
```

Get the arc cosine of x. Unit: Radians.

#### atan
```clj
(atan x)
```

Get the arc tangent of x. Unit: Radians.

#### atan2
```clj
(atan2 y x)
```

Get the arc tangent of y / x. Unit: Radians. This version uses the signs of y and x to determine the quadrant.

#### pow
```clj
(pow base power)
```

Get base raised to power.

#### sqrt
```clj
(sqrt x)
```

Get the square root of x.

#### log
```clj
(log x)
```

Get the base-e logarithm of x.

#### log10
```clj
(log10 x)
```

Get the base-10 logarithm of x.

#### deg2rad
```clj
(deg2rad x)
```

Converts x from degrees to radians.

#### rad2deg
```clj
(rad2deg x)
```

Converts x from radians to degrees.

#### vec3-rot
```clj
(vec3-rot x1 x2 x3 roll pitch yaw optRev)
```

Rotate vector x1,x2,x3 around roll, pitch and yaw. optRev (1 or 0) will apply the rotation in reverse (apply the inverse of the rotation matrix) if set to 1.

### Bit Operations

#### bits-enc-int
```clj
(bits-enc-int initial number offset bits)
```

Put bits of number in initial at offset and return the result. For example, if the bits initial are aaaaaaaa, number is bbb, offset is 2 and bits is 3 the result is aaabbbaa. For reference, the corresponding operation in C is:

```c
initial &= ~((0xFFFFFFFF >> (32 - bits)) << offset);
initial |= (number << (32 - bits)) >> (32 - bits - offset);
```

#### bits-dec-int
```clj
(bits-dec-int value offset size)
```

Return size bits of value at offset. For example if the bits of value are abcdefgh, offset is 3 and size it 3 a number with the bits cde is returned. The corresponding operation in C is:

```c
val >>= offset;
val &= 0xFFFFFFFF >> (32 - bits);
```

### Raw Commands

Raw data commands useful for debugging hardware issues.

#### raw-adc-current
```clj
(raw-adc-current motor phase useRaw)
```

Get raw current measurements. Motor is the motor index (1 or 2), phase is the phase (1, 2 or 3) and useRaw is whether to convert the measurements to currents or to use raw ADC values.

Example for reading phase B on motor 1 as raw ADC values:

```clj
(raw-adc-current 1 2 1)
```

#### raw-adc-voltage
```clj
(raw-adc-voltage motor phase useRaw)
```

Same as (raw-adc-current), but measures phase voltages instead.

#### raw-mod-alpha
```clj
(raw-mod-alpha)
```

Get alpha modulation. Range -1.0 to 1.0 (almost).

#### raw-mod-beta
```clj
(raw-mod-beta)
```

Get beta modulation. Range -1.0 to 1.0 (almost).

#### raw-mod-alpha-measured
```clj
(raw-mod-alpha-measured)
```

Same as (raw-mod-alpha), but derives the modulation from the phase voltage reading and/or dead-time compensation.

#### raw-mod-beta-measured
```clj
(raw-mod-beta-measured)
```
Same as (raw-mod-beta), but derives the modulation from the phase voltage reading and/or dead-time compensation.

#### raw-hall
```clj
(raw-hall motor optSamples)
```
Read hall sensors for motor (1 or 2) and return their states in a list. The optional argument optSamples (max 20) can be used to set how many times the hall sensors are sampled; if it is not supplied the number of samples from the motor configuration will be used.

The function (ix list ind) can be used to get an element from the list. Example:
```clj
(ix (raw-hall 1) 0) ; Get hall sensor 1 state (index 0)
```

### UART

#### uart-start

```clj
(uart-start baudrate)
```

Start the UART driver at baudrate on the COMM-port on the VESC. If any app is using the UART pins it will be stopped first. Example:

```clj
(uart-start 115200)
```

#### uart-write

```clj
(uart-write array)
```

Write array (see [byte array](#byte-arrays) for details) to the UART. Examples:

```clj
(uart-write "Hello World!") ; Write the string hello world!
```

```clj
(define arr (array-create 6)) ; Create a 6 byte long array
(bufset-i16 arr 0 1123) ; Set byte 0 and 1 to 1123
(bufset-i32 arr 2 424242) ; Set byte 2 to 5 to 424242 
(uart-write arr) ; Write arr to the uart
```

#### uart-read

```clj
(uart-read array num optOffset optStopAt)
```

Read num bytes into array at offset optOffset. Stop reading if the character optStopAt is received. The last two arguments are optional. Note that this function returns immediately if there is nothing to be read, so it is not blocking. The return value is the number of bytes read.

#### uart-read-bytes

```clj
(uart-read-bytes array num offset)
```

Read num bytes into buffer at offset. This function is blocking, so it will not return until the specified amount of bytes is read.

#### uart-read-until

```clj
(uart-read-until array num offset end)
```

Same as uart-read-bytes, but will return when the byte end is read.

### I2C

#### i2c-start

```clj
(i2c-start)
```

Start the I2C driver on the COMM-port on the VESC. If any app is using the I2C pins it will be stopped first.

#### i2c-tx-rx

```clj
(i2c-tx-rx addr arrTx optArrRx)
```

Send array (or list) arrTx to the I2C-device with address addr. Optionally receive a response to opArrRx. Example:

```clj
; Create 14 byte long array
(define arr (array-create 14))

; Send 0x3B to device 0x68 and receive 14 bytes into arr
(i2c-tx-rx 0x68 (list 0x3B) arr)
```

#### i2c-restore

```clj
(i2c-restore)
```

Sends a sequence of bits in an attempt to restore the i2c-bus. Can be used if an i2c-device hangs and refuses to respond.

### Useful Lisp Functions

There are a number of lisp functions that can be used from lispBM in the VESC firmware. They will be loaded to the environment the first time they are used, so they do not use up memory before the first use.

#### defun

```clj
(defun (args) body)
```

Shorthand macro for defining a function. Example:

```clj
; Create function f with argument x that prints x
(defun f (x)
    (print x)
)

; The above is equivalent to
(define f (lambda (x)
    (print x)
))
```

#### map

```clj
(map f lst)
```

Apply function f to every element in list lst. Example:

```clj
(map (lambda (x) (* x 5)) '(1 2 3 4))
> (5 10 15 20)
```

This example creates an anonymous function that takes one argument and returns that argument multiplied by 5. Map then applies it to every element in the list (1 2 3 4), which yields the list (5 10 15 20).

#### iota

```clj
(iota n)
```

Create list from 0 to n. Example:

```clj
(iota 5)
> (0 1 2 3 4 5)
```

#### range

```clj
(range start end)
```

Create a list from start to end. Example:

```clj
(range 2 8)
> (2 3 4 5 6 7 8)
```

#### foldl

```clj
(foldl f init lst)
```

Apply the function f to pairs of init and each element of the list lst and accumulate the result. Example:

```clj
(foldl + 0 '(1 2 3 4 5))
> 15
```

#### foldr

Same as foldl, but start from the right side of lst.

#### reverse

```clj
(reverse lst)
```

Returns the list lst in reverse. Example:

```clj
(reverse '(1 2 3 4 5))
> (5 4 3 2 1)
```

#### length

```clj
(length lst)
```

Returns the length of list lst. Example:

```clj
(length '(1 2 3))
> 3
```

#### apply

```clj
(apply f lst)
```

Use the elements in list lst as arguments to function f. Example:

```clj
(apply + '(1 2 3))
> 6
```

#### zipwith

```clj
(zipwith f x y)
```

Apply the function f to pairs between the elements in list x and list y. Example:

```clj
(zipwith * '(1 2 3) '(3 4 5))
> (3 8 15)
```

#### filter

```clj
(filter f lst)
```

Filter list by keeping the elements on which f returns true. Example:

```clj
(filter (lambda (x) (< x 5)) '(3 9 5 8 2 4 7))
> (3 2 4)
```

#### sort

```clj
(sort f lst)
```

Sort list lst using comparison function f. Example:

```clj
(sort < '(5 6 2 1 5 63 33 7 7 8))
> (1 2 5 5 6 7 7 8 33 63)

(sort > '(5 6 2 1 5 63 33 7 7 8))
> (63 33 8 7 7 6 5 5 2 1)

; Split sentence to words and sort them in ascending order
(sort str-cmp-asc (str-split "this is a string" " "))
> ("a" "is" "string" "this")
```

### String Manipulation

#### str-from-n

```clj
(str-from-n n optFormat)
```

Create a string from the number n. Also takes an optional format argument optFormat that works in the same way as the printf-function in C. The optFormat argument can also be used together with other characters as long as the resulting output string is shorter than 100 characters. Example:

```clj
(str-from-n 10)
> "10"

(str-from-n 2.5)
> "2.500000"

(str-from-n 2.5 "%.1f")
> "2.5"

(str-from-n 10 "0x%04X") ; Here we also append 0x in front of optFormat
> "0x000A"

(str-from-n 0.023e3)
> "2.500000"
```

#### str-merge

```clj
(str-merge str1 str2 ...)
```

Merge a number of strings into one. Example:

```clj
(str-merge "A" "bC" "D")
> "AbCD"

(str-merge "Num1: " (str-from-n 10) " Num2: " (str-from-n 2.1 "%.1f"))
> "Num1: 10 Num2: 2.1"
```

#### str-to-i

```clj
(str-to-i str optBase)
```

Convert string to integer. By default the base is chosen automatically, but it can also be specified. Example:

```clj
(str-to-i "123")
> {123}

(str-to-i "a" 16)
> {10}

(str-to-i "0xa") ; Automatic base16 if str starts with 0x
> {10}
```

#### str-to-f

```clj
(str-to-f str)
```

Convert string to floating point number. Example:

```clj
(str-to-f "2.5")
> {2.500000}

; Also supports scientific notation
(str-to-f "0.0025e3")
> {2.500000}
```

#### str-part

```clj
(str-part str start optN)
```

Take part of string str starting at start for optN characters. If optN is omitted the rest of str will be taken. Example:

```clj
(str-part "Hello World!" 6)
> "World!"

(str-part "Hello World!" 6 2)
> "Wo"

(str-part "Hello World!" 0 2)
> "He"
```

#### str-split

```clj
(str-split str delim)
```

Split string str into tokens using delimiter delim. If delim is a number str will be split into tokens the size of that number. Example:

```clj
(str-split "This is a test" " ")
> ("This" "is" "a" "test")

(str-split "this_o_is_o_a_o_test" "_o_")
> ("This" "is" "a" "test")

(str-split "This is a test" 3)
> ("Thi" "s i" "s a" " te" "st")

(str-split "This is a test" 1)
> ("T" "h" "i" "s" " " "i" "s" " " "a" " " "t" "e" "s" "t")
```

#### str-replace

```clj
(str-replace str rep optWith)
```

Replace every occurrence of rep in str with opnWith. If optWith is omitted every rep will be removed. Example:

```clj
(str-replace "Hello World!" "World" "LispBM")
> "Hello LispBM!"

(str-replace "Hello World!" " World")
> "Hello!"
```

#### str-to-upper

```clj
(str-to-upper str)
```

Convert string str to upper case. Example:

```clj
(str-to-upper "TesTt")
> "TESTT"
```

#### str-to-lower

```clj
(str-to-lower str)
```

Convert string str to lower case. Example:

```clj
(str-to-lower "TesTt")
> "testt"
```

#### str-cmp

```clj
(str-cmp str1 str1)
```

Compare strings str1 and str2. Works in the same way as the strcmp-function in C, meaning that equal strings return 0 and different strings return their difference according how they would be sorted. Example:

```clj
(str-cmp "Hello" "Hello")
> 0

(str-cmp "Hello" "World")
> -15

(str-cmp "World" "Hello")
> 15
```

#### str-cmp-asc

```clj
(str-cmp-asc str1 str1)
```

Return true if str1 comes before str2, nil otherwise. Useful for sorting strings using the [sort](#sort) function in ascending order.

#### str-cmp-dsc

```clj
(str-cmp-dsc str1 str1)
```

Return true if str2 comes before str1, nil otherwise. Useful for sorting strings using the [sort](#sort) function in descending order.

#### str-len

```clj
(str-len str)
```

Calculate length of string str excluding the null termination. Example:

```clj
(str-len "Hello")
> 5
```

## Events

Events can be used to execute code for certain events, such as when CAN-frames are received. To use events you must first register an event handler, then enable the events you want to receive. As the event handler blocks until the event arrives it is useful to spawn a thread to handle events so that other things can be done in the main thread at the same time.

The following example shows how to spawn a thread that handles SID (standard-id) CAN-frames and custom app data:

```clj
(define proc-sid (lambda (id data)
    (print (list id data)) ; Print the ID and data
))

(define proc-data (lambda (data)
    (progn
        (print data)
)))

(define event-handler (lambda ()
    (progn
        (recv ((event-can-sid (? id) . (? data)) (proc-sid id data))
        (recv ((event-data-rx ? data) (proc-data data))
              (_ nil)) ; Ignore other events
        (event-handler) ; Call self again to make this a loop
)))

; Spawn the event handler thread and pass the ID it returns to C
(event-register-handler (spawn event-handler))

; Enable the CAN event for standard ID (SID) frames
(event-enable 'event-can-sid)

; Enable the custom app data event
(event-enable 'event-data-rx)
```

Possible events to register are

```clj
(event-enable 'event-can-sid) ; Sends (signal-can-sid id data), where id is U32 and data is a byte array
(event-enable 'event-can-eid) ; Sends (signal-can-eid id data), where id is U32 and data is a byte array
(event-enable 'event-data-rx) ; Sends (signal-data-rx data), where data is a byte array
```

The CAN-frames arrive whenever data is received on the CAN-bus and data-rx is received for example when data is sent from a Qml-script in VESC Tool.

## Byte Arrays

Byte arrays (and text strings) are allocated in memory as consecutive arrays of bytes (not linked lists). They can be shared with C and are more space and performance efficient than linked lists. Several of the extensions also take byte arrays as input as an alternative to lists and some of the events return byte arrays.

To allocate a byte array with 20 bytes and bind the symbol arr to it you can use

```clj
(define arr (array-create 20))
```

The length of a byte array can be read with

```clj
(buflen arr)
```

Which will return 20 for the array arr above.

To read data from the byte array you can use

```clj
(bufget-[x] arr index)
```

Where \[x\] is i8, u8, i16, u16, i32, u32 or f32. Index is the position in the array to start reading from, starting at 0. Here are some examples

```clj
(bufget-i8 arr 0) ; read byte 0 as int8
(bufget-i16 arr 0) ; read byte 0 and 1 as int16
(bufget-i32 arr 0) ; read byte 0 to 3 as i32
(bufget-u8 arr 0) ; read byte 0 as uint8
(bufget-u16 arr 0) ; read byte 0 and 1 as uint16
(bufget-u32 arr 0) ; read byte 0 to 3 as uint32
(bufget-f32 arr 0) ; read byte 0 to 3 as float32 (IEEE 754)
```

By default the byte order is big endian. The byte order can also be specified as an extra argument. E.g. to read 4 bytes as int32 from position 6 in little endian you can use

```clj
(bufget-i32 arr 6 little-endian)
```

Writing to the array can be done in a similar way

```clj
(bufset-[x] arr index value)
```

Here are some examples

```clj
(bufset-i8 arr 0 12) ; write 12 to byte 0 as int8
(bufset-i16 arr 0 -5621) ; write -5621 to byte 0 and 1 as int16
(bufset-i32 arr 0 2441) ; write 2441 to byte 0 to 3 as i32
(bufset-u8 arr 0 12) ; write 12 to byte 0 as uint8
(bufset-u16 arr 0 420) ; write 420 to byte 0 and 1 as uint16
(bufset-u32 arr 0 119) ; write 119 to byte 0 to 3 as uint32
(bufset-f32 arr 0 3.14) ; write 3.14 to byte 0 to 3 as float32 (IEEE 754)
```

As with bufget big endian is the default byte order and little-endian can be passed as the last argument to use little-endian byte order instead.

**Note**  
Byte arrays will be de-allocated by the garbage collector on a regular basis, but can still use a lot of memory until then and large byte arrays cause a risk of running out of memory. It is possible to manually de-allocate the byte arrays when done with them by calling free

```clj
(free arr)
```

This will clear the allocated memory for arr.

**Note**  
Strings in lispBM are treated the same as byte arrays, so all of the above can be done to the characters in strings too.

## How to update

To update from remote repository:

```bash
git remote add lispBM git@github.com:svenssonjoel/lispBM.git
git subtree pull --squash --prefix=lispBM/lispBM/ lispBM master
```

The first command might fail if it already is added, but the second one should still work. If there are uncomitted changes you can run **git stash** before the commands and **git stash pop** after them.