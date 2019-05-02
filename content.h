//  INCLUDE
#include <U8g2lib.h>
//para o display OLED SSD1306
//https://techtutorialsx.com/2017/12/02/esp32-arduino-interacting-with-a-ssd1306-oled-display/)
//alimentação pela 3.3V

#include <Wire.h> //biblioteca para trabalhar com protocolo I2C //SDA->D21, SCL->D22
#include <Ultrasonic.h> //para o SR04, que opera em 5V

#include "FS.h" //para o SD
#include "SD.h"
#include "SPI.h"
//SD: MISO->D19, MOSI->D23, SCK->D18, CS->D5
//se módulo com AMS1117->alimentação pela VIN

//  DEFINE

#define PIN_LED_VERMELHO_PEDESTRE 12
#define PIN_LED_VERDE_PEDESTRE 13

//#define PIN_DATA_OLED 21  //referencia de pinos para OLED
//#define PIN_CLOCK_OLED 22

#define PIN_LED_VERMELHO_MOTORISTA 25
#define PIN_LED_AMARELO_MOTORISTA 26
#define PIN_LED_VERDE_MOTORISTA 27

#define PIN_ULTRASSOM_TRIGGER 34  //pinos para o ultrassom SR04
#define PIN_ULTRASSOM_ECHO 35

#define TEMPO_BASE_FAROL_VERMELHO_MOTORISTA 3000
#define TEMPO_BASE_FAROL_AMARELO_MOTORISTA 3000
#define TEMPO_BASE_FAROL_VERDE_MOTORISTA 7000

#define MSG_LED_VERMELHO_MOTORISTA "Vermelho"
#define MSG_LED_AMARELO_MOTORISTA "Amarelo"
#define MSG_LED_VERDE_MOTORISTA "Verde"

#define INTERVALO_LEITURA_ULTRASSOM 250 //tempo em ms

//referencia para tempos de pedestre
//#define TEMPO_BASE_FAROL_VERDE_PEDESTRE 3000
//#define TEMPO_BASE_FAROL_VERMELHO_PEDESTRE 3000

#define TEMPO_MUDANCA_MODAL 250  //diferenca quando muda de modal (carro ou pernas)


//  GLOBAIS
long distancia; //usada pelo HC-SR04 como armazenamento de distancia

int entradaSerial; //serve para guardar valor da entrada serial no caso do SD
int randNumber;
String stringRand;
boolean aguardaEntrada = true;


//  OBJETOS
U8G2_SSD1306_128X64_NONAME_2_HW_I2C u8g2(U8G2_R0);  //criação de objeto para uso da tela OLED
//rotação, SDA (D21), SCL (D22), reset
//1 = buffer pequeno, 2 = buffer grande, F = buffer completo

Ultrasonic ultrasonic(PIN_ULTRASSOM_TRIGGER, PIN_ULTRASSOM_ECHO);   //Inicializa o sensor nos pinos definidos acima

//  STRUCTS
//funcoes para os LEDs
struct SemaforoMotorista {
  int verde = PIN_LED_VERDE_MOTORISTA;
  int amarelo = PIN_LED_AMARELO_MOTORISTA;
  int vermelho = PIN_LED_VERMELHO_MOTORISTA;
  int tempoVerde = TEMPO_BASE_FAROL_VERDE_MOTORISTA;
  int tempoAmarelo = TEMPO_BASE_FAROL_AMARELO_MOTORISTA;
  int tempoVermelho = TEMPO_BASE_FAROL_VERMELHO_MOTORISTA;
  String msgVerde = MSG_LED_VERDE_MOTORISTA;
  String msgAmarelo = MSG_LED_AMARELO_MOTORISTA;
  String msgVermelho = MSG_LED_VERMELHO_MOTORISTA;

  void alternaLuz(int pinoApagar, int pinoAcender) { //desliga o primeiro e liga o segundo
    digitalWrite(pinoApagar, LOW);
    digitalWrite(pinoAcender, HIGH);
  }
};

