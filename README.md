## What it does:
A bare-metal device driver for a 4x4 keypad using SysTick, debounce and a ring buffer.

## Hardware:
Programmed for the STM32F4-Discovery (STM32F407VG)

## Wiring/Pins
Pins PE8 to PE11 are used for the columns and set to input with pull up resistors.
Pins PE12 to PE15 are used for the rows and set to output.

## How it works:
Using a 1ms SysTick interrupt the program scans one row per tick, taking 4ms for the full scan. 
To ensure a stable press or release, the history of the key state is saved and if consistant for 5 samples, the key press will be send to the queue on the basis that: 1. queue is not full 2. key not being held down (software prevents this - one press -> one event).
