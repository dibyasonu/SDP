#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Fingerprint.h>
#include <ArduinoJson.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// SoftwareSerial mySerial(2, 3);//serial3 in mega
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial3);//mega

// Global data for low memory consumption
uint8_t op_mode;         //i
uint8_t fp_packet1[128]; //io
uint8_t fp_packet2[128]; //io 
uint8_t fp_packet3[128]; //io
uint8_t fp_packet4[128]; //io
uint8_t staus_code;      //o
uint8_t student_id;      //i
int fp_confidence;       //o
int p;

void setup() 
{
  while(!Serial);
  Serial.begin(9600);
  finger.begin(57600);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }   
  delay(2000);
  if (finger.verifyPassword()) 
  {
    display.clearDisplay();  
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    display.print(F("Found fingerprint sensor!"));
    display.display();    
  }
  else 
  {
    display.clearDisplay();  
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    display.print(F("Did not find fingerprint sensor :("));
    display.display();
    while (1) { delay(1); }
  }      
}





void storeFingerPrint(uint8_t CB)
{
  p = -1;
  
  while (p != FINGERPRINT_OK) 
  {
    p = finger.getImage();
    switch (p) 
    {
      case FINGERPRINT_OK:
      {
        display.setCursor(0, 30);
        display.println(F("IMAGE TAKEN"));
        display.display();
      }
      break;
      case FINGERPRINT_NOFINGER:
      {
        
        display.setCursor(0, 30);
        display.println(F("NO FINGER"));
        display.display();  
      }
      break;
      case FINGERPRINT_PACKETRECIEVEERR:
      {
    
        display.setCursor(0, 30);
        display.println(F("COMMUNICATION ERROR"));
        display.display();        
      }
      break;
      case FINGERPRINT_IMAGEFAIL:
      {
   
        display.setCursor(0, 30);
        display.println(F("IMAGING ERROR"));
        display.display();        
      }
      break;
      default:
      {
      
        display.setCursor(0, 30);
        display.println(F("UNKNOWN ERROR"));
        display.display();        
      }
      break;
    }

    if(p != FINGERPRINT_OK) continue;

    p = finger.image2Tz(CB);
    switch (p) 
    {
      case FINGERPRINT_OK:
      {
      
        display.setCursor(0, 40);
        display.println(F("IMAGE CONVERTED"));
        display.display();        
      }        
      break;
      case FINGERPRINT_IMAGEMESS:
      {
      
        display.setCursor(0, 40);
        display.println(F("IMAGE TOO MESSY"));
        display.display();        
      }        
      break;
      case FINGERPRINT_PACKETRECIEVEERR:
      {
      
        display.setCursor(0, 40);
        display.println(F("COMMUNICATION ERROR"));
        display.display();        
      }        
      break;        
      case FINGERPRINT_FEATUREFAIL:
      {
      
        display.setCursor(0, 40);
        display.println(F("COULDN'T FIND FINGERPRINT FEATURES"));
        display.display();        
      }        
      break;
      case FINGERPRINT_INVALIDIMAGE:
      {
      
        display.setCursor(0, 40);
        display.println(F("COULDN'T FIND FINGERPRINT FEATURES"));
        display.display();        
      }        
      break;        
      default:
      {
        display.setCursor(0, 40);
        display.println(F("UNKOWN ERROR"));
        display.display();  
      }
      break;
    }
  }  
}





void enrollStudent()
{
  begin:
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);  
  display.setCursor(0, 10);
  display.println(F("Waiting..."));
  display.display();
  storeFingerPrint(1);
  
  // OK success!
  
  display.setCursor(0, 50);
  display.println(F("Remove finger "));
  display.display();
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) 
  {
    p = finger.getImage();
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);  
  display.setCursor(0, 10);
  display.println(F("Place same finger again..."));
  display.display();

  storeFingerPrint(2);
  // OK CB1 and CB2 done

 
  delay(1000);
  
  p = finger.createModel();
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);  
  display.setCursor(0, 10);
    
  if (p == FINGERPRINT_OK) 
  {
    display.println(F("PRINTS MATCHED!"));
    display.display();
    delay(1000);
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    display.println(F("COMMUNICATION ERROR"));
    display.display();
    delay(1000);
    goto begin;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    display.println(F("PRINTS MISMATCH"));
    display.display();
    delay(1000);
    goto begin;
  } else {
    display.println(F("UNKOWN ERROR"));
    display.display();
    delay(1000);
    goto begin;
  }  
  


  p = finger.getModel();

  if(p != FINGERPRINT_OK) 
    goto begin;   

 
  
  uint8_t bytesReceived[554];

  int i = 0;int counter = 0;
  while (i <= 554 ) { 
      if (Serial3.available()) {
          counter += 1;
          bytesReceived[i++] = Serial3.read();
      }
  }
  Serial.print(counter);
  counter = 9;
  for(i = 0;i < 128;i++)
  {
    fp_packet1[i] = bytesReceived[counter++];
  }
  counter = 148;
  for(i = 0;i < 128;i++)
  {
    fp_packet2[i] = bytesReceived[counter++];
  }
  counter = 287;
  for(i = 0;i < 128;i++)
  {
    fp_packet3[i] = bytesReceived[counter++];
  }
  counter = 426;
  for(i = 0;i < 128;i++)
  {
    fp_packet4[i] = bytesReceived[counter++];
  }
  display.println(F("Complete"));
  display.display();      
}



