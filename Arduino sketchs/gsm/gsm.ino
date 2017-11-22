#include <SoftwareSerial.h>
#include <JeeLib.h>
#include <gprs.h>
#include <DHT.h>
#include <RF24Network.h>
#include <RF24.h>

#define DHTPIN A3       // Pino DATA do Sensor DHT.
#define DHTTYPE DHT22   // Define o tipo do sensor utilizado DHT 1
#define SENSORTENSAO A4 // Pino conectado sensor de tensao

DHT dht(DHTPIN, DHTTYPE);
SoftwareSerial SIM800L(2, 3); // ConfiguraÃ§Ã£o da Porta Serial

//CONFIGURACOES INICIAIS NO NRF
RF24 radio(8,9);                // nRF24L01(+) radio attached using Getting Started board 
RF24Network network(radio);      // Network uses that radio
const uint16_t id_origem = 011;    // Address of our node in Octal format ( 04,031, etc)

//PACOTE QUE É RECEBIDO DOS SENSORES
struct payload_t {                  // Structure of our payload
  int colmeia;
  float temperatura;
  float umidade;
  float tensao_c;
  float tensao_r;
};

//VARIAVEIS GLOBAIS
float tensao_lida=0;
float temperatura_lida=0;
float umidade_lida=0;

float tensao_colmeia=0;
float temperatura_colmeia=0;
float umidade_colmeia=0;
int colmeia_id=0;

ISR(WDT_vect) {
  Sleepy::watchdogEvent();
}

void lerTensao(){
  float valor_lido_tensao = analogRead(SENSORTENSAO);
  tensao_lida=((valor_lido_tensao*0.00489)*5);
}
//FUNCAO QUE LE TEMPERATURA E UMIDADE E SALVA NAS VARIAVEIS GLOBAIS

void lerDHT(){
  if (isnan(dht.readTemperature())) {temperatura_lida = 0;}
  
  else {temperatura_lida = dht.readTemperature();}
  
  if (isnan(dht.readHumidity())){umidade_lida = 0;}
  
  else {umidade_lida = dht.readHumidity();}
}

void resposta() {
  while (SIM800L.available())
  {
    if (SIM800L.available() > 0)   //se recebe dados
    {
      Serial.write(SIM800L.read()); //impreme a resposta

    }
  }

  Serial.flush();
  Serial.flush();
}
bool sendSMS(const char number, const char text)
{
    cleanBuffer();
    SIM800L.println("AT+CMGF=1");
    if ( waitFor("OK", "ERROR") != 1 ) return false;

    cleanBuffer();
    SIM800L.print("AT+CMGS=\"");
    SIM800L.print(number);
    SIM800L.println("\"");
    if ( waitFor(">", "ERROR") != 1 ) return false;

    cleanBuffer();
    SIM800L.print(text);
    SIM800L.println((char)26);
    if ( waitFor("+CMGS:", "ERROR") != 1 ) return false;

    return true;
}

void cleanBuffer()
{
    delay( 250 );
    while ( SIM800L.available() > 0) 
    {
        SIM800L.read();    // Clean the input buffer
        delay(50);
    }
}
int8_t waitFor(const char* expected_answer1, const char* expected_answer2)
{
    uint8_t x=0, answer=0;
    char response[100];
    unsigned long previous;

    memset(response, (char)0, 100);    // Initialize the string

    delay( 250 );

    x = 0;
    previous = millis();

    // this loop waits for the answer
    do{
        // if there are data in the UART input buffer, reads it and checks for the asnwer
        if(SIM800L.available() > 0){
            response[x] = SIM800L.read();
            x++;
            // check if the desired answer 1  is in the response of the module
            if (strstr(response, expected_answer1) != NULL)
            {
                answer = 1;
            }
            // check if the desired answer 2 is in the response of the module
            else if (strstr(response, expected_answer2) != NULL)
            {
                answer = 2;
            }
        }
        delay(10);
    }
    // Waits for the asnwer with time out
    while((answer == 0) && ((millis() - previous) < 10000 ));

    return answer;
}

