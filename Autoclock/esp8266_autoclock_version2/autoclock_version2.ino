#include <ESP8266WiFi.h>      // Biblioteca para Wifi
#include <ESP8266WebServer.h> // Bibliteca Para Servidor
#include <LiquidCrystal.h>    // Biblioteca para o Display
#include <NTPClient.h>        // Biblioteca para NTP
#include <WiFiUdp.h>          // Biblioteca para uso de UDP
#include <Wire.h>             // Biblioteca I2C
#include <RTClib.h>           // Biblioteca do Real Time CLock
#include <FS.h>               // Sistema de Arquivos
#include <ArduinoJson.h>      // Interpretação de Arquivos JSON
#include <string.h>


// Nomeclatura da Máquina de Estados
#define wifiConnectState 1
#define stacionModeState 2
#define accessPointModeState 3
#define NTPReadState 4
#define setRTCState 5
#define getRTCState 6
#define displayTimeState 7
#define idle1SecState 8
#define stopState 9


// Variáveis de Estado
int currentState = wifiConnectState;
int nextState = wifiConnectState;
bool firstNTPRead = true;
bool rtcSeted = false;


// Váriaveis da Rede que Será Gerada Pelo ESP8266
ESP8266WebServer server;
char* clockPassword = "password";
char* clockSsid = "AutoClock";


// Informações da Rede Gerada Pelo ESP8266
IPAddress local_ip(192,168,0,1);
IPAddress gateway(192,168,0,1);
IPAddress netmask(255,255,255,0);


// WebPage
char webpage[] PROGMEM = R"=====(
<html>
<head>
</head>
<body>
  <form>
    <fieldset>
      <div>
        <label for="ssid">SSID</label>      
        <input value="" id="ssid" placeholder="SSID" style="margin-left: 56px;">
      </div>
      <div>
        <label for="password">PASSWORD</label>
        <input type="password" value="" id="password" placeholder="PASSWORD">
      </div>
      <div>
        <button class="primary" id="savebtn" type="button" onclick="myFunction()">SAVE</button>
      </div>
    </fieldset>
  </form>
</body>
<script>
function myFunction()
{
  console.log("button was clicked!");
  var ssid = document.getElementById("ssid").value;
  var password = document.getElementById("password").value;
  var data = {ssid:ssid, password:password};
  var xhr = new XMLHttpRequest();
  var url = "/settings";
  xhr.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      // Typical action to be performed when the document is ready:
      if(xhr.responseText != null){
        console.log(xhr.responseText);
      }
    }
  };
  xhr.open("POST", url, true);
  xhr.send(JSON.stringify(data));
};
</script>
</html>
)=====";


// Informações da rede a ser conectada
char ssid[20] = "";
char password[20] = "";


int dayOfWeek;
 

// Fuso Horário de São Paulo, horário de Brasília 
// float timeZone = -3;
// int offset = timeZone*3600


// Inicia objecto para uso de UDP
WiFiUDP ntpUDP;


// Váriaveis do cliente NTP
char ntpServer[40];                               // Servidor ntp
int offset = -3 * 3600;                           // Deslocamento do fuso horário exemplo São Paulo (UTC - 3:00) -> -3 * 3600


// Define cliente NTP Client para pegar tempo
// Objeto responsável por recuperar dados sobre horário
NTPClient timeClient(
    ntpUDP,           //Socket UDP
    ntpServer,        //NTP Server
    offset,           //Deslocamento do horário em relacão ao GMT 0
    60000);     

// Nomes dos dias da semana
char* dayOfWeekNames[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};


// Variáveis para salvar data e horário
String formattedDate;
String dayStamp;
String timeStamp;


// Variáveis do LCD
const int RS = D3, EN = D4, d4 = D5, d5 = D6, d6 = D7, d7 = D8;
LiquidCrystal lcd(RS,EN,d4,d5,d6,d7);


// Objeto do módulo RTC
RTC_DS3231 rtc;


// Váriaveis de data e horário
int year;
int month;
int day;
int hour;
int minute;
int second;

int nowYear;
int nowMonth;
int nowDay;
int nowHour;
int nowMinute;
int nowSecond;


// Variáveis do Timer de 15 Minutos
unsigned long currentMillis  = 0;                // Armazena o instante atual do processamento
unsigned long previousMillis = 0;                // Armazena o tempo anterior do processamento
const long requestInterval15Min = 900000;        // intervalo de 15 minutos em milisegundos
bool passed15min = false;                        // Passou 15 minutos
bool passedOneHour = false;                      // Passou uma hora


void startSerial() {
  // Inicialização da Comunicação Serial
  Serial.begin(115200);
  Serial.println("Serial Started!");  
}


