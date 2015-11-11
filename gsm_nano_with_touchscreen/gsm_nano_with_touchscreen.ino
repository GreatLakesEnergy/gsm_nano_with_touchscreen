  /***************************************************
    This is our library for the Adafruit HX8357D Breakout
    ----> http://www.adafruit.com/products/2050
  
    Check out the links above for our tutorials and wiring diagrams
    These displays use SPI to communicate, 4 or 5 pins are required to
    interface (RST is optional)
    Adafruit invests time and resources providing this open source code,
    please support Adafruit and open-source hardware by purchasing
    products from Adafruit!
  
    Written by Limor Fried/Ladyada for Adafruit Industries.
    MIT license, all text above must be included in any redistribution
   ****************************************************/
  
  #include <SPI.h>
  #include "Adafruit_GFX.h"
  #include "Adafruit_HX8357.h"
  #include "TouchScreen.h"

  #include <SIM800.h>
  //GSM configs
  #define __PWRKEY__ 11
  #define __RESET__ 12
  
  SIM800Core  core;
  SIM800GSM GSM800;
  SIM800IPApplication GPRS800;

  //END GSM
  

  // These are 'flexible' lines that can be changed
  #define TFT_CS 10
  #define TFT_DC 9
  #define TFT_RST 8 // RST can be set to -1 if you tie it to Arduino's reset
  
  // This is calibration data for the raw touch data to the screen coordinates
  #define TS_MINX 110
  #define TS_MINY 80
  #define TS_MAXX 900
  #define TS_MAXY 940
  
  #define MINPRESSURE 10
  #define MAXPRESSURE 1000

  //Pen size is finger size
  #define PENRADIUS 3
  #define BOXSIZE 60
  #define ONSCREEN_BOXSIZE 40
 
  //hack as boxes seem to be getting drawn half the size
  
  #define offset 7

  
  #define start_x  150
  #define start_y  50

  //logging area
  #define LOG_START_X 400
  #define LOG_START_Y 150

  
  // These are the four touchscreen analog pins
  #define YP A2  // must be an analog pin, use "An" notation!
  #define XM A3  // must be an analog pin, use "An" notation!
  #define YM 7   // can be a digital pin
  #define XP 8   // can be a digital pin
  
  // Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
  Adafruit_HX8357 tft = Adafruit_HX8357(TFT_CS, TFT_DC, TFT_RST);

  // For better pressure precision, we need to know the resistance
  // between X+ and X- Use any multimeter to read it
  // For the one we're using, its 300 ohms across the X plate
  TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
  ///GSM things here
  
  //Keep track of our log list
  int log_line = 0;

    
  char Choice, Number[11], Message [64];
  void setup() {
    Serial.begin(9600);
    Serial.println("GSM Test Script"); 
  
    tft.begin(HX8357D);
  
    tft.setRotation(1);
  
   // Serial.println(testText());
    ShowText();
    draw_keypad();
    //unsure what this does copying from test code
    core.host.echo(false);
  }
  
  
  
  void ReadString (char Str[])
  {
    int i=0;
    char Dummy;
    // clear the leftovers of \r \n and other symbols like them
    while (Serial.available () > 0)
      Dummy = Serial.read ();
    while (1)
    {
      if (Serial.available () > 0)
      {
        Str[i] = Serial.read ();
        if (Str[i] == '\r' || Str[i] == '\n')
          break;
        i++;
      }
    }
    Str[i] = 0;
  }


  void loop(void) {

    // Retrieve a point  
    TSPoint p = ts.getPoint();
   
   // we have some minimum pressure we consider 'valid'
   // pressure of 0 means no pressing!
   if (p.z < MINPRESSURE || p.z > MAXPRESSURE) {
      
       return;
    }


    // Scale from ~0->1000 to tft.width using the calibration #'s
    p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
    p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());

      
    //Serial.print("X = "); Serial.print(p.x);
    //Serial.print("\tY = "); Serial.print(p.y);
    //Serial.print("\tPressure = "); Serial.println(p.z);  
    
    //Hardcoded phone number for now
    String phone_number = "0783486047";
    String sms = "I'm alive!";
    
    //catch any incoming calls
    if (Serial.available () > 0)
        log_box(String(Serial.read()));

    
    //wierd bug in screen where boxsize in x axis is not reflected in touch coordinates
    if (p.x < start_x + (BOXSIZE *2) && p.x > start_x) {
        
       if (p.y < ONSCREEN_BOXSIZE) { 
          log_box("clicked on 0");
          flash_box_red(0,0);
       } else if (p.y < (ONSCREEN_BOXSIZE*2)+(offset)) {
          flash_box_red(1,0);
          log_box("calling "+phone_number);
          GSM800.Call(phone_number);
          log_box("calling ....");  
          
          
       } else if (p.y < (ONSCREEN_BOXSIZE*3)+(2*offset)) {
          flash_box_red(2,0);
          GSM800.HangUp ();
          log_box("Hanging up...");
          
       } else if (p.y < (ONSCREEN_BOXSIZE*4)+(3*offset)) {
          flash_box_red(3,0);
          GSM800.SendSMS (phone_number, sms);
          log_box("sending sms...");
          
       } else if (p.y < (ONSCREEN_BOXSIZE*5)+(4*offset)) {
          flash_box_red(4,0);
          log_box("clicked on 4");
          
       }
    
    }
    else if(p.x < start_x ){
      Serial.println("lower half");
       if (p.y < ONSCREEN_BOXSIZE) { 
             flash_box_red(5,1);
              log_box("clicked on 5");
       } else if (p.y < (ONSCREEN_BOXSIZE*2)+(offset)) {
          flash_box_red(6,1);
          log_box("clicked on 6");
       } else if (p.y < (ONSCREEN_BOXSIZE*3)+(2*offset)) {
          flash_box_red(7,1);
          log_box("clicked on 7");
       } else if (p.y < (ONSCREEN_BOXSIZE*4)+(3*offset)) {
          flash_box_red(8,1);
          log_box("clicked on 8");
          
       } else if (p.y < (ONSCREEN_BOXSIZE*5)+(4*offset)) {
          flash_box_red(9,1);
          log_box("clicked on 9");
          
       }
    }
  }
  unsigned long ShowText() {
    tft.fillScreen(HX8357_BLACK);
    unsigned long start = micros();
    tft.setCursor(0, 0);
    tft.setTextColor(HX8357_RED);    tft.setTextSize(3);
    tft.println("GSM Test Script!");
    tft.setTextSize(5);


    //Print Menu
    tft.setTextColor(HX8357_GREEN);
    tft.setTextSize(2);
    tft.println ();
    tft.println ("1. Call");
    tft.println ("2. Hang up");
    tft.println ("3. Send SMS");
    tft.println ("4. Read SMS");
    tft.println ("5. Read all SMS");
  }

  void flash_box_red(int i,int row){
      //to get second column in the right place
      tft.setTextColor(HX8357_GREEN);
      tft.setTextSize(4);
      int j = i % 5;
      if (row == 0){
        tft.fillRect((BOXSIZE*i)+(offset*i),start_x , BOXSIZE, BOXSIZE, HX8357_RED);
        delay(100);
        tft.fillRect((BOXSIZE*i)+(offset*i),start_x , BOXSIZE, BOXSIZE, HX8357_YELLOW);
        tft.setCursor( (BOXSIZE*i)+ (offset * i), start_x);
        tft.print (i);
      }
      else{
        tft.fillRect((BOXSIZE*j)+(offset*j),start_x + BOXSIZE + offset , BOXSIZE, BOXSIZE, HX8357_RED);
        delay(100);
        tft.fillRect((BOXSIZE*j)+(offset*j),start_x + BOXSIZE + offset , BOXSIZE, BOXSIZE, HX8357_YELLOW);
        tft.setCursor( (BOXSIZE*j)+ (offset * j), start_x + BOXSIZE + offset );
        tft.print (i);
      }
  }
  void draw_keypad(){

    
    tft.setTextColor(HX8357_GREEN);
    tft.setTextSize(4);
    int j = 0;
    for(int i = 0; i < 5; i++){
          
      tft.fillRect((BOXSIZE*i)+(offset*i),start_x , BOXSIZE, BOXSIZE, HX8357_YELLOW);
      tft.setCursor( (BOXSIZE*i)+ (offset * i), start_x);
      tft.print (i);
    }
    for(int i = 5; i < 10; i++){
      j = i%5;
      tft.fillRect((BOXSIZE * j)+(offset * j),start_x + BOXSIZE + offset , BOXSIZE, BOXSIZE, HX8357_YELLOW);
      tft.setCursor( (BOXSIZE * j)+ (offset * j), start_x + BOXSIZE + offset);
      tft.print (i);
    }
  }
  void log_box(String to_display){
    tft.setTextColor(HX8357_WHITE);
    tft.setTextSize(1);
    int line = log_line % 10;
    int cursor = line * 10;
    if (line == 0){
      tft.fillRect(190,30 ,300 ,100, HX8357_BLACK);
      cursor = 0;
    }

    tft.setCursor(190,30 + cursor);
    tft.print (to_display);
    log_line++;
  
  }
  

