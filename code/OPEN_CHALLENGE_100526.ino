// ============================================================
// OPEN CHALLENGE CODE - COM19 PLACA ESP32 DEV MODULE
// ============================================================

// --- LIBRERÍAS ---
#include <ESP32Servo.h>     // Control del servo de dirección
#include <Wire.h>           // Comunicación I2C
#include <VL53L1X.h>        // Sensores de distancia ToF (láser)
#include <Adafruit_TCS34725.h> // Sensor de color para detectar vueltas

// --- VARIABLES DE LIBRERÍAS ---
Servo servo;                              // Servo para la dirección
const uint8_t sensorCount = 3;            // Número de sensores de distancia
VL53L1X sensors[sensorCount];             // Array de 3 sensores VL53L1X
Adafruit_TCS34725 tcs = Adafruit_TCS34725(); // Sensor de color

// --- PINES DEL MOTOR (Puente H) ---
const int motorA = 27;   // Control A del puente H
const int motorB = 2;    // Control B del puente H
const int ENA    = 32;   // PWM para velocidad del motor

// --- PINES DE ENTRADA ---
const int buttonPin = 13;  // Botón de inicio
const int OdoPin    = 15;  // Pin para odometría (encoder)

// --- PINES XSHUT DE LOS SENSORES DE DISTANCIA ---
// Se usan para encenderlos uno a uno y asignarles dirección I2C única
const uint8_t xshutPins[sensorCount] = {23, 5, 14};

// --- VARIABLES GLOBALES ---
int  buttonState = HIGH;    // Estado del botón
int  LapCount    = 0;       // Contador de vueltas (detectadas por color)
long DelayLine   = 0;       // (sin usar)
long MotorMillis = 0;       // Temporizador para parada del motor
long PrevMillis  = 0;       // Temporizador antirrebote de vueltas
long PrevMillisF = 0;       // Temporizador antirrebote del obstáculo frontal
int  TravelSense = 1;       // Estado de marcha: 0=STOP, 1=AVANCE, 2=REVERSA
int  CRearSeq    = 0;       // Contador para la secuencia de retroceso

long filtro_odo   = 0;      // Filtro antirrebotes para odometría (micros)
int  odo_integral = 0;      // Contador de pulsos del encoder

// ============================================================
// INTERRUPCIÓN DEL ODOMETRO (cuenta vueltas)
// ============================================================
// Se ejecuta en cada flanco del pin OdoPin
void IRAM_ATTR ODOMETRO_INTERRUPT() {
  // Filtro antirrebotes: ignora pulsos de menos de 1ms
  if ((micros() - filtro_odo) > 1000) {
    switch (TravelSense) {
      case 0: odo_integral++; break;  // STOP -> incrementa
      case 1: odo_integral++; break;  // AVANCE -> incrementa
      case 2: odo_integral--; break;  // REVERSA -> decrementa
    }
    filtro_odo = micros();
  }
}

// ============================================================
// SETUP - Configuración inicial
// ============================================================
void setup() {
  Serial.begin(115200);

  // --- INICIALIZACIÓN I2C ---
  //Wire.begin();                           // I2C0 por defecto (pins 21,22)
  Wire.begin(16, 17);                       // I2C0 en pins 16,17 (sensores distancia)
  Wire1.begin(21, 22);                      // I2C1 en pins 21,22 (sensor color)
  Wire.setClock(400000);                    // Velocidad I2C a 400kHz

  // --- SERVO DE DIRECCIÓN ---
  servo.attach(26);                         // Pin del servo
  servo.write(90);                          // Posición inicial centrada

  // --- BOTÓN DE INICIO ---
  pinMode(buttonPin, INPUT_PULLUP);

  // --- PUENTE H (MOTOR) ---
  pinMode(motorA, OUTPUT);
  pinMode(motorB, OUTPUT);
  digitalWrite(motorA, LOW);                // Motor parado
  digitalWrite(motorB, LOW);

  ledcAttachChannel(ENA, 1000, 8, 2);       // PWM: frecuencia 1kHz, resolución 8 bits, canal 2

  // --- SENSORES DE DISTANCIA VL53L1X ---
  // Apagamos todos los sensores
  for (uint8_t i = 0; i < sensorCount; i++) {
    pinMode(xshutPins[i], OUTPUT);
    digitalWrite(xshutPins[i], LOW);        // XSHUT LOW -> apagado
  }
  // Encendemos uno por uno asignando dirección I2C única
  for (uint8_t i = 0; i < sensorCount; i++) {
    pinMode(xshutPins[i], INPUT);           // XSHUT floating -> encendido
    delay(10);
    sensors[i].setTimeout(500);
    if (!sensors[i].init()) {
      Serial.print("Error al inicializar sensor ");
      Serial.println(i);
      while (1);                            // Bucle infinito si falla
    }
    sensors[i].setAddress(0x3A + i);        // Direcciones: 0x3A, 0x3B, 0x3C
    sensors[i].setDistanceMode(VL53L1X::Long); // Modo larga distancia
    sensors[i].setMeasurementTimingBudget(33000); // 33ms por medición
    sensors[i].startContinuous(33);         // Mediciones continuas cada 33ms
  }

  // --- SENSOR DE COLOR TCS34725 ---
  if (tcs.begin(0x29, &Wire1)) {
    Serial.println("Sensor de color encontrado");
  } else {
    Serial.println("Sensor de color NO encontrado");
    while (1);
  }
  tcs.setIntegrationTime(TCS34725_INTEGRATIONTIME_2_4MS);
  tcs.setGain(TCS34725_GAIN_60X);
  //attachInterrupt(OdoPin, ODOMETRO_INTERRUPT, CHANGE); // Odometría (comentada)

  // --- ESPERA DEL BOTÓN DE INICIO ---
  // Espera a que se pulse el botón 10 veces (antirrebote)
  int countButton = 0;
  while (countButton <= 10) {
    buttonState = digitalRead(buttonPin);
    if (buttonState == LOW) countButton++;
    delay(10);
  }
}

