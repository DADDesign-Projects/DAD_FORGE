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

Welcome to the **FORGE** project!

**FORGE** (**F**ramework for **O**pen and **R**esourceful **G**uitar **E**ffects) is a software framework for **STM32** microcontrollers designed to help developers build their own professional digital audio effects for guitars, basses, vocals, keyboards, and many other audio applications.

The goal of FORGE is to provide a robust, efficient, and reliable software foundation that handles the complexity of embedded audio development. It covers everything from low-level hardware drivers to middleware and high-level application components, allowing developers to focus on designing great audio effects instead of infrastructure.

The framework includes all the building blocks required to create professional digital effects:

* Hardware drivers (CODECs, GPIO, SDRAM, QSPI Flash, timers, etc.)
* **DAD_GFX2** graphics library (ST7789 and other LCD controllers)
* DSP components (filters, oscillators, delay lines, mixers, envelopes, and more)
* Graphical user interface (menus, widgets, control panels, VU meters, etc.)
* **PersistentStorage** for preset management in non-volatile memory
* Utilities (monitoring, debugging helpers, macros, and more)
* Development and testing tools (MIDI controller, BIQUAD filter designer, etc.)

FORGE also includes several fully featured, production-quality audio effects. They can be used as-is, customized to fit your own projects, or studied as reference implementations to learn embedded DSP programming on STM32.

The currently available effects include:

* Reverb
* Delay
* Chorus
* Flanger
* Phaser
* Vibrato
* Tremolo
* ...with many more to come.

FORGE has already been deployed on the following hardware platforms:

* **OSCAR**: https://github.com/DADDesign-Projects/OSCAR_P01A01
* **PENDA**: https://github.com/DADDesign-Projects/PENDA_01

# 🛠️ Development Tools

FORGE is developed using **STM32CubeIDE**, the official development environment provided by STMicroelectronics.

The framework is intended to be integrated into a project as a **Git submodule**, making it easy to maintain, update, and reuse across multiple applications.

# 📬 Contact

Feel free to contact me if you have any questions, feedback, improvement suggestions, or collaboration ideas regarding the FORGE framework or the OSCAR hardware platform.

Community contributions are always welcome.

📧 [daddesign.projects@gmail.com](mailto:daddesign.projects@gmail.com)

🎛️🎶 Follow all the latest news, project progress, and demonstrations on my Facebook page!

https://www.facebook.com/people/DADDesign-Projects/61583645957068/

