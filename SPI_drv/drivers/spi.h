#ifndef SPI_H_
#define SPI_H_

#include "stdint.h"
#include "stdbool.h"

/*
Necesitamos:
    Estructura de Clock
        Polarity (enum)
        Phase      (enum)
        Scaler  (enum)

    enum de Bit Order
        MSB First
        LSB First

    enum de SS Polarity
        Active High
        Active Low

    enum de los IDs de los slaves

    enum de las instancias de SPI
    
    Estructura de Config
        baudRate
        frameSize (Que seria?)
        ClockConfig (polarity, phase, scaler)
        BitOrder
        SSPolarity

    Funciones:
        SPI Init
        SPI Send
        SPI Receive (hace falta?)
        SPI onTransferComplete (hace falta?)
*/




#endif /* SPI_H_ */

