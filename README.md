# Luckfox LoRaWAN Station

This repository contains a **Linux-based LoRaWAN station** written in C, targeting Luckfox boards and similar embedded Linux platforms.  
The goal of this project is to provide a simple, hackable LoRaWAN station that can be integrated into custom gateways, test setups, or educational projects.

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

- A **Linux** environment (PC or cross-compilation toolchain)
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
