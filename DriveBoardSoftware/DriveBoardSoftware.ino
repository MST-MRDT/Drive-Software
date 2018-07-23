/*
 * DriveBoard Software Rev 2 2018
 * Used with DriveBoard Rev 2 2018
 * Writes Serial to 6 motor controllers, controls RGB LD strips, Headlights, and 4 Dropbay Servos
 * 
 * Andrew Van Horn, Judah Schad
 * Disclaimer- I know this code is jank. The servo libraries were giving me trouble at competition, and this conficguration somehow worked. Give me a break
 */

#include "RoveWare.h"
#include "Servo.h"

#include "Adafruit_NeoPixel.h" // todo judah/andrew

///////////////////////////////////////////
//Pin Assignments
const uint8_t DROPBAY_SERVO1_PIN   = PM_3;
const uint8_t DROPBAY_SERVO2_PIN  = PN_2;
const uint8_t SECONDARY_GIMBAL_PAN_SERVO3_PIN  = PM_7;
const uint8_t SECONDARY_GIMBAL_TILT_SERVO4_PIN = PP_5;
const uint8_t HEADLIGHT_PIN                    = PM_6;
const uint8_t RED_LED_PIN                      = PM_1;
const uint8_t GREEN_LED_PIN                    = PM_0;
const uint8_t BLUE_LED_PIN                     = PM_2;

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

int16_t pan_servo_position  = 180;
int16_t tilt_servo_position = 90;

//Only used for extreme retrieval task
//Commented out to remove complexity in the servo library to add two axs gimbal code
//It was jank
//We were having trouble with the main gimbalboard code which used a lot of servo instances, so we decided not to instantiate the drop bay servo in the code
//Commented out code is related to the dropbay
/*
Servo DropBay1;
Servo DropBay2; */
Servo SecondaryGimbalPan;
Servo SecondaryGimbalTilt;
Servo servo1;
Servo servo2;


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

 // pinMode(LEDSTRIP_SERVO4_PIN, OUTPUT);
  pinMode(HEADLIGHT_PIN,       OUTPUT);

  //digitalWrite(LEDSTRIP_SERVO4_PIN, LOW); judah todo
  digitalWrite(HEADLIGHT_PIN,       LOW);

/*
  DropBay1.attach(DROPBAY_SERVO1_PIN);
  DropBay2.attach(DROPBAY_SERVO2_PIN);

  DropBay1.write(0);
  DropBay2.write(0);*/

  SecondaryGimbalPan.attach( SECONDARY_GIMBAL_PAN_SERVO3_PIN);
  SecondaryGimbalTilt.attach(SECONDARY_GIMBAL_TILT_SERVO4_PIN);
  servo1.attach(DROPBAY_SERVO1_PIN);
  servo2.attach(DROPBAY_SERVO2_PIN);
  

  SecondaryGimbalPan.write( pan_servo_position);
  SecondaryGimbalTilt.write(tilt_servo_position);
  servo1.write(pan_servo_position);
  servo2.write(tilt_servo_position);

  // Setup LED pins
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(BLUE_LED_PIN, LOW);

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

/*    case DROP_BAY_OPEN:
    {
      uint8_t drop_bay = data_value[0];
      Serial.println("Got to Open Dropbay");
      switch (drop_bay)
      {   
        case DROP_BAY_1:
        
          DropBay1.write(150);        
          Watchdog.clear();
          break;  
        
        case DROP_BAY_2:
        
          DropBay2.write(150);  //Used 150 because of testing. it fits better      
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
    } */
    
    case HEADLIGHTS:
    {
      uint8_t on_off = data_value[0];
      Serial.println("Headlights");
      
      digitalWrite(HEADLIGHT_PIN, on_off);
      Watchdog.clear();
      break;
    }
    
    case UNDERGLOW_COLOR:
    {
      /*
      for (int i = 0; i < LED_COUNT; i++)
      {
        NeoPixel.setPixelColor(i, data_value[0], data_value[1], data_value[2]);
      }
      NeoPixel.show();
      */
      
      analogWrite(RED_LED_PIN,   data_value[0]);
      analogWrite(GREEN_LED_PIN, data_value[1]);
      analogWrite(BLUE_LED_PIN,  data_value[2]);
      
      Watchdog.clear();
      break;
    }

    case 102: //Pan
    {
      int16_t *gimbal_values = ((int16_t*)(data_value));
      int16_t pan_inc = gimbal_values[0];
      pan_servo_position += pan_inc;
      pan_servo_position = constrain(pan_servo_position, 0, 180);

      servo1.write(pan_servo_position);
      Serial.println(pan_servo_position);
      break;
    }

    case 103:  //Tilt
    {
      int16_t *gimbal_values = ((int16_t*)(data_value));
      int16_t tilt_inc = gimbal_values[0];
      tilt_servo_position += tilt_inc;
      tilt_servo_position = constrain(tilt_servo_position, 0, 180);

      servo2.write(tilt_servo_position);
      Serial.println(tilt_servo_position);
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
