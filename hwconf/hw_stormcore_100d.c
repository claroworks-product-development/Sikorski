/*
	Copyright 2016 Benjamin Vedder	benjamin@vedder.se

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "hw.h"
#include "ch.h"
#include "hal.h"
#include "stm32f4xx_conf.h"
#include "drv8323s.h"
#include "comm_can.h"
#include "mc_interface.h"
#include "ledpwm.h"


typedef enum {
	SWITCH_BOOTED = 0,
	SWITCH_TURN_ON_DELAY_ACTIVE,
	SWITCH_HELD_AFTER_TURN_ON,
	SWITCH_TURNED_ON,
	SWITCH_SHUTTING_DOWN,
} switch_states;

// Variables
static volatile bool i2c_running = false;
static THD_WORKING_AREA(smart_switch_thread_wa, 128);
static THD_WORKING_AREA(mux_thread_wa, 128);
static THD_WORKING_AREA(switch_color_thread_wa, 128);
static THD_FUNCTION(mux_thread, arg);
static THD_FUNCTION(switch_color_thread, arg);
static volatile switch_states switch_state = SWITCH_BOOTED;

static volatile float switch_bright = 0.75;



// I2C configuration
static const I2CConfig i2cfg = {
								OPMODE_I2C,
								100000,
								STD_DUTY_CYCLE
};

void hw_init_gpio(void) {
	// GPIO clock enable
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);




	// LEDs
	palSetPadMode(GPIOA, 8,
				  PAL_MODE_OUTPUT_PUSHPULL |
				  PAL_STM32_OSPEED_HIGHEST);
	palSetPadMode(GPIOC, 9,
				  PAL_MODE_OUTPUT_PUSHPULL |
				  PAL_STM32_OSPEED_HIGHEST);

	//Temp switches
	palSetPadMode(ADC_SW_EN_PORT, ADC_SW_EN_PIN,
				  PAL_MODE_OUTPUT_PUSHPULL |
				  PAL_STM32_OSPEED_HIGHEST);
	palSetPadMode(ADC_SW_1_PORT, ADC_SW_1_PIN ,
				  PAL_MODE_OUTPUT_PUSHPULL |
				  PAL_STM32_OSPEED_HIGHEST);
	palSetPadMode(ADC_SW_2_PORT, ADC_SW_2_PIN,
				  PAL_MODE_OUTPUT_PUSHPULL |
				  PAL_STM32_OSPEED_HIGHEST);
	palSetPadMode(ADC_SW_3_PORT, ADC_SW_3_PIN ,
				  PAL_MODE_OUTPUT_PUSHPULL |
				  PAL_STM32_OSPEED_HIGHEST);


	ENABLE_MOS_TEMP1();

	// GPIOC (ENABLE_GATE)
	palSetPadMode(GPIOE, 14,
				  PAL_MODE_OUTPUT_PUSHPULL |
				  PAL_STM32_OSPEED_HIGHEST);
	palSetPadMode(GPIOD, 4,
				  PAL_MODE_OUTPUT_PUSHPULL |
				  PAL_STM32_OSPEED_HIGHEST);
	DISABLE_GATE();
	// GPIOB (DCCAL)


	// GPIOA Configuration: Channel 1 to 3 as alternate function push-pull
	palSetPadMode(GPIOE, 8, PAL_MODE_ALTERNATE(GPIO_AF_TIM1) |
				  PAL_STM32_OSPEED_HIGHEST |
				  PAL_STM32_PUDR_FLOATING);
	palSetPadMode(GPIOE, 9, PAL_MODE_ALTERNATE(GPIO_AF_TIM1) |
				  PAL_STM32_OSPEED_HIGHEST |
				  PAL_STM32_PUDR_FLOATING);
	palSetPadMode(GPIOE, 10, PAL_MODE_ALTERNATE(GPIO_AF_TIM1) |
				  PAL_STM32_OSPEED_HIGHEST |
				  PAL_STM32_PUDR_FLOATING);

	palSetPadMode(GPIOE, 11, PAL_MODE_ALTERNATE(GPIO_AF_TIM1) |
				  PAL_STM32_OSPEED_HIGHEST |
				  PAL_STM32_PUDR_FLOATING);
	palSetPadMode(GPIOE, 12, PAL_MODE_ALTERNATE(GPIO_AF_TIM1) |
				  PAL_STM32_OSPEED_HIGHEST |
				  PAL_STM32_PUDR_FLOATING);
	palSetPadMode(GPIOE, 13, PAL_MODE_ALTERNATE(GPIO_AF_TIM1) |
				  PAL_STM32_OSPEED_HIGHEST |
				  PAL_STM32_PUDR_FLOATING);

	palSetPadMode(GPIOC, 6, PAL_MODE_ALTERNATE(GPIO_AF_TIM8) |
				  PAL_STM32_OSPEED_HIGHEST |
				  PAL_STM32_PUDR_FLOATING);
	palSetPadMode(GPIOC, 7, PAL_MODE_ALTERNATE(GPIO_AF_TIM8) |
				  PAL_STM32_OSPEED_HIGHEST |
				  PAL_STM32_PUDR_FLOATING);
	palSetPadMode(GPIOC, 8, PAL_MODE_ALTERNATE(GPIO_AF_TIM8) |
				  PAL_STM32_OSPEED_HIGHEST |
				  PAL_STM32_PUDR_FLOATING);

	palSetPadMode(GPIOB, 14, PAL_MODE_ALTERNATE(GPIO_AF_TIM8) |
				  PAL_STM32_OSPEED_HIGHEST |
				  PAL_STM32_PUDR_FLOATING);
	palSetPadMode(GPIOB, 15, PAL_MODE_ALTERNATE(GPIO_AF_TIM8) |
				  PAL_STM32_OSPEED_HIGHEST |
				  PAL_STM32_PUDR_FLOATING);
	palSetPadMode(GPIOA, 7, PAL_MODE_ALTERNATE(GPIO_AF_TIM8) |
				  PAL_STM32_OSPEED_HIGHEST |
				  PAL_STM32_PUDR_FLOATING);

	// Hall sensors
	palSetPadMode(HW_HALL_ENC_GPIO1, HW_HALL_ENC_PIN1, PAL_MODE_INPUT_PULLUP);
	palSetPadMode(HW_HALL_ENC_GPIO2, HW_HALL_ENC_PIN2, PAL_MODE_INPUT_PULLUP);
	palSetPadMode(HW_HALL_ENC_GPIO3, HW_HALL_ENC_PIN3, PAL_MODE_INPUT_PULLUP);
	palSetPadMode(HW_HALL_ENC_GPIO4, HW_HALL_ENC_PIN4, PAL_MODE_INPUT_PULLUP);
	palSetPadMode(HW_HALL_ENC_GPIO5, HW_HALL_ENC_PIN5, PAL_MODE_INPUT_PULLUP);
	palSetPadMode(HW_HALL_ENC_GPIO6, HW_HALL_ENC_PIN6, PAL_MODE_INPUT_PULLUP);

	// Fault pin
	palSetPadMode(GPIOE, 3, PAL_MODE_INPUT_PULLUP);
	palSetPadMode(GPIOD, 3, PAL_MODE_INPUT_PULLUP);

	// ADC Pins
	palSetPadMode(GPIOA, 0, PAL_MODE_INPUT_ANALOG);
	palSetPadMode(GPIOA, 1, PAL_MODE_INPUT_ANALOG);
	palSetPadMode(GPIOA, 2, PAL_MODE_INPUT_ANALOG);
	palSetPadMode(GPIOA, 3, PAL_MODE_INPUT_ANALOG);
	palSetPadMode(GPIOA, 4, PAL_MODE_INPUT_ANALOG);
	palSetPadMode(GPIOA, 5, PAL_MODE_INPUT_ANALOG);
	palSetPadMode(GPIOA, 6, PAL_MODE_INPUT_ANALOG);
	palSetPadMode(GPIOB, 0, PAL_MODE_INPUT_ANALOG);
	palSetPadMode(GPIOB, 1, PAL_MODE_INPUT_ANALOG);
	palSetPadMode(GPIOC, 0, PAL_MODE_INPUT_ANALOG);
	palSetPadMode(GPIOC, 1, PAL_MODE_INPUT_ANALOG);
	palSetPadMode(GPIOC, 2, PAL_MODE_INPUT_ANALOG);
	palSetPadMode(GPIOC, 3, PAL_MODE_INPUT_ANALOG);
	palSetPadMode(GPIOC, 4, PAL_MODE_INPUT_ANALOG);
	palSetPadMode(GPIOC, 5, PAL_MODE_INPUT_ANALOG);
	ENABLE_GATE();

	drv8323s_init();
}

void hw_setup_adc_channels(void) {

	// ADC1 regular channels
	ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 1, ADC_SampleTime_15Cycles); //0
	ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 2, ADC_SampleTime_15Cycles); //3
	ADC_RegularChannelConfig(ADC1, ADC_Channel_5 , 3, ADC_SampleTime_15Cycles); //6
	ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 4, ADC_SampleTime_15Cycles); //9
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 5, ADC_SampleTime_15Cycles); //12

	// ADC2 regular channels
	ADC_RegularChannelConfig(ADC2, ADC_Channel_8, 1, ADC_SampleTime_15Cycles); //1
	ADC_RegularChannelConfig(ADC2, ADC_Channel_15, 2, ADC_SampleTime_15Cycles); //4
	ADC_RegularChannelConfig(ADC2, ADC_Channel_6, 3, ADC_SampleTime_15Cycles); //7
	ADC_RegularChannelConfig(ADC2, ADC_Channel_12, 4, ADC_SampleTime_15Cycles); //10
	ADC_RegularChannelConfig(ADC2, ADC_Channel_1, 5, ADC_SampleTime_15Cycles); //13

	// ADC3 regular channels
	ADC_RegularChannelConfig(ADC3, ADC_Channel_10, 1, ADC_SampleTime_15Cycles); //2
	ADC_RegularChannelConfig(ADC3, ADC_Channel_3, 2, ADC_SampleTime_15Cycles); //5
	ADC_RegularChannelConfig(ADC3, ADC_Channel_13, 3, ADC_SampleTime_15Cycles); //8
	ADC_RegularChannelConfig(ADC3, ADC_Channel_11, 4, ADC_SampleTime_15Cycles); //11
	ADC_RegularChannelConfig(ADC3, ADC_Channel_2, 5, ADC_SampleTime_15Cycles); //14

	// Injected channels
	ADC_InjectedChannelConfig(ADC1, ADC_Channel_9, 1, ADC_SampleTime_15Cycles);
	ADC_InjectedChannelConfig(ADC1, ADC_Channel_8, 2, ADC_SampleTime_15Cycles);


	ADC_InjectedChannelConfig(ADC2, ADC_Channel_5, 1, ADC_SampleTime_15Cycles);
	ADC_InjectedChannelConfig(ADC2, ADC_Channel_4, 2, ADC_SampleTime_15Cycles);

	//  ADC_InjectedChannelConfig(ADC3, ADC_Channel_2, 1, ADC_SampleTime_15Cycles);
	// ADC_InjectedChannelConfig(ADC3, ADC_Channel_0, 2, ADC_SampleTime_15Cycles);
	//ADC_InjectedChannelConfig(ADC3, ADC_Channel_1, 3, ADC_SampleTime_15Cycles);

	chThdCreateStatic(mux_thread_wa, sizeof(mux_thread_wa), NORMALPRIO, mux_thread, NULL);
	chThdCreateStatic(switch_color_thread_wa, sizeof(switch_color_thread_wa), LOWPRIO, switch_color_thread, NULL);

}

void hw_start_i2c(void) {
	i2cAcquireBus(&HW_I2C_DEV);

	if (!i2c_running) {
		palSetPadMode(HW_I2C_SCL_PORT, HW_I2C_SCL_PIN,
					  PAL_MODE_ALTERNATE(HW_I2C_GPIO_AF) |
					  PAL_STM32_OTYPE_OPENDRAIN |
					  PAL_STM32_OSPEED_MID1 |
					  PAL_STM32_PUDR_PULLUP);
		palSetPadMode(HW_I2C_SDA_PORT, HW_I2C_SDA_PIN,
					  PAL_MODE_ALTERNATE(HW_I2C_GPIO_AF) |
					  PAL_STM32_OTYPE_OPENDRAIN |
					  PAL_STM32_OSPEED_MID1 |
					  PAL_STM32_PUDR_PULLUP);

		i2cStart(&HW_I2C_DEV, &i2cfg);
		i2c_running = true;
	}

	i2cReleaseBus(&HW_I2C_DEV);
}

void hw_stop_i2c(void) {
	i2cAcquireBus(&HW_I2C_DEV);

	if (i2c_running) {
		palSetPadMode(HW_I2C_SCL_PORT, HW_I2C_SCL_PIN, PAL_MODE_INPUT);
		palSetPadMode(HW_I2C_SDA_PORT, HW_I2C_SDA_PIN, PAL_MODE_INPUT);

		i2cStop(&HW_I2C_DEV);
		i2c_running = false;

	}

	i2cReleaseBus(&HW_I2C_DEV);
}

/**
 * Try to restore the i2c bus
 */
