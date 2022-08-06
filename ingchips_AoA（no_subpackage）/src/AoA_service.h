#ifndef _AoA_SERVICE_H_
#define _AoA_SERVICE_H_

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "platform_api.h"
#include "profile.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
/*
* If your array-boards' addresses are not in default, you can set AoA_MANAUL_SETUP_ARRAY = 1.
* Then you can config 8266 and connect the array-board with special address manaully.
* Otherwise, use default array-boards and set AoA_MANAUL_SETUP_ARRAY = 0 is highly recommended. 
*/
#define AoA_MANAUL_SETUP_ARRAY 0

/*
* This MQTT is base on mosquitto of a PC. There are three factors: wifi, MQTT and publishing topic.
* Keep in mind that you need to modify these information manaully when one of them change.
* Of course except MQTT_CLIENT_ID for it is a random value.
* Modifying MQTT_TOPIC_NAME is prohibited!!!
*/
#define MQTT_WIFI_NAME     "wifiName"
#define MQTT_WIFI_PASSWORD "wifiPassword"
#define MQTT_CLIENT_ID     "123"
#define MQTT_PASSWORD      "123456"
#define MQTT_USER_NAME     "userName"
#define MQTT_HOST_NAME     "192.168.3.1"
#define MQTT_PORT_NUMBER    1883
#define MQTT_TOPIC_NAME    "ingchips/iq_report"

#define AoA_DEVICE_NUM_MAX   	1
#define AoA_DEVICE_ADDR_LEN 	17
#define AoA_SETUP_HEARBEAT_MS   1000
#define AoA_IQ_STR_BUFFER_LEN 	300
#define AoA_IQ_REPORT_TIME_MS 	90  // The value between *80-120* is recommended.

extern TimerHandle_t AoA_iqReportProcessTimer;
extern int8_t  g_devNum;
extern int16_t g_iqReportStrLen;
extern int8_t  g_iq_str_buffer[AoA_IQ_STR_BUFFER_LEN];
typedef struct deviceInfo {
	int8_t deviceAddr[AoA_DEVICE_ADDR_LEN];					
	int8_t iq_str_buffer[AoA_IQ_STR_BUFFER_LEN];
} __attribute__ ((packed)) deviceInfo_t;
extern deviceInfo_t g_deviceList[AoA_DEVICE_NUM_MAX];
extern uint8_t slave_addr[];
typedef uint8_t addrStr[AoA_DEVICE_ADDR_LEN + 1];

#if AoA_MANAUL_SETUP_ARRAY
extern int8_t g_checkNewFlg;
void AoA_SetAddrStrMapByUart(char *addr);
extern addrStr AoA_addrStrMap[3];
#else
typedef uint8_t *sequence_t;
typedef uint8_t deviceAddr_t[6];
extern int8_t g_deviceConnected;
extern int8_t g_deviceDisonnected;
extern uint16_t AoA_arrayChecker[2][AoA_DEVICE_NUM_MAX];
extern addrStr AoA_addrStrMap[AoA_DEVICE_NUM_MAX];
#endif

void AoA_ServiceInit(void);
#endif
