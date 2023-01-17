//A0 Humedad, A1 PH, A2 Temperatura, A3 Iluminación, ADC Salinidad
//#define power_pin 5 //Pin alimentación Salinidad
#include <Adafruit_ADS1X15.h> // Librería del ADS1115
#include <Wire.h> // Librería para usar el BUS I2C
#include <math.h>
#include <ESP8266WiFi.h> //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
//tension del sensor pH 3,3 a 5,5v
#define channelValue 0
#define Offset +2.25 //Definimos el Offset
#define samplingInterval 20
#define printInterval 800
#define ArrayLength 40 

/* Programa prueba GPS UBLOX-6M
 *  Conexiones GPIO 12--> TX
 *             GPIO 13--> RX
 */

#include <TinyGPS++.h> //Libreria TinyGPS

#include <SoftwareSerial.h> //Libreria para el UART del GPS
SoftwareSerial gps(12,13);

//definiciones wifi

// Comentar/Descomentar para ver mensajes de depuracion en monitor serie y/o respuesta del HTTP server
#define PRINT_DEBUG_MESSAGES
//#define PRINT_HTTP_RESPONSE

// Comentar/Descomentar para conexion Fuera/Dentro de UPV
#define WiFi_CONNECTION_UPV

// Selecciona que servidor REST quieres utilizar entre ThingSpeak y Dweet
#define REST_SERVER_THINGSPEAK //Selecciona tu canal para ver los datos en la web (https://thingspeak.com/channels/360979)
//#define REST_SERVER_DWEET //Selecciona tu canal para ver los datos en la web (http://dweet.io/follow/PruebaGTI)

///////////////////////////////////////////////////////
/////////////// WiFi Definitions /////////////////////
//////////////////////////////////////////////////////

#ifdef WiFi_CONNECTION_UPV //Conexion UPV
  const char WiFiSSID[] = "GTI1";
  const char WiFiPSK[] = "1PV.arduino.Toledo";
#else //Conexion fuera de la UPV
  const char WiFiSSID[] = "MySSID";
  const char WiFiPSK[] = "MyPassWord";
#endif



///////////////////////////////////////////////////////
/////////////// SERVER Definitions /////////////////////
//////////////////////////////////////////////////////

#if defined(WiFi_CONNECTION_UPV) //Conexion UPV
  const char Server_Host[] = "proxy.upv.es";
  const int Server_HttpPort = 8080;
#elif defined(REST_SERVER_THINGSPEAK) //Conexion fuera de la UPV
  const char Server_Host[] = "api.thingspeak.com";
  const int Server_HttpPort = 80;
#else
  const char Server_Host[] = "dweet.io";
  const int Server_HttpPort = 80;
#endif

WiFiClient client;

///////////////////////////////////////////////////////
/////////////// HTTP REST Connection ////////////////
//////////////////////////////////////////////////////

#ifdef REST_SERVER_THINGSPEAK 
  const char Rest_Host[] = "api.thingspeak.com";
  String MyWriteAPIKey="JGX3DIDYPGJG47I1"; // Escribe la clave de tu canal ThingSpeak
#else 
  const char Rest_Host[] = "dweet.io";
  String MyWriteAPIKey="12345pruebaclase1"; // Escribe la clave de tu canal Dweet
#endif

#define NUM_FIELDS_TO_SEND 5 //Numero de medidas a enviar al servidor REST (Entre 1 y 8)

/////////////////////////////////////////////////////
/////////////// Pin Definitions ////////////////
//////////////////////////////////////////////////////

const int LED_PIN = 5; // Thing's onboard, green LED

//acaba definiciones wifi

int pHArray[ArrayLength]; //Array para toma de valores
int pHArrayIndex = 0; //Indice de incremento lista
int N=0;
//Temperatura:
int16_t adc_2;
int16_t adc0;
int16_t adc;
  int16_t humedad; //Inicializa Humedad
  int16_t salinidad; 
int temperaturaArray[20]; //Array de valores a hacer media de temperatura

//Humedad:
int Seco = 0;  // Valor en seco
int Mojado = 0;  // Valor en agua
   
