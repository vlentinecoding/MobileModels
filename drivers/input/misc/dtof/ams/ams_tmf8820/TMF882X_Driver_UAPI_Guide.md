TMF882X Driver User API Guide
=============================

Version 1.3

Contents
========

1. [Introduction](#introduction)
2. [ToF Char Device](#tof-char-device)
3. [ToF Input Device](#tof-input-device)
4. [SysFS Attributes](#sysfs-attributes)
5. [Factory Calibration](#factory-calibration)
6. [Custom SPAD Configuration](#custom-spad-configuration)


Introduction
============

This document describes User Application Programming Interface (API) to the
TMF882X Linux Kernel Driver.

ToF Char Device
===============

The TMF882X driver registers a character device with the Linux system called
**_/dev/tof_**. The ToF char device is used for streaming messages from the
driver. After module probe, the TMF882X device is in a low power state with
the **_APPLICATION_** mode loaded. Opening the ToF char device will bring the
device out of standby to idle state.


Common types of messages include:

- Histogram data
- Measurement Result data
- Driver error codes

All messages have a common header format with an identifier and message
length. The driver buffers messages in an internal FIFO that user space
applications may read. When reading from the ToF char device the user buffer
should be large enough for at least one message, the driver will fill the
buffer with as many messages that will fit completely. The driver will never
put partial messages in the user buffer.

> **Note** 1: If the user buffer is smaller than the next message in the FIFO
>             the driver will return an error.

> **Note** 2: If the driver FIFO is empty by default the driver will block
>             until a message is available

> **Note** 3: Since the ToF char device is a message stream, _seek()_ operations
>             and file _offset_ are not supported

> **Note** 4: Since the ToF char device is a message stream, the ToF char
>             device will never return EOF

Example 'C' code reading from ToF Char device:

```
    char buf[TMF882X_MAX_MSG_SIZE];     /* buffer to hold at least 1 message */
    int off = 0;
    int ret = 0;
    while (1) {

        /* read as many messages as our buffer size allows */
        ret = read(fd, buf, sizeof(buf)));
        off = 0;

        /* loop through the messages */
        while (ret > 0 && off < ret) {
            struct tmf882x_msg *msg = buf + off;

            switch (msg->hdr.msg_id) {
                case ID_ERROR:      /* Error code message received */
                   struct tmf882x_msg_error *err = &msg->err_msg;
                   *** Handle msg 'err' ***
                   break;
                case .
                     .
                     .
            }

            off += msg->hdr.msg_len;     /* Move to next message in buffer */
        }

    }
```

Refer to **_./include/linux/i2c/ams/tmf882x.h_** for a detailed description of
message definitions.


ToF Input Device
================

The TMF882X registers an input character device with the Linux Input system.
After module probe, the TMF882X device is in a low power state with
the **_APPLICATION_** mode loaded. Opening the ToF input device will bring the
device out of standby and start measurements with the default configuration.

Below is the Linux Kernel definition of an input event:

```
struct input_event {
    struct timeval time;
    __u16 type;
    __u16 code;
    __s32 value;
};
```

Only measurement results are published on the input device. The TMF882X driver
registers an input device with the following encoding:

```
type = EV_ABS;
code = channel;
val(31:24) = sub capture index
val(23:16) = confidence;
val(15:0) = distance in millimeters;
```


SysFS Attributes
================

General configuration of the TMF882X kernel driver is done through Sysfs attributes.

List of SysFS Attributes
------------------------

Sysfs attributes marked as mode == 'N/A' are valid to use in any mode.

|   Mode    |   SysFS Attribute                                   |   Read / Write    |   Format  |
|:---------:|-----------------------------------------------------|:-----------------:|:---------:|
|   N/A     |[mode](#mode)                                        |       R/W         |  string   |
|   N/A     |[chip_enable](#chip_enable)                          |       R/W         |  string   |
|   N/A     |[driver_debug](#driver_debug)                        |       R/W         |  string   |
|   N/A     |[firmware_version](#firmware_version)                |       R           |  string   |
|   N/A     |[registers](#registers)                              |       R           |  string   |
|   N/A     |[register_write](#register_write)                    |       W           |  string   |
|   N/A     |[request_ram_patch](#request_ram_patch)              |       W           |  string   |
|   N/A     |[device_uid](#device_uid)                            |       R           |  string   |
|   N/A     |[device_revision](#device_revision)                  |       R           |  string   |
|   0x2     |[app/capture](#appcapture)                           |       R/W         |  string   |
|   0x2     |[app/report_period_ms](#appreport_period_ms)         |       R/W         |  string   |
|   0x2     |[app/iterations](#appiterations)                     |       R/W         |  string   |
|   0x2     |[app/alg_setting](#appalg_setting)                   |       R/W         |  string   |
|   0x2     |[app/power_cfg](#apppower_cfg)                       |       R/W         |  string   |
|   0x2     |[app/gpio_0](#appgpio_0)                             |       R/W         |  string   |
|   0x2     |[app/gpio_1](#appgpio_1)                             |       R/W         |  string   |
|   0x2     |[app/histogram_dump](#apphistogram_dump)             |       R/W         |  string   |
|   0x2     |[app/set_default_cfg](#appset_default_cfg)           |       W           |  string   |
|   0x2     |[app/conf_threshold](#appconf_threshold)             |       R/W         |  string   |
|   0x2     |[app/low_threshold](#applow_threshold)               |       R/W         |  string   |
|   0x2     |[app/high_threshold](#apphigh_threshold)             |       R/W         |  string   |
|   0x2     |[app/persistence](#apppersistence)                   |       R/W         |  string   |
|   0x2     |[app/zone_mask](#appzone_mask)                       |       R/W         |  string   |
|   0x2     |[app/spad_map_id](#appspad_map_id)                   |       R/W         |  string   |
|   0x2     |[app/xoff_q1_0](#appxoff_q1_0)                       |       R/W         |  string   |
|   0x2     |[app/xoff_q1_1](#appxoff_q1_1)                       |       R/W         |  string   |
|   0x2     |[app/yoff_q1_0](#appyoff_q1_0)                       |       R/W         |  string   |
|   0x2     |[app/yoff_q1_1](#appyoff_q1_1)                       |       R/W         |  string   |
|   0x2     |[app/xsize_0](#appxsize_0)                           |       R/W         |  string   |
|   0x2     |[app/xsize_1](#appxsize_1)                           |       R/W         |  string   |
|   0x2     |[app/ysize_0](#appysize_0)                           |       R/W         |  string   |
|   0x2     |[app/ysize_1](#appysize_1)                           |       R/W         |  string   |
|   0x2     |[app/spad_mask_0](#appspad_mask_0)                   |       R/W         |  string   |
|   0x2     |[app/spad_mask_1](#appspad_mask_1)                   |       R/W         |  string   |
|   0x2     |[app/spad_map_0](#appspad_map_0)                     |       R/W         |  string   |
|   0x2     |[app/spad_map_1](#appspad_map_1)                     |       R/W         |  string   |
|   0x2     |[app/commit_spad_cfg](#appcommit_spad_cfg)           |       R/W         |  string   |
|   0x2     |[app/clock_compensation](#appclock_compensation)     |       R/W         |  string   |
|   0x2     |[app/factory_calibration](#appfactory_calibration)   |       R           |  bin      |
|   0x2     |[app/calibration_data](#appcalibration_data)         |       R/W         |  bin      |

SysFS Attribute Details
-----------------------

###mode

Read or Write the current mode of the TMF882X device.

| Value   | Description |
|---------|-------------|
| 0x80    | Bootloader  |
| 0x02    | Application |

###chip_enable

Read or Write the CE GPIO line to control power to the TMF882X device.

| Value | Description |
|-------|-------------|
| 0     | Unpowered   |
| 1     | Powered     |

###driver_debug

Read or Write the driver verbosity log level

| Value | Description      |
|-------|------------------|
| 0     | No debug logging |
| 1     | Debug logging    |

###firmware_version

Dump the current mode's firmware version string.

###registers

Dump the I2C register map of the TMF882X device.

###register_write

Write a value to a register.

| Value          | Description                                      |
|----------------|--------------------------------------------------|
| _aa_:_bb_      | Write value _bb_ to register _aa_                |
| _aa_:_bb_:_cc_ | Write value _bb_ to register _aa_ with mask _cc_ |

###request_ram_patch

Write any value to request a RAM patch firmware download. The TMF882X Linux
kernel driver uses the Linux Firmare Framework to request the firmware
**/lib/firmware/tmf882x_firmware.bin**. The driver assumes the _APPLICATION_
mode to be running after a successful FWDL.

###device_uid

Dump the current TMF882X device Unique ID or serial number string.

###device_revision

Dump the current TMF882X device revision string.

###app/capture

Read or Write the Time-of-Flight distance measurement state.

| Value | Description        |
|-------|--------------------|
| 0     | Stop measurements  |
| 1     | Start measurements |

###app/report_period_ms

Read or Write measurement reporting period in milliseconds. Configuring a
report period shorter than the time necessary to complete the measurements
based on the current configuration results in a reporting period at the
fastest possible rate.

| Value | Description                                   |
|-------|-----------------------------------------------|
| _aa_  | The reporting period in _aa_ milliseconds     |

###app/iterations

Read or Write the measurement iterations of the TMF882X device.

> **Note**: Iterations may not match exactly what was set by the user due to
>       the TMF882X device only supporting measurement iterations in units
>       of _iterations_ * 1024

| Value | Description                        |
|-------|------------------------------------|
| _aa_  | The measurement iterations in _aa_ |

###app/alg_setting

Read or Write the current ToF algorithm measurement settings. Please refer
to the TMF882X datasheet for details on the **alg_setting** register.

| Value | Description                           |
|-------|---------------------------------------|
| _aa_  | _aa_ is the  **alg_setting** register setting |

###app/power_cfg

Read or Write the current ToF power configuration settings. Please refer
to the TMF882X datasheet for details on the **power_cfg** register.

| Value | Description                           |
|-------|---------------------------------------|
| _aa_  | _aa_ is the  **power_cfg** register setting |

###app/gpio_0

Read or Write the current GPIO_0 setting. Please refer
to the TMF882X datasheet for details on the **gpio_0** register.

| Value | Description                           |
|-------|---------------------------------------|
| _aa_  | _aa_ is the  **gpio_0** register setting      |

###app/gpio_1

Read or Write the current GPIO_1 setting. Please refer
to the TMF882X datasheet for details on the **gpio_1** register.

| Value | Description                           |
|-------|---------------------------------------|
| _aa_  | _aa_ is the  **gpio_1** register setting      |

###app/histogram_dump

Read or Write the current histogram dump setting. Please refer
to the TMF882X datasheet for details on the **histogram_dump** register.

| Value | Description                                |
|-------|--------------------------------------------|
| _aa_  | _aa_ is the  **histogram_dump** register setting   |

###app/set_default_cfg

Write any value to have the driver set its default measurement configuration,
this is the default state after driver probe.

###app/conf_threshold

Read or Write the current confidence threshold setting. Please refer
to the TMF882X datasheet for details on the **confidence_threshold** register.

| Value | Description                                     |
|-------|-------------------------------------------------|
| _aa_  | _aa_ is the  **confidence_threshold** register setting  |

###app/low_threshold

Read or Write the current low distance threshold setting. Please refer
to the TMF882X datasheet for details on the **low_threshold** register.

| Value | Description                                |
|-------|--------------------------------------------|
| _aa_  | _aa_ is the  **low_threshold** register setting    |

###app/high_threshold

Read or Write the current high distance threshold setting. Please refer
to the TMF882X datasheet for details on the **high_threshold** register.

| Value | Description                                |
|-------|--------------------------------------------|
| _aa_  | _aa_ is the  **high_threshold** register setting   |

###app/persistence

Read or Write the current persistence setting. Please refer
to the TMF882X datasheet for details on the **persistence** register.

| Value | Description                                |
|-------|--------------------------------------------|
| _aa_  | _aa_ is the  **persistence** register setting      |

###app/zone_mask

Read or Write the current zone mask setting. Please refer
to the TMF882X datasheet for details on the **zone_mask** register.

| Value | Description                                |
|-------|--------------------------------------------|
| _aa_  | _aa_ is the  **zone_mask** register setting        |

###app/spad_map_id

Read or Write the current SPAD map ID setting. Please refer
to the TMF882X datasheet for details on the **spad_map_id** register.

> **Note**: Calibration data must be updated when changing the SPAD configuration

| Value | Description                                |
|-------|--------------------------------------------|
| _aa_  | _aa_ is the  **spad_map_id** register setting      |

###app/xoff_q1_0

Read or Write the X-direction offset of the SPAD map in fixed point Q1 format
for sub capture 0 of time-multiplexed measurements.

> **Note** 1: Refer to [Custom SPAD Configuration](#custom-spad-configuration)

| Value | Description                                      |
|-------|--------------------------------------------------|
| _aa_  | _aa_ is the X-direction offset in Q1 fixed point |

###app/xoff_q1_1

Read or Write the X-direction offset of the SPAD map in fixed point Q1 format
for sub capture 1 of time-multiplexed measurements.

> **Note** 1: Refer to [Custom SPAD Configuration](#custom-spad-configuration)

| Value | Description                                      |
|-------|--------------------------------------------------|
| _aa_  | _aa_ is the X-direction offset in Q1 fixed point |

###app/yoff_q1_0

Read or Write the Y-direction offset of the SPAD map in fixed point Q1 format
for sub capture 0 of time-multiplexed measurements.

> **Note** 1: Refer to [Custom SPAD Configuration](#custom-spad-configuration)

| Value | Description                                      |
|-------|--------------------------------------------------|
| _aa_  | _aa_ is the Y-direction offset in Q1 fixed point |

###app/yoff_q1_1

Read or Write the Y-direction offset of the SPAD map in fixed point Q1 format
for sub capture 1 of time-multiplexed measurements.

> **Note** 1: Refer to [Custom SPAD Configuration](#custom-spad-configuration)

| Value | Description                                      |
|-------|--------------------------------------------------|
| _aa_  | _aa_ is the Y-direction offset in Q1 fixed point |

###app/xsize_0

Read or Write the number of columns for the SPAD configuration
for sub capture 0 of time-multiplexed measurements.

> **Note** 1: Refer to [Custom SPAD Configuration](#custom-spad-configuration)

| Value | Description                                      |
|-------|--------------------------------------------------|
| _aa_  | _aa_ is the X-direction size in number of SPADs  |

###app/xsize_1

Read or Write the number of columns for the SPAD configuration
for sub capture 1 of time-multiplexed measurements.

> **Note** 1: Refer to [Custom SPAD Configuration](#custom-spad-configuration)

| Value | Description                                      |
|-------|--------------------------------------------------|
| _aa_  | _aa_ is the X-direction size in number of SPADs  |

###app/ysize_0

Read or Write the number of rows for the SPAD configuration
for sub capture 0 of time-multiplexed measurements.

> **Note** 1: Refer to [Custom SPAD Configuration](#custom-spad-configuration)

| Value | Description                                      |
|-------|--------------------------------------------------|
| _aa_  | _aa_ is the Y-direction size in number of SPADs  |

###app/ysize_1

Read or Write the number of rows for the SPAD configuration
for sub capture 1 of time-multiplexed measurements.

> **Note** 1: Refer to [Custom SPAD Configuration](#custom-spad-configuration)

| Value | Description                                      |
|-------|--------------------------------------------------|
| _aa_  | _aa_ is the Y-direction size in number of SPADs  |

###app/spad_mask_0

Read or Write the SPAD enable/disable mask for sub capture 0 of
time-multiplexed measurements.

> **Note** 1: Refer to [Custom SPAD Configuration](#custom-spad-configuration)

> **Note** 2: Values are white-space separated list of zero and non-zero values.
>       Total number of values when reading/writing should equal
>       **[app/xsize_0](#app/xsize_0) * [app/ysize_0](#app/ysize_0)**

| Value    | Description  |
|----------|--------------|
| 0        | Disable SPAD |
| Non-Zero | Enable SPAD  |

###app/spad_mask_1

Read or Write the SPAD enable/disable mask for sub capture 1 of
time-multiplexed measurements.

> **Note** 1: Refer to [Custom SPAD Configuration](#custom-spad-configuration)

> **Note** 2: Values are white-space separated list of zero and non-zero values.
>       Total number of values when reading/writing should equal
>       **[app/xsize_1](#appxsize_1) * [app/ysize_1](#appysize_1)**

| Value    | Description  |
|----------|--------------|
| 0        | Disable SPAD |
| Non-Zero | Enable SPAD  |

###app/spad_map_0

Read or Write the SPAD channel mapping for sub capture 0 of
time-multiplexed measurements.

> **Note** 1: Refer to [Custom SPAD Configuration](#custom-spad-configuration)

> **Note** 2: Values are white-space separated list of values.
>         Total number of values when reading/writing should equal
>         **[app/xsize_0](#appxsize_0) * [app/ysize_0](#appysize_0)**

> **Note** 3: Channel 0 should not be used

> **Note** 4: Channels 1 and 8/9 cannot be mapped on the same row.

| Value | Description              |
|-------|--------------------------|
| _aa_  | Map SPAD to channel _aa_ |

###app/spad_map_1

Read or Write the SPAD channel mapping for sub capture 1 of
time-multiplexed measurements.

> **Note** 1: Refer to [Custom SPAD Configuration](#custom-spad-configuration)

> **Note** 2: Values are white-space separated list of values.
>         Total number of values when reading/writing should equal
>         **[app/xsize_1](#appxsize_1) * [app/ysize_1](#appysize_1)**

> **Note** 3: Channel 0 should not be used

> **Note** 4: Channels 1 and 8/9 cannot be mapped on the same row.

| Value | Description              |
|-------|--------------------------|
| _aa_  | Map SPAD to channel _aa_ |

###app/commit_spad_cfg

Read or Write whether the SPAD configuration SysFS attributes are committed
to the device configuration as a single bulk configuration:

> **Note** 1: Refer to [Custom SPAD Configuration](#custom-spad-configuration)

- [app/xoff_q1_0](#appxoff_q1_0)
- [app/xoff_q1_1](#appxoff_q1_1)
- [app/yoff_q1_0](#appyoff_q1_0)
- [app/yoff_q1_1](#appyoff_q1_1)
- [app/xsize_0](#appxsize_0)
- [app/xsize_1](#appxsize_1)
- [app/ysize_0](#appysize_0)
- [app/ysize_1](#appysize_1)
- [app/spad_mask_0](#appspad_mask_0)
- [app/spad_mask_1](#appspad_mask_1)
- [app/spad_map_0](#appspad_map_0)
- [app/spad_map_1](#appspad_map_1)

| Value | Description                                     |
|-------|-------------------------------------------------|
| 0     | SPAD configuration contains uncommitted changes |
| 1     | SPAD configuration is committed to the device   |

###app/clock_compensation

Read or Write whether the driver performs clock skew compensation.

| Value    | Description                                     |
|----------|-------------------------------------------------|
| 0        | Clock skew compensation is disabled             |
| non-zero | Clock skew compensation is enabled              |

###app/factory_calibration

Perform a factory calibration and dump the factory calibration binary data.
Please refer to the TMF882X datasheet for details on **factory calibration**
configuration.

###app/calibration_data

Read or Write the calibration binary data. Please refer to the TMF882X
datasheet for details on **factory calibration** configuration.

> **Note**: The TMF882X device continuously updates calibration data during
>       runtime. For best performance, client applications should periodically
>       save the calibration data and write it back to the TMF882X device
>       after power loss or a SPAD configuration change.


Factory Calibration
===================

The TMF882X device supports a calibration data read/write mechanism (see
**factory calibration** details in the TMF882X datasheet). Factory calibration
provides a default calibration data. The TMF882X internal calibration data is
updated at runtime based off captured measurements. For best performance, the
host should periodically save the calibration data that should be written
back to the device after any power loss or SPAD configuration change.

Performing Factory Calibration
------------------------------

The TMF882X driver factory calibration process will stop all measurements,
disable any histogram dump settings, then perform the the factory calibration
command as documented in the TMF882X datasheet. The factory calibration process
can be performed by reading the SysFS
[app/factory_calibration](#appfactory_calibration).

>Example performing factory calibration:
>
>```
>    cat factory_calibration > calib.bin
>```

Reading Measurement-Updated Calibration Data
--------------------------------------------

The current measurement-updated calibration data can be read from the TMF882X
device by reading the SysFS [app/calibration_data](#appcalibration_data). Any
active measurements will be stopped, and then resumed after updating the
calibration data if there were no errors.

>Example reading current calibration data:
>
>```
>    cat calibration_data > calib.bin
>```

Writing Calibration Data
------------------------

Updating the TMF882X calibration data can be used to write the factory
calibration, or a measurement-updated calibration. Any active measurements
will be stopped, and then resumed after updating the calibration data if there
were no errors. The calibration data can be written to the device through SysFS by writing to
[app/calibration_data](#appcalibration_data).

>Example writing calibration data to TMF882X device:
>
>```
>    cat calib.bin > app/calibration_data
>```


Custom SPAD Configuration
=========================

The TMF882X device has several predefined SPAD configurations as detailed in
TMF882X datasheet, but also supports user custom SPAD configurations.

Order of Operations for Custom SPAD configuration:

1. Set [app/spad_map_id](#appspad_map_id) to a custom SPAD configuration
    value as detailed in the TMF882X datasheet.
2. Set the X and Y SPAD mask offsets in
    - [app/xoff_q1_0](#appxoff_q1_0)
    - [app/yoff_q1_0](#appyoff_q1_0)
3. Set the X and Y SPAD mask size in
    - [app/xsize_0](#appxsize_0)
    - [app/ysize_0](#appysize_0)
4. Set the SPAD enable mask in
    - [app/spad_mask_0](#appspad_mask_0)
5. Set the SPAD channel mapping in
    - [app/spad_map_0](#appspad_map_0)
6. Commit the custom SPAD configuration by writing to
    - [app/commit_spad_config](#appcommit_spad_cfg)

> **Note** 1: The SPAD mask and SPAD channel mapping should be equal in size to
>         the **[app/xsize_0](#appxsize_0) * [app/ysize_0](#appysize_0)**

> **Note** 2: If using a time-multiplexed custom SPAD configuration, the
>         _**N**th_ sub-capture SPAD configuration can be configured by using
>          the corresponding SysFS SPAD config attributes with postfix _\_**N**_

Example custom SPAD configuration with two time-multiplexed sub captures:

```
    # Set SPAD map ID to custom configuration with two multiplexed subcaptures
    #  that combine to a 4x4 zone
    echo 15 > app/spad_map_id

    # Move FOV South one SPAD for both subcaptures
    echo 0 > app/xoff_q1_0
    echo -2 > app/yoff_q1_0
    echo 0 > app/xoff_q1_1
    echo -2 > app/yoff_q1_1

    # Set SPAD mask size to 8 x 8
    echo 8 > app/xsize_0
    echo 8 > app/ysize_0
    echo 8 > app/xsize_1
    echo 8 > app/ysize_1

    # Set subcapture 0 to be left half
    echo -n "
    1 1 1 1 0 0 0 0
    1 1 1 1 0 0 0 0
    1 1 1 1 0 0 0 0
    1 1 1 1 0 0 0 0
    1 1 1 1 0 0 0 0
    1 1 1 1 0 0 0 0
    1 1 1 1 0 0 0 0
    1 1 1 1 0 0 0 0
    " > app/spad_mask_0

    # Set subcapture 1 to be right half
    echo -n "
    0 0 0 0 1 1 1 1
    0 0 0 0 1 1 1 1
    0 0 0 0 1 1 1 1
    0 0 0 0 1 1 1 1
    0 0 0 0 1 1 1 1
    0 0 0 0 1 1 1 1
    0 0 0 0 1 1 1 1
    0 0 0 0 1 1 1 1
    " > app/spad_mask_1

    # Set subcapture 0 SPAD channel mapping
    echo -n "
    1 1 2 2 2 2 1 1
    1 1 2 2 2 2 1 1
    3 3 4 4 4 4 3 3
    3 3 4 4 4 4 3 3
    5 5 6 6 6 6 5 5
    5 5 6 6 6 6 5 5
    7 7 8 8 8 8 7 7
    7 7 8 8 8 8 7 7
    " > app/spad_map_0

    # Set subcapture 1 SPAD channel mapping
    echo -n "
    1 1 2 2 2 2 1 1
    1 1 2 2 2 2 1 1
    3 3 4 4 4 4 3 3
    3 3 4 4 4 4 3 3
    5 5 6 6 6 6 5 5
    5 5 6 6 6 6 5 5
    7 7 8 8 8 8 7 7
    7 7 8 8 8 8 7 7
    " > app/spad_map_1

    # Commit custom SPAD configuration
    echo 1 > app/commit_spad_cfg
```

