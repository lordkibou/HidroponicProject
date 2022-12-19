//A0 Humedad, A1 PH, A2 Temperatura, A3 Iluminación, ADC Salinidad
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
int N=0;
//Temperatura:
int16_t adc_2;
int temperaturaArray[20]; //Array de valores a hacer media de temperatura

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
  delay(1000);
  Serial.println("Inicializando el medidor de pH");
  delay(1000);
  Serial.println("Inicializando el sensor de Temperatura");

  //Inicializamos el ADS1115
   ads1115.begin(0x48);

  //Configuramos la ganancia del ADS1115
  ads1115.setGain(GAIN_ONE);
}

void loop(void) {
//variables calculo ahora Temperatura
  double m = (34.6*(pow(10,-3)));
  double b = (0.789);
  
  //Captura una muestra del ADS1115
  int16_t adc_2 = ads1115.readADC_SingleEnded(2);

  for(int i=0;i<=4;i++){ //Toma de 5 valores
  temperaturaArray[i]=ads1115.readADC_SingleEnded(2);
  delay(200);
  }//for()

  adc_2 = averageSample(&temperaturaArray[0],5);//Valor del average sample a partir de la lista
  
  //formula dependiendo de la lectura digital del sensor de temperatura
  double temp = ((4.096/(pow(2,15)))*adc_2-b)*(1/m);
  Serial.println("Lectura Digital Temperatura: ");
  Serial.println(adc_2);
  Serial.println("Temperatura: ");
  Serial.println(temp);
  delay(2000);
  for(int i=0;i<=5;i++){
  Serial.println(".");
  delay(500);
  }

  //Temp
  static unsigned long samplingTime = millis();
  static unsigned long printTime = millis();
  static float pHValue,voltage;
  for(int i=0;i<=2000;i++){
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
  }//for()

  for(int i=0;i<=5;i++){
  Serial.println(".");
  delay(500);
  }
}