//Salinidad:
int AguaNormal = 0; // Agua destilada
int AguaSalada = 0; // Agua salada
int power_pin = 5; // Alimentación de la sonda

double gramos = 0; //Gramos sal

//GPS
char dato=' ';

//Construir ADS1115
  
  Adafruit_ADS1115 ads1115;


//---------------------------------------------

//voids de WIFI

/////////////////////////////////////////////////////
/////////////// WiFi Connection ////////////////
//////////////////////////////////////////////////////

void connectWiFi()
{
  byte ledStatus = LOW;

  #ifdef PRINT_DEBUG_MESSAGES
    Serial.print("MAC: ");
    Serial.println(WiFi.macAddress());
  #endif
  
  WiFi.begin(WiFiSSID, WiFiPSK);

  while (WiFi.status() != WL_CONNECTED)
  {
    // Blink the LED
    digitalWrite(LED_PIN, ledStatus); // Write LED high/low
    ledStatus = (ledStatus == HIGH) ? LOW : HIGH;
    #ifdef PRINT_DEBUG_MESSAGES
       Serial.println(".");
    #endif
    delay(500);
  }
  #ifdef PRINT_DEBUG_MESSAGES
     Serial.println( "WiFi Connected" );
     Serial.println(WiFi.localIP()); // Print the IP address
  #endif
}

/////////////////////////////////////////////////////
/////////////// HTTP POST  ThingSpeak////////////////
//////////////////////////////////////////////////////

void HTTPPost(String fieldData[], int numFields){

// Esta funcion construye el string de datos a enviar a ThingSpeak mediante el metodo HTTP POST
// La funcion envia "numFields" datos, del array fieldData.
// Asegurate de ajustar numFields al número adecuado de datos que necesitas enviar y activa los campos en tu canal web
  
    if (client.connect( Server_Host , Server_HttpPort )){
       
        // Construimos el string de datos. Si tienes multiples campos asegurate de no pasarte de 1440 caracteres
   
        String PostData= "api_key=" + MyWriteAPIKey ;
        for ( int field = 1; field < (numFields + 1); field++ ){
            PostData += "&field" + String( field ) + "=" + fieldData[ field ];
        }     
        
        // POST data via HTTP
        #ifdef PRINT_DEBUG_MESSAGES
            Serial.println( "Connecting to ThingSpeak for update..." );
        #endif
        client.println( "POST http://" + String(Rest_Host) + "/update HTTP/1.1" );
        client.println( "Host: " + String(Rest_Host) );
        client.println( "Connection: close" );
        client.println( "Content-Type: application/x-www-form-urlencoded" );
        client.println( "Content-Length: " + String( PostData.length() ) );
        client.println();
        client.println( PostData );
        #ifdef PRINT_DEBUG_MESSAGES
            Serial.println( PostData );
            Serial.println();
            //Para ver la respuesta del servidor
            #ifdef PRINT_HTTP_RESPONSE
              delay(500);
              Serial.println();
              while(client.available()){String line = client.readStringUntil('\r');Serial.print(line); }
              Serial.println();
              Serial.println();
            #endif
        #endif
    }
}

////////////////////////////////////////////////////
/////////////// HTTP GET  ////////////////
//////////////////////////////////////////////////////

void HTTPGet(String fieldData[], int numFields){
  
// Esta funcion construye el string de datos a enviar a ThingSpeak o Dweet mediante el metodo HTTP GET
// La funcion envia "numFields" datos, del array fieldData.
// Asegurate de ajustar "numFields" al número adecuado de datos que necesitas enviar y activa los campos en tu canal web
  
    if (client.connect( Server_Host , Server_HttpPort )){
           #ifdef REST_SERVER_THINGSPEAK 
              String PostData= "GET https://api.thingspeak.com/update?api_key=";
              PostData= PostData + MyWriteAPIKey ;
           #else 
              String PostData= "GET http://dweet.io/dweet/for/";
              PostData= PostData + MyWriteAPIKey +"?" ;
           #endif
           
           for ( int field = 1; field < (numFields + 1); field++ ){
              PostData += "&field" + String( field ) + "=" + fieldData[ field ];
           }
          
           
           #ifdef PRINT_DEBUG_MESSAGES
              Serial.println( "Connecting to Server for update..." );
           #endif
           client.print(PostData);         
           client.println(" HTTP/1.1");
           client.println("Host: " + String(Rest_Host)); 
           client.println("Connection: close");
           client.println();
           #ifdef PRINT_DEBUG_MESSAGES
              Serial.println( PostData );
              Serial.println();
              //Para ver la respuesta del servidor
              #ifdef PRINT_HTTP_RESPONSE
                delay(500);
                Serial.println();
                while(client.available()){String line = client.readStringUntil('\r');Serial.print(line); }
                Serial.println();
                Serial.println();
              #endif
           #endif  
    }
}


