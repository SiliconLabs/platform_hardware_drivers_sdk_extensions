# Relay 2 Click #

## Summary ##

This project shows the implementation of a two single-pole solid state relays (SSR) IC that is integrated on the Relay 2 Click board.

## Required Hardware ##

- [A BGM220P Explorer Kit board.](https://www.silabs.com/development-tools/wireless/bluetooth/bgm220-explorer-kit)

- [A MikroE Relay 2 Click board.](https://www.mikroe.com/relay-2-click)

## Hardware Connection ##

The Relay 2 Click board can just be "clicked" into its place. Be sure that the board's 45-degree corner matches the Explorer Kit's 45-degree white line.

![board](hardware_connection.png)

## Setup ##

You can either create a project based on a example project or start with an empty example project.

### Create a project based on a example project ###

1. From the Launcher Home, add the BRD4314A to MyProducts, click on it, and click on the **EXAMPLE PROJECTS & DEMOS** tab. Find the example project with filter **relay**.

2. Click **Create** button on the **Third Party Hardware Drivers - LCA717 - Single-pole Solid State Relays (Relay 2 Click Board)** example. Example project creation dialog pops up -> click Create and Finish and Project should be generated.
![Create_example](create_example.png)

3. Build and flash this example to the board.

### Start with an empty example project ###

1. Create an "Empty C Project" for the "BGM220 Explorer Kit Board" using Simplicity Studio v5. Use the default project settings.

2. Copy the file [app.c](https://github.com/SiliconLabs/third_party_hw_drivers_extension/tree/feature/master/app/example/mikroe_relay2_lca717/app.c) (overwriting existing file), into the project root folder.

3. Install the software components:

    - Open the .slcp file in the project.

    - Select the SOFTWARE COMPONENTS tab.

    - Install the following components:

        - [Services] → [Sleep Timer]
        - [Third Party Hardware Drivers] → [Miscellaneous] → [Relay 2 Click (Mikroe)] → use default configuration.
        ![config](configuration.png)

4. Build and flash this example to the board.

**Note:**

- Make sure the SDK extension already be installed. If not please follow [this documentation](https://github.com/SiliconLabs/third_party_hw_drivers_extension/blob/master/README.md).

- SDK Extension must be enabled for the project to install "Relay 2 Click (Mikroe)" component.

- The example project are built on the BRD4314A board. For another boards, selecting the "Relay 2 Click (Mikroe)" component. This instance should be configurated by users.

## How It Works ##

The input stage of the device is comprised of a highly efficient GaAIAs infrared LED, used to drive the photovoltaic elements of the SSR. The output stage has two N-type MOSFETs, which allow both DC and AC to be switched to the output stage.

## Report Bugs & Get Support ##

To report bugs in the Application Examples projects, please create a new "Issue" in the "Issues" section of [platform_hardware_drivers_sdk_extensions](https://github.com/SiliconLabs/platform_hardware_drivers_sdk_extensions) repo. Please reference the board, project, and source files associated with the bug, and reference line numbers. If you are proposing a fix, also include information on the proposed fix. Since these examples are provided as-is, there is no guarantee that these examples will be updated to fix these issues.

Questions and comments related to these examples should be made by creating a new "Issue" in the "Issues" section of [platform_hardware_drivers_sdk_extensions](https://github.com/SiliconLabs/platform_hardware_drivers_sdk_extensions) repo.