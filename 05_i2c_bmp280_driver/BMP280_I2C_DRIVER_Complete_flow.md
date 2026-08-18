===============================================================================
              BMP280 LINUX I2C DRIVER - COMPLETE FLOW
===============================================================================


1. PHYSICAL HARDWARE
---------------------

BMP280 is physically connected to Raspberry Pi.

        BMP280
        +----------------+
        |                |
        | VCC ---------- +---- 3.3V
        | GND ---------- +---- GND
        | SDA ---------- +---- I2C SDA
        | SCL ---------- +---- I2C SCL
        |                |
        +----------------+

In our case:

        I2C Bus     = 1
        I2C Address = 0x77


-------------------------------------------------------------------------------
2. I2C CONTROLLER
-------------------------------------------------------------------------------

Raspberry Pi has an I2C controller.

Device Tree describes the controller:

        i2c@7e804000
                |
                |
                +---- Linux I2C adapter
                            |
                            v
                         i2c-1

We verified:

        cat /sys/class/i2c-dev/i2c-1/name

Output:

        bcm2835 (i2c@7e804000)


-------------------------------------------------------------------------------
3. PHYSICAL COMMUNICATION TEST
-------------------------------------------------------------------------------

During hardware bring-up we can use:

        sudo i2cdetect -y 1

We got:

        70: -- -- -- -- -- -- -- 77

This means:

        Something is responding at I2C address 0x77.


IMPORTANT:

        i2cdetect does NOT create a Linux device.

It only answers:

        "Is something responding at address 0x77?"


So:

        i2cdetect
             |
             v
        Physical I2C test
             |
             v
        Device responds at 0x77


-------------------------------------------------------------------------------
4. DEVICE TREE DESCRIBES THE HARDWARE
-------------------------------------------------------------------------------

Linux needs to know that a BMP280 exists at address 0x77.

We create a Device Tree overlay:

        /dts-v1/;
        /plugin/;

        / {
                fragment@0 {
                        target-path = "/soc/i2c@7e804000";

                        __overlay__ {
                                #address-cells = <1>;
                                #size-cells = <0>;
                                status = "okay";

                                bmp280@77 {
                                        compatible = "bosch,bmp280";
                                        reg = <0x77>;
                                        status = "okay";
                                };
                        };
                };
        };


The important part is:

        bmp280@77 {
                compatible = "bosch,bmp280";
                reg = <0x77>;
        };


Meaning:

        Device type:
                "bosch,bmp280"

        I2C address:
                0x77


-------------------------------------------------------------------------------
5. APPLY DEVICE TREE OVERLAY
-------------------------------------------------------------------------------

During development we can apply the overlay at runtime:

        sudo dtoverlay mybmp280-overlay.dtbo


The overlay modifies the running Device Tree.

Conceptually:

        Existing Device Tree
                |
                | add BMP280 node
                v
        Modified Device Tree

        i2c@7e804000
                |
                +---- bmp280@77
                        |
                        +---- compatible = "bosch,bmp280"
                        |
                        +---- reg = <0x77>


-------------------------------------------------------------------------------
6. I2C CORE CREATES THE LINUX DEVICE
-------------------------------------------------------------------------------

After Device Tree describes the BMP280:

        Device Tree
             |
             v
        I2C core
             |
             v
        creates I2C client
             |
             v
        1-0077


Linux naming:

        1-0077
        | |
        | +---- I2C address = 0x77
        |
        +------ I2C bus = 1


We can see it with:

        ls /sys/bus/i2c/devices/


Before overlay:

        i2c-1
        i2c-2

After overlay:

        i2c-1
        i2c-2
        1-0077


IMPORTANT:

        This is the important transition:

        Physical device
              |
              v
        Device Tree description
              |
              v
        Linux I2C device
              |
              v
        1-0077


-------------------------------------------------------------------------------
7. WRITE AND COMPILE OUR DRIVER
-------------------------------------------------------------------------------

Our driver source:

        mybmp280.c

Compile:

        make

Result:

        mybmp280.ko


The driver contains:

        struct i2c_driver bmp280_driver


