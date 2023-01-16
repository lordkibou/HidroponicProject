/* Programa prueba GPS UBLOX-6M
 *  Conexiones GPIO 12--> TX
 *             GPIO 13--> RX
 */

#include <TinyGPS++.h>

#include <SoftwareSerial.h>

SoftwareSerial gps(12,13);

char dato=' ';

void setup()
{
 Serial.begin(115200);            
 gps.begin(9600); 
 Serial.println("Inicializando el GPS...");
 delay(500);
 Serial.println("Esperando datos");
}


void loop(){
  //if(gps.available())
    dato=gps.read();
    if(dato=='$'){
      Serial.print("\n");
      }
    Serial.print(dato);
    delay(50);
}

   
