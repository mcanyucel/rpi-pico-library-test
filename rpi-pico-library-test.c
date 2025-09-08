#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"

// Include your libraries
#include "ssd1306_i2c.h"
#include "ds3231.h"

// Display buffer for SSD1306
static uint8_t display_buf[SSD1306_BUF_LEN];

void setup_i2c_ports() {
    // Initialize I2C0 for SSD1306 (pins 16,17)
    i2c_init(i2c0, SSD1306_I2C_CLK * 1000); // Convert kHz to Hz
    gpio_set_function(SSD1306_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SSD1306_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SSD1306_I2C_SDA_PIN);
    gpio_pull_up(SSD1306_I2C_SCL_PIN);
    
    // Initialize I2C1 for DS3231 (pins 18,19)
    i2c_init(DS3231_I2C_PORT, 100 * 1000); // 100kHz for DS3231
    gpio_set_function(DS3231_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(DS3231_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(DS3231_SDA_PIN);
    gpio_pull_up(DS3231_SCL_PIN);
    
    printf("I2C0 initialized for SSD1306 on pins %d,%d\n", 
           SSD1306_I2C_SDA_PIN, SSD1306_I2C_SCL_PIN);
    printf("I2C1 initialized for DS3231 on pins %d,%d\n", 
           DS3231_SDA_PIN, DS3231_SCL_PIN);
}

void display_time_and_temp() {
    ds3231_datetime_t datetime;
    float temperature;
    
    // Clear display buffer
    SSD1306_CLEAR_BUFFER(display_buf);
    
    // Read current date/time from DS3231
    if (ds3231_read_datetime(&datetime)) {
        printf("Time: %02d:%02d:%02d Date: %02d/%02d/%02d\n", 
               datetime.time.hours, datetime.time.minutes, datetime.time.seconds,
               datetime.date.day, datetime.date.month, datetime.date.year);
        
        // Display time (24-hour format)
        char time_str[16];
        sprintf(time_str, "%02d:%02d:%02d", 
                datetime.time.hours, datetime.time.minutes, datetime.time.seconds);
        WriteString(display_buf, 0, 0, time_str);
        
        // Display date
        char date_str[16];
        sprintf(date_str, "%02d/%02d/20%02d", 
                datetime.date.day, datetime.date.month, datetime.date.year);
        WriteString(display_buf, 0, 12, date_str);
        
    } else {
        printf("ERROR: Failed to read time from DS3231\n");
        WriteString(display_buf, 0, 0, "RTC ERROR!");
    }
    
    // Read temperature from DS3231
    if (ds3231_read_temperature(&temperature)) {
        printf("Temperature: %.1f°C\n", temperature);
        
        char temp_str[16];
        sprintf(temp_str, "%.1fC", temperature);
        WriteString(display_buf, 0, 24, temp_str);
    } else {
        WriteString(display_buf, 0, 24, "TEMP ERR");
    }
    
    // Display weekday
    const char* weekdays[] = {"", "MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN"};
    if (datetime.date.weekday >= 1 && datetime.date.weekday <= 7) {
        WriteString(display_buf, 0, 36, (char*)weekdays[datetime.date.weekday]);
    }
    
    // Status line
    WriteString(display_buf, 0, 48, "RTC + OLED OK");
    
    // Render to display
    struct render_area full_screen = SSD1306_FULL_SCREEN_AREA();
    calc_render_area_buflen(&full_screen);
    render(display_buf, &full_screen);
}

int main() {
    // Initialize stdio
    stdio_init_all();
    
    // Wait for USB connection
    sleep_ms(3000);
    printf("\n=== Pico Library Test: SSD1306 + DS3231 ===\n");
    
    // Setup both I2C ports
    setup_i2c_ports();
    
    // Initialize SSD1306 OLED
    printf("Initializing SSD1306 OLED...\n");
    SSD1306_init();
    printf("SSD1306 initialized successfully\n");
    
    // Initialize DS3231 RTC
    printf("Initializing DS3231 RTC...\n");
    if (!ds3231_init()) {
        printf("ERROR: Failed to initialize DS3231!\n");
        return -1;
    }
    
    // Check if DS3231 is present
    if (!ds3231_is_present()) {
        printf("ERROR: DS3231 not detected!\n");
        return -1;
    }
    printf("DS3231 initialized and detected successfully\n");
    
    // Set initial time if needed (uncomment and adjust if RTC needs setting)
    /*
    ds3231_datetime_t init_datetime = {
        .date = {
            .year = 25,      // 2025
            .month = 9,
            .day = 8,
            .weekday = 1     // Monday
        },
        .time = {
            .hours = 15,
            .minutes = 30,
            .seconds = 0
        }
    };
    if (ds3231_set_datetime(&init_datetime)) {
        printf("Initial time set successfully\n");
    }
    */
    
    // Show startup message on display
    SSD1306_CLEAR_BUFFER(display_buf);
    WriteCentered(display_buf, 16, "RTC Clock");
    WriteCentered(display_buf, 32, "Starting...");
    
    struct render_area full_screen = SSD1306_FULL_SCREEN_AREA();
    calc_render_area_buflen(&full_screen);
    render(display_buf, &full_screen);
    
    sleep_ms(2000);
    
    printf("Starting main loop...\n");
    
    // Main loop - update display every second
    int loop_count = 0;
    while (1) {
        display_time_and_temp();
        
        // Print status to console every 30 seconds
        if (loop_count % 30 == 0) {
            printf("Display updated - Loop #%d\n", loop_count);
        }
        
        loop_count++;
        sleep_ms(1000);  // Update every second
    }
    
    return 0;
}