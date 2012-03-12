/*
  NMEA Display

  Displays the information from incoming NMEA stream on a 16x2 LCD display
  
  Copyright (c) 2012 Pavel Kalian
 */

// include the library code:
#include <LiquidCrystal.h>
#include <TinyGPS.h>
#include <SoftwareSerial.h>
TinyGPS gps;
int iteration = 0;

#define RXPIN 10
#define TXPIN 13

#define POSITIONDISPLAYSEC 10
#define WHOLECYCLESEC 13

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 2, 3, 4, 5, 6, 7, 8, 9);
SoftwareSerial nss(RXPIN, TXPIN);

static void gpsdump(TinyGPS &gps);
static bool feedgps();
static void print_float(float val, float invalid, bool islatitude);

void setup(){
    // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  lcd.write("Howdy navigator?");
  lcd.setCursor(0, 1);
  // initialize the serial communications:
  long baud = detRate(RXPIN);
  lcd.write("Baudrate: ");
  lcd.print(baud);
  if (baud == 0)
    baud = 4800;
  nss.begin(baud);
}

void loop()
{
  bool newdata = false;
  unsigned long start = millis();
  
  // when characters arrive over the serial port...
  // Every second we print an update
  while (millis() - start < 1000)
  {
    if (feedgps())
      newdata = true;
  }
  // clear the screen
  //lcd.clear();
  lcd.setCursor(0,0);
  gpsdump(gps);
  iteration++;
}

static void gpsdump(TinyGPS &gps)
{
  float flat, flon;
  bool disp = true;
  unsigned long age;//, date, time, chars = 0;
  //unsigned short sentences = 0, failed = 0;
  //static const float LONDON_LAT = 51.508131, LONDON_LON = -0.128002;
  
  //print_int(gps.satellites(), TinyGPS::GPS_INVALID_SATELLITES, 5);
  //print_int(gps.hdop(), TinyGPS::GPS_INVALID_HDOP, 5);
  if (iteration < POSITIONDISPLAYSEC)
  {
    gps.f_get_position(&flat, &flon, &age);
    if (age == TinyGPS::GPS_INVALID_AGE)
      disp = false;
    else if (age > 600000) //Older than 10 minutes - considered totally lost
      disp = false;
    else if (age > 300000) //Older than 5 minutes
      lcd.write("!!:");
    else if (age > 60000) //Older than a minute
      lcd.write("?!:");
    else if (age > 5000) //Older than 5 seconds
      lcd.write("??:");
    else
      lcd.write("OK:");
    if (disp)
    {
      lcd.write("LA:");
      print_float(flat, TinyGPS::GPS_INVALID_F_ANGLE, true);
      lcd.setCursor(0,1);
      lcd.write("   LO:");
      print_float(flon, TinyGPS::GPS_INVALID_F_ANGLE, false);
    } else {
      lcd.write("No fix detected ");
      lcd.setCursor(0,1);
      lcd.write("                ");
    }
  }
  else if (iteration <= WHOLECYCLESEC)
  {
    int year;
    byte month, day, hour, minutes, second, hundredths;
    
    int course = gps.f_course();
    float speed = gps.f_speed_knots();
    
    gps.crack_datetime(&year, &month, &day, &hour, &minutes, &second, &hundredths, &age);
    
    lcd.write("C:");
    
    if(course != TinyGPS::GPS_INVALID_F_ANGLE)
      lcd.print(course);
    else
      lcd.write("---");
    lcd.write((char)223);
    lcd.write(" S:");
    if(course != TinyGPS::GPS_INVALID_F_SPEED && speed >= 0.0)
      lcd.print(speed, 1);
    else
      lcd.write("---");
    lcd.write("kt");
    lcd.write("    ");
    
    lcd.setCursor(0,1);
    if (month != 0 && day != 0)
    {
      lcd.print(year);
      lcd.write("-");
      lcd.print(month);
      lcd.write("-");
      lcd.print(day);
      lcd.write(" ");
      lcd.print(hour);
      lcd.write(":");
      if (minutes < 10)
        lcd.write("0");
      lcd.print(minutes);
      lcd.write("    ");
    } else
      lcd.write("                ");
    if (iteration == WHOLECYCLESEC)
      iteration = 0;
  }
}

static void print_float(float val, float invalid, bool islatitude)
{
  if (val == invalid)
    lcd.write("*******");
  else
  {
    int degs = (int)floor(abs(val));
    int degsmath = (int)floor(val);
    float mins = (val - degsmath) * 60;
    if (islatitude)
      lcd.write(" ");
    else
      if (degs < 100)
        lcd.write("0");
    lcd.print(degs);
    lcd.write((char)223);
    if (mins < 10)
      lcd.write("0");
    lcd.print(mins, 2);
    if (islatitude)
      if (val < 0)
        lcd.write("S");
      else
        lcd.write("N");
    else
      if (val < 0)
        lcd.write("W");
      else
        lcd.write("E");
  }
  feedgps();
}

static bool feedgps()
{
  while (nss.available())
  {
    if (gps.encode(nss.read()))
      return true;
  }
  return false;
}

long detRate(int recpin)  // function to return valid received baud rate
                          // Note that the serial monitor has no 600 baud option and 300 baud
                          // doesn't seem to work with version 22 hardware serial library
  {
  long baud, rate = 10000, x;
  for (int i = 0; i < 10; i++) {
      x = pulseIn(recpin,LOW);   // measure the next zero bit width
      if (x > 0)
        rate = x < rate ? x : rate;
  }
  if (rate < 12)
      baud = 115200;
      else if (rate < 20)
      baud = 57600;
      else if (rate < 29)
      baud = 38400;
      else if (rate < 40)
      baud = 28800;
      else if (rate < 60)
      baud = 19200;
      else if (rate < 80)
      baud = 14400;
      else if (rate < 150)
      baud = 9600;
      else if (rate < 300)
      baud = 4800;
      else if (rate < 600)
      baud = 2400;
      else if (rate < 1200)
      baud = 1200;
      else
      baud = 0;  
   return baud;
  }
