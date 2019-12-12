#include <MFRC522.h>
#include <SPI.h>
#include <Keypad.h>

#define SS_PIN 53
#define RST_PIN 49

/* Create an instance of the RFID library */ 
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key; 

int code[] = {195,222,214,26}; //This is the stored UID
int codeRead = 0; 
String uidString; 

const byte ROWS = 4; //four rows                                              //declare rows
const byte COLS = 4; //three columns                                          //declare columns
char keys[COLS][ROWS] = {                                                     //declare 2 dimensional array to map keys
  {'1', '4', '7', '*'},
  {'2', '5', '8', '0'},
  {'3', '6', '9', '#'},
  {'A', 'B', 'C', 'D'}
};
byte rowPins[COLS] = {5, 4, 3, 2};                                            //connect to the row pinouts of the keypad
byte colPins[ROWS] = {9, 8, 7, 6};                                            //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );     //create instance of Keypad called keypad

String  passwd = "12345"; // Defualt password
 
String passwd2=""; //to save the keys user entered in this variable. 

String passwd3=""; // to hold the renew password.

int lock = 12;

void setup() {
  
  Serial.begin(9600);
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)

   Serial.print("Scan RFID Lock"); // ask user to scan the RFID card
  
  pinMode(lock, OUTPUT); // assign the pin 12 to control the locker.
  
  keypad.addEventListener(keypadEvent); // Add an event listener for this keypad

  digitalWrite(lock,LOW); // set the locker as locked
}


void loop() {
  if(  rfid.PICC_IsNewCardPresent()) //if there is RFID scanned go to the function readRFID 
  {
     readRFID();

  }
  delay(100);
  
    char key = keypad.getKey(); // save the keys that are pressed on key variable 
  if (key != NO_KEY){  
       Serial.print(key); // print the keys that are pressed on the serial monitor
             
  }

}


void readRFID() // check if the RFID card type is MIFARE Classic.
{
  
  rfid.PICC_ReadCardSerial();
  Serial.print(F("\nPICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }

    
    printDec(rfid.uid.uidByte, rfid.uid.size);

    uidString = String(rfid.uid.uidByte[0])+" "+String(rfid.uid.uidByte[1])+" "+String(rfid.uid.uidByte[2])+ " "+String(rfid.uid.uidByte[3]);
    
    int i = 0;
    boolean match = true;
    while(i<rfid.uid.size)
    {
      if(!(rfid.uid.uidByte[i] == code[i]))
      {
           match = false;
      }
      i++;
    }

    if(match)
    
    {

  // Serial.println(uidString); // Uncomment to see the UID for scanned cards
    
    if (passwd == passwd2){
      Serial.println("\nI know this card!"); // if authorized card scan with correct password entered it will print I know this card
     
    
    }
    }
    else {
    
      Serial.println("\nWrong Card"); // if unauthorized card scan it will print Wrong Card

       delay(500);
   
    
     pinMode(lock, LOW); // Do not unlocked. 
       
    }

    
    // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
}

/**
 * Helper routine to dump a byte array as dec values to Serial.
 */
void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0 " : " ");
    Serial.print(buffer[i], DEC);
  }
}

  

// Taking care of some special events.
void keypadEvent(KeypadEvent key)
{

    switch (keypad.getState())
    {
    case PRESSED:
    
        if (key =='1' || key =='2' || key =='3' || key =='4' || key =='5' || key =='6' || key =='7' || key =='8' || key =='9')
        {
          passwd2 += key; //save the pressed keys on passwd2 variable   
        }
        if (key == '#')  // If the key # pressed 
        { // check if password is correct
         
           // Serial.println("User pressed # I am going to check password"); // I have used this line just for debugging 
          
            if(passwd == passwd2)
            {
              Serial.println("Success");  
              
                Serial.println("Unlocked");

              digitalWrite(lock,HIGH);//Unlock the locker.
       

                 // printUnlockMessage();
                  
            // keyy();
             
              passwd2="";
            }
            else
            {
              //  display.clearDisplay();
             // String errmsg = "Wrong password ! Expected " + passwd + "User Entered" + passwd2 + "Please try again"; //// I have used this line just for debugging 
          
              passwd2="";
              
          //    Serial.println(errmsg);

            
              Serial.println("Wrong password");

         
              digitalWrite(lock,LOW); // Do not Unlock the locker.
            }
            delay(60);      
         }           
       
        break;

    case RELEASED:    // If the key * pressed let user change password.
        if (key == '*') { 
 
        Serial.println("Enter current password11:  ");

         /*         I used the comment lines below for debugging 
                      String Cpasswd = "Current password" + passwd ;  
                       String Repass = "* Entered" + passwd2 ;
                       String Dpass = "D Entered" + passwd3 ;
        Serial.println(Cpasswd);
        Serial.println(Repass);
        Serial.println(Dpass);
        */
        }
        break;

  case HOLD: // If the key "D" is Pressed for more than 1/2 second this event will occure 
    if (key == 'D'){  
      
      if ( passwd == passwd2){
      
        Serial.println("Enter new password:  ");
      //  passwd = "";
        passwd2 = passwd;
         
      } else {
        Serial.print("Current password is wrong");
        passwd2 = "";
      }
  
    }

}

    } 
      void newpass(){ // i was trying to save the new password in new variable, but i did not work!!

          String Cpasswd = "Current password" + passwd ;
                       String Repass = "* Entered" + passwd2 ;
                       String Dpass = "D Entered" + passwd3 ;
         // passwd = "";
       char key = keypad.getState();
            if (key =='1' || key =='2' || key =='3' || key =='4' || key =='5' || key =='6' || key =='7' || key =='8' || key =='9')
        {
          passwd3 += key;
        //Serial.println(Dpass);
        } else {
         
          Serial.print("suppose to be passwd");
        }


                    Serial.println("~~~~~~~~~~~~~~~~");
                     
        Serial.println(Cpasswd);
        Serial.println(Repass);
        Serial.println(Dpass);
     
      }
  
