
#include "FS.h"
#include "SD.h"
#include "SPI.h"

//SD: MISO->D19, MOSI->D23, SCK->D18, CS->5
//se módulo com AMS1117->alimentação pela VIN

int entradaSerial; //serve para guardar valor da entrada serial
int randNumber;

String stringRand;

boolean aguardaEntrada = true;

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



void setup() {
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



void loop() {

  Serial.println("\n Iniciando cenario de geracao de numeros aleatorios para log...");

  while(true) {
      for (int i = 0; i < 3; i++) {
      stringRand = String(random(1024)); //gera número aleatório e guarda na RAM como String
      stringRand.concat("\n"); //concatena um caractere de escape correspondente a uma quebra de linha
      char entradaLog[sizeof(stringRand)]; //cria um char para repassar ao const * char da função que dá append
      stringRand.toCharArray(entradaLog, sizeof(entradaLog)); //pega a String e passa para o vetor de caracteres
      appendFileWithoutPrint(SD, "/log.txt", entradaLog); //faz append do random no arquivo
      //Serial.println(stringRand); //imprime o número (para mostrar)
      delay(100);
    }
    separadorTarefa();
    readFile(SD, "/log.txt");
    delay(5000);
  }
}
