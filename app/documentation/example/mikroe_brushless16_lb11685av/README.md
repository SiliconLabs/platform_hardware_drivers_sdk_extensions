# LB11685AV - Brushless 16 Click (Mikroe) #

## Summary ##

This project shows the driver implementation for the Brushless 16 Click Board to control a BLDC motor using Silicon Labs Platform.

Brushless 16 Click as its foundation uses the LB11685AV, a three-phase full-wave current-linear-drive motor driver from ON Semiconductor. The LB11685AV adopts a sensorless control system without the use of a Hall-effect device. It features a current soft switching circuit for silent operation. Not only can it be used for home appliances such as air-conditioners, cooling fans, air purifiers, but it is appropriate for industrial equipment too.

## Required Hardware ##

- [**BGM220-EK4314A** BGM220 Bluetooth Module Explorer Kit (BRD4314A BGM220 Explorer Kit Board)](https://www.silabs.com/development-tools/wireless/bluetooth/bgm220-explorer-kit)

- Or [SiWx917 Wi-Fi 6 and Bluetooth LE 8 MB Flash SoC Pro Kit](https://www.silabs.com/development-tools/wireless/wi-fi/siwx917-pk6031a-wifi-6-bluetooth-le-soc-pro-kit) (BRD4002 + BRD4338A)

- [**Brushless 16 Click** board based on LB11685AV IC](https://www.mikroe.com/brushless-16-click)

## Hardware Connection ##

- If the BGM220P Explorer Kit is used:

  The Brushless 16 board supports MikroBus, so it can connect easily to the Explorer Kit via MikroBus header. Assure that the 45-degree corner of the Click board matches the 45-degree white line of the Explorer Kit.

  The hardware connection is shown in the image below:

  ![board](image/hardware_connection.png "Hardware connection")

- If the SiWx917 Wi-Fi 6 and Bluetooth LE 8 MB Flash SoC Pro Kit is used:

  | Description               | BRD4338A GPIO | BRD4002 Breakout | Brushless 16 Click |
  | ------------------------- | ------------- | ---------------- | ------------------ |
  | Enable                    | GPIO_48       | P28              | EN                 |
  | Rotation Speed Indication | GPIO_47       | P26              | FG                 |
  | Motor Lock Detection      | GPIO_46       | P24              | RD                 |

## Setup ##

You can either create a project based on an example project or start with an empty example project.

### Create a project based on an example project ###

1. From the Launcher Home, add your device to My Products, click on it, and click on the **EXAMPLE PROJECTS & DEMOS** tab. Find the example project filtering by *brushless*.

2. Click **Create** button on the **Third Party Hardware Drivers - LB11685AV - Brushless 16 Click (Mikroe)** example. Example project creation dialog pops up -> click Create and Finish and Project should be generated.

   ![Create_example](image/create_example.png)

3. Build and flash this example to the board.

### Start with an empty example project ###

1. Create an "Empty C Project" for your board using Simplicity Studio v5. Use the default project settings.

2. Copy the file `app/example/mikroe_brushless16_lb11685av/app.c` into the project root folder (overwriting existing file).

3. Install the software components:

    - Open the .slcp file in the project.

    - Select the SOFTWARE COMPONENTS tab.

    - Install the following components:

      **If the BGM220P Explorer Kit is used:**
        - [Services] → [Timers] → [Sleep Timer]
        - [Services] → [IO Stream] → [IO Stream: USART] → default instance name: vcom
        - [Application] → [Utility] → [Log]
        - [Application] → [Utility] → [Assert]
        - [Third Party Hardware Drivers] → [Motor Control] → [LB11685AV - Brushless 16 Click (Mikroe)] → use default configuration

      **If the SiWx917 Wi-Fi 6 and Bluetooth LE 8 MB Flash SoC Pro Kit is used:**
        - [Application] → [Utility] → [Assert]
        - [WiSeConnect 3 SDK] → [Device] → [Si91x] → [MCU] → [Service] → [Sleep Timer for Si91x]
        - [Third Party Hardware Drivers] → [Motor Control] → [LB11685AV - Brushless 16 Click (Mikroe)] → use default configuration

4. Build and flash this example to the board.

**Note:**

- Make sure that the **Third Party Hardware Drivers** extension is installed. If not, follow [this documentation](https://github.com/SiliconLabs/third_party_hw_drivers_extension/blob/master/README.md#how-to-add-to-simplicity-studio-ide).

- SDK Extension must be enabled for the project to install "LB11685AV - Brushless 16 Click (Mikroe)" component.

## How It Works ##

The application checks the state of information pins, and stops and starts motor output every second.
The workflow of this example is the following image:

![workflow](image/workflow.png "Workflow")

## Report Bugs & Get Support ##

To report bugs in the Application Examples projects, please create a new "Issue" in the "Issues" section of [third_party_hw_drivers_extension](https://github.com/SiliconLabs/third_party_hw_drivers_extension) repo. Please reference the board, project, and source files associated with the bug, and reference line numbers. If you are proposing a fix, also include information on the proposed fix. Since these examples are provided as-is, there is no guarantee that these examples will be updated to fix these issues.

Questions and comments related to these examples should be made by creating a new "Issue" in the "Issues" section of [third_party_hw_drivers_extension](https://github.com/SiliconLabs/third_party_hw_drivers_extension) repo.
