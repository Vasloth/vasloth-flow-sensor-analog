/**
 * @file Example_basic.ino
 * @author Vasloth
 * @brief Basic usage of the Vasloth Non-Intrusive Flow Sensor (V2.8).
 * * This example demonstrates how to initialize the sensor and detect 
 * fluid flow using the VSL-Persist algorithm.
 */

#include <Vasloth_FlowSensor.h>

// Instancia del sensor: Pines 14 y 27 (Matriz Piezoeléctrica Vasloth V2.8)
// El primer parámetro es Pin0 y el segundo es Pin1.
VaslothFlowSensor sensor(14, 27);

void setup() {
    // Iniciamos comunicación serie para telemetría
    Serial.begin(115200);
    
    // Configuración obligatoria para ESP32 (Resolución de 12 bits)
    analogReadResolution(12);
    
    // Inicialización del hardware del sensor y sus buffers internos
    sensor.begin();
    
    delay(2000);
    Serial.println("--- VASLOTH: Flow Monitoring System Initialized ---");
    Serial.println("[INFO] Calibrating acoustic signature...");
}

void loop() {
    // La función update() procesa la señal y aplica el algoritmo de persistencia.
    // Debe llamarse en cada ciclo del loop para no perder muestras de la firma armónica.
    sensor.update();

    // Verificamos si el algoritmo confirmó la presencia de flujo
    if (sensor.isFlowing()) {
        // El estado se mantiene activo mientras la firma de vibración sea constante
        static bool lastState = false;
        if (!lastState) {
            Serial.println("[!] ALERTA: Flujo detectado en la línea.");
            lastState = true;
        }
    } else {
        // Reposo confirmado por el algoritmo VSL-Persist
        static bool lastState = true;
        if (lastState) {
            Serial.println("[.] ESTADO: Sistema en reposo (Sin flujo).");
            lastState = false;
        }
    }

    // Telemetría de diagnóstico cada 500ms
    // Útil para calibrar la posición física del sensor en la tubería.
    static unsigned long lastDebug = 0;
    if (millis() - lastDebug > 500) {
        Serial.print("Energia Promedio: ");
        Serial.print(sensor.getAverageEnergy());
        Serial.print(" | Status: ");
        Serial.println(sensor.isFlowing() ? "FLOWING" : "IDLE");
        
        lastDebug = millis();
    }
}