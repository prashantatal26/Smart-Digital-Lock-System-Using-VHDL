
#include <LiquidCrystal.h>
#include <Keypad.h>
#include <Servo.h>


LiquidCrystal LCD( 8, 9, 10, 11, 12, 13 ) ; 

Servo servo ;

const int buzzerPin = 14 ;  
const int servoPin = 15 ;   
const byte ROWS = 4, COLS = 4 ;

char keys[ROWS][COLS] = 
{
   {'1','2','3','A'},
   {'4','5','6','B'},
   {'7','8','9','C'},
   {'*','0','#','D'}
} ;

byte rowPins[ROWS] = { 16, 17, 2, 3 } ;
byte colPins[COLS] = { 4, 5, 6, 7 } ;


Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS ) ;

char password[16], string[16] ;
int flag_h_setpassword = 1, flag_inputpassword = 0, flag_inputstring = 0, flag_opendoor = 1, flag_state = 0, flag_remoteopen = 0, flag_lockdown = 0 ;
int count = 0, trial_count=0, pos = 0, state = 0 ;
// flag_setpassword = flag for setting the password,
// flag_stringinput = flag for taking input the string,



void setup() {
   for(int k=8 ; k<14 ; k++) {
      pinMode(k,OUTPUT) ;  
   }

   LCD.begin(16, 2) ;   

   pinMode(buzzerPin, OUTPUT) ;
   pinMode(servoPin, OUTPUT) ;
   servo.attach(servoPin) ;

   // for Bluetooth-Module
   Serial.begin(9600) ;
   LCD.setCursor(0,0) ;
   LCD.print("   WELCOME !!") ;
   LCD.setCursor(0,1) ;
   LCD.print("Set a Password :") ;
   InitializePassword(), InitializeString() ;
   CloseDoor() ;
}



