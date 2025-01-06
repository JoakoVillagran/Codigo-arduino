#include <WiFi.h>
#include <HTTPClient.h>

// Configuración WiFi y ThingsBoard
const char* ssid = "Villagran's";
const char* password = "papitobonito1234";
const char* server = "http://demo.thingsboard.io/api/v1/VCEBEAhOqT8qNgVLPc9c/telemetry"; 

// Pines del sensor ultrasónico
const int trigPin = 5;
const int echoPin = 18;

// Datos fijos
const float maxHeight = 50.0; // Altura total del basurero en cm
const String latitude = "-38.73965"; // Latitud ficticia
const String longitude = "-72.59842"; // Longitud ficticia
const String locationName = "Temuco";
const String binName = "Basurero A";
const int maxCapacity = 100; // Máximo contenido permitido en kg

void setup() {
  Serial.begin(115200);

  // Configuración del sensor ultrasónico
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Conexión WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Conectando a WiFi...");
  }
  Serial.println("Conectado a WiFi");
}

void loop() {
  // Leer nivel de llenado con el sensor ultrasónico
  long duration;
  float distance, fillLevel;

  // Enviar pulso
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Leer eco
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;

  // Calcular nivel de llenado
  if (distance > 0 && distance <= maxHeight) {
    fillLevel = (1 - (distance / maxHeight)) * 100;
  } else {
    fillLevel = 0;
  }

  // Datos enviados a ThingsBoard
  String payload = "{";
  payload += "\"latitude\":\"" + latitude + "\",";
  payload += "\"longitude\":\"" + longitude + "\",";
  payload += "\"location_name\":\"" + locationName + "\",";
  payload += "\"bin_name\":\"" + binName + "\",";
  payload += "\"current_fill_level\":" + String(fillLevel) + ",";
  payload += "\"max_capacity\":" + String(maxCapacity);
  payload += "}";

  // Enviar datos a ThingsBoard
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(server);
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(payload);

    if (httpResponseCode > 0) {
      Serial.println("Datos enviados: " + payload);
    } else {
      Serial.println("Error al enviar datos");
    }
    http.end();
  } else {
    Serial.println("WiFi no conectado");
  }

  delay(15000);
}
