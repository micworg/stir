//////////////////////////////////////////////////////////////////////////////////// Stir Control V2 (mwx'2019)
#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>

#define MS (long)millis()
#define NP 19                                                                 

String VERSION    = "2.1.0";

int FANINIT       = 0;                                                // initialize fan with high voltage (0/1)

int SPEEDINC1     = 20;                                                               // speed increment values
int SPEEDINC2     = 50;
int SPEEDINC3     = 100;

int SPEEDSTEP1    = 400;                           // speed increment steps (increment changes at these values)
int SPEEDSTEP2    = 1000;

int RNDINC        = 50;                                                         // random value increment (rpm)

int FANMIN        = 200;            // fan minimum speed (should be a value at which the fan runs safely) (rpm)
int FANMAX        = 2500;              // fan maximum speed (should be the real maximum value of the fan) (rpm)

int BOFF          = 0;                             // if set boost off will also turn the normal mode off (0/1)

int BINC          = 1;                                                                 // BTIME increment (min)
int CINC          = 10;                                                                // CTIME increment (min)
int RINC          = 10;                                                                // RTIME increment (sec)
int OINC          = 3;                                                                // OTIME increment (hour)

int CATCHSTOP     = 20;                                                         // fish catch stop period (sec)

int PWM0          = 9;                                                                    // PWM pin for 1. fan
int PWM1          = 10;                                                                   // PWM pin for 2. fan
int PWM2          = 11;                                                           // PWM pin for LCD brightness  
  
int I0            = 0;            // interrupt for fan 0 rpm signal (use 2 for Leonardo/ProMicro and 0 for Uno)
int I1            = 1;            // interrupt for fan 1 rpm signal (use 3 for Leonardo/ProMicro and 1 for Uno)

int SWAPENC       = 1;
                                  
int CLK           = 5;                           // clk on KY-040 encoder (swap clk and dt to invert direction)
int DT            = 6;                                                                  // dt on KY-040 encoder
int SW            = 4;                                                                  // sw on KY-040 encoder

int R0            = 7;                                                               // voltage select relais 0
int R1            = 8;                                                               // voltage select relais 1
int RTHRES        = 700;                                                      // voltage switch threshold (rpm)
                                      
long RINTERVAL    = 5000;                                                          // regulation internval (ms) 
long RDELAY       = 3000;                                                   // regulation delay on changes (ms)
int RTOL          = 8;                                                            // regulation tolerance (rpm)

int RNDINTERVAL   = 300;                                            // randon value range change interval (sec)

long SINTERVAL    = 2000;                                                   // speed measurement internval (ms) 
int SAVERAGE      = 4;                                                             // speed measurement average 

int SAVETAG       = 2015;                                                                           // save tag 
long SAVEDELAY    = 10000;                                                            // EEPROM save delay (ms)

int LGHT          = 5;                                                                        // LCD brightness 

byte LCDB[]       = {4,8,16,24,32,64,96,128,192,255};       // LCD brightness steps (10 values, 0=off, 255=max)

byte aright[]     = {0x00,0x08,0x0C,0x0E,0x0C,0x08,0x00,0x00};                                 // LCD character
byte aup[]        = {0x04,0x0E,0x1F,0x00,0x00,0x00,0x00,0x00};
byte arnd[]       = {0x0E,0x0E,0x0E,0x00,0x00,0x00,0x00,0x00};

LiquidCrystal_I2C lcd(0x27,16,2);                                           // LCD display (connect to SDA/SCL)

int v[2],b[2],r[2]={0};double q,rpm[2]={0},xpm[2]={0},xb[2]={0},xv[2]={0},rtime[2];     // speed and regulation
long ac[2]={0},bc[2]={0};                                                              // interrupt rpm counter
long xts,sts,rts,swts,buts,savets,catts[2],stop[2],bts[2],vts[2],ots[2],rndts[2];                     // timing
int bdelay,bprocess=0,enclast,encval,M=2,S=0,C=0;                                  // button/encoder processing
int F[2],bstate[2]={0},btime[2],cat[2],ctime[2],cstate[2]={0},bclr=0,SAVE=0,LOCK=0;         // operating states
int ostate[2]={0},otime[2]={0};                                                                    // off timer
int rnd[2]={0},rnval[2]={0};long seed;                                                          // random speed
char form[8],out[20];                                                                          // string buffer
String CF[NP];int *P[NP],Cinc[NP],Cmin[NP],Cmax[NP];                                      // configuration menu
int clkorg,dtorg;                                                                          // encoder direction