void loop() {
   // for Bluetooth-module
   if( Serial.available() > 0 ) {
      state = Serial.read() ;
      flag_state = 0 ;
   }

// Bluetooth-module is used to remotely open the door lock when system is locked
   if( state == '0' ) {
      trial_count = 4, CloseDoor() ;
      if( flag_state == 0 ) {
         Serial.println("SYSTEM LOCKED") ;
         flag_state = 1 ;
      }
      flag_lockdown = 1 ;
   }
   else if( state == '1'  &&  flag_lockdown == 1 ) {
      trial_count = 0 ;
      LCD.clear() ;
      LCD.setCursor(0,0) ;
      LCD.print("LOCKDOWN LIFTED!") ;
      LCD.setCursor(0,1) ;
      LCD.print("Press * ...") ;
      if( flag_state == 0 ) {
         Serial.println("LOCKDOWN LIFTED") ;
         flag_state = 1 ;
      }
      flag_lockdown = 0, state = 0 ;
   }
   else if( state == '2'  &&  flag_remoteopen == 0  &&  flag_lockdown == 0 ) {
      LCD.clear() ;
      LCD.print("RemotelyVERIFIED") ;
      Serial.println("UNLOCKED !!") ;
      trial_count = 0 ;
      for( int i=0 ; i<3 ; ++i ) {
         tone(buzzerPin, 500, 200) ;
         delay(230) ;
         tone(buzzerPin, 100, 200) ;
         delay(300) ;
         OpenDoor() ;
      }
      if( flag_state == 0 ) {
         Serial.println("Remotely Unlocked !!") ;
         flag_state = 1 ;
      }
      flag_remoteopen = 1 ;
   }
   else if( state == '3'  &&  flag_lockdown == 0 ) {
      trial_count = 0, InitializeString(), H_EnterPassword() ;
      if( flag_state == 0 ) {
         Serial.println("Remotely Locked !!") ;
         flag_state = 1 ;
      }
      flag_remoteopen = 0, state = 0 ;
   }


   //Keypad

   if( trial_count < 3 ) {

      char key = kpd.getKey() ;   //storing pressed key value in a char

      if( key != NO_KEY ) {
         
         if( flag_h_setpassword == 1 ) {
            H_SetPassword() ;
         }
         if( key == '*' ) {
            if( flag_inputpassword == 1 ) {
               InitializePassword(), H_SetPassword() ;
            }
            else if( flag_inputstring = 1 ) {
               InitializeString(), H_EnterPassword() ;
            }
         }
         else if( key == '#' ) {
            if( flag_inputpassword == 1  &&  count > 0 ) {
               flag_inputpassword = 0 ;
               password[count] = '\0' ;
               H_EnterPassword() ;
            } 
            else if( flag_inputstring == 1  &&  count > 0 ) {
               flag_inputstring = 0 ;
               string[count] = '\0' ;
               if( Compare_Password_and_String() == 1 ) {
                  LCD.clear() ;
                  LCD.print("  VERIFIED !  ") ;
                  Serial.println("UNLOCKED") ;
                  trial_count = 0 ;
                  for( int i=0 ; i<3 ; ++i ) {
                     tone(buzzerPin, 500, 100) ;
                     delay(230) ;
                     tone(buzzerPin, 100, 100) ;
                     delay(230) ;
                     OpenDoor() ;
                  }
               }
               else {
                  LCD.clear() ;
                  LCD.print("Wrong Password !") ;
                  delay(1000) ;
                  Serial.println("Someone unsuccessfully attempted to open the lock !") ;
                  ++trial_count ;
                  tone(buzzerPin, 100, 1000) ;
                  delay(1000) ;
                  H_EnterPassword() ;
               }
            }
         }
         else if( flag_inputpassword == 1  ||  flag_inputstring == 1 ) {
            LCD.print(key) ;
            delay(100) ;
            LCD.setCursor(count,1) ;
            LCD.print('*') ;
            if( flag_inputpassword == 1 )   password[count] = key ;
            else if( flag_inputstring == 1 )   string[count] = key ;
            ++count ;
         }
      }
   }
   else {
      LCD.clear() ;
      LCD.setCursor(0,0) ;
      LCD.print("SYSTEM LOCKDOWN!") ;
      tone(buzzerPin, 1000, 1000) ;
      delay(1500) ;
      flag_lockdown = 1 ;
   }
}



void InitializePassword() {
   for( int i=0 ; i<16 ; ++i )
      password[i] = 0 ;
}

void InitializeString() {
   for( int i=0 ; i<16 ; ++i )
      string[i] = 1 ;
}

void H_SetPassword() {
   LCD.clear() ;
   LCD.setCursor(0,0) ;
   LCD.print("Set a Password :") ;
   LCD.setCursor(0,1) ;
   flag_h_setpassword = 0 ;
   flag_inputpassword = 1, count = 0 ;
}

void H_EnterPassword() {
   CloseDoor() ;
   LCD.clear() ;
   LCD.setCursor(0,0) ;
   LCD.print("Enter Password :") ;
   LCD.setCursor(0,1) ;
   flag_inputstring = 1, count = 0 ;
}

int Compare_Password_and_String() {
   int i ;
   for( i=0  ;  password[i]!='\0' && string[i]!='\0'  ;  ++i ) {
      if( password[i] != string[i] )
         return 0 ;
   }
   if( password[i] == '\0'  &&  string[i] == '\0' )
      return 1 ;
   else   return 0 ;
}

void OpenDoor() {
   if( flag_opendoor == 1 )
      return;
   for( pos=15 ; pos<=100 ; ++pos ) {
      servo.write(pos) ;
      delay(15) ;
   }
   flag_opendoor = 1 ;
}

void CloseDoor()  {
   if( flag_opendoor == 0 )
      return;
   for( pos=100 ; pos>=15; --pos ) {
      servo.write(pos) ;
      delay(15) ;
   }
   flag_opendoor = 0 ;
}



