//***************** Device Libraries *********************//
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_AM2315.h>
#include <SPI.h>
#include "Adafruit_GFX.h"
#include "Adafruit_HX8357.h"
#include <EEPROM.h>

//*** TFT Display ***//
#define TFT_CS 10
#define TFT_DC 9
//#define TFT_RST 8
#define TFT_RST -1 // RST can be set to -1 if you tie it to Arduino's reset

//***************** Pin Definitions *********************//
#define buttonUp         A5
#define buttonDown       3
#define relayPin1        A0
#define relayPin2        A1
#define relayPin3        A2
#define relayPin4        A3
#define CONFIG_ADDRESS      0
#define CONFIG_VERSION_0    47
#define CONFIG_VERSION_1    25
#define CONFIG_VERSION_2    103
#define SAVED_UPPER_LIMIT_ADDRESS   5
#define SAVED_LOWER_LIMIT_ADDRESS   7
#define DEFAULT_UPPER_LIMIT  900
#define DEFAULT_LOWER_LIMIT  885

unsigned long Watch, _micro, time = micros();
unsigned int Clock = 0, R_clock;
boolean Reset = false, Stop = false, Paused = false;
volatile boolean timeFlag = false;

int upperLimit = DEFAULT_UPPER_LIMIT;
int lowerLimit = DEFAULT_LOWER_LIMIT;
int buttonStateUp = 0;         // variable for reading the pushbutton status
int buttonStateDown = 0;// variable for reading the pushbutton status
int selectorSwitch = 0; // variable to read slector switch for adjusting upper and lower limit
int photocellPin = A4; // the cell and 10K pulldown are connected to a0
int photocellReading; // the analog reading from the sensor divider
int buzzer = 8;

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_HX8357 tft = Adafruit_HX8357(TFT_CS, TFT_DC, TFT_RST);

// **************** COLOR DEFINITIONS *****************//

#define   BLACK           0x0000
#define   BLUE            0x001F
#define   RED             0xF800
#define   GREEN           0x07E0
#define   YELLOW          0x07FF
#define   MAGENTA         0xF81F
#define   CYAN            0xFFE0
#define   WHITE           0xFFFF
#define   DKBRED          0x0018
#define   DKGREEN         0x0600
#define   DKBLUE          0x9802
#define   ORANGE          0xF400
#define   CREAM           0xFF96
#define   BEIGE           0xFF50
#define   GREY            0xC618
#define   MATRIXBLUE      0x031F
#define   MATRIXGREEN     0x07E5

