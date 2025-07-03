# STM32H5xx SSD1306 OLED Display Project

A comprehensive STM32H5xx microcontroller project featuring SSD1306 OLED display integration with multiple peripheral support.

## 🚀 Project Overview

This project is designed for STM32H5xx microcontrollers (specifically tested on STM32H563xx) and provides a complete framework for:

- **SSD1306 OLED Display Control** - Full graphics and text rendering capabilities
- **Multiple Communication Interfaces** - I2C, UART, USB
- **Hardware Abstraction** - Clean peripheral management
- **Rich Font Support** - Multiple font sizes and styles

## 🛠 Hardware Requirements

### Microcontroller
- **STM32H563ZI** (or compatible STM32H5xx series)
- **Development Board**: STM32 Nucleo-H563ZI (recommended)

### External Components
- **SSD1306 OLED Display** (128x64 pixels, I2C interface)
  - VCC → 3.3V
  - GND → Ground
  - SCL → PB8 (I2C1_SCL)
  - SDA → PB9 (I2C1_SDA)
  - I2C Address: 0x3C

### Onboard Features Used
- **LEDs**: LED1 (PB0), LED2 (PF4), LED3 (PG4)
- **User Button**: PC13 (with interrupt)
- **USB**: Full-Speed USB device
- **UART**: UART4 (PD0/PD1), UART5 (PB12/PC12)
- **Debug**: SWD interface

## 📁 Project Structure

```
├── Core/
│   ├── Src/
│   │   ├── main.c                 # Main application entry point
│   │   ├── ssd1306.c             # SSD1306 OLED driver
│   │   ├── ssd1306_fonts.c       # Font definitions and data
│   │   ├── i2c.c                 # I2C peripheral configuration
│   │   ├── gpio.c                # GPIO configuration
│   │   ├── tim.c                 # Timer configuration
│   │   ├── usart.c               # UART configuration
│   │   ├── usb.c                 # USB configuration
│   │   ├── gpdma.c               # DMA configuration
│   │   └── stm32h5xx_*.c         # STM32 HAL system files
│   └── Inc/
│       ├── main.h                # Main header definitions
│       ├── ssd1306.h             # SSD1306 driver header
│       ├── ssd1306_conf.h        # SSD1306 configuration
│       ├── ssd1306_fonts.h       # Font declarations
│       └── *.h                   # Other peripheral headers
```

## ⚙️ Configuration

### SSD1306 Display Configuration
```c
// ssd1306_conf.h
#define SSD1306_USE_I2C              // Communication method
#define SSD1306_I2C_PORT    hi2c1    // I2C instance
#define SSD1306_I2C_ADDR    (0x3C << 1)  // Device address
#define SSD1306_WIDTH       128      // Display width
#define SSD1306_HEIGHT      64       // Display height

// Supported fonts
#define SSD1306_INCLUDE_FONT_6x8
#define SSD1306_INCLUDE_FONT_7x10
#define SSD1306_INCLUDE_FONT_11x18
#define SSD1306_INCLUDE_FONT_16x26
#define SSD1306_INCLUDE_FONT_16x24
#define SSD1306_INCLUDE_FONT_16x15
```

### System Configuration
- **CPU Clock**: 250 MHz (via PLL from 8 MHz HSE)
- **I2C1 Clock**: PCLK1
- **USB Clock**: HSI48 (48 MHz)
- **Debug**: SWD enabled

## 🔧 Setup and Build

### Prerequisites
- **STM32CubeIDE** or compatible ARM development environment
- **STM32CubeMX** (for configuration changes)
- **STM32 HAL Libraries** (included)

### Build Instructions
1. **Clone/Download** this project
2. **Open** in STM32CubeIDE
3. **Build** the project (Ctrl+B)
4. **Flash** to your STM32H563ZI board
5. **Connect** SSD1306 display to I2C1 pins

### Hardware Connections
```
STM32H563ZI          SSD1306 OLED
PB8 (I2C1_SCL)   →   SCL
PB9 (I2C1_SDA)   →   SDA
3.3V             →   VCC
GND              →   GND
```

