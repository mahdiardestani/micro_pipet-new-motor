#include "U8glib.h"
#include "stdlib.h"
#include <EEPROM.h> 

//__________________________________
/* Create an instance for the SSD1306 OLED display in SPI mode
   connection scheme:
     CS=Pin 8 -> 8 ->4
     DC=A0=Pin 9 ->11 ->7->5
     Reset=Pin 10 ->13 ->15->6
     D1=mosi=Pin 11 ->9->7
     D0=sck=Pin 12->10 ->8  
*/

//_________________________________
/* Pins use for pins
  pin 9 = UP
  pin 10 = Left
  pin 11 = ok
  pin 12 = down
  pin 13 = right
 */

//**************************************************
U8GLIB_SSD1306_128X64 u8g(8, 7, 4, 5, 6);
//**************************************************

//__________address of the step of the micropipet counter in the eeprom
const int pipet_step_address=0;
byte pipet_step;

byte get_pipet_step(){
  pipet_step =  EEPROM.read(pipet_step_address);
  return pipet_step;
}

void set_pipet_step(byte val){
   EEPROM.write(pipet_step_address,val);
}

//*_____________________Pins that use for steppermotor
const int stepPin = 3; 
const int dirPin = 2;

//______________________Values use for steppermotor
  int step_number = 0;
  // false=clockwise,true=counterClockwise
  boolean dir = false; // false is flush pipet
  int step_motor = -1;

//____________________This values use for Render function
  int current_page = 0;
  int next_page = 0;
  int scroll = 0;
  boolean have_update = false;
  int down_key = 0 ;

//____________________This value uses for filling & inject display
  char buff[50];
  float inject_value = 0;
  float inject_wait = 0;
  float filling_value = 0;
  int delayTime = 0;

//______________________Enable interrupt for pins
void pciSetup(byte pin) {
  // enable pin
  *digitalPinToPCMSK(pin) |= bit (digitalPinToPCMSKbit(pin));
  // clear any outstanding interrupt
  PCIFR  |= bit (digitalPinToPCICRbit(pin));
  // enable interrupt for the group
  PCICR  |= bit (digitalPinToPCICRbit(pin));
}

//_____________________Functions of main pages
void page_loading() {
  int step = 40;
  int y = 60;
  u8g.firstPage();
  do {
    //draw curve one
    u8g.setColorIndex(1);       //one set color white
    u8g.drawFilledEllipse(65, 10, 20, 10, U8G_DRAW_UPPER_RIGHT);
    u8g.drawFilledEllipse(65, 10, 20, 10, U8G_DRAW_UPPER_LEFT);
    u8g.setColorIndex(0);      //zero set color black
    u8g.drawFilledEllipse(65, 10, 15, 5, U8G_DRAW_UPPER_RIGHT);
    u8g.drawFilledEllipse(65, 10, 15, 5, U8G_DRAW_UPPER_LEFT);
    //draw boxes
    u8g.setColorIndex(1);
    u8g.drawBox(45, 15, 41, 5);
    u8g.drawBox(63, 20, 5, 15);
    u8g.drawBox(73, 25, 5, 10);
    //draw curve two
    u8g.drawFilledEllipse(70, 35, 7, 7, U8G_DRAW_LOWER_RIGHT);
    u8g.drawFilledEllipse(70, 35, 7, 7, U8G_DRAW_LOWER_LEFT);
    //draw curve three
    u8g.setColorIndex(0);
    u8g.drawFilledEllipse(70, 35, 2, 2, U8G_DRAW_LOWER_RIGHT);
    u8g.drawFilledEllipse(70, 35, 2, 2, U8G_DRAW_LOWER_LEFT);
    //draw loading bar
    u8g.setColorIndex(1);
    u8g.drawLine(40, y, step, y);
    step ++;
    if (step >= 90) {
      step = 40;
    }
  } while (u8g.nextPage());
}