void hw_try_restore_i2c(void) {
	if (i2c_running) {
		i2cAcquireBus(&HW_I2C_DEV);

		palSetPadMode(HW_I2C_SCL_PORT, HW_I2C_SCL_PIN,
					  PAL_STM32_OTYPE_OPENDRAIN |
					  PAL_STM32_OSPEED_MID1 |
					  PAL_STM32_PUDR_PULLUP);

		palSetPadMode(HW_I2C_SDA_PORT, HW_I2C_SDA_PIN,
					  PAL_STM32_OTYPE_OPENDRAIN |
					  PAL_STM32_OSPEED_MID1 |
					  PAL_STM32_PUDR_PULLUP);

		palSetPad(HW_I2C_SCL_PORT, HW_I2C_SCL_PIN);
		palSetPad(HW_I2C_SDA_PORT, HW_I2C_SDA_PIN);

		chThdSleep(1);

		for(int i = 0;i < 16;i++) {
			palClearPad(HW_I2C_SCL_PORT, HW_I2C_SCL_PIN);
			chThdSleep(1);
			palSetPad(HW_I2C_SCL_PORT, HW_I2C_SCL_PIN);
			chThdSleep(1);
		}

		// Generate start then stop condition
		palClearPad(HW_I2C_SDA_PORT, HW_I2C_SDA_PIN);
		chThdSleep(1);
		palClearPad(HW_I2C_SCL_PORT, HW_I2C_SCL_PIN);
		chThdSleep(1);
		palSetPad(HW_I2C_SCL_PORT, HW_I2C_SCL_PIN);
		chThdSleep(1);
		palSetPad(HW_I2C_SDA_PORT, HW_I2C_SDA_PIN);

		palSetPadMode(HW_I2C_SCL_PORT, HW_I2C_SCL_PIN,
					  PAL_MODE_ALTERNATE(HW_I2C_GPIO_AF) |
					  PAL_STM32_OTYPE_OPENDRAIN |
					  PAL_STM32_OSPEED_MID1 |
					  PAL_STM32_PUDR_PULLUP);

		palSetPadMode(HW_I2C_SDA_PORT, HW_I2C_SDA_PIN,
					  PAL_MODE_ALTERNATE(HW_I2C_GPIO_AF) |
					  PAL_STM32_OTYPE_OPENDRAIN |
					  PAL_STM32_OSPEED_MID1 |
					  PAL_STM32_PUDR_PULLUP);

		HW_I2C_DEV.state = I2C_STOP;
		i2cStart(&HW_I2C_DEV, &i2cfg);

		i2cReleaseBus(&HW_I2C_DEV);
	}
}


