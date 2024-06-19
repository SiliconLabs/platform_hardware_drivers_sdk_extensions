# DWM1000 - UWB Click (Mikroe) #

## Summary ##

This example project showcases the driver integration of the Mikroe DWM1000 - UWB Click board.

UWB Click is an Ultra-Wideband transceiver Click board™ that can be used in 2-way ranging or TDOA location systems to locate assets to a precision of 10 cm and supports data rates of up to 6.8 Mbps. This Click board™ features the DWM1000 module based on Qorvo's DW1000 Ultra-Wideband (UWB) transceiver from Qorvo.

It features a frequency range from 3.5GHz to 6.5GHz, a very precise location of tagged objects, up to 6.8Mbps data rates, low power consumption, and more while the communication with the MCU is accomplished through a standard SPI serial interface. This Click board™ is optimized for applications in real-time location systems and wireless sensor networks across a variety of markets including agriculture, building control and automation, factory automation, healthcare, safety & security, warehousing & logistics, and a range of others.

## Required Hardware ##

- [EFR32xG24 Explorer Kit](https://www.silabs.com/development-tools/wireless/efr32xg24-explorer-kit?tab=overview)

- [UWB Click](https://www.mikroe.com/uwb-click)

## Hardware Connection ##

The DWM1000 - UWB Click board supports MikroBus; therefore, it can easily connect to the MikroBus socket of the EFR32xG24 Explorer Kit. Be sure that the 45-degree corner of the board matches the 45-degree white line of the Explorer Kit.

The hardware connection is shown in the image below:

![board](image/hardware_connection.png)

### Driver Layer Diagram ###

The driver of the DWM1000 - UWB Click board builds upon more than one level of software. On the first layer, there are the SPIDRV and GPIO drivers from Silabs, which focus on interfacing with the xG24 motherboard. On top of that, there are multiple layers of drivers, which either work as an interfacing layer between the Click board and the motherboard or control the DWM1000 UWB module. Here you can see the high-level overview of the software layers:

![logging_screen](image/sw_layer.png)

## Setup ##

You can either create a project based on an example project or start with an empty example project.

### Create a project based on an example project ###

1. From the Launcher Home, add the BRD2703A to My Products, click on it, and click on the **EXAMPLE PROJECTS & DEMOS** tab. Find the example project filtering by uwb.

2. Click **Create** button on the **Third Party Hardware Drivers - DWM1000 - UWB Click (Mikroe)** example. Example project creation dialog pops up -> click Create and Finish and Project should be generated.

![create_example](image/create_example.png)

3. Build and flash this example to the board.

### Start with an empty example project ###

1. Create an "Empty C Project" for the "EFR32xG24 Explorer Kit" using Simplicity Studio v5. Use the default project settings.

2. Copy the file `app/example/mikroe_uwb_dwm1000/app.c` into the project root folder (overwriting the existing file).

3. Install the software components:

    - Open the .slcp file in the project.

    - Select the SOFTWARE COMPONENTS tab.

    - Install the following components:

        - **[Services] → [IO Stream] → [IO Stream: EUSART]** → default instance name: vcom
        - **[Application] → [Utility] → [Log]**
        - **[Application] → [Utility] → [Assert]**
        - **[Platform] → [Driver] → [Button] → [Simple Button]** → use default instance name: btn0
            - For using the SPI interface
                - [Platform] → [Driver] → [SPI] → [SPIDRV] → [mikroe] → change the configuration for [SPI master chip select (CS) control scheme] to "CS controlled by the application", use the default instance name "mikroe"
        - **[Third Party Hardware Drivers] → [Wireless Connectivity] → [DWM1000 - UWB Click (Mikroe)]** → use default configuration.

            ![config](image/uwb_config.png)

4. Build and flash this example to the board.

**Note:**

- Make sure that the SDK extension has already been installed. If not, please follow [this documentation](https://github.com/SiliconLabs/third_party_hw_drivers_extension/blob/master/README.md#how-to-add-to-simplicity-studio-ide).

- SDK Extension must be enabled for the project to install "DWM1000 - UWB Click (Mikroe)" component.

- Make sure that the SPI CS Control scheme is set to "CS controlled by the application", otherwise the driver is not able to communicate with the module properly.

## How It Works ##

This is an example that demonstrates the use of the DWM1000 - UWB Click board. First, you have to decide whether you would like to use the device in Transmitter (Tx) or Receiver (Rx) mode, which you can do by setting the dev_mode to either **MIKROE_DWM1000_MODE_TX** or **MIKROE_DWM1000_MODE_RX**.

In Tx mode, the device transmits a packet upon startup. The packet is re-transmitted every time when the BTN0 is pressed on the xG24 Explorer kit. The screenshot of the console can be seen below:

![transmit_cli](image/transmit_cli.png)

In Rx mode, the device enters receiver mode upon startup, and it prints each received packet after every IRQ event. The screenshot of the console can be seen below:

![receive_cli](image/receive_cli.png)

## Report Bugs & Get Support ##

To report bugs in the Application Examples projects, please create a new "Issue" in the "Issues" section of [third_party_hw_drivers_extension](https://github.com/SiliconLabs/third_party_hw_drivers_extension) repo. Please reference the board, project, and source files associated with the bug, and reference line numbers. If you are proposing a fix, also include information on the proposed fix. Since these examples are provided as-is, there is no guarantee that these examples will be updated to fix these issues.

Questions and comments related to these examples should be made by creating a new "Issue" in the "Issues" section of [third_party_hw_drivers_extension](https://github.com/SiliconLabs/third_party_hw_drivers_extension) repo.