void page_main_filling() {
  u8g.firstPage();
  do {
    u8g.setColorIndex(1);
    // up arrow
    u8g.drawLine(60, 4, 64, 0);
    u8g.drawLine(64, 0, 68, 4);
    //text
    u8g.drawStr( 37, 36 , "Filling");
    //down arrow
    u8g.drawLine(60, 60, 64, 64);
    u8g.drawLine(64, 64, 68, 60);
  } while (u8g.nextPage());
}

void page_main_clean() {
  u8g.firstPage();
  do {
    u8g.setColorIndex(1);
    // up arrow
    u8g.drawLine(60, 4, 64, 0);
    u8g.drawLine(64, 0, 68, 4);
    //text
    u8g.drawStr( 45, 36 , "Clean");
    // down arrow
    u8g.drawLine(60, 60, 64, 64);
    u8g.drawLine(64, 64, 68, 60);
  } while (u8g.nextPage());
}

void page_main_about() {
  u8g.firstPage();
  do {
    u8g.setColorIndex(1);
    //up arrow
    u8g.drawLine(60, 4, 64, 0);
    u8g.drawLine(64, 0, 68, 4);
    //text
    u8g.drawStr( 45, 36 , "About");
    //down arrow
    u8g.drawLine(60, 60, 64, 64);
    u8g.drawLine(64, 64, 68, 60);
  } while (u8g.nextPage());
}

void page_main_setting() {
  u8g.firstPage();
  do {
    u8g.setColorIndex(1);
    //up arrow
    u8g.drawLine(60, 4, 64, 0);
    u8g.drawLine(64, 0, 68, 4);
    //text
    u8g.drawStr( 39, 36 , "Setting");
    //down arrow
    u8g.drawLine(60, 60, 64, 64);
    u8g.drawLine(64, 64, 68, 60);
  } while (u8g.nextPage());
}

void page_main_inject() {
  u8g.firstPage();
  do {
    u8g.setColorIndex(1);
    //up arrow
    u8g.drawLine(60, 4, 64, 0);
    u8g.drawLine(64, 0, 68, 4);
    //text
    u8g.drawStr( 38, 36 , "Inject");
    //down arrow
    u8g.drawLine(60, 60, 64, 64);
    u8g.drawLine(64, 64, 68, 60);
  } while (u8g.nextPage());
}

void change_fillingvalue_display(){
  u8g.firstPage();
  do {
    //text
    u8g.setColorIndex(1);
    u8g.drawStr( 0, 10 , "Filling");
    //left arrow
    u8g.drawLine(0, 32, 4, 28);
    u8g.drawLine(0, 32, 4, 36);
    //draw box
    u8g.drawBox(18, 16, 95, 40);
    //text
    u8g.setColorIndex(0);
    u8g.drawStr(45, 40, "  ul" );
    //dtostrf(value,1,1,buff);
    u8g.drawStr(24,40,dtostrf(filling_value,1,1,buff));
    //vertical Line in box
    u8g.drawLine(90, 19, 90, 55);
    //horizontal line in box
    u8g.drawLine(90, 37, 110, 37);
    //up arrow in box
    u8g.drawLine(96, 28, 100, 24);
    u8g.drawLine(100, 24, 104, 28);
    //down arrow in box
    u8g.drawLine(96, 44, 100, 48);
    u8g.drawLine(100, 48, 104, 44);
  } while (u8g.nextPage());
}

void main_fillingvalue_display() {
  u8g.firstPage();
  do {
    u8g.setColorIndex(1);
    u8g.drawStr( 0, 10 , "Filling");
    u8g.setColorIndex(1);
    //left arrow
    u8g.drawLine(0, 32, 4, 28);
    u8g.drawLine(0, 32, 4, 36);
    //draw box
    u8g.drawFrame(18, 16, 95, 40);
    //text
    u8g.setColorIndex(1);
    u8g.drawStr(45, 40, "  ul" );
    u8g.drawStr(24,40,dtostrf(filling_value,1,1,buff));
    //vertical Line in box
    u8g.drawLine(90, 19, 90, 55);
    //horizontal line in box
    u8g.drawLine(90, 37, 110, 37);
    //up arrow in box
    u8g.drawLine(96, 28, 100, 24);
    u8g.drawLine(100, 24, 104, 28);
    //down arrow in box
    u8g.drawLine(96, 44, 100, 48);
    u8g.drawLine(100, 48, 104, 44);
  } while (u8g.nextPage());
}

