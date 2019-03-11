/////////////////////////////////////////////////////////////////////////////////////// Stir Control (mwx'2019)
#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>

#define MS (long)millis()

#define SX Serial.print 
#define SXN Serial.println

String VERSION    = "1.5.2";

int SPEEDINC      = 50;                                                                // speed increment (rpm)

int FANMIN        = 200;            // fan minimum speed (should be a value at which the fan runs safely) (rpm)
int FANMAX        = 1600;              // fan maximum speed (should be the real maximum value of the fan) (rpm)

long CATCHSTOP    = 20000;                                                       // fish catch stop period (ms)

int PWM0          = 9;                                                                    // PWM pin for 1. fan
int PWM1          = 10;                                                                   // PWM pin for 2. fan
                                      
int I0            = 2;            // interrupt for fan 0 rpm signal (use 2 for Leonardo/ProMicro and 0 for Uno)
int I1            = 3;            // interrupt for fan 1 rpm signal (use 3 for Leonardo/ProMicro and 1 for Uno)
                                  
int CLK           = 5;                                                                 // clk on KY-040 encoder
int DT            = 6;                                                                  // dt on KY-040 encoder
int SW            = 4;                                                                  // sw on KY-040 encoder

long RINTERVAL    = 5000;                                                           // regulation interval (ms) 
long RDELAY       = 3000;                                                   // regulation delay on changes (ms)
int RTOL          = 8;                                                            // regulation tolerance (rpm)

long SINTERVAL    = 2000;                                                         // speed measurement interval 
int SAVERAGE      = 4;                                                             // speed measurement average 

int SAVETAG       = 1011;                                                                           // save tag 
long SAVEDELAY    = 60000;                                                            // EEPROM save delay (ms)

byte aright[]     = {0x00,0x08,0x0C,0x0E,0x0C,0x08,0x00,0x00};                                 // LCD character
byte aup[]        = {0x04,0x0E,0x1F,0x00,0x00,0x00,0x00,0x00};

LiquidCrystal_I2C lcd(0x27,16,2);                                           // LCD display (connect to SDA/SCL)

int v[2],b[2],r[2]={0};double q,rpm[2]={0},xpm[2]={0},xb[2]={0},xv[2]={0},rtime[2];     // speed and regulation
long ac[2]={0},bc[2]={0};                                                              // interrupt rpm counter
long xts,sts,rts,swts,buts,savets,catts[2],stop[2],bts[2],vts[2],ots[2];                              // timing
int bdelay,bprocess=0,enclast,encval,M=2,S=0;                                      // button/encoder processing
int F[2],bstate[2]={0},btime[2],cat[2],ctime[2],cstate[2]={0},bclr=0,SAVE=0,LOCK=0;         // operating states
int ostate[2]={0},otime[2]={0};                                                                    // off timer
char form[8],out[20];String cmd[8];int icmd[8];                                                // string buffer


void setup() { ////////////////////////////////////////////////////////////////////////////////////////// SETUP  
  Serial.begin(9600);                                                                           // start serial
  
  lcd.init();lcd.clear();lcd.backlight();lcd.createChar(0,aright);lcd.createChar(1,aup);      // initialize lcd
  pinMode(PWM0,OUTPUT);pinMode(PWM1,OUTPUT);                                                    // set PWM pins
  pinMode(CLK,INPUT);pinMode(DT,INPUT);pinMode(SW,INPUT);                                    // set KY-040 pins
  digitalWrite(CLK,true);digitalWrite(DT,true);digitalWrite(SW,true);               // turn ON pullup resistors

  attachInterrupt(I0,rpmint0,FALLING);                                           // setup interrupts vor rpm in
  attachInterrupt(I1,rpmint1,FALLING);

  TCCR1A=0;TCCR1B=0;TCNT1=0;                                                      // setup timer for 25 kHz PWM
  TCCR1A=_BV(COM1A1)|_BV(COM1B1)|_BV(WGM11);TCCR1B=_BV(WGM13)|_BV(CS10);ICR1=320;

  SAVE=0;                                                                           // load/initialize settings
  if (eer(0)!=SAVETAG) {
    for (int i=0;i<2;i++) {;v[i]=300;b[i]=700;btime[i]=30;cat[i]=0;ctime[i]=120;rtime[i]=60;F[i]=0;}
    eew(0,SAVETAG);save();
  } else {
    for (int i=0;i<2;i++) {
      v[i]=eer(1+i);b[i]=eer(3+i);btime[i]=eer(5+i);cat[i]=eer(7+i);
      ctime[i]=eer(9+i);F[i]=eer(11+i);rtime[i]=eer(13+i);
    }
  }

  enclast=digitalRead(CLK);                                                                // get encoder state

  for (int i=0;i<2;i++) catts[i]=stop[i]=vts[i]=ots[i]=MS;xts=sts=rts=swts=buts=savets=MS;             // timer
  updatePWM();updatelcd();updatespeed();updatemarker();                                           // update all

  slcd(0,0,5,VERSION);                                                                          // show version
}


