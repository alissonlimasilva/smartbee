#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include <DHT.h>
#include <JeeLib.h>

#define DHTPIN A2       // Pino DATA do Sensor DHT.
#define DHTTYPE DHT22   // Define o tipo do sensor utilizado DHT 11
#define IDCOLMEIA "Colmeia2" //ID da Colmeia monitorada
#define TEMPOENTRECADALEITURA 2000 //Tempo entre cada leitura 

DHT dht(DHTPIN, DHTTYPE); //Objeto do sensor de temperatura

RF24 radio(8, 9);                   // nRF24L01(+) radio attached using Getting Started board
RF24Network network(radio);          // Network uses that radio

const uint16_t id_origem = 011;        // Address of our node in Octal format
const uint16_t id_destino = 04;       // Address of the other node in Octal format

struct payload_t {                  // Structure of our payload
  char id[50];
};

String mensagem = "";

float temperatura_lida = 0;
float umidade_lida = 0;
float co2_lido = 0;
float som_lido = 0;
float tensao_lida = 0;

int SENSORSOM = A0;
//int SENSORCO2 = 0;
int SENSORTENSAO = A4;

ISR(WDT_vect) { Sleepy::watchdogEvent(); }

void concatena(){
  mensagem = IDCOLMEIA;
  mensagem +=":";
  mensagem +=temperatura_lida;
  mensagem +=":";
  mensagem +=umidade_lida;
  mensagem +=":";
  mensagem +=som_lido;
  mensagem +=":";
  mensagem +=co2_lido;
  mensagem +=":";
  mensagem +=tensao_lida;
}

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
  tensao_lida=((valor_lido_tensao*0.004887586)*4.65);
}

void setup(void)
{
  Serial.begin(57600);
  Serial.println("Transmissor");
 
  SPI.begin();
  radio.begin();
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  network.begin(/*channel*/ 90, /*node address*/ id_origem);
}

void loop() {
  
  network.update();                          // Check the network regularly
    
    Serial.print("Sending...");
    //cria payload
    payload_t payload;
    //ler sensores
    lerDHT();
    lerMQandKy();
    lerTensao();
    
    //cria mensagem a ser enviada
    concatena();
    //converte essa mensagem em char
    char char_mensagem[50];
    mensagem.toCharArray(char_mensagem, 50);
    //adiciona ao payload
    strcpy( payload.id, char_mensagem);
    RF24NetworkHeader header(id_destino);
    bool ok = network.write(header,&payload,sizeof(payload));

  Sleepy::loseSomeTime(TEMPOENTRECADALEITURA);
}


