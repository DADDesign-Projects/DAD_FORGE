[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
![Platform](https://img.shields.io/badge/Platform-Software-orange)
![MCU](https://img.shields.io/badge/MCU-STM32H7-brightgreen)
![Languages](https://img.shields.io/badge/Languages-C%20%7C%20C++-yellow)
![IDE](https://img.shields.io/badge/IDE-STM32CubeIDE-lightgrey)
![Status](https://img.shields.io/badge/Status-Mature%20%7C%20Active%20Development-success)
```
                           ███████╗  ██████╗   ██████╗    ██████╗   ███████╗
                           ██╔════╝ ██╔═══██╗  ██╔══██╗  ██╔════╝   ██╔════╝ 
                           █████╗   ██║   ██║  ██████╔╝  ██║  ███╗  █████╗  
                           ██╔══╝   ██║   ██║  ██╔══██╗  ██║   ██║  ██╔══╝  
                           ██║      ╚██████╔╝  ██║  ██║  ╚██████╔╝  ███████╗
                           ╚═╝       ╚═════╝   ╚═╝  ╚═╝   ╚═════╝   ╚══════╝

                    ██████╗  ██████╗   ██████╗      ██╗  ███████╗  ██████╗ ████████╗
                    ██╔══██╗ ██╔══██╗ ██╔═══██╗     ██║  ██╔════╝ ██╔════╝ ╚══██╔══╝
                    ██████╔╝ ██████╔╝ ██║   ██║     ██║  █████╗   ██║         ██║   
                    ██╔═══╝  ██╔══██╗ ██║   ██║ ██   ██║ ██╔══╝   ██║         ██║   
                    ██║      ██║  ██║ ╚██████╔╝ ╚█████╔╝ ███████╗ ╚██████╗    ██║   
                    ╚═╝      ╚═╝  ╚═╝  ╚═════╝   ╚════╝  ╚══════╝  ╚═════╝    ╚═╝   

              🔥 Open Source Hardware and Software Platform for Digital Audio Effects 🔊
```
# 🛠️ FORGE Project                 
**Welcome to the FORGE Project!**  
**FORGE** (**F**ramework for **O**pen and **R**esourceful **G**uitar **E**ffects) is a software platform designed to empower creators to build their own digital effects pedals for guitar, voice, keyboards, and more.  
  
**The goal** of this project is to provide a solid and reliable foundation that handles the full complexity of embedded audio software development, covering both driver-level and middleware-level layers.  
  
**FORGE manages** all the hardware interfaces—microcontroller, audio CODEC, encoders, switches, memory devices, TFT display, USB—as well as the software modules needed for the display, user interface, parameter persistence, filter libraries, delay lines, DCO, and more.

This leaves you free to focus on what truly matters: **designing and creating the digital effects you’ve always dreamed of**.  

**FORGE** runs on the **OSCAR hardware platform** (https://github.com/DADDesign-Projects/OSCAR_P01A01)  
  
The framework is **now mature** and fully operational: it already includes a **delay**, a **tremolo/vibrato**, and a **chorus** effect.

Development is actively ongoing to add new features, introduce new audio effects, and bring FORGE to additional hardware platforms.

# 📁 Project Structure
The repository is organized as follows:
   * **DSP/**: DSP components (audio processing, filters, oscillators, etc.)
   * **Drivers/**: hardware drivers (audio CODEC, GPIO, memory devices, etc.)
   * **Effects/**: effect implementations (delay, distortion, modulation, etc.)
   * **GUI/**: graphical user interface (menus, widgets, UI panels)
   * **PersistentStorage/**: management of parameter saving/loading in non-volatile memory
   * **STM_GFX2/**: graphics driver and library (ST7789, etc.)
   * **Utilities/**: shared utility functions (monitoring, logging, conversions, helpers)
