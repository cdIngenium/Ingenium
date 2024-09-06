#include <Servo.h>

// Definir pines de motores DC, sensores ultrasónicos y botón
const int motorIzqA = 3;
const int motorIzqB = 4;
const int motorDerA = 5;
const int motorDerB = 6;
const int trigPinIzq = 7;  // Pin trig sensor ultrasónico izquierdo
const int echoPinIzq = 8;  // Pin echo sensor ultrasónico izquierdo
const int trigPinDer = 9;  // Pin trig sensor ultrasónico derecho
const int echoPinDer = 10; // Pin echo sensor ultrasónico derecho
const int botonPin = 2;    // Pin del botón para comenzar

Servo servoMotor;

int distanciaIzq = 0;
int distanciaDer = 0;
int vueltas = 0;
bool carroActivo = false;  // Variable para saber si el carro está en marcha
unsigned long tiempoInicio;
unsigned long tiempoTotalVuelta = 30000; // Ajustar según el tiempo de cada vuelta

// Función para medir la distancia con el sensor ultrasónico
int medirDistancia(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duracion = pulseIn(echoPin, HIGH);
  int distanciaCM = duracion * 0.034 / 2; // Convertir a cm
  return distanciaCM;
}

// Función para avanzar recto
void avanzarRecto() {
  digitalWrite(motorIzqA, HIGH);
  digitalWrite(motorIzqB, LOW);
  digitalWrite(motorDerA, HIGH);
  digitalWrite(motorDerB, LOW);
}

// Función para girar a la izquierda
void girarIzquierda() {
  servoMotor.write(135); // Girar el servo a la izquierda
  delay(1000);           // Esperar para que el carro gire
  servoMotor.write(90);  // Volver a posición recta
}

// Función para girar a la derecha
void girarDerecha() {
  servoMotor.write(45);  // Girar el servo a la derecha
  delay(1000);           // Esperar para que el carro gire
  servoMotor.write(90);  // Volver a posición recta
}

// Función para detener los motores
void detenerMotores() {
  digitalWrite(motorIzqA, LOW);
  digitalWrite(motorIzqB, LOW);
  digitalWrite(motorDerA, LOW);
  digitalWrite(motorDerB, LOW);
}

void setup() {
  // Configurar pines de motores, sensores ultrasónicos y botón
  pinMode(motorIzqA, OUTPUT);
  pinMode(motorIzqB, OUTPUT);
  pinMode(motorDerA, OUTPUT);
  pinMode(motorDerB, OUTPUT);
  pinMode(trigPinIzq, OUTPUT);
  pinMode(echoPinIzq, INPUT);
  pinMode(trigPinDer, OUTPUT);
  pinMode(echoPinDer, INPUT);
  pinMode(botonPin, INPUT_PULLUP);  // Configurar el botón con pull-up interno

  // Iniciar el servo motor
  servoMotor.attach(11); // Pin del servomotor
  servoMotor.write(90);  // Posición recta

  // Iniciar el puerto serial
  Serial.begin(115200);
}

void loop() {
  // Leer el estado del botón
  if (digitalRead(botonPin) == LOW && !carroActivo) {  // Comienza al presionar el botón
    delay(200);  // Evitar rebotes del botón
    carroActivo = true;
    tiempoInicio = millis();  // Iniciar el tiempo de la carrera
  }

  // Si el carro está activo, ejecutar la lógica de movimiento
  if (carroActivo) {
    // Leer las distancias con los sensores ultrasónicos
    distanciaIzq = medirDistancia(trigPinIzq, echoPinIzq);
    distanciaDer = medirDistancia(trigPinDer, echoPinDer);

    // Verificar si hay datos disponibles en el puerto serial para los colores
    if (Serial.available() > 0) {
      String comando = Serial.readStringUntil('\n');  // Leer el comando hasta un salto de línea

      // Comandos para girar el servomotor según el color detectado
      if (comando == "Block_Verde") {
        girarIzquierda();  // Girar 45 grados a la izquierda
        Serial.println("Servomotor girado 45 grados a la izquierda");
      } else if (comando == "Block_Rojo") {
        girarDerecha();  // Girar 45 grados a la derecha
        Serial.println("Servomotor girado 45 grados a la derecha");
      } else {
        Serial.println("Comando no reconocido");
      }
    }

    // Si detecta una pared a más de 60 cm en el lado izquierdo o derecho, girar
    if (distanciaIzq > 60) {
      girarIzquierda();
    } else if (distanciaDer > 60) {
      girarDerecha();
    } else {
      avanzarRecto();  // Avanzar cuando no hay obstáculos
    }

    // Contar vueltas basado en el tiempo (se puede mejorar con un encoder)
    if (millis() - tiempoInicio >= tiempoTotalVuelta) {
      vueltas++;
      tiempoInicio = millis();  // Reiniciar el tiempo para la siguiente vuelta
    }

    // Si se han completado 3 vueltas, detener el carro
    if (vueltas >= 3) {
      detenerMotores();
      carroActivo = false;  // Detener el carro
      while (true);  // Detener el programa
    }
  }
}