void loop() { //////////////////////////////////////////////////////////////////////////////////////////// LOOP

  if (Serial.available() > 0) { ////////////////////////////////////////////////////////// serial communication
    int n=cutcmd(Serial.readString());
    int err=1;
    int i=cut(icmd[1],0,1);
    if (cmd[0]=="info"    && n==1) {;err=0;}
    if (cmd[0]=="version" && n==1) {;SXN(VERSION);return;}
    if (cmd[0]=="save"    && n==1) {;save();}
    if (cmd[0]=="speed"   && n==3) {;v[i]=cut(icmd[2],FANMIN,FANMAX);err=0;}
    if (cmd[0]=="bspeed"  && n==3) {;b[i]=cut(icmd[2],FANMIN,FANMAX);err=0;}
    if (cmd[0]=="btime"   && n==3) {;btime[i]=cut(icmd[2],0,60);err=0;}
    if (cmd[0]=="ctime"   && n==3) {;ctime[i]=cut(icmd[2],60,240);err=0;}
    if (cmd[0]=="rtime"   && n==3) {;rtime[i]=cut(icmd[2],0,240);err=0;}
    if (cmd[0]=="on"      && n==2) {;fset(i,1);err=0;}
    if (cmd[0]=="off"     && n==2) {;fset(i,0);err=0;}
    if (cmd[0]=="bon"     && n==2) {;bset(i,1);err=0;}
    if (cmd[0]=="boff"    && n==2) {;bset(i,0);err=0;}
    if (cmd[0]=="con"     && n==2) {;cat[i]=1;catts[i]=MS;err=0;}
    if (cmd[0]=="coff"    && n==2) {;cat[i]=0;catts[i]=MS;err=0;}
    if (cmd[0]=="otime"   && n==3) {;oset(i,icmd[2]);err=0;}
    
    updatelcd();
    for (int i=0;i<2;i++) {
      SX(F[i]);SX(":");SX((int)(v[i]));SX(":");SX((int)(b[i]));SX(":");SX(rpm[i]);SX(":"); 
      SX(xpm[i]);SX(":");SX(r[i]);SX(":");SX(bstate[i]);SX(":");SX(btime[i]);SX(":"); 
      SX(cat[i]);SX(":");SX(ctime[i]);SX(":");SX(rtime[i]);SX(":");SX(otime[i]);SX(":");
      if (bstate[i]) SX((((long)btime[i]*60000)-(MS-(long)bts[i]))/1000+1); else SX(0);SX(":");
      if (ostate[i]) SX((((long)otime[i]*3600000)-(MS-(long)ots[i]))/1000+1); else SX(0);SX(":");
    }
    SX(VERSION);SX(":");SXN(err);
  }

  if (SAVE>0 && MS-savets>SAVEDELAY) {;save();SAVE=0;savets=MS;} ////////////////////// save settings if needed
  if (MS-savets>SAVEDELAY/5) bclr=1;
  
  for (int i=0;i<2;i++) {
    
    if (ostate[i] && MS-ots[i]>(long)otime[i]*3600000) { ////////////////////////////////////// check off timer
      otime[i]=0;ostate[i]=0;fset(i,0);
    }
    
    if (cat[i] && MS-catts[i]>(long)ctime[i]*60000 && F[i]==1) { //////////////////////////// initiate cat fish
      catts[i]=MS;cstate[i]=1;F[i]=0;stop[i]=MS;updatePWM();updatespeed();
    }
    
    if (cstate[i] && MS-stop[i]>CATCHSTOP && F[i]==0) { ////////////////////////// stop cat fish and start over
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
      bdelay++;delay(25);bprocess=1;if (bdelay>20) break;
    }
  }
  
  if (bdelay>0 && MS-buts>100) {                                                           // long button press
    if (bdelay>20) {
      
      if (M==2) {                                                                                // lock/unlock
        if (LOCK==0) LOCK=1;
        else LOCK=0;
        updatemarker();save();
      } else {
        if (S==0 && !LOCK) {                                                                      // fan on/off
          if (F[M]==0) fset(M,1); else fset(M,0);
        }
        if (S==1 && !LOCK) {                                                                    // boost on/off
          if (bstate[M]==0) bset(M,1); else bset(M,0);
          bclr=1;
        }
      }
      
      updatespeed();bdelay=0;
      
    } else if (bdelay>0 && bdelay<20 && !LOCK) {          // short button press, switch: menu -> fan 0 -> fan 1
      M++;if (M>2) M=0;
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
        if (M==2) S++;               // scroll menu
        else {
          if (S==0) v[M]+=SPEEDINC;  // fan 0 speed up
          if (S==1) b[M]+=SPEEDINC;  // boost 0 speed up
          if (S==2) btime[M]++;      // boost time 0 up
          if (S==3) cat[M]++;        // cat 0 on/off
          if (S==4) ctime[M]+=10;    // cat time 0 up
          if (S==5) rtime[M]+=10;    // rise time 0 up
          if (S==6) otime[M]++;      // off time 0 up
        }
      } else {                                                                 // turn encoder counterclockwise
        if (M==2) S--;               // scroll menu
        else {
          if (S==0) v[M]-=SPEEDINC;  // fan 0 speed down
          if (S==1) b[M]-=SPEEDINC;  // boost 0 speed down
          if (S==2) btime[M]--;      // boost time 0 down
          if (S==3) cat[M]--;        // cat 0 on/off
          if (S==4) ctime[M]-=10;    // cat time 0 down
          if (S==5) rtime[M]-=10;    // rise time 0 down
          if (S==6) otime[M]--;      // off time 0 down
        }
      }
      
      for (int i=0;i<2;i++) if (b[i]<v[i]) b[i]=v[i];

      if (M!=2) {
        if (S<=1) {;updatePWM();rts=MS+RDELAY;}                                   // apply (boost) speed change
        if (S==2) btime[M]=cut(btime[M],0,60);                                              // check boost time
        if (S==3) {;cat[M]=cut(cat[M],0,1);catts[0]=MS;}                                           // check cat
        if (S==4) ctime[M]=cut(ctime[M],60,240);                                              // check cat time
        if (S==5) rtime[M]=cut(rtime[M],0,240);                                              // check rise time
        if (S==6) oset(M,otime[M]);                                                           // check off time
      }
      
      if (M==2) S=cut(S,0,6);                                                                // check menu mode
      
      SAVE++;updatelcd();delay(50);
    } 
  }
  enclast=encval;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////// SUPPORT

