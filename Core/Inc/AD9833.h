#ifndef _AD9833_H_
#define _AD9833_H_
/*
*																										 Драйвер управления микросхемой AD9833
* Для работы с библиотекой необходимо настроить аппаратный модуль SPI
* размер данных 8 бит, Clock Polarity - High, Clock Phase - 1 Edge, MSB First.
*** Пример работы с  AD9833 ***
*
* // Инициализация AD9833
*  ad9833_init();
*
* // Установка регистра частоты 0 (FREQ_0) на частоту 5кГц:
*  ad9833_set_frequency(0, 5000);
*
* // Выбор регистра частоты 0:
*  ad9833_set_freq_out(0);
*
* // Установка формы сигнала и включение генерации:
*  ad9833_set_mode(AD_SINE);
*
* Для остановки генерации сигнала выбрать режим OFF:
* ad9833_set_mode(AD_OFF);
*
* С помощью соответствующих функций возможно изменение 
* фазы сигнала, по умолчанию фаза = 0.
*
*/
#include "stm32l4xx_hal.h"

/* Для работы библиотеки необходима настройка
	 аппаратного модуля SPI*/
extern SPI_HandleTypeDef hspi2;  //																																		<-- Проверить

/*
* NOP для маленьких задержек
 */
#define NOP()		__NOP()
/*
* Макросы управления входом FSYNC микросхемы AD9833 (чип селект).
* Ножки микроконтроллера должны быть сконфигурированны на выход: Push-Pull
 */
#define AD_FSYNC_PORT 	GPIOA						// порт для управления FSYNC																	<-- Проверить
#define AD_FSYNC_PIN		GPIO_PIN_2			// пин для управления FSYNC																		<-- Проверить
#define AD_FSYNC_HI() 	HAL_GPIO_WritePin(AD_FSYNC_PORT, AD_FSYNC_PIN, GPIO_PIN_SET);
#define AD_FSYNC_LO() 	HAL_GPIO_WritePin(AD_FSYNC_PORT, AD_FSYNC_PIN, GPIO_PIN_RESET);


/*
* Макросы регистра частоты и фазы из котороых будет идти генерация сигнала  
 */
 #define AD_FREQ_0			0
 #define AD_FREQ_1			1
 #define AD_PHASE_0			0
 #define AD_PHASE_1			1
/* 
*	Режимы вывода формы сигнала AD9833
* Параметры для ad9833SetMode() 
 */
#define AD_OFF      0
#define AD_TRIANGLE 1
#define AD_SQUARE   2
#define AD_SINE     3

/*
*	Биты регистра команд микросхемы AD9833 
 */
#define AD_B28     13
#define AD_HLB     12
#define AD_FSELECT 11
#define AD_PSELECT 10
#define AD_RESET   8
#define AD_SLEEP1  7
#define AD_SLEEP12 6
#define AD_OPBITEN 5
#define AD_DIV2    3
#define AD_MODE    1

/*
* Макросы для адресации регистров частоты и фазы AD9833
* Два регистра частоты и два регистра фазы
 */
#define AD_FREQ0  (1 << 14)
#define AD_FREQ1  (1 << 15)
#define AD_PHASE0 (3 << 14)
#define AD_PHASE1 ((3 << 14)|(1 << 13))

/*
* Макросы для вычислений необходимой частоты и фазы AD9833
 */
#define AD_F_MCLK 25000000    // Опорная частота тактирования микросхемы ad9833												<-- Проверить
#define AD_2POW28 268435456   /* Константа используется для вычисления выходной 
																 частоты (2^28, 28-разрядные регистры частоты) */
 
/* Макрос, который вычисляет значение регистра частоты для заданной частоты */
#define AD_FREQ_CALC(freq) (uint32_t)(((float)AD_2POW28 / (float)AD_F_MCLK) * (float)freq)
	
/*
* Макрос, который вычисляет значение регистра фазы ad9833
* по заданному значению фазы в градусах */
#define AD_PHASE_CALC(phase_deg) (uint16_t)((float)651.8986 * (float)phase_deg)
	
/* 
* Структура, в которой хранятся все значения конфигурации, инициализированные
* в глобальной переменной файла ad9833.c
 */
typedef struct{
    float freq[2];   // Здесь хранятся частоты
    float phase[2];	// Здесь хранятся фазы
    float mod_freq;
    uint8_t freq_out;	
    uint8_t phase_out;
    uint8_t mode;
    uint16_t command_reg;
} ad9833_settings_t;

// Инициализация модуля
void ad9833_init(void);
// Установка формы сигнала и включение генерации:
void ad9833_set_mode(uint8_t mode);
//// Установка и получение частоты в(из) выбранный(го) регистр(а) reg = 0 или 1 FREQ_0 или FREQ_1
void ad9833_set_frequency(uint8_t reg, float freq);
float ad9833_get_frequency(uint8_t reg);
// Установка и получение фазы в(из) выбранный(го) регистр(а) reg = 0 или 1 PHASE_0 или PHASE_1 
void ad9833_set_phase(uint8_t reg, float phase);
float ad9833_get_phase(uint8_t reg);
// Установка и получение номера регистра из которого генерируется частота freq_out = 0 или 1 FREQ_0 или FREQ_1 
void ad9833_set_freq_out(uint8_t freq_out);
uint8_t ad9833_get_freq_out(void);
// Установка и получение номера регистра из которого добавляется фаза сигнала freq_out = 0 или 1 PHASE_0 или PHASE_1
void ad9833_set_phase_out(uint8_t phase_out);
uint8_t ad9833_get_phase_out(void);

#endif  //def _AD9833_H_

