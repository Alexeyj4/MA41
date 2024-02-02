#include "AD9833.h"

/* 
* Структура в которой хранятся настройки микросхемы AD9833
 */
ad9833_settings_t ad_settings;

/*
* Функция для отправки 16-битных пакетов SPI
 */
void writeSPI(uint16_t packet){
	uint8_t txData[2];
	txData[0] = (uint8_t)(packet >> 8);
	txData[1] = (uint8_t)packet;
	HAL_SPI_Transmit(&hspi2, (uint8_t*)txData, 2, 500);
	__NOP();
}
/*
* Фкнкция инициализации микросхемы AD9833 
* и инициализация полей структуры ad_settings
 */
void ad9833_init(void){
	AD_FSYNC_HI();																											// отпускаем микросхему
	HAL_Delay(10);																											// ждём
	ad_settings.command_reg |= ((1 << AD_SLEEP12)|(1 << AD_RESET));			// модифицируем регистр команд (отключаем DAC микросхемы и сбрасываем состояние внутренних регистров)
	AD_FSYNC_LO();
	writeSPI(ad_settings.command_reg);																	// записываем в микросхему 
	AD_FSYNC_HI();
/* сбрасываем регистры частоты и фазы */
	ad9833_set_frequency(0, 0);
  ad9833_set_frequency(1, 0);
  ad9833_set_phase(0, 0);
  ad9833_set_phase(1, 0);
  ad9833_set_freq_out(0);
  ad9833_set_phase_out(0);
	ad_settings.command_reg &= ~(1 << AD_RESET);												// сбрасываем бит RESET		
	AD_FSYNC_LO();
	writeSPI(ad_settings.command_reg);																	// записываем в микросхему
	AD_FSYNC_HI();
}


/*
* установка формы выходного сигнала ad9833
* mode возможные значения:
*  - AD_OFF - отклюает DAC
*  - AD_TRIANGLE - устанавливаем режим и запускает DAC
*  - AD_SQUARE	- устанавливаем режим и запускает DACм
*  - AD_SINE	- устанавливаем режим и запускает DAC
 */
void ad9833_set_mode(uint8_t mode){
  ad_settings.mode = mode;
   switch (mode){
		case AD_OFF:
			ad_settings.command_reg |= (1 << AD_SLEEP12);
			ad_settings.command_reg |= (1 << AD_SLEEP1);
		break;
		case AD_TRIANGLE:
			ad_settings.command_reg &= ~(1 << AD_OPBITEN);
			ad_settings.command_reg |=  (1 << AD_MODE);
			ad_settings.command_reg &= ~(1 << AD_SLEEP12);
			ad_settings.command_reg &= ~(1 << AD_SLEEP1);
		break;
		case AD_SQUARE:
			ad_settings.command_reg |=  (1 << AD_OPBITEN);
			ad_settings.command_reg &= ~(1 << AD_MODE);
			ad_settings.command_reg |=  (1 << AD_DIV2);
			ad_settings.command_reg &= ~(1 << AD_SLEEP12);
			ad_settings.command_reg &= ~(1 << AD_SLEEP1);
				break;
		case AD_SINE:
			ad_settings.command_reg &= ~(1 << AD_OPBITEN);
			ad_settings.command_reg &= ~(1 << AD_MODE);
			ad_settings.command_reg &= ~(1 << AD_SLEEP12);
			ad_settings.command_reg &= ~(1 << AD_SLEEP1);
		break;
  }
/* записывем в микросхему */
	AD_FSYNC_LO();
  writeSPI(ad_settings.command_reg);
	AD_FSYNC_HI();
}