//acaban voids de WIFI

void setup() {

   
  //Inicializamos el Monitor Serie
    //inicializamos WIFI
  #ifdef PRINT_DEBUG_MESSAGES
    Serial.begin(115200);
  #endif
  
  connectWiFi();
  digitalWrite(LED_PIN, HIGH);

  #ifdef PRINT_DEBUG_MESSAGES
      Serial.print("Server_Host: ");
      Serial.println(Server_Host);
      Serial.print("Port: ");
      Serial.println(String( Server_HttpPort ));
      Serial.print("Server_Rest: ");
      Serial.println(Rest_Host);
  #endif
  //acabamos de inicializar wifi

  delay(1000);
  Serial.println("ON");
  pinMode(power_pin, OUTPUT); // Pin de salida
  Serial.println("Voltaje ADS1115 +/- 4.096v");
  Serial.println("1 bit = 2mV");
  Serial.println("Inicializando el medidor de pH");
  delay(1000);
  Serial.println("Inicializando el sensor de Temperatura");

//GPS           
 gps.begin(9600); 
 Serial.println("Inicializando el GPS...");
 delay(500);
 Serial.println("Esperando datos");
//GPS

  //Inicializamos el ADS1115
   ads1115.begin(); //Salinidad ADC 
   ads1115.begin(0x48); //Sensores ADS1115

  //Configuramos la ganancia del ADS1115
  ads1115.setGain(GAIN_ONE);
}

