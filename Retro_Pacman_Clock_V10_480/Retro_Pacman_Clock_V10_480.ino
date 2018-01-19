 /*  Retro Pac-Man Clock
 Author: @TechKiwiGadgets Date 08/04/2017
  V5 
  - introduction of optional Backlight Dimmer Code
  - fix issues with Scoreboard display
 V6 
  - Introduce TEST button on Alarm screen to enable playing alarm sound
 V7 
 - Fix AM/PM error at midday
 V8
 - Add Ms Pac-Man feature in Setup menu
 V9
 - Fix issue with Ms Pac-Man leaving a trail at corners
 - Randomseed will shuffle the random function using Analog pin 0 
 V10 
 - Finally Retro PacMan Dots added and traditional gameplay- enjoy!!
 *****************************************************
 *Modified by Larry Bonnette for use with 480X320 TFT
 *****************************************************
*/
 
#include <UTFT.h> 
#include <URTouch.h>
#include <EEPROM.h>
#include <Time.h>  
#include <Wire.h>  
#include <DS1307RTC.h>  // a basic DS1307 library that returns time as a time_t

// Alarm Variables
boolean alarmstatus = false; // flag where false is off and true is on
boolean soundalarm = false; // Flag to indicate the alarm needs to be initiated
int alarmhour = 0;  // hour of alarm setting
int alarmminute = 0; // Minute of alarm setting
byte ahour; //Byte variable for hour
byte amin; //Byte variable for minute
int actr = 300; // When alarm sounds this is a counter used to reset sound card until screen touched
int act = 0;

boolean mspacman = false;  //  if this is is set to true then play the game as Ms Pac-man
// Changed Larry
//Dot Array - There are 112 Dots with 4 of them that will turn Ghost Blue!

byte dot[113]; // Where if dot is zero then has been gobbled by Pac-Man



// Initializes RTC time values: 
const int DS1307 = 0x68; // Address of DS1307 see data sheets

// Display Dimmer Variables
int dimscreen = 255; // This variable is used to drive the screen brightness where 255 is max brightness
int LDR = 100; // LDR variable measured directly from Analog 7

//==== Creating Objects

UTFT myGLCD(ILI9341_16,38,39,40,41); //Parameters should be adjusted to your Display/Schield model
//UTFT    myGLCD(SSD1289,38,39,40,41); //Parameters should be adjusted to your Display/Schield model
URTouch  myTouch( 6, 5, 4, 3, 2);


//==== Defining Fonts
extern uint8_t SmallFont[];
extern uint8_t BigFont[];
extern uint8_t SevenSegNumFont[];
extern uint8_t SevenSeg_XXXL_Num[];

extern unsigned int c_pacman[0x310]; // Ghost Bitmap Straight ahead


extern unsigned int ms_c_pacman_u[0x310]; // Ghost Bitmap Straight ahead
extern unsigned int ms_c_pacman_d[0x310]; // Ghost Bitmap Straight ahead
extern unsigned int ms_c_pacman_l[0x310]; // Ghost Bitmap Straight ahead
extern unsigned int ms_c_pacman_r[0x310]; // Ghost Bitmap Straight ahead

extern unsigned int ms_d_m_pacman[0x310]; // Ghost Bitmap Straight ahead
extern unsigned int ms_d_o_pacman[0x310]; // Ghost Bitmap Straight ahead
extern unsigned int ms_l_m_pacman[0x310]; // Ghost Bitmap Straight ahead
extern unsigned int ms_l_o_pacman[0x310]; // Ghost Bitmap Straight ahead
extern unsigned int ms_r_m_pacman[0x310]; // Ghost Bitmap Straight ahead
extern unsigned int ms_r_o_pacman[0x310]; // Ghost Bitmap Straight ahead
extern unsigned int ms_u_m_pacman[0x310]; // Ghost Bitmap Straight ahead
extern unsigned int ms_u_o_pacman[0x310]; // Ghost Bitmap Straight ahead

extern unsigned int d_m_pacman[0x310]; // Ghost Bitmap Straight ahead
extern unsigned int d_o_pacman[0x310]; // Ghost Bitmap Straight ahead
extern unsigned int l_m_pacman[0x310]; // Ghost Bitmap Straight ahead
extern unsigned int l_o_pacman[0x310]; // Ghost Bitmap Straight ahead
extern unsigned int r_m_pacman[0x310]; // Ghost Bitmap Straight ahead
extern unsigned int r_o_pacman[0x310]; // Ghost Bitmap Straight ahead
extern unsigned int u_m_pacman[0x310]; // Ghost Bitmap Straight ahead
extern unsigned int u_o_pacman[0x310]; // Ghost Bitmap Straight ahead



extern unsigned int ru_ghost[0x310]; // Ghost Bitmap Straight ahead
extern unsigned int rd_ghost[0x310]; // Ghost Bitmap Straight ahead
extern unsigned int rl_ghost[0x310]; // Ghost Bitmap Straight ahead
extern unsigned int rr_ghost[0x310]; // Ghost Bitmap Straight ahead
extern unsigned int fruit[0x310]; // Ghost Bitmap Straight ahead
extern unsigned int bluepacman[0x310]; // Ghost Bitmap Straight ahead

// Touch screen coordinates
boolean screenPressed = false;
int xT,yT;
int userT = 4; // flag to indicate directional touch on screen
boolean setupscreen = false; // used to access the setup screen

// Fruit flags
boolean fruitgone = false;
boolean fruitdrawn = false;
boolean fruiteatenpacman = false;

//Pacman & Ghost kill flags
boolean pacmanlost = false;
boolean ghostlost = false;

//Alarm setup variables
boolean xsetup = false; // Flag to determine if existing setup mode

// Scorecard
int pacmanscore = 0;
int ghostscore = 0;

// Animation delay to slow movement down
int dly = 18; // Orignally 30

// Time Refresh counter 
int rfcvalue = 900; // wait this long untiul check time for changes
int rfc = 1;

// Pacman coordinates of top LHS of 28x28 bitmap
int xP = 4;
int yP = 148;
int P = 0;  // Pacman Graphic Flag 0 = Closed, 1 = Medium Open, 2 = Wide Open, 3 = Medium Open
int D = 0;  // Pacman direction 0 = right, 1 = down, 2 = left, 3 = up
int prevD;  // Capture legacy direction to enable adequate blanking of trail
int direct;   //  Random direction variable

// Ghost coordinates of top LHS of 28x28 bitmap
int xG = 452;
int yG = 148;
int GD = 2;  // Ghost direction 0 = right, 1 = down, 2 = left, 3 = up
int prevGD;  // Capture legacy direction to enable adequate blanking of trail
int gdirect;   //  Random direction variable 

// Declare global variables for previous time,  to enable refesh of only digits that have changed
// There are four digits that need to be drawn independently to ensure consisitent positioning of time
  int c1 = 20;  // Tens hour digit
  int c2 = 20;  // Ones hour digit
  int c3 = 20;  // Tens minute digit
  int c4 = 20;  // Ones minute digit


void setup() {

// Initialize Dot Array Larry
  for (int dotarray = 1; dotarray < 113; dotarray++) {    
    dot[dotarray] = 1;    
    }
  
//Initialize RTC
    Serial.begin(9600);
  // while (!Serial) ; // wait until Arduino Serial Monitor opens
  delay(200);
  setSyncProvider(RTC.get);   // the function to get the time from the RTC
  setSyncInterval(60); // sync the time every 60 seconds (1 minutes)
  if(timeStatus()!= timeSet){ 
     Serial.println("Unable to sync with the RTC");
     RTC.set(1408278800); // set the RTC to Aug 25 2014 9:00 am
     setTime(1408278800);
    }
    else{
     Serial.println("RTC has set the system time");   
    }

// Setup Alarm enable pin to play back sound on the ISD1820 board
   pinMode(8, OUTPUT); // D8 used to toggle sound
   digitalWrite(8,LOW);  // Set to low to turn off sound
   
// Setup Pin 9 of Arduino to LED A control brightness of the backlight. 
// This is done by firstly disconnecting Pin 19 of the TFT screen
// Followed by connecting D9 of Arduino to Pin 19 of TFT via a 47 ohm resistor
   pinMode(9, OUTPUT); // D9 used to PWM voltage to backlight on TFT screen
   analogWrite(9, 255); // Controls brightness 0 is Dark, Ambient room is approx 25 and 70 is direct sunlight
   
// Randomseed will shuffle the random function
randomSeed(analogRead(0));

  // Initiate display
  myGLCD.InitLCD();
  myGLCD.clrScr();
  myTouch.InitTouch();
  myTouch.setPrecision(PREC_LOW);
  
 
  drawscreen(); // Initiate the game
  UpdateDisp(); // update value to clock 


}

