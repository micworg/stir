/////////////////////////////////////////////////////////////////////////////// Stir Control (mwx'2019, v1.3.3)
#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>

#define SX Serial.print 
#define SXN Serial.println
#define MS (long)millis()

int SPEEDINC      = 50;                                                                // speed increment (rpm)

int FANMIN        = 200;                                                             // fan minimum speed (rpm)
int FANMAX        = 1600;                                                            // fan maximum speed (rpm)

int CATCHSTOP     = 20000;                                                            // catch stop period (ms)
double BOOSTRAMP  = 60000;                                                    // rise time for boost speed (ms)

int PWM0          = 9;                                                                    // PWM pin for 1. fan
int PWM1          = 10;                                                                   // PWM pin for 2. fan
                                      
int I0            = 2;            // interrupt for fan 0 rpm signal (use 2 for Leonardo/ProMicro and 0 for Uno)
int I1            = 3;            // interrupt for fan 1 rpm signal (use 3 for Leonardo/ProMicro and 1 for Uno)
                                  
int CLK           = 5;                                                                 // clk on KY-040 encoder
int DT            = 6;                                                                  // dt on KY-040 encoder
int SW            = 4;                                                                  // sw on KY-040 encoder

int RINTERVAL     = 5000;                                                           // regulation interval (ms) 
int RDELAY        = 3000;                                                   // regulation delay on changes (ms)
int RTOL          = 8;                                                            // regulation tolerance (rpm)

int SINTERVAL     = 2000;                                                         // speed measurement interval 
int SAVERAGE      = 4;                                                             // speed measurement average 

int SAVETAG       = 1006;                                                                           // save tag 
int SAVEDELAY     = 60000;                                                            // EEPROM save delay (ms)

LiquidCrystal_I2C lcd(0x27,16,2);                                           // LCD display (connect to SDA/SCL)

int i,v0,b0,r0,v1,b1,r1,fanstate0,fanstate1;                                            // speed and regulation
long rpmcount0,rpmcount1,speedcount0,speedcount1;                                                // rpm counter
double r,rx,rpm0,rpm1,xpm0,xpm1,xb0,xb1;                                                               // rpm

long xts,sts,rts,swts,bts,ox,savets,catchts0,catchts1,stop0,stop1,b0ts,b1ts;                          // timing
int bdelay,bprocess,enclast,encval,swmode,mode;                                    // button/encoder processing
int OK,SAVE,LOCK,bstate0,bstate1,btime0,btime1,catch0,catch1,ctime0,ctime1;                  // operation state
char form[8],out[128];String str;                                                              // string buffer
 
 
void setup() { ////////////////////////////////////////////////////////////////////////////////////////// SETUP
  rpmcount0=0;rpmcount1=0;speedcount0=0;speedcount1=0;rpm0=0;rpm1=0;xpm0=0;xpm1=0;bprocess=0;r0=0;r1=0;swmode=2;
  
  Serial.begin(9600);                                                                           // start serial
  
  lcd.init();lcd.clear();lcd.backlight();                                                     // initialize lcd

  pinMode(PWM0,OUTPUT);pinMode(PWM1,OUTPUT);                                                    // set PWM pins
  pinMode(CLK,INPUT);pinMode(DT,INPUT);pinMode(SW,INPUT);                                    // set KY-040 pins

  attachInterrupt(I0,rpmint0,FALLING);                                           // setup interrupts vor rpm in
  attachInterrupt(I1,rpmint1,FALLING);

  TCCR1A=0;TCCR1B=0;TCNT1=0;                                                      // setup timer for 25 kHz PWM
  TCCR1A=_BV(COM1A1)|_BV(COM1B1)|_BV(WGM11);TCCR1B=_BV(WGM13)|_BV(CS10);ICR1=320;

  SAVE=0;                                                                           // load/initialize settings
  if (eer(0)!=SAVETAG) {
    v0=300;v1=300;b0=700;b1=700;btime0=30;btime1=30;catch0=0;catch1=0;ctime0=120;ctime1=120;
    fanstate0=0;fanstate1=0;
    eew(0,SAVETAG);save();
  } else {
    v0=eer(1);v1=eer(2);b0=eer(3);b1=eer(4);btime0=eer(5);btime1=eer(6);
    catch0=eer(7);catch1=eer(8);ctime0=eer(9);ctime1=eer(10);fanstate0=eer(11);fanstate1=eer(12);
  }

  enclast=digitalRead(CLK);                                                                // get encoder state

  xts=MS;sts=MS;rts=MS;swts=MS;bts=MS;savets=MS;catchts0=MS;catchts1=MS;stop0=MS;stop1=MS;         // set timer
  bstate0=0;bstate1=0;mode=0;updatePWM();updatelcd();updatespeed();updatemarker();LOCK=0; // set initial states

}


