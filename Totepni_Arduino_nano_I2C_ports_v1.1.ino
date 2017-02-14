
// #include "SPI.h"
#include "Wire.h"
#include "OneWire.h"
#include "Ethernet.h"
#include "LiquidCrystal_I2C.h"
#include "SoftwareSerial.h"
#include "DallasTemperature.h"
#include "AM2320.h"
//#include "avr/wdt.h"

// definice cidla teplota a vlhkost
AM2320 th;

float teplota;
float vlhkost;

int val_int;
int val_decimal;

// precteny data z I2C pro textovy vystup na odeslani do displeje
// I2C PINs A4, A5
String dataI2C; 

// convert to bin
byte bin_convert[9] = {99, 1, 2, 4, 8, 16, 32, 64, 128};
// tlacitka vstupu
byte tlacitko_modul[5] = {99, A2, A3, A0, A1};
// pamet rele vystupu
byte rele_modul[17] = {99, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// detekce alarmu
byte alarm = 8;
boolean is_alarm = false;
byte internet_error = 9;

char s;
SoftwareSerial SIM900(5, 6); //RX,TX

// nastavení čísla vstupního pinu
const int pinCidlaDS = 3;
// vytvoření instance oneWireDS z knihovny OneWire
OneWire oneWireDS(pinCidlaDS);
// vytvoření instance senzoryDS z knihovny DallasTemperature
DallasTemperature senzoryDS(&oneWireDS);


//nastavíme adresu a typ displeje
LiquidCrystal_I2C lcd(0x3F,20,4);  // nastavení adresy (0x3F) je důležité!!!

EthernetClient client;
// Ethernet PINs 10, 11, 12, 13
signed long next;
byte page_cnt;
int next_sd;  
int next_wd;
int data_read;
int set_val;
char read_buffer;
boolean nalez = false;

// the dns server ip
IPAddress dnServer(193, 85, 1, 12);
// the router's gateway address:
IPAddress gateway(192, 168, 1, 1);
// the subnet:
IPAddress subnet(255, 255, 255, 0);
//the IP address is dependent on your network
IPAddress ip(192, 168, 1, 15);

/*
// --- Dneboh ---
// the dns server ip
IPAddress dnServer(10, 20, 30, 1);
// the router's gateway address:
IPAddress gateway(10, 20, 30, 1);
// the subnet:
IPAddress subnet(255, 255, 255, 0);
//the IP address is dependent on your network
IPAddress ip(10, 20, 30, 21);
*/


// byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };   //physical mac address
uint8_t mac[6] = {0x00,0x01,0x02,0x03,0x04,0x05}; 
char server[] = "www.ipf.cz"; //server, kam se pripojujeme




void setup()   {                

  // wdt_enable(WDTO_8S);
  
  Wire.begin();  
  Serial.begin(9600);
  SIM900.begin(9600); 

  // zapnutí komunikace knihovny s Dallas teplotním čidlem
  senzoryDS.begin();

  
  lcd.init();                      // initialize the lcd 
 
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor ( 0, 0 );
  lcd.print("Hello, world!");  

  // tlacitka relatek jako vstupy s pull up odporem na vstupu
  for(int i = 1; i <= 4; i++){
        pinMode(tlacitko_modul[i], INPUT_PULLUP);
  }

  // vstup z alarmu    
  pinMode(alarm, INPUT_PULLUP);

  // kontrolka ze nejede internet
  pinMode(internet_error, OUTPUT);


  Ethernet.begin(mac, ip, dnServer, gateway, subnet);


  // MCP23016 I/O expander configure
  Wire.begin(); // connect the Arduino as a master 
  Wire.beginTransmission(0x20); // setup out direction registers 
  Wire.write(0x06); // pointer  
  Wire.write(0x00); // DDR Port0 all output 
  Wire.write(0x00); // DDR Port1 all input 0xFF = B11111111   
  Wire.endTransmission();     

   /*
   Wire.beginTransmission(0x20); // setup out direction registers 
   Wire.write(0x04); // pointer 
   Wire.write(0xff); // DDR Port0 no invert Port0  
   Wire.write(0x00); // DDR Port1 invert Port0   
   Wire.endTransmission(); 
   */

  // strankovac displeje
  page_cnt = 1;
  
  // info casovac dalsiho precteni dat z webu
  next = 0;

  

}


void loop() {
  
  // --------- tepomery ----------
  senzoryDS.requestTemperatures();
  readTemp();
  // read_data_topeni();

   
  // setI2Cvals(senzoryDS.getTempCByIndex(0));

  /*
  showtext("Kuchyn + K", "teplota", 16);
  showtext("Pokoj puda", "teplota", 20);
  showtext("Obyvak", "teplota", 22);
  showtext("Loznice", "teplota", 24);
  showtext("Venku", "teplota", 22);
  */


 
  next_sd++;
  
  if (next_sd > 5){  

        next_sd = 0;
  
      
        // posleme stav relatek
        for(int i = 1; i <= 16; i++){
              // Wire.write(rele_modul[i]);
        }
      
        
        // zobrazime stav teplomeru a vlhkomeru
        //setI2Cvals(teplota);
        
        if(page_cnt == 1){
                  
                  lcd.clear();
                  lcd.setCursor ( 0, 0 );
                  lcd.print("Kuchyn + Koup.");
                  lcd.setCursor ( 16, 0 );  
                  lcd.print(teplota, 1);
                  // lcd.print(vlhkost);    
                
                  lcd.setCursor ( 0, 1 );
                  lcd.print("Pokoj puda ");
                  lcd.setCursor ( 16, 1 ); 
                  lcd.print(senzoryDS.getTempCByIndex(0), 1);
          
                  lcd.setCursor ( 0, 2 );
                  lcd.print("Obyvak ");  
                  lcd.setCursor ( 16, 2 );
                  lcd.print(senzoryDS.getTempCByIndex(1), 1);
          
                  lcd.setCursor ( 0, 3 );
                  lcd.print("Loznice ");
                  lcd.setCursor ( 16, 3 ); 
                  lcd.print(senzoryDS.getTempCByIndex(2), 1);
          
                  // lcd.setCursor ( 0, 4 );
                  // lcd.print("Venku: ");  
                  // lcd.print(senzoryDS.getTempCByIndex(1));
          
                  page_cnt++;
                  
        }else if(page_cnt == 2){

                  lcd.clear();
                  lcd.setCursor ( 0, 0 );
                  lcd.print("Kuchyn + Koupelna");
                  lcd.setCursor ( 0, 1 );
                  lcd.print("Vlhkost ");                  
                  lcd.setCursor ( 14, 1 );  
                  lcd.print(vlhkost); 
                  lcd.print("%");   
                
                  lcd.setCursor ( 0, 3 );
                  lcd.print("Teplota venku");
                  lcd.setCursor ( 16, 3 ); 
                  lcd.print(senzoryDS.getTempCByIndex(2), 1);
                    
                  page_cnt = 1;
          
        }
              
        delay(100);
        
        clr_wdt();
        
  }


  // ----------- tlacitka a alarm a Internet ----------


   // zkontrolujeme alarm 
  // Serial.println("Test alarm");  
  if(digitalRead(alarm) == LOW){
             delay(300);
             if(digitalRead(alarm) == LOW){
                if(is_alarm == false){
                        digitalWrite(internet_error, HIGH);
                        setAlarm();
                  }
                is_alarm = true;
             }else{
                is_alarm = false;
             }
        }else{
             is_alarm = false;
        }  
    // Serial.println("OK");  






    // projedeme si stisknuta tlacitka
    // Serial.println("Test key");  
    for(int i = 1; i <= 4; i++){
          if(digitalRead(tlacitko_modul[i]) == LOW){
            Serial.println(i); //vypise stisknute tlacitko
            next_sd = 999;
            setRelayFromKey(i);
          }
          delay(1);
    }
    // Serial.println("OK");  



  // Serial.println("Test internet");  
  if (((signed long)(millis() - next)) > 0){
      
        Serial.println("Read internet ready");  
        next = millis() + 11000;

        digitalWrite(internet_error, HIGH);

        read_data_topeni(0);
        
  }
  // Serial.println("Test internet OK");  



        // zapnu relatka
        
        set_val = 0;
        for(int i = 1; i <= 8; i++){
              if(rele_modul[i] == 0){
                set_val+= bin_convert[i];                                
                }               
        } 
                
      
        Wire.beginTransmission(0x20);  // set mcp23016 for all output
        Wire.write(0x00); // begin here
        Wire.write(set_val); 
        Wire.write(set_val); 
        Wire.endTransmission();

    

}





// alarm zavola na mobil
void setAlarm(){
            Serial.println("Alarm");                   
            
            //makeCall("604833891");
            //makeCall("605906254");
            makeCall("737226659");
            //sendSMS("737226659");
            //makeCall("737226659");
            
}




// prevede float cislo na dve desetine na prenos po I2C
void setI2Cvals(float teplo_conv){
  
        val_int = teplo_conv;
        val_decimal = (teplo_conv - val_int) * 10;  
}



void readTemp() {
  switch(th.Read()) {
    case 2:
      teplota = 99;
      break;
    case 1:
      teplota = -99;
      break;
    case 0:
      vlhkost = th.h;
      teplota = th.t;
      break;
  }

  delay(10); // 200
  clr_wdt();
}


void clr_wdt() {

      // wdt_reset();
  
}


void read_data_topeni(int send_relay) {

      
      clr_wdt();

      Serial.print("Connect to ");
      Serial.println(server);

      
      if (client.connect(server,80)){

          Serial.println("Connected");

          // digitalWrite(internet_error, LOW);
          clr_wdt();
          
          String myURL = "GET /topeni/topeni.php?relay=";
          client.print(myURL);
          client.print(send_relay);
          client.print("&status=");
                  
                if (rele_modul[send_relay] == 0){client.print(0);}
                if (rele_modul[send_relay] == 1){client.print(1);}               
          
          client.print("&temp[1]=");
          client.print(teplota);
          client.print("&humid[1]=");
          client.print(vlhkost);
          client.print("&temp[2]=");
          client.print(senzoryDS.getTempCByIndex(0));
          client.print("&temp[3]=");
          client.print(senzoryDS.getTempCByIndex(1));
          client.print("&temp[4]=");
          client.print(senzoryDS.getTempCByIndex(2));
          client.print("&temp[5]=");
          client.print(senzoryDS.getTempCByIndex(1));
          
          client.println(" HTTP/1.0");
          client.print("Host: ");
          client.println(server);
          client.println("Connection: close");
          client.println();

          Serial.print(myURL);
          Serial.print(send_relay);
          Serial.print("&status=");
          Serial.println(rele_modul[send_relay]);

          nalez = false;
          byte relec = 1;        
          delay(1);
          

          // precteni dat z internetu
                    
          while(client.connected()) {
            if(client.available()) {
                char read_buffer = client.read();
     
                
                if (read_buffer == '>'){nalez = false;}
                 
                if (nalez){

                  digitalWrite(internet_error, LOW);
                  // Serial.print(read_buffer); //vypise prijata data
                  // na tohle mozna udelame funkci  
                  if (read_buffer == '0'){rele_modul[relec] = 0;}
                  if (read_buffer == '1'){rele_modul[relec] = 1;}
                  relec++;                  

                }
  
                if (read_buffer == '<'){nalez = true;}
                         
            }
          }
        
    
         //Serial.println();
         //Serial.println("Odpojuji.");
         delay(1);
         client.flush();
         delay(1);
         //Serial.println();
         client.stop();   
         
       }else{
        
        //Serial.println("Connect failed");

        digitalWrite(internet_error, HIGH);

        clr_wdt();

       }
 
    Serial.println("Internet OK");  
    clr_wdt();
}









// nastavi rele a zapise na internet
void setRelayFromKey(int tlac_press){

            Serial.print("Ctu stav rele ");
            Serial.print(tlac_press);
            Serial.print(" ten je nastaven na ");
            Serial.println(rele_modul[tlac_press]);            

            // na tohle mozna udelame funkci  
            // rele nastavime
            rele_modul[tlac_press] = !rele_modul[tlac_press];

            
            Serial.print("Tlacitko ");
            Serial.print(tlac_press);
            Serial.print(" stlaceno, rele vstup ");
            Serial.print(rele_modul[tlac_press]);
            Serial.println(" nastaveno");
            
            read_data_topeni(tlac_press);
            
            // delay(1000);
            clr_wdt();
}









void delayWDT(int delaysec = 30){
            clr_wdt();
       
            int waiting_call = 0;
            while(waiting_call <= delaysec){
                delay (1000);
                clr_wdt();
                waiting_call += 1;
            }            
            clr_wdt();
}





void makeCall(String callnumber){

            lcd.clear();
            lcd.setCursor ( 3, 0 );
            lcd.print("*** Alarm ***");
            lcd.setCursor ( 4, 1 );  
            lcd.print("Volam cislo:");
            lcd.setCursor ( 6, 2 );
            lcd.print(callnumber);            

            clr_wdt();
            
            SIM900.print("ATD");
            SIM900.print(callnumber);
            SIM900.println(";");
            delay(100);            
             
            if(SIM900.available()){
                  lcd.setCursor ( 5, 3 );
                  lcd.print("vytacim...");
                  while (SIM900.available()){
                  s = SIM900.read();
                    Serial.print(s);
                  }
            }else{
                  lcd.setCursor ( 3, 3 );
                  lcd.print("nedostupny...");                    
            }
        
            delayWDT(29);
                       
            SIM900.println("ATH");                      
            delay (1000);
            clr_wdt();
}


/*


void sendSMS(char smsnumber[]){
          SIM900.println("AT+CMGF=1");  // AT command na odeslani SMS zpravy
          delay(100);
          SIM900.print("AT+CMGS=\"");  // cislo prijemce
          SIM900.print(smsnumber);  // cislo prijemce
          SIM900.println("\"");  // cislo prijemce
          delay(100);
          SIM900.println("Alarm send.");  // zprava k odeslani
          SIM900.print("Status: ");  // zprava k odeslani
          SIM900.print(rele_modul[1]);  // zprava k odeslani
          SIM900.print(rele_modul[2]);  // zprava k odeslani
          SIM900.print(rele_modul[3]);  // zprava k odeslani
          SIM900.println(rele_modul[4]);  // zprava k odeslani
          SIM900.print(" is set.");  // zprava k odeslani
          delay(100);
          SIM900.println((char)26);  //  AT command znak a ^Z, ASCII code 26 pro konec SMS zpravy
          delay(100); 
          SIM900.println();
          delayWDT(11);  // pokej na odesln SMS
          //SIM900power();  // vypni GSM module
          clr_wdt();
        }

*/

