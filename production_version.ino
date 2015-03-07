/*************************************************************************************

  (cc) Jose Berengueres 4-8th Feb  2015, Dubai
  (LCD sample code by Mark Bramwell, July 2010)
  
  This is an Arduino based Ollie box  implmentation used for regata starts
  
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

#define NUM_SEQ  3
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

#define RELAY_HORN    11   // horn relay
#define RELAY_BEEP   2  // beeper 
#define STD_DELAY 300

#define SHORT_BEEP_LENGTH 500

#define WARNING_BEEP 0
#define SHORT_HORN 1
#define LONG_HORN 2
#define SUPER_LONG_HORN 3

bool toggle = true;
bool my_start = false;

unsigned long  len_of_note[] = {500,400,800,1500}; // in ms

int prev_s;
int t;
int prev_mode = 5;
int mode = 5;
long start;
long ctdwn;
long sound_start;

unsigned long *sch;
int *h_or_b;
int index;

//                          warning long buzz (sb)= 0,   short horn (sh) = 1, long horn (lh) =2  extralong horn 3
// Short seq countdown // 10lb  5sh   5blank  3lh

unsigned long sch_3[] =   {   0,   10,20,30,40, 50,
                              100, 
                              190, 200, 
                              280,290,300, 
                              
                              1*600, 600 +10, 600+20, 600+30, 600+40, 600+50,
                              900-15-20, 900-15-10, 900-15, 900,
                              2*600-15, 2*600, 2*600+10, 2*600+20, 2*600+30, 2*600+40, 2*600+50,
                              3*600-30, 3*600-15, 3*600,          3*600+60, 3*600+70,3*600+80,3*600+90,      3*600+100,3*600+110, 3*600+120, 3*600+130, 3*600+140, 3*600+150, 3*600+160,3*600+170, 3*600+180, 3*600+190, 3*600+200  };
int h_or_b3[] ={
                               3,   1,1,1,1,1,
                               1,
                               1,1,
                               1,1,1,
                            2, 0,0,0,0,0,
                            1,1,1,2,
                            2,2,0,0,0,0,0,
                            2,2,2,   1,1,1,1,1,      0, 0, 0, 0, 0, 0, 0, 0, 0, 0  };
int index_3 = 6 + 6+ 6+ 4 +  7+ 18-1;
int ctdwn_3 = 205; // 3* 60 +39;



//************************************* FIVE MIN SEQ ********************************
unsigned long sch_5[] =   {    0,      10, 20,30,40,50, 

                               1*600, 1*600+10, 1*600+20, 1*600+30,1*600+40,1*600+50, 
                            
                               4*600, 4*600+10, 4*600+20, 4*600+30,4*600+40,4*600+50, 
                          
                               5*600,5*600+10,5*600+20,5*600+30,5*600+40,5*600+50, 5*600+60, 5*600+70,5*600+80,5*600+90,5*600+100,5*600+110, 5*600+120, 5*600+130, 5*600+140, 5*600+150, 5*600+160,5*600+170, 5*600+180, 5*600+190, 5*600+200  };

int h_or_b5[] =            {   3,    0,0,0,0,0,
                               2, 0,0,0,0,0,
                               2,0,0,0,0,0,
                               2,0, 0, 0, 0, 0,1,1,1,1,1,0, 0, 0, 0, 0, 0, 0, 0, 0, 0  };
int index_5 = 28+6+5-1; // 
int ctdwn_5 = 5*60 + 25; // 9; //


//************************************* FIVE BRITISH MIN SEQ ********************************
unsigned long sch_5british[] =   {    0, 10,             20,       30, 

                                  1*600, 1*600+10, 1*600+20, 1*600+30, 
                            
                                  4*600, 4*600+10, 4*600+20, 4*600+30, 
                          
                                  5*600,5*600+10,5*600+20,   5*600+30  };

int h_or_b5british[] =            {  3, 0,0,0,
                                     2, 0,0,0,
                                     2, 0,0,0,
                                     2, 0,0,0 };
int index_5british = 4*4-1; // 
int ctdwn_5british = 5*60 + 6; //

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
    lcd_w("BY CHRIS LABORDE"," & J BERENGUERES");
    lcd_w("SELECT 5 OR 3 ","MINUTE SEQUENCE ");
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
   t =0;
   Serial.begin(9600);
   Serial.println("hello");
   pinMode(RELAY_HORN, OUTPUT);
   pinMode(RELAY_BEEP, OUTPUT);
   lcd.begin(16, 2);
   lcd.setCursor(0,0);
   mymenu();
   lcd.setCursor(0,1);            
   lcd.print("00:00 ");
    
}

void activate_sound(int a) {
  // check what instrument to sound
      int what_beep = RELAY_HORN;
      if (a==WARNING_BEEP) { what_beep = RELAY_BEEP;}
      digitalWrite(what_beep, HIGH);
      sound_start = millis();
      lcd.setCursor(7,1);
      if (a==WARNING_BEEP)  {    
            lcd.print("B");
      }else{
            lcd.print("H");
      }
}

void de_activate_sound(int a) {
      int what_beep = RELAY_HORN;
      if (a==WARNING_BEEP) { what_beep = RELAY_BEEP;}
      digitalWrite(what_beep, LOW);
      lcd.setCursor(7,1);
      lcd.print("        ");
}

void horn_or_beep(long s){
    if (sound_on) {
         if ( ((millis()-sound_start) > len_of_note[h_or_b[index]] ) ) {
            de_activate_sound(h_or_b[index]);
            sound_on = false;
            index = index - 1;
         }
    }else{
         unsigned long v = (sch[index]*100);
         //Serial.println("long v = (long) (sch[index]*1000);");
         //Serial.println(index);
         //Serial.println(s);
         //Serial.println(v);
         

         if ( s < v +1000  ) {  //if ( s < v + 500  ) {
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
     digitalWrite(RELAY_HORN, LOW);
  
  }
  
}

void loop(){
   long s =  millis()-start ;    
   if ( my_start ) {
     long d = ctdwn*1000 - s ;
     horn_or_beep( d );
     diplay_timer( d ); 
   }
   
   
   lcd_key = read_LCD_buttons();   // read the buttons
   delay(50);
   switch (lcd_key){               // depending on which button was pushed, we perform an action
       case btnLEFT:{
          my_start = !my_start;
          if (my_start) {
             if (t==0){
               lcd_w(" ...STARTING... ","FIVE MINUTE SEQ ");
               ctdwn = ctdwn_5 ; 
               sch = sch_5;
               h_or_b = h_or_b5;
               index = index_5;
             }
             if(t==1){
               lcd_w(" ...STARTING... ","THREE MINUTE SEQ");
               ctdwn = ctdwn_3;
               sch = sch_3;
               h_or_b = h_or_b3;
               index = index_3;
             }
             if (t==2) {
               lcd_w(" ...STARTING... ","BRITISH 5MIN SEQ");
               ctdwn = ctdwn_5british;
               sch = sch_5british;
               h_or_b = h_or_b5british;
               index = index_5british;
             }
             lcd.setCursor(0,1);            
             lcd.print("00:00                        ");
             sound_on = false;
             start = millis();
             break;
          }else{
             // stop
             digitalWrite(RELAY_HORN, LOW);
             digitalWrite(RELAY_BEEP, LOW);
             lcd.setCursor(0,0);            
             lcd.print("SEQ. CANCELLED         ");
             my_start = false;
          
          }          
          delay(400);
          break;
      }
 
       case btnSELECT:{
         if (!my_start) {
             t = t + 1;
             if (t>(NUM_SEQ-1)) {t= 0;}
             lcd.setCursor(0,0);              
             if ( t == 0 ) {     lcd.print("FIVE  MIN. START      "); }
             if ( t == 1 ) {     lcd.print("THREE MIN. START     "); }
             if ( t == 2 ) {     lcd.print("BRITISH (5)START     "); }
             lcd.setCursor(0,1);              
             lcd.print("                       "); 
             delay(400);
             break;
         }
       }
       
       case btnNONE:{
             break;
       }
   }
}
