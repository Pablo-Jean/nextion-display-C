/**
 * @file nextion.c
 * @author Pablo Jean Rozario (pablo.jean.eel@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-04-08
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "nextion.h"

/*
 * Internal Macros
 */

#define _TIMEOUT_PERIOD		10

/*
 * Structs and Unions
 */

typedef union{
	struct __attribute__((__packed__)){
		uint8_t response;
		uint32_t number;
	};
	uint8_t _raw[5];
}_nextion_num_resp_t;

/**
 * Privates
 */

uint8_t _Tail[3] = {0xFF, 0xFF, 0xFF};

void __rx_interrupt(nextion_t *obj){
	obj->buffer.i++;
	if (obj->buffer.i > 3){
		if (memcmp(&obj->buffer.d[obj->buffer.i-3], _Tail, 3) == 0){
			obj->stat.rxDone = 1;
			obj->stat.rxLen = obj->buffer.i;
			obj->buffer.i = 0;
			if (obj->buffer.d[0] == NEXTION_EVENT_TOUCH_PRESS){
				obj->stat.pendingTouchPress = 1;
				memcpy(obj->event._raw, obj->buffer.d, sizeof(nextion_event_resp_t));
			}
		}
	}
	obj->fnx.UartRx(&obj->buffer.d[obj->buffer.i], 1);
}

void __nextion_write(nextion_t *obj, uint8_t *pData, uint8_t len){
	if (obj != NULL){
		obj->fnx.UartTx(pData, len);
		obj->fnx.UartTx(_Tail, sizeof(_Tail));
	}
}

uint8_t __nextion_read(nextion_t *obj, uint8_t *pData, uint8_t *len){
	if (obj != NULL){
		if (obj->interruptMode){
			obj->buffer.i = 0;
			obj->fnx.UartRx(obj->buffer.d, 1);
			obj->stat.timeout = 500;
			obj->stat.rxDone = 0;
			obj->stat.rxLen = 0;
			while (obj->stat.rxDone == 0 && obj->stat.timeout > 0){
				obj->fnx.OsDelay(_TIMEOUT_PERIOD);
				obj->stat.timeout -= _TIMEOUT_PERIOD;
			}
			if (obj->stat.timeout > 0){
				if (obj->stat.rxLen <= *len){
					memcpy(pData, obj->buffer.d, obj->stat.rxLen);
					*len = obj->stat.rxLen;
				}
				else{
					memcpy(pData, obj->buffer.d, *len);
				}
				return 1;
			}
			else{
				return 0;
			}
		}
		else{
			if (obj->fnx.UartRx(pData, *len)){
				return 1;
			}
			else{
				return 0;
			}
		}
	}

	return 0;
}


/**
 * Publics
 */

nextion_err_e nextion_init(nextion_t *obj){
	if (obj != NULL){
		obj->list = list_new();
		if (obj->list == NULL || obj->fnx.UartRx == NULL || obj->fnx.UartTx == NULL){
			return NEXTION_ERR_FAIL;
		}
		obj->buffer.i = 0;
		if (obj->interruptMode){
			obj->fnx.UartRx(obj->buffer.d, 1);
		}
		return NEXTION_OK;
	}

	return NEXTION_ERR_FAIL;
}

void nextion_element_monitor_add(nextion_t *obj, nextion_element_e type, void* element){
	list_node_t *node;

	if (obj != NULL){
		switch (type){
		case 	NEXTION_ELEMENT_TEXT:
		case	NEXTION_ELEMENT_BUTTON:
		case 	NEXTION_ELEMENT_PROGRESS_BAR:
		case	NEXTION_ELEMENT_SLIDER:
		case	NEXTION_ELEMENT_PAGE:
		case	NEXTION_ELEMENT_PICTURE:
			memcpy(element, &type, sizeof(nextion_element_e));
			node = list_node_new(element);
			list_lpush(obj->list, node);
			break;
		default:

			break;
		}
	}
}

