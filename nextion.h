/**
 * @file nextion.h
 * @author Pablo Jean Rozario (pablo.jean.eel@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-04-08
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _API_NEXTION_H
#define _API_NEXTION_H

#include <stdint.h>
#include <stddef.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>

#include "list/list.h"

#include "nextion_defs.h"


/**
 * Macros
 */

/**
 * Enumerates
 */

typedef enum{
    NEXTION_OK,
	NEXTION_ERR_FAIL,
    NEXTION_ERR_INV_PAGE        = 0X3,
    NEXTION_ERR_INV_VARIABLE    = 0x1A,
    NEXTION_ASSIGN_FAIL         = 0x1C
}nextion_err_e;

typedef enum{
	NEXTION_ELEMENT_TEXT,
	NEXTION_ELEMENT_BUTTON,
	NEXTION_ELEMENT_PROGRESS_BAR,
	NEXTION_ELEMENT_SLIDER,
	NEXTION_ELEMENT_PAGE,
	NEXTION_ELEMENT_PICTURE
}nextion_element_e;

typedef enum{
	NEXTION_EVENT_NONE,
	NEXTION_EVENT_DETECTED
}nextion_event_stat_e;


/*
 * Function typedefs
 */

typedef void (*UartTxFxn)(uint8_t *data, uint16_t len);

typedef uint8_t (*UartRxFnx)(uint8_t *data, uint16_t len);

typedef void (*osDelay)(uint32_t delay);


/*
 * Structs and Unions
 */

typedef struct{
	nextion_element_e el;
	uint8_t number;
	char objName[6];
}nextion_page_t;

typedef struct{
	nextion_element_e el;
	uint16_t id;
	char objName[6];
	uint32_t val;
}nextion_button_t;

typedef struct{
	nextion_element_e el;
	uint16_t id;
	char objName[6];
	uint32_t val;
	uint32_t minVal;
	uint32_t maxVal;
}nextion_slider_t;

typedef struct{
	nextion_element_e el;
	uint16_t id;
	char objName[6];
	uint8_t value;
}nextion_progress_bar_t;

typedef struct{
	nextion_element_e el;
	uint16_t id;
	char objName[6];
	char *txt;
}nextion_text_t;

typedef struct{
	nextion_element_e Element;
	void *pElement;
	uint8_t page;
	uint8_t index;
	uint8_t type;
}nextion_event_t;

typedef union{
	struct __attribute__((__packed__)){
		uint8_t response;
		uint8_t page;
		uint8_t index;
		uint8_t type;
	};
	uint8_t _raw[4];
}nextion_event_resp_t;

typedef struct{
	list_t *list;
	uint8_t interruptMode;
	nextion_event_resp_t event;
	struct{
		uint8_t i;
		uint8_t d[150];
	}buffer;
	struct{
		UartTxFxn UartTx;
		UartRxFnx UartRx;
		osDelay	OsDelay;
	}fnx;
	struct{
		uint8_t rxDone;
		uint8_t rxLen;
		uint8_t pendingTouchPress;
		uint32_t timeout;
	}stat;
}nextion_t;

/*
 * Function Prptotypes
 */

nextion_err_e nextion_init(nextion_t *obj);

void nextion_element_monitor_int(nextion_t *obj, nextion_element_e type, void* element);

void nextion_element_set_val(nextion_t *obj, void* element, uint16_t value);

void nextion_element_set_txt(nextion_t *obj, void* element, char* txt);

void nextion_element_set_page(nextion_t *obj, uint8_t page);

void nextion_element_get_val(nextion_t *obj, void* element, uint16_t *value);

void nextion_element_get_txt(nextion_t *obj, void* element, char *txt);


// interrupt mode
nextion_event_stat_e nextion_check_events(nextion_t *obj, nextion_event_t *event);

void nextion_rx_interrupt(nextion_t *obj);

#endif