void startSPIFFS() {
  // Inicialização do Sistema de Arquivos
  SPIFFS.begin();
  Serial.println("File System Started!");
}


void startLCD() {
  //Incialização do LCD
  lcd.begin(16,2);
  lcd.clear();
  Serial.println("LCD Started!");
}


void startI2C() {
  // Inicialização da Comunicação I2C
  Wire.begin(D1,D2);
  Serial.println("I2C Started!");
}


void startRTC() {
  // Inicializa o Módulo RTC
  bool started = 0;
  started = rtc.begin();
  if (started)
    Serial.println("RTC DS3231 Module Started!");
  else
    Serial.println("RTC DS3231 Module NOT Started!");
}


void startServer() {
  server.on("/",[](){server.send_P(200,"text/html", webpage);});
  server.on("/settings", HTTP_POST, handleSettingsUpdate);

  server.begin();
}


void handleSettingsUpdate()
{
  String data = server.arg("plain");
  DynamicJsonBuffer jBuffer;
  JsonObject& jObject = jBuffer.parseObject(data);

  File configFile = SPIFFS.open("/config.json", "w");
  jObject.printTo(configFile);  
  configFile.close();
  
  server.send(200, "application/json", "{\"status\" : \"ok\"}");
  delay(500);
  
  nextState = wifiConnectState;
}


bool past15Min() {
  if (currentMillis - previousMillis >= requestInterval15Min) {
    previousMillis = currentMillis;
    return 1;
  } else {
    return 0;
  }
}


// Contador de uma hora
int counterOneHour = 0;

bool pastOneHour(bool passed15min) {
  if (passed15min) {
    counterOneHour++;
  }

  if (counterOneHour == 4) {
    counterOneHour = 0;
    return true;
  } else {
    return false;
  }
}


int stateWifiConnect() {
  // Reseta a Rede
  WiFi.softAPdisconnect(true);
  WiFi.disconnect();          
  delay(1000);

  // Checa se a credencias armazenadas
  if(loadData()) {
    nextState =  stacionModeState;
  } else {
    nextState = accessPointModeState;
  }

}


bool loadData() {
    if(SPIFFS.exists("/config.json")){
    File configFile = SPIFFS.open("/config.json", "r");
    if(configFile){
      size_t size = configFile.size();
      std::unique_ptr<char[]> buf(new char[size]);
      configFile.readBytes(buf.get(), size);
      configFile.close();

     DynamicJsonBuffer jsonBuffer;
     JsonObject& jObject = jsonBuffer.parseObject(buf.get());
     const char * _ssid = "", *_pass = "";
     if(jObject.success()) {

        _ssid = jObject["ssid"];
        _pass = jObject["password"];
        strcpy(ssid,_ssid);
        strcpy(password,_pass);
        
        return true;

      } else {

        return false;
        
      }
    }
  } else {

    return false;

  }
}


int stateStacionMode() {

  if (startStacionMode()) {
    Serial.println("Stacion Mode Success!");
    WiFi.printDiag(Serial);

    return NTPReadState;

  } else {

    return accessPointModeState;

  }
}


bool startStacionMode() {
  // Seta Stacion Mode
  WiFi.mode(WIFI_STA);

  // Conecta a Rede Wifi
  Serial.print("Connecting to ");
  Serial.println(ssid);

  if (!rtcSeted) {
    lcd.clear();
    lcd.print("Connecting");
  }
  
  WiFi.begin(ssid, password);

  // Marca inicio do timer de conexão
  unsigned long startTime = millis();
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");

    if (!rtcSeted) {
      lcd.print(".");
    }

    if ((unsigned long) (millis() - startTime >= 5000)) {
      break;
    }

  }


   if (WiFi.status() == WL_CONNECTED) {

    // Imnprime Endereço Local de IP e Inicia o Servidor Web
    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    lcd.clear();
    lcd.print("IP address:");
    lcd.setCursor(0,1);
    lcd.print(WiFi.localIP());
    delay(6000);
    lcd.clear();

    return true;

  } else {

    return false;

  }
  
}


int stateAccessPointMode() {
  
  WiFi.mode(WIFI_AP);   // Seta Access Point Mode
  WiFi.softAPConfig(local_ip, gateway, netmask);
  WiFi.softAP(clockSsid, clockPassword);
  WiFi.printDiag(Serial);

  if (rtcSeted) {
    return getRTCState;
  }

  lcd.clear();
  lcd.print("Waiting Setup!");

  return stopState;
}


int stateNTPRead() {
  if (firstNTPRead) {
    readNTP();
    firstNTPRead = false;  
  }

  if (WiFi.status() == WL_CONNECTED)
    readNTP();
    
  return setRTCState;
}


