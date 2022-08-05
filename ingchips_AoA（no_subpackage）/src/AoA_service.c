#include "AoA_service.h"

void sub_to_char(int handle);
TimerHandle_t AoA_iqReportProcessTimer = 0;
int8_t  g_devNum = 0;
int16_t g_iqReportStrLen;
int8_t g_iq_str_buffer[AoA_IQ_STR_BUFFER_LEN];
deviceInfo_t g_deviceList[AoA_DEVICE_NUM_MAX];
#if AoA_MANAUL_SETUP_ARRAY
int8_t g_checkNewFlg = 0;
addrStr AoA_addrStrMap[3];
#else
TimerHandle_t AoA_setupArrayTimer = 0;
TimerHandle_t AoA_arrayCheckerTimer = 0;
int8_t g_deviceConnected = 0;
int8_t g_deviceDisonnected = 0;
int8_t g_deviceSub = 0;
int8_t g_deviceSubFlg = 0;
int8_t g_deviceConnectOverFlg = 0;
uint16_t AoA_arrayChecker[2][AoA_DEVICE_NUM_MAX];
addrStr AoA_addrStrMap[AoA_DEVICE_NUM_MAX];
#endif

void Delay(int time)
{
    for (int t = 5854 * time; t > 0; t--) { 
    	__NOP();__NOP();__NOP(); 
    }
}

void AoA_MQTT_Setup_8266(void)
{
    printf("AT\r\n");
    Delay(100);
    printf("AT+RESTORE\r\n");
    Delay(3000);
    printf("AT+CWMODE=1\r\n");
    Delay(100);
    printf("AT+CWJAP=\"%s\",\"%s\"\r\n", MQTT_WIFI_NAME, MQTT_WIFI_PASSWORD);
    Delay(5000);
    printf("AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\"\r\n", MQTT_CLIENT_ID, MQTT_USER_NAME, MQTT_PASSWORD);
    Delay(1000);
    printf("AT+MQTTCONN=0,\"%s\",%d,0\r\n", MQTT_HOST_NAME, MQTT_PORT_NUMBER);
    Delay(1000);
}

#if AoA_MANAUL_SETUP_ARRAY
static void AoA_CheckNewDevice(char *str_buffer)
{
    if (g_devNum == AoA_DEVICE_NUM_MAX ||
        !memcmp(g_deviceList[0].deviceAddr, str_buffer, AoA_DEVICE_ADDR_LEN) ||
        !memcmp(g_deviceList[1].deviceAddr, str_buffer, AoA_DEVICE_ADDR_LEN) ||
        !memcmp(g_deviceList[2].deviceAddr, str_buffer, AoA_DEVICE_ADDR_LEN)) {
        printf("AoA_CheckNewDevice: bad new device address!\n");
        return;
    }

    memcpy(g_deviceList[g_devNum].deviceAddr, str_buffer, AoA_DEVICE_ADDR_LEN);
    g_devNum++;
    g_checkNewFlg = 0;
    printf("AoA_CheckNewDevice: new device registed! Now device number: [%d]\n", g_devNum);
}