//Funcion completa para sal y humedad
int salyhumedad(int pin1,int pinADC,int &humedad){ //Entero,Entero---->salyhumedad()---->Entero,Entero
  if(N==0) //N siendo una variable que "cuenta"
  {
  delay(2000);
  digitalWrite(power_pin, HIGH);
  delay(100);

  Serial.println("Comienza la calibración. Atención a las instrucciones:");
  Serial.println(" ");
  delay(1500); 

// Calibrar sensor de Humedad, no solo el nuestro único, sino cualquiera 
//(en el nuestro cuanta más humedad más valor digital, en el de clase cuanta más humedad, menor valor digital)
  Serial.println("Humedad:");
  delay(2000);
  Serial.println(" ");
  Serial.print("Calibrando sensor de Humedad en ambiente seco...");
  for(int i=0; i<=6; i++)
    {
       delay(2500);
       Serial.print(".");
    }
  adc0 = ads1115.readADC_SingleEnded(pin1);
  Serial.println(" ");
  Serial.println(adc0);
  Seco = adc0; 
  Serial.println("Valor guardado.");
  delay(3500);

  Serial.println(" ");
  Serial.print("Ahora moje el sensor en un recipiente con agua.");
  for(int i=0; i<=6; i++)
    {
       delay(2500);
       Serial.print(".");
    }
  adc0 = ads1115.readADC_SingleEnded(pin1);
  Serial.println(" ");
  Serial.println(adc0);
  Mojado=adc0; 
  Serial.println("Calibrado.");
  Serial.println(" ");
  delay(6000);

// Calibrar sensor Salinidad
  Serial.println("Salinidad:");
  Serial.println(" ");
  delay(3500);
  Serial.print("Introduzca el sensor en agua limpia.");
  for(int i=0; i<=6; i++)
    {
       delay(2000);
       Serial.print(".");
    }
  digitalWrite( power_pin, HIGH );
  delay(500);
  adc = analogRead(pinADC); //adc del sparkfun
  digitalWrite( power_pin, LOW );
  Serial.println(" ");
  Serial.println(adc);
  AguaNormal=adc; 
  Serial.println("Valor guardado.");
  delay(3500);

  Serial.println(" ");
  Serial.print("Introduzca el sensor de salinidad en agua con sal máxima.");
  for(int i=0; i<=6; i++)
    {
       delay(2000);
       Serial.print(".");
    }
  digitalWrite( power_pin, HIGH );
  delay(500);
  adc = analogRead(pinADC);
  digitalWrite( power_pin, LOW );
  Serial.println(" ");
  Serial.println(adc);
  AguaSalada=adc; 
  Serial.println("Calibrado.");
  delay(3500);

// Mensaje de OK
  Serial.println(" ");
  Serial.println(" Se han calibrado los sensores con éxito. El programa se adaptará a estos...");
  Serial.println(" ");
  N=1;
  digitalWrite(power_pin, LOW);
  }

//Imprimir Humedad y Salinidad

digitalWrite(power_pin, HIGH);
  delay(200);
  
  adc0 = ads1115.readADC_SingleEnded(pin1);
  adc = analogRead(pinADC);
  digitalWrite(power_pin, LOW);
  humedad = 100*Seco/(Seco-Mojado)-adc0*100/(Seco-Mojado); //% de humedad
  salinidad = 100*AguaNormal/(AguaNormal-AguaSalada)-adc*100/(AguaNormal-AguaSalada); //% de salinidad

 if(humedad>=100){
    humedad=100; //que no sobrepase el 100%
 }
 else if(humedad<=0){
    humedad=0;  //que no baje del 0%
 }
 else if(salinidad>=100){
    salinidad=100; //que no sobrepase el 100%
 }
 else if(salinidad<0){
  salinidad=0;
  }
 digitalWrite( power_pin, LOW );
 
// Lo que aparecerá en el Monitor Serie:

  Serial.println(" ");
  Serial.print("Humedad: ");
  Serial.println(adc0);
  Serial.print(" ");
  Serial.print(humedad);
  Serial.println("%");

  Serial.print("Salinidad: ");
  Serial.println(adc);
  Serial.print(" ");
  Serial.print(salinidad);
  Serial.println("%");
  //Serial.print("Gramos de Sal: ");
  //Serial.println(gramos);
  delay(500);
return salinidad;
}

//Función sensor de temperatura
double temperatura(int pin){
   double m = (34.6*(pow(10,-3)));
  double b = (0.789);
  
  //Captura una muestra del ADS1115
  int16_t adc_2 = ads1115.readADC_SingleEnded(pin);

  for(int i=0;i<=4;i++){ //Toma de 5 valores
  temperaturaArray[i]=ads1115.readADC_SingleEnded(pin);
  delay(200);
  }//for()

  adc_2 = averageSample(&temperaturaArray[0],5);//Valor del average sample a partir de la lista
  
  //formula dependiendo de la lectura digital del sensor de temperatura
  double temp = ((4.096/(pow(2,15)))*adc_2-b)*(1/m);
  Serial.print("Sensor de Temperatura: ");
  Serial.println(adc_2);
  Serial.print(" ");
  Serial.print(temp);
  Serial.println(" ºC");
  delay(2000);
  for(int i=0;i<=5;i++){
  Serial.println(".");
  delay(500);
  }
  return temp;
}

//Funcion ph
float phFuncion(int pin){
  static unsigned long samplingTime = millis();
  static unsigned long printTime = millis();
  static float pHValue,voltage;
  for(int i=0;i<=2000;i++){
  if (millis() - samplingTime > samplingInterval){
    pHArray[pHArrayIndex++]= ads1115.readADC_SingleEnded(pin) ; //Guardamos valores para el array
    if (pHArrayIndex == ArrayLength){
      pHArrayIndex=0;
      voltage=(4.096/(pow(2,15)-1))*averageSample(&pHArray[0],40);
      pHValue = 3.5 * voltage + Offset;
      samplingTime = millis();
    }
  }
  //Mostramos en pantalla finalmente
 if (millis() - printTime > printInterval){
   Serial.print("Voltaje: ");
   Serial.print(voltage, 2);
   Serial.println(" Valor pH: ");
   Serial.print(pHValue, 2);
    printTime = millis();
 }
  }//for()

  for(int i=0;i<=5;i++){
  Serial.println(".");
  delay(100);
  }
  return pHValue;
}