// ============================================================
// LOOP PRINCIPAL
// ============================================================
void loop() {
  uint16_t r, g, b, c, colorTemp;

  // --- LECTURA DEL SENSOR DE COLOR ---
  tcs.getRawData(&r, &g, &b, &c);
  colorTemp = tcs.calculateColorTemperature_dn40(r, g, b, c);

  //Serial.print("Temp Color: ");
  //Serial.println(colorTemp, DEC);           // Para calibración

  // --- CONTEO DE VUELTAS ---
  // Si la temperatura de color supera 4500, cuenta una vuelta
  if (colorTemp > 4000) {
    // Antirrebote: mínimo 700ms entre detecciones
    if ((PrevMillis + 1000) <= millis()) {
      PrevMillis = millis();
      LapCount++;
    }
  }

  // --- PARADA POR NÚMERO DE VUELTAS ---
  if (LapCount >= 12) {                     // 12 vueltas completadas
    if ((MotorMillis + 1000) <= millis()) { // Espera 1 segundo y para
      TravelSense = 0;                      // STOP
    }
  } else {
    MotorMillis = millis();                 // Reinicia temporizador
  }

  // --- MÁQUINA DE ESTADOS DE DIRECCIÓN ---
  switch (TravelSense) {
    case 0:  // STOP
      digitalWrite(motorA, LOW);
      digitalWrite(motorB, LOW);
      break;
    case 1:  // AVANCE
      digitalWrite(motorA, HIGH);
      digitalWrite(motorB, LOW);
      break;
    case 2:  // REVERSA
      digitalWrite(motorA, LOW);
      digitalWrite(motorB, HIGH);
      break;
  }

  drive();  // Control de dirección y velocidad
}

// ============================================================
// FUNCIÓN DRIVE - Control de dirección y velocidad
// ============================================================
void drive() {
  static signed int distanceRight, distanceLeft, distanceFront;

  // --- LECTURA DE SENSORES DE DISTANCIA ---
  if (sensors[0].dataReady()) {             // Sensor derecho
    distanceRight = sensors[0].read(false);
    if (distanceRight > 1500) distanceRight = 1500;  // Limitar a 1500mm
  }

  if (sensors[1].dataReady()) {             // Sensor izquierdo
    distanceLeft = sensors[1].read(false);
    if (distanceLeft > 1500) distanceLeft = 1500;
  }

  if (sensors[2].dataReady()) {             // Sensor frontal
    distanceFront = sensors[2].read(false);
  }

  // --- DETECCIÓN DE OBSTÁCULO FRONTAL ---
  // Si hay un obstáculo a menos de 200mm, inicia secuencia de retroceso
  if (distanceFront < 200) {
    // Antirrebote: mínimo 1500ms entre activaciones
    if ((PrevMillisF + 1500) <= millis()) {
      PrevMillisF = millis();
      CRearSeq = 200;   // Inicia contador de retroceso (inicial 300)
    }
  }

  // --- SECUENCIA DE RETROCESO ---
  // CRearSeq es un contador decreciente que gobierna 3 fases:
  if (CRearSeq >= 200) TravelSense = 0;     // Fase 1: STOP (contador 300..200)
  if (CRearSeq > 0 && CRearSeq < 200) TravelSense = 2; // Fase 2: REVERSA (199..1)
  if (CRearSeq == 1) TravelSense = 1;       // Fase 3: AVANCE (contador = 1)
  if (CRearSeq > 0) CRearSeq--;             // Decrementa el contador

  // --- ALGORITMO DE DIRECCIÓN (SERVO) ---
  // Calcula el error de dirección: diferencia entre distancia derecha e izquierda
  signed int dServo = (distanceRight - distanceLeft) / 20;
  // Limita el ángulo del servo a ±42 grados
  if (dServo > 42) dServo = 42;
  if (dServo < -42) dServo = -42;
  // En reversa se invierte la dirección del servo
  if (TravelSense >= 2) servo.write(-dServo + 85);
  else if (TravelSense == 1) servo.write(dServo + 85);

  // --- AJUSTE DE VELOCIDAD AL GIRAR ---
  // Acelera la rueda exterior en los giros para mejorar el viraje
  if (dServo >= 0) ledcWrite(ENA, 120 + dServo);
  else ledcWrite(ENA, 120 - dServo);
}