void loop() { //////////////////////////////////////////////////////////////////////////////////////////// LOOP

  if (SAVE>0 && MS-savets>SAVEDELAY) {;save();SAVE=0;savets=MS;} ////////////////////// save settings if needed
  
  if (catch0 && MS-catchts0>(long)ctime0*60000) { ////////////////////////////////////// check catch fish state
    catchts0=MS;stop0=MS+CATCHSTOP;
    updatePWM();
  }
  if (catch1 && MS-catchts1>(long)ctime1*60000) {
    catchts1=MS;stop1=MS+CATCHSTOP;
    updatePWM();
  }

  if (bstate0 && MS-b0ts>(long)btime0*60000) {;bstate0=0;mode=0;updatelcd();} /////////////// check boost state
  if (bstate1 && MS-b1ts>(long)btime1*60000) {;bstate1=0;mode=0;updatelcd();}

  if (Serial.available() > 0) { ////////////////////////////////////////////////////////// serial communication
    str=Serial.readString();OK=0;

    if (str=="info") {;serinfo();OK=1;}                                                                 // info
    
    if (str.substring(0,5)=="speed") {                                                   // speed <fan0> <fan1>
      v0=sstr(str,':',1).toInt();v1=sstr(str,':',2).toInt();
      updatePWM();r0=0;r1=0;rts=MS+RDELAY;
      SAVE++;OK=1;updatelcd();serinfo();
    }

    if (str.substring(0,6)=="bspeed") {                                                 // bspeed <fan0> <fan1>
      b0=sstr(str,':',1).toInt();b1=sstr(str,':',2).toInt();
      updatePWM();r0=0;r1=0;rts=MS+RDELAY;
      SAVE++;OK=1;updatelcd();updatespeed();serinfo();
    }
    
    if (str.substring(0,2)=="on") {                                         // on <fan0> <fan1> (value: 0 or 1)
      if (sstr(str,':',1).toInt()==1) {;fanstate0=1;r0=0;} else fanstate0=0;
      if (sstr(str,':',2).toInt()==1) {;fanstate1=1;r1=0;} else fanstate1=0;
      updatePWM();rts=MS+RDELAY;
      SAVE++;OK=1;updatelcd();updatespeed();serinfo();
    }

    if (str.substring(0,5)=="boost") {                                   // boost <fan0> <fan1> (value: 0 or 1)
      if (sstr(str,':',1).toInt()==1) {;rts=MS+RDELAY;bstate0=1;b0ts=MS;} else {;bstate0=0;}
      if (sstr(str,':',2).toInt()==1) {;rts=MS+RDELAY;bstate1=1;b1ts=MS;} else {;bstate1=0;}
      SAVE++;OK=1;updatelcd();updatespeed();serinfo();
    }

    if (str.substring(0,5)=="catch") {                                   // catch <fan0> <fan1> (value: 0 or 1)
      if (sstr(str,':',1).toInt()==1) {;catch0=1;catchts0=MS;} else {;catch0=0;}
      if (sstr(str,':',2).toInt()==1) {;catch1=1;catchts1=MS;} else {;catch1=0;}
      SAVE++;OK=1;updatelcd();updatespeed();serinfo();
    }
    
    if (str.substring(0,5)=="btime") {                                     // btime <fan0> <fan1> (value: 0-60)
      btime0=cut(sstr(str,':',1).toInt(),0,60);btime1=cut(sstr(str,':',2).toInt(),0,60);
      SAVE++;OK=1;updatelcd();updatespeed();serinfo();
    }
    
    if (str.substring(0,5)=="ctime") {                                   // ctime <fan0> <fan1> (value: 60-240)
      ctime0=cut(sstr(str,':',1).toInt(),60,240);ctime1=cut(sstr(str,':',2).toInt(),60,240);
      SAVE++;OK=1;updatelcd();updatespeed();serinfo();
    }

    if (!OK) SXN("error");
  }

  if (MS-xts>SINTERVAL) { /////////////////////////////////////////////////////////////////// speed measurement    
    xpm0=xpm0*(SAVERAGE-1)/SAVERAGE+(speedcount0/((MS-xts)/1000.0)*30.0)/SAVERAGE;
    xpm1=xpm1*(SAVERAGE-1)/SAVERAGE+(speedcount1/((MS-xts)/1000.0)*30.0)/SAVERAGE;
    updatespeed();xts=MS;speedcount0=0;speedcount1=0;
  }

  if (MS-rts>RINTERVAL) { //////////////////////////////////////////////////////////////////// speed regulation
    rpm0=rpmcount0/((MS-sts)/1000.0)*30.0;
    rpm1=rpmcount1/((MS-sts)/1000.0)*30.0;
    sts=MS;rpmcount0=0;rpmcount1=0;

    xb0=b0;if (bstate0) xb0=cut((((double)b0-(double)v0)/BOOSTRAMP*(MS-b0ts))+v0,v0,b0);
    xb1=b1;if (bstate1) xb1=cut((((double)b1-(double)v1)/BOOSTRAMP*(MS-b1ts))+v1,v1,b1);

    if (MS>=stop0+20000) {
      if (!fanstate0) r0=0;
      else {
        r=(bstate0?xb0:v0)-rpm0;
        if (abs(r)>RTOL) r0=r<0?r0-1-abs(r)/10:r0+1+abs(r)/10;
      }
    }

    if (MS>=stop1+20000) {
      if (!fanstate1) r1=0;
      else {
        r=(bstate1?xb1:v1)-rpm1;
        if (abs(r)>RTOL) r1=r<0?r1-1-abs(r)/10:r1+1+abs(r)/10;      
      } 
    }
    updatePWM();updatespeed();rts=MS;
  } 

  bdelay=0; //////////////////////////////////////////////////////////////////////////////////// process switch
  if (!bprocess) {
    while (!digitalRead(SW)){
      bdelay++;
      delay(25);
      bprocess=1;
      if (bdelay>20) break;
    }
  }
  
  if (bdelay>0 && MS-bts>100) {                                                            // long button press
    if (bdelay>20) {
      
      if (swmode==0 && mode==0 && !LOCK) {                                                      // fan 0 on/off
        if (fanstate0==0) {;rts=MS+RDELAY;fanstate0=1;r0=0;updatePWM();}
        else {;fanstate0=0;bstate0=0;}
        updatespeed();
      }
      if (swmode==1 && mode==0 && !LOCK) {                                                      // fan 1 on/off
        if (fanstate1==0) {;rts=MS+RDELAY;fanstate1=1;r1=0;updatePWM();}
        else {;fanstate1=0;bstate1=0;}
        updatespeed();
      }

      if (swmode==0 && mode==1 && !LOCK) {                                                // boost fan 0 on/off
        if (bstate0==0) {
          rts=MS+RDELAY;bstate0=1;b0ts=MS;fanstate0=1;
        } else {;bstate0=0;rts=MS+RDELAY+2000;updatePWM();}
        updatespeed();
      }
      if (swmode==1 && mode==1 && !LOCK) {                                                // boost fan 1 on/off
        if (bstate1==0) {
          rts=MS+RDELAY;bstate1=1;b1ts=MS;fanstate1=1;
        } else {;bstate1=0;rts=MS+RDELAY+2000;updatePWM();}
        updatespeed();
      }

      if (swmode==2) {                                                                           // lock/unlock
        if (LOCK==0) LOCK=1;
        else LOCK=0;
        updatemarker();
        save();
      }
      
      bdelay=0;
      
    } else if (bdelay>0 && bdelay<20 && !LOCK) {          // short button press, switch: menu -> fan 0 -> fan 1
      swmode++;if (swmode>2) swmode=0;
      updatemarker();
      bdelay=0;
    }
    
    SAVE++;bts=MS;
  }
  if (digitalRead(SW)) bprocess=0;

  encval = digitalRead(CLK); ////////////////////////////////////////////////////////////////// process encoder
  if (encval != enclast && !LOCK) {
    if(!encval){
            
      if (digitalRead(DT) != encval) {                                                // turn encoder clockwise
        if (swmode==0 && mode==0) v0+=SPEEDINC;  // fan 0 speed up
        if (swmode==1 && mode==0) v1+=SPEEDINC;  // fan 1 speed up
        if (swmode==0 && mode==1) b0+=SPEEDINC;  // boost 0 speed up
        if (swmode==1 && mode==1) b1+=SPEEDINC;  // boost 1 speed up
        if (swmode==0 && mode==2) btime0++;      // boost time 0 up
        if (swmode==1 && mode==2) btime1++;      // boost time 1 up
        if (swmode==0 && mode==3) catch0++;      // catch 0 on/off
        if (swmode==1 && mode==3) catch1++;      // catch 1 on/off
        if (swmode==0 && mode==4) ctime0+=10;    // catch time 0 up
        if (swmode==1 && mode==4) ctime1+=10;    // catch time 1 up
        if (swmode==2) mode++;                   // scroll menu
      } else {                                                                 // turn encoder counterclockwise
        if (swmode==0 && mode==0) v0-=SPEEDINC;  // fan 0 speed down
        if (swmode==1 && mode==0) v1-=SPEEDINC;  // fan 1 speed down
        if (swmode==0 && mode==1) b0-=SPEEDINC;  // boost 0 speed down
        if (swmode==1 && mode==1) b1-=SPEEDINC;  // boost 1 speed down
        if (swmode==0 && mode==2) btime0--;      // boost time 0 down
        if (swmode==1 && mode==2) btime1--;      // boost time 1 down
        if (swmode==0 && mode==3) catch0--;      // catch 0 on/off
        if (swmode==1 && mode==3) catch1--;      // catch 1 on/off
        if (swmode==0 && mode==4) ctime0-=10;    // catch time 0 down
        if (swmode==1 && mode==4) ctime1-=10;    // catch time 1 down
        if (swmode==2) mode--;                   // scroll menu
      }
    
      if (swmode==0 && mode<=1) {;updatePWM();rts=MS+RDELAY;}               // apply (boost) speed change fan 0
      if (swmode==1 && mode<=1) {;updatePWM();rts=MS+RDELAY;}               // apply (boost) speed change fan 1
      
      if (b0<v0) b0=v0;
      
      if (swmode==0 && mode==2) btime0=cut(btime0,0,60);                              // check boost time fan 0
      if (swmode==1 && mode==2) btime1=cut(btime1,0,60);                              // check boost time fan 1
 
      if (swmode==0 && mode==3) {;catch0=cut(catch0,0,1);catchts0=MS;}                     // check catch fan 0
      if (swmode==1 && mode==3) {;catch1=cut(catch1,0,1);catchts1=MS;}                     // check catch fan 1
 
      if (swmode==0 && mode==4) ctime0=cut(ctime0,60,240);                            // check catch time fan 0
      if (swmode==1 && mode==4) ctime1=cut(ctime1,60,240);                            // check catch time fan 1

      if (swmode==2) mode=cut(mode,0,4);                                                     // check menu mode
      
      SAVE++;updatelcd();delay(50);
    } 
  }
  enclast=encval;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////// SUPPORT

void updatelcd() { ///////////////////////////////////////////////////////////////////////////////// update LCD
  if (mode==0) {
    slcd(1,1,5,"SPEED");ilcd(7,1,-4,int(v0));ilcd(12,1,-4,int(v1));
  }
  if (mode==1) {
    slcd(1,1,5,"BOOST");ilcd(7,1,-4,int(b0));ilcd(12,1,-4,int(b1));
  }
  if (mode==2) {
    slcd(1,1,5,"BTIME");ilcd(7,1,-4,int(btime0));ilcd(12,1,-4,int(btime1));
  }
  if (mode==3) {
    slcd(1,1,5,"CATCH");
    if (catch0==0) slcd( 7,1,-4,"OFF"); else slcd( 7,1,-3,"ON");
    if (catch1==0) slcd(12,1,-4,"OFF"); else slcd(12,1,-3,"ON");
  }
  if (mode==4) {
    slcd(1,1,5,"CTIME");ilcd(7,1,-4,int(ctime0));ilcd(12,1,-4,int(ctime1));
  }
}  
  
void updatemarker() { /////////////////////////////////////////////////////////////////// update current marker
  slcd(0,1,1," ");slcd(6,1,1," ");slcd(11,1,1," ");
  if (!LOCK) {
    if (swmode==0) slcd( 6,1,1,">");
    if (swmode==1) slcd(11,1,1,">");
    if (swmode==2) slcd( 0,1,1,">");
  }
}

void updatespeed() { ///////////////////////////////////////////////////////////////////////// update fan speed
  slcd(1,0,5,"     ");
  if (fanstate0) {
    if (MS<stop0) slcd(7,0,-4,"CAT");
    else {
      if (bstate0) ilcd(1,0,2, (((long)btime0*60000)-(MS-(long)b0ts))/1000/60+1);
      else slcd(7,0,1," ");
      ilcd(7,0,-4,round(xpm0));
    }
  } else slcd(7,0,-4,"OFF");

  if (fanstate1) {
    if (MS<stop1) slcd(12,0,-4,"CAT");
    else {
      if (bstate1) ilcd(4,0,2, (((long)btime1*60000)-(MS-(long)b1ts))/1000/60+1);
      else slcd(12,0,1," ");
      ilcd(12,0,-4,round(xpm1));
    }
  } else slcd(12,0,-4,"OFF");
}

void serinfo() { /////////////////////////////////////////////////////////////////////////// serial info output 
  SX(fanstate0);SX(":");SX((int)(v0));SX(":");SX((int)(b0));SX(":");
  SX(fanstate1);SX(":");SX((int)(v1));SX(":");SX((int)(b1));SX(":");
  SX(rpm0);SX(":");SX(rpm1);SX(":");SX(r0);SX(":");SX(r1);SX(":");
  SX(bstate0);SX(":");SX(bstate1);SX(":");SX(btime0);SX(":");SX(btime1);SX(":");
  SX(catch0);SX(":");SX(catch1);SX(":");SX(ctime0);SX(":");SX(ctime1);SXN("");
}

void ilcd(int x,int y, int l,int v) { //////////////////////////////////////////////////// write integer to LCD
  sprintf(form,"%%%dd",l);
  sprintf(out,form,v);
  lcd.setCursor(x,y);lcd.print(out);
}

void slcd(int x,int y, int l,String s) { ////////////////////////////////////////////////// write string to LCD
  sprintf(form,"%%%ds",l);
  sprintf(out,form,s.c_str());
  lcd.setCursor(x,y);lcd.print(out);
}

void updatePWM() { ////////////////////////////////////////////////////////////// update PWM output (fan speed)
  v0=cut(v0,FANMIN,FANMAX);v1=cut(v1,FANMIN,FANMAX);
  b0=cut(b0,FANMIN,FANMAX);b1=cut(b1,FANMIN,FANMAX);

  if (!fanstate0 || MS<stop0) OCR1A=0;
  else OCR1A=cut((bstate0?xb0:v0)/(FANMAX/320.0)+r0,0,320);

  if (!fanstate1 || MS<stop1) OCR1B=0;
  else OCR1B=cut((bstate1?xb1:v1)/(FANMAX/320.0)+r1,0,320); 
} 

void rpmint0() {;rpmcount0++;speedcount0++;} /////////////////////////////////////////////////// rpm interrupts
void rpmint1() {;rpmcount1++;speedcount1++;}

double cut(double v,double min,double max) {;return v>max?max:v<min?min:v>max?max:v;} ///// cut values to limit
int eer(int adr) {;return EEPROM.read(adr*2)+EEPROM.read(adr*2+1)*256;} /////////////////////////// read EEPROM
void eew(int adr, int val) {;EEPROM.write(adr*2,val%256);EEPROM.write(adr*2+1,val/256);} /////// save to EEPROM

void save() { /////////////////////////////////////////////////////////////////////////////////// save settings
  eew(1,v0);eew(2,v1);eew(3,b0);eew(4,b1);eew(5,btime0);eew(6,btime1);
  eew(7,catch0);eew(8,catch1);eew(9,ctime0);eew(10,ctime1);eew(11,fanstate0);eew(12,fanstate1);
}

String sstr(String data, char sep, int idx) { ///////////////////////////////////////// get saperated substring
  int found=0;
  int si[]={0,-1};
  int mi=data.length()-1;

  for (int i=0; i<=mi && found<=idx; i++) {
    if (data.charAt(i) == sep || i == mi) {
      found++;si[0]=si[1]+1;si[1]=(i==mi) ? i+1 : i;
    }
  }
  return found>idx?data.substring(si[0],si[1]):"";
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////// END