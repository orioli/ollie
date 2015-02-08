/*************************************************************************************

  (cc) Jose Berengueres 4-8th Feb  2015, Dubai
  (LCD sample code by Mark Bramwell, July 2010)
  
  This is an aRduino Ollie box used for regata starts
  
  Components: 
  DF ROBOT LCD PANEL, 2 x RELAY Module v3.2, 1 x Arduino 
  connect the relay of the horn to D2 of LCD shield. This relay drives aircompressor (12VDC battery)
  connect the relay that controls buzzer/beeper to D11. 

**************************************************************************************/

#include <LiquidCrystal.h>

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);           // select the pins used on the LCD panel

// define some values used by the panel and buttons
int lcd_key     = 0;
int adc_key_in  = 0;

#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

#define RELAY_HORN   2   // horn relay
#define RELAY_BEEP   11  // beeper 
#define STD_DELAY 300

#define SHORT_BEEP_LENGTH 500

long prev_s;

int prev_mode = 5;
int mode = 5;
unsigned long start;
unsigned long ctdwn;
unsigned long sound_start;

unsigned long  *sch;
int *h_or_b;
int index;

// Short seq countdown
unsigned long  sch_3[] =   { 0, 1 ,2, 3, 4, 5, 10, 19, 20, 28, 29, 30, 1*60, 2*60, 2*60+58, 2*60+59, 3*60, 3*60+1, 3*60+2, 3*60+3  };
int h_or_b3[] =            { 0, 1, 1, 1, 1, 1,  1,  1,  1,  1,  1,  1,    0,    0,       0,       0,    0,      1,      1,      1  };
int index_3 = 19;


// SLong seq countdown
unsigned long  sch_5[] =   { 0, 1 ,2, 3, 4, 5, 10, 19, 20, 28, 29, 30, 1*60, 4*60, 4*60+56, 4*60+57, 4*60+58, 4*60+59, 5*60, 5*60+1, 5*60+2, 5*60+3, 5*60+4, 5*60+5  };
int h_or_b5[] =            { 0, 1, 1, 1, 1, 1,  1,  1,  1,  1,  1,  1,    0,    0,       0,       0,       0,     0,    0,      1,      1,      1,      1,      1    };
int index_5 = 23;

bool sound_on = false;

int read_LCD_buttons(){               // read the buttons
    adc_key_in = analogRead(0);       // read the value from the sensor 

    // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
    // we add approx 50 to those values and check to see if we are close
    // We make this the 1st option for speed reasons since it will be the most likely result

    if (adc_key_in > 1000) return btnNONE; 

    // For V1.1 use this threshold
    if (adc_key_in < 50)   return btnRIGHT;  
    if (adc_key_in < 250)  return btnUP; 
    if (adc_key_in < 450)  return btnDOWN; 
    if (adc_key_in < 650)  return btnLEFT; 
    if (adc_key_in < 850)  return btnSELECT;  

   // For V1.0 comment the other threshold and use the one below:
   /*
     if (adc_key_in < 50)   return btnRIGHT;  
     if (adc_key_in < 195)  return btnUP; 
     if (adc_key_in < 380)  return btnDOWN; 
     if (adc_key_in < 555)  return btnLEFT; 
     if (adc_key_in < 790)  return btnSELECT;   
   */

    return btnNONE;                // when all others fail, return this.
}

// utility to print on LCD
void lcd_w(char a[16],char b[16]) {
    lcd.setCursor(0,0);
    lcd.print("                 ");
    lcd.setCursor(0,1);
    lcd.print("                 ");
    lcd.setCursor(0,0);
    lcd.print(a);
    delay(STD_DELAY);
    delay(STD_DELAY);
    lcd.setCursor(0,1);
    lcd.print(b);
    delay(STD_DELAY);
    delay(STD_DELAY);
}

void mymenu(){
    lcd_w("OLLIE BOX v 2.1 ","BY J BERENGUERES");
    lcd_w("  INSTRUCTIONS  ","                ");
    lcd_w(" 'LEFT' --> BUZZ","'RIGHT' --> HORN");
    lcd_w("TO STOP SEQUENCE","  --> 'SELECT'  ");
    lcd_w(" PRESS 'UP'-->5m","     'DOWN'-->3m");
}
/*
void vibrate(int n,int b, int c) {
  for(int i=0;i<n;i++){
    digitalWrite(RELAY_PIN, HIGH);
    delay(b);
    digitalWrite(RELAY_PIN, LOW);
    delay(c);
    
  }
}
*/

void setup(){
   pinMode(RELAY_HORN, OUTPUT);
   pinMode(RELAY_BEEP, OUTPUT);
   lcd.begin(16, 2);
   lcd.setCursor(0,0);
   mymenu();
   lcd.setCursor(0,1);            
   lcd.print("00:00 ");
}

