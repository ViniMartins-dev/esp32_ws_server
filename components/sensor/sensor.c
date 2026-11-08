#include "sensor.h"

static int valor = 0;
static int direcao = 1;

int sensor_read(void)
{
    valor += direcao * 5;

    if (valor >= 100)
    {
        valor = 100;
        direcao = -1;
    }
    else if (valor <= 0)
    {
        valor = 0;
        direcao = 1;
    }

    return valor;
}