It also contains the match table:

        static const struct of_device_id bmp280_of_match[] = {
                { .compatible = "bosch,bmp280" },
                { }
        };


This tells Linux:

        "I can handle a device whose compatible string
         is bosch,bmp280."


-------------------------------------------------------------------------------
8. LOAD THE DRIVER
-------------------------------------------------------------------------------

Load:

        sudo insmod mybmp280.ko


The module registers the driver with the I2C core.

Conceptually:

        insmod mybmp280.ko
                |
                v
        module_init()
                |
                v
        i2c_add_driver()
                |
                v
        mybmp280 driver registered
                |
                v
        I2C core


At this point:

        Linux knows:

        "mybmp280 driver exists."


IMPORTANT:

        Driver registration alone does NOT mean probe()
        will necessarily be called.


-------------------------------------------------------------------------------
9. DRIVER / DEVICE MATCHING
-------------------------------------------------------------------------------

Now Linux has:

        DEVICE:

                1-0077
                compatible = "bosch,bmp280"


And:

        DRIVER:

                mybmp280
                compatible = "bosch,bmp280"


The I2C core compares them:

        DEVICE                         DRIVER

        "bosch,bmp280"  <---------->  "bosch,bmp280"
                              MATCH
                                |
                                v
                              probe()


This is the critical connection.


-------------------------------------------------------------------------------
10. WHY PROBE() WAS NOT CALLED INITIALLY
-------------------------------------------------------------------------------

Initially our situation was:

        Physical BMP280
                |
                v
        responds at 0x77
                |
                +------ i2cdetect sees it
                |
                v
        BUT Linux had no 1-0077
                |
                v
        insmod mybmp280.ko
                |
                v
        driver registered
                |
                v
        matching device?
                |
                v
                NO
                |
                v
        probe() NOT called


The missing piece was:

        Device Tree description of the BMP280.


After applying the overlay:

        Device Tree
                |
                v
        creates 1-0077
                |
                v
        driver already registered
                |
                v
        compatible matches
                |
                v
        probe() called


-------------------------------------------------------------------------------
11. PROBE()
-------------------------------------------------------------------------------

Our driver contains:

        static int bmp280_probe(struct i2c_client *client)


The I2C core calls:

        bmp280_probe(client);


The client contains information about the device.

For example:

        client->addr
                |
                v
              0x77

        client->adapter
                |
                v
        I2C bus 1 / bcm2835 controller


-------------------------------------------------------------------------------
12. WHAT HAPPENS INSIDE PROBE()
-------------------------------------------------------------------------------

Our probe sequence:

        probe()
          |
          +---- 1. Allocate private data
          |
          +---- 2. Store client pointer
          |
          +---- 3. Read CHIP_ID
          |
          +---- 4. Verify BMP280
          |
          +---- 5. Soft reset
          |
          +---- 6. Read calibration
          |
          +---- 7. Configure sensor
          |
          +---- 8. Read temperature
          |
          +---- 9. Return 0
          |
          v
        Device successfully bound


-------------------------------------------------------------------------------
13. READ CHIP ID
-------------------------------------------------------------------------------

Driver executes:

        chip_id = i2c_smbus_read_byte_data(client, 0xD0);


This causes actual I2C communication.

IMPORTANT:

        0x77 = I2C DEVICE ADDRESS

        0xD0 = BMP280 REGISTER ADDRESS

        0x58 = VALUE STORED IN CHIP ID REGISTER


Communication:

        Raspberry Pi
             |
             | Address = 0x77
             | Register = 0xD0
             | READ
             |
             +---------------------> BMP280
                                      |
                                      | register 0xD0
                                      |
                                      | value = 0x58
                                      |
             <-----------------------+
                       0x58


Driver checks:

        chip_id == 0x58


If true:

        BMP280 detected.


-------------------------------------------------------------------------------
14. READ CALIBRATION
-------------------------------------------------------------------------------

Driver reads 24 bytes starting at:

        0x88


Example:

        T1 = 28179
        T2 = 26318
        T3 = 50
        ...


These values are factory calibration coefficients
stored inside the BMP280.


The driver stores them in:

        struct bmp280_data


