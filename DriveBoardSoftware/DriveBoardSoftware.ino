/*
 * DriveBoard Software Rev 2 2018
 * Used with DriveBoard Rev 2 2018
 * Writes Serial to 6 motor controllers, controls RGB LD strips, Headlights, and 4 Dropbay Servos
 * 
 * Andrew Van Horn, Judah Schad
 */

#include "RoveWare.h"
#include "Servo.h"

#include "Adafruit_NeoPixel.h" // todo judah/andrew

///////////////////////////////////////////
//Pin Assignments
const uint8_t DROPBAY_SERVO1_PIN  = PM_3;
const uint8_t DROPBAY_SERVO2_PIN  = PN_2;
const uint8_t LEDSTRIP_SERVO4_PIN = PP_5;   // Not a servo, just on that silkscreen pin
const uint8_t HEADLIGHT_PIN       = PM_6;

//////////////////////////////////////////////////
// We send serial speed bytes to motor controllers 
const byte DRIVE_MAX_FORWARD = 255;
const byte DRIVE_MAX_REVERSE = 0;
const byte DRIVE_ZERO        = 127;
byte left_drive_speed        = DRIVE_ZERO;
byte right_drive_speed       = DRIVE_ZERO;

const byte LED_COUNT         = 60;
const byte LED_SPI_MODULE    = 3;

bool notification_on                 = 0;
const int HEADLIGHT_ON_OFF_FOR_LOOPS = 1000;
int headlight_loop_count             = 0;

Servo DropBay1;
Servo DropBay2;

RoveWatchdog        Watchdog;
Adafruit_NeoPixel   NeoPixel(LED_COUNT, LED_SPI_MODULE);

void roveEstopDriveMotors();

/////////////////////////////////////////////////////////////////////////////////////////////////

void setup()
{
  Serial.begin( 9600); 
  Serial2.begin(19200); 
  Serial3.begin(19200);
  Serial4.begin(19200);
  Serial5.begin(19200);
  Serial6.begin(19200);
  Serial7.begin(19200);
  delay(1);
  
  roveComm_Begin(ROVE_FIRST_OCTET, ROVE_SECOND_OCTET, ROVE_THIRD_OCTET, DRIVEBOARD_FOURTH_OCTET); 
  delay(1);

  pinMode(LEDSTRIP_SERVO4_PIN, OUTPUT);
  pinMode(HEADLIGHT_PIN,       OUTPUT);

  //digitalWrite(LEDSTRIP_SERVO4_PIN, LOW); judah todo
  digitalWrite(HEADLIGHT_PIN,       LOW);

  DropBay1.attach(DROPBAY_SERVO1_PIN);
  DropBay2.attach(DROPBAY_SERVO2_PIN);

  DropBay1.write(0);
  DropBay2.write(0);

  NeoPixel.begin();
  
  Watchdog.begin(roveEstopDriveMotors, 150); 
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop()
{
  uint16_t data_id   = 0;
  size_t   data_size = 0;
  uint8_t  data_value[4];
  roveComm_GetMsg(&data_id, &data_size, &data_value);

  
  Serial.println("");
  Serial.print("ID: ");
  Serial.println(data_id);
  Serial.print("Value: ");
  Serial.println(data_value[0]);

  digitalWrite(HEADLIGHT_PIN, LOW);
  Serial.println("Headlights LOW");
  
  switch (data_id) 
  {     
    case DRIVE_LEFT_RIGHT:
    {       
      int32_t speed            = *(int32_t*)(data_value);     
      int16_t left_speed_temp  =  (int16_t) (speed >> 16); // 2 high bytes contain RED's left speed  as int16
      int16_t right_speed_temp =  (int16_t)  speed;        // 2 low  bytes contain RED's right speed as int16
    
      left_speed_temp   = -left_speed_temp; // Motors were wired backwards     
      left_drive_speed  = map(left_speed_temp,  RED_MAX_REVERSE, RED_MAX_FORWARD, DRIVE_MAX_REVERSE, DRIVE_MAX_FORWARD); 
      right_drive_speed = map(right_speed_temp, RED_MAX_REVERSE, RED_MAX_FORWARD, DRIVE_MAX_REVERSE, DRIVE_MAX_FORWARD);     
      Watchdog.clear();
      break;  
    }

    case DROP_BAY_OPEN:
    {
      uint8_t drop_bay = data_value[0];
      Serial.println("Got to Open Dropbay");
      switch (drop_bay)
      {   
        case DROP_BAY_1:
        
          DropBay1.write(255);        
          Watchdog.clear();
          break;  
        
        case DROP_BAY_2:
        
          DropBay2.write(255);        
          Watchdog.clear();
          break; 
          
        default:
          break; 
      }
      break;
    }
    
    case DROP_BAY_CLOSE:
    {
      uint8_t drop_bay = data_value[0];
      Serial.println("Got to Close Dropbay");
      switch (drop_bay)  
      {
        case DROP_BAY_1: 
        
          DropBay1.write(0);        
          Watchdog.clear();
          break;  
        
        case DROP_BAY_2:  
        
          DropBay2.write(0);        
          Watchdog.clear();
          break; 
           
        default:
        break; 
      }
      break;
    }
    
    case HEADLIGHTS:
    {
      uint8_t on_off = data_value[0];
      
      digitalWrite(HEADLIGHT_PIN, on_off);
      Watchdog.clear();
      break;
    }
    
    case UNDERGLOW_COLOR:
    {
      for (int i = 0; i < LED_COUNT; i++)
      {
        NeoPixel.setPixelColor(i, data_value[0], data_value[1], data_value[2]);
      }
      NeoPixel.show();
      Watchdog.clear();
      break;
    }
    default:
      break;       
  }

  if(notification_on)
  {
    headlight_loop_count ++;
    if((headlight_loop_count % HEADLIGHT_ON_OFF_FOR_LOOPS) < HEADLIGHT_ON_OFF_FOR_LOOPS/2)
    {
      digitalWrite(HEADLIGHT_PIN, 1);
    }
    if((headlight_loop_count % HEADLIGHT_ON_OFF_FOR_LOOPS) >= HEADLIGHT_ON_OFF_FOR_LOOPS/2)
    {
      digitalWrite(HEADLIGHT_PIN, 0);
    }
  }
  
  Serial2.write(left_drive_speed );
  Serial3.write(left_drive_speed );
  Serial4.write(left_drive_speed );  
  Serial5.write(right_drive_speed);
  Serial6.write(right_drive_speed);
  Serial7.write(right_drive_speed);
  delay(1);
}

////////////////////////////////

void roveEstopDriveMotors() 
{ 
  left_drive_speed  = DRIVE_ZERO;
  right_drive_speed = DRIVE_ZERO;
  Watchdog.clear();
}
