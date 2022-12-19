#include <Adafruit_ADS1X15.h> // Librería del ADS1115
int16_t adc0;
//tension del sensor pH 3,3 a 5,5v
#define channelValue 0
#define Offset -0.14 //Definimos el Offset
#define samplingInterval 20
#define printInterval 800
#define ArrayLength 40 
int pHArray[ArrayLength]; //Array para toma de valores
int pHArrayIndex = 0; //Indice de incremento lista

double averageSample(int *p,int cuantos){ //Funcion Puntero para varios valores y hacer media
  double x=0;
  for(int i=0;i<=cuantos-1;i++){
    x+=p[i];
  }
  return (x/cuantos);
}

//Construir ADS1115
  
  Adafruit_ADS1115 ads1115;


//---------------------------------------------
void setup() {

   
  //Inicializamos el Monitor Serie
  Serial.begin(9600);
  Serial.println("Inicializando el medidor de pH");

  //Inicializamos el ADS1115
   ads1115.begin(0x48);

  //Configuramos la ganancia del ADS1115
  ads1115.setGain(GAIN_ONE);

  adc0 = ads1115.readADC_SingleEnded(0);
}

void loop(void) {
  static unsigned long samplingTime = millis();
  static unsigned long printTime = millis();
  static float pHValue,voltage;
  if (millis() - samplingTime > samplingInterval){
    pHArray[pHArrayIndex++]= ads1115.readADC_SingleEnded(0) ; //Guardamos valores para el array
    if (pHArrayIndex == ArrayLength){
      pHArrayIndex=0;
      voltage=(4.096/(pow(2,15)-1))*averageSample(&pHArray[0],40);
      pHValue = 3.5 * voltage + Offset;
      samplingTime = millis();
    }
  }
  //Mostramos en pantalla finalmente
 if (millis() - printTime > printInterval){
   Serial.print("Voltage:");
   Serial.print(voltage, 2);
   Serial.print("     pH value: ");
   Serial.println(pHValue, 2);
    printTime = millis();
 }

}
