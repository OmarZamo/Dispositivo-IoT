#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>

#define USE_SERIAL Serial

#define ADELANTE 1
#define ATRAS 2
#define DETENER 0
#define VUELTA_IZQUIERDA 3
#define VUELTA_DERECHA 4
#define GIRO_IZQUIERDA 5
#define GIRO_DERECHA 6

#define LED_Adelante 26;
#define LED_Atras 27;
#define LED_Izquierda 13;
#define  LED_Derecha 14;


const int EchoPin = 34;
const int TriggerPin = 25;

WiFiMulti wifiMulti;

int mover[7][6] = {
    {0, 0, 0, 0, 0, 0}, 
    {1, 0, 200, 1, 0, 200},  // Adelante
    {0, 1, 200, 0, 1, 200},  // Atrás     
    {1, 0, 150, 1, 0, 200},   // Vuelta izquierda
    {1, 0, 200, 1, 0, 150},   // Vuelta derecha
    {1, 0, 200, 0, 1, 200},  // Giro izquierda
    {0, 1, 200, 1, 0, 200}   // Giro derecha
};

int led[7][4] = {
    {0, 0, 0, 0}, 
    {1, 0, 0, 0},  // Adelante
    {0, 1, 0, 0},  // Atrás     
    {0, 0, 1, 0},   // Vuelta izquierda
    {0, 0, 0, 1},   // Vuelta derecha
    {1, 0, 1, 0},  // Giro izquierda
    {1, 0, 0, 1}   // Giro derecha
};

void setup() {
    USE_SERIAL.begin(115200);
    USE_SERIAL.println();
    USE_SERIAL.println("Iniciando...");
    USE_SERIAL.println();

    for (uint8_t t = 4; t > 0; t--) {
        USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
        USE_SERIAL.flush();
        delay(1000);
    }

    wifiMulti.addAP("INFINITUM039F_2.4", "FxMQtw7Avb");

    pinMode(16, OUTPUT); // int 1
    pinMode(17, OUTPUT); // int 2
    pinMode(18, OUTPUT); // int 3
    pinMode(19, OUTPUT); // int 4

    pinMode(32, OUTPUT); // enable1
    pinMode(33, OUTPUT); // enable2

    pinMode(TriggerPin, OUTPUT);
    pinMode(EchoPin, INPUT);

    pinMode(26, OUTPUT);
    pinMode(27, OUTPUT);
    pinMode(13, OUTPUT);
    pinMode(14, OUTPUT);

   // DEMO();
}

void mueveCarro(int indice) {
    // Motor izquierdo
    digitalWrite(16, mover[indice][0]);
    digitalWrite(17, mover[indice][1]);
    analogWrite(32, mover[indice][2]);

    // Motor derecho
    digitalWrite(18, mover[indice][3]);
    digitalWrite(19, mover[indice][4]);
    analogWrite(33, mover[indice][5]);

    digitalWrite(26, led[indice][0]);
    digitalWrite(27, led[indice][1]);
    digitalWrite(13, led[indice][2]);
    digitalWrite(14, led[indice][3]);

    // delay(tiempo*1000);
}

void DEMO() {
    for (int i = 0; i < 3; i++) {
        int numeroAleatorio = random(0, 7);  // Genera un número entre 0 y 6
        mueveCarro(numeroAleatorio);
        delay(500);
    }
}

int ping(int TriggerPin, int EchoPin) {
    unsigned short int duration;
    float distanceCm;

    digitalWrite(TriggerPin, LOW); // para generar un pulso limpio ponemos a LOW 4us
    delayMicroseconds(4);
    digitalWrite(TriggerPin, HIGH); // generamos Trigger (disparo) de 10us
    delayMicroseconds(10);
    digitalWrite(TriggerPin, LOW);

    duration = pulseIn(EchoPin, HIGH); // medimos el tiempo entre pulsos, en microsegundos

    distanceCm = float(duration) / 58.2; // convertimos a distancia, en cm
    return distanceCm;
}

void anticolicion(float cm ,int mov){
    if(cm >= 40.0){
      mueveCarro(mov);
      delay(1000);
    }else if(mov == 0){
       mueveCarro(mov);
       delay(1000);
    }else{
         mueveCarro(DETENER);
         delay(500);
         mueveCarro(GIRO_DERECHA);
         delay(500);
        
    }

}


void loop() {
    // Wait for WiFi connection
    if (wifiMulti.run() == WL_CONNECTED) {
        HTTPClient http;

        USE_SERIAL.print("[HTTP] begin...\n");
        // Configure target server and URL
        http.begin("http://54.167.237.127/php-apirest-oop/back-end/api.php?action=lastest"); // HTTP

        USE_SERIAL.print("[HTTP] GET...\n");
        // Start connection and send HTTP header
        int httpCode = http.GET();

        // httpCode will be negative on error
        if (httpCode > 0) {
            // HTTP header has been sent and Server response header has been handled
            USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);

            // File found at server
            if (httpCode == HTTP_CODE_OK) {
                String payload = http.getString();
                USE_SERIAL.println("Respuesta de la API:");
                USE_SERIAL.println(payload);

                int movimiento = payload.toInt();
                if (movimiento >= 0 && movimiento <= 6) {
                   float cm = ping(TriggerPin, EchoPin);
                      anticolicion(cm,movimiento);
                } 
                else if(movimiento == 7){
                  DEMO();

                }
                
                else {
                    USE_SERIAL.println("Movimiento inválido recibido.");
                }
            }
        } else {
            USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }

        http.end();
    } else {
        USE_SERIAL.println("[WiFi] Not connected. Attempting to reconnect...");
        WiFi.reconnect();
    }

    delay(1000);  // Pausa antes de la siguiente solicitud
}