// bitmap of UK logo
PROGMEM const unsigned char uklogo[] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFC,
        0x00, 0x00, 0x7F, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFC, 0x00, 0x00, 0x64, 0x00, 0x09, 0x00, 0x90,
        0x00, 0x24, 0x00, 0x00, 0x67, 0xFF, 0xF9, 0x00, 0x9F, 0xFF, 0xE4, 0x00, 0x00, 0x67, 0xFF, 0xF9,
        0x00, 0x9F, 0xFF, 0xE4, 0x00, 0x00, 0x60, 0x7F, 0x85, 0x00, 0x81, 0xFE, 0x14, 0x00, 0x00, 0x3C,
        0x7F, 0x9F, 0x00, 0xF9, 0xFE, 0x7C, 0x00, 0x00, 0x04, 0x7F, 0x90, 0x00, 0x09, 0xFE, 0x40, 0x00,
        0x00, 0x04, 0x7F, 0x90, 0x00, 0x09, 0xFE, 0x40, 0x00, 0x00, 0x04, 0x7F, 0x90, 0x00, 0x09, 0xFE,
        0x40, 0x00, 0x00, 0x04, 0x7F, 0x90, 0x00, 0x09, 0xFE, 0x40, 0x00, 0x00, 0x04, 0x7F, 0x90, 0x00,
        0x09, 0xFE, 0x40, 0x00, 0x00, 0x04, 0x7F, 0x90, 0x00, 0x09, 0xFE, 0x40, 0x00, 0x00, 0x04, 0x7F,
        0x90, 0x00, 0x09, 0xFE, 0x40, 0x00, 0x00, 0x04, 0x7F, 0x90, 0x00, 0x09, 0xFE, 0x40, 0x00, 0x00,
        0x04, 0x7F, 0x90, 0x00, 0x09, 0xFE, 0x40, 0x00, 0x00, 0x04, 0x7F, 0x90, 0x00, 0x09, 0xFE, 0x40,
        0x00, 0x00, 0x04, 0x7F, 0x9F, 0xFF, 0xF9, 0xFE, 0x7F, 0xFF, 0xFC, 0x04, 0x7F, 0x88, 0x00, 0x11,
        0xFE, 0x3F, 0xFF, 0xFC, 0x04, 0x7F, 0x8F, 0xFF, 0xF1, 0xFE, 0x00, 0x00, 0x04, 0x04, 0x7F, 0x8F,
        0xFF, 0xF1, 0xFE, 0x3F, 0xFF, 0xE4, 0x04, 0x7F, 0x8F, 0xFF, 0xF1, 0xFE, 0x3F, 0xFF, 0xE4, 0x04,
        0x7F, 0x80, 0xFF, 0x01, 0xFE, 0x0F, 0xFF, 0x04, 0x04, 0xFF, 0xFC, 0xFF, 0x3F, 0xFE, 0x1F, 0xFC,
        0xFC, 0x06, 0x3F, 0xFC, 0xFF, 0x3F, 0xFC, 0xFF, 0xE3, 0x80, 0x03, 0x8F, 0xFC, 0xFF, 0x3F, 0xF3,
        0xFF, 0x8E, 0x00, 0x00, 0xE3, 0xFC, 0xFF, 0x3F, 0x8F, 0xFE, 0x70, 0x00, 0x00, 0x38, 0x00, 0xFF,
        0x00, 0x3F, 0xF9, 0xC0, 0x00, 0x00, 0x0F, 0xFC, 0xFF, 0x01, 0xFF, 0xE6, 0x00, 0x00, 0x00, 0x00,
        0x04, 0xFF, 0x07, 0xFF, 0xF2, 0x00, 0x00, 0x00, 0x00, 0x0C, 0xFF, 0x1F, 0xFF, 0xF9, 0x80, 0x00,
        0x00, 0x00, 0x0C, 0xFF, 0xFF, 0xE7, 0xFC, 0xC0, 0x00, 0x00, 0x00, 0x0C, 0xFF, 0xFF, 0x83, 0xFE,
        0x60, 0x00, 0x00, 0x00, 0x0C, 0xFF, 0xFE, 0x79, 0xFF, 0x30, 0x00, 0x00, 0x00, 0x0C, 0xFF, 0xF1,
        0xCC, 0xFF, 0x98, 0x00, 0x00, 0x00, 0x0C, 0xFF, 0xCE, 0x06, 0xFF, 0xCC, 0x00, 0x00, 0x00, 0x0C,
        0xFF, 0x38, 0x03, 0x7F, 0xE6, 0x00, 0x00, 0x00, 0x0C, 0xFF, 0x20, 0x01, 0xBF, 0xF3, 0x00, 0x00,
        0x00, 0x0C, 0xFF, 0x20, 0x00, 0xDF, 0xF9, 0x00, 0x00, 0x00, 0x0C, 0xFF, 0x30, 0x00, 0x6F, 0xFC,
        0xC0, 0x00, 0x00, 0x78, 0xFF, 0x3E, 0x01, 0xE7, 0xFE, 0x7C, 0x00, 0x00, 0x43, 0xFF, 0x82, 0x01,
        0x03, 0xFF, 0x04, 0x00, 0x00, 0x4F, 0xFF, 0xF2, 0x01, 0x3F, 0xFF, 0xE4, 0x00, 0x00, 0x4F, 0xFF,
        0xF2, 0x01, 0x3F, 0xFF, 0xE4, 0x00, 0x00, 0x40, 0x00, 0x02, 0x01, 0x00, 0x00, 0x04, 0x00, 0x00,
        0x7F, 0xFF, 0xFE, 0x01, 0xFF, 0xFF, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
void setup()
{
    Serial.begin(115200);
    // Enter desired countdown time here
    //*********************************************************************************************************************************************
    SetTimer(00, 00, 10); // Hours: Minutes: Seconds  *********************************************************************************************
    //*********************************************************************************************************************************************

    StartTimer();
    pinMode(buzzer, OUTPUT); //buzzer pin
    digitalWrite(buzzer, LOW);
    pinMode(relayPin1, OUTPUT); //Relay output pin
    digitalWrite(relayPin1, LOW);
    pinMode(relayPin2, OUTPUT); //Relay output pin
    digitalWrite(relayPin2, LOW);
    pinMode(relayPin3, OUTPUT); //Relay output pin
    digitalWrite(relayPin3, LOW);
    pinMode(relayPin4, OUTPUT); //Relay output pin
    digitalWrite(relayPin4, LOW);

    pinMode(buttonUp, INPUT); //push button to add value to preset Limits
    pinMode(buttonDown, INPUT); //push button to subtract value to preset Limits

    //************ Initialize The Display **************//
    tft.begin(HX8357D);
    tft.setRotation(3); //Set to landscape
    digitalWrite(relayPin1, HIGH);

    //****************PhotoSensor**************
    tft.fillScreen(BLUE);
    tft.setTextSize(4);
    tft.setCursor(105, 100);
    tft.setTextColor(WHITE);
    tft.print ("TIME SETTING");
    tft.setTextSize(2);
    tft.setCursor(5, 260);
    tft.setTextColor(WHITE);
    tft.print ("Light Parameters");
    tft.setTextSize(2);
    tft.setCursor(185, 280);
    tft.setTextColor(WHITE);
    tft.print (upperLimit);
    tft.setCursor(5, 280);
    tft.print ("Setting -");
    tft.setCursor(5, 300);
    tft.print ("Actual--------");
}

