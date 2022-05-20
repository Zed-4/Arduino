/*  Project by: Abtin Ortgoli
    Course: CSCE 3612
*/

#include <TFT_eSPI.h>
#include <SPI.h>
#include"LIS3DHTR.h"

#define LCD_BACKLIGHT (72Ul)
#define TFT_GREY 0x5AEB
#define BUZZER_DURATION 100
#define BLUE_LED_DURATION 100
#define ACCLRM_PROCESS_INTERVAL 100
#define INTERRUPT_DEBOUCE 100
#define LCD_SLEEP_TIMEOUT 5000
#define ACCLRM_SLEEP_THRESHOLD 0.05
#define ACCLRM_X_WAKE_THRESHOLD_MIN 0.39
#define ACCLRM_X_WAKE_THRESHOLD_MAX 0.48

TFT_eSPI tft = TFT_eSPI();
LIS3DHTR<TwoWire> acclrm;

uint32_t digital_clock_should_update_at = 0;
uint32_t acclrm_values_should_processed_at = 0;
uint32_t lcd_backlight_should_turn_off_at = 0;

uint32_t buzzing_started_at = 0;
uint32_t blue_led_turned_on_at = 0;

uint32_t wio_a_interrupted_at = 0;
uint32_t wio_b_interrupted_at = 0;

double acclrm_x_last = 0;
double acclrm_y_last = 0;

bool should_force_redraw_digital_clock = false;

static uint8_t conv2d(const char* p);

uint8_t hh = conv2d(__TIME__);
uint8_t mm = conv2d(__TIME__ + 3);
uint8_t ss = conv2d(__TIME__ + 6);

// set the time to anything desire
//uint8_t hh = 00 ;
//uint8_t mm = 00;
//uint8_t ss = 00;

unsigned int colour = 0;

void setup(void) {
    Serial.begin(9600);

    tft.init();
    tft.setRotation(3);
    tft.setTextSize(6);
    tft.setTextColor(TFT_BLACK, TFT_GREEN);
    tft.fillScreen(TFT_GREEN);
    tft.setTextColor(TFT_BLACK);

    acclrm.begin(Wire1);

    if (!acclrm) {
        while(1);
    }

    acclrm.setOutputDataRate(LIS3DHTR_DATARATE_25HZ);
    acclrm.setFullScaleRange(LIS3DHTR_RANGE_2G);

    digital_clock_should_update_at = millis() + 1000;

    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(WIO_BUZZER, OUTPUT);
    pinMode(WIO_KEY_A, INPUT);

    digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(LCD_BACKLIGHT, HIGH);

    attachInterrupt(digitalPinToInterrupt(WIO_KEY_A), wio_key_a_handler, RISING);
    attachInterrupt(digitalPinToInterrupt(WIO_KEY_B), wio_key_b_handler, RISING);
}

void wio_key_a_handler() {
    if (wio_a_interrupted_at + INTERRUPT_DEBOUCE < millis()) {
        wio_a_interrupted_at = millis();
        should_force_redraw_digital_clock = true;
        change_minute();
        turn_buzzer_on();
    }
}

void wio_key_b_handler() {
    if (wio_b_interrupted_at + INTERRUPT_DEBOUCE < millis()) {
        wio_b_interrupted_at = millis();
        should_force_redraw_digital_clock = true;
        change_hour();
        turn_buzzer_on();
    }
}

void loop() {
  
    if (should_force_redraw_digital_clock == true) {
        should_force_redraw_digital_clock = false;
        draw_digital_clock_v2();
    }

    watch_buzzer();
    watch_blue_led();
    watch_lcd();
    process_acclrm_values();
    update_and_draw_digital_clock();
}

void watch_buzzer() {
    if (buzzing_started_at == 0) {
        return;
    }

    if (buzzing_started_at + BUZZER_DURATION < millis()) {
        turn_buzzer_off();
    }
}

void watch_blue_led() {
    if (blue_led_turned_on_at == 0) {
        return;
    }

    if (blue_led_turned_on_at + BLUE_LED_DURATION < millis()) {
        turn_blue_led_off();
    }
}

