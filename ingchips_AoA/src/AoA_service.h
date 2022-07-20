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
#define MQTT_WIFI_NAME     "INGCHIPS"
#define MQTT_WIFI_PASSWORD "ING-123!"
#define MQTT_CLIENT_ID     "123"
#define MQTT_PASSWORD      "123456"
#define MQTT_USER_NAME     "liyuhang"
#define MQTT_HOST_NAME     "192.168.3.5"
#define MQTT_PORT_NUMBER    1883
#define MQTT_TOPIC_NAME    "ingchips/iq_report"

#define AoA_DEVICE_NUM_MAX        3
#define AoA_DEVICE_ADDR_LEN       17
#define AoA_SETUP_HEARBEAT_MS     1000
#define AoA_IQ_STR_BUFFER_LEN     300
#define AoA_IQ_STR_BUFFER_LEN_TMP 200
#define AoA_IQ_REPORT_TIME_MS     90  // The value between *80-120* is recommended.
#define AoA_IQ_STR_LEN_FIRST      ((100) + (AoA_DEVICE_ADDR_LEN))
#define AoA_IQ_STR_LEN_SECOND     155
#define AoA_IQ_STR_LEN_CHECK      10
#define AoA_IQ_STR_LEN_FIRST_CP   ((AoA_IQ_STR_LEN_FIRST) - (AoA_IQ_STR_LEN_CHECK))

typedef uint8_t *sequence_t;
typedef uint8_t deviceAddr_t[6];
typedef struct deviceInfo {
	int8_t deviceAddr[AoA_DEVICE_ADDR_LEN];					
	int8_t iq_str_buffer[AoA_IQ_STR_BUFFER_LEN];
	int8_t iq_str_buffer_tmp[AoA_IQ_STR_BUFFER_LEN_TMP];
} __attribute__ ((packed)) deviceInfo_t;
extern deviceInfo_t g_deviceList[AoA_DEVICE_NUM_MAX];

extern TimerHandle_t AoA_iqReportProcessTimer;
extern int8_t  g_devNum;
extern uint8_t slave_addr[];
#if AoA_MANAUL_SETUP_ARRAY
extern int8_t g_checkNewFlg;
#else
extern int8_t g_deviceConnected;
extern int8_t g_deviceDisonnected;
extern uint16_t AoA_arrayChecker[2][AoA_DEVICE_NUM_MAX];
extern int8_t g_deviceSubFlg;
extern int8_t g_deviceSub;
#endif

void Delay(int time);
void AoA_ServiceInit(void);
void AoA_MQTT_Setup_8266(void);

#endif
