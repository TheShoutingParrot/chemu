# chemu

[![Codacy Badge](https://api.codacy.com/project/badge/Grade/bd60fc3cea614d8a84401e5f1a735210)](https://app.codacy.com/manual/TheShoutingParrot/chemu?utm_source=github.com&utm_medium=referral&utm_content=TheShoutingParrot/chemu&utm_campaign=Badge_Grade_Dashboard)

[![GitHub](LICENSE)](https://img.shields.io/github/license/theshoutingparrot/chemu)

Chemu is a CHIP-8 emulator that is written in C and uses the SDL2 library.

## Usage

Using chemu is very easy, you only need to specify the (CHIP-8) rom you want to run. E.g. ``` chemu rom.ch8 ```

### Keybindings

CHIP-8 uses a hex keyboard. This means that input is done with the 0-9 and A-F keys on your keyboard.

## Installation

To install this program you must simply run ``` make install ``` which will install it to the ``` $(PREFIX)/bin/ ``` directory.

## Current progress

Chemu, is still very early in development. Some games may not work perfectly (such as tetris) but most games should be working. I still don't know why some of these games don't work but in the coming days these bugs should be fixed.

- [x] All CHIP-8 instructions/opcodes emulated

  - [ ] All CHIP-8 games working

- [ ] All SCHIP instructions/opcodes emulated

  - [ ] All SCHIP games working

- [ ] Configurable keyboard layout

- [ ] Simple graphical debugger


## What is CHIP-8

"CHIP-8 is an interpreted programming language, developed by Joseph Weisbecker. (...) CHIP-8 programs are run on a CHIP-8 virtual machine."

--[Wikipedia EN: CHIP-8](https://en.wikipedia.org/wiki/CHIP-8)

As Wikipedia explains, CHIP-8 is an interpreted programming language which runs on a virtual machine. CHIP-8 is usually implemented as an emulator, though some pure hardware implementations also exist. 

The CHIP-8 virtual machine consists of:
  - four kilobytes of memory
  - 16 8-bit general purpose registers V0-VF
    - VF is also used as a status register
  - An index register (I) 
  - a simple stack which stores return addresses
    - This stack uses a special stack pointer register that isn't directly accessible to the user
  - a delay and sound timer
    - when the sound timer is not zero it makes a beeping sound
  - a 64 x 32 resolution monochrome screen
  - 35 opcodes
  - hex keyboard input

### Why make a CHIP-8 emulator

Though creating a CHIP-8 emulator is not very practical, CHIP-8 is a great system to start learning about emulation and generally learning about computer hardware..

Things you may learn when making a CHIP-8 emulator are:

  - The basics of emulation
  - Key concepts in computing
    - Timers
    - Stacks
    - Registers
    - Memory
    - How are subroutines called

Even though CHIP-8 was never used on anything (on a real computer or console) it still has a suprising amount of games ported to it such as Pong, Tetris, Space Invaders, etc.
