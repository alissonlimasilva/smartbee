#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include <DHT.h>
#include <JeeLib.h>
#include <ArduinoJson.h>

#define DHTPIN A0       // Pino DATA do Sensor DHT.
#define DHTTYPE DHT22   // Define o tipo do sensor utilizado DHT 11
#define IDCOLMEIA 3//ID da Colmeia monitorada
#define TEMPOENTRECADALEITURA 60000 //Tempo entre cada leitura 
#define SENSOR "Sensor 3"

DHT dht(DHTPIN, DHTTYPE); //Objeto do sensor de temperatura

RF24 radio(8,9);                   // nRF24L01(+) radio attached using Getting Started board
RF24Network network(radio);          // Network uses that radio

const uint16_t id_origem = 0311;        // Address of our node in Octal format
const uint16_t id_destino = 011;       // Address of the other node in Octal format

struct payload_t {                  // Structure of our payload
  int colmeia;
  float temperatura;
  float umidade;
  float tensao_c;
  float tensao_r;
};

float temperatura_lida = 0;
float umidade_lida = 0;
float co2_lido = 0;
float som_lido = 0;
float tensao_lida = 0;

int SENSORSOM = A0;
//int SENSORCO2 = 0;
int SENSORTENSAO = A4;

ISR(WDT_vect) { Sleepy::watchdogEvent(); }

void lerDHT(){
  if (isnan(dht.readTemperature())) {temperatura_lida = 0;}
  
  else {temperatura_lida = dht.readTemperature();}
  
  if (isnan(dht.readHumidity())){umidade_lida = 0;}
  
  else {umidade_lida = dht.readHumidity();}
}

void lerMQandKy(){
  som_lido = analogRead(SENSORSOM);
  co2_lido = 0;
}

void lerTensao(){
  float valor_lido_tensao = analogRead(SENSORTENSAO);
  tensao_lida=((valor_lido_tensao*0.00489)*5);
}

void setup(void)
{
  Serial.begin(57600); 
  SPI.begin();
  radio.begin();
  radio.setPayloadSize(32);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  network.begin(/*channel*/ 120, /*node address*/ id_origem);
}

void loop() {
  
  network.update();                          // Check the network regularly
    //cria payload
    payload_t payload;
    //ler sensores
    lerDHT();
    //lerMQandKy();
    //lerTensao();
   
    RF24NetworkHeader header(id_destino);
    payload.colmeia=IDCOLMEIA;
    payload.temperatura=temperatura_lida;
    payload.umidade=umidade_lida;
    payload.tensao_c=tensao_lida;
    payload.tensao_r=0;
    bool ok = network.write(header,&payload,sizeof(payload));
    //delay(TEMPOENTRECADALEITURA);
    Sleepy::loseSomeTime(TEMPOENTRECADALEITURA);
}