void oset(int n,int t) { ///////////////////////////////////////////////////////////////// set switch off timer
  otime[n]=cut(t,0,99);ots[n]=MS;ostate[n]=otime[n]?1:0;
}

void fset(int n,int s) { /////////////////////////////////////////////////////////////// set fan state (on/off)
  if (s==1) {
    rts=MS+RDELAY;F[n]=1;catts[n]=vts[n]=MS;r[n]=0;updatePWM();
  } else {
    F[n]=0;bstate[n]=0;
  }
}

void bset(int n,int s) { ///////////////////////////////////////////////////////////// set boost state (on/off)
  if (s==1) {
    rts=MS+RDELAY;bstate[n]=1;bts[n]=MS;F[n]=1;vts[n]=MS;  
  } else {
    bstate[n]=0;rts=MS+RDELAY;updatePWM();  
  }
}

void updatelcd() { ///////////////////////////////////////////////////////////////////////////////// update LCD
  for (int i=0;i<2;i++) {
    if (S==0) {;slcd(1,1,5,"SPEED");ilcd(7+i*5,1,-4,int(v[i]));}
    if (S==1) {;slcd(1,1,5,"BOOST");ilcd(7+i*5,1,-4,int(b[i]));}
    if (S==2) {;slcd(1,1,5,"BTIME");ilcd(7+i*5,1,-4,int(btime[i]));}
    if (S==3) {
      slcd(1,1,5,"CATCH");
      if (cat[i]==0) slcd( 7+i*5,1,-4,"OFF"); else slcd( 7+i*5,1,-3,"ON");
    }
    if (S==4) {;slcd(1,1,5,"CTIME");ilcd(7+i*5,1,-4,int(ctime[i]));}
    if (S==5) {;slcd(1,1,5,"RTIME");ilcd(7+i*5,1,-4,int(rtime[i]));}
    if (S==6) {;slcd(1,1,5,"OTIME");ilcd(7+i*5,1,-4,int(otime[i]));}
  }
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
        else slcd(0,0,1," ");
        ilcd(7+i*5,0,-4,round(xpm[i]));
        if ((!bstate[i] && xv[i]<v[i]) || (bstate[i] && xb[i]<b[i])) clcd(6+i*5,0,1);
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
  xb[i]=b[i];if (bstate[i]) xb[i]=cut(((b[i]-v[i])/(rtime[i]*1000.0)*(MS-bts[i]))+v[i],v[i],b[i]);
  xv[i]=v[i];xv[i]=cut(((v[i]-FANMIN)/(rtime[i]*1000.0)*(MS-vts[i]))+FANMIN,FANMIN,v[i]);
}