static THD_FUNCTION(mux_thread, arg) {
	chRegSetThreadName("adc_mux");
	(void)arg;

	for (;;) {
		ENABLE_MOS_TEMP1();
		chThdSleepMilliseconds(1);
		ADC_Value[ADC_IND_TEMP_MOS] = ADC_Value[ADC_IND_ADC_MUX];

		ENABLE_MOS_TEMP2();
		chThdSleepMilliseconds(1);
		ADC_Value[ADC_IND_TEMP_MOS_M2] = ADC_Value[ADC_IND_ADC_MUX];

		ENABLE_MOT_TEMP1();
		chThdSleepMilliseconds(1);
		ADC_Value[ADC_IND_TEMP_MOTOR] = ADC_Value[ADC_IND_ADC_MUX];

		ENABLE_MOT_TEMP2();
		chThdSleepMilliseconds(1);
		ADC_Value[ADC_IND_TEMP_MOTOR_2] = ADC_Value[ADC_IND_ADC_MUX];

		ENABLE_ADC_EXT_1();
		chThdSleepMilliseconds(1);
		ADC_Value[ADC_IND_EXT] = ADC_Value[ADC_IND_ADC_MUX];

		ENABLE_ADC_EXT_2();
		chThdSleepMilliseconds(1);
		ADC_Value[ADC_IND_EXT2] = ADC_Value[ADC_IND_ADC_MUX];

		ENABLE_ADC_EXT_3();
		chThdSleepMilliseconds(1);
		ADC_Value[ADC_IND_EXT3] = ADC_Value[ADC_IND_ADC_MUX];

		ENABLE_V_BATT_DIV();
		chThdSleepMilliseconds(1);
		ADC_Value[ADC_IND_V_BATT] = ADC_Value[ADC_IND_ADC_MUX];
	}
}

