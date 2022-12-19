/**
 Sprint 3 : Senor de Luz
 */
#define channelValue 0
#include <Wire.h>
#include <Adafruit_ADS1X15.h>

Adafruit_ADS1115 ads1115;
int  val = 0;

// Real----->lightReading()
void lightReading(int pin){ //Funcion de intervalos
   int val = ads1115.readADC_SingleEnded(pin);
   double valF;
   if(val<3){
        val=3;
      }
      if(val<=21 && val>=3){
       Serial.println("Sensor Tapado/obstruido");
      }
      if(val<=450 && val>21){
       Serial.println("Sombreado/Nubes");
      }
      if(val>450 && val<2100){
        Serial.println("Soleado");
       }
      if(val>=2100){
        Serial.println("Demasiada luz para el huerto");
      }
      valF=(4.096*10/32767)*val;
      Serial.println(valF);
       delay(2000);
     }

void setup() {
  Serial.begin(9600);
  ads1115.begin(0x48);
  
  
  ads1115.setGain(GAIN_ONE);  
  }


 void loop(void){
   lightReading(3);
 }