void page_filling_run() {
  u8g.firstPage();
  do {
    u8g.setColorIndex(1);
    //text
    u8g.drawStr( 0, 10 , "Filling");
    //up arrow
    u8g.drawLine(60, 4, 64, 0);
    u8g.drawLine(64, 0, 68, 4);
    //down arrow
    u8g.drawLine(60, 60, 64, 64);
    u8g.drawLine(64, 64, 68, 60);
    //left arrow
    u8g.drawLine(0, 32, 4, 28);
    u8g.drawLine(0, 32, 4, 36);
    // draw box
    u8g.drawFrame(40, 21, 50, 30);
    //text
    u8g.drawStr( 53, 39 , "Run");
  } while (u8g.nextPage());
}

void page_clean_run() {
  u8g.firstPage();
  do {
    u8g.setColorIndex(1);
    //text
    u8g.drawStr( 0, 10 , "Clean");
    //left arrow
    u8g.drawLine(0, 32, 4, 28);
    u8g.drawLine(0, 32, 4, 36);
    // draw box
    u8g.drawFrame(40, 21, 50, 30);
    //text
    u8g.drawStr( 53, 39 , "Run");
  } while (u8g.nextPage());
}

void change_inject_step(){
  u8g.firstPage();
  do {
    //text
    u8g.setColorIndex(1);
    u8g.drawStr( 0, 10 , "Inject");
    //left arrow
    u8g.drawLine(0, 32, 4, 28);
    u8g.drawLine(0, 32, 4, 36);
    //draw box
    u8g.drawBox(18, 16, 95, 40);
    //text
    u8g.setColorIndex(0);
    u8g.drawStr(45, 40, "  ul" );
    u8g.drawStr(24,40,dtostrf(inject_value,1,1,buff));
    //vertical Line in box
    u8g.drawLine(90, 19, 90, 55);
    //horizontal line in box
    u8g.drawLine(90, 37, 110, 37);
    //up arrow in box
    u8g.drawLine(96, 28, 100, 24);
    u8g.drawLine(100, 24, 104, 28);
    //down arrow in box
    u8g.drawLine(96, 44, 100, 48);
    u8g.drawLine(100, 48, 104, 44);
  } while (u8g.nextPage());
}

void page_inject_step() {
  u8g.firstPage();
  do {
    u8g.setColorIndex(1);
    u8g.drawStr( 0, 10 , "Inject");
    u8g.setColorIndex(1);
    //left arrow
    u8g.drawLine(0, 32, 4, 28);
    u8g.drawLine(0, 32, 4, 36);
    //draw box
    u8g.drawFrame(18, 16, 95, 40);
    //u8g.drawBox(18,16,95,40);
    //text
    u8g.setColorIndex(1);
    u8g.drawStr(45, 40, "  ul" );
    u8g.drawStr(24,40,dtostrf(inject_value,1,1,buff));
    //vertical Line in box
    u8g.drawLine(90, 19, 90, 55);
    //horizontal line in box
    u8g.drawLine(90, 37, 110, 37);
    //up arrow in box
    u8g.drawLine(96, 28, 100, 24);
    u8g.drawLine(100, 24, 104, 28);
    //down arrow in box
    u8g.drawLine(96, 44, 100, 48);
    u8g.drawLine(100, 48, 104, 44);
  } while (u8g.nextPage());
}

void change_inject_wait(){
  u8g.firstPage();
  do {
    //text
    u8g.setColorIndex(1);
    u8g.drawStr( 0, 10 , "Inject");
    //left arrow
    u8g.drawLine(0, 32, 4, 28);
    u8g.drawLine(0, 32, 4, 36);
    //draw box
    u8g.drawBox(18, 16, 95, 40);
    //text
    u8g.setColorIndex(0);
    u8g.drawStr(45, 40, " s" );
    u8g.drawStr(24,40,dtostrf(inject_wait,1,1,buff));
    //vertical Line in box
    u8g.drawLine(90, 19, 90, 55);
    //horizontal line in box
    u8g.drawLine(90, 37, 110, 37);
    //up arrow in box
    u8g.drawLine(96, 28, 100, 24);
    u8g.drawLine(100, 24, 104, 28);
    //down arrow in box
    u8g.drawLine(96, 44, 100, 48);
    u8g.drawLine(100, 48, 104, 44);
  } while (u8g.nextPage());
}