bool setupGPRS(){
    if ( !hasIP() ){
      Serial.println("Failed to acquire IP address.");
     // sendSMS( "+31612345678", "ERRO " );
      
      return false;
    }

    return true;
    }


bool hasIP()
{
    cleanBuffer();
   SIM800L.println("AT+SAPBR=2,1");
    if ( waitFor("0.0.0.0", "OK") != 2 ) return false;

    return true;
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); //iniciando comunicaÃ§Ã£o serial
  SIM800L.begin(9600); //iniciando SoftwareSerial

  SIM800L.println("AT"); //enviando Comando AT
  delay (9000); //Tempo de espera
  resposta();

  radio.begin();
  radio.setPayloadSize(32);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  network.begin(/*channel*/ 120, /*node address*/ id_origem);
  
  //GetBattery();
  //GetSignalQuality();
}

void loop() {
 network.update();                  // Check the network regularly
  //esse metodo só executa quando chega alguma mensagem
  while ( network.available() ) {
    RF24NetworkHeader header;        // If so, grab it and print it out
    payload_t recebido;
    network.read(header,&recebido,sizeof(recebido));

    tensao_colmeia= recebido.tensao_c;
    temperatura_colmeia=recebido.temperatura;
    umidade_colmeia=recebido.umidade;
    colmeia_id=recebido.colmeia;
  }
 gsm_enviohttp();

 // colocando o módulo em modo sleep automático.
 SIM800L.println("AT+CSCLK=2");
 delay (3000); //Tempo de espera
 resposta();
 
 //Sleepy::loseSomeTime(60000);
  
}

void gsm_enviohttp(){
  
  //setupGPRS();
  
  SIM800L.println("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\""); //enviando Comando AT
  delay (1000); //Tempo de espera
  resposta();

  SIM800L.println("AT+SAPBR=3,1,\"APN\",\"gprs.oi.com.br\""); //enviando Comando AT
  delay (3000); //Tempo de espera
  resposta();

  SIM800L.println("AT+SAPBR=3,1,\"USER\",\"oi\""); //enviando Comando AT
  delay (3000); //Tempo de espera
  resposta();

  SIM800L.println("AT+SAPBR=3,1,\"PWD\",\"oi\""); //enviando Comando AT
  delay (3000); //Tempo de espera
  resposta();

  SIM800L.println("AT+SAPBR=1,1");
  delay (2000);
  resposta();

  SIM800L.println("AT+SAPBR=2,1");
  delay (2000);
  resposta();


  hasIP(); 
  //setupGPRS();
  
  SIM800L.println("AT+HTTPINIT");
  delay (2000);
  resposta();

  SIM800L.println("AT+HTTPPARA=\"CID\",1");
  delay (2000);
  resposta();


  SIM800L.println("AT+HTTPPARA=\"URL\",\"http://35.199.66.232:8080/webservice/coleta/4/23.4/12/12.3/12.7\"");
  delay (2000);
  resposta();
  
  SIM800L.println("AT+HTTPACTION=0");
  delay (3000);
  resposta();

  SIM800L.println("AT+HTTPREAD");
  delay (3000);
  resposta();

  //SIM800L.println("AT+CIPACK");
  //delay (3000);
  //resposta();

  SIM800L.println("AT+HTTPTERM");
  delay (1000);
  resposta();

  SIM800L.println("AT+SAPBR= 0,1"); //EHCERRA
  delay (1000);
  resposta();
  }

void GetSignalQuality(){
     String response = ""; 
     long int time = millis();   
     Serial.println("Getting the sinal quality...");
     Serial.println("Tips:+CSQ: XX,QQ : It means the Signal Quality poorly when the XX is '99'!");
     SIM800L.println("AT+CSQ");    
     while( (time+2000) > millis()){
        while(SIM800L.available()){       
          response += char(SIM800L.read());
        }  
      }    
      Serial.print(response);    
}

void GetBattery(){
     String response = ""; 
     long int time = millis();   
     Serial.println("Porcentagem da Bateria!");
     SIM800L.println("AT+CBC");    
     while( (time+3000) > millis()){
        while(SIM800L.available()){       
          response += char(SIM800L.read());
        }  
      }    
      Serial.print(response);    
}