## 💻 Usage

### Basic SSD1306 Operations

```c
#include "ssd1306.h"
#include "ssd1306_fonts.h"

// Initialize display
ssd1306_Init();

// Clear screen
ssd1306_Fill(Black);

// Write text
ssd1306_SetCursor(0, 0);
ssd1306_WriteString("Hello World!", Font_11x18, White);

// Draw shapes
ssd1306_DrawCircle(64, 32, 20, White);
ssd1306_DrawRectangle(10, 10, 50, 30, White);

// Update display
ssd1306_UpdateScreen();
```

### Available Fonts
- **Font_6x8** - Small, compact text
- **Font_7x10** - Slightly larger
- **Font_11x18** - Medium size
- **Font_16x26** - Large text
- **Font_16x24** - Large alternative
- **Font_16x15** - Roboto Thin (proportional)

### Graphics Functions
```c
// Pixel operations
ssd1306_DrawPixel(x, y, color);

// Lines and shapes
ssd1306_Line(x1, y1, x2, y2, color);
ssd1306_DrawCircle(x, y, radius, color);
ssd1306_FillCircle(x, y, radius, color);
ssd1306_DrawRectangle(x1, y1, x2, y2, color);
ssd1306_FillRectangle(x1, y1, x2, y2, color);

// Advanced graphics
ssd1306_DrawArc(x, y, radius, start_angle, sweep, color);
ssd1306_DrawBitmap(x, y, bitmap, width, height, color);
```

## 🔧 Peripheral Features

### I2C1
- **Pins**: PB8 (SCL), PB9 (SDA)
- **Speed**: Standard mode (100 kHz)
- **Purpose**: SSD1306 communication

### UART4 & UART5
- **UART4**: PD0 (RX), PD1 (TX)
- **UART5**: PB12 (RX), PC12 (TX)
- **Baud Rate**: 115200
- **Usage**: Serial communication, debugging

### USB Device
- **Pins**: PA11 (D-), PA12 (D+), PA8 (SOF)
- **Speed**: Full-Speed (12 Mbps)
- **Purpose**: USB device functionality

### Timers
- **TIM6 & TIM7**: Basic timers for timing operations
- **Interrupts**: Enabled for both timers

## 🐛 Troubleshooting

### Display Issues
**Problem**: Display not showing anything
- ✅ Check I2C connections (PB8/PB9)
- ✅ Verify 3.3V power supply
- ✅ Confirm I2C address (0x3C)
- ✅ Call `ssd1306_UpdateScreen()` after drawing

**Problem**: Corrupted display
- ✅ Check I2C signal integrity
- ✅ Verify ground connections
- ✅ Reduce I2C speed if needed

### Build Issues
**Problem**: Compilation errors
- ✅ Ensure all HAL modules are enabled
- ✅ Check include paths
- ✅ Verify STM32H5 family selection

## 📚 API Reference

### Core Functions
- `ssd1306_Init()` - Initialize display
- `ssd1306_Fill(color)` - Fill entire screen
- `ssd1306_UpdateScreen()` - Refresh display
- `ssd1306_SetCursor(x, y)` - Set text position

### Text Functions
- `ssd1306_WriteChar(ch, font, color)` - Write single character
- `ssd1306_WriteString(str, font, color)` - Write text string

### Graphics Functions
- `ssd1306_DrawPixel(x, y, color)` - Draw single pixel
- `ssd1306_Line(x1, y1, x2, y2, color)` - Draw line
- `ssd1306_DrawCircle(x, y, r, color)` - Draw circle outline
- `ssd1306_FillCircle(x, y, r, color)` - Draw filled circle

### Control Functions
- `ssd1306_SetContrast(value)` - Adjust display brightness
- `ssd1306_SetDisplayOn(on)` - Turn display on/off

## 📄 License

This project uses STMicroelectronics HAL libraries and includes an SSD1306 driver. Please refer to individual file headers for specific license information.



**Happy coding with STM32! 🚀**
