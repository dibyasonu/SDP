#include <Adafruit_Fingerprint.h>
#include <ArduinoJson.h>

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
  if (finger.verifyPassword()) 
  {
    Serial.println(F("Found fingerprint sensor!"));
  }
  else 
  {
    Serial.println(F("Did not find fingerprint sensor :("));
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
        Serial.println(F("\nImage taken"));
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(",");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println(F("Communication error"));
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println(F("Imaging error"));
        break;
      default:
        Serial.println(F("Unknown error"));
        break;
    }

    if(p != FINGERPRINT_OK) continue;

    p = finger.image2Tz(CB);
    switch (p) 
    {
      case FINGERPRINT_OK:
        Serial.println(F("Image converted"));
        break;
      case FINGERPRINT_IMAGEMESS:
        Serial.println(F("Image too messy"));
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println(F("Communication error"));
        break;
      case FINGERPRINT_FEATUREFAIL:
        Serial.println(F("Could not find fingerprint features"));
        break;
      case FINGERPRINT_INVALIDIMAGE:
        Serial.println(F("Could not find fingerprint features"));
        break;
      default:
        Serial.println(F("Unknown error"));
        break;
    }
  }  
}





void enrollStudent()
{
  begin:
  Serial.print("Waiting for valid finger to enroll as #"); Serial.println(student_id);
  storeFingerPrint(1);
  // OK success!
 
  Serial.print("Remove finger ");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) 
  {
    p = finger.getImage();
  }


  Serial.println(F("Place same finger again"));
  storeFingerPrint(2);
  // OK CB1 and CB2 done

 

  p = finger.createModel();
  if (p == FINGERPRINT_OK) 
  {
    Serial.println(F("Prints matched!"));
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println(F("Communication error"));
    goto begin;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println(F("Fingerprints did not match"));
    goto begin;
  } else {
    Serial.println(F("Unknown error"));
    goto begin;
  }  
  


  p = finger.getModel();

  switch (p) 
  {
    case FINGERPRINT_OK:
      break;
   default:
      Serial.print(F("Unknown error ")); 
      break;
  }


Serial.println(F("Writting bytes "));  
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
}



void loadTemplate()
{
  
  finger.uploadModelPercobaan(fp_packet1, fp_packet2, fp_packet3, fp_packet4);
  
  Serial.print(F("ID ")); Serial.println(student_id);

  p = finger.storeModel(student_id);
  staus_code = p;
  if (p == FINGERPRINT_OK) {
    Serial.println(F("Stored!"));
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println(F("Communication error"));
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println(F("Could not store in that location"));
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println(F("Error writing to flash"));
  } else {
    Serial.println(F("Unknown error"));
  }  



}
void checkAttendance()
{
  p = -1;
  Serial.print(F("Waiting for valid finger"));
  while (p != FINGERPRINT_OK) 
  {
    p = finger.getImage();
    switch (p) 
    {
      case FINGERPRINT_OK:
        Serial.println(F("\nImage taken"));
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(F(","));
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println(F("Communication error"));
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println(F("Imaging error"));
        break;
      default:
        Serial.println(F("Unknown error"));
        break;
    }

    if(p != FINGERPRINT_OK) continue;

    p = finger.image2Tz();
    switch (p) 
    {
      case FINGERPRINT_OK:
        Serial.println(F("Image converted"));
        break;
      case FINGERPRINT_IMAGEMESS:
        Serial.println(F("Image too messy"));
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println(F("Communication error"));
        break;
      case FINGERPRINT_FEATUREFAIL:
        Serial.println(F("Could not find fingerprint features"));
        break;
      case FINGERPRINT_INVALIDIMAGE:
        Serial.println(F("Could not find fingerprint features"));
        break;
      default:
        Serial.println(F("Unknown error"));
        break;
    }
  }
  fp_confidence = 0; student_id = 0x00;
  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println(F("Found a print match!"));
    student_id = finger.fingerID;
    fp_confidence = finger.confidence;
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println(F("Communication error"));

  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println(F("Did not find a match"));

  } else {
    Serial.println(F("Unknown error"));

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

