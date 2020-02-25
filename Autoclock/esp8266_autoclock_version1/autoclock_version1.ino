#include <ESP8266WiFi.h>
#include <LiquidCrystal.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h> //I2C library
#include <RTClib.h>


// Nomeclatura da Máquina de Estados
#define acessPointState 1
#define NTPReadState 2
#define setRTCState 3
#define getRTCState 4
#define displayTimeState 5
#define idle1SecState 6

// Variáveis de Estado
int currentState = acessPointState;
int nextState = acessPointState;
bool firstNTPRead = true;


// Substitua com as informações da rede
char* ssid     = "Florencio";
const char* password = "brasilia";

int dayOfWeek;
 
// Fuso Horário de São Paulo, horário de Brasília 
float timeZone = -3;

// Define cliente NTP Client para pegar tempo
WiFiUDP ntpUDP;
//NTPClient timeClient(ntpUDP);

// Objeto responsável por recuperar dados sobre horário
NTPClient timeClient(
    ntpUDP,                 //socket udp
    "0.br.pool.ntp.org",    //URL do servwer NTP
    timeZone*3600,          //Deslocamento do horário em relacão ao GMT 0
    60000);     

// Nomes dos dias da semana
char* dayOfWeekNames[] = {"Domingo", "Segunda", "Terça", "Quarta", "Quinta", "Sexta", "Sábado"};


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
unsigned long currentMillis  = 0;               // Armazena o instante atual do processamento
unsigned long previousMillis = 0;           // Armazena o tempo anterior do processamento
const long requestInterval15Min = 900000;   // intervalo de q5 minutos em milisegundos


void startSerial() {
  // Inicialização da Comunicação Serial
  Serial.begin(115200);
  Serial.println("Serial Started!");  
}

void startLCD() {
  //Incialização do LCD
  lcd.begin(16,2);
  lcd.clear();
  Serial.println("LCD Started!");
}

void startNTP() {
  // Inicialização do Cliente NTP para obter Tempo Universal Coordenado
  timeClient.begin();
  Serial.println("NTP Client Started!");
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

void startAcessPoint() {
  // Conecta a Rede Wifi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  lcd.print("Connecting");
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    lcd.print(".");
  }
  
  // Imnprime Endereço Local de IP e Inicia o Servidor Web
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

bool irq15Min() {
  if (currentMillis - previousMillis >= requestInterval15Min) {
    previousMillis = currentMillis;
    return 1;
  } else {
    return 0;
  }
}

int stateAcessPoint() {
  startAcessPoint();
  return NTPReadState;  
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
  
  lcd.clear();
  
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
  startLCD();     // Inicia modulo display LCD
  startNTP();     // Inicializa Cliente NTP
  startI2C();     // Inicializa Comunicação I2C
  startRTC();     // Inicializa o Módulo Real Time Clock

}
void loop() {

  currentMillis = millis();
  
  if (irq15Min() && (WiFi.status() == WL_CONNECTED)) {
    nextState = NTPReadState;
  }
  
  currentState = nextState;

  switch (currentState) {
    
    case acessPointState:
      nextState = stateAcessPoint();
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
  }

}
