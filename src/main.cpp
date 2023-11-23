#include <Arduino.h>
#include <rak3272_lib_p2p.h>
#include <FirebaseESP32.h>

//define pin use

//Credenciales de red WIFI
const char* ssid = "EduardoCB";
const char* password = "123456789";

// Credenciales Firebase
//#define FIREBASE_HOST "database-29ad3-default-rtdb.firebaseio.com"
//#define FIREBASE_AUTH "AIzaSyBIJ2KdgXFT5tqaaH0ebmG2qYDoDeuKxAo"
//#define USER_EMAIL "eduardo.caceres.b@uni.pe"
//#define USER_PASSWORD "20190151J"
#define DATABASE_URL "database-29ad3-default-rtdb.firebaseio.com"
#define API_KEY "AIzaSyBIJ2KdgXFT5tqaaH0ebmG2qYDoDeuKxAo"
#define USER_EMAIL "eduardo.caceres.b@uni.pe"
#define USER_PASSWORD "20190151J"

//variables
#define FREQ 915000000
#define SF 7
#define BW 0
#define CR 0
#define PREAMB_LEN 20
#define TX_PW 19
//variables estáticas



FirebaseData firebaseData;
FirebaseAuth auth;
FirebaseConfig config;
FirebaseData fbdo;

//TIMER VARIABLES
hw_timer_t *timer = NULL;
volatile bool flag = false;
volatile bool toggle = false;
const uint16_t interval = 4000; //en milisegundos


float Text;
float Hext;  
float Tint;    
float Hint;
float Vviento; 

float datarecuperada[5];

int AutomaticoAnterior=2, EncenderAnterior=2;

String message_LoRa = "", message_pc = "";
//String datarecuperada = "";
RAK3272 node1(FREQ, SF, BW, CR, PREAMB_LEN, TX_PW);

// put function declarations here:
String codificacion(float data[], int length);
void decodificacion(String hexData, float data[], int length); 
void IRAM_ATTR onTimer();
void decideandwriteMessage(int automa, int encend);
void waitMessage();
void enviarDatosFirebase();
int readAutomFirebase();
int readEncenFirebase(int autom);


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial2.begin(115200);
  Serial.println("Incializando...");

  timer = timerBegin(0,80,true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 1000*interval, true);
  timerAlarmEnable(timer);
  pinMode(2,OUTPUT);

  WiFi.mode(WIFI_STA); //Optional
  WiFi.begin(ssid, password);
  Serial.println("\nConnecting");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());

  config.api_key = API_KEY;

  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;


  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;
  Firebase.reconnectNetwork(true);

  fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);
  Firebase.begin(&config, &auth);

  //Firebase.begin(&config, &auth);
  // Espera a que Firebase se conecte
  //while (Firebase.ready() != 1) {
    //delay(100);
    //Serial.print(".");
  //}

  pinMode(26, OUTPUT);
  digitalWrite(26, HIGH);

  node1.inicialize();
  delay(500);
  node1.sendCommand("AT+PRECV=65533",true);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  if (flag==true) {
    timerStop(timer);
    //int autom = readAutomFirebase();
    //int encen = readEncenFirebase(autom);
    //decideandwriteMessage(autom, encen);
    //enviarDatosFirebase();
    int autom = readAutomFirebase();
    int encen = readEncenFirebase(autom);
    decideandwriteMessage(autom,encen);
    enviarDatosFirebase();

    digitalWrite(2, toggle? HIGH:LOW);
    
    flag=false;
    timerStart(timer);
    
  }
  waitMessage();
  //String mensaje = node1.receiveMessage();
  //decodificacion(mensaje, datarecuperada, 4);
}

// put function definitions here:


void IRAM_ATTR onTimer() {
  flag = true;
  toggle ^=true;
}


