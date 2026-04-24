#include "Vasloth_FlowSensor.h"

// Configuraciones optimizadas para Vasloth V2.8
const int UMBRAL_ON  = 105;
const int UMBRAL_OFF = 75;
const int PERSISTENCIA_ON  = 150;
const int PERSISTENCIA_OFF = 5;
const int MUESTRAS_VENTANA = 100;
const int MICRO_DELAY      = 30;

VaslothFlowSensor::VaslothFlowSensor(int pin0, int pin1) {
    _pin0 = pin0; _pin1 = pin1;
    _contadorPersistencia = 0;
    _flujoActivo = false;
    _indiceProximo = 0;
}

void VaslothFlowSensor::begin() {
    pinMode(_pin0, INPUT);
    pinMode(_pin1, INPUT);
    for(int i=0; i<CANT_PROMEDIO; i++) _lecturasPasadas[i] = 0;
}

int VaslothFlowSensor::_obtenerEnergiaCruda() {
    int max0 = 0, min0 = 4095, max1 = 0, min1 = 4095;
    for (int i = 0; i < MUESTRAS_VENTANA; i++) {
        int v0 = analogRead(_pin0);
        int v1 = analogRead(_pin1);
        if (v0 > max0) max0 = v0; if (v0 < min0) min0 = v0;
        if (v1 > max1) max1 = v1; if (v1 < min1) min1 = v1;
        delayMicroseconds(MICRO_DELAY);
    }
    return (max(max0 - min0, max1 - min1));
}

void VaslothFlowSensor::update() {
    int energiaActual = _obtenerEnergiaCruda();
    _lecturasPasadas[_indiceProximo] = energiaActual;
    _indiceProximo = (_indiceProximo + 1) % CANT_PROMEDIO;

    long suma = 0;
    for(int i=0; i<CANT_PROMEDIO; i++) suma += _lecturasPasadas[i];
    int promedioLargo = suma / CANT_PROMEDIO;

    // Algoritmo VSL-Persist
    if (!_flujoActivo) {
        if (promedioLargo >= UMBRAL_ON) {
            _contadorPersistencia++;
            if (_contadorPersistencia >= PERSISTENCIA_ON) {
                _flujoActivo = true;
                _contadorPersistencia = 0;
            }
        } else { _contadorPersistencia = 0; }
    } else {
        if (promedioLargo < UMBRAL_OFF) {
            _contadorPersistencia++;
            if (_contadorPersistencia >= PERSISTENCIA_OFF) {
                _flujoActivo = false;
                _contadorPersistencia = 0;
            }
        } else { _contadorPersistencia = 0; }
    }
}

bool VaslothFlowSensor::isFlowing() { return _flujoActivo; }
int VaslothFlowSensor::getAverageEnergy() {
    long suma = 0;
    for(int i=0; i<CANT_PROMEDIO; i++) suma += _lecturasPasadas[i];
    return suma / CANT_PROMEDIO;
}