void activate_sound(int a) {
      int what_beep = RELAY_HORN;
      if (a==1) { what_beep = RELAY_BEEP;}
      digitalWrite(what_beep, HIGH);
      sound_start = millis();
      lcd.setCursor(7,1);
      if (a==0)  {    
            lcd.print("H");
      }else{
            lcd.print("B");
      
      }
}

void de_activate_sound(int a) {
      int what_beep = RELAY_HORN;
      if (a==1) { what_beep = RELAY_BEEP;}
      digitalWrite(what_beep, LOW);
      lcd.setCursor(7,1);
      lcd.print("        ");
}

void horn_or_beep(long s){
    if (sound_on) {
         if ( ((millis()-sound_start) > SHORT_BEEP_LENGTH) ) {
            de_activate_sound(h_or_b[index]);
            sound_on = false;
            index = index - 1;
         }
    }else{
      if ( s < (sch[index]*1000 + 500) ) {
           activate_sound(h_or_b[index]);
           sound_on = true;
      }
    }
}

void diplay_timer(long s){
  if (s>-1){
     int ss = (s/1000) %60;
     int m =  (s/1000)/60;
     int s_pos = 3;
     if (ss<10) 
         {   s_pos = s_pos +1; 
             lcd.setCursor(3,1);            
             lcd.print("0");
         }
   
     lcd.setCursor(1,1);            
     lcd.print(m);

     lcd.setCursor(2,1);            
     lcd.print(":");

     lcd.setCursor(s_pos,1);            
     lcd.print(ss);
  }else{
  
     lcd.setCursor(7,1);            
     lcd.print("START!");
  
  }
  
}

void loop(){
   long s =  millis()-start ;    
   if (mode == btnUP || mode == btnDOWN ) {
    long d = ctdwn*1000 - s ;
     horn_or_beep( d );
     diplay_timer( d ); 
   }
   
   
   lcd_key = read_LCD_buttons();   // read the buttons
   delay(50);
   switch (lcd_key){               // depending on which button was pushed, we perform an action
       
       
       case btnRIGHT:{             //  push button "RIGHT" and show the word on the screen
            //mode = btnRIGHT;
            digitalWrite(RELAY_HORN, HIGH);
            lcd.setCursor(7,1);
            lcd.print("H");
            delay(SHORT_BEEP_LENGTH);
            digitalWrite(RELAY_HORN, LOW);
            lcd.setCursor(0,1);
            lcd.print("                    ");
            
            break;
       }
       case btnLEFT:{
            //mode = btnLEFT;
            digitalWrite(RELAY_BEEP, HIGH);
            lcd.setCursor(7,1);
            lcd.print("B");
            delay(SHORT_BEEP_LENGTH);
            digitalWrite(RELAY_BEEP, LOW);
            lcd.setCursor(0,1);
            lcd.print("                    ");
             break;
       }    
       case btnUP:{
             mode = btnUP;
             lcd_w(" ...STARTING... ","FIVE MINUTE SEQ ");
             lcd.setCursor(0,0);
             lcd.print("FIVE MINUTE SEQ ");  //  push button "DOWN" and show the word on the screen
             lcd.setCursor(0,1);
             lcd.print("                ");  //  push button "DOWN" and show the word on the screen
             lcd.setCursor(0,1);            
             lcd.print("00:00");
             mode = btnUP;
             ctdwn = 5*60 + 5 + 1;
             sound_on = false;
             sch = sch_5;
             h_or_b = h_or_b5;
             index = 23;
             start = millis();
             
             break;
       }
       case btnDOWN:{
             mode = btnDOWN;
             lcd_w(" ...STARTING... ","THREE MINUTE SEQ");
             lcd.setCursor(0,0);
             lcd.print("THREE MINUTE SEQ");  //  push button "DOWN" and show the word on the screen
             lcd.setCursor(0,1);
             lcd.print("                ");  //  push button "DOWN" and show the word on the screen
             lcd.setCursor(0,1);            
             lcd.print("00:00");
             mode = btnDOWN;
             ctdwn = 3*60 + 5 + 1;
             sound_on = false;
             sch = sch_3;
             h_or_b = h_or_b3;
             index = 19;
             start = millis();
 
             break;
       }
       case btnSELECT:{
             if ( mode != btnSELECT ) { 
                  mode = btnSELECT;
                  lcd.setCursor(7,1);
                  lcd.print("CANCELLED"); 
                  digitalWrite(RELAY_HORN, LOW);
                  digitalWrite(RELAY_BEEP, LOW);
                  delay(1000);
             }else{ 
                  mymenu();
             }
             break;
       }
       
       case btnNONE:{
             break;
       }
   }
}