void decideandwriteMessage(int automa, int encend){

  if(automa!=AutomaticoAnterior || encend!=EncenderAnterior){
    //char buff[30];
    //sprintf(buff, "AUTO : %d | Encendido : %d",automa, encend);
    //String message = String(buff);
    float datosaenviar[2];
    datosaenviar[0] = automa;
    datosaenviar[1] = encend;
    
    //sendMessage(message);
    node1.sendMessage(codificacion(datosaenviar, 2));
    //Serial.print("Sending " + message);
    //Serial.println("  with messageid: "+msgCount);
    AutomaticoAnterior = automa;
    EncenderAnterior = encend;
  }
  
}


void enviarDatosFirebase() {
  
  Firebase.setFloat(firebaseData,"vdetonantes/Text", Text);
  Firebase.setFloat(firebaseData,"vdetonantes/Hext", Hext);
  Firebase.setFloat(firebaseData,"vdetonantes/Tint", Tint);
  Firebase.setFloat(firebaseData,"vdetonantes/Hint", Hint);
  Firebase.setFloat(firebaseData,"vdetonantes/viento", Vviento);
  
}

int readAutomFirebase(){
  bool aux = Firebase.getBool(firebaseData, "vdetonantes/selector");
  if (firebaseData.boolData()) {
    return 1;
  } else {
    return 0;
  }
}

int readEncenFirebase(int autom){
  if(autom == 1){
    return 0;
  }
  else{
    bool aux = Firebase.getBool(firebaseData, "vdetonantes/encender");
      if (firebaseData.boolData()) {
        return 1;
      } else {
        return 0;
      }
  }
}

String codificacion(float data[], int length) {
  //String pre = "AT+SEND=2:";
  //String post = "\r\n";
  String payload = "";

  for (int i = 0; i < length; i++) {
    // Aplica la conversión a cada elemento en la lista
    int intValue = (int)(data[i] * 10) + 656;
    String hexStr = String(intValue, HEX);
    String nox = hexStr.substring(0);

    if (nox.length() % 2 != 0) {
      // Añade un 0 a la izquierda si la longitud no es par
      nox = "0" + nox;
    }

    payload += nox;
  }

  //String code = pre + payload + post;
  return payload;
}

void decodificacion(String hexData, float data[], int length) {
  // Asegúrate de que la longitud de la cadena hexData sea suficiente
  /**/
  int posInicial = hexData.indexOf(':');
  int posFinal = hexData.indexOf(':', posInicial + 1);
  posFinal = hexData.indexOf(':', posFinal + 1);
  posFinal = hexData.indexOf(':', posFinal + 1);

  // Verificar si se encontraron tres ':' en la cadena
  if (posFinal != -1) {
    // Obtener la subcadena a partir del tercer ':'
    String resultado = hexData.substring(posFinal + 1);
    
    // Imprimir el resultado
    Serial.println("Resultado: " + resultado);
    if (resultado.length() < length * 4) {
      Serial.println("La longitud de la cadena hexData es insuficiente.");
    }

  // Recorre la cadena hexData y convierte los datos hexadecimales a decimales
    for (int i = 0; i < length; i++) {
      String hexValue = resultado.substring(i * 4, (i + 1) * 4);
      long intValue = strtol(hexValue.c_str(), NULL, 16);
      data[i] = (float)(intValue - 656) / 10.0;
    }


  } else {
    Serial.println("No se encontraron cuatro ':' en la cadena");
    return;
  }

  
}

void waitMessage(){
  String message_LoRa=node1.receiveMessage();
  float datarec[2];
  if(message_LoRa != ""){
    Serial.println(message_LoRa);
    decodificacion(message_LoRa,datarecuperada,4);
    Serial.println("La data decodificada es:");
    Text = datarecuperada[0];
    Hext = datarecuperada[1];
    Tint = datarecuperada[2];
    Hint = datarecuperada[3];
    Vviento = 4.3;
    Serial.print(String(datarecuperada[0])+" ");
    Serial.print(String(datarecuperada[1])+" ");
    Serial.print(String(datarecuperada[2])+" ");
    Serial.print(String(datarecuperada[3])+" ");

    message_LoRa="";
  }
  
}