void loop() {

// Set Screen Brightness
// Check the ambient light and adjust LED brightness to suit Ambient approx 500 dark is below 100
LDR = analogRead(A7);

/* Test value range of LDR
  myGLCD.setColor(237, 28, 36);
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.printNumI(LDR,250,60,3);

if (LDR >=121){
    dimscreen = 255;
   } 
   
if (LDR <=120)   {  
    dimscreen = 20;
   }    
*/
/*
//Print positions for testing
  myGLCD.setFont(BigFont);
  myGLCD.setColor(0, 0, 0);
  myGLCD.setBackColor(114, 198, 206);
  myGLCD.printNumI(xG,228,200); // Print xP
  myGLCD.printNumI(yG,228,220); // Print yP
*/

dimscreen = (LDR/4)+5;

if (dimscreen >= 255){
  dimscreen = 255;
  }
analogWrite(9, dimscreen); // Controls brightness 0 is Dark, Ambient room is approx 25 and 70 is direct sunlight 
  
  
//Print scoreboard

if((ghostscore >= 95)||(pacmanscore >= 95)){ // Reset scoreboard if over 95
ghostscore = 0;
pacmanscore = 0;

  for (int dotarray = 1; dotarray < 113; dotarray++) {
    
    dot[dotarray] = 1;
    
    }

// Blank the screen across the digits before redrawing them
  myGLCD.setColor(0, 0, 0);
  myGLCD.setBackColor(0, 0, 0);

//    myGLCD.fillRect(299, 87, 314, 97); // Blankout ghost score  
    myGLCD.fillRect(399, 87, 414, 97); // Blankout ghost score diff=6
//    myGLCD.fillRect(7, 87, 22, 97);   // Blankout pacman score
    myGLCD.fillRect(107, 87, 122, 97);   // Blankout pacman score

drawscreen(); // Redraw dots  
}
  
  myGLCD.setFont(SmallFont);

// Account for position issue if over or under 10

if (ghostscore >= 10){
  myGLCD.setColor(237, 28, 36);
  myGLCD.setBackColor(0, 0, 0);
//  myGLCD.printNumI(ghostscore,299,87);
  myGLCD.printNumI(ghostscore,459,127);
} else {
  myGLCD.setColor(237, 28, 36);
  myGLCD.setBackColor(0, 0, 0);
//  myGLCD.printNumI(ghostscore,307,87);  // Account for being less than 10
  myGLCD.printNumI(ghostscore,467,127);  // Account for being less than 10
}

if (pacmanscore >= 10){
  myGLCD.setColor(248, 236, 1);
  myGLCD.setBackColor(0, 0, 0);
//  myGLCD.printNumI(pacmanscore,7,87);  
  myGLCD.printNumI(pacmanscore,7,127);  

} else{
  myGLCD.setColor(248, 236, 1);
  myGLCD.setBackColor(0, 0, 0);
//  myGLCD.printNumI(pacmanscore,15,87);  // Account for being less than 10
  myGLCD.printNumI(pacmanscore,15,127);  // Account for being less than 10
} 
  
  // Draw fruit
if ((fruitdrawn == false)&&(fruitgone == false)){ // draw fruit and set flag that fruit present so its not drawn again
//    myGLCD.drawBitmap (146, 168, 28, 28, fruit); //   draw fruit 
    myGLCD.drawBitmap (229, 319, 28, 28, fruit); //   draw fruit 
    fruitdrawn = true;
}  

// Redraw fruit if Ghost eats fruit only if Ghost passesover 172 or 120 on the row 168
if ((fruitdrawn == true)&&(fruitgone == false)&&(xG >= 262)&&(xG <= 264)&&(yG >= 248)&&(yG <= 260)){
    myGLCD.drawBitmap (229, 319, 28, 28, fruit); //   draw fruit  
}

if ((fruitdrawn == true)&&(fruitgone == false)&&(xG == 192)&&(yG == 248)){
    myGLCD.drawBitmap (229, 319, 28, 28, fruit); //   draw fruit  
}

// Award Points if Pacman eats Big Dots
if ((fruitdrawn == true)&&(fruitgone == false)&&(xP == 228)&&(yP == 248)){
  fruitgone = true; // If Pacman eats fruit then fruit disappears  
  pacmanscore = pacmanscore + 5; //Increment pacman score 
}





// Read the current date and time from the RTC and reset board
rfc++;
  if (rfc >= rfcvalue) { // count cycles and print time
    UpdateDisp(); // update value to clock then ...
     fruiteatenpacman =  false; // Turn Ghost red again  
     fruitdrawn = false; // If Pacman eats fruit then fruit disappears
     fruitgone = false;
     // Reset every minute both characters
     pacmanlost = false;
     ghostlost = false;
     dly = 18; // reset delay
     rfc = 0;
     
  }

//=== Check if Alarm needs to be sounded
   if (alarmstatus == true){  
     if ( (alarmhour == hour()) && (alarmminute == minute())) {  // Sound the alarm        
           soundalarm = true;
       }     
   }

//=== Start Alarm Sound - Sound pays for 10 seconds then will restart at 20 second mark

if ((alarmstatus == true)&&(soundalarm==true)){ // Set off a counter and take action to restart sound if screen not touched

    if (act == 0) { // Set off alarm by toggling D8, recorded sound triggered by LOW to HIGH transition
        digitalWrite(8,HIGH); // Set high
        digitalWrite(8,LOW); // Set low
        UpdateDisp(); // update value to clock 
    }
    act = act +1;
   
    if (act == actr) { // Set off alarm by toggling D8, recorded sound triggered by LOW to HIGH transition
        digitalWrite(8,HIGH); // Set high
        digitalWrite(8,LOW); // Set low
        act = 0; // Reset counter hopfully every 20 seconds
    } 

}

// Check if user input to touch screen
// UserT sets direction 0 = right, 1 = down, 2 = left, 3 = up, 4 = no touch input


     myTouch.read();
 if (myTouch.dataAvailable() && !screenPressed) {
    xT = myTouch.getX();
    yT = myTouch.getY();        
    // Capture direction request from user
    if ((xT>=1) && (xT<=80) && (yT>=80) && (yT<=160)) { // Left
        userT = 2; // Request to go left   
    }
    if ((xT>=240) && (xT<=318) && (yT>=80) && (yT<=160)) { // Right
        userT = 0; // Request to go right   
    }
    if ((xT>=110) && (xT<=210) && (yT>=1) && (yT<=80)) { // Up
        userT = 3; // Request to go Up   
    }
     if ((xT>=110) && (xT<=210) && (yT>=160) && (yT<=238)) { // Down
        userT = 1; // Request to go Down   
    }
 
 // **********************************
 // ******* Enter Setup Mode *********
 // **********************************
 
    if (((xT>=120) && (xT<=200) && (yT>=105) && (yT<=140)) &&  (soundalarm !=true)) { // Call Setup Routine if alarm is not sounding
        xsetup = true;  // Toggle flag
        clocksetup(); // Call Clock Setup Routine 
        UpdateDisp(); // update value to clock
        
    } else  // If centre of screen touched while alarm sounding then turn off the sound and reset the alarm to not set 
    
    if (((xT>=120) && (xT<=200) && (yT>=105) && (yT<=140)) && ((alarmstatus == true) && (soundalarm ==true))) {
     
      alarmstatus = false;
      soundalarm = false;
      digitalWrite(8,LOW); // Set low
    } 
    
    
 
    if(pacmanlost == false){ // only apply requested changes if Pacman still alive

       if (userT == 2 && D == 0 ){ // Going Right request to turn Left OK
         D = 2;
         }
       if (userT == 0 && D == 2 ){ // Going Left request to turn Right OK
         D = 0;
         }
       if (userT == 1 && D == 3 ){ // Going Up request to turn Down OK
         D = 1;
         }
       if (userT == 3 && D == 1 ){ // Going Down request to turn Up OK
         D = 3;
         }
      }
         screenPressed = true;
   }
    // Doesn't allow holding the screen / you must tap it
    else if ( !myTouch.dataAvailable() && screenPressed){
      screenPressed = false;
   }

// Pacman Captured
// If pacman captured then pacman dissapears until reset
if ((fruiteatenpacman == false)&&(abs(xG-xP)<=5)&&(abs(yG-yP)<=5)){ 
// firstly blank out Pacman
    myGLCD.setColor(0,0,0);
    myGLCD.fillRect(xP, yP, xP+27, yP+27); 

  if (pacmanlost == false){
    ghostscore = ghostscore + 15;  
  }
  pacmanlost = true;
 // Slow down speed of drawing now only one moving charater
  dly = 28;
  }
 
if (pacmanlost == false) { // Only draw pacman if he is still alive


// Draw Pac-Man
drawPacman(xP,yP,P,D,prevD); // Draws Pacman at these coordinates
  

// If Pac-Man is on a dot then print the adjacent dots if they are valid

  myGLCD.setColor(200, 200, 200);
  
// Check Rows

if (yP== 4) {  // if in Row 1 **********************************************************
  if (xP== 4) { // dot 1
     if (dot[2] == 1) {  // Check if dot 2 gobbled already
	  myGLCD.fillCircle(42, 19, 2); // dot 2
     }    
      if (dot[19] == 1) {  // Check if dot 19 gobbled already
  	myGLCD.fillCircle(19, 40, 7); // Big dot 19
     }    

  } else
  if (xP== 28) { // dot 2
     if (dot[1] == 1) {  // Check if dot 1 gobbled already
	  myGLCD.fillCircle(19, 19, 2); // dot 1
     }    
      if (dot[3] == 1) {  // Check if dot 3 gobbled already
  	myGLCD.fillCircle(65, 19, 2); // dot 3
     }    

  } else
  if (xP== 52) { // dot 3
     if (dot[2] == 1) {  // Check if dot 2 gobbled already
	  myGLCD.fillCircle(42, 19, 2); // dot 2
     }    
      if (dot[4] == 1) {  // Check if dot 4 gobbled already
  	myGLCD.fillCircle(88, 19, 2); // dot 4
     } 
      if (dot[20] == 1) {  // Check if dot 20 gobbled already
  	myGLCD.fillCircle(77, 40, 2);  // dot 20
     }   
  } else
  if (xP== 74) { // dot 4
     if (dot[3] == 1) {  // Check if dot 3 gobbled already
  	myGLCD.fillCircle(65, 19, 2); // dot 3
     }    
      if (dot[5] == 1) {  // Check if dot 5 gobbled already
  	myGLCD.fillCircle(112, 19, 2); // dot 5
     }   
      if (dot[20] == 1) {  // Check if dot 20 gobbled already
  	myGLCD.fillCircle(77, 40, 2);  // dot 20
     }    
  } else
  if (xP== 98) { // dot 5
     if (dot[4] == 1) {  // Check if dot 4 gobbled already
  	myGLCD.fillCircle(88, 19, 2); // dot 4
     }    
      if (dot[6] == 1) {  // Check if dot 6 gobbled already
  	myGLCD.fillCircle(136, 19, 2); // dot 6
     }     
  } else
  if (xP== 120) { // dot 6
     if (dot[5] == 1) {  // Check if dot 5 gobbled already
  	myGLCD.fillCircle(112, 19, 2); // dot 5
     }    
      if (dot[21] == 1) {  // Check if dot 21 gobbled already
  	myGLCD.fillCircle(136, 40, 2);  // dot 21
     }     
  } else
 

 if (xP== 168) { // dot 7
      if (dot[22] == 1) {  // Check if dot 22 gobbled already
  	myGLCD.fillCircle(183, 40, 2);  // dot 22
     }    
      if (dot[8] == 1) {  // Check if dot 8 gobbled already
  	myGLCD.fillCircle(206, 19, 2); // dot 8
     }    
  } else
  if (xP== 192) { // dot 8
      if (dot[7] == 1) {  // Check if dot 7 gobbled already
  	myGLCD.fillCircle(183, 19, 2); // dot 7
     }    
      if (dot[9] == 1) {  // Check if dot 9 gobbled already
  	myGLCD.fillCircle(229, 19, 2); // dot 9
     }    
  } else
  if (xP== 216) { // dot 9
      if (dot[10] == 1) {  // Check if dot 10 gobbled already
  	myGLCD.fillCircle(252, 19, 2); // dot 10
     }    
      if (dot[8] == 1) {  // Check if dot 8 gobbled already
  	myGLCD.fillCircle(206, 19, 2); // dot 8
     }      
      if (dot[23] == 1) {  // Check if dot 23 gobbled already
  	myGLCD.fillCircle(241, 40, 2);  // dot 23
     }    
 } else
  if (xP== 238) { // dot 10
      if (dot[11] == 1) {  // Check if dot 11 gobbled already
  	myGLCD.fillCircle(275, 19, 2); // dot 11
     }    
      if (dot[9] == 1) {  // Check if dot 9 gobbled already
  	myGLCD.fillCircle(229, 19, 2); // dot 9
     }      
      if (dot[23] == 1) {  // Check if dot 23 gobbled already
  	myGLCD.fillCircle(241, 40, 2);  // dot 23
     }
  } else
  if (xP== 262) { // dot 11
      if (dot[10] == 1) {  // Check if dot 10 gobbled already
  	myGLCD.fillCircle(252, 19, 2); // dot 10
     }    
      if (dot[12] == 1) {  // Check if dot 12 gobbled already
  	myGLCD.fillCircle(298, 19, 2); // dot 12
     }    
  } else
  if (xP== 284) { // dot 12
      if (dot[11] == 1) {  // Check if dot 11 gobbled already
  	myGLCD.fillCircle(275, 19, 2); // dot 11
     }    
      if (dot[24] == 1) {  // Check if dot 24 gobbled already
  	myGLCD.fillCircle(298, 40, 2);  // dot 24
     }  
  }else
 if (xP== 332) { // dot 13
      if (dot[25] == 1) {  // Check if dot 25 gobbled already
   myGLCD.fillCircle(347, 40, 2);  // dot 25
     }    
      if (dot[14] == 1) {  // Check if dot 14 gobbled already
    myGLCD.fillCircle(368, 19, 2);  // dot 14
     }    
  } else
  if (xP== 356) { // dot 14
      if (dot[13] == 1) {  // Check if dot 13 gobbled already
    myGLCD.fillCircle(345, 19, 2);  // dot 13
     }    
      if (dot[15] == 1) {  // Check if dot 15 gobbled already
    myGLCD.fillCircle(391, 19, 2);  // dot 15
     }    
  } else
  if (xP== 380) { // dot 15
      if (dot[14] == 1) {  // Check if dot 14 gobbled already
    myGLCD.fillCircle(368, 19, 2);  // dot 14
     }    
      if (dot[26] == 1) {  // Check if dot 26 gobbled already
    myGLCD.fillCircle(406, 40, 2);  // dot 26
     }      
      if (dot[16] == 1) {  // Check if dot 16 gobbled already
    myGLCD.fillCircle(414, 19, 2);  // dot 16
     }    
 } else
  if (xP== 402) { // dot 16
      if (dot[15] == 1) {  // Check if dot 15 gobbled already
    myGLCD.fillCircle(391, 19, 2);  // dot 15
     }    
      if (dot[26] == 1) {  // Check if dot 26 gobbled already
    myGLCD.fillCircle(406, 40, 2);  // dot 26
     }      
      if (dot[17] == 1) {  // Check if dot 17 gobbled already
    myGLCD.fillCircle(437, 19, 2);  // dot 17
     }  
  } else
  if (xP== 424) { // dot 17
      if (dot[16] == 1) {  // Check if dot 16 gobbled already
    myGLCD.fillCircle(414, 19, 2);  // dot 16
     }    
      if (dot[18] == 1) {  // Check if dot 18 gobbled already
    myGLCD.fillCircle(460, 19, 2);  // dot 18
     }    
  } else
  if (xP== 448) { // dot 18
      if (dot[17] == 1) {  // Check if dot 17 gobbled already
    myGLCD.fillCircle(437, 19, 2);  // dot 17
     }    
      if (dot[27] == 1) {  // Check if dot 27 gobbled already
    myGLCD.fillCircle(465, 40, 7); // Big dot 27
     }  
  } 
} else 
if (yP== 26) {  // if in Row 2  **********************************************************
  if (xP== 4) { // dot 19
     if (dot[1] == 1) {  // Check if dot 1 gobbled already
	  myGLCD.fillCircle(19, 19, 2); // dot 1
     }    
      if (dot[28] == 1) {  // Check if dot 28 gobbled already
  	myGLCD.fillCircle(19, 60, 2); // Dot 28
     }   
  } else
  
    if (xP== 62) { // dot 20
      if (dot[3] == 1) {  // Check if dot 3 gobbled already
  	myGLCD.fillCircle(65, 19, 2); // dot 3
     }   
         if (dot[4] == 1) {  // Check if dot 4 gobbled already
  	myGLCD.fillCircle(88, 19, 2); // dot 4
     } 
         if (dot[30] == 1) {  // Check if dot 30 gobbled already
  	myGLCD.fillCircle(65, 60, 2); //Dot 30
     }   
      if (dot[31] == 1) {  // Check if dot 31 gobbled already
  	myGLCD.fillCircle(88, 60, 2); // Dot 31
     }    
     
  } else
  
  if (xP== 120) { // dot 21
     if (dot[33] == 1) {  // Check if dot 33 gobbled already
  	myGLCD.fillCircle(136, 60, 2); // Dot 33
     }    
      if (dot[6] == 1) {  // Check if dot 6 gobbled already
  	myGLCD.fillCircle(136, 19, 2); // dot 6
     }      
  } else
  if (xP== 168) { // dot 22
      if (dot[7] == 1) {  // Check if dot 7 gobbled already
  	myGLCD.fillCircle(183, 19, 2); // dot 7
     }    
      if (dot[35] == 1) {  // Check if dot 35 gobbled already
  	myGLCD.fillCircle(183, 60, 2); // Dot 35
     }         
  } else
    if (xP== 228) { // dot 23
      if (dot[9] == 1) {  // Check if dot 9 gobbled already
  	myGLCD.fillCircle(229, 19, 2); // dot 9
     }      
       if (dot[10] == 1) {  // Check if dot 10 gobbled already
  	myGLCD.fillCircle(252, 19, 2); // dot 10
     }  
      if (dot[37] == 1) {  // Check if dot 37 gobbled already
	  myGLCD.fillCircle(229, 60, 2); // Dot 37
     }  
       if (dot[38] == 1) {  // Check if dot 38 gobbled already
  	myGLCD.fillCircle(252, 60, 2); // Dot 38
     }      
     
  } else
  if (xP== 284) { // dot 24
      if (dot[12] == 1) {  // Check if dot 12 gobbled already
  	myGLCD.fillCircle(298, 19, 2);  // dot 12
     }    
      if (dot[40] == 1) {  // Check if dot 40 gobbled already
  	myGLCD.fillCircle(298, 60, 2); // Dot 40
     }// ****************** Add decisions Larry ********************   
  } else
  if (xP== 332) { // dot 25
      if (dot[13] == 1) {  // Check if dot 13 gobbled already
   myGLCD.fillCircle(345, 19, 2);  // dot 13
     }    
      if (dot[42] == 1) {  // Check if dot 42 gobbled already
    myGLCD.fillCircle(344, 60, 2); // Dot 42
     }       
  } else
    if (xP== 394) { // dot 26
      if (dot[15] == 1) {  // Check if dot 15 gobbled already
    myGLCD.fillCircle(391, 19, 2);  // dot 15
     }      
       if (dot[16] == 1) {  // Check if dot 16 gobbled already
    myGLCD.fillCircle(414, 19, 2);  // dot 16
     }  
      if (dot[44] == 1) {  // Check if dot 44 gobbled already
    myGLCD.fillCircle(390, 60, 2); // Dot 44
     }  
       if (dot[45] == 1) {  // Check if dot 45 gobbled already
    myGLCD.fillCircle(413, 60, 2); // Dot 45
     }     
       
  } else
  if (xP== 448) { // dot 27
      if (dot[18] == 1) {  // Check if dot 18 gobbled already
    myGLCD.fillCircle(460, 19, 2);  // dot 18
     }    
      if (dot[47] == 1) {  // Check if dot 47 gobbled already
    myGLCD.fillCircle(459, 60, 2); //Dot 47 
     }   
  }
} else
if (yP== 46) {  // if in Row 3  **********************************************************
  if (xP== 4) { // dot 28
     if (dot[19] == 1) {  // Check if dot 19 gobbled already
	  myGLCD.fillCircle(19, 40, 7); // Big dot 19
     }    
      if (dot[29] == 1) {  // Check if dot 29 gobbled already
  	myGLCD.fillCircle(42, 60, 2); // Dot 29
     }  
  } else
  if (xP== 28) { // dot 29
     if (dot[28] == 1) {  // Check if dot 28 gobbled already
  	myGLCD.fillCircle(19, 60, 2); // Dot 28
     }    
      if (dot[48] == 1) {  // Check if dot 48 gobbled already
  	myGLCD.fillCircle(42, 80, 2); // Dot 48
     }   
      if (dot[30] == 1) {  // Check if dot 30 gobbled already
  	myGLCD.fillCircle(65, 60, 2); //Dot 30
     }    
  } else
  if (xP== 52) { // dot 30
     if (dot[29] == 1) {  // Check if dot 29 gobbled already
	 myGLCD.fillCircle(42, 60, 2); // Dot 29
     }    
      if (dot[20] == 1) {  // Check if dot 20 gobbled already
  	myGLCD.fillCircle(77, 40, 2);  // dot 20
     } 
      if (dot[31] == 1) {  // Check if dot 31 gobbled already
  	myGLCD.fillCircle(88, 60, 2); // Dot 31
     }         
  } else
  if (xP== 74) { // dot 31
      if (dot[30] == 1) {  // Check if dot 30 gobbled already
  	myGLCD.fillCircle(65, 60, 2); //Dot 30
     }    
      if (dot[20] == 1) {  // Check if dot 20 gobbled already
  	myGLCD.fillCircle(77, 40, 2);  // dot 20
     } 
      if (dot[32] == 1) {  // Check if dot 32 gobbled already
  	myGLCD.fillCircle(112, 60, 2); // Dot 32
     }   
  } else
  if (xP== 98) { // dot 32
     if (dot[31] == 1) {  // Check if dot 31 gobbled already
	  myGLCD.fillCircle(88, 60, 2); // Dot 31
     }    
      if (dot[33] == 1) {  // Check if dot 33 gobbled already
  	myGLCD.fillCircle(136, 60, 2); // Dot 33
     }  
    
  } else
  if (xP== 120) { // dot 33
      if (dot[21] == 1) {  // Check if dot 21 gobbled already
  	myGLCD.fillCircle(136, 40, 2);  // dot 21
     }    
      if (dot[32] == 1) {  // Check if dot 32 gobbled already
  	myGLCD.fillCircle(112, 60, 2); // Dot 32
     }
      if (dot[34] == 1) {  // Check if dot 34 gobbled already
  	myGLCD.fillCircle(160, 60, 2); // Dot 34
     }        
  } else
  if (xP== 146) { // dot 34
     if (dot[33] == 1) {  // Check if dot 33 gobbled already
	  myGLCD.fillCircle(136, 60, 2); // Dot 33
     }    
      if (dot[35] == 1) {  // Check if dot 35 gobbled already
  	myGLCD.fillCircle(183, 60, 2); // Dot 35
     }    
  } else
  if (xP== 168) { // dot 35
      if (dot[34] == 1) {  // Check if dot 34 gobbled already
  	myGLCD.fillCircle(160, 60, 2); // Dot 34
     }    
      if (dot[22] == 1) {  // Check if dot 22 gobbled already
  	myGLCD.fillCircle(183, 40, 2);  // dot 22
     }
      if (dot[36] == 1) {  // Check if dot 36 gobbled already
  	myGLCD.fillCircle(206, 60, 2); // Dot 36
     }    
  } else
  if (xP== 192) { // dot 36
     if (dot[35] == 1) {  // Check if dot 35 gobbled already
	  myGLCD.fillCircle(183, 60, 2); // Dot 35
     }    
      if (dot[37] == 1) {  // Check if dot 37 gobbled already
  	myGLCD.fillCircle(229, 60, 2); // Dot 37
     }      
  } else
  if (xP== 216) { // dot 37
      if (dot[36] == 1) {  // Check if dot 36 gobbled already
  	myGLCD.fillCircle(206, 60, 2); // Dot 36
     }    
      if (dot[23] == 1) {  // Check if dot 23 gobbled already
  	myGLCD.fillCircle(241, 40, 2);  // dot 23
     }      
      if (dot[38] == 1) {  // Check if dot 38 gobbled already
  	myGLCD.fillCircle(252, 60, 2); // Dot 38
     }  
  } else
  if (xP== 238) { // dot 38
     if (dot[37] == 1) {  // Check if dot 37 gobbled already
	  myGLCD.fillCircle(229, 60, 2); // Dot 37
     }    
      if (dot[23] == 1) {  // Check if dot 23 gobbled already
  	myGLCD.fillCircle(241, 40, 2);  // dot 23
     }      
      if (dot[39] == 1) {  // Check if dot 39 gobbled already
  	myGLCD.fillCircle(275, 60, 2); // Dot 39
     }    

  } else
  if (xP== 262) { // dot 39
      if (dot[38] == 1) {  // Check if dot 38 gobbled already
  	myGLCD.fillCircle(252, 60, 2); // Dot 38
     }    
      if (dot[40] == 1) {  // Check if dot 40 gobbled already
  	myGLCD.fillCircle(298, 60, 2); // Dot 40
     }      
  } else
  if (xP== 284) { // dot 40
   if (dot[39] == 1) {  // Check if dot 39 gobbled already
    myGLCD.fillCircle(275, 60, 2); // Dot 39
   }
   if (dot[24] == 1) {  // Check if dot 24 gobbled already
  	myGLCD.fillCircle(298, 40, 2);  // dot 24
   }     
   if (dot[41] == 1) {  // Check if dot 41 gobbled already
  	myGLCD.fillCircle(321, 60, 2); // Dot 41
   } // ***************** Add Decisions Larry ***************
  }else
  if (xP== 310) { // dot 41
     if (dot[40] == 1) {  // Check if dot 40 gobbled already
   myGLCD.fillCircle(298, 60, 2); // Dot 40 
     }    
      if (dot[42] == 1) {  // Check if dot 42 gobbled already
    myGLCD.fillCircle(344, 60, 2); // Dot 42
     }        
  } else
  if (xP== 332) { // dot 42
      if (dot[41] == 1) {  // Check if dot 41 gobbled already
    myGLCD.fillCircle(321, 60, 2); // Dot 41
     }
      if (dot[25] == 1) {  // Check if dot 25 gobbled already
    myGLCD.fillCircle(160, 60, 2); // dot 25
     }    
      if (dot[43] == 1) {  // Check if dot 43 gobbled already
    myGLCD.fillCircle(367, 60, 2); // Dot 43
     }   
  } else
  if (xP== 356) { // dot 43
     if (dot[42] == 1) {  // Check if dot 42 gobbled already
    myGLCD.fillCircle(344, 60, 2); // Dot 42
     }    
      if (dot[44] == 1) {  // Check if dot 44 gobbled already
    myGLCD.fillCircle(390, 60, 2); // Dot 44
     }      
  } else
  if (xP== 380) { // dot 44
      if (dot[43] == 1) {  // Check if dot 43 gobbled already
    myGLCD.fillCircle(367, 60, 2); // Dot 43
     }
      if (dot[26] == 1) {  // Check if dot 26 gobbled already
    myGLCD.fillCircle(406, 40, 2);  // dot 26
     }     
      if (dot[45] == 1) {  // Check if dot 45 gobbled already
    myGLCD.fillCircle(413, 60, 2); // Dot 45
     }         
  } else
  if (xP== 402) { // dot 45
     if (dot[44] == 1) {  // Check if dot 44 gobbled already
    myGLCD.fillCircle(390, 60, 2); // Dot 44
     }
     if (dot[26] == 1) {  // Check if dot 26 gobbled already
    myGLCD.fillCircle(406, 40, 2);  // dot 26
     }          
      if (dot[46] == 1) {  // Check if dot 46 gobbled already
    myGLCD.fillCircle(436, 60, 2); // Dot 46
     }   
   
  } else
  if (xP== 424) { // dot 46
      if (dot[45] == 1) {  // Check if dot 45 gobbled already
    myGLCD.fillCircle(413, 60, 2); // Dot 45
     }    
      if (dot[49] == 1) {  // Check if dot 49 gobbled already
    myGLCD.fillCircle(435, 80, 2); // Dot 49
     }      
      if (dot[47] == 1) {  // Check if dot 47 gobbled already
    myGLCD.fillCircle(459, 60, 2); //Dot 47
     }  
  
  } else
  if (xP== 448) { // dot 47
   if (dot[27] == 1) {  // Check if dot 27 gobbled already
    myGLCD.fillCircle(465, 40, 7); // Big dot 27
   }     
   if (dot[46] == 1) {  // Check if dot 46 gobbled already
    myGLCD.fillCircle(436, 60, 2); // Dot 46
   } 
  }
} else
if (yP== 248) {  // if in Row 4  **********************************************************
  if (xP== 4) { // dot 66
     if (dot[86] == 1) {  // Check if dot 86 gobbled already
	  myGLCD.fillCircle(19, 281, 7); // Big dot 86
     }     
     if (dot[67] == 1) {  // Check if dot 67 gobbled already
	  myGLCD.fillCircle(42, 260, 2); // Dot 67
     }     
  } else
  if (xP== 28) { // dot 67
     if (dot[66] == 1) {  // Check if dot 66 gobbled already
	  myGLCD.fillCircle(19, 260, 2); // Dot 66
     }     
     if (dot[64] == 1) {  // Check if dot 64 gobbled already
	  myGLCD.fillCircle(42, 240, 2); // Dot 64
     }   
      if (dot[68] == 1) {  // Check if dot 68 gobbled already
  	myGLCD.fillCircle(65, 260, 2); // Dot 68
     }       
  } else
  if (xP== 52) { // dot 68
     if (dot[67] == 1) {  // Check if dot 67 gobbled already
	  myGLCD.fillCircle(42, 260, 2); // Dot 67
     }     
     if (dot[87] == 1) {  // Check if dot 87 gobbled already
	  myGLCD.fillCircle(77, 281, 2); // Dot 87
     } 
     if (dot[69] == 1) {  // Check if dot 69 gobbled already
	  myGLCD.fillCircle(88, 260, 2); // Dot 69
     }  
  } else
  if (xP== 74) { // dot 69
     if (dot[68] == 1) {  // Check if dot 68 gobbled already
	  myGLCD.fillCircle(65, 260, 2); // Dot 68
     }     
     if (dot[87] == 1) {  // Check if dot 87 gobbled already
	  myGLCD.fillCircle(77, 281, 2); // Dot 87
     } 
     if (dot[70] == 1) {  // Check if dot 70 gobbled already
	  myGLCD.fillCircle(112, 260, 2); // Dot 70 
     }    
     
  } else
  if (xP== 98) { // dot 70
     if (dot[69] == 1) {  // Check if dot 69 gobbled already
	  myGLCD.fillCircle(88, 260, 2); // Dot 69
     }     
     if (dot[71] == 1) {  // Check if dot 71 gobbled already
	  myGLCD.fillCircle(136, 260, 2); // Dot 71
     }   
  } else
  if (xP== 120) { // dot 71
     if (dot[70] == 1) {  // Check if dot 70 gobbled already
	  myGLCD.fillCircle(112, 260, 2); // Dot 70
     }     
     if (dot[88] == 1) {  // Check if dot 88 gobbled already
	  myGLCD.fillCircle(136, 281, 2); // Dot 88
     } 
     if (dot[72] == 1) {  // Check if dot 72 gobbled already
	  myGLCD.fillCircle(160, 260, 2); // Dot 72 
     }      
  } else
  if (xP== 146) { // dot 72
     if (dot[71] == 1) {  // Check if dot 71 gobbled already
	  myGLCD.fillCircle(136, 260, 2); // Dot 71
     }     
     if (dot[73] == 1) {  // Check if dot 73 gobbled already
	  myGLCD.fillCircle(183, 260, 2); // Dot 73
     }  
  } else

  if (xP== 168) { // dot 73
     if (dot[72] == 1) {  // Check if dot 72 gobbled already
	  myGLCD.fillCircle(160, 260, 2); // Dot 72
     }     
     if (dot[89] == 1) {  // Check if dot 89 gobbled already
	  myGLCD.fillCircle(183, 281, 2); // Dot 89
     } 
     if (dot[74] == 1) {  // Check if dot 74 gobbled already
	  myGLCD.fillCircle(206, 260, 2);  // Dot 74
     }        
  } else
  if (xP== 192) { // dot 74
     if (dot[73] == 1) {  // Check if dot 73 gobbled already
	  myGLCD.fillCircle(183, 260, 2); // Dot 73
     }     
     if (dot[75] == 1) {  // Check if dot 75 gobbled already
	  myGLCD.fillCircle(229, 260, 2); // Dot 75
     }      
  } else
  if (xP== 216) { // dot 75
     if (dot[74] == 1) {  // Check if dot 74 gobbled already
	  myGLCD.fillCircle(206, 260, 2);  // Dot 74
     }    
     if (dot[90] == 1) {  // Check if dot 90 gobbled already
	  myGLCD.fillCircle(241, 281, 2); // Dot 90
     } 
     if (dot[76] == 1) {  // Check if dot 76 gobbled already
	  myGLCD.fillCircle(252, 260, 2); // Dot 76
     }     
  } else
  if (xP== 238) { // dot 76
     if (dot[75] == 1) {  // Check if dot 75 gobbled already
	  myGLCD.fillCircle(229, 260, 2); // Dot 75
     }    
     if (dot[90] == 1) {  // Check if dot 90 gobbled already
	  myGLCD.fillCircle(241, 281, 2); // Dot 90
     }  
     if (dot[77] == 1) {  // Check if dot 77 gobbled already
	  myGLCD.fillCircle(275, 260, 2); // Dot 77
     }     
  } else
 

 if (xP== 262) { // dot 77
     if (dot[76] == 1) {  // Check if dot 76 gobbled already
	  myGLCD.fillCircle(252, 260, 2); // Dot 76
     }    
     if (dot[78] == 1) {  // Check if dot 78 gobbled already
	  myGLCD.fillCircle(298, 260, 2); // Dot 78
     }     
  } else
  if (xP== 284) { // dot 78
     if (dot[77] == 1) {  // Check if dot 77 gobbled already
    myGLCD.fillCircle(275, 260, 2); // Dot 77
     }
     if (dot[91] == 1) {  // Check if dot 91 gobbled already
	  myGLCD.fillCircle(298, 281, 2);  // dot 91
     }    
     if (dot[79] == 1) {  // Check if dot 79 gobbled already
	  myGLCD.fillCircle(321, 260, 2); // Dot 79
     } // ************************** Add decisions Larry *******************     
  } else
  if (xP== 310) { // dot 79
     if (dot[78] == 1) {  // Check if dot 78 gobbled already
   myGLCD.fillCircle(298, 260, 2); // Dot 78
     }     
     if (dot[80] == 1) {  // Check if dot 80 gobbled already
    myGLCD.fillCircle(344, 260, 2); // Dot 80
     }  
  } else

  if (xP== 332) { // dot 80
     if (dot[79] == 1) {  // Check if dot 79 gobbled already
    myGLCD.fillCircle(321, 260, 2); // Dot 79
     }     
     if (dot[92] == 1) {  // Check if dot 92 gobbled already
    myGLCD.fillCircle(347, 281, 2); // Dot 92
     } 
     if (dot[81] == 1) {  // Check if dot 81 gobbled already
   myGLCD.fillCircle(367, 260, 2); // Dot 81
     }         
  } else
  if (xP== 356) { // dot 81
     if (dot[80] == 1) {  // Check if dot 80 gobbled already
    myGLCD.fillCircle(344, 260, 2); // Dot 80
     }     
     if (dot[82] == 1) {  // Check if dot 82 gobbled already
    myGLCD.fillCircle(390, 260, 2); // Dot 82
     }      
  } else
  if (xP== 380) { // dot 82
     if (dot[81] == 1) {  // Check if dot 81 gobbled already
    myGLCD.fillCircle(367, 260, 2); // Dot 81
     }    
     if (dot[93] == 1) {  // Check if dot 93 gobbled already
    myGLCD.fillCircle(406, 281, 2); // Dot 93
     } 
     if (dot[83] == 1) {  // Check if dot 83 gobbled already
    myGLCD.fillCircle(413, 260, 2); // Dot 83
     }    
  } else
  if (xP== 402) { // dot 83
     if (dot[82] == 1) {  // Check if dot 82 gobbled already
    myGLCD.fillCircle(390, 260, 2); // Dot 82
     }    
     if (dot[93] == 1) {  // Check if dot 93 gobbled already
    myGLCD.fillCircle(406, 281, 2); // Dot 93
     }  
     if (dot[84] == 1) {  // Check if dot 84 gobbled already
    myGLCD.fillCircle(436, 260, 2); // Dot 84
     }     
  } else
 

 if (xP== 424) { // dot 84
     if (dot[83] == 1) {  // Check if dot 83 gobbled already
    myGLCD.fillCircle(413, 260, 2); // Dot 83
     }    
     if (dot[65] == 1) {  // Check if dot 65 gobbled already
    myGLCD.fillCircle(435, 240, 2); // Dot 65
     } 
     if (dot[85] == 1) {  // Check if dot 85 gobbled already
    myGLCD.fillCircle(459, 260, 2); // Dot 85
     }     
  } else
  if (xP== 448) { // dot 85
     if (dot[84] == 1) {  // Check if dot 84 gobbled already
    myGLCD.fillCircle(436, 260, 2); // Dot 84
     }    
     if (dot[94] == 1) {  // Check if dot 94 gobbled already
    myGLCD.fillCircle(465, 281, 7); // Big dot 94
     }       
  } 

} else
if (yP== 268) {  // if in Row 5  **********************************************************
  if (xP== 4) { // dot 86
     if (dot[66] == 1) {  // Check if dot 66 gobbled already
	  myGLCD.fillCircle(19, 260, 2); // Dot 66
     } 
     if (dot[95] == 1) {  // Check if dot 95 gobbled already
	  myGLCD.fillCircle(19, 301, 2); // Dot 95
     }    
  } else
   if (xP== 62) { // dot 87
     if (dot[68] == 1) {  // Check if dot 68 gobbled already
	  myGLCD.fillCircle(65, 260, 2); // Dot 68
     } 
     if (dot[69] == 1) {  // Check if dot 69 gobbled already
	  myGLCD.fillCircle(88, 260, 2); // Dot 69
     } 
     if (dot[97] == 1) {  // Check if dot 97 gobbled already
	  myGLCD.fillCircle(65, 301, 2); // Dot 97
     }
     if (dot[98] == 1) {  // Check if dot 98 gobbled already
	  myGLCD.fillCircle(88, 301, 2); // Dot 98
     }      
     
  } else
  
  if (xP== 120) { // dot 88
     if (dot[71] == 1) {  // Check if dot 71 gobbled already
	  myGLCD.fillCircle(136, 260, 2); // Dot 71
     }     
     if (dot[100] == 1) {  // Check if dot 100 gobbled already
	  myGLCD.fillCircle(136, 301, 2); // Dot 100
     }    
  } else
  if (xP== 168) { // dot 89
     if (dot[73] == 1) {  // Check if dot 73 gobbled already
	  myGLCD.fillCircle(183, 260, 2); // Dot 73
     }     
     if (dot[101] == 1) {  // Check if dot 101 gobbled already
	  myGLCD.fillCircle(183, 301, 2); // Dot 101
     }       
  } else
  
  if (xP== 228) { // dot 90
     if (dot[75] == 1) {  // Check if dot 75 gobbled already
	  myGLCD.fillCircle(229, 260, 2); // Dot 75
     }
     if (dot[76] == 1) {  // Check if dot 76 gobbled already
	  myGLCD.fillCircle(252, 260, 2); // Dot 76
     } 
     if (dot[104] == 1) {  // Check if dot 104 gobbled already
	  myGLCD.fillCircle(252, 301, 2); // Dot 104
     } 
     if (dot[103] == 1) {  // Check if dot 103 gobbled already
	  myGLCD.fillCircle(229, 301, 2); // Dot 103
     }      
     
  } else
  
  if (xP== 284) { // dot 91
     if (dot[78] == 1) {  // Check if dot 78 gobbled already
	  myGLCD.fillCircle(298, 260, 2); // Dot 78
     } 
     if (dot[106] == 1) {  // Check if dot 106 gobbled already
	  myGLCD.fillCircle(298, 301, 2); // Dot 106
     } // ********************* Added more decisions Larry ***************************   
  }else
  if (xP== 332) { // dot 92
     if (dot[80] == 1) {  // Check if dot 80 gobbled already
   myGLCD.fillCircle(344, 260, 2); // Dot 80
     }     
     if (dot[107] == 1) {  // Check if dot 107 gobbled already
    myGLCD.fillCircle(345, 301, 2);  // dot 107
     }       
  } else
  
  if (xP== 356) { // dot 93
     if (dot[82] == 1) {  // Check if dot 82 gobbled already
    myGLCD.fillCircle(390, 260, 2); // Dot 82
     }
     if (dot[83] == 1) {  // Check if dot 83 gobbled already
    myGLCD.fillCircle(413, 260, 2); // Dot 83
     } 
     if (dot[109] == 1) {  // Check if dot 109 gobbled already
    myGLCD.fillCircle(391, 301, 2);  // dot 109
     } 
     if (dot[110] == 1) {  // Check if dot 110 gobbled already
    myGLCD.fillCircle(414, 301, 2);  // dot 110
     }      
     
  } else
  
  if (xP== 448) { // dot 94
     if (dot[112] == 1) {  // Check if dot 112 gobbled already
    myGLCD.fillCircle(460, 301, 2);  // dot 112
     } 
     if (dot[85] == 1) {  // Check if dot 85 gobbled already
    myGLCD.fillCircle(459, 260, 2); // Dot 85
     }    
  }

} else


if (yP== 288) {  // if in Row 6  **********************************************************
  if (xP== 4) { // dot 95
     if (dot[86] == 1) {  // Check if dot 86 gobbled already
	  myGLCD.fillCircle(19, 281, 7); // Big dot 86
     } 
     if (dot[96] == 1) {  // Check if dot 96 gobbled already
	  myGLCD.fillCircle(42, 301, 2); // Dot 96
     }   
  } else
  if (xP== 28) { // dot 96
     if (dot[95] == 1) {  // Check if dot 95 gobbled already
	  myGLCD.fillCircle(19, 301, 2); // Dot 95
     }  
     if (dot[97] == 1) {  // Check if dot 97 gobbled already
	 myGLCD.fillCircle(65, 301, 2); // Dot 97
     }      
  } else
  if (xP== 52) { // dot 97
     if (dot[96] == 1) {  // Check if dot 96 gobbled already
	  myGLCD.fillCircle(42, 301, 2); // Dot 96
     } 
     if (dot[87] == 1) {  // Check if dot 87 gobbled already
	  myGLCD.fillCircle(77, 281, 2); // Dot 87
     }  
     if (dot[98] == 1) {  // Check if dot 98 gobbled already
	  myGLCD.fillCircle(88, 301, 2); // Dot 98 
     }      
  } else
  if (xP== 74) { // dot 98
     if (dot[97] == 1) {  // Check if dot 97 gobbled already
	  myGLCD.fillCircle(65, 301, 2); // Dot 97
     } 
     if (dot[87] == 1) {  // Check if dot 87 gobbled already
	  myGLCD.fillCircle(77, 281, 2); // Dot 87
     }  
     if (dot[99] == 1) {  // Check if dot 99 gobbled already
	  myGLCD.fillCircle(112, 301, 2); // Dot 99 
     }     
  } else
  if (xP== 98) { // dot 99
     if (dot[98] == 1) {  // Check if dot 98 gobbled already
	  myGLCD.fillCircle(88, 301, 2); // Dot 98
     } 
     if (dot[100] == 1) {  // Check if dot 100 gobbled already
	  myGLCD.fillCircle(136, 301, 2); // Dot 100
     }    
  } else
  if (xP== 120) { // dot 100
     if (dot[99] == 1) {  // Check if dot 99 gobbled already
	  myGLCD.fillCircle(112, 301, 2); // Dot 99
     } 
     if (dot[88] == 1) {  // Check if dot 88 gobbled already
	  myGLCD.fillCircle(136, 281, 2); // Dot 88
     }    
  } else
  if (xP== 168) { // dot 101
     if (dot[89] == 1) {  // Check if dot 89 gobbled already
	  myGLCD.fillCircle(183, 281, 2); // Dot 89
     } 
     if (dot[102] == 1) {  // Check if dot 102 gobbled already
	  myGLCD.fillCircle(206, 301, 2); // Dot 102
     }     
  } else
  if (xP== 192) { // dot 102
     if (dot[101] == 1) {  // Check if dot 101 gobbled already
	  myGLCD.fillCircle(183, 301, 2); // Dot 101
     } 
     if (dot[103] == 1) {  // Check if dot 103 gobbled already
	  myGLCD.fillCircle(229, 301, 2); // Dot 103
     }    
  } else
  if (xP== 216) { // dot 103
     if (dot[102] == 1) {  // Check if dot 102 gobbled already
	  myGLCD.fillCircle(206, 301, 2); // Dot 102
     } 
     if (dot[90] == 1) {  // Check if dot 90 gobbled already
	  myGLCD.fillCircle(241, 281, 2); // Dot 90
     }
     if (dot[104] == 1) {  // Check if dot 104 gobbled already
	  myGLCD.fillCircle(252, 301, 2); // Dot 104
     }    
  } else
  if (xP== 238) { // dot 104
     if (dot[103] == 1) {  // Check if dot 103 gobbled already
	  myGLCD.fillCircle(229, 301, 2); // Dot 103
     } 
     if (dot[90] == 1) {  // Check if dot 90 gobbled already
	  myGLCD.fillCircle(241, 281, 2); // Dot 90
     }
     if (dot[105] == 1) {  // Check if dot 105 gobbled already
	  myGLCD.fillCircle(275, 301, 2); // Dot 105
     }       
  } else
  if (xP== 262) { // dot 105
     if (dot[104] == 1) {  // Check if dot 104 gobbled already
	  myGLCD.fillCircle(252, 301, 2); // Dot 104
     }  
     if (dot[106] == 1) {  // Check if dot 106 gobbled already
	  myGLCD.fillCircle(298, 301, 2); // Dot 106
     }       
  } else
  if (xP== 310) { // dot 106
     if (dot[105] == 1) {  // Check if dot 105 gobbled already
	  myGLCD.fillCircle(275, 301, 2); // Dot 105
     } 
     if (dot[91] == 1) {  // Check if dot 91 gobbled already
	  myGLCD.fillCircle(298, 281, 2);  // dot 91
     }  // *************  Larry added ********************   
  }else
  if (xP== 332) { // dot 107
     if (dot[92] == 1) {  // Check if dot 92 gobbled already
   myGLCD.fillCircle(347, 281, 2); // Dot 92
     } 
     if (dot[108] == 1) {  // Check if dot 108 gobbled already
    myGLCD.fillCircle(368, 301, 2);  // dot 108
     }     
  } else
  if (xP== 356) { // dot 108
     if (dot[107] == 1) {  // Check if dot 107 gobbled already
    myGLCD.fillCircle(345, 301, 2);  // dot 107
     } 
     if (dot[109] == 1) {  // Check if dot 109 gobbled already
    myGLCD.fillCircle(391, 301, 2);  // dot 109
     }    
  } else
  if (xP== 380) { // dot 109
     if (dot[108] == 1) {  // Check if dot 108 gobbled already
    myGLCD.fillCircle(368, 301, 2);  // dot 108
     } 
     if (dot[93] == 1) {  // Check if dot 93 gobbled already
    myGLCD.fillCircle(406, 281, 2); // Dot 93
     }
     if (dot[110] == 1) {  // Check if dot 110 gobbled already
    myGLCD.fillCircle(414, 301, 2);  // dot 110
     }    
  } else
  if (xP== 402) { // dot 110
     if (dot[109] == 1) {  // Check if dot 109 gobbled already
    myGLCD.fillCircle(391, 301, 2);  // dot 109
     } 
     if (dot[93] == 1) {  // Check if dot 93 gobbled already
    myGLCD.fillCircle(406, 281, 2); // Dot 93
     }
     if (dot[111] == 1) {  // Check if dot 111 gobbled already
    myGLCD.fillCircle(437, 301, 2);  // dot 111
     }       
  } else
  if (xP== 424) { // dot 111
     if (dot[110] == 1) {  // Check if dot 110 gobbled already
    myGLCD.fillCircle(414, 301, 2);  // dot 110
     }  
     if (dot[112] == 1) {  // Check if dot 112 gobbled already
    myGLCD.fillCircle(460, 301, 2);  // dot 112
     }       
  } else
  if (xP== 448) { // dot 112
     if (dot[94] == 1) {  // Check if dot 94 gobbled already
    myGLCD.fillCircle(465, 281, 7); // Big dot 94
     } 
     if (dot[111] == 1) {  // Check if dot 111 gobbled already
    myGLCD.fillCircle(437, 301, 2);  // dot 111
     }     
  }
} else


// Check Columns


if (xP== 28) {  // if in Column 2
  if (yP== 66) { // dot 48
     if (dot[29] == 1) {  // Check if dot 29 gobbled already
	  myGLCD.fillCircle(42, 60, 2); // Dot 29
     }     
     if (dot[50] == 1) {  // Check if dot 50 gobbled already
	  myGLCD.fillCircle(42, 100, 2); // Dot 50
     }        
  } else
  if (yP== 86) { // dot 50
      if (dot[48] == 1) {  // Check if dot 48 gobbled already
  	myGLCD.fillCircle(42, 80, 2); // Dot 48
     }  
      if (dot[52] == 1) {  // Check if dot 52 gobbled already
  	myGLCD.fillCircle(42, 120, 2); // Dot 52
     }      
  } else
  if (yP== 106) { // dot 52
     if (dot[50] == 1) {  // Check if dot 50 gobbled already
	  myGLCD.fillCircle(42, 100, 2); // Dot 50
     }     
     if (dot[54] == 1) {  // Check if dot 54 gobbled already
	 myGLCD.fillCircle(42, 140, 2); // Dot 54
     }      
  } else
  if (yP== 126) { // dot 54
      if (dot[52] == 1) {  // Check if dot 52 gobbled already
  	myGLCD.fillCircle(42, 120, 2); // Dot 52
     } 
      if (dot[56] == 1) {  // Check if dot 56 gobbled already
  	myGLCD.fillCircle(42, 160, 2); // Dot 56
     }       
  } else
  if (yP== 146) { // dot 56
     if (dot[54] == 1) {  // Check if dot 54 gobbled already
	  myGLCD.fillCircle(42, 140, 2); // Dot 54
     }     
     if (dot[58] == 1) {  // Check if dot 58 gobbled already
	  myGLCD.fillCircle(42, 180, 2); // Dot 58
     }      
  }else
  if (yP== 166) { // dot 58
      if (dot[56] == 1) {  // Check if dot 56 gobbled already
   myGLCD.fillCircle(42, 160, 2); // Dot 56
     }  
      if (dot[60] == 1) {  // Check if dot 60 gobbled already
    myGLCD.fillCircle(42, 200, 2); // Dot 60
     }      
  } else
  if (yP== 186) { // dot 60
     if (dot[58] == 1) {  // Check if dot 58 gobbled already
    myGLCD.fillCircle(42, 180, 2); // Dot 58
     }     
     if (dot[62] == 1) {  // Check if dot 62 gobbled already
    myGLCD.fillCircle(42, 220, 2); // Dot 62
     }      
  } else
  if (yP== 206) { // dot 62
      if (dot[60] == 1) {  // Check if dot 60 gobbled already
    myGLCD.fillCircle(42, 200, 2); // Dot 60
     } 
      if (dot[64] == 1) {  // Check if dot 64 gobbled already
    myGLCD.fillCircle(42, 240, 2); // Dot 64
     }       
  } else
  if (yP== 226) { // dot 64
     if (dot[62] == 1) {  // Check if dot 62 gobbled already
    myGLCD.fillCircle(42, 220, 2); // Dot 62
     }     
     if (dot[67] == 1) {  // Check if dot 67 gobbled already
    myGLCD.fillCircle(42, 260, 2); // Dot 67
     }      
  }

} else
if (xP== 424) {  // if in Column 7

  if (yP== 66) { // dot 49
      if (dot[46] == 1) {  // Check if dot 46 gobbled already
  	myGLCD.fillCircle(436, 60, 2); // Dot 46
     }   
      if (dot[51] == 1) {  // Check if dot 51 gobbled already
  	myGLCD.fillCircle(435, 100, 2); // Dot 51
     }   
  } else
  if (yP== 86) { // dot 51
      if (dot[49] == 1) {  // Check if dot 49 gobbled already
  	myGLCD.fillCircle(435, 80, 2); // Dot 49
     }  
      if (dot[53] == 1) {  // Check if dot 53 gobbled already
  	myGLCD.fillCircle(435, 120, 2); // Dot 53
     }     
  } else
  if (yP== 106) { // dot 53
      if (dot[51] == 1) {  // Check if dot 51 gobbled already
  	myGLCD.fillCircle(435, 100, 2); // Dot 51
     }  
      if (dot[55] == 1) {  // Check if dot 55 gobbled already
  	myGLCD.fillCircle(435, 140, 2); // Dot 55
     }      
  } else
  if (yP== 126) { // dot 55
      if (dot[53] == 1) {  // Check if dot 53 gobbled already
  	myGLCD.fillCircle(435, 120, 2); // Dot 53
     }
     if (dot[57] == 1) {  // Check if dot 57 gobbled already
	  myGLCD.fillCircle(435, 160, 2); // Dot 57
     }       
  } else
  if (yP== 146) { // dot 57
      if (dot[55] == 1) {  // Check if dot 55 gobbled already
  	myGLCD.fillCircle(435, 140, 2); // Dot 55
     } 
     if (dot[59] == 1) {  // Check if dot 59 gobbled already
	  myGLCD.fillCircle(435, 180, 2); // Dot 59
     }     
  } else
  if (yP== 166) { // dot 59
      if (dot[57] == 1) {  // Check if dot 57 gobbled already
    myGLCD.fillCircle(435, 160, 2); // Dot 57
     }  
      if (dot[61] == 1) {  // Check if dot 61 gobbled already
    myGLCD.fillCircle(435, 200, 2); // Dot 61
     }     
  } else
  if (yP== 186) { // dot 61
      if (dot[59] == 1) {  // Check if dot 59 gobbled already
    myGLCD.fillCircle(435, 180, 2); // Dot 59
     }  
      if (dot[63] == 1) {  // Check if dot 63 gobbled already
    myGLCD.fillCircle(435, 220, 2); // Dot 63
     }      
  } else
  if (yP== 206) { // dot 63
      if (dot[61] == 1) {  // Check if dot 61 gobbled already
    myGLCD.fillCircle(435, 200, 2); // Dot 61
     }
     if (dot[65] == 1) {  // Check if dot 65 gobbled already
    myGLCD.fillCircle(435, 240, 2); // Dot 65
     }       
  } else
  if (yP== 226) { // dot 65
      if (dot[63] == 1) {  // Check if dot 63 gobbled already
    myGLCD.fillCircle(435, 220, 2); // Dot 63
     } 
     if (dot[84] == 1) {  // Check if dot 84 gobbled already
    myGLCD.fillCircle(436, 260, 2); // Dot 84
     }     
  }
}// *************************** Done To Here ******************************** 
 
// increment Pacman Graphic Flag 0 = Closed, 1 = Medium Open, 2 = Wide Open
P=P+1; 
if(P==4){
  P=0; // Reset counter to closed
}

      
       
// Capture legacy direction to enable adequate blanking of trail
prevD = D;

/* Temp print variables for testing
  myGLCD.setFont(SmallFont);
  myGLCD.setColor(0, 0, 0);
  myGLCD.setBackColor(114, 198, 206);

  myGLCD.printNumI(xT,100,140); // Print xP
  myGLCD.printNumI(yT,155,140); // Print yP 
*/


// Check if Dot has been eaten before and incrementing score

// Check Rows

if (yP == 4) {  // if in Row 1 **********************************************************
  if (xP == 4) { // dot 1
     if (dot[1] == 1) {  // Check if dot gobbled already
        dot[1] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 28) { // dot 2
     if (dot[2] == 1) {  // Check if dot gobbled already
        dot[2] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 52) { // dot 3
     if (dot[3] == 1) {  // Check if dot gobbled already
        dot[3] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 74) { // dot 4
     if (dot[4] == 1) {  // Check if dot gobbled already
        dot[4] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 98) { // dot 5
     if (dot[5] == 1) {  // Check if dot gobbled already
        dot[5] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 120) { // dot 6
     if (dot[6] == 1) {  // Check if dot gobbled already
        dot[6] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 168) { // dot 7
     if (dot[7] == 1) {  // Check if dot gobbled already
        dot[7] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 192) { // dot 8
     if (dot[8] == 1) {  // Check if dot gobbled already
        dot[8] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 216) { // dot 9
     if (dot[9] == 1) {  // Check if dot gobbled already
        dot[9] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 238) { // dot 10
     if (dot[10] == 1) {  // Check if dot gobbled already
        dot[10] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 262) { // dot 11
     if (dot[11] == 1) {  // Check if dot gobbled already
        dot[11] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 284) { // dot 12
     if (dot[12] == 1) {  // Check if dot gobbled already
        dot[12] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     } // ***************** Add check Larry ****************************    
  }  else
  if (xP == 332) { // dot 13
     if (dot[13] == 1) {  // Check if dot gobbled already
        dot[13] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 356) { // dot 14
     if (dot[14] == 1) {  // Check if dot gobbled already
        dot[14] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 380) { // dot 15
     if (dot[15] == 1) {  // Check if dot gobbled already
        dot[15] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 402) { // dot 16
     if (dot[16] == 1) {  // Check if dot gobbled already
        dot[16] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 424) { // dot 17
     if (dot[17] == 1) {  // Check if dot gobbled already
        dot[17] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 448) { // dot 18
     if (dot[18] == 1) {  // Check if dot gobbled already
        dot[18] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } 

} else 
if (yP == 26) {  // if in Row 2  **********************************************************
  if (xP == 4) { // dot 19
     if (dot[19] == 1) {  // Check if dot gobbled already
        dot[19] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score 
        // Turn Ghost Blue if Pacman eats Big Dots
        fruiteatenpacman = true; // Turn Ghost blue      
     }     
  } else
  if (xP == 62) { // dot 20
     if (dot[20] == 1) {  // Check if dot gobbled already
        dot[20] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 120) { // dot 21
     if (dot[21] == 1) {  // Check if dot gobbled already
        dot[21] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 168) { // dot 22
     if (dot[22] == 1) {  // Check if dot gobbled already
        dot[22] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 228) { // dot 23
     if (dot[23] == 1) {  // Check if dot gobbled already
        dot[23] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 284) { // dot 24
     if (dot[24] == 1) {  // Check if dot gobbled already
        dot[24] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 332) { // dot 25
     if (dot[25] == 1) {  // Check if dot gobbled already
        dot[25] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 394) { // dot 26
     if (dot[26] == 1) {  // Check if dot gobbled already
        dot[26] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 448) { // dot 27
     if (dot[27] == 1) {  // Check if dot gobbled already
        dot[27] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score
        // Turn Ghost Blue if Pacman eats Big Dots
        fruiteatenpacman = true; // Turn Ghost Blue       
     }     
  } 

} else
if (yP == 46) {  // if in Row 3  **********************************************************
  if (xP == 4) { // dot 28
     if (dot[28] == 1) {  // Check if dot gobbled already
        dot[28] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 28) { // dot 29
     if (dot[29] == 1) {  // Check if dot gobbled already
        dot[29] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 52) { // dot 30
     if (dot[30] == 1) {  // Check if dot gobbled already
        dot[30] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 74) { // dot 31
     if (dot[31] == 1) {  // Check if dot gobbled already
        dot[31] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 98) { // dot 32
     if (dot[32] == 1) {  // Check if dot gobbled already
        dot[32] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 120) { // dot 33
     if (dot[33] == 1) {  // Check if dot gobbled already
        dot[33] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 146) { // dot 34
     if (dot[34] == 1) {  // Check if dot gobbled already
        dot[34] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else

  if (xP == 168) { // dot 35
     if (dot[35] == 1) {  // Check if dot gobbled already
        dot[35] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 192) { // dot 36
     if (dot[36] == 1) {  // Check if dot gobbled already
        dot[36] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 216) { // dot 37
     if (dot[37] == 1) {  // Check if dot gobbled already
        dot[37] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 238) { // dot 38
     if (dot[38] == 1) {  // Check if dot gobbled already
        dot[38] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 262) { // dot 39
     if (dot[39] == 1) {  // Check if dot gobbled already
        dot[39] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 284) { // dot 40
     if (dot[40] == 1) {  // Check if dot gobbled already
        dot[40] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }   // ****************** Add Larry *******************  
  }  else
  if (xP == 310) { // dot 41
     if (dot[41] == 1) {  // Check if dot gobbled already
        dot[41] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else

  if (xP == 332) { // dot 42
     if (dot[42] == 1) {  // Check if dot gobbled already
        dot[42] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 356) { // dot 43
     if (dot[43] == 1) {  // Check if dot gobbled already
        dot[43] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 380) { // dot 44
     if (dot[44] == 1) {  // Check if dot gobbled already
        dot[44] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 402) { // dot 45
     if (dot[45] == 1) {  // Check if dot gobbled already
        dot[45] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 424) { // dot 46
     if (dot[46] == 1) {  // Check if dot gobbled already
        dot[46] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 448) { // dot 47
     if (dot[47] == 1) {  // Check if dot gobbled already
        dot[47] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  }

} else
if (yP == 248) {  // if in Row 4  **********************************************************
  if (xP == 4) { // dot 66
     if (dot[66] == 1) {  // Check if dot gobbled already
        dot[66] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 28) { // dot 67
     if (dot[67] == 1) {  // Check if dot gobbled already
        dot[67] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 52) { // dot 68
     if (dot[68] == 1) {  // Check if dot gobbled already
        dot[68] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 74) { // dot 69
     if (dot[69] == 1) {  // Check if dot gobbled already
        dot[69] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 98) { // dot 70
     if (dot[70] == 1) {  // Check if dot gobbled already
        dot[70] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 120) { // dot 71
     if (dot[71] == 1) {  // Check if dot gobbled already
        dot[71] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 146) { // dot 72
     if (dot[72] == 1) {  // Check if dot gobbled already
        dot[72] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else

  if (xP == 168) { // dot 73
     if (dot[73] == 1) {  // Check if dot gobbled already
        dot[73] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 192) { // dot 74
     if (dot[74] == 1) {  // Check if dot gobbled already
        dot[74] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 216) { // dot 75
     if (dot[75] == 1) {  // Check if dot gobbled already
        dot[75] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 238) { // dot 76
     if (dot[76] == 1) {  // Check if dot gobbled already
        dot[76] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 262) { // dot 77
     if (dot[77] == 1) {  // Check if dot gobbled already
        dot[77] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 284) { // dot 78
     if (dot[78] == 1) {  // Check if dot gobbled already
        dot[78] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     } // *************** Add Larry ******************   
  } else
  if (xP == 310) { // dot 79
     if (dot[79] == 1) {  // Check if dot gobbled already
        dot[79] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else

  if (xP == 332) { // dot 80
     if (dot[80] == 1) {  // Check if dot gobbled already
        dot[80] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 356) { // dot 81
     if (dot[81] == 1) {  // Check if dot gobbled already
        dot[81] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 380) { // dot 82
     if (dot[82] == 1) {  // Check if dot gobbled already
        dot[82] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 402) { // dot 83
     if (dot[83] == 1) {  // Check if dot gobbled already
        dot[83] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 424) { // dot 84
     if (dot[84] == 1) {  // Check if dot gobbled already
        dot[84] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 448) { // dot 85
     if (dot[85] == 1) {  // Check if dot gobbled already
        dot[85] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }   
  }

} else
if (yP == 268) {  // if in Row 5  **********************************************************
  if (xP == 4) { // dot 86
     if (dot[86] == 1) {  // Check if dot gobbled already
        dot[86] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score
         // Turn Ghost Blue if Pacman eats Big Dots
        fruiteatenpacman = true; // Turn Ghost blue         
     }     
  } else
  if (xP == 62) { // dot 87
     if (dot[87] == 1) {  // Check if dot gobbled already
        dot[87] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 120) { // dot 88
     if (dot[88] == 1) {  // Check if dot gobbled already
        dot[88] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 168) { // dot 89
     if (dot[89] == 1) {  // Check if dot gobbled already
        dot[89] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 228) { // dot 90
     if (dot[90] == 1) {  // Check if dot gobbled already
        dot[90] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  }else
  if (xP == 284) { // dot 91
     if (dot[91] == 1) {  // Check if dot gobbled already
        dot[91] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 332) { // dot 92
     if (dot[92] == 1) {  // Check if dot gobbled already
        dot[92] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 394) { // dot 93
     if (dot[93] == 1) {  // Check if dot gobbled already
        dot[93] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 448) { // dot 94
     if (dot[94] == 1) {  // Check if dot gobbled already
        dot[94] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score 
          // Turn Ghost Blue if Pacman eats Big Dots
        fruiteatenpacman = true; // Turn Ghost blue        
     }     
  } 

} else
if (yP == 288) {  // if in Row 6  **********************************************************
  if (xP == 4) { // dot 95
     if (dot[95] == 1) {  // Check if dot gobbled already
        dot[95] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 28) { // dot 96
     if (dot[96] == 1) {  // Check if dot gobbled already
        dot[96] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 52) { // dot 97
     if (dot[97] == 1) {  // Check if dot gobbled already
        dot[97] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 74) { // dot 98
     if (dot[98] == 1) {  // Check if dot gobbled already
        dot[98] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 98) { // dot 99
     if (dot[99] == 1) {  // Check if dot gobbled already
        dot[99] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 120) { // dot 100
     if (dot[100] == 1) {  // Check if dot gobbled already
        dot[100] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 168) { // dot 101
     if (dot[101] == 1) {  // Check if dot gobbled already
        dot[101] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 192) { // dot 102
     if (dot[102] == 1) {  // Check if dot gobbled already
        dot[102] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 216) { // dot 103
     if (dot[103] == 1) {  // Check if dot gobbled already
        dot[103] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 238) { // dot 104
     if (dot[104] == 1) {  // Check if dot gobbled already
        dot[104] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 262) { // dot 105
     if (dot[105] == 1) {  // Check if dot gobbled already
        dot[105] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 284) { // dot 106
     if (dot[106] == 1) {  // Check if dot gobbled already
        dot[106] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  }  else
  if (xP == 332) { // dot 107
     if (dot[107] == 1) {  // Check if dot gobbled already
        dot[107] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 356) { // dot 108
     if (dot[108] == 1) {  // Check if dot gobbled already
        dot[108] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 380) { // dot 109
     if (dot[109] == 1) {  // Check if dot gobbled already
        dot[109] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 402) { // dot 110
     if (dot[110] == 1) {  // Check if dot gobbled already
        dot[110] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 424) { // dot 111
     if (dot[111] == 1) {  // Check if dot gobbled already
        dot[111] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (xP == 448) { // dot 112
     if (dot[112] == 1) {  // Check if dot gobbled already
        dot[112] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  }

}   

 

// Check Columns


if (xP == 28) {  // if in Column 2
  if (yP == 66) { // dot 48
     if (dot[48] == 1) {  // Check if dot gobbled already
        dot[48] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (yP == 86) { // dot 50
     if (dot[50] == 1) {  // Check if dot gobbled already
        dot[50] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (yP == 106) { // dot 52
     if (dot[52] == 1) {  // Check if dot gobbled already
        dot[52] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (yP == 126) { // dot 54
     if (dot[54] == 1) {  // Check if dot gobbled already
        dot[54] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (yP == 148) { // dot 56
     if (dot[56] == 1) {  // Check if dot gobbled already
        dot[56] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }   //************Add Larry **************   
  }  else
  if (yP == 166) { // dot 58
     if (dot[58] == 1) {  // Check if dot gobbled already
        dot[58] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (yP == 186) { // dot 60
     if (dot[60] == 1) {  // Check if dot gobbled already
        dot[60] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }    
  } else
  if (yP == 206) { // dot 62
     if (dot[62] == 1) {  // Check if dot gobbled already
        dot[62] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (yP == 226) { // dot 64
     if (dot[64] == 1) {  // Check if dot gobbled already
        dot[64] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }    
  }

} else
if (xP == 424) {  // if in Column 7
  if (yP == 66) { // dot 49
     if (dot[49] == 1) {  // Check if dot gobbled already
        dot[49] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (yP == 86) { // dot 51
     if (dot[51] == 1) {  // Check if dot gobbled already
        dot[51] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (yP == 106) { // dot 53
     if (dot[53] == 1) {  // Check if dot gobbled already
        dot[53] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (yP == 126) { // dot 55
     if (dot[55] == 1) {  // Check if dot gobbled already
        dot[55] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (yP == 148) { // dot 57
     if (dot[57] == 1) {  // Check if dot gobbled already
        dot[57] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (yP == 166) { // dot 59
     if (dot[59] == 1) {  // Check if dot gobbled already
        dot[59] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (yP == 186) { // dot 61
     if (dot[61] == 1) {  // Check if dot gobbled already
        dot[61] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (yP == 206) { // dot 63
     if (dot[63] == 1) {  // Check if dot gobbled already
        dot[63] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  } else
  if (yP == 226) { // dot 65
     if (dot[65] == 1) {  // Check if dot gobbled already
        dot[65] = 0; // Reset flag to Zero
        pacmanscore++; // Increment pacman score       
     }     
  }// *************************** Done To Here *********************************
} 

 

//Pacman wandering Algorithm 
// Note: Keep horizontal and vertical coordinates even numbers only to accomodate increment rate and starting point
// Pacman direction variable D where 0 = right, 1 = down, 2 = left, 3 = up

//****************************************************************************************************************************
//Right hand motion and ***************************************************************************************************
//****************************************************************************************************************************



if(D == 0){
// Increment xP and then test if any decisions required on turning up or down
  xP = xP+2; 

 // There are four horizontal rows that need rules

//********* Larry repair below
  // First Horizontal Row
  if (yP == 4) { 

    // Past first block decide to continue or go down
    if (xP == 62) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }
    // Past second block only option is down
    if (xP == 120) { 
         D = 1; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
    // Past third block decide to continue or go down
    if (xP == 228) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }
    // Past fourth block only option is down
    if (xP == 284) { // plus 58
         D = 1; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
    // ********************* added two more doorway decisions ********************
        // Past fifth block decide to continue or go down
    if (xP == 394) { // plus 108
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }
    // Past sixth block only option is down
    if (xP == 448) { 
         D = 1; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
  }

  // 2nd Horizontal Row
  if (yP == 46) { 

    // Past upper doorway on left decide to continue or go down
    if (xP == 28) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

    // Past first block decide to continue or go up
    if (xP == 62) { 
      direct = random(4); // generate random number between 0 and 3
      if (direct == 3){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }
     // Past Second block decide to continue or go up
    if (xP == 120) { // +58
      direct = random(4); // generate random number between 0 and 3
      if (direct == 3){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

     // Past First Wall decide to continue or go up
    if (xP == 168) { // +48
      direct = random(4); // generate random number between 0 and 3
      if (direct == 3){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

    // Past third block decide to continue or go up
    if (xP == 228) { // 
      direct = random(4); // generate random number between 0 and 3
      if (direct == 3){ // +60
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }

    }

//**************** Larry added more doorway options *******************

    // Past Fourth block decide to continue or go up
    if (xP == 284) { 
      direct = random(4); // generate random number between 0 and 3
      if (direct == 3){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }

    }
     // Past Second Wall decide to continue right or go up
    if (xP == 332) { // Plus 48
      direct = random(2); // generate random number between 0 and 3
      if (direct == 0){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } else {D = 3;}
    }
    // Past Fifth block decide to continue or go up
    if (xP == 394) { 
      direct = random(4); // generate random number between 0 and 3
      if (direct == 3){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }

    }
    // Past last clock digit decide to continue or go down
    if (xP == 424) { 
      direct = random(2); // generate random number between 0 and 2
      if (direct == 1){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }
//**************** Larry end added more doorway options *******************
    // Past sixth block only option is up
    if (xP == 448) { 
         D = 3; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
  }

  // LHS Door Horizontal Row
  if (yP == 148) { //was 108 

    // Past upper doorway on left decide to go up or go down
    if (xP == 28) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = 1; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { D = 3;}    
    }
  }

  // 3rd Horizontal Row
  if (yP == 248) { // Larry change from 168 to 248

    // Past lower doorway on left decide to continue or go up Larry Only up
    if (xP == 28) { 
      direct = random(4); // generate random number between 0 and 3
      if (direct == 3){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

    // Past first block decide to continue or go down
    if (xP == 62) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
  }
     // Past Second block decide to continue or go down
    if (xP == 120) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

     // Past Mid Wall decide to continue or go down
    if (xP == 168) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

    // Past third block decide to continue or go down
    if (xP == 228) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

    // Past last clock digit decide to continue or go up
    if (xP == 424) { 
      direct = random(4); // generate random number between 0 and 3
      if (direct == 3){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

    // Past fourth block only option is down
    if (xP == 448) { 
         D = 1; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
  }
 
  
  // 4th Horizontal Row
  if (yP == 288) { 

    // Past first block decide to continue or go up
    if (xP == 62) { 
      direct = random(4); // generate random number between 0 and 3
      if (direct == 3){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }
    // Past second block only option is up
    if (xP == 120) { 
         D = 3; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
    // Past third block decide to continue or go up
    if (xP == 228) { 
      direct = random(4); // generate random number between 0 and 3
      if (direct == 3){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }
 // * ************** Larry Mod *****************
    // Past fourth block only option is up
    if (xP == 284) { 
         D = 3; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
    // Past fifth block decide to continue or go up
    if (xP == 332) { 
      direct = random(4); // generate random number between 0 and 3
      if (direct == 3){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }
    
    // Past sixth block only option is up
    if (xP == 448) { 
         D = 3; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
   }
}

//****************************************************************************************************************************
//Left hand motion **********************************************************************************************************
//****************************************************************************************************************************

else if(D == 2){
// Increment xP and then test if any decisions required on turning up or down
  xP = xP-2; 

/* Temp print variables for testing
  myGLCD.setFont(SmallFont);
  myGLCD.setColor(0, 0, 0);
  myGLCD.setBackColor(114, 198, 206);
  myGLCD.printNumI(xP,80,165); // Print xP
  myGLCD.printNumI(yP,110,165); // Print yP
*/

 // There are four horizontal rows that need rules

  // First Horizontal Row  ******************************
  if (yP == 4) { 

     // Past first block only option is down
    if (xP == 4) { 
         D = 1; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
 
    // Past second block decide to continue or go down
    if (xP == 62) { // +58
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }
    // Past third block only option is down
    if (xP == 168) { //+106
         D = 1; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
    // Past fourth block decide to continue or go down
    if (xP == 228) { //+60 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }
// *********** Added more decisions Larry ***************
    // Past fifth block only option is down
    if (xP == 332) { //+106
         D = 1; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
    // Past sixth block decide to continue or go down
    if (xP == 394) { //+60 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }
  }

  // 2nd Horizontal Row ******************************
  if (yP == 46) { 

    // Meet LHS wall only option is up
    if (xP == 4) { 
         D = 3; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
    // Meet upper doorway on left decide to continue or go down
    if (xP == 28) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

    // Meet first block decide to continue or go up
    if (xP == 62) { 
      direct = random(4); // generate random number between 0 and 3
      if (direct == 3){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }
     // Meet Second block decide to continue or go up
    if (xP == 120) { 
      direct = random(4); // generate random number between 0 and 3
      if (direct == 3){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

     // Meet Mid Wall decide to continue or go up
    if (xP == 168) { 
      direct = random(4); // generate random number between 0 and 3
      if (direct == 3){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

    // Meet third block decide to continue or go up
    if (xP == 228) { 
      direct = random(4); // generate random number between 0 and 3
      if (direct == 3){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }
// ********** Add more decisions Larry ************
     // Meet Fourth block decide to continue or go up
    if (xP == 284) { 
      direct = random(4); // generate random number between 0 and 3
      if (direct == 3){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

     // Meet Mid Wall decide to continue or go up
    if (xP == 332) { 
      direct = random(4); // generate random number between 0 and 3
      if (direct == 3){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

    // Meet fifth block decide to continue or go up
    if (xP == 394) { 
      direct = random(4); // generate random number between 0 and 3
      if (direct == 3){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

// *********** end Larry **************************
    // Meet last clock digit decide to continue or go down
    if (xP == 424) { 
      direct = random(2); // generate random number between 0 and 3
      if (direct == 1){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

  }
  

  // 3rd Horizontal Row ******************************
  if (yP == 248) { 

    // Meet LHS lower wall only option is down
    if (xP == 4) { 
         D = 1; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }

    // Meet lower doorway on left decide to continue or go up
    if (xP == 28) { 
      direct = random(4); // generate random number between 0 and 3
      if (direct == 3){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

    // Meet first block decide to continue or go down
    if (xP == 62) { 
      direct = random(2); // generate random number between 0 and 3
      if (direct == 1){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }
     // Meet Second block decide to continue or go down
    if (xP == 120) { 
      direct = random(2); // generate random number between 0 and 3
      if (direct == 1){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

     // Meet Mid Wall decide to continue or go down
    if (xP == 168) { 
      direct = random(2); // generate random number between 0 and 3
      if (direct == 1){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

    // Meet third block decide to continue or go down
    if (xP == 228) { 
      direct = random(2); // generate random number between 0 and 3
      if (direct == 1){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }
// *************** Add decisions Larry***************
     // Meet Second block decide to continue or go down
    if (xP == 284) { 
      direct = random(2); // generate random number between 0 and 3
      if (direct == 1){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

     // Meet Second Wall decide to continue or go down
    if (xP == 332) { 
      direct = random(2); // generate random number between 0 and 3
      if (direct == 1){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

    // Meet third block decide to continue or go down
    if (xP == 394) { 
      direct = random(2); // generate random number between 0 and 3
      if (direct == 1){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

//***********************end ********************
    // Meet last clock digit above decide to continue or go up
    if (xP == 424) { 
      direct = random(4); // generate random number between 0 and 3
      if (direct == 3){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    
    }
    
  }
   // 4th Horizontal Row ******************************
  if (yP == 288) { 

    // Meet LHS wall only option is up
    if (xP == 4) { 
         D = 3; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }  
    // Meet first block decide to continue or go up
    if (xP == 62) { 
      direct = random(4); // generate random number between 0 and 3
      if (direct == 3){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }
    // Meet bottom divider wall only option is up
    if (xP == 168) { 
         D = 3; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
    // Meet 3rd block decide to continue or go up
    if (xP == 228) { 
      direct = random(4); // generate random number between 0 and 3
      if (direct == 3){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }
 //*******************  Add Larry ********************
     // Meet second bottom divider wall only option is up
    if (xP == 332) { 
         D = 3; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
    // Meet 3rd block decide to continue or go up
    if (xP == 394) { 
      direct = random(4); // generate random number between 0 and 3
      if (direct == 3){
         D = direct; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }  
  
  }
}  
  


//****************************************************************************************************************************
//Down motion **********************************************************************************************************
//****************************************************************************************************************************

else if(D == 1){
// Increment yP and then test if any decisions required on turning up or down
  yP = yP+2; 

/* Temp print variables for testing
  myGLCD.setFont(SmallFont);
  myGLCD.setColor(0, 0, 0);
  myGLCD.setBackColor(114, 198, 206);
  myGLCD.printNumI(xP,80,165); // Print xP
  myGLCD.printNumI(yP,110,165); // Print yP
*/

 // There are vertical rows that need rules

  // First Vertical Row  ******************************
  if (xP == 4) { 

     // Past first block only option is right
    if (yP == 46) { 
         D = 0; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
 
    // Towards bottom wall only option right
    if (yP == 288) { 
         D = 0; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    } 
  }

  // 2nd Vertical Row ******************************
  if (xP == 28) { 

    // Meet bottom doorway on left decide to go left or go right
    if (yP == 248) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { D = 0;}    
    }
  }

  // 3rd Vertical Row ******************************
  if (xP == 62) { 

    // Meet top lh digit decide to go left or go right
    if (yP == 46) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { D = 0;}    
    }

    // Meet bottom wall decide to go left or go right
    if (yP == 288) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { D = 0;}    
    }
  }

  // 5th Vertical Row ******************************
  if (xP == 120) { 

    // Meet top lh digit decide to go left or go right
    if (yP == 46) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { D = 0;}    
    }

    // Meet bottom wall only opgion to go left
    if (yP == 288) { 
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up   
    }
  }

  // 6th Vertical Row ******************************
  if (xP == 168) { 

    // Meet top lh digit decide to go left or go right
    if (yP == 46) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { D = 0;}    
    }

    // Meet bottom wall only opgion to go right
    if (yP == 288) { 
         D = 0; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up   
    }
  }

  // 8th Vertical Row ******************************
  if (xP == 228) { 

    // Meet top lh digit decide to go left or go right
    if (yP == 46) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { D = 0;}    
    }

    // Meet bottom wall
    if (yP == 288) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { D = 0;}    
    }
  }
//****************** Larry *****************************
  // 5ish Vertical Row ******************************
  if (xP == 284) { 

    // Meet top lh digit decide to go left or go right
    if (yP == 46) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { D = 0;}    
    }

    // Meet bottom wall only opgion to go left
    if (yP == 288) { 
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up   
    }
  }

  // 6ish Vertical Row ******************************
  if (xP == 332) { 

    // Meet top lh digit decide to go left or go right
    if (yP == 46) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { D = 0;}    
    }

    // Meet bottom wall only opgion to go right
    if (yP == 288) { 
         D = 0; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up   
    }
  }

  // 8ish Vertical Row ******************************
  if (xP == 394) { 

    // Meet top lh digit decide to go left or go right
    if (yP == 46) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { D = 0;}    
    }

    // Meet bottom wall
    if (yP == 288) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { D = 0;}    
    }
  }
  //****************** end Larry ********************************
  // 9th Vertical Row ******************************
  if (xP == 424) { 

    // Meet bottom right doorway  decide to go left or go right
    if (yP == 248) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { D = 0;}    
    }
  }

  // 10th Vertical Row  ******************************
  if (xP == 448) { 

     // Past first block only option is left
    if (yP == 46) { 
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
 
    // Towards bottom wall only option right
    if (yP == 288) { 
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    } 
  } 
}  

//****************************************************************************************************************************
//Up motion **********************************************************************************************************
//****************************************************************************************************************************

else if(D == 3){
// Decrement yP and then test if any decisions required on turning up or down
  yP = yP-2; 

/* Temp print variables for testing
  myGLCD.setFont(SmallFont);
  myGLCD.setColor(0, 0, 0);
  myGLCD.setBackColor(114, 198, 206);
  myGLCD.printNumI(xP,80,165); // Print xP
  myGLCD.printNumI(yP,110,165); // Print yP
*/


 // There are vertical rows that need rules

  // First Vertical Row  ******************************
  if (xP == 4) { 

     // Past first block only option is right
    if (yP == 4) { 
         D = 0; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
 
    // Towards bottom wall only option right
    if (yP == 248) { 
         D = 0; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    } 
  }

  // 2nd Vertical Row ******************************
  if (xP == 28) { 

    // Meet top doorway on left decide to go left or go right
    if (yP == 46) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { D = 0;}    
    }
  }

  // 3rd Vertical Row ******************************
  if (xP == 62) { 

    // Meet bottom lh digit decide to go left or go right
    if (yP == 248) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { D = 0;}    
    }

    // Meet top wall decide to go left or go right
    if (yP == 4) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { D = 0;}    
    }
  }

  // 5th Vertical Row ******************************
  if (xP == 120) { 

    // Meet bottom lh digit decide to go left or go right
    if (yP == 248) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { D = 0;}    
    }

    // Meet top wall only opgion to go left
    if (yP == 4) { 
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up   
    }
  }

  // 6th Vertical Row ******************************
  if (xP == 168) { 

    // Meet bottom lh digit decide to go left or go right
    if (yP == 248) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { D = 0;}    
    }

    // Meet top wall only opgion to go right
    if (yP == 4) { 
         D = 0; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up   
    }
  }

  // 8th Vertical Row ******************************
  if (xP == 228) { 

    // Meet bottom lh digit decide to go left or go right
    if (yP == 248) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { D = 0;}    
    }

    // Meet top wall go left or right
    if (yP == 4) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { D = 0;}    
    }
  }
//****************** Larry *****************************
  // 5ish Vertical Row ******************************
  if (xP == 284) { 

    // Meet bottom lh digit decide to go left or go right
    if (yP == 248) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { D = 0;}    
    }

    // Meet top wall only opgion to go left
    if (yP == 4) { 
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up   
    }
  }

  // 6ish Vertical Row ******************************
  if (xP == 332) { 

    // Meet bottom lh digit decide to go left or go right
    if (yP == 248) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { D = 0;}    
    }

    // Meet top wall only opgion to go right
    if (yP == 4) { 
         D = 0; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up   
    }
  }

  // 8ish Vertical Row ******************************
  if (xP == 394) { 

    // Meet bottom lh digit decide to go left or go right
    if (yP == 248) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { D = 0;}    
    }

    // Meet top wall
    if (yP == 4) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { D = 0;}    
    }
  }
  //****************** end Larry ********************************
  // 9th Vertical Row ******************************
  if (xP == 424) { 

    // Meet top right doorway  decide to go left or go right
    if (yP == 46) { 
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { D = 0;}    
    }
  }

  // 10th Vertical Row  ******************************
  if (xP == 448) { 

     // Past first block only option is left
    if (yP == 248) { 
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
 
    // Towards top wall only option right
    if (yP == 4) { 
         D = 2; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    } 
  } 
}  
}

  
//******************************************************************************************************************


//Ghost ; 
// Note: Keep horizontal and verticalcoordinates even numbers only to accomodateincrement rate and starting point
// Ghost direction variable  D where 0 = right, 1 = down, 2 = left, 3 = up

//****************************************************************************************************************************
//Right hand motion **********************************************************************************************************
//****************************************************************************************************************************


// If ghost captured then ghost dissapears until reset
if ((fruiteatenpacman == true)&&(abs(xG-xP)<=5)&&(abs(yG-yP)<=5)){ 
  
  if (ghostlost == false){
    pacmanscore++;
    pacmanscore++;  
  }

  ghostlost = true;

  dly = 28; // slowdown now only drawing one item
  }
  

if (ghostlost == false){ // only draw ghost if still alive

drawGhost(xG,yG,GD,prevGD); // Draws Ghost at these coordinates
/*Serial.println(xG);
Serial.println(yG);
Serial.println(" ");
*/
// If Ghost is on a dot then print the adjacent dots if they are valid

  myGLCD.setColor(200, 200, 200);
  
// Check Rows
//delay(dly); // Larry Delay
if (yG == 4) {  // if in Row 1 **********************************************************
  if (xG == 4) { // dot 1
     if (dot[2] == 1) {  // Check if dot 2 gobbled already
	  myGLCD.fillCircle(42, 19, 2); // dot 2
     }    
      if (dot[19] == 1) {  // Check if dot 19 gobbled already
  	myGLCD.fillCircle(19, 40, 7); // Big dot 19
     }    

  } else
  if (xG == 28) { // dot 2 +24
     if (dot[1] == 1) {  // Check if dot 1 gobbled already
	  myGLCD.fillCircle(19, 19, 2); // dot 1
     }    
      if (dot[3] == 1) {  // Check if dot 3 gobbled already
  	myGLCD.fillCircle(65, 19, 2); // dot 3
     }    

  } else
  if (xG == 52) { // dot 3 +24
     if (dot[2] == 1) {  // Check if dot 2 gobbled already
	  myGLCD.fillCircle(42, 19, 2); // dot 2
     }    
      if (dot[4] == 1) {  // Check if dot 4 gobbled already
  	myGLCD.fillCircle(88, 19, 2); // dot 4
     } 
      if (dot[20] == 1) {  // Check if dot 20 gobbled already
  	myGLCD.fillCircle(77, 40, 2);  // dot 20
     }   
  } else
  if (xG == 74) { // dot 4 +22
     if (dot[3] == 1) {  // Check if dot 3 gobbled already
  	myGLCD.fillCircle(65, 19, 2); // dot 3
     }    
      if (dot[5] == 1) {  // Check if dot 5 gobbled already
  	myGLCD.fillCircle(112, 19, 2); // dot 5
     }   
      if (dot[20] == 1) {  // Check if dot 20 gobbled already
  	myGLCD.fillCircle(77, 40, 2);  // dot 20
     }    
  } else
  if (xG == 98) { // dot 5 +24
     if (dot[4] == 1) {  // Check if dot 4 gobbled already
  	myGLCD.fillCircle(88, 19, 2); // dot 4
     }    
      if (dot[6] == 1) {  // Check if dot 6 gobbled already
  	myGLCD.fillCircle(136, 19, 2); // dot 6
     }     
  } else
  if (xG == 120) { // dot 6 +22
     if (dot[5] == 1) {  // Check if dot 5 gobbled already
  	myGLCD.fillCircle(112, 19, 2); // dot 5
     }    
      if (dot[21] == 1) {  // Check if dot 21 gobbled already
  	myGLCD.fillCircle(136, 40, 2);  // dot 21
     }     
  } else
 

 if (xG == 168) { // dot 7 +48
      if (dot[22] == 1) {  // Check if dot 22 gobbled already
  	myGLCD.fillCircle(183, 40, 2);  // dot 22
     }    
      if (dot[8] == 1) {  // Check if dot 8 gobbled already
  	myGLCD.fillCircle(206, 19, 2); // dot 8
     }     
  } else
  if (xG == 192) { // dot 8 +24
      if (dot[7] == 1) {  // Check if dot 7 gobbled already
  	myGLCD.fillCircle(183, 19, 2); // dot 7
     }    
      if (dot[9] == 1) {  // Check if dot 9 gobbled already
  	myGLCD.fillCircle(229, 19, 2); // dot 9
     }    
  } else
  if (xG == 216) { // dot 9 +24
      if (dot[10] == 1) {  // Check if dot 10 gobbled already
  	myGLCD.fillCircle(252, 19, 2); // dot 10
     }    
      if (dot[8] == 1) {  // Check if dot 8 gobbled already
  	myGLCD.fillCircle(206, 19, 2); // dot 8
     }      
      if (dot[23] == 1) {  // Check if dot 23 gobbled already
  	myGLCD.fillCircle(241, 40, 2); // dot 23
     }   
 } else
  if (xG == 238) { // dot 10 +22
      if (dot[11] == 1) {  // Check if dot 11 gobbled already
  	myGLCD.fillCircle(275, 19, 2); // dot 11
     }    
      if (dot[9] == 1) {  // Check if dot 9 gobbled already
  	myGLCD.fillCircle(229, 19, 2); // dot 9
     }      
      if (dot[23] == 1) {  // Check if dot 23 gobbled already
  	myGLCD.fillCircle(241, 40, 2); // dot 23
     }   
  } else
  if (xG == 262) { // dot 11 +24
      if (dot[10] == 1) {  // Check if dot 10 gobbled already
  	myGLCD.fillCircle(252, 19, 2); // dot 10
     }    
      if (dot[12] == 1) {  // Check if dot 12 gobbled already
  	myGLCD.fillCircle(298, 19, 2); // dot 12
     }    
  } else
  if (xG == 284) { // dot 12 +22
      if (dot[11] == 1) {  // Check if dot 11 gobbled already
  	myGLCD.fillCircle(275, 19, 2); // dot 11
     }    
      if (dot[24] == 1) {  // Check if dot 24 gobbled already
  	myGLCD.fillCircle(298, 40, 2);  // dot 24
     }  
  }else // ***************** continue row 1 Larry *******************
  if (xG == 332) { // dot 13 +48
      if (dot[14] == 1) {  // Check if dot 14 gobbled already
    myGLCD.fillCircle(368, 19, 2);
     }    
      if (dot[25] == 1) {  // Check if dot 25 gobbled already
    myGLCD.fillCircle(347, 40, 2);  // dot 25
     }  
  }else 
  if (xG == 356) { // dot 14 +24
      if (dot[13] == 1) {  // Check if dot 13 gobbled already
    myGLCD.fillCircle(345, 19, 2);  // dot 13 
     }    
      if (dot[15] == 1) {  // Check if dot 15 gobbled already
    myGLCD.fillCircle(391, 19, 2);  // dot 15
     }  
  }else 
  if (xG == 380) { // dot 15 +24
      if (dot[14] == 1) {  // Check if dot 14 gobbled already
    myGLCD.fillCircle(368, 19, 2);  // dot 14 
     }
      if (dot[16] == 1) {  // Check if dot 16 gobbled already
    myGLCD.fillCircle(414, 19, 2);  // dot 16
     }     
      if (dot[26] == 1) {  // Check if dot 26 gobbled already
    myGLCD.fillCircle(406, 40, 2);  // dot 26
     }  
  }else 
  if (xG == 402) { // dot 16 +22
      if (dot[15] == 1) {  // Check if dot 15 gobbled already
    myGLCD.fillCircle(391, 19, 2);  // dot 15 
     }
      if (dot[26] == 1) {  // Check if dot 26 gobbled already
    myGLCD.fillCircle(406, 40, 2);  // dot 26
     }     
      if (dot[17] == 1) {  // Check if dot 17 gobbled already
    myGLCD.fillCircle(437, 19, 2);  // dot 17
     }  
  }else 
  if (xG == 424) { // dot 17 +24
      if (dot[16] == 1) {  // Check if dot 16 gobbled already
    myGLCD.fillCircle(414, 19, 2);  // dot 16 
     }    
      if (dot[18] == 1) {  // Check if dot 18 gobbled already
    myGLCD.fillCircle(460, 19, 2);  // dot 18
     }  
  }else 
  if (xG == 448) { // dot 18 +22
      if (dot[17] == 1) {  // Check if dot 17 gobbled already
    myGLCD.fillCircle(437, 19, 2);  // dot 17
     }    
      if (dot[27] == 1) {  // Check if dot 27 gobbled already
    myGLCD.fillCircle(465, 40, 7); // Big dot 27
     }  
  }
} else 
if (yG == 26) {  // if in Row 2  **********************************************************
  if (xG == 4) { // dot 19
     if (dot[1] == 1) {  // Check if dot 1 gobbled already
	  myGLCD.fillCircle(19, 19, 2); // dot 1
     }    
      if (dot[28] == 1) {  // Check if dot 28 gobbled already
  	myGLCD.fillCircle(19, 60, 2); //  dot 28
     }   
  } else
  
    if (xG == 62) { // dot 20 +58
      if (dot[3] == 1) {  // Check if dot 3 gobbled already
  	myGLCD.fillCircle(65, 19, 2); // dot 3
     }   
         if (dot[4] == 1) {  // Check if dot 4 gobbled already
  	myGLCD.fillCircle(88, 19, 2); // dot 4
     } 
         if (dot[30] == 1) {  // Check if dot 30 gobbled already
  	myGLCD.fillCircle(65, 60, 2); // dot 30
     }   
      if (dot[31] == 1) {  // Check if dot 31 gobbled already
  	myGLCD.fillCircle(88, 60, 2); // dot 31
     }    
     
  } else
  
  if (xG == 120) { // dot 21 +58
     if (dot[33] == 1) {  // Check if dot 33 gobbled already
  	myGLCD.fillCircle(136, 60, 2); // dot 33
     }    
      if (dot[6] == 1) {  // Check if dot 6 gobbled already
  	myGLCD.fillCircle(136, 19, 2); // dot 6
     }      
  } else
  if (xG == 168) { // dot 22 +48
      if (dot[7] == 1) {  // Check if dot 7 gobbled already
  	myGLCD.fillCircle(183, 19, 2); // dot 7
     }    
      if (dot[35] == 1) {  // Check if dot 35 gobbled already
  	myGLCD.fillCircle(183, 60, 2); // dot 35
     }         
  } else
    if (xG == 228) { // dot 23 +60
      if (dot[9] == 1) {  // Check if dot 9 gobbled already
  	myGLCD.fillCircle(229, 19, 2); // dot 9
     }      
       if (dot[10] == 1) {  // Check if dot 10 gobbled already
  	myGLCD.fillCircle(252, 19, 2); // dot 10
     }  
      if (dot[37] == 1) {  // Check if dot 37 gobbled already
	  myGLCD.fillCircle(229, 60, 2); // dot 37
     }  
       if (dot[38] == 1) {  // Check if dot 38 gobbled already
  	myGLCD.fillCircle(252, 60, 2); // dot 38
     }         
  } else  // **************** Larry Add to row 2 *********************** 
  if (xG == 284) { // dot 24 +58
      if (dot[40] == 1) {  // Check if dot 40 gobbled already
  	myGLCD.fillCircle(298, 60, 2); // Dot 40
     }    
      if (dot[12] == 1) {  // Check if dot 12 gobbled already
  	myGLCD.fillCircle(298, 19, 2); // dot 12
     }  
  }else
  if (xG == 332) { // dot 25 +60
      if (dot[42] == 1) {  // Check if dot 42 gobbled already
   myGLCD.fillCircle(344, 60, 2); // Dot 42
     }    
      if (dot[13] == 1) {  // Check if dot 13 gobbled already
    myGLCD.fillCircle(345, 19, 2);  // dot 13
     }  
  }else
    if (xG == 394) { // dot 26 +58
      if (dot[15] == 1) {  // Check if dot 15 gobbled already
    myGLCD.fillCircle(391, 19, 2);  // dot 15
     }      
       if (dot[16] == 1) {  // Check if dot 16 gobbled already
    myGLCD.fillCircle(414, 19, 2);  // dot 16
     }  
      if (dot[44] == 1) {  // Check if dot 44 gobbled already
    myGLCD.fillCircle(390, 60, 2); // Dot 44
     }  
       if (dot[45] == 1) {  // Check if dot 45 gobbled already
    myGLCD.fillCircle(413, 60, 2); // Dot 45
     }         
  } else
  if (xG == 448) { // dot 27 +58
      if (dot[18] == 1) {  // Check if dot 18 gobbled already
   myGLCD.fillCircle(460, 19, 2);  // dot 18
     }    
      if (dot[47] == 1) {  // Check if dot 47 gobbled already
    myGLCD.fillCircle(459, 60, 2); //Dot 47
     }
  }          
} else
if (yG == 46) {  // if in Row 3  **********************************************************
//    Serial.println("Got there 1");
  if (xG == 4) { // dot 28
     if (dot[19] == 1) {  // Check if dot 19 gobbled already
	  myGLCD.fillCircle(19, 40, 7); // Big dot 19
     }    
      if (dot[29] == 1) {  // Check if dot 29 gobbled already
  	myGLCD.fillCircle(42, 60, 2); // Dot 29
     }    
  } else
  if (xG == 28) { // dot 29 +24
     if (dot[28] == 1) {  // Check if dot 28 gobbled already
  	myGLCD.fillCircle(19, 60, 2); // Dot 28
     }    
      if (dot[48] == 1) {  // Check if dot 48 gobbled already
  	myGLCD.fillCircle(42, 80, 2); // Dot 48
     }   
      if (dot[30] == 1) {  // Check if dot 30 gobbled already
  	myGLCD.fillCircle(65, 60, 2); //Dot 30
     }    
  } else
  if (xG == 52) { // dot 30 +24
     if (dot[29] == 1) {  // Check if dot 29 gobbled already
	  myGLCD.fillCircle(42, 60, 2); // Dot 29
     }    
      if (dot[20] == 1) {  // Check if dot 20 gobbled already
  	myGLCD.fillCircle(77, 40, 2);  // dot 20
     } 
      if (dot[31] == 1) {  // Check if dot 31 gobbled already
  	myGLCD.fillCircle(88, 60, 2); // Dot 31
     }       
  } else
  if (xG == 74) { // dot 31 +22
      if (dot[30] == 1) {  // Check if dot 30 gobbled already
  	myGLCD.fillCircle(65, 60, 2); //Dot 30
     }    
      if (dot[32] == 1) {  // Check if dot 32 gobbled already
  	myGLCD.fillCircle(112, 60, 2); // Dot 32
     } 
      if (dot[20] == 1) {  // Check if dot 20 gobbled already
  	myGLCD.fillCircle(77, 40, 2);  // dot 20
     }  
  } else
  if (xG == 98) { // dot 32 +24
     if (dot[31] == 1) {  // Check if dot 31 gobbled already
	 myGLCD.fillCircle(88, 60, 2); // Dot 31
     }    
      if (dot[33] == 1) {  // Check if dot 33 gobbled already
  	myGLCD.fillCircle(136, 60, 2); // Dot 33
     }  
       
  } else
  if (xG == 120) { // dot 33 +22
      if (dot[32] == 1) {  // Check if dot 32 gobbled already
  	myGLCD.fillCircle(112, 60, 2); // Dot 32
     }    
      if (dot[21] == 1) {  // Check if dot 21 gobbled already
  	myGLCD.fillCircle(136, 40, 2);  // dot 21
     }
      if (dot[34] == 1) {  // Check if dot 34 gobbled already
  	myGLCD.fillCircle(160, 60, 2); // Dot 34
     }         
  } else
  if (xG == 146) { // dot 34 +26
     if (dot[33] == 1) {  // Check if dot 33 gobbled already
	  myGLCD.fillCircle(136, 60, 2); // Dot 33
     }    
      if (dot[35] == 1) {  // Check if dot 35 gobbled already
  	myGLCD.fillCircle(183, 60, 2); // Dot 35
     }    
  } else
  if (xG == 168) { // dot 35 +22
      if (dot[34] == 1) {  // Check if dot 34 gobbled already
  	myGLCD.fillCircle(160, 60, 2); // Dot 34
     }    
      if (dot[22] == 1) {  // Check if dot 22 gobbled already
  	myGLCD.fillCircle(183, 40, 2);  // dot 22
     }
      if (dot[36] == 1) {  // Check if dot 36 gobbled already
  	myGLCD.fillCircle(206, 60, 2); // Dot 36
     }    
  } else
  if (xG == 192) { // dot 36 +24
     if (dot[35] == 1) {  // Check if dot 35 gobbled already
	  myGLCD.fillCircle(183, 60, 2); // Dot 35
     }    
      if (dot[37] == 1) {  // Check if dot 37 gobbled already
  	myGLCD.fillCircle(229, 60, 2); // Dot 37
     }     
  } else
  if (xG == 216) { // dot 37 +24
      if (dot[36] == 1) {  // Check if dot 36 gobbled already
  	myGLCD.fillCircle(206, 60, 2); // Dot 36
     }    
      if (dot[23] == 1) {  // Check if dot 23 gobbled already
  	myGLCD.fillCircle(241, 40, 2);  // dot 23
     }      
      if (dot[38] == 1) {  // Check if dot 38 gobbled already
  	myGLCD.fillCircle(252, 60, 2); // Dot 38
     }    
  } else
  if (xG == 238) { // dot 38 +22
     if (dot[37] == 1) {  // Check if dot 37 gobbled already
	  myGLCD.fillCircle(229, 60, 2); // Dot 37
     }    
      if (dot[23] == 1) {  // Check if dot 23 gobbled already
  	myGLCD.fillCircle(241, 40, 2);  // dot 23
     }      
      if (dot[39] == 1) {  // Check if dot 39 gobbled already
  	myGLCD.fillCircle(275, 60, 2); // Dot 39
     }   
  
  } else
  if (xG == 262) { // dot 39 +24
      if (dot[38] == 1) {  // Check if dot 38 gobbled already
  	myGLCD.fillCircle(252, 60, 2); // Dot 38
     }    
      if (dot[40] == 1) {  // Check if dot 40 gobbled already
  	myGLCD.fillCircle(298, 60, 2); // Dot 40 
     }       
// **************** Larry Add to row 3 ***********************   
  } else
  if (xG == 284) { // dot 40 +22
   if (dot[39] == 1) {  // Check if dot 39 gobbled already
  	myGLCD.fillCircle(275, 60, 2); // Dot 39
   }     
   if (dot[24] == 1) {  // Check if dot 24 gobbled already
  	myGLCD.fillCircle(298, 40, 2);  // dot 24
   }
   if (dot[41] == 1) {  // Check if dot 41 gobbled already
   myGLCD.fillCircle(321, 60, 2); // Dot 41
     } 
  } else
  if (xG == 310) { // dot 41 +26
//     Serial.println("Got there 2");
     if (dot[40] == 1) {  // Check if dot 40 gobbled already
    myGLCD.fillCircle(298, 60, 2); // Dot 40
     }    
      if (dot[42] == 1) {  // Check if dot 42 gobbled already
    myGLCD.fillCircle(344, 60, 2); // Dot 42
     }    
  } else
  if (xG == 332) { // dot 42 +22
      if (dot[41] == 1) {  // Check if dot 41 gobbled already
    myGLCD.fillCircle(321, 60, 2); // Dot 41
     }    
      if (dot[25] == 1) {  // Check if dot 25 gobbled already
    myGLCD.fillCircle(347, 40, 2);  // dot 25
     }
      if (dot[43] == 1) {  // Check if dot 43 gobbled already
    myGLCD.fillCircle(367, 60, 2); // Dot 43
     }    
  } else
  if (xG == 356) { // dot 43 +24
     if (dot[42] == 1) {  // Check if dot 42 gobbled already
   myGLCD.fillCircle(344, 60, 2); // Dot 42
     }    
      if (dot[44] == 1) {  // Check if dot 44 gobbled already
    myGLCD.fillCircle(390, 60, 2); // Dot 44
     }      
  } else
  if (xG == 380) { // dot 44 +24
      if (dot[43] == 1) {  // Check if dot 43 gobbled already
    myGLCD.fillCircle(367, 60, 2); // Dot 43
     }    
      if (dot[26] == 1) {  // Check if dot 26 gobbled already
    myGLCD.fillCircle(406, 40, 2);  // dot 26
     }      
      if (dot[45] == 1) {  // Check if dot 45 gobbled already
    myGLCD.fillCircle(413, 60, 2); // Dot 45
     }   
  } else
  if (xG == 402) { // dot 45 +22
     if (dot[44] == 1) {  // Check if dot 44 gobbled already
    myGLCD.fillCircle(390, 60, 2); // Dot 44
     }    
      if (dot[26] == 1) {  // Check if dot 26 gobbled already
    myGLCD.fillCircle(406, 40, 2);  // dot 26
     }      
      if (dot[46] == 1) {  // Check if dot 46 gobbled already
    myGLCD.fillCircle(436, 60, 2); // Dot 46
     }   
   
  } else
  if (xG == 424) { // dot 46 +24
      if (dot[45] == 1) {  // Check if dot 45 gobbled already
    myGLCD.fillCircle(413, 60, 2); // Dot 45
     }    
      if (dot[49] == 1) {  // Check if dot 49 gobbled already
    myGLCD.fillCircle(435, 80, 2); // Dot 49
     }      
      if (dot[47] == 1) {  // Check if dot 47 gobbled already
    myGLCD.fillCircle(459, 60, 2); //Dot 47
     } 
  
  } else
  if (xG == 448) { // dot 47 +22
   if (dot[46] == 1) {  // Check if dot 46 gobbled already
    myGLCD.fillCircle(436, 60, 2); // Dot 46
   }     
   if (dot[27] == 1) {  // Check if dot 27 gobbled already
    myGLCD.fillCircle(465, 40, 7); // Big dot 27
   } 
  }
} else
if (yG == 248) {  // if in Row 4  **********************************************************
  if (xG == 4) { // dot 66
     if (dot[86] == 1) {  // Check if dot 86 gobbled already
	  myGLCD.fillCircle(19, 281, 7); // Big dot 86
     }     
     if (dot[67] == 1) {  // Check if dot 67 gobbled already
	  myGLCD.fillCircle(42, 260, 2); // Dot 67
     }    
  } else
  if (xG == 28) { // dot 67
     if (dot[66] == 1) {  // Check if dot 66 gobbled already
	  myGLCD.fillCircle(19, 260, 2); // Dot 66
     }     
     if (dot[64] == 1) {  // Check if dot 64 gobbled already
	  myGLCD.fillCircle(42, 240, 2); // Dot 64
     }   
      if (dot[68] == 1) {  // Check if dot 68 gobbled already
  	myGLCD.fillCircle(65, 260, 2); // Dot 68
     }        
  } else
  if (xG == 52) { // dot 68
     if (dot[67] == 1) {  // Check if dot 67 gobbled already
	   myGLCD.fillCircle(42, 260, 2); // Dot 67
     }     
     if (dot[87] == 1) {  // Check if dot 87 gobbled already
	  myGLCD.fillCircle(77, 281, 2); // Dot 87
     } 
     if (dot[69] == 1) {  // Check if dot 69 gobbled already
	  myGLCD.fillCircle(88, 260, 2); // Dot 69
     }    
  } else
  if (xG == 74) { // dot 69
     if (dot[68] == 1) {  // Check if dot 68 gobbled already
	  myGLCD.fillCircle(65, 260, 2); // Dot 68
     }     
     if (dot[87] == 1) {  // Check if dot 87 gobbled already
	  myGLCD.fillCircle(77, 281, 2); // Dot 87
     } 
     if (dot[70] == 1) {  // Check if dot 70 gobbled already
	  myGLCD.fillCircle(112, 260, 2); // Dot 70 
     }    
     
  } else
  if (xG == 98) { // dot 70
     if (dot[69] == 1) {  // Check if dot 69 gobbled already
	  myGLCD.fillCircle(88, 260, 2); // Dot 69
     }     
     if (dot[71] == 1) {  // Check if dot 71 gobbled already
	  myGLCD.fillCircle(136, 260, 2); // Dot 71
     }  
  } else
  if (xG == 120) { // dot 71
     if (dot[70] == 1) {  // Check if dot 70 gobbled already
	  myGLCD.fillCircle(112, 260, 2); // Dot 70
     }     
     if (dot[88] == 1) {  // Check if dot 88 gobbled already
	  myGLCD.fillCircle(136, 281, 2); // Dot 88
     } 
     if (dot[72] == 1) {  // Check if dot 72 gobbled already
	  myGLCD.fillCircle(160, 260, 2); // Dot 72 
     }     
  } else
  if (xG == 146) { // dot 72
     if (dot[71] == 1) {  // Check if dot 71 gobbled already
	  myGLCD.fillCircle(136, 260, 2); // Dot 71
     }     
     if (dot[73] == 1) {  // Check if dot 73 gobbled already
	  myGLCD.fillCircle(183, 260, 2); // Dot 73
     } 
  } else

  if (xG == 168) { // dot 73
     if (dot[72] == 1) {  // Check if dot 72 gobbled already
	  myGLCD.fillCircle(160, 260, 2); // Dot 72
     }     
     if (dot[89] == 1) {  // Check if dot 89 gobbled already
	  myGLCD.fillCircle(183, 281, 2); // Dot 89
     } 
     if (dot[74] == 1) {  // Check if dot 74 gobbled already
	  myGLCD.fillCircle(206, 260, 2);  // Dot 74
     }          
  } else
  if (xG == 192) { // dot 74
     if (dot[73] == 1) {  // Check if dot 73 gobbled already
	  myGLCD.fillCircle(183, 260, 2); // Dot 73
     }     
     if (dot[75] == 1) {  // Check if dot 75 gobbled already
	  myGLCD.fillCircle(229, 260, 2); // Dot 75
     }      
  } else
  if (xG == 216) { // dot 75
     if (dot[74] == 1) {  // Check if dot 74 gobbled already
	  myGLCD.fillCircle(206, 260, 2);  // Dot 74
     }    
     if (dot[90] == 1) {  // Check if dot 90 gobbled already
	  myGLCD.fillCircle(241, 281, 2); // Dot 90
     } 
     if (dot[76] == 1) {  // Check if dot 76 gobbled already
	  myGLCD.fillCircle(252, 260, 2); // Dot 76
     }    
  } else
  if (xG == 238) { // dot 76
     if (dot[75] == 1) {  // Check if dot 75 gobbled already
	  myGLCD.fillCircle(229, 260, 2); // Dot 75
     }    
     if (dot[90] == 1) {  // Check if dot 90 gobbled already
	  myGLCD.fillCircle(241, 281, 2); // Dot 90
     }  
     if (dot[77] == 1) {  // Check if dot 77 gobbled already
	  myGLCD.fillCircle(275, 260, 2); // Dot 77
     }      
  } else
 if (xG == 262) { // dot 77
     if (dot[76] == 1) {  // Check if dot 76 gobbled already
	  myGLCD.fillCircle(252, 260, 2); // Dot 76
     }    
     if (dot[78] == 1) {  // Check if dot 78 gobbled already
	  myGLCD.fillCircle(298, 260, 2); // Dot 78
     } 
     if (dot[91] == 1) {  // Check if dot 91 gobbled already
	  myGLCD.fillCircle(298, 281, 2);  // dot 91
     }     
  } else
  if (xG == 284) { // dot 78
     if (dot[77] == 1) {  // Check if dot 77 gobbled already
	  myGLCD.fillCircle(275, 260, 2); // Dot 77
     }    
     if (dot[91] == 1) {  // Check if dot 91 gobbled already
	  myGLCD.fillCircle(298, 281, 2);  // dot 91
     }
    if (dot[79] == 1) {  // Check if dot 79 gobbled already
    myGLCD.fillCircle(321, 260, 2); // Dot 79
     }  // ************** Add Larry ******************  
       
  } else
  if (xG == 310) { // dot 79 +26
     if (dot[78] == 1) {  // Check if dot 78 gobbled already
    myGLCD.fillCircle(298, 260, 2); // Dot 78
     }    
      if (dot[80] == 1) {  // Check if dot 80 gobbled already
    myGLCD.fillCircle(344, 260, 2); // Dot 80
     }   
  } else
  if (xG == 332) { // dot 80 +22
      if (dot[79] == 1) {  // Check if dot 79 gobbled already
    myGLCD.fillCircle(321, 260, 2); // Dot 79
     }    
      if (dot[92] == 1) {  // Check if dot 92 gobbled already
    myGLCD.fillCircle(347, 281, 2); // Dot 92
     }
      if (dot[81] == 1) {  // Check if dot 81 gobbled already
    myGLCD.fillCircle(367, 260, 2); // Dot 81
     }    
  } else
  if (xG == 356) { // dot 81 +24
     if (dot[80] == 1) {  // Check if dot 80 gobbled already
   myGLCD.fillCircle(344, 260, 2); // Dot 80
     }    
      if (dot[82] == 1) {  // Check if dot 82 gobbled already
    myGLCD.fillCircle(390, 260, 2); // Dot 82
     }      
  } else
  if (xG == 380) { // dot 82 +24
      if (dot[81] == 1) {  // Check if dot 81 gobbled already
    myGLCD.fillCircle(367, 260, 2); // Dot 81
     }    
      if (dot[93] == 1) {  // Check if dot 93 gobbled already
    myGLCD.fillCircle(406, 281, 2); // Dot 93
     }      
      if (dot[83] == 1) {  // Check if dot 83 gobbled already
    myGLCD.fillCircle(413, 260, 2); // Dot 83
     }   
  } else
  if (xG == 402) { // dot 83 +22
     if (dot[82] == 1) {  // Check if dot 82 gobbled already
    myGLCD.fillCircle(390, 260, 2); // Dot 82
     }    
      if (dot[93] == 1) {  // Check if dot 93 gobbled already
    myGLCD.fillCircle(406, 281, 2); // Dot 93
     }      
      if (dot[84] == 1) {  // Check if dot 84 gobbled already
    myGLCD.fillCircle(436, 260, 2); // Dot 84
     }   
   
  } else
  if (xG == 424) { // dot 84 +24
      if (dot[83] == 1) {  // Check if dot 83 gobbled already
    myGLCD.fillCircle(413, 260, 2); // Dot 83
     }    
      if (dot[65] == 1) {  // Check if dot 65 gobbled already
    myGLCD.fillCircle(435, 240, 2); // Dot 65
     }
      if (dot[85] == 1) {  // Check if dot 85 gobbled already
    myGLCD.fillCircle(459, 260, 2); // Dot 85
     }        
  } else
  if (xG == 448) { // dot 85 +22
   if (dot[84] == 1) {  // Check if dot 84 gobbled already
    myGLCD.fillCircle(436, 260, 2); // Dot 84
   }     
   if (dot[94] == 1) {  // Check if dot 94 gobbled already
    myGLCD.fillCircle(465, 281, 7); // Big dot 94
   } 
  } 

} else
if (yG == 268) {  // if in Row 5  **********************************************************
  if (xG == 4) { // dot 86
     if (dot[66] == 1) {  // Check if dot 66 gobbled already
	  myGLCD.fillCircle(19, 260, 2); // Dot 66
     } 
     if (dot[95] == 1) {  // Check if dot 95 gobbled already
	  myGLCD.fillCircle(19, 301, 2); // Dot 95
     }   
  } else
   if (xG == 62) { // dot 87
     if (dot[68] == 1) {  // Check if dot 68 gobbled already
	  myGLCD.fillCircle(65, 260, 2); // Dot 68
     } 
     if (dot[69] == 1) {  // Check if dot 69 gobbled already
	  myGLCD.fillCircle(88, 260, 2); // Dot 69
     } 
     if (dot[97] == 1) {  // Check if dot 97 gobbled already
	  myGLCD.fillCircle(65, 301, 2); // Dot 97
     }
     if (dot[98] == 1) {  // Check if dot 98 gobbled already
	  myGLCD.fillCircle(88, 301, 2); // Dot 98
     }      
     
  } else
  
  if (xG == 120) { // dot 88
     if (dot[71] == 1) {  // Check if dot 71 gobbled already
	  myGLCD.fillCircle(136, 260, 2); // Dot 71
     }     
     if (dot[100] == 1) {  // Check if dot 100 gobbled already
	  myGLCD.fillCircle(136, 301, 2); // Dot 100
     }    
  } else
  if (xG == 168) { // dot 89
     if (dot[73] == 1) {  // Check if dot 73 gobbled already
	  myGLCD.fillCircle(183, 260, 2); // Dot 73
     }     
     if (dot[101] == 1) {  // Check if dot 101 gobbled already
	  myGLCD.fillCircle(183, 301, 2); // Dot 101
     }       
  } else
  
  if (xG == 228) { // dot 90
     if (dot[75] == 1) {  // Check if dot 75 gobbled already
	  myGLCD.fillCircle(229, 260, 2); // Dot 75
     }
     if (dot[76] == 1) {  // Check if dot 76 gobbled already
	  myGLCD.fillCircle(252, 260, 2); // Dot 76
     } 
     if (dot[103] == 1) {  // Check if dot 103 gobbled already
	  myGLCD.fillCircle(229, 301, 2); // Dot 103
     } 
     if (dot[104] == 1) {  // Check if dot 104 gobbled already
	  myGLCD.fillCircle(252, 301, 2); // Dot 104
     }      
     
  }else
  
  if (xG == 284) { // dot 91
     if (dot[78] == 1) {  // Check if dot 78 gobbled already
   myGLCD.fillCircle(298, 260, 2); // Dot 78
     }
     if (dot[106] == 1) {  // Check if dot 106 gobbled already
    myGLCD.fillCircle(298, 301, 2); // Dot 106
     }      
     
  }else
  
  if (xG == 332) { // dot 92
     if (dot[80] == 1) {  // Check if dot 80 gobbled already
   myGLCD.fillCircle(344, 260, 2); // Dot 80
     }
     if (dot[107] == 1) {  // Check if dot 107 gobbled already
    myGLCD.fillCircle(345, 301, 2);  // dot 107
     }       
     
  }else
  
  if (xG == 394) { // dot 93
     if (dot[82] == 1) {  // Check if dot 82 gobbled already
    myGLCD.fillCircle(390, 260, 2); // Dot 82
     }
     if (dot[83] == 1) {  // Check if dot 83 gobbled already
    myGLCD.fillCircle(413, 260, 2); // Dot 83
     } 
     if (dot[109] == 1) {  // Check if dot 109 gobbled already
    myGLCD.fillCircle(391, 301, 2);  // dot 109
     } 
     if (dot[110] == 1) {  // Check if dot 110 gobbled already
    myGLCD.fillCircle(414, 301, 2);  // dot 110
     }     
     
  } else
  
  if (xG == 448) { // dot 94
     if (dot[85] == 1) {  // Check if dot 85 gobbled already
	  myGLCD.fillCircle(459, 260, 2); // Dot 85
     } 
     if (dot[112] == 1) {  // Check if dot 112 gobbled already
	  myGLCD.fillCircle(460, 301, 2);  // dot 112
     }    
  } 

} else


if (yG == 288) {  // if in Row 6  **********************************************************
  if (xG == 4) { // dot 95
     if (dot[86] == 1) {  // Check if dot 86 gobbled already
	  myGLCD.fillCircle(19, 281, 7); // Big dot 86
     } 
     if (dot[96] == 1) {  // Check if dot 96 gobbled already
	  myGLCD.fillCircle(42, 221, 2); // dot 62
     }    
  } else
  if (xG == 28) { // dot 96
     if (dot[95] == 1) {  // Check if dot 95 gobbled already
	  myGLCD.fillCircle(19, 301, 2); // Dot 95
     }  
     if (dot[97] == 1) {  // Check if dot 97 gobbled already
	  myGLCD.fillCircle(65, 301, 2); // Dot 97
     }      
  } else
  if (xG == 52) { // dot 97
     if (dot[96] == 1) {  // Check if dot 96 gobbled already
	  myGLCD.fillCircle(42, 301, 2); // Dot 96
     } 
     if (dot[98] == 1) {  // Check if dot 98 gobbled already
	  myGLCD.fillCircle(88, 301, 2); // Dot 98
     }  
     if (dot[87] == 1) {  // Check if dot 87 gobbled already
	  myGLCD.fillCircle(77, 281, 2); // Dot 87 
     }      
  } else
  if (xG == 74) { // dot 98
     if (dot[97] == 1) {  // Check if dot 97 gobbled already
	  myGLCD.fillCircle(65, 301, 2); // Dot 97
     } 
     if (dot[87] == 1) {  // Check if dot 87 gobbled already
	  myGLCD.fillCircle(77, 281, 2); // Dot 87
     }  
     if (dot[99] == 1) {  // Check if dot 99 gobbled already
	  myGLCD.fillCircle(112, 301, 2); // Dot 99
     }     
  } else
  if (xG == 98) { // dot 99
     if (dot[98] == 1) {  // Check if dot 98 gobbled already
	  myGLCD.fillCircle(88, 301, 2); // Dot 98
     } 
     if (dot[100] == 1) {  // Check if dot 100 gobbled already
	  myGLCD.fillCircle(136, 301, 2); // Dot 100
     }   
  } else
  if (xG == 120) { // dot 100
     if (dot[99] == 1) {  // Check if dot 99 gobbled already
	  myGLCD.fillCircle(112, 301, 2); // Dot 99
     } 
     if (dot[88] == 1) {  // Check if dot 88 gobbled already
	  myGLCD.fillCircle(136, 281, 2); // Dot 88
     }     
  } else
  if (xG == 168) { // dot 101
     if (dot[89] == 1) {  // Check if dot 89 gobbled already
	  myGLCD.fillCircle(183, 281, 2); // Dot 89
     } 
     if (dot[102] == 1) {  // Check if dot 102 gobbled already
	  myGLCD.fillCircle(206, 301, 2); // Dot 102
     }     
  } else
  if (xG == 192) { // dot 102
     if (dot[101] == 1) {  // Check if dot 101 gobbled already
	  myGLCD.fillCircle(183, 301, 2); // Dot 101
     } 
     if (dot[103] == 1) {  // Check if dot 103 gobbled already
	  myGLCD.fillCircle(229, 301, 2); // Dot 103
     }    
  } else
  if (xG == 216) { // dot 103
     if (dot[102] == 1) {  // Check if dot 102 gobbled already
	  myGLCD.fillCircle(206, 301, 2); // Dot 102
     } 
     if (dot[90] == 1) {  // Check if dot 90 gobbled already
	  myGLCD.fillCircle(241, 281, 2); // Dot 90
     }
     if (dot[104] == 1) {  // Check if dot 104 gobbled already
	  myGLCD.fillCircle(252, 301, 2); // Dot 104
     }   
  } else
  if (xG == 238) { // dot 104
     if (dot[103] == 1) {  // Check if dot 103 gobbled already
	  myGLCD.fillCircle(229, 301, 2); // Dot 103
     } 
     if (dot[90] == 1) {  // Check if dot 90 gobbled already
	  myGLCD.fillCircle(241, 281, 2); // Dot 90
     }
     if (dot[105] == 1) {  // Check if dot 105 gobbled already
	  myGLCD.fillCircle(275, 301, 2); // Dot 105
     }       
  } else
  if (xG == 262) { // dot 105
     if (dot[104] == 1) {  // Check if dot 104 gobbled already
	  myGLCD.fillCircle(252, 301, 2); // Dot 104
     }  
     if (dot[106] == 1) {  // Check if dot 106 gobbled already
	  myGLCD.fillCircle(298, 301, 2); // Dot 106
     }        
  } else
  if (xG == 284) { // dot 106
     if (dot[105] == 1) {  // Check if dot 105 gobbled already
	  myGLCD.fillCircle(275, 301, 2); // Dot 105
     } 
     if (dot[91] == 1) {  // Check if dot 91 gobbled already
	  myGLCD.fillCircle(298, 281, 2);  // dot 91
     }    
  } else
  if (xG == 332) { // dot 107
     if (dot[92] == 1) {  // Check if dot 92 gobbled already
   myGLCD.fillCircle(347, 281, 2); // Dot 92
     } 
     if (dot[108] == 1) {  // Check if dot 108 gobbled already
    myGLCD.fillCircle(368, 301, 2);  // dot 108
     }    
  } else
  if (xG == 356) { // dot 108
     if (dot[107] == 1) {  // Check if dot 107 gobbled already
    myGLCD.fillCircle(345, 301, 2);  // dot 107
     } 
     if (dot[109] == 1) {  // Check if dot 109 gobbled already
    myGLCD.fillCircle(391, 301, 2);  // dot 109
     }      
  } else
  if (xG == 380) { // dot 109
     if (dot[108] == 1) {  // Check if dot 108 gobbled already
    myGLCD.fillCircle(368, 301, 2);  // dot 108
     } 
     if (dot[93] == 1) {  // Check if dot 93 gobbled already
    myGLCD.fillCircle(406, 281, 2); // Dot 93
     }
     if (dot[110] == 1) {  // Check if dot 110 gobbled already
    myGLCD.fillCircle(414, 301, 2);  // dot 110
     }    
  } else
  if (xG == 402) { // dot 110
     if (dot[109] == 1) {  // Check if dot 109 gobbled already
    myGLCD.fillCircle(391, 301, 2);  // dot 109
     } 
     if (dot[93] == 1) {  // Check if dot 93 gobbled already
    myGLCD.fillCircle(406, 281, 2); // Dot 93
     }
     if (dot[111] == 1) {  // Check if dot 111 gobbled already
    myGLCD.fillCircle(437, 301, 2);  // dot 111
     }       
  } else
  if (xG == 424) { // dot 111
     if (dot[110] == 1) {  // Check if dot 110 gobbled already
    myGLCD.fillCircle(414, 301, 2);  // dot 110
     }  
     if (dot[112] == 1) {  // Check if dot 112 gobbled already
    myGLCD.fillCircle(460, 301, 2);  // dot 112
     }       
  } else
  if (xG == 448) { // dot 112
     if (dot[111] == 1) {  // Check if dot 111 gobbled already
    myGLCD.fillCircle(437, 301, 2);  // dot 111
     } 
     if (dot[94] == 1) {  // Check if dot 94 gobbled already
    myGLCD.fillCircle(465, 281, 7); // Big dot 94
     }     
  }
} else



// Check Columns
//delay(dly); // Larry Delay
//    Serial.println("Got there");
if (xG == 28) {  // if in Column 2
  if (yG == 66) { // dot 48
     if (dot[29] == 1) {  // Check if dot 29 gobbled already
	  myGLCD.fillCircle(42, 60, 2); // Dot 29
     }     
     if (dot[50] == 1) {  // Check if dot 50 gobbled already
	  myGLCD.fillCircle(42, 100, 2); // Dot 50
     }        
  } else
  if (yG == 86) { // dot 50
      if (dot[48] == 1) {  // Check if dot 48 gobbled already
  	myGLCD.fillCircle(42, 80, 2); // Dot 48
     }  
      if (dot[52] == 1) {  // Check if dot 52 gobbled already
  	myGLCD.fillCircle(42, 120, 2); // Dot 52
     }      
  } else
  if (yG == 106) { // dot 52
     if (dot[50] == 1) {  // Check if dot 50 gobbled already
	  myGLCD.fillCircle(42, 100, 2); // Dot 50
     }     
     if (dot[54] == 1) {  // Check if dot 54 gobbled already
	  myGLCD.fillCircle(42, 140, 2); // Dot 54
     }      
  } else
  if (yG == 126) { // dot 54
      if (dot[52] == 1) {  // Check if dot 52 gobbled already
  	myGLCD.fillCircle(42, 120, 2); // Dot 52
     } 
      if (dot[56] == 1) {  // Check if dot 56 gobbled already
  	myGLCD.fillCircle(42, 160, 2); // Dot 56
     }     
  } else
  if (yG == 146) { // dot 56
     if (dot[54] == 1) {  // Check if dot 54 gobbled already
	  myGLCD.fillCircle(42, 140, 2); // Dot 54
     }     
     if (dot[58] == 1) {  // Check if dot 58 gobbled already
	  myGLCD.fillCircle(42, 180, 2); // Dot 58
     } // ****************  Added more decisions Larry **************     
  } else
  if (yG == 166) { // dot 58
      if (dot[56] == 1) {  // Check if dot 56 gobbled already
   myGLCD.fillCircle(42, 160, 2); // Dot 56
     } 
      if (dot[60] == 1) {  // Check if dot 60 gobbled already
    myGLCD.fillCircle(42, 200, 2); // Dot 60
     }         
  } else
  if (yG == 186) { // dot 60
     if (dot[58] == 1) {  // Check if dot 58 gobbled already
    myGLCD.fillCircle(42, 180, 2); // Dot 58
     }     
     if (dot[62] == 1) {  // Check if dot 62 gobbled already
    myGLCD.fillCircle(42, 220, 2); // Dot 62
     }      
  } else
  if (yG == 206) { // dot 62
      if (dot[60] == 1) {  // Check if dot 60 gobbled already
   myGLCD.fillCircle(42, 200, 2); // Dot 60
     } 
      if (dot[64] == 1) {  // Check if dot 64 gobbled already
    myGLCD.fillCircle(42, 240, 2); // Dot 64
     }       
  } else
  if (yG == 226) { // dot 64
     if (dot[62] == 1) {  // Check if dot 62 gobbled already
    myGLCD.fillCircle(42, 220, 2); // Dot 62
     }     
     if (dot[67] == 1) {  // Check if dot 67 gobbled already
    myGLCD.fillCircle(42, 260, 2); // Dot 67
     }      
  } 

} else

if (xG == 424) {  // if in Column 7
  
  if (yG == 66) { // dot 49
      if (dot[46] == 1) {  // Check if dot 46 gobbled already
  	myGLCD.fillCircle(436, 60, 2); // Dot 46
     }   
      if (dot[51] == 1) {  // Check if dot 51 gobbled already
  	myGLCD.fillCircle(435, 100, 2); // Dot 51
     }   
  } else
  if (yG == 86) { // dot 51
      if (dot[49] == 1) {  // Check if dot 49 gobbled already
  	myGLCD.fillCircle(435, 80, 2); // Dot 49
     }  
      if (dot[53] == 1) {  // Check if dot 53 gobbled already
  	myGLCD.fillCircle(435, 120, 2); // Dot 53
     }     
  } else
  if (yG == 106) { // dot 53
      if (dot[51] == 1) {  // Check if dot 51 gobbled already
  	myGLCD.fillCircle(435, 100, 2); // Dot 51
     }  
      if (dot[55] == 1) {  // Check if dot 55 gobbled already
  	myGLCD.fillCircle(435, 140, 2); // Dot 55
    Serial.println("D53");
     }      
  } else
  if (yG == 126) { // dot 55
      if (dot[53] == 1) {  // Check if dot 53 gobbled already
  	myGLCD.fillCircle(435, 120, 2); // Dot 53
     }
     if (dot[57] == 1) {  // Check if dot 57 gobbled already
	  myGLCD.fillCircle(435, 160, 2); // Dot 57
     }       
  } else
  if (yG == 146) { // dot 57
      if (dot[55] == 1) {  // Check if dot 55 gobbled already
  	myGLCD.fillCircle(435, 140, 2); // Dot 55
     } 
     if (dot[59] == 1) {  // Check if dot 59 gobbled already
	  myGLCD.fillCircle(435, 180, 2); // Dot 59 
     } // ******************* Added more decisions Larry ***************************** 
        
  } else
  if (yG == 166) { // dot 59
      if (dot[57] == 1) {  // Check if dot 57 gobbled already
   myGLCD.fillCircle(435, 160, 2); // Dot 57
     }  
      if (dot[61] == 1) {  // Check if dot 61 gobbled already
    myGLCD.fillCircle(435, 200, 2); // Dot 61
     }     
  } else
  if (yG == 186) { // dot 61
      if (dot[59] == 1) {  // Check if dot 59 gobbled already
    myGLCD.fillCircle(435, 180, 2); // Dot 59
     }  
      if (dot[63] == 1) {  // Check if dot 63 gobbled already
    myGLCD.fillCircle(435, 220, 2); // Dot 63
     }      
  } else
  if (yG == 206) { // dot 63
      if (dot[61] == 1) {  // Check if dot 61 gobbled already
    myGLCD.fillCircle(435, 200, 2); // Dot 61
     }
     if (dot[65] == 1) {  // Check if dot 65 gobbled already
    myGLCD.fillCircle(435, 240, 2); // Dot 65
     }       
  } else
  if (yG == 226) { // dot 65
      if (dot[63] == 1) {  // Check if dot 63 gobbled already
    myGLCD.fillCircle(435, 220, 2); // Dot 63
     } 
     if (dot[84] == 1) {  // Check if dot 84 gobbled already
    myGLCD.fillCircle(436, 260, 2); // Dot 84 
     }     
  } 
 
}




// Capture legacy direction to enable adequate blanking of trail
prevGD = GD;
// Ghost motion right 
if(GD == 0){
// Increment xG and then test if any decisions required on turning up or down
  xG = xG+2; 

/* Temp print variables for testing
  myGLCD.setFont(SmallFont);
  myGLCD.setColor(0, 0, 0);
  myGLCD.setBackColor(114, 198, 206);
  myGLCD.printNumI(xG,80,165); // Print xG
  myGLCD.printNumI(yP,110,165); // Print yP
*/


 // There are four horizontal rows that need rules

  // First Horizontal Row
  if (yG== 4) { 

    // Past first block decide to continue or go down
    if (xG == 62) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = gdirect; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }
    // Past second block only option is down
    if (xG == 120) { 
         GD = 1; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
    // Past third block decide to continue or go down
    if (xG == 228) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = gdirect; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }
    // ********************* added two more doorway decisions ********************
    // Past fourth block only option is down
    if (xG == 284) { // plus 58
         GD = 1; // set Ghost direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
    // Past fifth block decide to continue or go down
    if (xG == 394) { // plus 108
      direct = random(2); // generate random number between 0 and 1
      if (direct == 1){
         GD = direct; // set Ghost direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }    
    // Past sixth block only option is down
    if (xG == 448) { 
         GD = 1; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
  }

  // 2nd Horizontal Row
  if (yG == 46) { 

    // Past upper doorway on left decide to continue right or go down
    if (xG == 28) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = gdirect; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

    // Past first block decide to continue right or go up
    if (xG == 62) { 
      if (random(2) == 0){
         GD = 0; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } else { GD = 3;}
    }
     // Past Second block decide to continue right or go up
    if (xG == 120) { 
      if (random(2) == 0){
         GD = 0; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } else { GD = 3;}
    }

     // Past Mid Wall decide to continue right or go up
    if (xG == 168) { 
      if (random(2) == 0){
         GD = 0; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } else { GD = 3;}
    }

    // Past third block decide to continue right or go up
    if (xG == 228) { 
      if (random(2) == 0){
         GD = 0; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } else { GD = 3;}
    }
// ************* Larry ******************
     // Past Forth block no option go down
    if (xG == 284) { 
      if (random(2) == 0){
         GD = 0; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } else { GD = 3;}
    }

     // Past Second Wall decide to continue right or go up
    if (xG == 332) { 
      if (random(2) == 0){
         GD = 0; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } else { GD = 3;}
    }
        // Past fifth block decide to continue right or go up
    if (xG == 394) { 
      if (random(2) == 0){
         GD = 0; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } else { GD = 3;}
    }
// **** End Larry ****
    // Past last clock digit decide to continue or go down
    if (xG == 424) { 
      gdirect = random(2); // generate random number between 0 and 2
      if (gdirect == 1){
         GD = gdirect; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

    // Past sixth block only option is up
    if (xG == 448) {
         GD = 3; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
  }

  // 3rd Horizontal Row
  if (yG== 248) { // Larry change from 168 to 248

    // Past lower doorway on left decide to continue right or go up
    if (xG == 28) { 
      if (random(2) == 0){
         GD = 0; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } else { GD = 3;}
    }

    // Past first block decide to continue or go down
    if (xG == 62) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = gdirect; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }
     // Past Second block decide to continue or go down
    if (xG == 120) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = gdirect; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

     // Past Mid Wall decide to continue or go down
    if (xG == 168) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = gdirect; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

    // Past third block decide to continue or go down
    if (xG == 228) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = gdirect; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

    // Past last clock digit decide to continue right or go up
    if (xG == 424) { 
      if (random(2) == 0){
         GD = 0; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } else { GD = 3;}
    }

    // Past fourth block only option is down
    if (xG == 448) { 
         GD = 1; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
  }
 
  
  // 4th Horizontal Row
  if (yG== 288) { 

    // Past first block decide to continue right or go up
    if (xG == 62) { 
      if (random(2) == 0){
         GD = 0; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } else { GD = 3;}
    }
    // Past second block only option is up
    if (xG == 120) { 
         GD = 3; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
    // Past third block decide to continue right or go up
    if (xG == 228) { 
      if (random(2) == 0){
         GD = 0; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } else { GD = 3;}
    }
    // Past fourth block only option is up
    if (xG == 284) { 
         GD = 3; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
// ***************** larry mod ********************
    // Past fifth block decide to continue right or go up
    if (xG == 332) { 
      if (random(2) == 0){
         GD = 0; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } else { GD = 3;}
    }
    // Past sixth block only option is up
    if (xG == 448) { 
         GD = 3; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
   }
}

//****************************************************************************************************************************
//Left hand motion **********************************************************************************************************
//****************************************************************************************************************************

else if(GD == 2){
// Increment xG and then test if any decisions required on turning up or down
  xG = xG-2; 

/* Temp print variables for testing
  myGLCD.setFont(SmallFont);
  myGLCD.setColor(0, 0, 0);
  myGLCD.setBackColor(114, 198, 206);
  myGLCD.printNumI(xG,80,165); // Print xG
  myGLCD.printNumI(yP,110,165); // Print yP
*/

 // There are four horizontal rows that need rules

  // First Horizontal Row  ******************************
  if (yG== 4) { 

     // Past first block only option is down
    if (xG == 4) { 
         GD = 1; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
 
    // Past second block decide to continue or go down
    if (xG == 62) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = gdirect; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }
    // Past third block only option is down
    if (xG == 168) { 
         GD = 1; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
    // Past fourth block decide to continue or go down
    if (xG == 228) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = gdirect; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }
// *********** Added more decisions Larry ***************
    // Past third block only option is down
    if (xG == 332) { 
         GD = 1; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
    // Past fourth block decide to continue or go down
    if (xG == 394) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = gdirect; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }
  }

  // 2nd Horizontal Row ******************************
  if (yG== 46) { 

    // Meet LHS wall only option is up
    if (xG == 4) { 
         GD = 3; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
    // Meet upper doorway on left decide to continue left or go down
    if (xG == 28) { 
      if (random(2) == 0){
         GD = 1; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } else { GD = 2;}
    }

    // Meet first block decide to continue left or go up
    if (xG == 62) { 
      if (random(2) == 0){
         GD = 3; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } else { GD = 2;}
    }
     // Meet Second block decide to continue left or go up
    if (xG == 120) { 
      if (random(2) == 0){
         GD = 3; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } else { GD = 2;}
    }

     // Meet Mid Wall decide to continue left or go up
    if (xG == 168) { 
      if (random(2) == 0){
         GD = 3; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } else { GD = 2;}
    }

    // Meet third block decide to continue left or go up
    if (xG == 228) { 
      if (random(2) == 0){
         GD = 3; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } else { GD = 2;}
    }
// ********** Add more decisions Larry ************
     // Meet Second block decide to continue left or go up
    if (xG == 284) { 
      if (random(2) == 0){
         GD = 3; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } else { GD = 2;}
    }

     // Meet Mid Wall decide to continue left or go up
    if (xG == 332) { 
      if (random(2) == 0){
         GD = 3; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } else { GD = 2;}
    }

    // Meet third block decide to continue left or go up
    if (xG == 394) { 
      if (random(2) == 0){
         GD = 3; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } else { GD = 2;}
    }
    // Meet last clock digit decide to continue or go down
    if (xG == 424) { 
      gdirect = random(2); // generate random number between 0 and 3
      if (gdirect == 1){
         GD = gdirect; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

  }
 
   // RHS Door Horizontal Row
  if (yG == 148) { 

    // Past upper doorway on left decide to go up or go down
    if (xG == 424) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = 1; // set Pacman direciton varialble to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { GD = 3;}    
    }
  } 

  // 3rd Horizontal Row ******************************
  if (yG== 248) { // Larry change from 168 to 248

    // Meet LHS lower wall only option is down
    if (xG == 4) { 
         GD = 1; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }

    // Meet lower doorway on left decide to continue left or go up
    if (xG == 28) { 
      if (random(2) == 0){
         GD = 3; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } else { GD = 2;}
    }

    // Meet first block decide to continue or go down
    if (xG == 62) { 
      gdirect = random(2); // generate random number between 0 and 3
      if (gdirect == 1){
         GD = gdirect; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }
     // Meet Second block decide to continue or go down
    if (xG == 120) { 
      gdirect = random(2); // generate random number between 0 and 3
      if (gdirect == 1){
         GD = gdirect; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

     // Meet Mid Wall decide to continue or go down
    if (xG == 168) { 
      gdirect = random(2); // generate random number between 0 and 3
      if (gdirect == 1){
         GD = gdirect; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

    // Meet third block decide to continue or go down
    if (xG == 228) { 
      gdirect = random(2); // generate random number between 0 and 3
      if (gdirect == 1){
         GD = gdirect; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }
// *************** Add decisions Larry***************
     // Meet Second block decide to continue or go down
    if (xG == 284) { 
      gdirect = random(2); // generate random number between 0 and 3
      if (gdirect == 1){
         GD = gdirect; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

     // Meet Mid Wall decide to continue or go down
    if (xG == 332) { 
      gdirect = random(2); // generate random number between 0 and 3
      if (gdirect == 1){
         GD = gdirect; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

    // Meet third block decide to continue or go down
    if (xG == 394) { 
      gdirect = random(2); // generate random number between 0 and 3
      if (gdirect == 1){
         GD = gdirect; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      }
    }

    // Meet last clock digit above decide to continue left or go up
    if (xG == 424) { 
      if (random(2) == 0){
         GD = 3; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } else { GD = 2;}
    
    }
    
  }
   // 4th Horizontal Row ******************************
  if (yG== 288) { 

    // Meet LHS wall only option is up
    if (xG == 4) { 
         GD = 3; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }  
    // Meet first block decide to continue left or go up
    if (xG == 62) { 
      if (random(2) == 0){
         GD = 3; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } else { GD = 2;}
    }
    // Meet bottom divider wall only option is up
    if (xG == 168) { 
         GD = 3; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
    // Meet 3rd block decide to continue left or go up
    if (xG == 228) { 
      if (random(2) == 0){
         GD = 3; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } else { GD = 2;}
    }
   
//*******************  Add Larry ********************
    // Meet bottom divider wall only option is up
    if (xG == 332) { 
         GD = 3; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
    // Meet 3rd block decide to continue left or go up
    if (xG == 394) { 
      if (random(2) == 0){
         GD = 3; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } else { GD = 2;}
    }   
    
  }
}  
  


//****************************************************************************************************************************
//Down motion **********************************************************************************************************
//****************************************************************************************************************************

else if(GD == 1){
// Increment yGand then test if any decisions required on turning up or down
  yG= yG+2; 

/* Temp print variables for testing
  myGLCD.setFont(SmallFont);
  myGLCD.setColor(0, 0, 0);
  myGLCD.setBackColor(114, 198, 206);
  myGLCD.printNumI(xG,80,165); // Print xG
  myGLCD.printNumI(yP,110,165); // Print yP
*/

 // There are vertical rows that need rules

  // First Vertical Row  ******************************
  if (xG == 4) { 

     // Past first block only option is right
    if (yG== 46) { 
         GD = 0; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
 
    // Towards bottom wall only option right
    if (yG== 288) { 
         GD = 0; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    } 
  }

  // 2nd Vertical Row ******************************
  if (xG == 28) { 

    // Meet bottom doorway on left decide to go left or go right
    if (yG== 248) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { GD = 0;}    
    }
  }

  // 3rd Vertical Row ******************************
  if (xG == 62) { 

    // Meet top lh digit decide to go left or go right
    if (yG== 46) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { GD = 0;}    
    }

    // Meet bottom wall decide to go left or go right
    if (yG== 288) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { GD = 0;}    
    }
  }

  // 5th Vertical Row ******************************
  if (xG == 120) { 

    // Meet top lh digit decide to go left or go right
    if (yG== 46) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { GD = 0;}    
    }

    // Meet bottom wall only opgion to go left
    if (yG== 288) { 
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up   
    }
  }

  // 6th Vertical Row ******************************
  if (xG == 168) { 

    // Meet top lh digit decide to go left or go right
    if (yG== 46) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { GD = 0;}    
    }

    // Meet bottom wall only opgion to go right
    if (yG== 288) { 
         GD = 0; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up   
    }
  }

  // 8th Vertical Row ******************************
  if (xG == 228) { 

    // Meet top lh digit decide to go left or go right
    if (yG== 46) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { GD = 0;}    
    }

    // Meet bottom wall
    if (yG== 288) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { GD = 0;}    
    }
  }
//****************** Larry *****************************
  // 5ish Vertical Row ******************************
  if (xG == 284) { 

    // Meet top lh digit decide to go left or go right
    if (yG== 46) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { GD = 0;}    
    }

    // Meet bottom wall only opgion to go left
    if (yG== 288) { 
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up   
    }
  }

  // 6ish Vertical Row ******************************
  if (xG == 332) {

    // Meet top lh digit decide to go left or go right
    if (yG== 46) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { GD = 0;}    
    }

    // Meet bottom wall only opgion to go right
    if (yG== 288) { 
         GD = 0; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up   
    }
  }

  // 8ish Vertical Row ******************************
  if (xG == 394) { 

    // Meet top lh digit decide to go left or go right
    if (yG== 46) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { GD = 0;}    
    }

    // Meet bottom wall
    if (yG== 288) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { GD = 0;}    
    }
  }
  //****************** end Larry ********************************
  // 9th Vertical Row ******************************
  if (xG == 424) { 

    // Meet bottom right doorway  decide to go left or go right
    if (yG== 248) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { GD = 0;}    
    }
  }

  // 10th Vertical Row  ******************************
  if (xG == 448) { 

     // Past first block only option is left
    if (yG== 46) { 
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
 
    // Towards bottom wall only option right
    if (yG== 288) { 
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    } 
  } 
}  

//****************************************************************************************************************************
//Up motion **********************************************************************************************************
//****************************************************************************************************************************

else if(GD == 3){
// Decrement yGand then test if any decisions required on turning up or down
  yG= yG-2; 

/* Temp print variables for testing
  myGLCD.setFont(SmallFont);
  myGLCD.setColor(0, 0, 0);
  myGLCD.setBackColor(114, 198, 206);
  myGLCD.printNumI(xG,80,165); // Print xG
  myGLCD.printNumI(yP,110,165); // Print yP
*/


 // There are vertical rows that need rules

  // First Vertical Row  ******************************
  if (xG == 4) { 

     // Past first block only option is right
    if (yG== 4) { 
         GD = 0; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
 
    // Towards bottom wall only option right
    if (yG== 248) { 
         GD = 0; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    } 
  }

  // 2nd Vertical Row ******************************
  if (xG == 28) { 

    // Meet top doorway on left decide to go left or go right
    if (yG== 46) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { GD = 0;}    
    }
  }

  // 3rd Vertical Row ******************************
  if (xG == 62) { 

    // Meet top wall decide to go left or go right
    if (yG== 4) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { GD = 0;}    
    }

    // Meet bottom lh digit decide to go left or go right
    if (yG== 248) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { GD = 0;}    
    }
  }

  // 5th Vertical Row ******************************
  if (xG == 120) { 

    // Meet bottom lh digit decide to go left or go right
    if (yG== 248) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { GD = 0;}    
    }

    // Meet top wall only opgion to go left
    if (yG== 4) { 
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up   
    }
  }

  // 6th Vertical Row ******************************
  if (xG == 168) { 

    // Meet bottom lh digit decide to go left or go right
    if (yG== 248) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { GD = 0;}    
    }

    // Meet top wall only opgion to go right
    if (yG== 4) { 
         GD = 0; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up   
    }
  }

  // 8th Vertical Row ******************************
  if (xG == 228) { 

    // Meet bottom lh digit decide to go left or go right
    if (yG== 248) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { GD = 0;}    
    }

    // Meet top wall go left or right
    if (yG== 4) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { GD = 0;}    
    }
  }
// ****************** Larry Add **************************
  // 5ish Vertical Row ******************************
  if (xG == 284) { 

    // Meet bottom lh digit decide to go left or go right
    if (yG== 248) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { GD = 0;}    
    }

    // Meet top wall only opgion to go left
    if (yG== 4) { 
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up   
    }
  }

  // 6ish Vertical Row ******************************
  if (xG == 332) { 

    // Meet bottom lh digit decide to go left or go right
    if (yG== 248) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { GD = 0;}    
    }

    // Meet top wall only option to go right
    if (yG== 4) { 
         GD = 0; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up   
    }
  }

  // 8ish Vertical Row ******************************
  if (xG == 394) { 

    // Meet bottom lh digit decide to go left or go right
    if (yG== 248) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { GD = 0;}    
    }

    // Meet top wall go left or right
    if (yG== 4) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { GD = 0;}    
    }
  }

// ***************** End Add ****************************
  // 9th Vertical Row ******************************
  if (xG == 424) { 

    // Meet top right doorway  decide to go left or go right
    if (yG== 46) { 
      gdirect = random(2); // generate random number between 0 and 1
      if (gdirect == 1){
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
      } 
      else { GD = 0;}    
    }
  }

  // 10th Vertical Row  ******************************
  if (xG == 448) { 

     // Past first block only option is left
    if (yG== 248) { 
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    }
 
    // Towards top wall only option right
    if (yG== 4) { 
         GD = 2; // set Ghost direction variable to new direction D where 0 = right, 1 = down, 2 = left, 3 = up
    } 
  } 
}  

}
  




//******************************************************************************************************************

//******************************************************************************************************************



/*
//temp barriers

if (yP>200) {
  yP=46;
}
if(xP>260){
  xP=4;
}
*/
delay(dly); 
  


}

// ************************************************************************************************************
// ===== Update Digital Clock
// ************************************************************************************************************
 void UpdateDisp(){
 
// Clear the time area
  myGLCD.setColor(0, 0, 0);
  myGLCD.setBackColor(0, 0, 0);
//  myGLCD.fillRect(60, 80 ,262, 166);

   
  int h; // Hour value in 24 hour format
  int e; // Minute value in minute format
  int pm = 0; // Flag to detrmine if PM or AM
  
  // There are four digits that need to be drawn independently to ensure consisitent positioning of time
  int d1;  // Tens hour digit
  int d2;  // Ones hour digit
  int d3;  // Tens minute digit
  int d4;  // Ones minute digit
  

  h = hour(); // 24 hour RT clock value
  e = minute();

/* TEST
h = 12;
e = 8;
*/


// Calculate hour digit values for time

if ((h >= 10) && (h <= 12)) {     // AM hours 10,11,12
  d1 = 1; // calculate Tens hour digit
  d2 = h - 10;  // calculate Ones hour digit 0,1,2
  } else  
  if ( (h >= 22) && (h <= 24)) {    // PM hours 10,11,12
  d1 = 1; // calculate Tens hour digit
  d2 = h - 22;  // calculate Ones hour digit 0,1,2    
  } else 
  if ((h <= 9)&&(h >= 1)) {     // AM hours below ten
  d1 = 0; // calculate Tens hour digit
  d2 = h;  // calculate Ones hour digit 0,1,2    
  } else
  if ( (h >= 13) && (h <= 21)) { // PM hours below 10
  d1 = 0; // calculate Tens hour digit
  d2 = h - 12;  // calculate Ones hour digit 0,1,2 
  } else { 
    // If hour is 0
  d1 = 1; // calculate Tens hour digit
  d2 = 2;  // calculate Ones hour digit 0,1,2   
  }
    
    
// Calculate minute digit values for time

if ((e >= 10)) {  
  d3 = e/10 ; // calculate Tens minute digit 1,2,3,4,5
  d4 = e - (d3*10);  // calculate Ones minute digit 0,1,2
  } else {
    // e is less than 10
  d3 = 0;
  d4 = e;
  }  

/* Print test results
myGLCD.setFont(SmallFont);
myGLCD.printNumI(d1,10,200); // Print 0
myGLCD.printNumI(d2,40,200); // Print 0
myGLCD.printNumI(d3,70,200); // Print 0
myGLCD.printNumI(d4,100,200); // Print 0
*/


if (h>=12){ // Set 
//  h = h-12; // Work out value
  pm = 1;  // Set PM flag
} 

// *************************************************************************
// Print each digit if it has changed to reduce screen impact/flicker

// Set digit font colour to white

  myGLCD.setColor(255, 255, 255);
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.setFont(SevenSeg_XXXL_Num);
  
// First Digit
if(((d1 != c1)||(xsetup == true))&&(d1 != 0)){ // Do not print zero in first digit position
//    myGLCD.printNumI(d1,10,70); // Printing thisnumber impacts LFH walls so redraw impacted area   
    myGLCD.printNumI(d1,80,110); // Printing thisnumber impacts LFH walls so redraw impacted area   
// ---------------- reprint two left wall pillars Larry
    myGLCD.setColor(1, 73, 240);
    
//    myGLCD.drawRoundRect(0, 80, 27, 105); 
//    myGLCD.drawRoundRect(2, 85, 25, 100); 
    myGLCD.drawRoundRect(0, 80, 27, 145); // New 480X320 diff 160 and 80 (80 was 120)
    myGLCD.drawRoundRect(2, 125, 25, 140); 
//    myGLCD.drawRoundRect(0, 140, 27, 165); 
//    myGLCD.drawRoundRect(2, 145, 25, 160);
    myGLCD.drawRoundRect(0, 250, 27, 178); 
    myGLCD.drawRoundRect(2, 185, 25, 200); 

// ---------------- Clear lines on Outside wall
    myGLCD.setColor(0,0,0);
//    myGLCD.drawRoundRect(1, 238, 318, 1);
    myGLCD.drawRoundRect(1, 318, 478, 1); 



}
//If prevous time 12:59 or 00:59 and change in time then blank First Digit

if((c1 == 1) && (c2 == 2) && (c3 == 5) && (c4 == 9) && (d2 != c2) ){ // Clear the previouis First Digit and redraw wall

    myGLCD.setColor(0,0,0);
    myGLCD.fillRect(50, 70, 70, 165);


}

if((c1 == 0) && (c2 == 0) && (c3 == 5) && (c4 == 9) && (d2 != c2) ){ // Clear the previouis First Digit and redraw wall

    myGLCD.setColor(0,0,0);
    myGLCD.fillRect(50, 70, 70, 165);


}

// Reprint the dots that have not been gobbled
    myGLCD.setColor(200,200,200);
// Row 4
if ( dot[48] == 1) {
  myGLCD.fillCircle(42, 80, 2); // Dot 48
} 

// Row 5

if ( dot[50] == 1) {
  myGLCD.fillCircle(42, 100, 2); // Dot 50
}

// Row 6
if ( dot[52] == 1) {
  myGLCD.fillCircle(42, 100, 2); // Dot 50
}

// Row 7
if ( dot[54] == 1) {
  myGLCD.fillCircle(42, 140, 2); // Dot 54
}

// Row 8
if ( dot[56] == 1) {
  myGLCD.fillCircle(42, 160, 2); // Dot 56
}

// Row 9

if ( dot[58] == 1) {
  myGLCD.fillCircle(42, 180, 2); // Dot 58
}

// Row 10
if ( dot[60] == 1) {
  myGLCD.fillCircle(42, 200, 2); // Dot 60
}

// Row 11
if ( dot[62] == 1) {
  myGLCD.fillCircle(42, 220, 2); // Dot 62
}

// Row 12
if ( dot[64] == 1) {
  myGLCD.fillCircle(42, 240, 2); // Dot 64
}



  myGLCD.setColor(255, 255, 255);
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.setFont(SevenSeg_XXXL_Num);
  
// Second Digit
if((d2 != c2)||(xsetup == true)){
  myGLCD.printNumI(d2,140,110); // Print 0
}

// Third Digit
if((d3 != c3)||(xsetup == true)){
  myGLCD.printNumI(d3,203,110); // Was 145    
}

// Fourth Digit
if((d4 != c4)||(xsetup == true)){
  myGLCD.printNumI(d4,284,110); // Was 205  
}

if (xsetup == true){
  xsetup = false; // Reset Flag now leaving setup mode
  } 
 // Print PM or AM
 
      myGLCD.setColor(1, 73, 240);
      myGLCD.setBackColor(0, 0, 0);
      myGLCD.setFont(SmallFont);
  if (pm == 0) {
      myGLCD.print("AM", 549, 187); // Fixed for 480X320
   } else {
      myGLCD.print("PM", 459, 187);  // Fixed for 480X320
   }

// ----------- Alarm Set on LHS lower pillar
if (alarmstatus == true) { // Print AS on fron screenleft hand side
      myGLCD.print("AS", 7, 147); 
}


  // Round dots

  myGLCD.setColor(255, 255, 255);
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.fillCircle(207, 148, 5);
  myGLCD.fillCircle(207, 168, 5);





//--------------------- copy exising time digits to global variables so that these can be used to test which digits change in future

c1 = d1;
c2 = d2;
c3 = d3;
c4 = d4;

}




// ===== initiateGame - Custom Function
void drawscreen() {

  

  //Draw Background lines

      myGLCD.setColor(1, 73, 240);
 
// ---------------- Outside wall

        //myGLCD.drawRoundRect(0, 239, 319, 0); //Old = 320X240
        //myGLCD.drawRoundRect(2, 237, 317, 2);
        myGLCD.drawRoundRect(0, 319, 479, 0); // New = 480X320
        myGLCD.drawRoundRect(2, 317, 477, 2);  

// ---------------- Four top spacers and wall pillar
 
        myGLCD.drawRoundRect(35, 35, 60, 45); // Spacer
        myGLCD.drawRoundRect(37, 37, 58, 43);

        myGLCD.drawRoundRect(93, 35, 118, 45); //Spacer
        myGLCD.drawRoundRect(95, 37, 116, 43);

        myGLCD.drawRoundRect(155, 0, 165, 45); //Wall Pillar
        myGLCD.drawRoundRect(157, 2, 163, 43);  

        myGLCD.drawRoundRect(201, 35, 226, 45); //Spacer
        myGLCD.drawRoundRect(203, 37, 224, 43);
        
        myGLCD.drawRoundRect(258, 35, 283, 45); //Spacer
        myGLCD.drawRoundRect(260, 37, 281, 43);       
//*************New added for 480X320 display ***********
        myGLCD.drawRoundRect(315, 0, 325, 45); //Wall Pillar
        myGLCD.drawRoundRect(317, 2, 327, 43);   

        myGLCD.drawRoundRect(365, 35, 390, 45); //Spacer
        myGLCD.drawRoundRect(367, 37, 388, 43);
        
        myGLCD.drawRoundRect(422, 35, 447, 45); //Spacer
        myGLCD.drawRoundRect(424, 37, 445, 43);       
 
 
// ---------------- Four bottom spacers and wall pillar

//        myGLCD.drawRoundRect(35, 196, 60, 206); //Spacer
//        myGLCD.drawRoundRect(37, 198, 58, 204);
        myGLCD.drawRoundRect(35, 276, 60, 286); 
        myGLCD.drawRoundRect(37, 278, 58, 284);

//        myGLCD.drawRoundRect(93, 196, 118, 206); //Spacer
//        myGLCD.drawRoundRect(95, 198, 116, 204);
        myGLCD.drawRoundRect(93, 276, 118, 286); 
        myGLCD.drawRoundRect(95, 278, 116, 284);

//        myGLCD.drawRoundRect(155, 196, 165, 239); //Wall Pillar
//        myGLCD.drawRoundRect(157, 198, 163, 237); 
        myGLCD.drawRoundRect(155, 276, 165, 319); 
        myGLCD.drawRoundRect(157, 278, 163, 317);

//        myGLCD.drawRoundRect(201, 196, 226, 206); //Spacer
//        myGLCD.drawRoundRect(203, 198, 224, 204);
        myGLCD.drawRoundRect(201, 276, 226, 286); 
        myGLCD.drawRoundRect(203, 278, 224, 284);
        
//        myGLCD.drawRoundRect(258, 196, 283, 206); //Spacer
//        myGLCD.drawRoundRect(260, 198, 281, 204);          
        myGLCD.drawRoundRect(258, 276, 283, 286); 
        myGLCD.drawRoundRect(260, 278, 281, 284); 

//*************New added for 480X320 display ***********
 
        myGLCD.drawRoundRect(315, 276, 325, 319); //Wall Pillar
        myGLCD.drawRoundRect(317, 278, 327, 317);   

        myGLCD.drawRoundRect(365, 276, 390, 286); //Spacer
        myGLCD.drawRoundRect(367, 278, 388, 284);
        
        myGLCD.drawRoundRect(422, 276, 447, 286); //Spacer
        myGLCD.drawRoundRect(424, 278, 445, 284); 

        
// ---------- Four Door Pillars 

//        myGLCD.drawRoundRect(0, 80, 27, 105); // Old
//        myGLCD.drawRoundRect(2, 85, 25, 100); 
        myGLCD.drawRoundRect(0, 80, 27, 145); // New 480X320 diff 160 and 80 (80 was 120)
        myGLCD.drawRoundRect(2, 125, 25, 140); 

//        myGLCD.drawRoundRect(0, 140, 27, 165); //Old
//        myGLCD.drawRoundRect(2, 145, 25, 160); 
        myGLCD.drawRoundRect(0, 250, 27, 178); 
        myGLCD.drawRoundRect(2, 185, 25, 200); 
        
//        myGLCD.drawRoundRect(292, 80, 319, 105); 
//        myGLCD.drawRoundRect(294, 85, 317, 100); //Old = 320X240
        myGLCD.drawRoundRect(452, 80, 479, 145); 
        myGLCD.drawRoundRect(454, 125, 477, 140);

//        myGLCD.drawRoundRect(292, 140, 319, 165); 
//        myGLCD.drawRoundRect(294, 145, 317, 160);  
        myGLCD.drawRoundRect(452, 250, 479, 178); 
        myGLCD.drawRoundRect(454, 185, 477, 200); 

// ---------------- Clear lines on Outside wall
        myGLCD.setColor(0,0,0);
//       myGLCD.drawRoundRect(1, 238, 318, 1); 
        myGLCD.drawRoundRect(1, 318, 248, 1); 

//        myGLCD.fillRect(0, 106, 2, 139);
        myGLCD.fillRect(0, 146, 2, 179); 
//        myGLCD.fillRect(317, 106, 319, 139); 
        myGLCD.fillRect(477, 146, 479, 179);

// Draw Dots
  myGLCD.setColor(200, 200, 200);
  myGLCD.setBackColor(0, 0, 0);


/*
// Row 4
if ( dot[32] == 1) {
  myGLCD.fillCircle(42, 80, 2);
} 
*/



// Row 1
if ( dot[1] == 1) {
  myGLCD.fillCircle(19, 19, 2); // dot 1
  }
if ( dot[2] == 1) {  
  myGLCD.fillCircle(42, 19, 2); // dot 2
  }
if ( dot[3] == 1) {
  myGLCD.fillCircle(65, 19, 2); // dot 3
  }
if ( dot[4] == 1) {
  myGLCD.fillCircle(88, 19, 2); // dot 4
  }
if ( dot[5] == 1) {
  myGLCD.fillCircle(112, 19, 2); // dot 5
  }
if ( dot[6] == 1) {
  myGLCD.fillCircle(136, 19, 2); // dot 6   
  }  
// 
if ( dot[7] == 1) {
  myGLCD.fillCircle(183, 19, 2); // dot 7
  }
if ( dot[8] == 1) {  
  myGLCD.fillCircle(206, 19, 2);  // dot 8 
  }
if ( dot[9] == 1) {  
  myGLCD.fillCircle(229, 19, 2); // dot 9
  }
if ( dot[10] == 1) {  
  myGLCD.fillCircle(252, 19, 2); // dot 10
  }
if ( dot[11] == 1) {  
  myGLCD.fillCircle(275, 19, 2);  // dot 11
  }
if ( dot[12] == 1) {
  myGLCD.fillCircle(298, 19, 2);  // dot 12
  }
//********** add dots for 480X320 Larry ******
if ( dot[13] == 1) {
  myGLCD.fillCircle(345, 19, 2);  // dot 13
  }
if ( dot[14] == 1) {
  myGLCD.fillCircle(368, 19, 2);  // dot 14
  }
if ( dot[15] == 1) {
  myGLCD.fillCircle(391, 19, 2);  // dot 15
  }
if ( dot[16] == 1) {
  myGLCD.fillCircle(414, 19, 2);  // dot 16
  }
if ( dot[17] == 1) {
  myGLCD.fillCircle(437, 19, 2);  // dot 17
  }
if ( dot[18] == 1) {
  myGLCD.fillCircle(460, 19, 2);  // dot 18
  }
// Changed dot Numbers from here forward Larry
// Row 2
if ( dot[19] == 1) {
  myGLCD.fillCircle(19, 40, 7); // Big dot 19
  }
if ( dot[20] == 1) {
  myGLCD.fillCircle(77, 40, 2);  // dot 20
  }
if ( dot[21] == 1) {
  myGLCD.fillCircle(136, 40, 2);  // dot 21
  }
if ( dot[22] == 1) {
  myGLCD.fillCircle(183, 40, 2);  // dot 22
  }
if ( dot[23] == 1) {
  myGLCD.fillCircle(241, 40, 2);  // dot 23
  }
// ******** Added Dots Larry *******
if ( dot[24] == 1) {
  myGLCD.fillCircle(298, 40, 2);  // dot 24
  }
if ( dot[25] == 1) {
  myGLCD.fillCircle(347, 40, 2);  // dot 25
  }
if ( dot[26] == 1) {
  myGLCD.fillCircle(406, 40, 2);  // dot 26
  }
if ( dot[27] == 1) {
  myGLCD.fillCircle(465, 40, 7); // Big dot 27
  }  

  
// Row 3

if ( dot[28] == 1) {
  myGLCD.fillCircle(19, 60, 2);
}
if ( dot[29] == 1) {
  myGLCD.fillCircle(42, 60, 2);
}
if ( dot[30] == 1) {
  myGLCD.fillCircle(65, 60, 2); 
}
if ( dot[31] == 1) {
  myGLCD.fillCircle(88, 60, 2);
}
if ( dot[32] == 1) {
  myGLCD.fillCircle(112, 60, 2);
}
if ( dot[33] == 1) {
  myGLCD.fillCircle(136, 60, 2); 
}
if ( dot[34] == 1) { 
  myGLCD.fillCircle(160, 60, 2);
}
if ( dot[35] == 1) {
  myGLCD.fillCircle(183, 60, 2);
}
if ( dot[36] == 1) {
  myGLCD.fillCircle(206, 60, 2);  
}
if ( dot[37] == 1) {
  myGLCD.fillCircle(229, 60, 2);
}
if ( dot[38] == 1) {
  myGLCD.fillCircle(252, 60, 2);
}
if ( dot[39] == 1) {
  myGLCD.fillCircle(275, 60, 2); 
}
if ( dot[40] == 1) {
  myGLCD.fillCircle(298, 60, 2);   
}
//**** Add Dots Larry ****
if ( dot[41] == 1) { 
  myGLCD.fillCircle(321, 60, 2);
}
if ( dot[42] == 1) {
  myGLCD.fillCircle(344, 60, 2);
}
if ( dot[43] == 1) {
  myGLCD.fillCircle(367, 60, 2);  
}
if ( dot[44] == 1) {
  myGLCD.fillCircle(390, 60, 2);
}
if ( dot[45] == 1) {
  myGLCD.fillCircle(413, 60, 2);
}
if ( dot[46] == 1) {
  myGLCD.fillCircle(436, 60, 2); 
}
if ( dot[47] == 1) {
  myGLCD.fillCircle(459, 60, 2);   
}

// Row 4
if ( dot[48] == 1) {
  myGLCD.fillCircle(42, 80, 2);
}
if ( dot[49] == 1) {
  myGLCD.fillCircle(435, 80, 2);   
}
// Row 5
if ( dot[50] == 1) {
  myGLCD.fillCircle(42, 100, 2);
}
if ( dot[51] == 1) {
  myGLCD.fillCircle(435, 100, 2);
}
// Row 6
if ( dot[52] == 1) {
  myGLCD.fillCircle(42, 120, 2);
}
if ( dot[53] == 1) {
  myGLCD.fillCircle(435, 120, 2);
}
// Row 7
if ( dot[54] == 1) {
  myGLCD.fillCircle(42, 140, 2);
}
if ( dot[55] == 1) {
  myGLCD.fillCircle(435, 140, 2);
}
// Row 8
if ( dot[56] == 1) {
  myGLCD.fillCircle(42, 160, 2);
}
if ( dot[57] == 1) {
  myGLCD.fillCircle(435, 160, 2);
}
// Row 9
if ( dot[58] == 1) {
  myGLCD.fillCircle(42, 180, 2);
}
if ( dot[59] == 1) {
  myGLCD.fillCircle(435, 180, 2);
}
// Row 10
if ( dot[60] == 1) {
  myGLCD.fillCircle(42, 200, 2);
}
if ( dot[61] == 1) {
  myGLCD.fillCircle(435, 200, 2);
}
// Row 11
if ( dot[62] == 1) {
  myGLCD.fillCircle(42, 220, 2);
}
if ( dot[63] == 1) {
  myGLCD.fillCircle(435, 220, 2);
}
// Row 12
if ( dot[64] == 1) {
  myGLCD.fillCircle(42, 240, 2);
}
if ( dot[65] == 1) {
  myGLCD.fillCircle(435, 240, 2);
}

//************************************** Start new rows
// Row 13
if ( dot[66] == 1) {
  myGLCD.fillCircle(19, 260, 2);
}
if ( dot[67] == 1) {
  myGLCD.fillCircle(42, 260, 2);
}
if ( dot[68] == 1) {
  myGLCD.fillCircle(65, 260, 2); 
}
if ( dot[69] == 1) {
  myGLCD.fillCircle(88, 260, 2);
}
if ( dot[70] == 1) {
  myGLCD.fillCircle(112, 260, 2);
}
if ( dot[71] == 1) {
  myGLCD.fillCircle(136, 260, 2); 
}
if ( dot[72] == 1) { 
  myGLCD.fillCircle(160, 260, 2);
}
if ( dot[73] == 1) {
  myGLCD.fillCircle(183, 260, 2);
}
if ( dot[74] == 1) {
  myGLCD.fillCircle(206, 260, 2);  
}
if ( dot[75] == 1) {
  myGLCD.fillCircle(229, 260, 2);
}
if ( dot[76] == 1) {
  myGLCD.fillCircle(252, 260, 2);
}
if ( dot[77] == 1) {
  myGLCD.fillCircle(275, 260, 2); 
}
if ( dot[78] == 1) {
  myGLCD.fillCircle(298, 260, 2);   
}
//**** Add Dots Larry ****
if ( dot[79] == 1) { 
  myGLCD.fillCircle(321, 260, 2);
}
if ( dot[80] == 1) {
  myGLCD.fillCircle(344, 260, 2);
}
if ( dot[81] == 1) {
  myGLCD.fillCircle(367, 260, 2);  
}
if ( dot[82] == 1) {
  myGLCD.fillCircle(390, 260, 2);
}
if ( dot[83] == 1) {
  myGLCD.fillCircle(413, 260, 2);
}
if ( dot[84] == 1) {
  myGLCD.fillCircle(436, 260, 2); 
}
if ( dot[85] == 1) {
  myGLCD.fillCircle(459, 260, 2);   
}
// Row 14
if ( dot[86] == 1) {
  myGLCD.fillCircle(19, 281, 7); // Big dot
}
if ( dot[87] == 1) {
  myGLCD.fillCircle(77, 281, 2);
}
if ( dot[88] == 1) {
  myGLCD.fillCircle(136, 281, 2);
}
if ( dot[89] == 1) {
  myGLCD.fillCircle(183, 281, 2);
}
if ( dot[90] == 1) {
  myGLCD.fillCircle(241, 281, 2);
}
//****** added dots ******
if ( dot[91] == 1) {
  myGLCD.fillCircle(298, 281, 2);  // dot 91
  }
if ( dot[92] == 1) {
  myGLCD.fillCircle(347, 281, 2);  
  }
if ( dot[93] == 1) {
  myGLCD.fillCircle(406, 281, 2);  
  }
if ( dot[94] == 1) {
  myGLCD.fillCircle(465, 281, 7); // Big dot 94
  } 

  // Row 15
if ( dot[95] == 1) {
  myGLCD.fillCircle(19, 301, 2);
}
if ( dot[96] == 1) {
  myGLCD.fillCircle(42, 301, 2);
}
if ( dot[97] == 1) {
  myGLCD.fillCircle(65, 301, 2); 
}
if ( dot[98] == 1) { 
  myGLCD.fillCircle(88, 301, 2);
}
if ( dot[99] == 1) {
  myGLCD.fillCircle(112, 301, 2);
}
if ( dot[100] == 1) {
  myGLCD.fillCircle(136, 301, 2);   
}  
if ( dot[101] == 1) {
  myGLCD.fillCircle(183, 301, 2);
}
if ( dot[102] == 1) {
  myGLCD.fillCircle(206, 301, 2);  
}
if ( dot[103] == 1) {
  myGLCD.fillCircle(229, 301, 2);
}
if ( dot[104] == 1) {
  myGLCD.fillCircle(252, 301, 2);
}
if ( dot[105] == 1) {
  myGLCD.fillCircle(275, 301, 2); 
}
if ( dot[106] == 1) {
  myGLCD.fillCircle(298, 301, 2); 
}
//********** add dots for 480X320 Larry ******
if ( dot[107] == 1) {
  myGLCD.fillCircle(345, 301, 2);  // dot 107
  }
if ( dot[108] == 1) {
  myGLCD.fillCircle(368, 301, 2);  // dot 108
  }
if ( dot[109] == 1) {
  myGLCD.fillCircle(391, 301, 2);  // dot 109
  }
if ( dot[110] == 1) {
  myGLCD.fillCircle(414, 301, 2);  // dot 110
  }
if ( dot[111] == 1) {
  myGLCD.fillCircle(437, 301, 2);  // dot 111
  }
if ( dot[112] == 1) {
  myGLCD.fillCircle(460, 301, 2);  // dot 112
  }

// TempTest delay

// delay(100000);
/*
Redblock(62,4);
Redblock(120,4);
Redblock(228,4);
Redblock(284,4);
Redblock(394,4);
Redblock(448,4);

Greenblock(4,46); //Vert 1
Greenblock(28,46); //Vert 2
Greenblock(62,46); //Vert 3
Greenblock(94,46); // ---
Greenblock(120,46); //Vert 5
Greenblock(168,46); //wall top first Vert 6
Greenblock(228,46); //Vert 8
Greenblock(284,46);
Greenblock(332,46); //wall top second
Greenblock(394,46);
Greenblock(448,46);
/*
//Redblock(332,46);
//Redblock(394,46);
Greenblock(424,46);
Redblock(448,46);

Greenblock(28,148);
Greenblock(28,168);

Redblock(62,168); //Left edge of clock = 62

//Greenblock(4,4);
//Greenblock(62,4);
//Greenblock(168,4);
//Greenblock(228,4);
//Greenblock(332,4);
//Greenblock(394,4);

Redblock(4,334);
Redblock(28,334);
Redblock(62,334);
Redblock(120,334);
Redblock(168,334);
Redblock(228,334);
Redblock(284,334);
Redblock(332,334);
Redblock(394,334);
Redblock(424,334);

Redblock(4,288);
Redblock(62,288);
Redblock(168,288);
Redblock(228,288);
Redblock(332,288);
Redblock(394,288);

//Redblock(4,46);
//Redblock(4,288);
//Redblock(28,168);
//Redblock(62,46);
Redblock(28,168); // Out left door
Redblock(424,168); //Out right door
Redblock(424,248); // Meet bottom right doorway
Redblock(448,248); // one up from Bottom right corner
Redblock(448,288); // Bottom right corner

// Dot 111 and 112
Redblock(345,301); 
Redblock(368,301);
*/


 }
 
//***************************************************************************************************** 
//====== Draws the Pacman - bitmap
//*****************************************************************************************************
void drawPacman(int x, int y, int p, int d, int pd) {



  // Draws the Pacman - bitmap
//  // Pacman direction d == 0 = right, 1 = down, 2 = left, 3 = up
  myGLCD.setColor(0, 0, 0);
  myGLCD.setBackColor(0, 0, 0);

if ( d == 0){ // Right

if (pd == 0){ // Legacy direction Right
  myGLCD.drawRect(x-1, y, x, y+27); // Clear trail off graphic before printing new position
  }
if (pd == 3){ // Legacy direction Up
 myGLCD.drawRect(x+1, y+29, x+28, y+28); // Clear trail off graphic before printing new position
}
if (pd == 1){ // Legacy direction Down
 myGLCD.drawRect(x, y-1, x+28, y); // Clear trail off graphic before printing new position 
}
if (pd == 2){ // Legacy direction Left
 myGLCD.drawRect(x+29, y, x+28, y+28); // Clear trail off graphic before printing new positi 
}

if (p == 0) { 


   if (mspacman == false){
    myGLCD.drawBitmap (x, y, 28, 28, c_pacman); //   Closed Pacman  
   } else {
    myGLCD.drawBitmap (x, y, 28, 28, ms_c_pacman_r); //   Closed Pacman        
   }


 } else if( p == 1) {

   if (mspacman == false){
   myGLCD.drawBitmap (x, y, 28, 28, r_m_pacman); //  Medium open Pacman 
   } else {
   myGLCD.drawBitmap (x, y, 28, 28, ms_r_m_pacman); //  Medium open Pacman       
   }
   
 } else if( p == 2) {

   if (mspacman == false){
   myGLCD.drawBitmap (x, y, 28, 28, r_o_pacman); //  Open Mouth Pacman  
   } else {
   myGLCD.drawBitmap (x, y, 28, 28, ms_r_o_pacman); //  Open Mouth Pacman       
   }
 }
} else  if ( d == 1){ // Down

if (pd == 0){ // Legacy direction Right
  myGLCD.drawRect(x-1, y+6, x, y+22); // Clear trail off graphic before printing new position
  }
if (pd == 3){ // Legacy direction Up
 myGLCD.drawRect(x+6, y+29, x+22, y+28); // Clear trail off graphic before printing new position
}
if (pd == 1){ // Legacy direction Down
 myGLCD.drawRect(x+6, y-1, x+22, y); // Clear trail off graphic before printing new position 
}
if (pd == 2){ // Legacy direction Left
 myGLCD.drawRect(x+29, y+6, x+28, y+22); // Clear trail off graphic before printing new positi 
}

 if (p == 0) { 
   
   if (mspacman == false){
    myGLCD.drawBitmap (x, y, 28, 28, c_pacman); //   Closed Pacman  
   } else {
    myGLCD.drawBitmap (x, y, 28, 28, ms_c_pacman_d); //   Closed Pacman        
   }
    
    
 } else if( p == 1) {

   if (mspacman == false){
   myGLCD.drawBitmap (x, y, 28, 28, d_m_pacman); //  Medium open Pacman   
   } else {
   myGLCD.drawBitmap (x, y, 28, 28, ms_d_m_pacman); //  Medium open Pacman     
   }

 } else if( p == 2) {

   if (mspacman == false){
     myGLCD.drawBitmap (x, y, 28, 28, d_o_pacman); //  Open Mouth Pacman  
   } else {
     myGLCD.drawBitmap (x, y, 28, 28, ms_d_o_pacman); //  Open Mouth Pacman     
   }

 }
} else  if ( d == 2){ // Left

if (pd == 0){ // Legacy direction Right
  myGLCD.drawRect(x-1, y+6, x, y+22); // Clear trail off graphic before printing new position
  }
if (pd == 3){ // Legacy direction Up
 myGLCD.drawRect(x+6, y+29, x+22, y+28); // Clear trail off graphic before printing new position
}
if (pd == 1){ // Legacy direction Down
 myGLCD.drawRect(x+6, y-1, x+22, y); // Clear trail off graphic before printing new position 
}
if (pd == 2){ // Legacy direction Left
 myGLCD.drawRect(x+29, y+6, x+28, y+22); // Clear trail off graphic before printing new positi 
}

 if (p == 0) { 

   if (mspacman == false){
    myGLCD.drawBitmap (x, y, 28, 28, c_pacman); //   Closed Pacman  
   } else {
    myGLCD.drawBitmap (x, y, 28, 28, ms_c_pacman_l); //   Closed Pacman        
   }


 } else if( p == 1) {

   if (mspacman == false){
     myGLCD.drawBitmap (x, y, 28, 28, l_m_pacman); //  Medium open Pacman   
   } else {
     myGLCD.drawBitmap (x, y, 28, 28, ms_l_m_pacman); //  Medium open Pacman   
   }
   
 } else if( p == 2) {
 
   if (mspacman == false){
     myGLCD.drawBitmap (x, y, 28, 28, l_o_pacman); //  Open Mouth Pacman   
   } else {
     myGLCD.drawBitmap (x, y, 28, 28, ms_l_o_pacman); //  Open Mouth Pacman  
   }

 }
} else  if ( d == 3){ // Up

if (pd == 0){ // Legacy direction Right
  myGLCD.drawRect(x-1, y+6, x, y+22); // Clear trail off graphic before printing new position
  }
if (pd == 3){ // Legacy direction Up
 myGLCD.drawRect(x+6, y+29, x+22, y+28); // Clear trail off graphic before printing new position
}
if (pd == 1){ // Legacy direction Down
 myGLCD.drawRect(x+6, y-1, x+22, y); // Clear trail off graphic before printing new position 
}
if (pd == 2){ // Legacy direction Left
 myGLCD.drawRect(x+29, y+6, x+28, y+22); // Clear trail off graphic before printing new positi 
}

 if (p == 0) { 

   if (mspacman == false){
    myGLCD.drawBitmap (x, y, 28, 28, c_pacman); //   Closed Pacman  
   } else {
    myGLCD.drawBitmap (x, y, 28, 28, ms_c_pacman_u); //   Closed Pacman        
   }


 } else if( p == 1) {

   if (mspacman == false){
     myGLCD.drawBitmap (x, y, 28, 28, u_m_pacman); //  Medium open Pacman    
   } else {
     myGLCD.drawBitmap (x, y, 28, 28, ms_u_m_pacman); //  Medium open Pacman   
   }
   

 } else if( p == 2) {

   if (mspacman == false){
     myGLCD.drawBitmap (x, y, 28, 28, u_o_pacman); //  Open Mouth Pacman    
   } else {
     myGLCD.drawBitmap (x, y, 28, 28, ms_u_o_pacman); //  Open Mouth Pacman  
   }
   
 }

}
  
}

//************ Test Block ****************************
void Redblock(int x,int y){
myGLCD.setColor(255, 0, 0); //Set color to red
myGLCD.fillRect(x,y,(x+5),(y+5)); //add 5 for small block
}

void Greenblock(int x,int y){
myGLCD.setColor(0, 255, 0); //Set color to red
myGLCD.fillRect(x,y,(x+5),(y+5)); //add 5 for small block
}

//**********************************************************************************************************
//====== Draws the Ghost - bitmap
void drawGhost(int x, int y, int d, int pd) {


  // Draws the Ghost - bitmap
//  // Ghost direction d == 0 = right, 1 = down, 2 = left, 3 = up
  myGLCD.setColor(0, 0, 0);
  myGLCD.setBackColor(0, 0, 0);

if ( d == 0){ // Right

if (pd == 0){ // Legacy direction Right
  myGLCD.drawRect(x-1, y+6, x, y+26); // Clear trail off graphic before printing new position
  }
if (pd == 3){ // Legacy direction Up
 myGLCD.drawRect(x, y+29, x+28, y+28); // Clear trail off graphic before printing new position
}
if (pd == 1){ // Legacy direction Down
 myGLCD.drawRect(x+6, y-1, x+22, y); // Clear trail off graphic before printing new position 
}
if (pd == 2){ // Legacy direction Left
 myGLCD.drawRect(x+29, y+6, x+28, y+27); // Clear trail off graphic before printing new positi 
}

  if (fruiteatenpacman == true){ 
    myGLCD.drawBitmap (x, y, 28, 28, bluepacman); //   Closed Ghost 
  } else {
    myGLCD.drawBitmap (x, y, 28, 28, rr_ghost); //   Closed Ghost 
  }
  
} else  if ( d == 1){ // Down

if (pd == 0){ // Legacy direction Right
  myGLCD.drawRect(x-1, y+6, x, y+26); // Clear trail off graphic before printing new position
  }
if (pd == 3){ // Legacy direction Up
 myGLCD.drawRect(x, y+29, x+28, y+28); // Clear trail off graphic before printing new position
}
if (pd == 1){ // Legacy direction Down
 myGLCD.drawRect(x+6, y-1, x+22, y); // Clear trail off graphic before printing new position 
}
if (pd == 2){ // Legacy direction Left
 myGLCD.drawRect(x+29, y+6, x+28, y+27); // Clear trail off graphic before printing new positi 
}

  if (fruiteatenpacman == true){ 
    myGLCD.drawBitmap (x, y, 28, 28, bluepacman); //   Closed Ghost 
  } else {
    myGLCD.drawBitmap (x, y, 28, 28, rd_ghost); //   Closed Ghost 
  }

} else  if ( d == 2){ // Left

if (pd == 0){ // Legacy direction Right
  myGLCD.drawRect(x-1, y+6, x, y+26); // Clear trail off graphic before printing new position
  }
if (pd == 3){ // Legacy direction Up
 myGLCD.drawRect(x, y+29, x+28, y+28); // Clear trail off graphic before printing new position
}
if (pd == 1){ // Legacy direction Down
 myGLCD.drawRect(x+6, y-1, x+22, y); // Clear trail off graphic before printing new position 
}
if (pd == 2){ // Legacy direction Left
 myGLCD.drawRect(x+29, y+6, x+28, y+27); // Clear trail off graphic before printing new positi 
}

  if (fruiteatenpacman == true){ 
    myGLCD.drawBitmap (x, y, 28, 28, bluepacman); //   Closed Ghost 
  } else {
    myGLCD.drawBitmap (x, y, 28, 28, rl_ghost); //   Closed Ghost 
  }

} else  if ( d == 3){ // Up

if (pd == 0){ // Legacy direction Right
  myGLCD.drawRect(x-1, y+6, x, y+26); // Clear trail off graphic before printing new position
  }
if (pd == 3){ // Legacy direction Up
 myGLCD.drawRect(x, y+29, x+28, y+28); // Clear trail off graphic before printing new position
}
if (pd == 1){ // Legacy direction Down
 myGLCD.drawRect(x+6, y-1, x+22, y); // Clear trail off graphic before printing new position 
}
if (pd == 2){ // Legacy direction Left
 myGLCD.drawRect(x+29, y+6, x+28, y+27); // Clear trail off graphic before printing new positi 
}

  if (fruiteatenpacman == true){ 
    myGLCD.drawBitmap (x, y, 28, 28, bluepacman); //   Closed Ghost 
  } else {
    myGLCD.drawBitmap (x, y, 28, 28, ru_ghost); //   Closed Ghost 
  }

}
  
}

//**********************************************************************************************************


 // **********************************
 // ******* Enter Setup Mode *********
 // **********************************
 // Use up down arrows to change time and alrm settings

 void clocksetup(){
 
int timehour = hour();
int timeminute = minute();

// Read Alarm Set Time from Eeprom

  // read a byte from the current address of the EEPROM
  ahour = EEPROM.read(100);
  alarmhour = (int)ahour;
  if (alarmhour >24 ) {
    alarmhour = 0;
  }

  amin = EEPROM.read(101);
  alarmminute = (int)amin;
  if (alarmminute >60 ) {
    alarmminute = 0;
  }


boolean savetimealarm = false; // If save button pushed save the time and alarm

 // Setup Screen
   myGLCD.clrScr();
// ---------------- Outside wall

      myGLCD.setColor(255, 255, 0);
      myGLCD.setBackColor(0, 0, 0);

   myGLCD.drawRoundRect(0, 239, 319, 0); 
   myGLCD.drawRoundRect(2, 237, 317, 2); 
   
//Reset screenpressed flag
screenPressed = false;

// Read in current clock time and Alarm time



  // Setup buttons
    myGLCD.setFont(BigFont);

    // Time Set buttons
    myGLCD.print("+  +", 135, 38); 
    myGLCD.print("-  -", 135, 82);
    myGLCD.drawRoundRect(132, 35, 152, 55); // time hour +
    myGLCD.drawRoundRect(180, 35, 200, 55); // time minute +
    
    myGLCD.drawRoundRect(132, 80, 152, 100); // time hour -
    myGLCD.drawRoundRect(180, 80, 200, 100); // time minute -   

    // Alarm Set buttons
    myGLCD.print("+  +", 135, 138); 
    myGLCD.print("-  -", 135, 182);
    myGLCD.drawRoundRect(132, 135, 152, 155); // alarm hour +
    myGLCD.drawRoundRect(180, 135, 200, 155); // alarm minute +

    myGLCD.drawRoundRect(132, 180, 152, 200);  // alarm hour -
    myGLCD.drawRoundRect(180, 180, 200, 200); // alarm minute -  



    
    myGLCD.print("SAVE", 13, 213);
    myGLCD.print("EXIT", 245, 213);    
    myGLCD.drawRoundRect(10, 210, 80, 230);
    myGLCD.drawRoundRect(243, 210, 310, 230);  

// Get your Ghost on
    myGLCD.drawBitmap (5, 100, 28, 28, rr_ghost); //   Closed Ghost 
    myGLCD.drawBitmap (285, 100, 28, 28, bluepacman); //   Closed Ghost 


// Display MS Pacman or Pacman in menu - push to change
if (mspacman == false) {
    myGLCD.drawBitmap (154, 208, 28, 28, r_o_pacman); //   Closed Ghost 
} else {
    myGLCD.drawBitmap (154, 208, 28, 28, ms_r_o_pacman); //   Closed Ghost  
}
// Begin Loop here

while (xsetup == true){


   if (alarmstatus == true){ // flag where false is off and true is on
    myGLCD.print("SET", 220, 160);
 } else {
    myGLCD.print("OFF", 220, 160);
 }   
    myGLCD.drawRoundRect(218, 157, 268, 177);

// Draw Sound Button

    myGLCD.print("TEST", 50, 110);  // Triggers alarm sound
    myGLCD.drawRoundRect(48, 108, 116, 128);    
    
// Display Current Time
   
    myGLCD.print("Time", 40, 60);    


//    myGLCD.printNumI(timehour, 130, 60); 
 if(timehour>=10){ // Annoyingly if number less than 10 doesnt print on RHS and misses zero so need to compensate
      myGLCD.printNumI(timehour, 130, 60);   // If >= 10 just print minute
      } else {
      myGLCD.print("0", 130, 60);
      myGLCD.printNumI(timehour, 146, 60);      
      } 

    myGLCD.print(":", 160, 60);       

 if(timeminute>=10){ // Annoyingly if number less than 10 doesnt print on RHS and misses zero so need to compensate
      myGLCD.printNumI(timeminute, 175, 60);   // If >= 10 just print minute
      } else {
      myGLCD.print("0", 175, 60);
      myGLCD.printNumI(timeminute, 193, 60);      
      } 
      
   
//Display Current Alarm Setting
   
    myGLCD.print("Alarm", 40, 160);    


//    myGLCD.printNumI(alarmhour, 130, 160); 
 if(alarmhour>=10){ // Annoyingly if number less than 10 doesnt print on RHS and misses zero so need to compensate
      myGLCD.printNumI(alarmhour, 130, 160);   // If >= 10 just print minute
      } else {
      myGLCD.print("0", 130, 160);
      myGLCD.printNumI(alarmhour, 146, 160);      
      } 



    myGLCD.print(":", 160, 160);       

 if(alarmminute>=10){ // Annoyingly if number less than 10 doesnt print on RHS and misses zero so need to compensate
      myGLCD.printNumI(alarmminute, 175, 160);   // If >= 10 just print minute
      } else {
      myGLCD.print("0", 175, 160);
      myGLCD.printNumI(alarmminute, 193, 160);      
      }    

// Read input to determine if buttons pressed
     myTouch.read();
 if (myTouch.dataAvailable()) {
    xT = myTouch.getX();
    yT = myTouch.getY();        

    // Capture input command from user
    if ((xT>=230) && (xT<=319) && (yT>=200) && (yT<=239)) { // (243, 210, 310, 230)  Exit Button
        xsetup = false; // Exit setupmode   
    } 
    
    else if ((xT>=0) && (xT<=90) && (yT>=200) && (yT<=239)) { // (243, 210, 310, 230)  Save Alarm and Time Button
        savetimealarm = true; // Exit and save time and alarm
        xsetup = false; // Exit setupmode    
      }  
    
    
    else if ((xT>=130) && (xT<=154) && (yT>=12) && (yT<=57)) { // Time Hour +  (132, 35, 152, 55)
        timehour = timehour + 1; // Increment Hour
        if (timehour == 24) {  // reset hour to 0 hours if 24
           timehour = 0 ;
       
      } 
    } 

    else if ((xT>=130) && (xT<=154) && (yT>=78) && (yT<=102)) { // (132, 80, 152, 100); // time hour -
        timehour = timehour - 1; // Increment Hour
        if (timehour == -1) {  // reset hour to 23 hours if < 0
           timehour = 23 ;
       
      } 
    }
    
    else if ((xT>=178) && (xT<=202) && (yT>=12) && (yT<=57)) { // Time Minute +  (180, 35, 200, 55)
        timeminute = timeminute + 1; // Increment Hour
        if (timeminute == 60) {  // reset minute to 0 minutes if 60
           timeminute = 0 ;
        }
      } 

    else if ((xT>=178) && (xT<=202) && (yT>=78) && (yT<=102)) { // (180, 80, 200, 100); // time minute - 
        timeminute = timeminute - 1; // Decrement Hour
        if (timeminute == -1) {  // reset minute to 0 minutes if 60
           timeminute = 59 ;
        }
      }       
 
     else if ((xT>=130) && (xT<=154) && (yT>=113) && (yT<=157)) { // (132, 135, 152, 155); // alarm hour +
        alarmhour = alarmhour + 1; // Increment Hour
        if (alarmhour == 24) {  // reset hour to 0 hours if 24
           alarmhour = 0 ;
       
      } 
    } 

    else if ((xT>=130) && (xT<=154) && (yT>=178) && (yT<=202)) { // (132, 180, 152, 200);  // alarm hour -
        alarmhour = alarmhour - 1; // Decrement Hour
        if (alarmhour == -1) {  // reset hour to 23 hours if < 0
           alarmhour = 23 ;
       
      } 
    }
    
    else if ((xT>=178) && (xT<=202) && (yT>=113) && (yT<=157)) { // (180, 135, 200, 155); // alarm minute +
        alarmminute = alarmminute + 1; // Increment Hour
        if (alarmminute == 60) {  // reset minute to 0 minutes if 60
           alarmminute = 0 ;
        }
      } 

    else if ((xT>=178) && (xT<=202) && (yT>=178) && (yT<=202)) { // (180, 180, 200, 200); // alarm minute -
        alarmminute = alarmminute - 1; // Increment Hour
        if (alarmminute == -1) {  // reset minute to 0 minutes if 60
           alarmminute = 59 ;
        }
      }      


    else if ((xT>=154) && (xT<=184) && (yT>=248) && (yT<=238)) { // toggle Pacman code 
         mspacman = !mspacman; // toggle the value
      } 

    else if ((xT>=216) && (xT<=270) && (yT>=155) && (yT<=179)) { // (218, 157, 268, 177); // alarm set button pushed
        if (alarmstatus == true) {  
             alarmstatus = false; // Turn off Alarm
        } else {
            alarmstatus = true; // Turn on Alarm
        }
      }
     else if ((xT>=46) && (xT<=118) && (yT>=106) && (yT<=130)) { // ((48, 108, 116, 128); // alarm test button pushed
        // Set off alarm by toggling D8, recorded sound triggered by LOW to HIGH transition
        digitalWrite(8,HIGH); // Set high
        digitalWrite(8,LOW); // Set low
     }
       
      // Should mean changes should scroll if held down
        delay(250);      

    }  

// Display MS Pacman or Pacman in menu - push to change
if (mspacman == false) {
    myGLCD.drawBitmap (154, 208, 28, 28, r_o_pacman); //   Closed Ghost 
} else {
    myGLCD.drawBitmap (154, 208, 28, 28, ms_r_o_pacman); //   Closed Ghost  
}

}   




if ( savetimealarm == true) {
  // The following codes transmits the data to the RTC
  Wire.beginTransmission(DS1307);
  Wire.write(byte(0));
  Wire.write(decToBcd(0));
  Wire.write(decToBcd(timeminute));
  Wire.write(decToBcd(timehour));
  Wire.write(decToBcd(0));
  Wire.write(decToBcd(0));
  Wire.write(decToBcd(0));
  Wire.write(decToBcd(0));
  Wire.write(byte(0));
  Wire.endTransmission();
  // Ends transmission of data
  
  // Write the Alarm Time to EEPROM so it can be stored when powered off
 
     //alarmhour = (int)ahour;
     ahour = (byte)alarmhour;
     amin = (byte)alarmminute;
     EEPROM.write(100, ahour);
     EEPROM.write(101, amin);   
    
  // Now time and alarm data saved reset flag
  savetimealarm = false;
}


     //* Clear Screen
      myGLCD.setColor(0, 0, 0); 
      myGLCD.setBackColor(0, 0, 0);
      myGLCD.fillRect(0,239,319,0);
     xsetup = true; // Set Flag now leaving setup mode in order to draw Clock Digits 
     setSyncProvider(RTC.get);   // the function to get the time from the RTC
     setSyncInterval(60); // sync the time every 60 seconds (1 minutes)
     drawscreen(); // Initiate the screen
     UpdateDisp(); // update value to clock
 
 }
 
 // ================= Decimal to BCD converter

byte decToBcd(byte val) {
  return ((val/10*16) + (val%10));
} 