void AoA_SetAddrStrMapByUart(char *addr)
{
    if (g_devNum == AoA_DEVICE_NUM_MAX) {
        printf("AoA_SetAddrStrMapByUart: no space!\n");
        return;
    }
    addrStr addrTmp;
    sprintf(addrTmp, "%X:%X:%X:%X:%X:%X\0", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

    for (int i = 0; i < g_devNum; i++) {
        if (!memcmp(AoA_addrStrMap[i], addrTmp, sizeof(addrTmp))) {
            return;
        }
    }

    memcpy(AoA_addrStrMap[g_devNum], addrTmp, sizeof(addrTmp));
    printf("AoA_SetAddrStrMapByUart: add new device in AoA_addrStrMap: %s\n", AoA_addrStrMap[g_devNum]);
}
#endif

static void AoA_IqReportProcessTimerCallback(TimerHandle_t xTimer)
{
#if AoA_MANAUL_SETUP_ARRAY
    if (g_checkNewFlg) {
        if (!memcmp(g_iq_str_buffer + AoA_DEVICE_ADDR_LEN, "EXT", strlen("EXT"))) {
            AoA_CheckNewDevice(g_iq_str_buffer);
        }
    }
#else
    if (g_deviceSubFlg) {
        if (!memcmp(g_deviceList[g_devNum].iq_str_buffer + AoA_DEVICE_ADDR_LEN, "EXT", strlen("EXT"))) {
            g_deviceSub++;
        }
    }
    if (!g_deviceConnectOverFlg) {
        return;
    }
#endif
#if AoA_MANAUL_SETUP_ARRAY
    for (int i = 0; i < g_devNum; i++) {
#else
    for (int i = 0; i < AoA_DEVICE_NUM_MAX; i++) {
#endif
        if (memcmp(g_deviceList[i].iq_str_buffer + AoA_DEVICE_ADDR_LEN, "EXT", strlen("EXT"))) {
            continue;
        }
	    printf("AT\r\n");
	    Delay(5);
        printf("AT+MQTTPUBRAW=0,\"%s\",%d,0,0\r\n", MQTT_TOPIC_NAME, g_iqReportStrLen);
        Delay(5);
        printf("%s\n", g_deviceList[i].iq_str_buffer);
        Delay(AoA_IQ_REPORT_TIME_MS - 13);
		memset(g_deviceList[i].iq_str_buffer, 0, AoA_IQ_STR_BUFFER_LEN);
    }
}

#if !AoA_MANAUL_SETUP_ARRAY
static sequence_t AoA_deviceSequence[] = {
    "[first]",
    "[second]",
    "[third]",
    "[noDevice]"
};
/*
* If your array-boards' addresses are not in default, you have to motify the information below.
* The Map's factors from up to down are the array-boards' number 1 to 3.
* Please note that the Map should be correct or the Aotomatic-Setup will work fail. 
*/
static deviceAddr_t AoA_deviceAddrMap[] = {
    {0xD1, 0x29, 0xF6, 0xBE, 0xF3, 0x26},
    {0xD2, 0x29, 0xF6, 0xBE, 0xF3, 0x26},
    {0xD3, 0x29, 0xF6, 0xBE, 0xF3, 0x26}
};
static void AoA_MakeAddrStrMap(void)
{
    uint8_t i;
    for (i = 0; i < AoA_DEVICE_NUM_MAX; i++) {
        sprintf(AoA_addrStrMap[i], "%X:%X:%X:%X:%X:%X\0", AoA_deviceAddrMap[i][0], AoA_deviceAddrMap[i][1], AoA_deviceAddrMap[i][2],
            AoA_deviceAddrMap[i][3], AoA_deviceAddrMap[i][4], AoA_deviceAddrMap[i][5]);
    }
}
void AoA_SetupArray(void);
static void AoA_ArrayCheckerInit(void)
{
    memset(AoA_arrayChecker[0], 0, AoA_DEVICE_NUM_MAX);
    memset(AoA_arrayChecker[1], 0, AoA_DEVICE_NUM_MAX);
}
static void AoA_ArrayCheckerTimerCallback(TimerHandle_t xTimer)
{
    static int cnt = 1;
    printf("AoA_arrayChecker: *%d* \n", cnt);
    for (int i = 0; i < AoA_DEVICE_NUM_MAX; i++) {
        printf("Device %10s: Now: [%4d] | Pre: [%4d]\n", AoA_deviceSequence[i], AoA_arrayChecker[1][i], AoA_arrayChecker[0][i]);
        if (AoA_arrayChecker[1][i] == AoA_arrayChecker[0][i]) {
            if (g_deviceDisonnected == 1) {
                printf("AoA_arrayChecker: Device %s no update! Reconnect it!\n", AoA_deviceSequence[i]);
                memset(g_deviceList[i].iq_str_buffer, 0, AoA_IQ_STR_BUFFER_LEN);
                memcpy(slave_addr, AoA_deviceAddrMap[i], sizeof(deviceAddr_t));
                printf("%X:%X:%X:%X:%X:%X\n", slave_addr[0], slave_addr[1], 
                    slave_addr[2], slave_addr[3], slave_addr[4], slave_addr[5]);
                g_deviceConnected = 0;
                g_deviceDisonnected = 0;
                g_devNum--;
                conn_to_slave();
                xTimerStart(AoA_setupArrayTimer, 0);
            } else if (g_devNum == AoA_DEVICE_NUM_MAX && (i == AoA_DEVICE_NUM_MAX - 1)) {
                printf("AoA_arrayChecker: Device %s no update! Sub it again!\n", AoA_deviceSequence[i]);
                sub_to_char(8);
            }
        }   
        AoA_arrayChecker[0][i] = AoA_arrayChecker[1][i]; 
    }
    cnt++;
}

static void AoA_SetupArrayTimerCallback(TimerHandle_t xTimer)
{
    if (g_devNum >= AoA_DEVICE_NUM_MAX) {
        return;
    }
    static int cnt = 1;
    // printf("AoA_SetupArrayTimerCallback: g_deviceDisonnected = %d, g_deviceConnected = %d, g_deviceSubFlg = %d, g_deviceSub = %d\n", 
    //    g_deviceDisonnected, g_deviceConnected, g_deviceSubFlg, g_deviceSub);
    printf("Connecting %s device...\n", AoA_deviceSequence[g_devNum]);
    if (g_deviceSub) {
        printf("Device %s: Sub OK, Go NEXT!!\n", AoA_deviceSequence[g_devNum]);
        g_deviceSubFlg = 0;
        g_deviceSub = 0;
        g_devNum++;
        (g_devNum > AoA_DEVICE_NUM_MAX) ? AoA_DEVICE_NUM_MAX : g_devNum;
        AoA_SetupArray();
    } else if (g_deviceSubFlg) {
        if (g_deviceDisonnected == 0 && (cnt > 0 && cnt % 5 == 0)) {
            printf("Device %s: Sub try again\n", AoA_deviceSequence[g_devNum]);
            sub_to_char(8);
            Delay(500);
            cnt++;
        } else {
            cnt++;
        }
    }
    if (g_deviceDisonnected) {
        printf("Device %s disonnected!! reconnect...\n", AoA_deviceSequence[g_devNum]);
        conn_to_slave();
        g_deviceDisonnected = 0;
    }
    if (g_deviceConnected) {
        printf("The %s device Connected!!! \n", AoA_deviceSequence[g_devNum]);
        g_deviceConnected= 0;
        Delay(4000);
        g_deviceSub = 0;
        cnt = 1;
        g_deviceSubFlg++;
        sub_to_char(8);
        Delay(500);
    }
}

static void AoA_SetupArray(void)
{
    int ret = 0;
    printf("AoA_SetupArray: g_devNum = %d\n", g_devNum);
	if (g_devNum >= AoA_DEVICE_NUM_MAX) {
		printf("AoA_SetupArray: setup device array over, success!!\n");
        ret = xTimerStop(AoA_setupArrayTimer, portMAX_DELAY);
        sub_to_char(8);
        g_deviceConnectOverFlg++;
        ret += xTimerStart(AoA_arrayCheckerTimer, portMAX_DELAY);
        if (ret) {
            printf("AoA_SetupArray: Timer FAIL! \n");
        }
		return;
	}
    memcpy(slave_addr, AoA_deviceAddrMap[g_devNum], sizeof(deviceAddr_t));
    printf("%X:%X:%X:%X:%X:%X\n", slave_addr[0], slave_addr[1], 
        slave_addr[2], slave_addr[3], slave_addr[4], slave_addr[5]);
    g_deviceConnected = 0;
    g_deviceDisonnected = 0;
    conn_to_slave();
    xTimerStart(AoA_setupArrayTimer, 0);
}
#endif

void AoA_ServiceInit(void)
{
	AoA_iqReportProcessTimer = xTimerCreate("AoA_iqReportProcessTimer",
										     pdMS_TO_TICKS(AoA_IQ_REPORT_TIME_MS * 3),
										     pdTRUE,
										     NULL,
										     AoA_IqReportProcessTimerCallback);    
#if !AoA_MANAUL_SETUP_ARRAY
	AoA_setupArrayTimer = xTimerCreate("AoA_setupArrayTimer",
										pdMS_TO_TICKS(AoA_SETUP_HEARBEAT_MS),
										pdTRUE,
										NULL,
										AoA_SetupArrayTimerCallback);
	AoA_arrayCheckerTimer = xTimerCreate("AoA_arrayCheckerTimer",
										  pdMS_TO_TICKS(AoA_SETUP_HEARBEAT_MS * 5),
										  pdTRUE,
										  NULL,
										  AoA_ArrayCheckerTimerCallback);
    for (int i = 0; i < AoA_DEVICE_NUM_MAX; i++) {
        sprintf(g_deviceList[i].deviceAddr, "%X:%X:%X:%X:%X:%X", 
            AoA_deviceAddrMap[i][0], AoA_deviceAddrMap[i][1], 
            AoA_deviceAddrMap[i][2], AoA_deviceAddrMap[i][3], 
            AoA_deviceAddrMap[i][4], AoA_deviceAddrMap[i][5]);
    }
    AoA_ArrayCheckerInit();
    AoA_MakeAddrStrMap();
    printf("AoA_MQTT_Setup_8266...\n");
    AoA_MQTT_Setup_8266();
    printf("AoA_SetupArray...\n");
	AoA_SetupArray();
#endif
}


