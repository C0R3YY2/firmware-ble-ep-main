
# Jeeva BLE Endpoint Development Repository

## Setup Instructions

### Download and Install Prerequisites

1. Install the [STSW-BNRGLP-DK software package](https://www.st.com/en/embedded-software/stsw-bnrglp-dk.html)

1. Install [STSW-WISE-STUDIO](https://www.st.com/en/embedded-software/stsw-wise-studio.html)

1. Clone this repo and initialize submodules

```sh
git clone git@gitlab.com:jeeva-wireless/firmware-ble-ep.git
cd firmware-ble-ep
git submodule update --init --recursive
```

### Open Projects

1. Launch WiSE Studio and select `firmware-ble-ep` as the workspace directory

1. Go to **File** and select **Open Projects from File System...**

1. Select **Directory...** and navigate to `firmware-ble-ep` and open it

1. De-select `firmware-ble-ep`, but leave all of the nested Eclipse projects checked

    > Alternatively, select only the projects you need

1. Select **Finish**

### Configure Projects

1. Select a project that you want to build, right click -> select **Properties**

1. Open the drop-down menu under **Resource** and select **Linked Resources**

1. Update the `SDK_HOME` path variable to the location where you installed the STSW-BNRGLP-DK software package, for example, `/home/jerrold/BlueNRG-LP_LPS DK 1.2.0`

1. Open the drop-down menu under **C/C++ Build** and select **Build Variables**

1. Update the `SdkDirPath` variable to the same location, for example, `/home/jerrold/BlueNRG-LP_LPS DK 1.2.0`

1. Select **Apply and Close**

### Build

1. To build, right click on the project, and select **Build Project**

1. To flash a board, right click on the project, and select **Run As -> 2 WiSE C/C++ Application**

1. If prompted to choose a JTAG interface, select **CMSIS-DAP**

## Troubleshooting

If you have issues flashing, such as being unable to find binaries, try opening project properties, and in the **Run/Debug Settings** delete your launch configurations. They will be re-generated when you try to launch the project again.

If you have errors related to files not being included/built properly, open your project properties, and under **Resource**, go to **Linked Resources** -> **Linked Resources**. All of the linked source files being used in your project should appear here, without errors. If there are path errors, you can resolve them here. In addition, make sure you have included the appropriate header files under **C/C++ Build** -> **GCC C Compiler** -> **Includes**

When you have made changes to project settings, sometimes errors won't be resolved until you right click on the project and select **Index -> Rebuild**. It also should never hurt to refresh, clean, and rebuild from this menu.

If you're using a dev kit to flash an endpoint, sometimes you might encounter issues finding the target. You might have to make sure the endpoint is powered from a different source (a battery or external power supply) and then plug in the dev kit afterwards. Once the dev kit has been plugged in and detects the chip, you should be able to remove the other power source without issue.