void page_inject_wait() {
  u8g.firstPage();
  do {
    u8g.setColorIndex(1);
    u8g.drawStr( 0, 10 , "Inject");
    u8g.setColorIndex(1);
    //left arrow
    u8g.drawLine(0, 32, 4, 28);
    u8g.drawLine(0, 32, 4, 36);
    //draw box
    u8g.drawFrame(18, 16, 95, 40);
    //text
    u8g.setColorIndex(1);
    u8g.drawStr(45, 40, " s" );
    u8g.drawStr(24,40,dtostrf(inject_wait,1,1,buff));
    //vertical Line in box
    u8g.drawLine(90, 19, 90, 55);
    //horizontal line in box
    u8g.drawLine(90, 37, 110, 37);
    //up arrow in box
    u8g.drawLine(96, 28, 100, 24);
    u8g.drawLine(100, 24, 104, 28);
    //down arrow in box
    u8g.drawLine(96, 44, 100, 48);
    u8g.drawLine(100, 48, 104, 44);
  } while (u8g.nextPage());
}

void page_inject_run() {
  u8g.firstPage();
  do {
    u8g.setColorIndex(1);
    //text
    u8g.drawStr( 0, 10 , "Inject");
    //up arrow
    u8g.drawLine(60, 4, 64, 0);
    u8g.drawLine(64, 0, 68, 4);
    //down arrow
    u8g.drawLine(60, 60, 64, 64);
    u8g.drawLine(64, 64, 68, 60);
    //left arrow
    u8g.drawLine(0, 32, 4, 28);
    u8g.drawLine(0, 32, 4, 36);
    // draw box
    u8g.drawFrame(40, 19, 50, 30);
    //text
    u8g.drawStr( 53, 39 , "Run");
  } while (u8g.nextPage());
}

//*_______________Start motor function
void motor() {
  // Enables the motor to move in a particular direction
  //High set motor direction Clockwise (flush micropipet)
  if(dir == 0){
    digitalWrite(dirPin,HIGH);
  }
  else{
   digitalWrite(dirPin,LOW); // Low is counterClockwise
  }
  // Makes 5 pulses for making 0.1 microliter
    if(step_motor > 0){
      step_motor --;
    for(int x = 0; x < 5; x++) { 
    digitalWrite(stepPin,HIGH); 
    delayMicroseconds(8000); 
    digitalWrite(stepPin,LOW); 
    delayMicroseconds(8000); 
    }
    delay(delayTime);
    }
    else{
      digitalWrite(stepPin,LOW); 
    }
}

//_________________________Handle pin change interrupt for digital pins (buttons)
ISR (PCINT0_vect) {
  down_key++;
  if (down_key>=1.8) {
    down_key = 0;
    int num_key = 0;
    
    //button UP
    if (digitalRead(9)) {
      num_key  += 1;
    }
    //button RIGHT
    if (digitalRead(13)) {
      num_key += 2;
    }
    //button DOWN
    if (digitalRead(12)) {
      num_key += 4;
    }
    //button LEFT
        if (digitalRead(10)) {
          num_key += 8;
        }
    //button OK
    if (digitalRead(11)) {
      num_key += 16;
    }
    key_press(num_key);
  }
}

//_______________________________Handling function for  key press
void key_press(int num_key) {
  switch (current_page) {
    case 1:
      key_press_home(num_key);
      return;
      break;
    case 2:
      key_press_filling(num_key);
      return;
      break;
    case 3:
      key_press_clean(num_key);
      return;
      break;
    case 4:
      key_press_inject(num_key);
      return;
      break;
    case 5:
      key_press_setting(num_key);
      return;
      break;
    case 6:
      key_press_about(num_key);
      return;
      break;
    case 7:
      key_press_filling_value(num_key);
      return;
      break;
    case 8:
      key_press_change_inject_step(num_key);
      return;
      break;
     case 9:
      key_press_change_inject_wait(num_key);
      return;
      break;
      case 10:
      key_press_clean_run(num_key);
      return;
      break;
      case 11:
      key_press_inject_run(num_key);
      return;
      break;
      default:
      break;
  }
}