/*
* Устанавливает выбранный регистр частоты ad9833
* reg регистр частоты, 0 или 1
* freq частота которую необходимо установить
 */
	void ad9833_set_frequency(uint8_t reg, float freq){
	uint32_t freq_reg;
	uint16_t set_reg;
	freq_reg = AD_FREQ_CALC(freq); 										// пересчитываем частоту на понятную микросхеме
	ad_settings.freq[reg] = freq;											// запоминаем что сделали
/* выбираем нужный регистр в микросхеме */	
	if(reg == 0)
		set_reg = AD_FREQ0;
	else
		set_reg = AD_FREQ1;
/* записываем полученные данные */	
	AD_FSYNC_LO();
	writeSPI((1 << AD_B28) | ad_settings.command_reg);							// установка бита B28 в 1 позволяет записать сразу 16 бит в регистр частоты
  writeSPI(set_reg | (0x3FFF & (uint16_t) freq_reg));							// пишем младшую (LSB) часть регистра частоты
  writeSPI(set_reg | (0x3FFF & (uint16_t)(freq_reg >> 14)));			// пишем старшую (MSB) часть регистра частоты
	AD_FSYNC_HI();
}


/*
* Устанавливает выбранный регистр фазы ad9833
* reg регистр фазы, 0 или 1
* phase фаза которую необходимо установить
 */
void ad9833_set_phase(uint8_t reg, float phase){
	uint16_t set_reg;
/* выбираем нужный регистр в микросхеме */
	if(reg == 0)
		set_reg = AD_PHASE0;
	else
		set_reg = AD_PHASE1;
/* запоминаем что сделали */
	ad_settings.phase[reg] = phase;
/* пересчитываем фазу на понятную микросхеме и записываем в неё */
	AD_FSYNC_LO();
	writeSPI(set_reg | AD_PHASE_CALC(phase));
	AD_FSYNC_HI();
}


/*
* Выбирает регистр частоты, который используется 
* для генерации выходного сигнала
*  - 0 = использовать регистр частоты 0
*  - 1 = использовать регистр частоты 1
 */
void ad9833_set_freq_out(uint8_t freq_out){
	ad_settings.freq_out = freq_out;
	/* модифицируем регистр commandв соответствии с 
	 выбранным регистроми и записывем в микросхему */
	switch(freq_out){
		case 0:
			ad_settings.command_reg &= ~(1 << AD_FSELECT);
		break;
		case 1:
			ad_settings.command_reg |= (1 << AD_FSELECT);
		break;
  }
	AD_FSYNC_LO();
  writeSPI(ad_settings.command_reg);
	AD_FSYNC_HI();
}


/*
* Выбирает регистр фазы, который используется 
* для генерации выходного сигнала
*  - 0 = использовать регистр фазы 0
*  - 1 = использовать регистр фазы 1
 */
void ad9833_set_phase_out(uint8_t phase_out){
	ad_settings.phase_out = phase_out;
/* модифицируем регистр command в соответствии с 
	 выбранным регистроми записывем в микросхему */
	switch (phase_out){
		case 0:
			ad_settings.command_reg &= ~(1 << AD_PSELECT);
		break;
		case 1:
			ad_settings.command_reg |= (1 << AD_PSELECT);
		break;
	}
	AD_FSYNC_LO();
  writeSPI(ad_settings.command_reg);
	AD_FSYNC_HI();
}


/*
* Возвращает фазу выбранного регистра
* reg - регистр, фазу которого нужно получить 0 или 1
 */
float ad9833_get_phase(uint8_t reg){
	return ad_settings.phase[reg];
}


/*
* Возвращает номер регистра из которого
* генерируется частота: FREQ_0 - 0, FREQ_1 - 1 
 */
uint8_t ad9833_get_freq_out(void){
	return ad_settings.freq_out;
}


/*
* Возвращает номер регистра из которого
* добавляется фаза к сигналу: PHASE_0 - 0, PHASE_1 - 1 
 */
uint8_t ad9833_get_phase_out(void){
	return ad_settings.phase_out;
}


/*
* Возвращает частоту выбранного регистра
* reg - регистр, частоту которого нужно получить 0 или 1
 */
float ad9833_get_frequency(uint8_t reg){
	return ad_settings.freq[reg];
}

