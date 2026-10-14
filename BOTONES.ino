// Definición de pines según las especificaciones del profesor
const int LedRojo = 6;
const int LedVerde = 4;

// Botones de control
const int btn_der = 8;
const int btn_izq = 12;

// Motor derecho
const int motor_right_dir = 13;
const int vel_motor_rigth = 5;

// Motor izquierdo
const int motor_left_dir = 10;
const int vel_motor_left = 9;

// Sensores de oponente (Digitales: 1 detecta, 0 no detecta)
const int S1_izq = A2;
const int S2_frent = A1;
const int S3_der = A0;

// Sensor de línea (Analógico)
const int sensar_linea = A3;

// --- CONSTANTES DE COMPORTAMIENTO ---
const int MAX_VEL_FRENT = 255; // Velocidad máxima
const int VEL_GIRO = 150;      // Velocidad reducida para no derrapar al girar
const int UMBRAL_LINEA = 500;  // Valor a partir del cual consideramos que pisó la línea

void setup() {
    Serial.begin(9600);

    pinMode(LedRojo, OUTPUT);
    pinMode(LedVerde, OUTPUT);

    pinMode(btn_der, INPUT);
    pinMode(btn_izq, INPUT);

    pinMode(motor_right_dir, OUTPUT);
    pinMode(vel_motor_rigth, OUTPUT);

    pinMode(motor_left_dir, OUTPUT);
    pinMode(vel_motor_left, OUTPUT);

    pinMode(S1_izq, INPUT);
    pinMode(S2_frent, INPUT);
    pinMode(S3_der, INPUT);

    Serial.println("Sistema iniciado - Iniciando prueba de motores");

    // Ejecutar la secuencia de prueba inicial
    pruebaMotoresProteus();

    // Detener los motores tras la prueba
    moverMotores(0, 0);
    Serial.println("INICIANDO MODO COMBATE...");
}

void loop() {
    int lecturaLinea = analogRead(sensar_linea);

    bool izq = digitalRead(S1_izq);
    bool frente = digitalRead(S2_frent);
    bool der = digitalRead(S3_der);

    // 1. DETECCIÓN DE LÍNEA (Prioridad absoluta - Supervivencia)
    // Asumimos que un valor alto significa que encontró el borde blanco
    if (lecturaLinea > UMBRAL_LINEA) {
        digitalWrite(LedRojo, HIGH);
        digitalWrite(LedVerde, LOW);
        Serial.println("¡ALARMA! LINEA DETECTADA: RETROCEDER");
        
        // Retrocede a máxima velocidad por 1 segundo (2 segundos puede ser mucho y salir por el otro lado)
        realizarManiobra(-MAX_VEL_FRENT, -MAX_VEL_FRENT, 1000); 
        return; // Obliga a reiniciar el loop y volver a leer sensores inmediatamente
    }

    // Si no hay peligro de caerse, apagamos el LED rojo
    digitalWrite(LedRojo, LOW);

    // 2. DETECCIÓN DE OPONENTE (Ataque)
    if (frente) {
        digitalWrite(LedVerde, HIGH);
        Serial.println("OPONENTE AL FRENTE: ATAQUE FRONTAL");
        realizarManiobra(MAX_VEL_FRENT, MAX_VEL_FRENT, 2000); 
    } 
    else if (izq) {
        digitalWrite(LedVerde, HIGH);
        Serial.println("OPONENTE IZQ: GIRO 45 GRADOS");
        realizarManiobra(-VEL_GIRO, VEL_GIRO, 1000); // Gira sobre su eje hacia la izquierda
        
        Serial.println("EMPUJE FRONTAL");
        realizarManiobra(MAX_VEL_FRENT, MAX_VEL_FRENT, 1000); // Empuja por 1 segundo
    } 
    else if (der) {
        digitalWrite(LedVerde, HIGH);
        Serial.println("OPONENTE DER: GIRO 45 GRADOS");
        realizarManiobra(VEL_GIRO, -VEL_GIRO, 1000); // Gira sobre su eje hacia la derecha
        
        Serial.println("EMPUJE FRONTAL");
        realizarManiobra(MAX_VEL_FRENT, MAX_VEL_FRENT, 1000); // Empuja por 1 segundo
    } 
    else {
        // 3. MODO BÚSQUEDA / GUARDIA
        digitalWrite(LedVerde, LOW);
        moverMotores(0, 0); // Se queda quieto esperando
    } 
    
    // Pequeña pausa para estabilidad del simulador
    delay(50);
}

// --- FUNCIONES AUXILIARES ---

// Función para simplificar el control de tiempo y detención
void realizarManiobra(int velIzq, int velDer, int tiempoMs) {
    moverMotores(velIzq, velDer);
    delay(tiempoMs);
    moverMotores(0, 0); // Detiene los motores al terminar el tiempo
}

// Función que traduce velocidades (positivas o negativas) a señales de los pines
void moverMotores(int velIzq, int velDer) {
    // Control Motor Izquierdo
    if (velIzq >= 0) {
        digitalWrite(motor_left_dir, HIGH); // Adelante
    } else {
        digitalWrite(motor_left_dir, LOW);  // Atrás
        velIzq = -velIzq;                   // Convertir a positivo para el PWM
    }
    analogWrite(vel_motor_left, velIzq);

    // Control Motor Derecho
    if (velDer >= 0) {
        digitalWrite(motor_right_dir, HIGH); // Adelante
    } else {
        digitalWrite(motor_right_dir, LOW);  // Atrás
        velDer = -velDer;                    // Convertir a positivo para el PWM
    }
    analogWrite(vel_motor_rigth, velDer);
}

// Función de prueba inicial
void pruebaMotoresProteus() {
    Serial.println("PRUEBA: MOTOR DERECHO ADELANTE");
    realizarManiobra(0, MAX_VEL_FRENT, 500);

    Serial.println("PRUEBA: MOTOR DERECHO ATRAS");
    realizarManiobra(0, -MAX_VEL_FRENT, 500);

    Serial.println("PRUEBA: MOTOR IZQUIERDO ADELANTE");
    realizarManiobra(MAX_VEL_FRENT, 0, 500);

    Serial.println("PRUEBA: MOTOR IZQUIERDO ATRAS");
    realizarManiobra(-MAX_VEL_FRENT, 0, 500);

    Serial.println("PRUEBA: AMBOS MOTORES ADELANTE");
    realizarManiobra(MAX_VEL_FRENT, MAX_VEL_FRENT, 500);

    Serial.println("PRUEBA: AMBOS MOTORES ATRAS");
    realizarManiobra(-MAX_VEL_FRENT, -MAX_VEL_FRENT, 500);

    Serial.println("PRUEBA: UNO PARA CADA LADO");
    realizarManiobra(MAX_VEL_FRENT, -MAX_VEL_FRENT, 500);

    Serial.println("PRUEBA FINALIZADA - MOTORES APAGADOS");
}
