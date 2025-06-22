# Inazuma Eleven Go Light/Shadow Modding Template

## Overview

This project is **not** a decompilation of Inazuma Eleven Go Light & Shadow games. Instead, it provides a comprehensive template that allows anyone to experiment with adding custom functions and features to the game.

The main goals of this project are:
- Create an accessible modding template for the community
- Document the game's internal structure and scripting systems
- Provide enhancement patches that add new functionality to the base game

## Requirements

### Essential Tools
- **Magikoopa**: This is a Magikoopa project and requires the Magikoopa application
- **Linux environment** (recommended)
  - If Linux is not available, you can use **MSYS2** as an alternative

### Required Files
Due to copyright restrictions, the following files are **not included** in this repository and must be obtained separately:
- `code.bin`
- `exheader.bin`

These files must be placed in the root directory of the project.

## Setup Instructions

1. Ensure you have Magikoopa installed and properly configured
2. Place the required `code.bin` and `exheader.bin` files in the project root
3. Open the project with Magikoopa
4. Run **Make and Insert** to build the modified game

## Game Specifications

Inazuma Eleven Go Light/Shadow uses two distinct scripting formats for event handling:

### .xq Format
- Used primarily for menu systems
- **Currently undocumented** (documentation in progress)

### Squirrel Scripts
- Used for game events and logic
- The game runs on **Squirrel 2.2.3 Stable Version**
- This scripting language handles most of the game's interactive elements

## Project Structure

```
projectz/
├── .vscode/                         # VS Code configuration
├── include/                         # Header files
├── clib/                            # Standard C lib implementation
├── inazuma/                         # Game-specific modules
├── squirrel-2.2.3/                  # Squirrel scripting engine
│   ├── sqstdlib/                    # Standard library headers
│   └── squirrel/                    # Core Squirrel headers
├── loader/                          # Game loader components
├── source/                          # Source code
│   ├── hook.hks                     # Hook definitions
│   └── inazuma/                     # Game modification source
│       ├── ie4sq.cpp                # Inazuma-Squirrel bridge
│       └── squirrel/                # Squirrel integration
│           └── memory_manager.cpp   # Custom Memory management
├── LICENSE                          # Project license
├── Makefile                         # Build configuration
├── README                           # Generated modified code
└── symbols.ld                       # Symbol definitions for preserved functions (linker addresses)
```

### Key Directories
- **source/**: Contains all source code modifications
- **squirrel-2.2.3/**: Squirrel scripting engine headers and libraries
- **include/**: Project header files

## Disclaimer

This project is for educational and research purposes. Use at your own risk and ensure compliance with local laws regarding game modification.