void smart_switch_keep_on(void) {
	palSetPad(SWITCH_OUT_GPIO, SWITCH_OUT_PIN);
	//#ifdef HW_HAS_RGB_SWITCH
	//	LED_SWITCH_B_ON();
	//	ledpwm_set_intensity(SWITCH_LED_B, 1.0);
	//#else
	//	ledpwm_set_intensity(SWITCH_LED, 1.0);
	//	ledpwm_set_switch_intensity(0.6);
	//#endif
}

void smart_switch_shut_down(void) {
	switch_state = SWITCH_SHUTTING_DOWN;
	palClearPad(SWITCH_OUT_GPIO, SWITCH_OUT_PIN);
#ifdef HW_HAS_STORMCORE_SWITCH
	palClearPad(SWITCH_PRECHARGED_GPIO, SWITCH_PRECHARGED_PIN);
#endif
	return;
}

bool smart_switch_is_pressed(void) {
	if(palReadPad(SWITCH_IN_GPIO, SWITCH_IN_PIN) == 1)
		return true;
	else
		return false;
}

static THD_FUNCTION(switch_color_thread, arg) {
	(void)arg;
	chRegSetThreadName("switch_color_thread");
	float switch_red = 0.0;
	float switch_green = 0.0;
	float switch_blue = 0.0;

	for(int i = 0; i < 400; i++) {
		float angle = i*3.14/400.0;
		float s,c;
		utils_fast_sincos_better(angle, &s, &c);
		switch_blue = 0.75* c*c;
		ledpwm_set_intensity(LED_HW1,switch_bright*switch_blue);
		utils_fast_sincos_better(angle + 3.14/3.0, &s, &c);
		switch_green = 0.75* c*c;
		ledpwm_set_intensity(LED_HW2,switch_bright*switch_green);
		utils_fast_sincos_better(angle + 6.28/3.0, &s, &c);
		switch_red = 0.75* c*c;
		ledpwm_set_intensity(LED_HW3,switch_bright*switch_red);
		chThdSleepMilliseconds(10);
	}
	float switch_red_old = switch_red_old;
	float switch_green_old = switch_green;
	float switch_blue_old = switch_blue;
	float wh_left;
	float left = mc_interface_get_battery_level(&wh_left);
	if(left < 0.5){
		float intense = utils_map(left,0.0, 0.5, 0.0, 1.0);
		utils_truncate_number(&intense,0,1);
		switch_blue = intense;
		switch_red  = 1.0-intense;
	}else{
		float intense = utils_map(left , 0.5, 1.0, 0.0, 1.0);
		utils_truncate_number(&intense,0,1);
		switch_green = intense;
		switch_blue  = 1.0-intense;
	}
	for(int i = 0; i < 100; i++) {
		float red_now = utils_map((float) i,0.0, 100.0, switch_red_old, switch_red);
		float blue_now = utils_map((float) i,0.0, 100.0, switch_blue_old, switch_blue);
		float green_now = utils_map((float) i,0.0, 100.0, switch_green_old, switch_green);
		ledpwm_set_intensity(LED_HW1, switch_bright*blue_now);
		ledpwm_set_intensity(LED_HW2, switch_bright*green_now);
		ledpwm_set_intensity(LED_HW3, switch_bright*red_now);
		chThdSleepMilliseconds(10);
	}

	for (;;) {
		mc_fault_code fault = mc_interface_get_fault();
		mc_interface_select_motor_thread(2);
		mc_fault_code fault2 = mc_interface_get_fault();
		mc_interface_select_motor_thread(1);
		if (fault != FAULT_CODE_NONE || fault2 != FAULT_CODE_NONE) {
			ledpwm_set_intensity(LED_HW2, 0);
			ledpwm_set_intensity(LED_HW1, 0);
			for (int i = 0;i < (int)fault;i++) {
				ledpwm_set_intensity(LED_HW3, 1.0);
				chThdSleepMilliseconds(250);
				ledpwm_set_intensity(LED_HW3, 0.0);
				chThdSleepMilliseconds(250);
			}

			chThdSleepMilliseconds(500);

			for (int i = 0;i < (int)fault2;i++) {
				ledpwm_set_intensity(LED_HW3, 1.0);
				chThdSleepMilliseconds(250);
				ledpwm_set_intensity(LED_HW3, 0.0);
				chThdSleepMilliseconds(250);
			}

			chThdSleepMilliseconds(500);
		} else {
			left = mc_interface_get_battery_level(&wh_left);
			if(left < 0.5){
				float intense = utils_map(left,0.0, 0.5, 0.0, 1.0);
				utils_truncate_number(&intense,0,1);
				switch_blue = intense;
				switch_red  = 1.0-intense;
				switch_green = 0;
			}else{
				float intense = utils_map(left , 0.5, 1.0, 0.0, 1.0);
				utils_truncate_number(&intense,0,1);
				switch_green = intense;
				switch_blue  = 1.0-intense;
				switch_red = 0;
			}
			ledpwm_set_intensity(LED_HW1, switch_bright*switch_blue);
			ledpwm_set_intensity(LED_HW2, switch_bright*switch_green);
			ledpwm_set_intensity(LED_HW3, switch_bright*switch_red);
		}

		chThdSleepMilliseconds(20);
	}
}

