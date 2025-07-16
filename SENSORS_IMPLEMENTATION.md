# HDC1080 and ADXL345 Sensor Implementation

## Overview
I've successfully added HDC1080 (temperature/humidity sensor) and ADXL345 (3-axis accelerometer) support to your STM32H563 embedded system. Both sensors communicate via I2C using the existing I2C1 interface.

## Files Created/Modified

### New Files Created:
1. **Core/Inc/hdc1080.h** - HDC1080 sensor header file
2. **Core/Src/hdc1080.c** - HDC1080 sensor implementation
3. **Core/Inc/adxl345.h** - ADXL345 sensor header file
4. **Core/Src/adxl345.c** - ADXL345 sensor implementation
5. **Core/Inc/sensors_monitor.h** - Unified sensors monitoring system
6. **Core/Src/sensors_monitor.c** - Sensors monitoring implementation

### Modified Files:
1. **Core/Src/main.c** - Added sensor includes
2. **Core/Src/system_config.c** - Added sensor initialization and command registration
3. **Core/Inc/display_module.h** - Added new display screens
4. **Core/Src/display_module.c** - Added sensor display functions

## HDC1080 Temperature/Humidity Sensor Features

### Key Features:
- **Temperature**: ±0.2°C accuracy, range -20°C to +85°C
- **Humidity**: ±2% accuracy, range 0% to 100% RH
- **Resolution**: Configurable 14-bit, 11-bit, or 8-bit
- **Built-in heater**: For condensation removal
- **Low power**: Multiple power modes

### I2C Configuration:
- **Address**: 0x40 (7-bit)
- **Interface**: I2C1
- **Speed**: Standard (100kHz) or Fast (400kHz)

### Shell Commands:
- `temp` - Quick temperature/humidity reading
- `hdc1080` - Basic sensor reading
- `hdc1080_status` - Detailed sensor status
- `hdc1080_config` - Configuration options:
  - `hdc1080_config reset` - Software reset
  - `hdc1080_config heater <on|off>` - Control heater
  - `hdc1080_config resolution <temp> <humi>` - Set resolution
- `hdc1080_monitor <on|off> [interval]` - Auto-monitoring control

## ADXL345 3-Axis Accelerometer Features

### Key Features:
- **Range**: ±2g, ±4g, ±8g, ±16g (configurable)
- **Resolution**: 10-bit to 13-bit
- **Data Rate**: 0.1Hz to 3200Hz
- **Motion Detection**: Activity, inactivity, free fall, tap detection
- **Interrupts**: Configurable interrupt sources

### I2C Configuration:
- **Address**: 0x53 (7-bit)
- **Interface**: I2C1
- **Speed**: Standard (100kHz) or Fast (400kHz)

### Shell Commands:
- `accel` - Quick acceleration reading
- `adxl345` - Basic sensor reading
- `adxl345_status` - Detailed sensor status
- `adxl345_config` - Configuration options:
  - `adxl345_config range <2|4|8|16>` - Set measurement range
  - `adxl345_config rate <value>` - Set data rate
  - `adxl345_config power <value>` - Set power mode
- `adxl345_monitor <on|off> [interval]` - Auto-monitoring control
- `adxl345_motion` - Motion detection configuration:
  - `adxl345_motion status` - Show motion status
  - `adxl345_motion activity <threshold>` - Set activity threshold
  - `adxl345_motion inactivity <threshold> <time>` - Set inactivity detection
  - `adxl345_motion freefall <threshold> <time>` - Set free fall detection
  - `adxl345_motion tap <threshold>` - Set tap detection
- `adxl345_calibrate` - Calibrate sensor (place on level surface)

## Unified Sensors Monitor System

### Features:
- **Global Control**: Enable/disable monitoring for all sensors
- **Individual Control**: Each sensor can be configured independently
- **Automatic Updates**: Respects each sensor's measurement interval
- **Statistics**: Tracks total readings and timing

### Shell Commands:
- `sensors` - Read all sensors at once
- `sensors_monitor <on|off|status>` - Control global monitoring
- `sensors_status` - Show all sensors status

## Display Integration

### New Display Screens:
- **HDC1080 Screen**: Shows temperature, humidity, and sensor status
- **ADXL345 Screen**: Shows X/Y/Z acceleration, magnitude, and status

### Navigation:
- Use `screen next` to cycle through display screens
- Display automatically updates based on DISPLAY_UPDATE_INTERVAL

## Hardware Connections

### HDC1080 Connections:
```
HDC1080 Pin    STM32H563 Pin    Function
VDD           3.3V              Power
GND           GND               Ground
SDA           PB9 (I2C1_SDA)    I2C Data
SCL           PB8 (I2C1_SCL)    I2C Clock
```

### ADXL345 Connections:
```
ADXL345 Pin    STM32H563 Pin    Function
VDD           3.3V              Power
GND           GND               Ground
SDA           PB9 (I2C1_SDA)    I2C Data
SCL           PB8 (I2C1_SCL)    I2C Clock
SDO           GND               I2C Address Select (0x53)
CS            3.3V              Chip Select (I2C mode)
```

## Usage Examples

### Basic Usage:
```bash
# Quick readings
temp                    # Read temperature/humidity
accel                   # Read acceleration

# Detailed status
hdc1080_status         # HDC1080 detailed info
adxl345_status         # ADXL345 detailed info

# Read all sensors
sensors                # All sensors at once
```

### Configuration:
```bash
# Configure HDC1080
hdc1080_config heater on           # Enable heater
hdc1080_config resolution 14 14    # Set 14-bit resolution

# Configure ADXL345
adxl345_config range 4             # Set ±4g range
adxl345_calibrate                  # Calibrate (on level surface)
```

### Auto-Monitoring:
```bash
# Enable auto-monitoring
sensors_monitor on                 # Enable global monitoring
hdc1080_monitor on 2000           # HDC1080 every 2 seconds
adxl345_monitor on 100            # ADXL345 every 100ms

# Check status
sensors_status                    # Show all sensors status
```

### Motion Detection:
```bash
# Configure motion detection
adxl345_motion activity 0.5       # Activity threshold 0.5g
adxl345_motion freefall 0.3 10    # Free fall: 0.3g, 50ms
adxl345_motion tap 3.0             # Tap threshold 3.0g
adxl345_motion status              # Show motion status
```

## Error Handling

### Sensor Availability:
- Both sensors check for proper device ID on initialization
- Commands gracefully handle sensor unavailability
- Display shows "Not Available" for disconnected sensors

### I2C Communication:
- Timeout handling for I2C operations
- Proper error reporting via shell commands
- Automatic retry mechanisms where appropriate

## Performance Considerations

### Memory Usage:
- HDC1080: ~80 bytes for sensor context
- ADXL345: ~120 bytes for sensor context
- Sensors Monitor: ~32 bytes for monitoring context

### Timing:
- HDC1080: 15ms conversion time for both measurements
- ADXL345: Real-time data available
- Auto-monitoring respects configured intervals

## Integration with Existing System

### Shell Integration:
- All new commands follow existing privilege system
- Consistent command syntax and help system
- Color-coded output for better readability

### Display Integration:
- New screens integrate with existing screen cycling
- Consistent display format and updating
- Automatic sensor updates on display

### System Integration:
- Sensors initialized during system startup
- Auto-monitoring integrated with main system loop
- Proper error handling and status reporting

This implementation provides a comprehensive sensor suite with professional-grade features, extensive configuration options, and seamless integration with your existing STM32H563 system.