void watch_lcd() {
    if (lcd_backlight_should_turn_off_at == 0) {
        return;
    }

    if (lcd_backlight_should_turn_off_at < millis()) {
        lcd_turn_off();
    }
}

void turn_blue_led_on() {
    blue_led_turned_on_at = millis();
    digitalWrite(LED_BUILTIN, HIGH);
}

void turn_blue_led_off() {
    blue_led_turned_on_at = 0;
    digitalWrite(LED_BUILTIN, LOW);
}

void turn_buzzer_on() {
    buzzing_started_at = millis();
    analogWrite(WIO_BUZZER, 128);
}

void turn_buzzer_off() {
    buzzing_started_at = 0;
    analogWrite(WIO_BUZZER, 0);
}

void schedule_lcd_turn_off() {
    lcd_backlight_should_turn_off_at = millis() + LCD_SLEEP_TIMEOUT;
}

void cancel_schedule_lcd_turn_off() {
    lcd_backlight_should_turn_off_at = 0;
}

void lcd_turn_on() {
    digitalWrite(LCD_BACKLIGHT, HIGH);
}

void lcd_turn_off() {
    digitalWrite(LCD_BACKLIGHT, LOW);
}

bool isLcdOn() {
    return digitalRead(LCD_BACKLIGHT) == HIGH;
}

bool isLcdOffScheduled() {
    return lcd_backlight_should_turn_off_at != 0;
}

void process_acclrm_values() {
    if (acclrm_values_should_processed_at < millis()) {
        acclrm_values_should_processed_at = millis() + ACCLRM_PROCESS_INTERVAL;

        double acclrm_x_current = acclrm.getAccelerationX();
        double acclrm_y_current = acclrm.getAccelerationY();

        double acclrm_x_delta = abs(acclrm_x_current - acclrm_x_last);
        double acclrm_y_delta = abs(acclrm_y_current - acclrm_y_last);

        if (acclrm_x_current > ACCLRM_X_WAKE_THRESHOLD_MIN && acclrm_x_current < ACCLRM_X_WAKE_THRESHOLD_MAX) {
            if (isLcdOffScheduled()) {
                cancel_schedule_lcd_turn_off();
            }

            if (!isLcdOn()) {
                lcd_turn_on();
            }
        } else if (acclrm_x_delta < ACCLRM_SLEEP_THRESHOLD && acclrm_y_delta < ACCLRM_SLEEP_THRESHOLD) {
            if (isLcdOn() && !isLcdOffScheduled()) {
                schedule_lcd_turn_off();
            }
        }

        acclrm_x_last = acclrm_x_current;
        acclrm_y_last = acclrm_y_current;
    }
}

void change_hour() {
    hh++;

    if (hh > 23) {
        hh = 0;
    }
}

void change_minute() {
    mm++;

    if (mm > 59) {
        mm = 0;
    }
}

void update_and_draw_digital_clock() {
    if (digital_clock_should_update_at < millis()) {
        digital_clock_should_update_at = millis() + 1000;

        turn_blue_led_on();

        ss++;

        if (ss == 60) {
            ss = 0;
            mm++;

            if (mm > 59) {
                mm = 0;
                hh++;

                if (hh > 23) {
                    hh = 0;
                }
            }
        }

        draw_digital_clock_v2();
    }
}

void draw_digital_clock_v2() {
    tft.fillScreen(TFT_GREEN);
    tft.setCursor(18,97.6);

    if (hh < 10) {
        tft.print("0");
    }

    tft.printf("%d", hh);

    tft.print(":");

    if (mm < 10) {
        tft.print("0");
    }

    tft.printf("%d", mm);

    tft.print(":");

    if (ss < 10) {
        tft.print("0");
    }

    tft.printf("%d", ss);
}

// function to extract numbers from compile time string
static uint8_t conv2d(const char* p) {
    uint8_t v = 0;

    if ('0' <= *p && *p <= '9') {
        v = *p - '0';
    }

    return 10 * v + *++p - '0';
}
