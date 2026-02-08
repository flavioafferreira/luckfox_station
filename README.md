# Luckfox LoRaWAN Station

This repository contains a **Linux-based LoRaWAN station** written in C as module you can install using insmod like a driver, targeting Luckfox boards and similar embedded Linux platforms.  
The goal of this project is to provide a simple, hackable LoRaWAN station that can be integrated into custom gateways, test setups, or educational projects. Borned with a giroscopic
sensor and it's being developed.
The board used in this project is the model Luckfox Pico Pi A W 

## About the Board

LUCKFOX Pico Pi A W is a Linux development board designed for developers, based on LUCKFOX Core1106 module, which combines powerful arithmetic and rich scalability. Its 1TOPS AI power and 8GB eMMC storage can easily handle intelligent coding, image analysis (supporting 5MP ISP3.2) and edge computing tasks, while the 16-bit 256MB DDR3L memory ensures smooth multi-tasking.

The development board provides full-featured interfaces including MIPI CSI camera, GPIO, UART, SPI, I2C and USB-C/A. With the dip switches, users can flexibly switch between 4G M.2 or USB-A interfaces, and utilize the on-board PoE interface to achieve Power over Ethernet, adapting to scenarios such as industrial IoT and smart home.

In addition, LUCKFOX Pico Pi A W is equipped with a professional audio system (patch microphone + 3.5mm output) and a third-generation ISP image processor, making it suitable for security monitoring, voice interaction and multimedia development. Whether it is AIoT prototyping or embedded product mass production, this development board can significantly improve the development efficiency with its low power consumption and high integration.

Application scenarios: intelligent hardware development, 4G IoT devices, AI visual recognition, audio and video processing, industrial automation control.

---

## Features

- Written in standard **C** for Linux-based embedded systems  
- Designed to run as a **LoRaWAN station** on a Luckfox board   
- Build system based on a simple **Makefile**
- Support files for:
  - **Cases** – mechanical/3D models or enclosure references
  - **Datasheets** – hardware documentation
  - **Images** – reference pictures or assembly examples
  - **Instructions** – step-by-step guides

---

## Repository Structure

```
.
├── Cases/          # Mechanical or enclosure-related files
├── Datasheets/     # Hardware datasheets and reference documents
├── Images/         # Photos, diagrams, or renders of the setup
├── Instructions/   # Additional how-to or setup instructions
├── station.c       # Main C source file for the LoRaWAN station
├── station.ko      # Kernel object/module or prebuilt binary (depending on build)
├── Makefile        # Build script for compiling the project
└── .gitignore
```

---

## Requirements

To build and run this project you typically need:

- A **Linux** environment (PC or cross-compilation toolchain). I used the WSL Ubuntu under Windows.  
- A **C compiler**, such as `gcc` or an appropriate cross-compiler for the Luckfox board
- `make`
- A Luckfox (or similar) embedded Linux board connected to:
  - A supported **LoRa radio module** (e.g. via SPI/UART)
  - Power and network (Ethernet/Wi-Fi), depending on your use case

---

## Building

```bash
make
```

For cross‑compilation:

```bash
export CC=arm-linux-gnueabihf-gcc
make
```

---

## Installation

```bash
scp station root@<luckfox-ip>:/usr/local/bin/
chmod +x /usr/local/bin/station
```

If using `station.ko`:

```bash
scp station.ko root@<luckfox-ip>:/lib/modules/
```

---

## Usage

```bash
./station [options]
```

Possible options (depending on implementation):

- `-c <file>` – configuration file  
- `-d <device>` – radio/interface device  
- `-v` – verbose mode  
- `-h` – help  

---

## Configuration

Configuration may be done via compile‑time settings or optional configuration files.  
Check comments inside `station.c` or documentation under `Instructions/`.

---

## Hardware Setup

Folders such as `Cases/`, `Datasheets/`, and `Images/` provide support materials for assembly, electrical connections, and enclosure design.

---

## Development

```bash
make clean
make
```

Modify `station.c`, adjust the `Makefile`, recompile and deploy.

---

## Contributing

Feel free to open issues or submit pull requests with improvements or fixes.

---

## License

Please include a `LICENSE` file in the repository to specify usage permissions.

---

## Author

GitHub Repository: **flavioafferreira/luckfox_station**
