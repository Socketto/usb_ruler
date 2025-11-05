# USB-C STone Ruler – User Guide

**Device developed by the CAREL Tools Team**

---

## Overview

This electronic ruler includes a USB-C connection and a **3-position DIP switch**.  
By setting different binary combinations (**000–111**), the board performs **eight different functions**.

> ⚠️ Some modes simulate a USB keyboard or mouse.  
> Use the device only on systems where you have permission.

---

## How to Select a Function

1. Locate the **3-switch selector** on the board.
2. Set the switches to match one of the binary combinations listed below.
3. Connect the board to a PC via **USB-C**.
4. The selected function starts automatically.

---

## Function Table

| Binary (SW1–SW2–SW3) | Function |
|----------------------|----------|
| **000** | LEDs turn on in **random mode**. |
| **001** | Simulates a USB keyboard and repeatedly sends the screen-lock command. |
| **010** | Simulates a USB RS-485 transceiver (similar to the CAREL "ovetto" interface). |
| **011** | Simulates a USB mouse and keeps moving the cursor to prevent the PC from locking. |
| **100** | Enables serial communication and simulates a Modbus device. The first 8 coils correspond to the LEDs. |
| **101** | Simulates a USB keyboard and automatically types a predefined word. |
| **110** | USB keyboard mode: pressing the **CAPS LOCK** key triggers a dice roll function. |
| **111** | LEDs turn on sequentially (chasing/sequence mode). |

---

## Credits

Designed and developed by the **CAREL Tools Team**.

---
