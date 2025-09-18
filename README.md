README - Embedded System: OS and Interfacing Project
===================================================

## Introduction
This repository contains the source code and project files for the **Embedded System: OS and Interfacing Group Assignment**. 
The project involves implementing a bare-metal operating system on the Raspberry Pi with three major components:
1. Command Line Interpreter (CLI)
2. Image, Video, and Text Display
3. Application Development (Game)

For a detailed explanation of implementation and results, please refer to the full report included in the repository.

---

## RPi3/4 Board GUIDE
- The project was emulated in **QEMU Emulator** using **Raspberry Pi 3 Board**. Hence, if you are using **Raspberry Pi 4 Board**, go to `kernel/gpio.h` and comment out the line
**#define RPI3**, such as shown below:
  
  ```bash
  //#define RPI3 //enable when using RPI3 (QEMU emulation/ real board)
  #ifdef RPI3 //RPI3
    #define MMIO_BASE       0x3F000000
  #else //RPI4
    #define MMIO_BASE       0xFE000000
  #endif
  ```
- Afterwards, type `make` in the Integrated Terminal and compile the project for RPi 4 Board and perform the steps instructed below.

## How to Build & Run
1. Download the .zip file and extract all contents into a folder.

2. Build the project using the provided `Makefile` by opening the project folder in VS Code and using its Integrated Terminal
   or by right-clicking and opening a Terminal within the folder of the project:
   ```bash
   make
   ```

4. Flash the generated `kernel8.img` onto an SD card for the Raspberry Pi, delete any other .img files for ensuring the board only runs the new `kernel8.img` file.

5. Insert the SD card into the Raspberry Pi and power it on.

6. Use a serial connection to access the CLI and interact with the system.

---

## Notes
- The project was tested on both **QEMU Emulator** and **Raspberry Pi hardware**.
- The `Makefile` can be customized for cross-compilation depending on your setup.
- For QEMU testing, ensure you have the ARM64 version of QEMU installed.

---

## Authors
- Hong Thieu Kiet (s3993986)
- Le Phuong Ngan (s3978567)
- Nguyen Hoang Son (s3990627)
- Lee Dohwan (s3878104)
- Huynh Nhat Anh (s3924763)

