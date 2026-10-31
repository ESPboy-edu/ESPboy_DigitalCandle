/*
    Optimized Interrupt-based SPI driver for ESP8266 (Xtensa LX106)
    - Zero ETS ROM attach overhead on transfers
    - Unrolled 64-byte FIFO filling
    - Direct APB write-strobe and register caching
    - Inlined isBusy polling
*/

#pragma once
#include <SPI.h>

#ifdef NBSPI_TDBG_IO
    #if NBSPI_TDBG_IO < 16
        #define NBSPI_TDBG_HIGH GPOS = (1<<NBSPI_TDBG_IO)
        #define NBSPI_TDBG_LOW GPOC = (1<<NBSPI_TDBG_IO)
    #else
        #error "Use IO 0-15!"
    #endif
#else
    #define NBSPI_TDBG_HIGH
    #define NBSPI_TDBG_LOW
#endif

static volatile uint16_t _nbspi_size = 0;
static volatile uint32_t * _nbspi_data = nullptr;
static volatile bool _nbspi_isbusy = false;
static bool _nbspi_attached = false;

static void IRAM_ATTR nbSPI_writeChunk();
void IRAM_ATTR nbSPI_ISR();

// Встраиваемая проверка без вызова функций через стек
static inline __attribute__((always_inline)) bool nbSPI_isBusy() {
    return _nbspi_isbusy || (SPI1CMD & SPIBUSY);
}

// Запуск отправки буфера любого размера
IRAM_ATTR void nbSPI_writeBytes(uint8_t *data, uint16_t size) {
    NBSPI_TDBG_HIGH;
    _nbspi_isbusy = true;
    _nbspi_size = size;
    _nbspi_data = (uint32_t*) data;

    SPI0S &= ~(0x1F); 
    SPI1S &= ~(0x1F);

    // Привязываем функцию-обработчик только один раз (это самая медленная операция)
    if (__builtin_expect(!_nbspi_attached, 0)) {
        ETS_SPI_INTR_ATTACH(nbSPI_ISR, NULL);
        _nbspi_attached = true;
    }
    
    // ОБЯЗАТЕЛЬНО: Открываем канал прерываний только на время отправки кадра
    ETS_SPI_INTR_ENABLE(); 

    nbSPI_writeChunk(); // Запись первого чанка
    NBSPI_TDBG_LOW;
}

// Отправка чанка до 64 байт (аппаратный лимит FIFO SPI)
static void IRAM_ATTR nbSPI_writeChunk() {    
    uint16_t rem = _nbspi_size;
    uint16_t chunk = (rem > 64) ? 64 : rem;
    rem -= chunk;
    _nbspi_size = rem; 

    const uint32_t mask = ~(SPIMMOSI << SPILMOSI);

    if (__builtin_expect(chunk == 64, 1)) {
        SPI1U1 = (SPI1U1 & mask) | (511 << SPILMOSI);

        volatile uint32_t *fifo = (volatile uint32_t *)&SPI1W0;
        const uint32_t *src = (const uint32_t *)_nbspi_data;

        fifo[0]  = src[0];  fifo[1]  = src[1];  fifo[2]  = src[2];  fifo[3]  = src[3];
        fifo[4]  = src[4];  fifo[5]  = src[5];  fifo[6]  = src[6];  fifo[7]  = src[7];
        fifo[8]  = src[8];  fifo[9]  = src[9];  fifo[10] = src[10]; fifo[11] = src[11];
        fifo[12] = src[12]; fifo[13] = src[13]; fifo[14] = src[14]; fifo[15] = src[15];

        _nbspi_data += 16;
   } else {
        uint32_t bits = (chunk << 3) - 1;
        SPI1U1 = (SPI1U1 & mask) | (bits << SPILMOSI);

        volatile uint32_t *fifo = (volatile uint32_t *)&SPI1W0;
        uint32_t *src = (uint32_t *)_nbspi_data;
        uint32_t words = (chunk + 3) >> 2;
        while (words--) {
            *fifo++ = *src++;
        }
        _nbspi_data = src;
    }

    __sync_synchronize();

    if (rem > 0) {
        SPI1S |= SPISTRIE; 
    } else {
        // ОБЯЗАТЕЛЬНО: Закрываем канал прерываний, чтобы флеш-память (SPI0) не сбивала экран
        ETS_SPI_INTR_DISABLE(); 
        _nbspi_isbusy = false;
    }

    SPI1CMD |= SPIBUSY; 
}

// Обработчик аппаратного прерывания SPI
IRAM_ATTR void nbSPI_ISR() {
    NBSPI_TDBG_HIGH;
    uint32_t ir = SPIIR; // Однократное чтение регистра прерываний с шины APB

    if (__builtin_expect(ir & (1 << SPII1), 1)) {
        SPI1S &= ~0x1F; // Сброс флагов SPI1
        nbSPI_writeChunk(); // Передача следующего блока
    }
    if (ir & (1 << SPII0)) {
        SPI0S &= ~0x1F; // Сброс флагов SPI0 (Flash), если возникли
    }
    NBSPI_TDBG_LOW;
}
