#ifndef VASLOTH_FLOWSENSOR_H
#define VASLOTH_FLOWSENSOR_H

#include <Arduino.h>

class VaslothFlowSensor {
public:
    VaslothFlowSensor(int pin0, int pin1);
    void begin();
    void update();
    bool isFlowing();
    int getAverageEnergy();

private:
    int _pin0, _pin1, _contadorPersistencia;
    bool _flujoActivo;
    static const int CANT_PROMEDIO = 30;
    int _lecturasPasadas[CANT_PROMEDIO];
    int _indiceProximo;
    int _obtenerEnergiaCruda();
};

#endif