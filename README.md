# ARMBoy &trade; Kernel.
the armboy kernel is the kernel used by the armboy gaming system. It is designed to run on a sam3x8e processor, based
around the cortex-M3. it should be fairly adaptable to a cortex-M4 such as the ones found on the teensy boards.

### Armboy Ecosystem
- [sam3x8e hardware api](https://github.com/CanadianCommander/sam3x8eHardwareAPI)
- [ArmBoy Flasher tool](https://github.com/CanadianCommander/ARMBoyFlasher)
- [SSD1289 lcd driver](https://github.com/CanadianCommander/armboy_SSD1289lcdDriver)
- [user input driver](https://github.com/CanadianCommander/ArmboyInputDriver)
- [FAT32 file system driver](https://github.com/CanadianCommander/armboy-fs)
- [System API](https://github.com/CanadianCommander/armboy-api)
- [Bootstrapper](https://github.com/CanadianCommander/armboy-init)

## documentation  
read the wiki. and look in the header files for premium function descriptions.

## Building
*only compatible with Linux/MacOS*

1. download the [GNU Arm Embedded Toolchain](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads).
2. install [bossa command line tools](http://www.shumatech.com/web/products/bossa). On Debian based distro just issue `sudo apt install bossa-cli`
3. clone this repository and these supporting repos:
  - [ArmBoy Flasher tool](https://github.com/CanadianCommander/ARMBoyFlasher)
  - [SSD1289 lcd driver](https://github.com/CanadianCommander/armboy_SSD1289lcdDriver)
  - [user input driver](https://github.com/CanadianCommander/ArmboyInputDriver)
  - [FAT32 file system driver](https://github.com/CanadianCommander/armboy-fs)
  - [System API](https://github.com/CanadianCommander/armboy-api)
  - [Bootstrapper](https://github.com/CanadianCommander/armboy-init)
4. build this repository by issuing these commands:
  - `cd armboy-kernel`
  - `git submodule update --init --recursive`
  - `make`
5. there should now be a file `src/kernel.bin` to upload this file to your Arduino run `make upload` (may have to run this as root). if make upload complains about `/dev/ttyACM0` not found you are 1. on Mac 2. your Arduino board has been given a different name. To correct this you must look under `/dev/` and determine the name of your Arduino board. On Mac it should be some thing like `/dev/cu.usbmodem-12345`. Once you have determined the correct device name update the `src/Makefile` (ya I know I should have used a variable, sorry about that.)

6. Build kernel modules in much the same way as the kernel its self. For example to build the  [SSD1289 lcd driver](https://github.com/CanadianCommander/armboy_SSD1289lcdDriver) you would issue:
  - `cd armboy_SSD1289lcdDriver`
  - `git submodule update --init --recursive`
  - `make -C armboy-kernel`
  - `make -C src`

7. After building a module there should be a .bin file in its src directory. to upload it to an ArmBoy (must already have the kernel flashed on to it) use the [ArmBoy Flasher tool](https://github.com/CanadianCommander/ARMBoyFlasher):
  - if you have not already, issue `delete all` command to the ArmBoy through the debug terminal.
  - `./abFlasher <.bin file> </dev/yourArduinoSerialPort>`

8. Continue building and uploading modules untill you have uploaded the kernel, along with,  [SSD1289 lcd driver](https://github.com/CanadianCommander/armboy_SSD1289lcdDriver), [user input driver](https://github.com/CanadianCommander/ArmboyInputDriver), [FAT32 file system driver](https://github.com/CanadianCommander/armboy-fs) and [Bootstrapper](https://github.com/CanadianCommander/armboy-init).

9. At this point your system should be able to run the examples found in [System API](https://github.com/CanadianCommander/armboy-api). Build the example and put it on a FAT32 formatted SD card as `boot.bin` (along with any assets it requires). Insert the card in to your ArmBoy and when you power it up the example program will start! Note, you can also flash/upload programs to the system over the debug port using the [ArmBoy Flasher tool](https://github.com/CanadianCommander/ARMBoyFlasher).