//Funcion Puntero para varios valores y hacer media
double averageSample(int *p,int cuantos){
  double x=0;
  for(int i=0;i<=cuantos-1;i++){
    x+=p[i];
  }
  return (x/cuantos);
}

//Iluminosidad:
int  val = 0;

// Real----->lightReading()
double lightReading(int pin){ //Funcion de intervalos
   int val = ads1115.readADC_SingleEnded(pin);
   double valF;
   if(val<3){
        val=3;
      }
      if(val<=21 && val>=3){
       Serial.println("Sensor Tapado/obstruido");
      }
      else if(val<=450 && val>21){
       Serial.println("Sombreado/Nubes");
      }
      else if(val>450 && val<2100){
        Serial.println("Soleado");
       }
      else if(val>=2100){
        Serial.println("Demasiada luz para el huerto");
      }
      valF=(4.096*10/32767)*val;
      Serial.println(valF);
       delay(2000);
     return valF;
     }

void internetH1(int humedad, int salinidad, double temp,float pHValue, double valF){

//void loop WIFI
    String data[ NUM_FIELDS_TO_SEND + 1];  // Podemos enviar hasta 8 datos

    
    data[ 1 ] = String( humedad ); //Escribimos el dato 1. Recuerda actualizar numFields
    #ifdef PRINT_DEBUG_MESSAGES
        Serial.print( "Humedad = " );
        Serial.println( data[ 1 ] );
    #endif

    data[ 2 ] = String( salinidad ); //Escribimos el dato 2. Recuerda actualizar numFields
    #ifdef PRINT_DEBUG_MESSAGES
        Serial.print( "Salinidad = " );
        Serial.println( data[ 2 ] );
    #endif
    data[ 3 ] = String( temp ); //Escribimos el dato 2. Recuerda actualizar numFields
    #ifdef PRINT_DEBUG_MESSAGES
        Serial.print( "Temperatura = " );
        Serial.println( data[ 3 ] );
    #endif
    data[ 4 ] = String( pHValue ); //Escribimos el dato 2. Recuerda actualizar numFields
    #ifdef PRINT_DEBUG_MESSAGES
        Serial.print( "Ph = " );
        Serial.println( data[ 4 ] );
    #endif
    data[ 5 ] = String( valF ); //Escribimos el dato 2. Recuerda actualizar numFields
    #ifdef PRINT_DEBUG_MESSAGES
        Serial.print( "Iluminación = " );
        Serial.println( data[ 5 ] );
    #endif

    //Selecciona si quieres enviar con GET(ThingSpeak o Dweet) o con POST(ThingSpeak)
    //HTTPPost( data, NUM_FIELDS_TO_SEND );
    HTTPGet( data, NUM_FIELDS_TO_SEND );

    //Selecciona si quieres un retardo de 15seg para hacer pruebas o dormir el SparkFun
    delay( 15000 );   
    //Serial.print( "Goodnight" );
    //ESP.deepSleep( sleepTimeSeconds * 1000000 );
  
}

void gpsFunc(){//Función para mostrar las coordenadas
  for(int i=0;i<=70;i++){
    if(gps.available())
      dato=gps.read();
      if(dato=='$'){
        Serial.print("\n");
      }
    Serial.print(dato);
    delay(50);
    }
  }//gps()

void loop(void) {
int humedad=0;
// Calibrado (para que el programa tenga un uso más genérico y no único al sensor que calibramos una vez)
int salinidad=salyhumedad(0,A0,humedad);//Devuelve la humedad por un Reference int & humedad

//variables calculo ahora Temperatura
double temp = temperatura(2);

//pH
float pHValue = phFuncion(1);

double valF = lightReading(3); //Función sensor iluminosidad,PIN 3 INT

gpsFunc();

internetH1(humedad, salinidad, temp, pHValue, valF);    

}