void setup() { ////////////////////////////////////////////////////////////////////////////////////////// SETUP  
  int i;
                
  i=0;  P[i]=&LGHT;        CF[i]="LGHT";  defcon(i,  1,   0,    9 );                // configuration menu setup
  i=1;  P[i]=&SPEEDINC1;   CF[i]="SPI1";  defcon(i, 10,  10,  500 );
  i=2;  P[i]=&SPEEDINC2;   CF[i]="SPI2";  defcon(i, 10,  10,  500 );
  i=3;  P[i]=&SPEEDINC3;   CF[i]="SPI3";  defcon(i, 10,  10,  500 );
  i=4;  P[i]=&SPEEDSTEP1;  CF[i]="SPS1";  defcon(i, 50, 100, 2000 );
  i=5;  P[i]=&SPEEDSTEP2;  CF[i]="SPS2";  defcon(i, 50, 100, 2000 );
  i=6;  P[i]=&SWAPENC;     CF[i]="SWEN";  defcon(i,  1,   0,    1 );
  i=7;  P[i]=&RNDINC;      CF[i]="RINC";  defcon(i,  1,   0,    9 );
  i=8;  P[i]=&CATCHSTOP;   CF[i]="CSTP";  defcon(i, 10,   0, 1000 );
  i=9;  P[i]=&FANMIN;      CF[i]="FMIN";  defcon(i, 50, 100, 1000 );
  i=10; P[i]=&FANMAX;      CF[i]="FMAX";  defcon(i, 50, 500, 4000 );
  i=11; P[i]=&FANINIT;     CF[i]="FINI";  defcon(i,  1,   0,    1 );
  i=12; P[i]=&RTHRES;      CF[i]="RTHR";  defcon(i, 50,   0, 1500 );
  i=13; P[i]=&RNDINTERVAL; CF[i]="RINT";  defcon(i,  1,   0,   60 );
  i=14; P[i]=&BOFF;        CF[i]="BOFF";  defcon(i,  1,   0,    1 );
  i=15; P[i]=&BINC;        CF[i]="BINC";  defcon(i,  1,   1,   60 );
  i=16; P[i]=&CINC;        CF[i]="CINC";  defcon(i,  1,   1,   60 );
  i=17; P[i]=&RINC;        CF[i]="RINC";  defcon(i,  1,   1,   60 );
  i=18; P[i]=&OINC;        CF[i]="OINC";  defcon(i,  1,   1,   60 );

  lcd.init();lcd.clear();lcd.backlight();                                                     // initialize lcd
  lcd.createChar(0,aright);lcd.createChar(1,aup);lcd.createChar(2,arnd);                 // load lcd characters
  pinMode(PWM0,OUTPUT);pinMode(PWM1,OUTPUT);pinMode(PWM2,OUTPUT);                             // setup PWM pins
  pinMode(CLK,INPUT);pinMode(DT,INPUT);pinMode(SW,INPUT);                                  // setup KY-040 pins
  digitalWrite(CLK,true);digitalWrite(DT,true);digitalWrite(SW,true);               // turn ON pullup resistors
  pinMode(R0,OUTPUT);pinMode(R1,OUTPUT);                                    // setup voltage selcet relais pins

  attachInterrupt(I0,rpmint0,FALLING);                                           // setup interrupts vor rpm in
  attachInterrupt(I1,rpmint1,FALLING);

  TCCR1A=0;TCCR1B=0;TCNT1=0;                                                      // setup timer for 25 kHz PWM
  TCCR1A=_BV(COM1A1)|_BV(COM1B1)|_BV(WGM11);TCCR1B=_BV(WGM13)|_BV(CS10);ICR1=320;

  SAVE=0;                                                                           // load/initialize settings
  if (eer(0)!=SAVETAG) {
    for (i=0;i<2;i++) {;v[i]=300;b[i]=700;btime[i]=30;cat[i]=0;ctime[i]=120;rtime[i]=60;F[i]=0;rnval[i]=0;}
    seed=0;
    eew(0,SAVETAG);save();
  } else {
    for (i=0;i<2;i++) {
      v[i]=eer(1+i);b[i]=eer(3+i);btime[i]=eer(5+i);cat[i]=eer(7+i);
      ctime[i]=eer(9+i);F[i]=eer(11+i);rtime[i]=eer(13+i);rnval[i]=eer(15+i);
    }
    seed=eer(100)+1;eew(100,seed);randomSeed(seed);
    for (i=0;i<NP;i++) *P[i]=eer(200+i);
  }

  clkorg=CLK;dtorg=DT;

  if (SWAPENC) {;DT=clkorg;CLK=dtorg;}
  
  lset();

  enclast=digitalRead(CLK);                                                                // get encoder state

  for (i=0;i<2;i++) catts[i]=stop[i]=vts[i]=ots[i]=rndts[i]=MS;xts=sts=rts=swts=buts=savets=MS;    // timer
  updatePWM();updatelcd();updatespeed();updatemarker();                                           // update all

  slcd(0,0,5,VERSION);                                                                          // show version
  
  if (FANINIT) {                                                                                  // setup fans
    digitalWrite(R0,LOW);digitalWrite(R1,LOW);delay(3000);digitalWrite(R0,HIGH);digitalWrite(R1,HIGH);
  }
}

