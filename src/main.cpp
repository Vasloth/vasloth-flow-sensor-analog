#include <Arduino.h>

// Configuración de Hardware (Pines ADC)
const int pinPiezo0 = 14; 
const int pinPiezo1 = 27; 

// --- LÓGICA DE DETECCIÓN (UMBRALES DINÁMICOS) ---
// Valores optimizados para detección por contacto externo en tuberías rígidas.
const int UMBRAL_ON  = 105;   // Umbral de activación (Superior al ruido base)
const int UMBRAL_OFF = 75;    // Umbral de desactivación (Histéresis técnica)

// --- CONFIGURACIÓN DE PERSISTENCIA (ALGORITMO VSL-PERSIST) ---
// Evita falsos positivos filtrando picos accidentales o vibraciones externas.
const int PERSISTENCIA_ON  = 150;   // Ciclos de confirmación para activar estado
const int PERSISTENCIA_OFF = 5;     // Ciclos de confirmación para retornar a reposo
int contadorPersistencia = 0;

// Configuración de Muestreo
const int MUESTRAS_VENTANA = 100;   // Ventana de muestreo por ciclo
const int MICRO_DELAY      = 30;    // Delay micro-segundal para captura de armónicos
const int CANT_PROMEDIO    = 30;    // Cantidad de muestras para suavizado de señal
int lecturasPasadas[CANT_PROMEDIO];
int indiceProximo = 0;

bool flujoActivo = false;

void setup() {
  Serial.begin(115200);
  analogReadResolution(12); // Resolución para ESP32
  pinMode(pinPiezo0, INPUT);
  pinMode(pinPiezo1, INPUT);
  
  for(int i=0; i<CANT_PROMEDIO; i++) lecturasPasadas[i] = 0;
  
  delay(2000);
  Serial.println("--- VASLOTH: Sensor de Flujo No Intrusivo V2.8 ---");
  Serial.println("[INFO] Algoritmo VSL-Persist Activo");
}

/**
 * Obtiene la energía diferencial capturada por la matriz piezoeléctrica.
 */
int obtenerEnergiaCruda() {
  int max0 = 0, min0 = 4095, max1 = 0, min1 = 4095;
  for (int i = 0; i < MUESTRAS_VENTANA; i++) {
    int v0 = analogRead(pinPiezo0);
    int v1 = analogRead(pinPiezo1);
    
    if (v0 > max0) max0 = v0; if (v0 < min0) min0 = v0;
    if (v1 > max1) max1 = v1; if (v1 < min1) min1 = v1;
    
    delayMicroseconds(MICRO_DELAY);
  }
  return (max(max0 - min0, max1 - min1));
}

void loop() {
  int energiaActual = obtenerEnergiaCruda();
  lecturasPasadas[indiceProximo] = energiaActual;
  indiceProximo = (indiceProximo + 1) % CANT_PROMEDIO;

  long suma = 0;
  for(int i=0; i<CANT_PROMEDIO; i++) suma += lecturasPasadas[i];
  int promedioLargo = suma / CANT_PROMEDIO;

  // --- LÓGICA DE PERSISTENCIA VASLOTH (VSL-PERSIST) ---
  if (!flujoActivo) {
    if (promedioLargo >= UMBRAL_ON) {
      contadorPersistencia++;
      if (contadorPersistencia >= PERSISTENCIA_ON) {
        flujoActivo = true;
        contadorPersistencia = 0; 
        Serial.println("[!] ESTADO: MOVIMIENTO DETECTADO (Confirmado)");
      }
    } else {
      contadorPersistencia = 0; // Reseteo ante baches de silencio
    }
  } else {
    if (promedioLargo < UMBRAL_OFF) {
      contadorPersistencia++;
      if (contadorPersistencia >= PERSISTENCIA_OFF) {
        flujoActivo = false;
        contadorPersistencia = 0;
        Serial.println("[.] ESTADO: REPOSO (Confirmado)");
      }
    } else {
      contadorPersistencia = 0; // Reseteo ante picos de ruido
    }
  }

  // Telemetría para Debugging y Calibración
  static unsigned long lastDebug = 0;
  if (millis() - lastDebug > 300) {
    Serial.printf("Inst: %d | Prom: %d | Conf: %d | Status: %s\n", 
                  energiaActual, promedioLargo, contadorPersistencia, 
                  flujoActivo ? ">>> MOVIMIENTO <<<" : "REPOSO");
    lastDebug = millis();
  }
}