void key_press_home(int key) {
  switch (key) {
    // button UP
    case 1:
      scroll > 4 ? scroll = 0 : scroll ++;
      have_update = true;
      break;
    //button RIGHT
    case 2:
      switch (scroll) {
        case 0:
          // filling page
          next_page = 2;
          have_update = true;
          break;
        case 1:
          //clean page
          next_page = 3;
          have_update = true;
          break;
        case 2:
          //inject page
          next_page = 4;
          have_update = true;
          break;
        case 3:
          //setting page
          next_page = 5;
          have_update = true;
          break;
        case 4:
          //about page
          next_page = 6;
          have_update = true;
          break;
      }
      break;
    //button DOWN
    case 4:
      scroll < 0 ? scroll = 4 : scroll --;
      have_update = true;
      break;
    //button LEFT
    case 8:
      // no work
      break;
    //button OK
    case 16:
      switch (scroll) {
        case 0:
          // filling page
          next_page = 2;
          have_update = true;
          break;
        case 1:
          //clean page
          next_page = 3;
          have_update = true;
          break;
        case 2:
          //inject page
          next_page = 4;
          have_update = true;
          break;
        case 3:
          //setting page
          next_page = 5;
          have_update = true;
          break;
        case 4:
          //about page
          next_page = 6;
          have_update = true;
          break;
      }
      break;
    default:
      break;
  }
}

void key_press_filling(int key) {
  switch (key) {
    // button UP
    case 1:
      scroll > 1 ?  scroll = 0 : scroll ++;
      have_update = true;
      break;
    //button RIGHT
    case 2:
      switch(scroll){
        case 0: 
          next_page = 7; 
          have_update = true;  
        break;
      }
      break;
    //button DOWN
    case 4:
      scroll < 0 ?  scroll = 1 : scroll --;
      have_update = true;
      break;
    //button LEFT
    case 8:
      next_page = 1; 
      have_update = true;
      break;
    //button OK
    case 16:
      switch(scroll){
        case 0: 
          next_page = 7; 
          have_update = true;  
        break;
        case 1: 
          dir = true;   // false is flush pipet
          step_motor=filling_value*10;
          byte tem = get_pipet_step(); 
          if ((filling_value + tem)<9.0){  
            byte tt =  (filling_value + tem)*10;
            set_pipet_step(tt);
            next_page = 1; 
            have_update = true;  
          }
          delayTime = 1000;
          break;
      }
      break;
    default:
      break;
  }
}

void key_press_filling_value(int key){
   switch (key) {
    // button UP
    case 1:
      if (filling_value>=9.9){filling_value =0.0;}else{filling_value +=0.1;}
      have_update = true;
      break;
    //button RIGHT
    case 2:
      //no work
      break;
    //button DOWN
    case 4:
      if(filling_value<=0.0){filling_value = 9.9;}else{filling_value -= 0.1;}
      have_update = true;
      break;
    //button LEFT
    case 8:
      next_page = 2; 
      have_update = true;
      break;
    //button OK
    case 16:
      next_page = 2; 
      have_update = true;
      break;
    default:
      break;
  }
}

void  key_press_clean(int key) {
  switch (key) {
    // button UP
    case 1:
    scroll = 0;
    break;
    //button RIGHT
    case 2:
    //no work
      break;
    //button DOWN
    case 4:
    scroll = 0;
      break;
    //button LEFT
    case 8:
      next_page = 1; 
      have_update = true;
      break;
    //button OK
    case 16:
    next_page = 10;
    have_update = true;
    break;
    default:
      break;
  }
}