static THD_FUNCTION(smart_switch_thread, arg) {
	(void)arg;
	chRegSetThreadName("smart_switch");
	unsigned int millis_switch_pressed = 0;

	for (;;) {
		switch (switch_state) {
		case SWITCH_BOOTED:
			switch_state = SWITCH_TURN_ON_DELAY_ACTIVE;
			break;
		case SWITCH_TURN_ON_DELAY_ACTIVE:
			switch_state = SWITCH_HELD_AFTER_TURN_ON;
			chThdSleepMilliseconds(5000);
			palSetPad(SWITCH_PRECHARGED_GPIO, SWITCH_PRECHARGED_PIN);
			break;
		case SWITCH_HELD_AFTER_TURN_ON:
			smart_switch_keep_on();
			if(smart_switch_is_pressed()){
				switch_state = SWITCH_HELD_AFTER_TURN_ON;
			} else {
				switch_state = SWITCH_TURNED_ON;
			}
			break;
		case SWITCH_TURNED_ON:
			if (smart_switch_is_pressed()) {
				millis_switch_pressed++;
				switch_bright = 1.0;
			} else {
				millis_switch_pressed = 0;
				switch_bright = 0.5;
			}

			if (millis_switch_pressed > SMART_SWITCH_MSECS_PRESSED_OFF) {
				switch_state = SWITCH_SHUTTING_DOWN;
			}
			break;
		case SWITCH_SHUTTING_DOWN:
			switch_bright = 0;
			comm_can_shutdown(255);
			smart_switch_shut_down();
			chThdSleepMilliseconds(10000);
			smart_switch_keep_on();
			switch_state = SWITCH_TURN_ON_DELAY_ACTIVE;
			break;
		default:
			break;
		}
		chThdSleepMilliseconds(1);
	}
}

