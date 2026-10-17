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

// Segunda dirección de cada motor, va al IN2 y al IN4 del L298 en lugar de las NOT
const int motor_left_dir2 = 11;
const int motor_right_dir2 = 7;

// Sensores de oponente (Digitales: 1 detecta, 0 no detecta)
const int S1_izq = A2;
const int S2_frent = A1;
const int S3_der = A0;

// Sensor de línea (Analógico)
const int sensar_linea = A3;

// Puerto analógico para números random, queda sin conectar
const int pin_random = A5;

// --- CONSTANTES DE COMPORTAMIENTO ---
const int MAX_VEL_FRENT = 255; // Velocidad máxima
const int VEL_GIRO = 150;      // Velocidad reducida para no derrapar al girar
const int UMBRAL_LINEA = 500;  // Valor a partir del cual consideramos que pisó la línea
const int VEL_AVANCE_INICIAL = 250; // Velocidad para avanzar recto en el round 1
const int TIEMPO_AL_CENTRO = 300;   // Tiempo para recorrer unos 14 cm, hay que ajustarlo con la velocidad real

// Guardamos cómo estaban los botones en la vuelta anterior
int estadoAnteriorDer = HIGH;
int estadoAnteriorIzq = HIGH;

// Recién empezamos a buscar cuando terminó la estrategia 1 o la 2
bool estrategiaTerminada = false;

void setup() {
    Serial.begin(9600);

    // Usamos el ruido del A5 sin conectar para que los números random cambien cada vez
    randomSeed(analogRead(pin_random));

    pinMode(LedRojo, OUTPUT);
    pinMode(LedVerde, OUTPUT);

    // Los botones van a tierra, así que usamos la resistencia interna del Arduino
    pinMode(btn_der, INPUT_PULLUP);
    pinMode(btn_izq, INPUT_PULLUP);

    pinMode(motor_right_dir, OUTPUT);
    pinMode(vel_motor_rigth, OUTPUT);

    pinMode(motor_left_dir, OUTPUT);
    pinMode(vel_motor_left, OUTPUT);

    pinMode(motor_left_dir2, OUTPUT);
    pinMode(motor_right_dir2, OUTPUT);

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
    // Los botones dan LOW cuando se aprietan
    int botonDer = digitalRead(btn_der);
    int botonIzq = digitalRead(btn_izq);

    // Mostramos el mensaje una sola vez cada vez que se aprieta el botón
    if (botonDer == LOW && estadoAnteriorDer == HIGH) {
        Serial.println("estrategia 1");

        // Round 1, arrancamos en el centro y avanzamos recto hasta sensar la línea blanca
        // Desde el centro hasta la línea son 13 cm como máximo
        moverMotores(VEL_AVANCE_INICIAL, VEL_AVANCE_INICIAL);
        while (analogRead(sensar_linea) <= UMBRAL_LINEA) {
        }
        // Cuando la ve sigue el loop y la parte de la línea lo hace retroceder
        estrategiaTerminada = true;
    }
    if (botonIzq == LOW && estadoAnteriorIzq == HIGH) {
        Serial.println("estrategia 2");

        // Round 3, arrancamos en el borde mirando al rival y avanzamos hasta la mitad del dohyo
        // Acá no miramos la línea porque arrancamos parados sobre ella
        // Desde el borde hasta la mitad son unos 14 cm
        realizarManiobra(VEL_AVANCE_INICIAL, VEL_AVANCE_INICIAL, TIEMPO_AL_CENTRO);
        estrategiaTerminada = true;
    }
    estadoAnteriorDer = botonDer;
    estadoAnteriorIzq = botonIzq;

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
        // 3. MODO BÚSQUEDA
        digitalWrite(LedVerde, LOW);
        if (estrategiaTerminada) {
            busqueda();
        } else {
            moverMotores(0, 0); // Hasta que termina la estrategia se queda quieto esperando
        }
    } 
    
    // Pequeña pausa para estabilidad del simulador
    delay(50);
}

// --- FUNCIONES AUXILIARES ---

// Búsqueda, avanza despacio y si sigue sin ver al rival gira para un lado al azar
void busqueda() {
    // Avance búsqueda
    realizarManiobra(100, 100, 1000);

    // Volvemos a leer los sensores después de avanzar
    bool izq = digitalRead(S1_izq);
    bool frente = digitalRead(S2_frent);
    bool der = digitalRead(S3_der);

    // Si ninguno ve al rival sorteamos para qué lado girar
    if (!izq && !frente && !der) {
        int direccion_giro = random(0, 2);
        if (direccion_giro == 0) {
            Serial.println("buscando hacia la derecha");
            realizarManiobra(250, 0, 1000);
        } else {
            Serial.println("buscando hacia la izquierda");
            realizarManiobra(0, 250, 1000);
        }
    }
    // Al volver al loop se leen los sensores de nuevo
}

// Función para simplificar el control de tiempo y detención
void realizarManiobra(int velIzq, int velDer, int tiempoMs) {
    moverMotores(velIzq, velDer);
    delay(tiempoMs);
    moverMotores(0, 0); // Detiene los motores al terminar el tiempo
}

// Función que traduce velocidades (positivas o negativas) a señales de los pines
// El L298 necesita las dos direcciones de cada motor al revés una de la otra
void moverMotores(int velIzq, int velDer) {
    // Control Motor Izquierdo
    if (velIzq >= 0) {
        digitalWrite(motor_left_dir, HIGH); // Adelante
        digitalWrite(motor_left_dir2, LOW);
    } else {
        digitalWrite(motor_left_dir, LOW);  // Atrás
        digitalWrite(motor_left_dir2, HIGH);
        velIzq = -velIzq;                   // Convertir a positivo para el PWM
    }
    analogWrite(vel_motor_left, velIzq);

    // Control Motor Derecho
    if (velDer >= 0) {
        digitalWrite(motor_right_dir, HIGH); // Adelante
        digitalWrite(motor_right_dir2, LOW);
    } else {
        digitalWrite(motor_right_dir, LOW);  // Atrás
        digitalWrite(motor_right_dir2, HIGH);
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