void nextion_element_set_val(nextion_t *obj, void* element, uint16_t value){
	nextion_element_e el;
	nextion_progress_bar_t *pg_bar;
	nextion_button_t *button;
	nextion_slider_t *slider;
	uint8_t pck[50];

	if (obj != NULL && element != NULL){
		memcpy(&el, element, sizeof(nextion_element_e));
		switch (el){
		case	NEXTION_ELEMENT_BUTTON:
			button = (nextion_button_t*)element;
			button->val = value;
			sprintf((char*)pck, "%s.txt=%u", button->objName, (uint16_t)button->val);
			__nextion_write(obj, pck, strlen((char*)pck));
			break;
		case	NEXTION_ELEMENT_SLIDER:
			slider = (nextion_slider_t*)element;
			slider->val = value;
			sprintf((char*)pck, "%s.txt=%u", slider->objName, (uint16_t)slider->val);
			__nextion_write(obj, pck, strlen((char*)pck));
			break;
		case 	NEXTION_ELEMENT_PROGRESS_BAR:
			pg_bar = (nextion_progress_bar_t*)element;
			sprintf((char*)pck, "%s.val=%d", pg_bar->objName, value);
			__nextion_write(obj, pck, strlen((char*)pck));
			break;
		default:

			break;
		}
	}
}

void nextion_element_set_txt(nextion_t *obj, void* element, char* txt){
	nextion_element_e el;
	nextion_text_t *text;
	uint8_t pck[25];

	if (obj != NULL && element != NULL){
		memcpy(&el, element, sizeof(nextion_element_e));
		switch (el){
		case 	NEXTION_ELEMENT_TEXT:
			text = (nextion_text_t*)element;
			strcpy(text->txt, txt);
			sprintf((char*)pck, "%s.txt=%s", text->objName, text->txt);
			__nextion_write(obj, pck, strlen((char*)pck));
			break;
		default:
			break;
		}
	}
}

void nextion_element_set_page(nextion_t *obj, uint8_t page){
	uint8_t pck[10];

	if (obj != NULL){
		sprintf((char*)pck, "page %d", page);
	}
}

void nextion_element_get_val(nextion_t *obj, void* element, uint16_t *value){
	nextion_element_e el;
	nextion_progress_bar_t *pg_bar;
	nextion_button_t *button;
	nextion_slider_t *slider;
	uint8_t pck[50];
	_nextion_num_resp_t resp;
	uint8_t len = 5;

	if (obj != NULL && element != NULL){
		memcpy(&el, element, sizeof(nextion_element_e));
		switch (el){
		case	NEXTION_ELEMENT_BUTTON:
			button = (nextion_button_t*)element;
			sprintf((char*)pck, "get %s.val", button->objName);
			__nextion_write(obj, pck, strlen((char*)pck));
			__nextion_read(obj, resp._raw, &len);
			*value = resp.number;
			break;
		case	NEXTION_ELEMENT_SLIDER:
			slider = (nextion_slider_t*)element;
			sprintf((char*)pck, "get %s.val", slider->objName);
			__nextion_write(obj, pck, strlen((char*)pck));
			__nextion_read(obj, resp._raw, &len);
			*value = resp.number;
			break;
		case 	NEXTION_ELEMENT_PROGRESS_BAR:
			pg_bar = (nextion_progress_bar_t*)element;
			sprintf((char*)pck, "get %s.val", pg_bar->objName);
			__nextion_write(obj, pck, strlen((char*)pck));
			__nextion_read(obj, resp._raw, &len);
			*value = resp.number;
			break;
		default:

			break;
		}
	}
}

void nextion_element_get_txt(nextion_t *obj, void* element, char *txt){
	//TODO
}


// interrupt mode
nextion_event_stat_e nextion_check_events(nextion_t *obj, nextion_event_t *event){
	list_node_t *node;
	uint8_t i;
	nextion_element_e el;
	nextion_button_t *Data;
	uint16_t idx;
	void *pElem;

	if (obj != NULL && event != NULL){
		if (obj->stat.pendingTouchPress){
			obj->stat.pendingTouchPress = 0;
			i = 0;
			do{
				node = list_at(obj->list, i);
				if (node != NULL){
					pElem = node->val;
					Data = (nextion_button_t*)pElem;
					el = Data->el;
					idx = Data->id;
					if (obj->event.index == idx){
						event->Element = el;
						event->index = obj->event.index;
						event->pElement = pElem;
						event->page = obj->event.page;
						event->type = obj->event.type;
						return NEXTION_EVENT_DETECTED;
					}
				}
				i++;
			}while (node != NULL);
		}
	}

	return NEXTION_EVENT_NONE;
}

void nextion_rx_interrupt(nextion_t *obj){
	if (obj != NULL){
		__rx_interrupt(obj);
	}
}