struct SemaforoPedestre {
  int verde = PIN_LED_VERDE_PEDESTRE;
  int vermelho = PIN_LED_VERMELHO_PEDESTRE;
  //int tempoVerde = TEMPO_BASE_FAROL_VERDE_PEDESTRE;
  //int tempoVermelho = TEMPO_BASE_FAROL_VERMELHO_PEDESTRE;

  void alternaLuz(int pinoApagar, int pinoAcender) {
    digitalWrite(pinoApagar, LOW);
    digitalWrite(pinoAcender, HIGH);
  }

  void vermelhoPiscante(int tempoAmarelo) {
    for (int i=0; i<tempoAmarelo; i+=500) {
      digitalWrite(PIN_LED_VERMELHO_PEDESTRE, LOW);
      delay(250);
      digitalWrite(PIN_LED_VERMELHO_PEDESTRE, HIGH);
      delay(250);
    }
  }
};

//funcoes para a tela
struct TelaOLED {
  void exibe(int tamanhoFonte, const String& conteudo) {
    String fonte;
    short int posicaoCursorX, posicaoCursorY;
    u8g2.firstPage();
    do {
      switch(tamanhoFonte) { //verifica valor da variável, efetuando operações de acordo com o valor
        case 1: //seria a execução para valor 1, por exemplo
          u8g2.setFont(u8g2_font_crox3c_tf); //uma fonte menor
          posicaoCursorX = 0, posicaoCursorY = 17;
        break;
        case 2:
          u8g2.setFont(u8g2_font_fur25_tn);
          posicaoCursorX = 46, posicaoCursorY = 60;
        break;
        default:
          u8g2.setFont(u8g2_font_fur25_tn); //uma fonte maior, para numeros
          posicaoCursorX = 32, posicaoCursorY = 60;
        break;
      }
      u8g2.setCursor(posicaoCursorX, posicaoCursorY); //ajusta x e y na tela para iniciar o texto
      u8g2.print(conteudo); //imprime efetivamente a String informada
    } while( u8g2.nextPage() );
  }

  void contagem(int tempo) {
    for (int i = tempo; i >= 0; i--) {
      exibe(2, String(i));
      delayMicroseconds(1000000);
    }
    exibe(2, "...");
  }
};

//funcoes para o HC-SR04

struct SR04 {
  int distanciaAtual;
  int distancia() {   //faz a leitura das informacoes do sensor (em cm)
    int distanciaCM;
    long microsec = ultrasonic.timing();  //deve ser para ter o tempo de trajeto...
    distanciaCM = ultrasonic.convert(microsec, Ultrasonic::CM);
    Serial.println(distanciaCM);
    distanciaAtual = distanciaCM;
    return distanciaCM;
  }
};


