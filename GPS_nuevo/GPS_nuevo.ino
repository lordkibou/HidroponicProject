
/* Programa prueba GPS UBLOX-6M
 *  Conexiones GPIO 12--> TX
 *             GPIO 13--> RX
 */
#include <SoftwareSerial.h>
#include <string>
std::string dato; //Dato completo recibido
char fragmentacionLat;
char fragmentacionLon;

SoftwareSerial gps(7,8);

void setup(){
 Serial.begin(115200);            
 gps.begin(9600); 
 Serial.println("Inicializando el GPS...");
 delay(27000);//Delay de 27 segundos
 Serial.println("Esperando datos");
}


void loop(){
  

  if (gps.available()){
    dato=gps.read();
    if(dato[18]== 'A'){
      Serial.print("El dato no es correcto ni fijo por ahora");
      delay(5000);
    }
    else{
      for(int i=20;i<=28;i++){
        fragmentacionLon+=dato[i];
        }
      for(int j=32;j<=41;j++){
        fragmentacionLat+=dato[j];
        }
      Serial.print("LONG=");  Serial.println(fragmentacionLon);  Serial.println("N");
      Serial.print("LAT="); Serial.println(fragmentacionLat); Serial.println("W");
      delay(5000);
    }
  }
}