void key_press_clean_run(int key){
    switch (key) {
    // button UP
    case 1:
    //no work
     break;
    //button RIGHT
    case 2:
    //no work
      break;
    //button DOWN
    case 4:
    //no work
      break;
    //button LEFT
    case 8:
      next_page = 1; 
      have_update = true;
      break;
    //button OK
    case 16: 
      dir = false;    // false is flush pipet
      float tem = get_pipet_step();
      tem/=10;
      step_motor = tem*10;
      delayTime = 1000;
      set_pipet_step(0);
      next_page = 1; 
      have_update = true;  
      break;
    default:
      break;
 }
}

void key_press_inject(int key) {
  switch (key) {
    // button UP
    case 1:
      scroll > 2 ?  scroll = 0 : scroll ++;
      have_update = true;
      break;
    //button RIGHT
    case 2:
      switch(scroll){
        case 0: 
          next_page = 8; 
          have_update = true;  
        break;
       case 1: 
          next_page = 9; 
          have_update = true;  
        break;
        case 2:
        next_page = 11;
        have_update = true;
        break;
      }
      break;
    //button DOWN
    case 4:
      scroll < 0 ?  scroll = 2 : scroll --;
      have_update = true;
      break;
    //button LEFT
    case 8:
      next_page = 1; 
      have_update = true;
      break;
    //button OK
    case 16:
      switch(scroll){
        case 0: 
          next_page = 8; 
          have_update = true;  
        break;
        case 1: 
          next_page = 9; 
          have_update = true;  
        break;
        case 2:         
        //Edit mahdi          //page of inject run
        next_page = 11;
        have_update = true;
        break;
      }
      break;
    default:
      break;
  }
}

void key_press_change_inject_step(int key){
  switch (key) {
    // button UP
    case 1:
      if (inject_value>=9.9){inject_value =0.0;}else{inject_value +=0.1;}
      have_update = true;
      break;
    //button RIGHT
    case 2:
      //no work
      break;
    //button DOWN
    case 4:
      if(inject_value<=0.0){inject_value = 9.9;}else{inject_value -= 0.1;}
      have_update = true;
      break;
    //button LEFT
    case 8:
      next_page = 4; 
      have_update = true;
      break;
    //button OK
    case 16:
      next_page = 4; 
      have_update = true;
      break;
    default:
      break;
  }
}

void key_press_change_inject_wait(int key){
  switch (key) {
    // button UP
    case 1:
      if(inject_wait>=10){inject_wait=0;}else{inject_wait+=0.1;}
      have_update = true;
      break;
    //button RIGHT
    case 2:
      //no work
      break;
    //button DOWN
    case 4:
      if(inject_wait<=0){inject_wait=10;}else{inject_wait-=0.1;}
      have_update = true;
      break;
    //button LEFT
    case 8:
      next_page = 4; 
      have_update = true;
      break;
    //button OK
    case 16:
     next_page = 4; 
     have_update = true;
      break;
    default:
      break;
  }
}

//edittttttttttt
void key_press_inject_run(int key){
    switch (key) {
    // button UP
    case 1:
     break;
    //button RIGHT
    case 2:
      break;
    //button DOWN
    case 4:
      break;
    //button LEFT
    case 8:
      next_page = 4; 
      have_update = true;
      break;
    //button OK
    case 16:       
        //Edit mahdi          //page of inject run
        if (inject_value > filling_value){
          step_motor = 0;
        }
        dir = false;         //false is flush inject
        step_motor = inject_value*10;
        byte tem = get_pipet_step();
        if((tem-inject_value)>0){
          byte tt = (tem-inject_value)*10;
          set_pipet_step(tt);
          next_page = 1;
          have_update = true;
          }
          delayTime = inject_wait * 100;
      break;
    default:
      break;
 }
}

void key_press_setting(int key){
  switch (key) {
    // button UP
    case 1:
      break;
    //button RIGHT
    case 2:
      break;
    //button DOWN
    case 4:
      break;
    //button LEFT
    case 8:
      next_page = 1; 
      have_update = true;
      break;
    //button OK
    case 16:
      break;
    default:
      break;
  }
}