void loop() { //////////////////////////////////////////////////////////////////////////////////////////// LOOP

  if (SAVE>0 && MS-savets>SAVEDELAY) {;save();SAVE=0;savets=MS;} ////////////////////// save settings if needed
  if (MS-savets>SAVEDELAY/5) bclr=1;
  
  for (int i=0;i<2;i++) {

    if (MS-rndts[i]>((long)RNDINTERVAL*1000)) { ////////////////////////////////////////////////// random timer
      rnd[i]=int(random(0,rnval[i]+1)/10)*10;rndts[i]=MS;updatelcd();
    }

    if (ostate[i] && MS-ots[i]>(long)otime[i]*3600000) { ////////////////////////////////////// check off timer
      otime[i]=0;ostate[i]=0;fset(i,0);
    }
    
    if (cat[i] && MS-catts[i]>(long)ctime[i]*60000 && F[i]==1) { //////////////////////////// initiate cat fish
      catts[i]=MS;cstate[i]=1;F[i]=0;stop[i]=MS;updatePWM();updatespeed();
    }
    
    if (cstate[i] && MS-stop[i]>((long)CATCHSTOP*1000) && F[i]==0) { ///////////// stop cat fish and start over
      cstate[i]=0;F[i]=1;vts[i]=MS;rts=MS+2000;updatePWM();updatespeed();
    }

    if (bstate[i] && MS-bts[i]>(long)btime[i]*60000) {;bstate[i]=0;S=0;updatelcd();} //////// check boost state

  }

  if (MS-xts>SINTERVAL) { /////////////////////////////////////////////////////////////////// speed measurement    
    for (int i=0;i<2;i++) xpm[i]=xpm[i]*(SAVERAGE-1)/SAVERAGE+(bc[i]/((MS-xts)/1000.0)*30.0)/SAVERAGE;
    updatespeed();xts=MS;bc[0]=0;bc[1]=0;
  }

  if (MS-rts>RINTERVAL) { //////////////////////////////////////////////////////////////////// speed regulation
    for (int i=0;i<2;i++) {
      rpm[i]=ac[i]/((MS-sts)/1000.0)*30.0;
      ac[i]=0;
      calcramp(i);
      if (!F[i]) {;r[i]=0;setPWM(i,0);}
      else {
        q=(bstate[i]?xb[i]:xv[i])-rpm[i];
        if (abs(q)>RTOL) r[i]=q<0?r[i]-1-abs(q)/10:r[i]+1+abs(q)/10;
        setPWM(i,cut((bstate[i]?xb[i]:xv[i])/(FANMAX/320.0)+r[i],0,320));
      }
    }

    sts=MS;rts=MS;
    updatespeed();
  } 

  bdelay=0; //////////////////////////////////////////////////////////////////////////////////// process switch
  if (!bprocess) {
    while (!digitalRead(SW)){
      bdelay++;delay(15);bprocess=1;if (bdelay>20) break;
    }
  }
  
  if (bdelay>0 && MS-buts>100) {                                                           // long button press
    if (bdelay>20) {
      
      if (M==2) {                                                                                // lock/unlock
        if (LOCK==0) LOCK=1;
        else LOCK=0;
        updatemarker();
      } else {
        if (S==0 && !LOCK) {                                                                      // fan on/off
          if (F[M]==0) fset(M,1); else fset(M,0);
        }
        if (S==1 && !LOCK) {                                                                    // boost on/off
          if (bstate[M]==0) bset(M,1); else {;bset(M,0);if (BOFF) fset(M,0);}
          bclr=1;
        }
        if (S==8 && !LOCK) {                                                                // exit config mode
          M=2;save();updatemarker(); 
          if (SWAPENC) {;DT=clkorg;CLK=dtorg;}
          else {;DT=dtorg;CLK=clkorg;}
        }
      }
      
      save();updatespeed();bdelay=0;
      
    } else if (bdelay>0 && bdelay<20 && !LOCK) {          // short button press, switch: menu -> fan 0 -> fan 1
      M++;
      if (S==8 && M==2) M=0;
      if (M>2) M=0;
      updatemarker();
      bdelay=0;
    }
    
    SAVE++;buts=MS;
  }
  if (digitalRead(SW)) bprocess=0;

  encval = digitalRead(CLK); ////////////////////////////////////////////////////////////////// process encoder
  if (encval != enclast && !LOCK) {
    if(!encval){

      if (digitalRead(DT) != encval) {                                                // turn encoder clockwise
        if (M==2) S++;                                                                           // scroll menu
        else {
          if (S==0) v[M]=speedinc(v[M],1);  // fan speed up
          if (S==1) b[M]=speedinc(b[M],1);  // boost speed up
          if (S==2) btime[M]+=BINC;         // boost time up
          if (S==3) cat[M]++;               // cat on/off
          if (S==4) ctime[M]+=CINC;         // cat time up
          if (S==5) rtime[M]+=RINC;         // rise time up
          if (S==6) otime[M]+=OINC;         // off time up
          if (S==7) rnval[M]+=RNDINC;       // random value
          if (S==8 && M==0) C++;            // increase config menu
          if (S==8 && M==1) *P[C] = cut(*P[C] + Cinc[C],Cmin[C],Cmax[C]);
        }
      } else {                                                                 // turn encoder counterclockwise
        if (M==2) S--;                                                                           // scroll menu
        else {
          if (S==0) v[M]=speedinc(v[M],0);  // fan speed down
          if (S==1) b[M]=speedinc(b[M],0);  // boost speed down
          if (S==2) btime[M]-=BINC;         // boost time down
          if (S==3) cat[M]--;               // cat on/off
          if (S==4) ctime[M]-=CINC;         // cat time down
          if (S==5) rtime[M]-=RINC;         // rise time down
          if (S==6) otime[M]-=OINC;         // off time down
          if (S==7) rnval[M]-=RNDINC;       // random value
          if (S==8 && M==0) C--;            // decrease config menu
          if (S==8 && M==1) *P[C] = cut(*P[C] - Cinc[C],Cmin[C],Cmax[C]);
        }
      }
      
      for (int i=0;i<2;i++) if (b[i]<v[i]) b[i]=v[i];

      if (M!=2) {
        if (S<=1) {;updatePWM();rts=MS+RDELAY;}                                   // apply (boost) speed change
        if (S==2) btime[M]=cut(btime[M],0,99);                                              // check boost time
        if (S==3) {;cat[M]=cut(cat[M],0,1);catts[M]=MS;}                                           // check cat
        if (S==4) ctime[M]=cut(ctime[M],60,240);                                              // check cat time
        if (S==5) rtime[M]=cut(rtime[M],0,240);                                              // check rise time
        if (S==6) oset(M,otime[M]);                                                           // check off time
        if (S==7) {;rnval[M]=cut(rnval[M],0,1000);updatePWM();}                    // apply random value change
        if (S==8 && C==0) lset();                                                         // set LCD brightness
      }
      
      C=cut(C,0,18);                                                                          // check cfg mode

      if (M==2) S=cut(S,0,8);                                                                // check menu mode
      
      SAVE++;updatelcd();delay(25);
    } 
  }
  enclast=encval;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////// SUPPORT

void defcon(int n, int inc, int min, int max) { ///////////////////////////////////////////// set config limits
  Cinc[n]=inc; Cmin[n]=min; Cmax[n]=max;
}

int speedinc(int s,int mode) { ////////////////////////////////////////////////////// calculate speed increment
  if (mode==0) {
    if (s<=SPEEDSTEP1) return s-SPEEDINC1;
    if (s<=SPEEDSTEP2) return s-SPEEDINC2;
    return s-SPEEDINC3;
  }
  if (mode==1) {
    if (s<SPEEDSTEP1) return s+SPEEDINC1;
    if (s<SPEEDSTEP2) return s+SPEEDINC2;
    return s+SPEEDINC3;
  }
  return s;
}

void lset() { ////////////////////////////////////////////////////////////////////////////// set LCD brightness
  LGHT=cut(LGHT,0,9);
  analogWrite(PWM2,LCDB[LGHT]);  
}

void oset(int n,int t) { ///////////////////////////////////////////////////////////////// set switch off timer
  otime[n]=cut(t,0,99);ots[n]=MS;ostate[n]=otime[n]?1:0;
  SAVE++;
}

void fset(int n,int s) { /////////////////////////////////////////////////////////////// set fan state (on/off)
  if (s==1) {
    rts=MS+RDELAY;F[n]=1;catts[n]=vts[n]=MS;r[n]=0;
  } else {
    F[n]=0;bstate[n]=0;
  }
  updatePWM();
  SAVE++;
}

void bset(int n,int s) { ///////////////////////////////////////////////////////////// set boost state (on/off)
  if (s==1) {
    rts=MS+RDELAY;bstate[n]=1;bts[n]=MS;F[n]=1;vts[n]=MS; 
  } else {
    bstate[n]=0;rts=MS+RDELAY;  
  }
  updatePWM();
  SAVE++;
}

void updatelcd() { ///////////////////////////////////////////////////////////////////////////////// update LCD
  
  for (int i=0;i<2;i++) {
    if (S==0) {;slcd(1,1,5,"SPEED");ilcd(7+i*5,1,-4,v[i]);}
    if (S==1) {;slcd(1,1,5,"BOOST");ilcd(7+i*5,1,-4,int(b[i]));}
    if (S==2) {;slcd(1,1,5,"BTIME");ilcd(7+i*5,1,-4,int(btime[i]));}
    if (S==3) {
      slcd(1,1,5,"CATCH");
      if (cat[i]==0) slcd( 7+i*5,1,-4,"OFF"); else slcd( 7+i*5,1,-3,"ON");
    }
    if (S==4) {;slcd(1,1,5,"CTIME");ilcd(7+i*5,1,-4,int(ctime[i]));}
    if (S==5) {;slcd(1,1,5,"RTIME");ilcd(7+i*5,1,-4,int(rtime[i]));}
    if (S==6) {;slcd(1,1,5,"OTIME");ilcd(7+i*5,1,-4,int(otime[i]));}
    if (S==7) {;slcd(1,1,5,"RNVAL");ilcd(7+i*5,1,-4,int(rnval[i]));}
  }
  if (S==8) {;slcd(1,1,-5,"CFG");slcd(7,1,-4,CF[C]);ilcd(7+5,1,-4,*P[C]);}
}  

void updatemarker() { ////////////////////////////////////////////////////////////////////// update menu marker
  slcd(0,1,1," ");slcd(6,1,1," ");slcd(11,1,1," ");
  int m[]={6,11,0};if (!LOCK) clcd(m[M],1,0);
}

void updatespeed() { ///////////////////////////////////////////////////////////////////////// update fan speed
  if (bclr) slcd(0,0,6,"      ");
  for (int i=0;i<2;i++) {
    slcd(6+i*5,0,1," ");
    if (cstate[i]) slcd(7+i*5,0,-4,"CAT");
    else {
      if (F[i]) {
        if (bstate[i]) ilcd(1+i*3,0,-2, (((long)btime[i]*60000)-(MS-(long)bts[i]))/1000/60+1);
        else if (ostate[i]) ilcd(1+i*3,0,-2, (((long)otime[i]*3600000)-(MS-(long)ots[i]))/1000/60/60+1);
        ilcd(7+i*5,0,-4,round(xpm[i]));
        if ((!bstate[i] && xv[i]<v[i]) || (bstate[i] && xb[i]<b[i])) clcd(6+i*5,0,1);
        else {
          if (xv[i]>=v[i] && rnval[i]>0 && !bstate[i]) clcd(6+i*5,0,2);
        }
      } else slcd(7+i*5,0,-4,"OFF");
    }
  }
}

void clcd(int x,int y, char v) { /////////////////////////////////////////////////////// write character to LCD
  lcd.setCursor(x,y);lcd.write(v);
}

void ilcd(int x,int y, int l,int v) { //////////////////////////////////////////////////// write integer to LCD
  sprintf(form,"%%%dd",l);sprintf(out,form,v);lcd.setCursor(x,y);lcd.print(out);
}

void slcd(int x,int y, int l,String s) { ////////////////////////////////////////////////// write string to LCD
  sprintf(form,"%%%ds",l);sprintf(out,form,s.c_str());lcd.setCursor(x,y);lcd.print(out);
}

void calcramp(int i) { ///////////////////////////////////////////////////////////// calculate speed ramp value
  xb[i]=b[i];
  int rt=rtime[i]?rtime[i]:1;
  if (bstate[i]) xb[i]=cut(((b[i]-v[i])/(rt*1000.0)*(MS-bts[i]))+v[i],v[i],b[i]);
  xv[i]=v[i];
  xv[i]=cut(((v[i]-FANMIN)/(rt*1000.0)*(MS-vts[i]))+FANMIN,FANMIN,v[i]);
  if (rnval[i]>0 && xv[i]>=v[i]) xv[i]=cut(xv[i]+rnd[i],FANMIN,FANMAX);  
}

void setPWM(int n,int v) { /////////////////////////////////////////// set value to OCR1x and states to off pin
  if (n==0) OCR1A=v;
  if (n==1) OCR1B=v;
}

void updatePWM() { ////////////////////////////////////////////////////////////// update PWM output (fan speed)
  for (int i=0;i<2;i++) {
    v[i]=cut(v[i],FANMIN,FANMAX);
    b[i]=cut(b[i],FANMIN,FANMAX);
    calcramp(i);
    if (!F[i]) setPWM(i,0); else setPWM(i,cut((bstate[i]?xb[i]:xv[i])/(FANMAX/320.0)+r[i],1,320));
  }
  
  if (v[0]>=RTHRES) digitalWrite(R0,LOW);
  else digitalWrite(R0,HIGH);
  
  if (v[1]>=RTHRES) digitalWrite(R1,LOW);
  else digitalWrite(R1,HIGH);
} 

void rpmint0() {;ac[0]++;bc[0]++;} ///////////////////////////////////////////////////////////// rpm interrupts
void rpmint1() {;ac[1]++;bc[1]++;}

double cut(double v,double min,double max) {;return v>max?max:v<min?min:v>max?max:v;} ///// cut values to limit

int eer(int adr) {;return EEPROM.read(adr*2)+EEPROM.read(adr*2+1)*256;} /////////////////////////// read EEPROM

void eew(int adr, int val) {;EEPROM.write(adr*2,val%256);EEPROM.write(adr*2+1,val/256);} /////// save to EEPROM

void save() { /////////////////////////////////////////////////////////////////////////////////// save settings
  for (int i=0;i<2;i++) {
    eew(1+i,v[i]);eew(3+i,b[i]);eew(5+i,btime[i]);eew(7+i,cat[i]);
    eew(9+i,ctime[i]);eew(11+i,F[i]);eew(13+i,rtime[i]);eew(15+i,rnval[i]);
  }
  eew(100,seed);
  for (int i=0;i<NP;i++) eew(200+i,*P[i]);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////// END