void setPWM(int n,int v) { ///////////////////////////////////////////////////////////////// set value to OCR1x
  if (n==0) OCR1A=v;
  if (n==1) OCR1B=v;
}

void updatePWM() { ////////////////////////////////////////////////////////////// update PWM output (fan speed)
  for (int i=0;i<2;i++) {
    v[i]=cut(v[i],FANMIN,FANMAX);
    b[i]=cut(b[i],FANMIN,FANMAX);
    calcramp(i);
    if (!F[i]) setPWM(i,0); else setPWM(i,cut((bstate[0]?xb[0]:xv[0])/(FANMAX/320.0)+r[0],0,320));
  }
} 

void rpmint0() {;ac[0]++;bc[0]++;} ///////////////////////////////////////////////////////////// rpm interrupts
void rpmint1() {;ac[1]++;bc[1]++;}

double cut(double v,double min,double max) {;return v>max?max:v<min?min:v>max?max:v;} ///// cut values to limit

int eer(int adr) {;return EEPROM.read(adr*2)+EEPROM.read(adr*2+1)*256;} /////////////////////////// read EEPROM

void eew(int adr, int val) {;EEPROM.write(adr*2,val%256);EEPROM.write(adr*2+1,val/256);} /////// save to EEPROM

void save() { /////////////////////////////////////////////////////////////////////////////////// save settings
  for (int i=0;i<2;i++) {
    eew(1+i,v[i]);eew(3+i,b[i]);eew(5+i,btime[i]);eew(7+i,cat[i]);
    eew(9+i,ctime[i]);eew(11+i,F[i]);eew(13+i,rtime[i]);
  }
}

int cutcmd(String data) { ///////////////////////////////////////////////////////////// get saperated substring
  int mi=data.length(),n=0,j=0;
  for (int i=0;i<=mi;i++) {
    if (data.charAt(i) == ':' || i == mi) {;cmd[n]=data.substring(j,i);j=i+1;n++;}
  }
  for (int i=1;i<n;i++) icmd[i]=cmd[i].toInt();
  return n;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////// END