# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a sensorless BLDC (Brushless DC) motor controller for the STC32G12K128 microcontroller. It uses BEMF (Back Electromotive Force) zero-crossing detection for commutation timing. The project is developed by SeekFree (逐飞科技) for use in smart car competitions.

## Build System

**IDE:** Keil MDK for C251 V5.60 (or compatible version)

**Project File:** `SEEKFREE_STC32G_BLDC/Project/MDK/STC32G.uvprojx`

**Target MCU:** STC32G12K128-LQFP48

**System Clock:** 33.1776 MHz

### Build Commands

- Open the project in Keil MDK and use the build button (F7)
- Output files are generated in `SEEKFREE_STC32G_BLDC/Project/MDK/Out_File/`
- Clean temporary files: Run `MDK删除临时文件.bat` in the MDK folder

## High-Level Architecture

### Motor Control Strategy

The motor controller implements a sensorless 3-phase BLDC control with three distinct phases:

1. **Pre-positioning (预定位):** Rotor is aligned to a known position by energizing one phase pair
2. **Open-loop Startup (开环启动):** Motor is force-commutated with gradually decreasing delay until sufficient BEMF is generated
3. **Closed-loop Operation (闭环控制):** Commutation is triggered by BEMF zero-crossing detection

### Key Modules

| Module | Path | Purpose |
|--------|------|---------|
| `motor.c/h` | `bldc/` | Core motor control logic, commutation, startup sequence |
| `pwm.c/h` | `bldc/` | 6-PWM generation for 3-phase half-bridge (center-aligned) |
| `comparator.c/h` | `bldc/` | Internal comparator for BEMF zero-crossing detection |
| `pwm_input.c/h` | `bldc/` | PWM input capture for throttle control (50-300Hz, 1-2ms pulse) |
| `battery.c/h` | `bldc/` | Battery voltage monitoring and low-voltage protection |
| `pit_timer.c/h` | `bldc/` | Periodic interrupt timer for LED status and motor monitoring |

### Hardware Resources

**PWM Outputs (6-channel):**
- Phase A: P20 (H), P21 (L)
- Phase B: P22 (H), P23 (L)
- Phase C: P24 (H), P25 (L)

**BEMF Detection:** Internal comparator with multiplexed inputs (phases A, B, C)

**Input Signal:** PWM input on P32 (timer capture mode)

**ADC:** Battery voltage monitoring

### Interrupt Structure

| Interrupt | Priority | Function |
|-----------|----------|----------|
| PWMA (PWM) | High | BEMF sampling during PWM high period (startup phase only) |
| Comparator | High | Zero-crossing detection (running phase) |
| Timer 1 | Normal | 10ms periodic - LED status, motor start/stop logic |
| Timer 3 | Normal | Open-loop commutation timing during startup |
| Timer 4 | Normal | Commutation timeout detection (stall detection) |
| Timer 0 | Normal | Demagnetization delay after commutation |
| PWMB | Normal | PWM input capture for throttle |

**Critical Timing:** PWM and comparator interrupts have highest priority. PWM interrupt is disabled after successful startup to reduce CPU load; comparator interrupt takes over for commutation.

### Motor State Machine

`motor.run_flag` states:
- `0`: Stopped
- `1`: Starting (open-loop forced commutation)
- `2`: Transition phase (closed-loop with PWM interrupt monitoring)
- `3`: Running (closed-loop with comparator interrupt)

### Configuration (`bldc_config.h`)

Key tunable parameters:

```c
BLDC_MAX_DUTY               // Maximum PWM duty cycle (default: 690)
BLDC_MIN_DUTY               // Minimum duty before motor stops (default: 50)
BLDC_START_VOLTAGE          // Startup voltage factor 0-1.5V (default: 1.25)
BLDC_MIN_BATTERY            // Minimum battery voltage in V (default: 10.0)
BLDC_COMMUTATION_FAILED_MAX // Max consecutive failed commutations before stall (default: 300)
BLDC_START_DELAY            // Delay before restart after stall in 10ms units (default: 100)
BLDC_BEEP_ENABLE            // Enable startup beep (default: 1)
BLDC_BEEP_VOLUME            // Beep volume 0-100 (default: 30)
```

### LED Status Indicators

The onboard LED (P52) indicates motor state:

| Pattern | Meaning |
|---------|---------|
| 0.1s on, 0.9s off | Low battery voltage |
| 0.05s on, 0.05s off | Motor stalled |
| 1s on, 1s off | Motor stopped/idle |
| 0.5s on, 0.5s off | Open-loop startup in progress |
| 0.1s on, 0.1s off | Transition phase (stabilizing) |
| Steady on | Normal operation |

### Input Signal Protocol

The controller accepts standard servo-style PWM input:
- Frequency: 50-300 Hz
- Pulse width: 1ms (min/stop) to 2ms (max speed)
- Input pin: Same as servo connector (signal line)

### Startup Sequence Details

1. Motor coils are energized to align rotor (pre-positioning, ~800ms)
2. Timer3 begins forced commutation with decreasing delay
3. PWMA interrupt monitors BEMF for consistent zero-crossings
4. When stable BEMF detected, switch to comparator interrupt
5. PWM interrupt is disabled to free CPU resources

### Safety Features

- **Low voltage protection:** Motor stops if battery < `BLDC_MIN_BATTERY`
- **Stall detection:** Timeout-based (Timer4) and statistical (commutation time variance)
- **Automatic restart:** After stall, waits `BLDC_START_DELAY` then retries
- **Demagnetization delay:** Timer0 prevents false zero-crossing detection after commutation

## Project Structure

```
SEEKFREE_STC32G_BLDC/
├── bldc/               # BLDC motor control core
├── Libraries/
│   ├── libraries/      # STC SDK (board.c, STC32Gxx.h, etc.)
│   ├── seekfree_libraries/  # SeekFree HAL (zf_*.c/h)
│   └── seekfree_peripheral/ # Device drivers (display, sensors, etc.)
├── Project/
│   ├── CODE/           # Application code (car_ctrl, param_ctrl)
│   ├── MDK/            # Keil project files
│   └── USER/           # main.c, isr.c/h
└── version/            # Version info
```

## Important Notes

- **PWM initialization MUST be called AFTER motor_init()** - otherwise hardware damage may occur
- The internal comparator input is multiplexed between phases; software switches the input based on current commutation step
- BEMF is compared against virtual neutral point (resistor network), not ground
- Main loop primarily sends data to virtual oscilloscope via UART1 for debugging
- P54 is configured as reset pin by default; modify `board_init()` in `board.c` if P54 is needed for other purposes
