[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg?style=flat-square)](https://opensource.org/licenses/Apache-2.0)
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

# 🛠️ Development Tools
FORGE is developed using STM32CubeIDE, the development environment and IDE provided by STMicroelectronics (https://www.st.com/en/development-tools/stm32cubeide.html).  
The framework must be integrated as a Git submodule within a main project, allowing easy updates and seamless reuse across multiple projects.

# 🚀 Getting Started

## 1. Clone the OSCAR hardware project

```bash
git clone --recurse-submodules https://github.com/DADDesign-Projects/OSCAR_P01A01.git
```

## 2. Create a new audio effect
Duplicate the template folder: OSCAR_P01A01/DAD_FORGE/Effects/Template
Rename the folder to match your new effect name.
Edit the source file: Src/Template.cpp to implement your own audio processing.

## 3. Register your effect
add your effet in the configuration file: OSCAR_P01A01/DAD_FORGE/Effects/@Config/EffectsConfig.h
Your effect will then be integrated into the FORGE framework.
    
# 📬 Contact
Feel free to contact me for any questions, feedback, improvement suggestions, or collaboration proposals related to the FORGE framework or the OSCAR hardware platform.
I am always open to discussion and community contributions (daddesign.projects@gmail.com).  
  
---


