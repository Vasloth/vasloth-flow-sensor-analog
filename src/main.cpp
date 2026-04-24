#include <Arduino.h>
#include "Vasloth_FlowSensor.h" 

// Instancia del sensor utilizando los pines de la PCB V2.8
VaslothFlowSensor flowSensor(14, 27);

void setup() {
    Serial.begin(115200);
    
    // Configuración de resolución ADC para ESP32
    analogReadResolution(12); 
    
    // Inicialización del sensor y buffers internos
    flowSensor.begin();
    
    delay(2000);
    Serial.println("--- VASLOTH: System Initialized ---");
    Serial.println("[INFO] Monitoring acoustic signature...");
}

void loop() {
    // Procesa las señales y aplica la lógica de persistencia
    flowSensor.update(); 

    // Verificación de estado mediante la API de la librería
    if (flowSensor.isFlowing()) {
        static bool lastState = false;
        if (!lastState) {
            Serial.println("[!] ESTADO: MOVIMIENTO DETECTADO");
            lastState = true;
        }
    } else {
        static bool lastState = true;
        if (lastState) {
            Serial.println("[.] ESTADO: REPOSO");
            lastState = false;
        }
    }

    // Telemetría de diagnóstico (opcional)
    static unsigned long lastDebug = 0;
    if (millis() - lastDebug > 500) {
        Serial.printf("Energy Avg: %d | Status: %s\n", 
                      flowSensor.getAverageEnergy(), 
                      flowSensor.isFlowing() ? "FLOWING" : "IDLE");
        lastDebug = millis();
    }
}