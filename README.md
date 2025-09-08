# Pico RTC Display Test

A demonstration project showing how to use the [rpi-pico-libraries](https://github.com/mcanyucel/rpi-pico-libraries) collection with a Raspberry Pi Pico. This test displays real-time clock data from a DS3231 RTC on an SSD1306 OLED screen.

## What This Test Demonstrates

- **Dual I2C Operation**: Using two I2C ports simultaneously (i2c0 + i2c1)
- **Real-time Display**: Live clock showing time, date, temperature, and weekday
- **Library Integration**: Professional use of external libraries via Git submodules
- **Error Handling**: Proper initialization and error checking
- **Buffer Management**: Efficient OLED buffer operations for smooth updates

## Demo Output

The OLED display shows:
```
15:30:45        ← Current time (HH:MM:SS)
08/09/2025      ← Current date (DD/MM/YYYY)  
23.5C           ← Temperature from DS3231
MON             ← Day of week
RTC + OLED OK   ← Status message
```

Serial output provides debug information and status updates.

## Hardware Requirements

### Required Components
- **Raspberry Pi Pico** (or compatible RP2040 board)
- **SSD1306 OLED Display** (128x64, I2C)
- **DS3231 RTC Module** (with CR2032 battery)
- **Breadboard and jumper wires**

### Wiring Connections

#### SSD1306 OLED Display (I2C0)
| SSD1306 Pin | Pico Pin | Description |
|-------------|----------|-------------|
| VCC | 3V3 | Power (3.3V) |
| GND | GND | Ground |
| SDA | GPIO 16 | I2C Data |
| SCL | GPIO 17 | I2C Clock |

#### DS3231 RTC Module (I2C1)
| DS3231 Pin | Pico Pin | Description |
|------------|----------|-------------|
| VCC | 3V3 | Power (3.3V) |
| GND | GND | Ground |
| SDA | GPIO 18 | I2C Data |
| SCL | GPIO 19 | I2C Clock |
| INT/SQW | GPIO 5 | Interrupt (optional) |

## Quick Setup

### 1. Clone This Test Project

```bash
git clone https://github.com/yourusername/pico-rtc-display-test.git
cd pico-rtc-display-test
```

### 2. Add the Libraries as Submodule

```bash
# Add the library collection
git submodule add https://github.com/mcanyucel/rpi-pico-libraries.git libs

# Initialize submodule
git submodule update --init --recursive
```

### 3. Set Up Pico SDK

Make sure you have the Pico SDK installed and the `PICO_SDK_PATH` environment variable set:

```bash
export PICO_SDK_PATH=/path/to/pico-sdk
```

Copy `pico_sdk_import.cmake` from the Pico SDK to your project root.

### 4. Build the Project

```bash
mkdir build
cd build
cmake ..
make
```

### 5. Flash to Pico

1. Hold the BOOTSEL button while connecting the Pico to USB
2. Copy `pico_rtc_display_test.uf2` to the mounted drive
3. The Pico will reset and start running the test

## Project Structure

```
pico-rtc-display-test/
├── CMakeLists.txt          ← Main build configuration
├── pico_sdk_import.cmake   ← Pico SDK import script
├── src/
│   └── main.c              ← Test application code
├── libs/                   ← Git submodule
│   ├── ssd1306/            ← OLED display library
│   ├── ds3231/             ← RTC library
│   ├── ads1115/            ← ADC library (not used in this test)
│   ├── sh1106/             ← Alternative OLED library
│   └── sdcard/             ← SD card library (not used)
├── build/                  ← Build output directory
└── README.md              ← This file
```

## Customization

### Setting Initial Time

If your DS3231 doesn't have the correct time, uncomment and modify this section in `main.c`:

```c
// Uncomment and adjust this section in main.c
ds3231_datetime_t init_datetime = {
    .date = {
        .year = 25,      // 2025 (years since 2000)
        .month = 9,      // September
        .day = 8,        // 8th
        .weekday = 1     // Monday (1=Mon, 7=Sun)
    },
    .time = {
        .hours = 15,     // 3 PM
        .minutes = 30,   // 30 minutes
        .seconds = 0     // 0 seconds
    }
};
if (ds3231_set_datetime(&init_datetime)) {
    printf("Initial time set successfully\n");
}
```

### Changing I2C Pins

The libraries use predefined pins, but you can modify them by editing the header files or using `#define` overrides:

```c
// In your main.c, before including headers
#define SSD1306_I2C_SDA_PIN 20
#define SSD1306_I2C_SCL_PIN 21
#define DS3231_SDA_PIN 22
#define DS3231_SCL_PIN 23

#include "ssd1306_i2c.h"
#include "ds3231.h"
```

## Troubleshooting

### Common Issues

**"RTC ERROR!" on display:**
- Check DS3231 wiring connections
- Verify I2C1 pins (GPIO 18, 19)
- Ensure DS3231 has power (3.3V)

**Blank OLED display:**
- Check SSD1306 wiring connections
- Verify I2C0 pins (GPIO 16, 17)  
- Try different I2C address (0x3D instead of 0x3C)

**Build errors:**
- Ensure `PICO_SDK_PATH` is set correctly
- Check that `pico_sdk_import.cmake` is in project root
- Verify submodule is properly initialized

**Time not accurate:**
- DS3231 needs CR2032 battery for time keeping
- Set initial time using the code section above
- Check that DS3231 is genuine (many clones have issues)

### Debug Output

Connect to the Pico's USB serial port to see debug messages:

```bash
# On Linux/macOS
screen /dev/ttyACM0 115200

# On Windows
# Use PuTTY or similar terminal program
```

Expected output:
```
=== Pico Library Test: SSD1306 + DS3231 ===
I2C0 initialized for SSD1306 on pins 16,17
I2C1 initialized for DS3231 on pins 18,19
SSD1306 initialized successfully
DS3231 initialized and detected successfully
Starting main loop...
Time: 15:30:45 Date: 08/09/25
Temperature: 23.5°C
Display updated - Loop #0
```

## Creating Your Own Project

Use this as a template for your own projects:

1. **Copy the CMakeLists.txt** structure
2. **Add the libraries submodule** to your project
3. **Include only the libraries you need** in CMake
4. **Follow the dual I2C pattern** if using multiple I2C devices

### Example: Adding ADS1115 ADC

```cmake
# Add to CMakeLists.txt
add_subdirectory(libs/ads1115)

target_link_libraries(my_project
    ssd1306
    ds3231
    ads1115  # Add this line
    pico_stdlib
)
```

```c
// Add to your main.c
#include "ads1115.h"

// In main()
ads1115_init();
int16_t sensor_value = ads1115_read_channel(0);
```

## Learn More

- **Library Documentation**: See the [main library repository](https://github.com/mcanyucel/rpi-pico-libraries)
- **Pico SDK Guide**: [Official Pico SDK Documentation](https://datasheets.raspberrypi.org/pico/raspberry-pi-pico-c-sdk.pdf)
- **I2C Tutorial**: [I2C Communication on Pico](https://www.raspberrypi.org/documentation/microcontrollers/raspberry-pi-pico.html)

## License

This test project is released under the MIT License. See individual library licenses in the submodule.

---