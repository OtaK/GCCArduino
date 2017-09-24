//This section is for user inputted data to make the code personalized to the particular controller

#define n__notch_x_value  0.0000
#define n__notch_y_value  0.0000

#define e__notch_x_value  0.0000
#define e__notch_y_value  0.0000

#define s__notch_x_value  0.0000
#define s__notch_y_value -0.0000

#define w__notch_x_value -0.0000
#define w__notch_y_value  0.0000

#define sw_notch_x_value -0.0000
#define sw_notch_y_value -0.0000

#define se_notch_x_value  0.0000
#define se_notch_y_value -0.0000

//once you have installed the mod, upload the code and go to live analog inputs display
//hold dpad down for 3 seconds to turn off the mod then record the notch values
//doing the steps in this order ensures the most accurate calibration

//to return to a stock controller hold dpad down for 3 seconds
//to set to dolphin mode hold dpad right for 3 seconds before launching the game

#include "Nintendo.h"
CGamecubeController controller(0); //sets RX0 on arduino to read data from controller
CGamecubeConsole console(1);       //sets TX1 on arduino to write data to console
Gamecube_Report_t gcc;             //structure for controller state
Gamecube_Data_t data;        
float r, deg, nang, eang, sang, wang, seang, swang;
char ax, ay, cx, cy, buf, snp, oldx, nx, ny;
bool shield, tilt, dolphin = 0, off = 0;
byte axm, aym, cxm, cym, cycles = 3;
unsigned long n;
word mode;

void convertinputs(){
  maxvectors();    //snaps sufficiently high cardinal inputs to vectors of 1.0 magnitude of analog stick and c stick 
  perfectangles(); //reduces deadzone of cardinals and gives steepest/shallowest angles when on or near the gate
  shielddrops();   //gives an 8 degree range of shield dropping centered on SW and SE gates
  backdash();      //fixes dashback by imposing a 1 frame buffer upon tilt turn values
  dolphinfix();    //ensures close to 0 values are reported as 0 on the sticks to fix dolphin calibration and fixes poll speed issues
  nocode();        //function to disable all code if dpad down is held for 3 seconds (unplug controller to reactivate)
} //more mods to come!

void maxvectors(){
  if(r>75.0){
    if(arc(nang)<6.0){gcc.xAxis = 128; gcc.yAxis = 255;}
    if(arc(eang)<6.0){gcc.xAxis = 255; gcc.yAxis = 128;}
    if(arc(sang)<6.0){gcc.xAxis = 128; gcc.yAxis =   1;}
    if(arc(wang)<6.0){gcc.xAxis =   1; gcc.yAxis = 128;}
  }
  if(cxm>75&&cym<23){gcc.cxAxis = (cx>0)?255:1; gcc.cyAxis = 128;}
  if(cym>75&&cxm<23){gcc.cyAxis = (cy>0)?255:1; gcc.cxAxis = 128;}
}

void perfectangles(){
  if(r>75.0){
    if(mid(arc(nang),6,19)){gcc.xAxis = (ax>0)?151:105; gcc.yAxis = 204;}
    if(mid(arc(eang),6,19)){gcc.yAxis = (ay>0)?151:105; gcc.xAxis = 204;}
    if(mid(arc(sang),6,19)){gcc.xAxis = (ax>0)?151:105; gcc.yAxis =  52;}
    if(mid(arc(wang),6,19)){gcc.yAxis = (ay>0)?151:105; gcc.xAxis =  52;}
  }
}

void shielddrops(){
  shield = gcc.l||gcc.r||gcc.left>74||gcc.right>74||gcc.z;
  if(shield){
    if(ay<0&&r>72){
      if(arc(swang)<4.0){gcc.yAxis = 73; gcc.xAxis =  73;}
      if(arc(seang)<4.0){gcc.yAxis = 73; gcc.xAxis = 183;}
    }
  }
}

void backdash(){
  if(aym<23){
    if(axm<23)buf = cycles;
    if(buf>0){buf--; if(axm<64) gcc.xAxis = 128+ax*(axm<23);}
  }else buf = 0;
}

void dolphinfix(){
  if(r<8)         {gcc.xAxis  = 128; gcc.yAxis  = 128;}
  if(mag(cx,cy)<8){gcc.cxAxis = 128; gcc.cyAxis = 128;}
  if(gcc.dright&&mode<2000) dolphin = dolphin||(mode++>2000);
  else mode = 0;
  cycles = 3 + (14*dolphin);
}

void nocode(){
  if(gcc.ddown){
    if(n == 0) n = millis();
    off = off||(millis()-n>500);
  }else n = 0;
}

void calibration(){
  ax = gcc.xAxis-128-nx; ay = gcc.yAxis-128-ny; //offsets from nuetral position of analog stick
  cx = gcc.cxAxis - 128; cy = gcc.cyAxis - 128; //offsets from nuetral position of c stick
  axm = abs(ax); aym = abs(ay);                 //magnitude of analog stick offsets
  cxm = abs(cx); cym = abs(cy);                 //magnitude of c stick offsets
  r   = mag(ax, ay); deg = ang(ax, ay);         //obtains polar coordinates
  if(r>1.0){
    gcc.xAxis = 128+r*cos(deg/57.3);
    gcc.yAxis = 128+r*sin(deg/57.3);
  }else{
    gcc.xAxis = 128;
    gcc.yAxis = 128;
  }
}

float ang(float x, float y){return atan(y/x)*57.3+180*(x<0)+360*(y<0&&x>0);} //returns angle in degrees when given x and y components
float mag(char  x, char  y){return sqrt(sq(x)+sq(y));}                       //returns vector magnitude when given x and y components
bool mid(float val, float n1, float n2){return val>n1&&val<n2;}              //returns whether val is between n1 and n2
float arc(float val){return abs(180-abs(abs(deg-val)-180));}                 //returns length of arc between deg and val

void setup(){
  gcc.origin=0; gcc.errlatch=0; gcc.high1=0; gcc.errstat=0;  //init values
  nang  = ang(n__notch_x_value, n__notch_y_value);
  eang  = ang(e__notch_x_value, e__notch_y_value);
  sang  = ang(s__notch_x_value, s__notch_y_value); 
  wang  = ang(w__notch_x_value, w__notch_y_value);
  seang = ang(se_notch_x_value, se_notch_y_value); 
  swang = ang(sw_notch_x_value, sw_notch_y_value);
  controller.read(); gcc = controller.getReport();
  nx = gcc.xAxis-128; ny = gcc.yAxis-128;
}

void loop(){
  controller.read();
  data = defaultGamecubeData;
  gcc = controller.getReport();
  calibration();
  if(!off) convertinputs();                 //implements all the fixes (remove this line to unmod the controller)
  data.report = gcc;
  console.write(data);                      //sends controller data to the console
  controller.setRumble(data.status.rumble); //fixes rumble
}