void loop()
{
    buttonStateUp = digitalRead(buttonUp);
    buttonStateDown = digitalRead(buttonDown);
    photocellReading = analogRead(photocellPin);

    tft.fillRect(182, 298, 60, 20, RED);
    tft.setCursor(185, 300);
    tft.setTextSize(2);
    tft.setTextColor(WHITE);
    tft.print (photocellReading);
    delay(500);

    if (photocellReading >= upperLimit) { //High Light setting

        //*******BUZZER*****************************************************************************************
        digitalWrite(buzzer, HIGH);
        delay(200);
        digitalWrite(buzzer, LOW);
        delay(300);
        digitalWrite(buzzer, HIGH);
        delay(200);
        digitalWrite(buzzer, LOW);
        delay(300);
        digitalWrite(buzzer, HIGH);
        delay(200);
        digitalWrite(buzzer, LOW);
        delay(300);
        digitalWrite(buzzer, HIGH);
        delay(200);
        digitalWrite(buzzer, LOW);
        delay(300);
        digitalWrite(buzzer, HIGH);
        delay(200);
        digitalWrite(buzzer, LOW);
        delay(300);
        digitalWrite(buzzer, HIGH);
        delay(200);
        digitalWrite(buzzer, LOW);
        //********************************************************************************************************
        digitalWrite(relayPin2, HIGH);
        delay(5000);
        digitalWrite(relayPin2, LOW);
    }

    CountDownTimer(); // run the timer

    // this prevents the time from being constantly shown.
    if (TimeHasChanged() )
    {
        //tft.fillScreen(BLUE);
        tft.fillRect(90, 135, 350, 60, RED);
        tft.setTextWrap(false);
        tft.setTextColor(WHITE);
        tft.setTextSize(7);
        tft.setCursor(100, 140);
        tft.print(ShowHours());
        tft.print(":");
        tft.print(ShowMinutes());
        tft.print(":");
        tft.println(ShowSeconds());
        //Serial.println(":");
        //Serial.print(ShowMilliSeconds());
        //Serial.print(":");
        //Serial.println(ShowMicroSeconds());
        // This DOES NOT format the time to 0:0x when seconds is less than 10.
        // if you need to format the time to standard format, use the sprintf() function.

    }

}

boolean CountDownTimer()
{
    static unsigned long duration = 1000000; // 1 second
    timeFlag = false;

    if (!Stop && !Paused) // if not Stopped or Paused, run timer
    {
        // check the time difference and see if 1 second has elapsed
        if ((_micro = micros()) - time > duration )
        {
            Clock--;
            timeFlag = true;

            if (Clock == 0) // check to see if the clock is 0
            {
                Stop = true; // If so, stop the timer
                digitalWrite(relayPin1, LOW);
                digitalWrite(buzzer, HIGH);
                delay(2000);
                digitalWrite(buzzer, LOW);

            }

            // check to see if micros() has rolled over, if not,
            // then increment "time" by duration
            _micro < time ? time = _micro : time += duration;
        }
    }

    return !Stop; // return the state of the timer


}

void ResetTimer()
{
    SetTimer(R_clock);
    Stop = false;


}

void StartTimer()
{
    Watch = micros(); // get the initial microseconds at the start of the timer
    time = micros(); // hwd added so timer will reset if stopped and then started
    Stop = false;
    Paused = false;
}

void StopTimer()
{
    Stop = true;

}

void StopTimerAt(unsigned int hours, unsigned int minutes, unsigned int seconds)
{
    if (TimeCheck(hours, minutes, seconds) )
        Stop = true;
    //Serial.print("Done");

}

void PauseTimer()
{
    Paused = true;
}

void ResumeTimer() // You can resume the timer if you ever stop it.
{
    Paused = false;
}

void SetTimer(unsigned int hours, unsigned int minutes, unsigned int seconds)
{
    // This handles invalid time overflow ie 1(H), 0(M), 120(S) -> 1, 2, 0
    unsigned int _S = (seconds / 60), _M = (minutes / 60);
    if (_S) minutes += _S;
    if (_M) hours += _M;

    Clock = (hours * 3600) + (minutes * 60) + (seconds % 60);
    R_clock = Clock;
    Stop = false;
}

void SetTimer(unsigned int seconds)
{
    // StartTimer(seconds / 3600, (seconds / 3600) / 60, seconds % 60);
    Clock = seconds;
    R_clock = Clock;
    Stop = false;
}

int ShowHours()
{
    return Clock / 3600;
}

int ShowMinutes()
{
    return (Clock / 60) % 60;
}

int ShowSeconds()
{
    return Clock % 60;
}

unsigned long ShowMilliSeconds()
{
    return (_micro - Watch) / 1000.0;
}

unsigned long ShowMicroSeconds()
{
    return _micro - Watch;
}

boolean TimeHasChanged()
{
    return timeFlag;
}

// output true if timer equals requested time
boolean TimeCheck(unsigned int hours, unsigned int minutes, unsigned int seconds)
{
    return (hours == ShowHours() && minutes == ShowMinutes() && seconds == ShowSeconds());

}