void loadTemplate()
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);  
  display.setCursor(0, 10);
   
  finger.uploadModelPercobaan(fp_packet1, fp_packet2, fp_packet3, fp_packet4);
  p = finger.storeModel(student_id);
  staus_code = p;

  if (p == FINGERPRINT_OK) {
    display.println(F("STORED SUCCESSFULLY !"));
    display.display();
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    display.println(F("COMMUNICATION ERROR"));
    display.display();
  } else if (p == FINGERPRINT_BADLOCATION) {
    display.println(F("BAD LOCATION"));
    display.display();
  } else if (p == FINGERPRINT_FLASHERR) {
    display.println(F("ERROR WRITTING TO FLASH"));
    display.display();
  } else {
    display.println(F("UNKOWN ERROR"));
    display.display();
  }  



}
void checkAttendance()
{
  p = -1;
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);  
  display.setCursor(0, 10);
  display.println(F("Waiting..."));
  display.display();
  storeFingerPrint(1);
  
  fp_confidence = 0; student_id = 0x00;
  p = finger.fingerFastSearch();

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);  
  display.setCursor(0, 10);
    
  if (p == FINGERPRINT_OK) {
    student_id = finger.fingerID;
    fp_confidence = finger.confidence;
    char str[80];
    sprintf(str, "Found match for %u with confidence %d", student_id,fp_confidence);
    display.println(str);
    display.display();    
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    display.println(F("Communication error"));
    display.display();
  } else if (p == FINGERPRINT_NOTFOUND) {
    display.println(F("Did not find a match"));
    display.display();
  } else {
    display.println(F("Unknown error"));
    display.display();

  }     

  
}




//Read data from input
// Loads data to global variable before calling the respective function
bool loadData()
{
  StaticJsonDocument<4182> doc;
//  DynamicJsonDocument doc(4182);
  DeserializationError error = deserializeJson(doc, Serial);
  if (error) 
    {
    return false;
  }
  else
  {
    op_mode = doc["op"];
    student_id = doc["id"];
    for (int i = 0; i < 128; ++i)
    {
      fp_packet1[i] = doc["fp_packet"][i];
    }
    for (int i = 128; i < 256; ++i)
    {
      fp_packet2[i-128] = doc["fp_packet"][i];
    }
    for (int i = 256; i < 384; ++i)
    {
      fp_packet3[i-256] = doc["fp_packet"][i];
    }
    for (int i = 384; i < 512; ++i)
    {
      fp_packet4[i-384] = doc["fp_packet"][i];
    }        
    return true;    
  }
      
}

// craft json data and sends to serial
// uses the value of global op_mode as this function is used from inside the functions, so op_mode remains the same 
void sendData()
{
  StaticJsonDocument<4182> doc; 
//  DynamicJsonDocument doc(4182);
  switch (op_mode) 
  {
    case 1:
    {
      doc["id"] = student_id;
      JsonArray data = doc.createNestedArray("fp_packet");

      for (int i = 0; i < 128; ++i)
       {
          data.add(fp_packet1[i]);
       }      

      for (int i = 0; i < 128; ++i)
       {
          data.add(fp_packet2[i]);
       }

      for (int i = 0; i < 128; ++i)
       {
          data.add(fp_packet3[i]);
       }

      for (int i = 0; i < 128; ++i)
       {
          data.add(fp_packet4[i]);
       }
    }                       
    break;

    case 2:
    {
      doc["id"] = student_id;
      doc["staus_code"] = staus_code;     
    }
    break;

    case 3:
    {
      doc["id"] = student_id;
      doc["confidence"] = fp_confidence;

    }
    break;
  }
  serializeJson(doc, Serial);
}

void loop() {
  bool x = loadData();
  if(x)
  {  
    switch(op_mode)
    {
      case 1:
        enrollStudent();
        sendData();
        break;
      case 2:
        loadTemplate();
        sendData();
        break;
      case 3:
        checkAttendance();
        sendData();
        break;        
    }
  } 

}