-------------------------------------------------------------------------------
15. CONFIGURE BMP280
-------------------------------------------------------------------------------

Driver writes configuration to:

        CTRL_MEAS = 0xF4


Example:

        0x27


This configures:

        Temperature oversampling
        Pressure oversampling
        Normal measurement mode


-------------------------------------------------------------------------------
16. READ TEMPERATURE
-------------------------------------------------------------------------------

Driver reads:

        0xFA
        0xFB
        0xFC


Example raw bytes:

        88 3F 00


Driver combines them into a 20-bit ADC value:

        0x883F0


Then:

        raw ADC
           |
           +---- calibration coefficients
           |
           v
        compensation formula
           |
           v
        temperature


Example:

        33.64 C


-------------------------------------------------------------------------------
17. PROBE SUCCESS
-------------------------------------------------------------------------------

If everything succeeds:

        return 0;


This tells the kernel:

        "The device was successfully initialized
         by this driver."


Final relationship:

        1-0077
           |
           |
           +------ mybmp280
                     |
                     v
                  probe()
                     |
                     v
              BMP280 initialized


-------------------------------------------------------------------------------
18. WHAT HAPPENS AFTER PROBE()
-------------------------------------------------------------------------------

probe() is mainly initialization.

After successful probe:

        Device
           |
           v
        Bound to driver
           |
           v
        Driver is ready


Later, we can expose the sensor to userspace.

For example:

        userspace application
                |
                v
        /dev/bmp280
                |
                v
        file_operations
                |
                v
        BMP280 driver
                |
                v
        I2C transaction
                |
                v
        BMP280 hardware


-------------------------------------------------------------------------------
19. COMPLETE FLOW - SHORT VERSION
-------------------------------------------------------------------------------

        PHYSICAL HARDWARE
                |
                v
        BMP280 @ 0x77
                |
                v
        Raspberry Pi I2C controller
                |
                v
        Device Tree
                |
                | compatible = "bosch,bmp280"
                | reg = <0x77>
                v
        I2C core
                |
                v
        creates i2c_client
                |
                v
        Linux device: 1-0077
                |
                |
                | driver matching
                v
        mybmp280 driver
                |
                v
        probe()
                |
                +---- Read CHIP_ID 0xD0
                |          |
                |          v
                |        0x58
                |
                +---- Read calibration
                |
                +---- Configure sensor
                |
                +---- Read temperature
                |
                v
        probe() returns 0
                |
                v
        DEVICE BOUND TO DRIVER


===============================================================================
KEY CONCEPTS TO REMEMBER
===============================================================================

1. i2cdetect
   -----------
   Tests physical I2C communication.

        "Is something responding at 0x77?"


2. Device Tree
   ------------
   Describes expected hardware.

        "There is a BMP280 at 0x77."


3. I2C core
   ---------
   Creates the Linux I2C device/client.

        "1-0077"


4. Driver
   -------
   Describes how to operate the device.

        "I support bosch,bmp280."


5. compatible
   ----------
   Connects the Device Tree device to the driver.

        "bosch,bmp280"
              |
              +------ MATCH ------+
                                  |
                                  v
                                probe()


6. probe()
   --------
   Driver initialization point.

        probe()
           |
           +---- communicate with hardware
           |
           +---- initialize device
           |
           +---- prepare driver
           |
           v
        return 0


7. I2C ADDRESS vs REGISTER vs VALUE
   ---------------------------------

        0x77 = DEVICE ADDRESS

        0xD0 = REGISTER ADDRESS

        0x58 = REGISTER VALUE


===============================================================================
MOST IMPORTANT INTERVIEW FLOW
===============================================================================

Question:

"How does your Linux I2C driver get connected to the BMP280?"


Answer:

"The Device Tree describes the BMP280 at I2C address 0x77
with compatible string 'bosch,bmp280'. The I2C core creates
an i2c_client for that device. My driver registers an
i2c_driver with a matching of_device_id table. When the
I2C core matches the device and driver, it calls my probe()
function. In probe(), I communicate with the BMP280 through
the I2C API, verify the CHIP_ID, read calibration data,
configure the sensor, and initialize it."


===============================================================================