void key_press_about(int key) {
  switch (key) {
    // button UP
    case 1:
      break;
    //button RIGHT
    case 2:
      break;
    //button DOWN
    case 4:
      break;
    //button LEFT
    case 8:
      next_page = 1; 
      have_update = true;
      break;
    //button OK
    case 16:
      break;
    default:
      break;
  }
}

void Render () {
  if (have_update) {
    switch (next_page) {
      case 1:
        if (next_page == current_page ) {
          switch (scroll) {
            case 0:
              page_main_filling();
              break;
            case 1:
              page_main_clean();
              break;
            case 2:
              page_main_inject();
              break;
            case 3:
              page_main_setting();
              break;
            case 4:
              page_main_about();
              break;
            default:
              break;
          }
        } else {
          page_main_filling();
          scroll = 0;
        }
        break;
      case 2:
        if (next_page == current_page ) {
          switch (scroll) {
            case 0:
              main_fillingvalue_display();
              break;
            case 1:
              page_filling_run();
              break;
            default:
              break;
          }
        } else {
          main_fillingvalue_display();
          scroll = 0;
        }
        break;
      case 3:
        if (next_page == current_page ) {
          switch (scroll) {
            case 0:
              page_clean_run();
              break;
            default:
              break;
          }
        } else {
          page_clean_run();
          scroll = 0;
        }
        break;
      case 4:
        if (next_page == current_page ) {
          switch (scroll) {
            case 0:
              page_inject_step();
              break;
            case 1:
              page_inject_wait();
              break;
            case 2:
              page_inject_run();
              break;
            default:
              break;
          }
        } else {
          page_inject_step();
          scroll = 0;
        }
        break;
      case 5:
        if (next_page == current_page ) {
          switch (scroll) {
            case 0:
              page_main_setting();
              break;
            default:
              break;
          }
        } else {
          page_main_setting();
          scroll = 0;
        }
        break;
      case 6:
        if (next_page == current_page ) {
          switch (scroll) {
            case 0:
              page_main_about();
              break;
            default:
              break;
          }
        } else {
          page_main_about();
          scroll = 0;
        }
        break;
      case 7:
        if (next_page == current_page ) {
            switch (scroll) {
              case 0:
                change_fillingvalue_display();
                break;
              default:
                break;
            }
          } else {
            change_fillingvalue_display();
            scroll = 0;
          }
        break;
        case 8:
        if (next_page == current_page ) {
            switch (scroll) {
              case 0:
                change_inject_step();
                break;
              default:
                break;
            }
          } else {
            change_inject_step();
            scroll = 0;
          }
        break;
        case 9:
        if (next_page == current_page ) {
            switch (scroll) {
              case 0:
                change_inject_wait();
                break;
              default:
                break;
            }
          } else {
            change_inject_wait();
            scroll = 0;
          }
          break;
        case 10:
        if(next_page == current_page){
          switch(scroll){
            case 0:
            page_clean_run();
            break;
            default:
            break;
          }
        }else{
          page_clean_run();
          scroll = 0;
        }
        break;
        case 11:
        if(next_page == current_page){
          switch(scroll){
            case 0:
          page_inject_run();
            break;
            default:
            break;
          }
        }else{
          page_inject_run();
          scroll = 0;
        }
        break;
      default:
        break;
    }
    current_page = next_page;
    have_update = false;
  }
}

void setup() {  
 // when on first program run this code & after that comment this code & program
    //____________
    set_pipet_step(0);
    filling_value = get_pipet_step(); 
    filling_value /= 10 ; 
    delay(1000);
  //_____________
  u8g.setFont(u8g_font_unifont);
  u8g.setColorIndex(1);
  //_____________________________pin mode for run motor
  pinMode(stepPin,OUTPUT); 
  pinMode(dirPin,OUTPUT);
  // button up
  pciSetup(9);
  // button down
  pciSetup(12);
  // button right
  pciSetup(13);
  // button left
  pciSetup(10);
  // button ok
  pciSetup(11);
  //Lodaing Page
  page_loading();
  delay(3000);
  next_page = 1;
  have_update = true;
}

void loop() {
 Render();
 motor();
}