void readNTP() {
  // Inicializa Cliente NTP
  startNTP();     

  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }
  
  // O formattedDate tem o seguinte formato:
  // 2018-05-28T16:00:13Z
  // Precisamos extrair data e horario
  
  formattedDate = timeClient.getFormattedDate(); // Pega data no formato comentado acima

  dayOfWeek = timeClient.getDay();               // Pega o numero do dia da semana para ser usado no vetor dayOfWeekNames[]

  // Extraindo data
  int splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);
  
  // Extraindo horario
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);

  splitDayAndTimeStamps();                      // Converte caracteres das strings "dayStamp" e "timeStamp" para números inteiros
}


void startNTP() {
  // Para o Cliente NTP
  timeClient.end();

  // Configura o Cliente NTP
  strcpy(ntpServer, "south-america.pool.ntp.org");

  // Inicialização do Cliente NTP para obter Tempo Universal Coordenado
  timeClient.begin();
  Serial.println("NTP Client Started!");
}


void splitDayAndTimeStamps() {
  // Converte caracteres das strings "dayStamp" e "timeStamp" para números inteiros
  year = ((dayStamp[2] - 48) * 10) + (dayStamp[3] - 48);
  month = ((dayStamp[5] - 48) * 10) + (dayStamp[6] - 48);
  day = ((dayStamp[8] - 48) * 10) + (dayStamp[9] - 48);
  hour = ((timeStamp[0] - 48) * 10) + (timeStamp[1] - 48);
  minute = ((timeStamp[3] - 48) * 10) + (timeStamp[4] - 48);
  second = ((timeStamp[6] - 48) * 10) + (timeStamp[7] - 48);
}


int stateSetRTC() {
  rtc.adjust(DateTime(year,month,day,hour,minute,second)); // Ajusta o Horário no módulo RTC  
  rtcSeted = true;
  return getRTCState;
}


int stateGetRTC() {
  DateTime nowDateTime = rtc.now();
    
  dayOfWeek = nowDateTime.dayOfTheWeek();
  
  nowYear = nowDateTime.year();
  nowMonth = nowDateTime.month();
  nowDay = nowDateTime.day();
  nowHour = nowDateTime.hour();
  nowMinute = nowDateTime.minute();
  nowSecond = nowDateTime.second();

  return displayTimeState;
}


int stateDisplayTime() {
  
  // lcd.clear();
  
  lcd.setCursor(4,0);
  if (nowHour < 10) {
    lcd.print("0");
    lcd.print(nowHour);
  } else {
    lcd.print(nowHour);
  }
  
  lcd.print(":");

  if (nowMinute < 10) {
    lcd.print("0");
    lcd.print(nowMinute);
  } else {
    lcd.print(nowMinute);
  }

  lcd.print(":");

  if (nowSecond < 10) {
    lcd.print("0");
    lcd.print(nowSecond);
  } else {
    lcd.print(nowSecond);
  }

  lcd.setCursor(3,1);

  if (nowDay < 10) {
    lcd.print("0");
    lcd.print(nowDay);
  } else {
    lcd.print(nowDay);
  }

  lcd.print("/");

  if (nowMonth < 10) {
    lcd.print("0");
    lcd.print(nowMonth);
  } else {
    lcd.print(nowMonth);
  }

  lcd.print("/");

  lcd.print(nowYear);

  return idle1SecState;
}



void setup() {

  startSerial();  // Inicia comunicação serial
  startSPIFFS();  // Inicia o sistema de arquivos
  startLCD();     // Inicia modulo display LCD
  startI2C();     // Inicializa Comunicação I2C
  startRTC();     // Inicializa o Módulo Real Time Clock
  startServer();  // Inicializa o Servidor

}



void loop() {

  currentMillis = millis();
  passed15min = past15Min();
  passedOneHour = pastOneHour(passed15min);
  
  if (passed15min && (WiFi.status() == WL_CONNECTED)) {
    nextState = NTPReadState;
  }

  if (passedOneHour && (WiFi.status() != WL_CONNECTED)) {
    nextState = wifiConnectState;
  }
  
  currentState = nextState;

  switch (currentState) {
    
    case wifiConnectState:
      nextState = stateWifiConnect();
      break;
    case stacionModeState:
      nextState = stateStacionMode();
      break;
    case accessPointModeState:
      nextState = stateAccessPointMode();
      break;
    case NTPReadState:
      nextState = stateNTPRead();
      break;
    case setRTCState:
      nextState = stateSetRTC();
      break;
    case getRTCState:
      nextState = stateGetRTC();
      break;
    case displayTimeState:
      nextState = stateDisplayTime();
      break;
    case idle1SecState:
      delay (1000);
      nextState = getRTCState;
      break;
    case stopState:
      break;
  }

  server.handleClient();
}
