# MLX90640 IR Array Breakout Sensor Driver #

## Summary ##

This project aims to implement the hardware driver interacting with the [SparkFun IR Array Breakout - 55 Degree FOV, MLX90640 (Qwiic)](https://www.sparkfun.com/products/14844) via APIs of GSDK.

The MLX90640 is equipped with a 32x24 array of thermopile sensors creating, in essence, a low resolution thermal imaging camera. With this breakout users can detect surface temperatures from many feet away with an accuracy of ±1.5°C (best case). To make it even easier to get the infrared image, all communication is enacted exclusively via I2C, utilizing our handy Qwiic system.

## Hardware Required ##

- [BGM220 Explorer Kit board](https://www.silabs.com/development-tools/wireless/bluetooth/bgm220-explorer-kit?tab=overview)
- [SparkFun IR Array Breakout - 55 Degree FOV, MLX90640 (Qwiic)](https://www.sparkfun.com/products/14844)

## Connections Required ##

The MLX90640 module can be easily connected up with two I2C wires (SDA and SCL) along with 3v3 and GND. For the designated boards, SparkFun Qwiic compatible STEMMA QT connectors can be used.

![connection](connection.png)

## Setup ##

You can either create an project based on a example project or start with an empty example project.

### Create a project based on an example project ###

1. From the Launcher Home, add the BRD4314A to MyProducts, click on it, and click on the **EXAMPLE PROJECTS & DEMOS** tab. Find the example project with filter "MLX90640".

2. Click **Create** button on the **Third Party Hardware Drivers - MLX90640 - IR Array Breakout (Sparkfun)** example. Example project creation dialog pops up -> click Create and Finish and Project should be generated.
![Create_example](create_example.png)

3. Build and flash this example to the board.

### Start with an empty example project ###

1. Create an "Empty C Project" for the "BGM220 Explorer Kit Board" using Simplicity Studio v5. Use the default project settings.

2. Copy the file [app.c](https://github.com/SiliconLabs/third_party_hw_drivers_extension/tree/master/app/example/sparkfun_ir_array_mlx90640/app.c) (overwriting the existing file), into the project root folder.

3. Install the software components:

   - Open the .slcp file in the project.

   - Select the SOFTWARE COMPONENTS tab.

   - Install the following components:

      - [Services] → [Sleep Timer]
      - [Services] → [IO Stream] → [IO Stream: USART] → default instance name: vcom
      - [Application] → [Utility] → [Log]
      - [Third Party Hardware Drivers] → [Sensors] → [MLX90640 - IR Array Breakout (Sparkfun)]

4. Install printf float

   - Open Properties of the project.

   - Select C/C++ Build > Settings > Tool Settings >GNU ARM C Linker > General. Check Printf float.

      ![float](float.png)

5. Build and flash the project to your device.

**Note:**

- Make sure the SDK extension already be installed. If not please follow [this documentation](https://github.com/SiliconLabs/third_party_hw_drivers_extension/blob/master/README.md).

- SDK Extension must be enabled for the project to install "MLX90640 - IR Array Breakout (Sparkfun)" component. Selecting this component will also include the "I2CSPM" component with default configurated instance: qwiic.

- The example project are built on the BRD4314A board. For other boards, selecting the "MLX90640 - IR Array Breakout (Sparkfun)" component will include the "I2CSPM" component with an unconfigured instance: inst0. This instance should be configurated by users.

- The driver stores the contents of the EEPROM, so the HEAP and STACK sizes need to be increased. Edit the config/sl_memory_config.h file and increase SL_STACK_SIZE to 10240 and SL_HEAP_SIZE to 6144

![memory_config](memory_config.png)

## How It Works ##

### API Overview ###

[sparkfun_mlx90640.c](https://github.com/SiliconLabs/third_party_hw_drivers_extension/tree/master/driver/public/silabs/ir_array_mlx90640/src/sparkfun_mlx90640.c) - This is the top-level API implementation. The user application should only use the APIs listed below.

- `sparkfun_mlx90640_init`: Initialize mlx90640 driver

- `sparkfun_mlx90640_get_image_array`: Provides an array of temperatures for all 768 pixel.

[sparkfun_mlx90640_i2c.c](https://github.com/SiliconLabs/third_party_hw_drivers_extension/tree/master/driver/public/silabs/ir_array_mlx90640/src/sparkfun_mlx90640_i2c.c) - Implements mlx90640 I2C communication.

- `sparkfun_mlx90640_i2c_read`: I2C read implementation for 16-bit values.

- `sparkfun_mlx90640_i2c_write`: I2C write implementation.

### Sensor operation principle ###

The MLX90640 sensor is factory calibrated with calibration constants stored in the EEPROM memory. The ambient and object temperature can be calculated based on these calibration constants and the measurement data.

The MLX90640 is factory calibrated in the ambient temperature range from -20 to 85˚C and from -20 to 200˚C for
the object temperature range. The measured value is the average temperature of all objects in the Field Of View
of the sensor (55 degrees).

The thermopile sensing element voltage signal is amplified and digitized. After digital filtering, the raw measurement result is stored in the RAM memory. Furthermore, the mlx90640 contains a sensor element to measure the temperature of the sensor itself. The raw information of this  sensor is also stored in RAM after processing. The result of each measurement conversion is accessible via I2C. From the measurement data and the calibration data the external unit can calculate both the sensor temperature and the object temperature.

### Testing ###

Initializing the driver and getting a temperature array from the sensor happens in the following way:

![init](init_workflow.png)

![workflow](workflow.png)

Application only needs to call **sparkfun_mlx90640_init** once at startup, then with **sparkfun_mlx90640_get_image_array()** function it's possible to request an array of temperatures for all 768 pixels.
For more features or possibilities please refer to the API function descriptions found in [sparkfun_mlx90640.h](https://github.com/SiliconLabs/third_party_hw_drivers_extension/tree/master/driver/public/silabs/ir_array_mlx90640/inc/sparkfun_mlx90640.h).

## Generating image with Python ##

There is a Python script [(image_generator.py)](image_generator.py) that parses the array of temperatures provided by the driver and generates an image based on the data. By listening on the COM port, it's possible to generate a low-refresh-rate live video out of the images. For this the following python packages are needed:

- numpy
- seaborn
- matplotlib.pylab
- pyserial

Simply replace the "COM19" in the script with the actual COM port your device is connected to.
Then run the script, and the live image will be visible in a new window.

![demo](demo.gif)

## Report Bugs & Get Support ##

To report bugs in the Application Examples projects, please create a new "Issue" in the "Issues" section of [third_party_hw_drivers_extension](https://github.com/SiliconLabs/third_party_hw_drivers_extension) repo. Please reference the board, project, and source files associated with the bug, and reference line numbers. If you are proposing a fix, also include information on the proposed fix. Since these examples are provided as-is, there is no guarantee that these examples will be updated to fix these issues.

Questions and comments related to these examples should be made by creating a new "Issue" in the "Issues" section of [third_party_hw_drivers_extension](https://github.com/SiliconLabs/third_party_hw_drivers_extension) repo.