#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>


RF24 radio(8,9);                // nRF24L01(+) radio attached using Getting Started board 

RF24Network network(radio);      // Network uses that radio
const uint16_t id_origem = 00;   // Address of the other node in Octal format

struct payload_t {                 // Structure of our payload
  char id[100];
};

String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void setup(void)
{
  Serial.begin(57600);
 
  SPI.begin();
  radio.begin();
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  network.begin(/*channel*/ 90, /*node address*/ id_origem);
}

void loop(void){
  
  network.update();                  // Check the network regularly
  
  while ( network.available() ) {     // Is there anything ready for us?
    
    RF24NetworkHeader header;        // If so, grab it and print it out
    payload_t payload;
    network.read(header,&payload,sizeof(payload));
    String idcolmeia = getValue(payload.id, ':', 0);
    String temperatura = getValue(payload.id, ':', 1);
    String umidade = getValue(payload.id, ':', 2);
    String som = getValue(payload.id, ':', 3);
    String co2 = getValue(payload.id, ':', 4);
    String tensaosensor1 = getValue(payload.id, ':', 5);
    String tensaorepetidor2 = getValue(payload.id, ':', 6);
    String tensaorepetidor1 = getValue(payload.id, ':', 7);

    //Serial.print("Colmeia: ");
    Serial.print(idcolmeia);
    Serial.print(" ");
    //Serial.print("Temperatura: ");
    Serial.print(temperatura);
    Serial.print(" ");
    //Serial.print("Umidade: ");
    Serial.print(umidade);
    Serial.print(" ");
    //Serial.print("Som: ");
    Serial.print(som);
    Serial.print(" ");
    //Serial.print("Co2: ");
    Serial.print(co2);
    Serial.print(" ");
    //Serial.print("Tensao sensor: ");
    Serial.println(tensaosensor1);
    Serial.print(" ");
    //Serial.print("Tensao repetidor: "); 
    Serial.println(tensaorepetidor2);   
  }
}

