# PCA9685 - Servo Click (Mikroe) #

## Summary ##

This example project shows an example of Mikroe Servo Click board driver integration with the Silicon Labs Platform.

Servo Click is a compact add-on board that contains a PWM servo driver with voltage-sensing circuitry. This board features the PCA9685, an integrated 12-bit, 16-channel PWM driver, which can be configured to either sink 25mA per channel or drive each channel sourcing up to 10mA from NXP. Each channel has its duty cycle independently set from 0% to 100%. It can be used to simultaneously control 16 servo motors, each with its own programmable PWM signal.

This example can be used to demonstrate the capability of controlling the servo motor with the Mikroe Servo Click board.

## Required Hardware ##

- [**EFR32xG24-EK2703A** EFR32xG24 Explorer Kit](https://www.silabs.com/development-tools/wireless/efr32xg24-explorer-kit?tab=overview)

- [**Mikroe Servo Click**](https://www.mikroe.com/servo-click) board is based on the PCA9685

- Or [SiWx917 Wi-Fi 6 and Bluetooth LE 8 MB Flash SoC Pro Kit](https://www.silabs.com/development-tools/wireless/wi-fi/siwx917-pk6031a-wifi-6-bluetooth-le-soc-pro-kit) (BRD4002 + BRD4338A)

- Servo Motor GS-90

## Hardware Connection ##

- If the EFR32xG24 Explorer Kit is used:

  The MIC 2 Click board supports MikroBus, so it can connect easily to the Explorer Kit via MikroBus header. Assure that the 45-degree corner of Click board matches the 45-degree white line of the Explorer Kit.

  The hardware connection is shown in the image below:

  ![board](image/hardware_connection.png)

- If the SiWx917 Wi-Fi 6 and Bluetooth LE 8 MB Flash SoC Pro Kit is used:

  | Description           | BRD4338A GPIO | BRD4002 EXP/Breakout Header | MIC 2 Click        |
  | --------------------- | ------------- | --------------------------- | ------------------ |
  | I2C_SDA               | ULP_GPIO_6    | EXP_16                      | SDA                |
  | I2C_SCL               | ULP_GPIO_7    | EXP_15                      | SCL                |
  | Output Enable         | GPIO_46       | P24                         | OE                 |

## Setup ##

You can either create a project based on an example project or start with an empty example project.

### Create a project based on an example project ###

1. From the Launcher Home, add the BRD2703A to My Products, click on it, and click on the **EXAMPLE PROJECTS & DEMOS** tab. Find the example project filtering by **"servo"**.

2. Click **Create** button on the **Third Party Hardware Drivers - PCA9685 - Servo Click (Mikroe)** example. Example project creation dialog pops up -> click Create and Finish and Project should be generated.
![create_project](image/create_project.png)

### Start with an empty example project ###

1. Create an "Empty C Project" for the your board using Simplicity Studio v5. Use the default project settings.

2. Copy the file `app/example/mikroe_servo_pca9685/app.c` into the project root folder (overwriting existing file).

3. Install the software components:

    - Open the .slcp file in the project.

    - Select the SOFTWARE COMPONENTS tab.

    - Install the following components:

      **If the EFR32xG24 Explorer Kit is used:**

        - [Services] → [Timers] → [Sleep Timer]
        - [Services] → [IO Stream] → [IO Stream: USART] → default instance name: **vcom**
        - [Application] → [Utility] → [Log]
        - [Platform] → [Driver] → [I2C] → [I2CSPM] → default instance name: mikroe
        - [Third Party Hardware Drivers] → [Sensors] → [PCA9685 - Servo Click (Mikroe)] → use default configuration

      **If the SiWx917 Wi-Fi 6 and Bluetooth LE 8 MB Flash SoC Pro Kit is used:**

        - [WiSeConnect 3 SDK] → [Device] → [Si91x] → [MCU] → [Service] → [Sleep Timer for Si91x]
        - [WiSeConnect 3 SDK] → [Device] → [Si91x] → [MCU] → [Peripheral] → [I2C] → [i2c2]
        - [Third Party Hardware Drivers] → [Sensors] → [PCA9685 - Servo Click (Mikroe)] → use the default configuration
4. Build and flash this example to the board.

**Note:**

- Make sure that the **Third Party Hardware Drivers** extension is installed. If not, follow [this documentation](https://github.com/SiliconLabs/third_party_hw_drivers_extension/blob/master/README.md#how-to-add-to-simplicity-studio-ide).

- SDK Extension must be enabled for the project to install the "PCA9685 - Servo Click (Mikroe)" component. Selecting this component will also include the "I2CSPM" component with instance "mikroe".

## How It Works ##

### Driver Layer Diagram ###

![software_layer](image/software_layer.png)

### Testing ###

This example sets the position of the servo motor according to the user's settings. With its ability to precisely control motors and control 16 servo motors simultaneously, Mikroe Servo Click can be used in applications where a large number of servos need to be easily controlled, such as in the movie or theater industry (animatronics), robotics, RC toys, and similar.  

You can launch Console that's integrated into Simplicity Studio or use a third-party terminal tool like TeraTerm to receive the data from the USB. A screenshot of the console output is shown in the figure below.

![console_log](image/console_log.png)

This example uses the I2CSPM driver with the "mikroe" instance to interact with the Mikroe Servo Click board. After the click board is initialized, the application changes the position of the servo motor every 10 milliseconds. The motor will run in a counterclockwise direction when it reaches the limited position and then turn back in a clockwise direction to the original position.

To test the functionality, you can follow the movement of the motor shaft and check if it meets your expectations or not.

You can see the servo motor operation below:

![servo_operation](image/servo_operation.gif)

## Report Bugs & Get Support ##

To report bugs in the Application Examples projects, please create a new "Issue" in the "Issues" section of [third_party_hw_drivers_extension](https://github.com/SiliconLabs/third_party_hw_drivers_extension) repo. Please reference the board, project, and source files associated with the bug, and reference line numbers. If you are proposing a fix, also include information on the proposed fix. Since these examples are provided as-is, there is no guarantee that these examples will be updated to fix these issues.

Questions and comments related to these examples should be made by creating a new "Issue" in the "Issues" section of [third_party_hw_drivers_extension](https://github.com/SiliconLabs/third_party_hw_drivers_extension) repo.