void smart_switch_thread_start(void) {
	chThdCreateStatic(smart_switch_thread_wa, sizeof(smart_switch_thread_wa),
					  NORMALPRIO, smart_switch_thread, NULL);
}

void smart_switch_pin_init(void) {
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

	palSetPadMode(SWITCH_IN_GPIO, SWITCH_IN_PIN, PAL_MODE_INPUT_PULLDOWN);
	palSetPadMode(SWITCH_OUT_GPIO,SWITCH_OUT_PIN, PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);
	palSetPadMode(SWITCH_LED_1_GPIO,SWITCH_LED_1_PIN, PAL_MODE_OUTPUT_OPENDRAIN | PAL_STM32_OSPEED_HIGHEST);
	palSetPadMode(SWITCH_LED_2_GPIO,SWITCH_LED_2_PIN, PAL_MODE_OUTPUT_OPENDRAIN | PAL_STM32_OSPEED_HIGHEST);
	palSetPadMode(SWITCH_LED_3_GPIO,SWITCH_LED_3_PIN, PAL_MODE_OUTPUT_OPENDRAIN | PAL_STM32_OSPEED_HIGHEST);
	palSetPadMode(SWITCH_PRECHARGED_GPIO, SWITCH_PRECHARGED_PIN, PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);
	palClearPad(SWITCH_PRECHARGED_GPIO, SWITCH_PRECHARGED_PIN);
	palSetPad(SWITCH_OUT_GPIO, SWITCH_OUT_PIN);
	LED_SWITCH_B_ON();
	LED_SWITCH_R_OFF();
	LED_SWITCH_G_OFF();
	return;
}