//funcoes para o SDCard
struct SDCard {


void iniciar() {
randomSeed(analogRead(0));
    if(!SD.begin()) {
        Serial.println("Incapaz de montar o microSD");
        return;
    }
    uint8_t cardType = SD.cardType();

    if(cardType == CARD_NONE) {
        Serial.println("Nenhum cartao de memoria encontrado");
        return;
    }

    Serial.print(F("Tipo de cartao de memoria: "));
    if(cardType == CARD_MMC) {
        Serial.print("MMC;");
        return;
    } else if(cardType == CARD_SD) {
        Serial.print("SDSC;");
    } else if(cardType == CARD_SDHC) {
        Serial.print("SDHC;");
    } else {
        Serial.print("UNKNOWN;");
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf(" Capacidade: %lluMB\n", cardSize);
    separadorTarefa();

    deleteFile(SD, "/log.txt");
    separadorTarefa();
    
    listDir(SD, "/", 0);
    separadorTarefa();
    
    writeFile(SD, "/log.txt", "\n\nInicio:\n");
    separadorTarefa();
    
    listDir(SD, "/", 0);
    separadorTarefa();
    
    for (int i = 0; i < 3; i++) {
      appendFile(SD, "/log.txt", "Registrado com sucesso!\n");
    }
    separadorTarefa();

    listDir(SD, "/", 0);
    separadorTarefa();

    readFile(SD, "/log.txt");
    separadorTarefa();
    
    //testFileIO(SD, "/test.txt");

    //deleteFile(SD, "/test.txt"); //remover depois
    //separadorTarefa();
}

void separadorTarefa() {
  Serial.println("\n___________________________________________");
}

void listDir(fs::FS &fs, const char * dirname, uint8_t levels) {
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if(!root) {
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()) {
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file) {
        if(file.isDirectory()) {
            Serial.print(" DIR : ");
            Serial.println(file.name());
            if(levels) {
                listDir(fs, file.name(), levels -1);
            }
        } else {
            Serial.print(" FILE: ");
            Serial.print(file.name());
            Serial.print(" SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

//era o createDir
//era o removeDir

void readFile(fs::FS &fs, const char * path) {
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if(!file) {
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    while(file.available()) {
        Serial.write(file.read());
    }
}

void writeFile(fs::FS &fs, const char * path, const char * message) {
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file) {
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(message)) {
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
}

void appendFile(fs::FS &fs, const char * path, const char * message) {
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file) {
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)) {
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
}

void appendFileWithoutPrint(fs::FS &fs, const char * path, const char * message) { //derivada da appendFile que desliga os prints
    //Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file) {
        //Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)) {
        //Serial.println("Message appended");
    } else {
        //Serial.println("Append failed");
    }
}

//era o renameFile

void deleteFile(fs::FS &fs, const char * path) {
    Serial.printf("Deleting file: %s\n", path);
    if(fs.remove(path)) {
        Serial.println("File deleted");
    } else {
        Serial.println("Delete failed");
    }
}

void testFileIO(fs::FS &fs, const char * path) {
    File file = fs.open(path);
    static uint8_t buf[512];
    size_t len = 0;
    uint32_t start = millis();
    uint32_t end = start;
    if(file) {
        len = file.size();
        size_t flen = len;
        start = millis();
        while(len) {
            size_t toRead = len;
            if(toRead > 512) {
                toRead = 512;
            }
            file.read(buf, toRead);
            len -= toRead;
        }
        end = millis() - start;
        Serial.printf("%u bytes read for %u ms\n", flen, end);
        file.close();
    } else {
        Serial.println("Failed to open file for reading");
    }

    file = fs.open(path, FILE_WRITE);
    if(!file) {
        Serial.println("Failed to open file for writing");
        return;
    }

    size_t i;
    start = millis();
    for(i=0; i<2048; i++) {
        file.write(buf, 512);
    }
    end = millis() - start;
    Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
    file.close();
}



/*void setup() {
    randomSeed(analogRead(0));
    delay(1000);
    Serial.begin(115200);
    Serial.print("\n\n");
    if(!SD.begin()) {
        Serial.println("Incapaz de montar o microSD");
        return;
    }
    uint8_t cardType = SD.cardType();

    if(cardType == CARD_NONE) {
        Serial.println("Nenhum cartao de memoria encontrado");
        return;
    }

    Serial.print(F("Tipo de cartao de memoria: "));
    if(cardType == CARD_MMC) {
        Serial.print("MMC;");
        return;
    } else if(cardType == CARD_SD) {
        Serial.print("SDSC;");
    } else if(cardType == CARD_SDHC) {
        Serial.print("SDHC;");
    } else {
        Serial.print("UNKNOWN;");
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf(" Capacidade: %lluMB\n", cardSize);
    separadorTarefa();

    deleteFile(SD, "/log.txt");
    separadorTarefa();
    
    listDir(SD, "/", 0);
    separadorTarefa();
    
    writeFile(SD, "/log.txt", "\n\nInicio:\n");
    separadorTarefa();
    
    listDir(SD, "/", 0);
    separadorTarefa();
    
    for (int i = 0; i < 3; i++) {
      appendFile(SD, "/log.txt", "Registrado com sucesso!\n");
    }
    separadorTarefa();

    listDir(SD, "/", 0);
    separadorTarefa();

    readFile(SD, "/log.txt");
    separadorTarefa();
    
    //testFileIO(SD, "/test.txt");

    //deleteFile(SD, "/test.txt"); //remover depois
    //separadorTarefa();
    
}
*/
};
