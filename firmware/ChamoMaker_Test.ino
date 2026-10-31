/*
 * ChamoMaker v1.0 — Firmware de Prueba de Fábrica (Self-Test)
 * Iniciativa Comunitaria: Fundación Infocentro & Origenbotics
 * Web: https://infocentroaguascalientes.com/autana/
 * TikTok: @origenbotics
 * Patrocinado por: PCBWay
 *
 * Prueba todos los sensores, actuadores y periféricos integrados:
 * - Pantalla OLED SSD1306 (I2C)
 * - Sensor de Ultrasonido HC-SR04
 * - Motores DC (Driver MX1508)
 * - Servomotor SG90
 * - LED RGB Neopixel WS2812B
 * - Buzzer piezoeléctrico
 * - LDR (Luz), Micrófono (Sonido), Sensor de inclinación (Tilt), Potenciómetro
 * - Pulsadores táctiles y LEDs de estado
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>
#include <ESP32Servo.h>

// Definición de pines del ChamoMaker
#define PIN_OLED_SDA     21
#define PIN_OLED_SCL     22
#define PIN_SONAR_TRIG   5
#define PIN_SONAR_ECHO   18
#define PIN_NEOPIXEL     23
#define PIN_BUZZER       19
#define PIN_SERVO        4
#define PIN_LED_ROJO     2
#define PIN_LED_VERDE    15
#define PIN_BTN1         17
#define PIN_BTN2         16
#define PIN_POT          36 // VP (ADC1)
#define PIN_LDR          34 // ADC1
#define PIN_MIC          35 // ADC1
#define PIN_TILT         33

// Pines del Driver de Motores MX1508
#define PIN_M1_A         25
#define PIN_M1_B         26
#define PIN_M2_A         27
#define PIN_M2_B         14

// Inicialización de periféricos
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_NeoPixel rgb(1, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);
Servo servo;

void setup() {
  Serial.begin(115200);
  Serial.println("Iniciando ChamoMaker v1.0 Self-Test...");

  // Configuración de Pines
  pinMode(PIN_SONAR_TRIG, OUTPUT);
  pinMode(PIN_SONAR_ECHO, INPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_LED_ROJO, OUTPUT);
  pinMode(PIN_LED_VERDE, OUTPUT);
  pinMode(PIN_BTN1, INPUT_PULLUP);
  pinMode(PIN_BTN2, INPUT_PULLUP);
  pinMode(PIN_TILT, INPUT_PULLUP);

  pinMode(PIN_M1_A, OUTPUT);
  pinMode(PIN_M1_B, OUTPUT);
  pinMode(PIN_M2_A, OUTPUT);
  pinMode(PIN_M2_B, OUTPUT);

  // Inicializar NeoPixel
  rgb.begin();
  rgb.setBrightness(50);
  rgb.setPixelColor(0, rgb.Color(0, 150, 255)); // Azul
  rgb.show();

  // Inicializar Servo
  servo.attach(PIN_SERVO);
  servo.write(90);

  // Inicializar Pantalla OLED
  Wire.begin(PIN_OLED_SDA, PIN_OLED_SCL);
  if (display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(15, 10);
    display.println("CHAMOMAKER v1.0");
    display.setCursor(20, 25);
    display.println("ORIGENBOTICS");
    display.setCursor(10, 40);
    display.println("FUND. INFOCENTRO");
    display.display();
  }

  // Melodía de bienvenida en el Buzzer
  tone(PIN_BUZZER, 523, 100); delay(120);
  tone(PIN_BUZZER, 659, 100); delay(120);
  tone(PIN_BUZZER, 784, 150); delay(160);
  noTone(PIN_BUZZER);
  delay(1000);
}

long leerDistanciaCM() {
  digitalWrite(PIN_SONAR_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_SONAR_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_SONAR_TRIG, LOW);
  long duracion = pulseIn(PIN_SONAR_ECHO, HIGH, 30000);
  if (duracion == 0) return 999;
  return duracion / 58;
}

void loop() {
  // Lectura de Sensores
  int pot = analogRead(PIN_POT);
  int ldr = analogRead(PIN_LDR);
  int mic = analogRead(PIN_MIC);
  long dist = leerDistanciaCM();
  bool btn1 = (digitalRead(PIN_BTN1) == LOW);
  bool btn2 = (digitalRead(PIN_BTN2) == LOW);
  bool tilt = (digitalRead(PIN_TILT) == LOW);

  // Control de LEDs con los botones
  digitalWrite(PIN_LED_ROJO, btn1 ? HIGH : LOW);
  digitalWrite(PIN_LED_VERDE, btn2 ? HIGH : LOW);

  // Mover Servo proporcional al potenciómetro (0 - 180 grados)
  int angulo = map(pot, 0, 4095, 0, 180);
  servo.write(angulo);

  // Si detecta obstáculo a menos de 10 cm, emitir alarma
  if (dist < 10) {
    tone(PIN_BUZZER, 1000, 50);
    rgb.setPixelColor(0, rgb.Color(255, 0, 0)); // Rojo
  } else if (tilt) {
    rgb.setPixelColor(0, rgb.Color(255, 150, 0)); // Naranja (inclinado)
  } else {
    rgb.setPixelColor(0, rgb.Color(0, 255, 0)); // Verde
  }
  rgb.show();

  // Actualizar OLED con telemetría en tiempo real
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.printf("Distancia: %ld cm\n", dist);
  display.printf("Luz (LDR): %d\n", ldr);
  display.printf("Sonido:    %d\n", mic);
  display.printf("Pot/Servo: %d (%d deg)\n", pot, angulo);
  display.printf("B1:%d B2:%d Tilt:%d\n", btn1, btn2, tilt);
  display.display();

  delay(100);
}
