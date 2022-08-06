#include "platform_api.h"
#include "profile.h"
#include "att_db.h"
#include "gap.h"
#include "btstack_event.h"
#include "btstack_util.h"
#include "btstack_defines.h"
#include "gatt_client.h"
#include "sig_uuid.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "str_util.h"
#include "trace.h"
#include "sm.h"

#include "uart_console.h"
#include "gatt_client_util.h"

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "AoA_service.h"

#include "../data/gatt.const"
#include "../data/advertising.const"

#define MAX_ADVERTISERS     50
bd_addr_t scaned_advertisers[MAX_ADVERTISERS] = {0};
int advertiser_num = 0;
int is_targeted_scan = 0;
uint64_t last_seen = 0;

struct gatt_client_discoverer *discoverer = NULL;
sm_persistent_t sm_persistent =
{
    .er = {1, 2, 3},
    .ir = {4, 5, 6},
    .identity_addr_type     = BD_ADDR_TYPE_LE_RANDOM,
    .identity_addr          = {0xC6, 0xFA, 0x5C, 0x20, 0x87, 0xA7}
};

#define find_char(handle)   gatt_client_util_find_char(discoverer, handle)
#define find_config_desc    gatt_client_util_find_config_desc

// GATT characteristic handles
#include "../data/gatt.const"

static uint8_t adv_data[31] = {
    #include "../data/advertising.adv"
};

const static uint8_t scan_data[] = {
    #include "../data/scan_response.adv"
};

const static uint8_t profile_data[] = {
    #include "../data/gatt.profile"
};

#define INVALID_HANDLE  0xffff
uint16_t mas_conn_handle = INVALID_HANDLE;
uint16_t sla_conn_handle = INVALID_HANDLE;
static int bonding_flag = 0;

static uint16_t att_read_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t offset,
                                  uint8_t * buffer, uint16_t buffer_size)
{
    switch (att_handle)
    {

    default:
        return 0;
    }
}

static btstack_packet_callback_registration_t hci_event_callback_registration;

static int att_write_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t transaction_mode,
                              uint16_t offset, const uint8_t *buffer, uint16_t buffer_size)
{
    switch (att_handle)
    {

    default:
        return 0;
    }
}

uint8_t slave_addr[] = {0,0,0,0,0,0};
bd_addr_type_t slave_addr_type = BD_ADDR_TYPE_LE_RANDOM;

void do_set_data()
{
    gap_set_ext_adv_data(0, adv_data[ADVERTISING_ITEM_OFFSET_COMPLETE_LOCAL_NAME - 2]
                            + ADVERTISING_ITEM_OFFSET_COMPLETE_LOCAL_NAME - 1, (uint8_t *)(adv_data));
}

#define iprintf platform_printf

void print_fun(const char *s)
{
    printf("%s\n", s);
}

void read_characteristic_value_callback(uint8_t packet_type, uint16_t _, const uint8_t *packet, uint16_t size)
{
    switch (packet[0])
    {
    case GATT_EVENT_CHARACTERISTIC_VALUE_QUERY_RESULT:
        {
            uint16_t value_size;
            const gatt_event_value_packet_t *value =
                gatt_event_characteristic_value_query_result_parse(packet, size, &value_size);
            if (value_size)
            {
                iprintf("VALUE of %d:\n", value->handle);
                print_hex_table(value->value, value_size, print_fun);
            }
        }
        break;
    case GATT_EVENT_QUERY_COMPLETE:
        iprintf("read_characteristic_value COMPLETE: %d\n", gatt_event_query_complete_parse(packet)->status);
        if (gatt_event_query_complete_parse(packet)->status != 0)
        {
            gap_disconnect(0);
            break;
        }
        break;
    }
}

void write_characteristic_value_callback(uint8_t packet_type, uint16_t _, const uint8_t *packet, uint16_t size)
{
    switch (packet[0])
    {
    case GATT_EVENT_QUERY_COMPLETE:
        iprintf("write_characteristic_value COMPLETE: %d\n", gatt_event_query_complete_parse(packet)->status);
        if (gatt_event_query_complete_parse(packet)->status != 0)
        {
            gap_disconnect(0);
            break;
        }
        break;
    }
}

void write_characteristic_descriptor_callback(uint8_t packet_type, uint16_t _, const uint8_t *packet, uint16_t size)
{
    switch (packet[0])
    {
    case GATT_EVENT_QUERY_COMPLETE:
        iprintf("write_characteristic_descriptor COMPLETE: %d\n", gatt_event_query_complete_parse(packet)->status);
        if (gatt_event_query_complete_parse(packet)->status != 0)
        {
            gap_disconnect(0);
            break;
        }
        break;
    }
}

static void output_notification_handler(uint8_t packet_type, uint16_t channel, const uint8_t *packet, uint16_t size)
{
    const gatt_event_value_packet_t *value;
    uint16_t value_size;
    switch (packet[0])
    {
    case GATT_EVENT_NOTIFICATION:
        value = gatt_event_notification_parse(packet, size, &value_size);
        if (value_size) {
            int i;
            strcpy(g_iq_str_buffer, value->value);
            strcat(g_iq_str_buffer, "\n");
            g_iqReportStrLen = strlen(g_iq_str_buffer) + AoA_DEVICE_ADDR_LEN + 1;
            // iprintf("NOTIFACATION of %d:\n", value->handle);
            // print_hex_table(value->value, value_size, print_fun);
#if AoA_MANAUL_SETUP_ARRAY
            if ((channel >= 0) && (channel < g_devNum)) {
#else
            if ((channel >= 0) && (channel < AoA_DEVICE_NUM_MAX)) {
                AoA_arrayChecker[1][channel]++;
#endif
                strcpy(g_deviceList[channel].iq_str_buffer, AoA_addrStrMap[channel]);
                strcat(g_deviceList[channel].iq_str_buffer, g_iq_str_buffer);
                strcat(g_deviceList[channel].iq_str_buffer, "\n");
            }
        }
        break;
    case GATT_EVENT_INDICATION:
        value = gatt_event_indication_parse(packet, size, &value_size);
        if (value_size)
        {
            iprintf("INDICATION of %d:\n", value->handle);
            print_hex_table(value->value, value_size, print_fun);
        }
        break;
    }
}


#define USER_MSG_START_ADV          0
#define USER_MSG_STOP_ADV           1
#define USER_MSG_CONN_TO_SLAVE      2
#define USER_MSG_CONN_CANCEL        3
#define USER_MSG_SUB_TO_CHAR        4
#define USER_MSG_UNSUB_TO_CHAR      5

const static ext_adv_set_en_t adv_sets_en[] = {{.handle = 0, .duration = 0, .max_events = 0}};

#define CONN_PARAM  {                   \
            .scan_int = 200,            \
            .scan_win = 200,            \
            .interval_min = 50,         \
            .interval_max = 50,         \
            .latency = 0,               \
            .supervision_timeout = 600, \
            .min_ce_len = 90,           \
            .max_ce_len = 90            \
    }

static initiating_phy_config_t phy_configs[] =
{
    {
        .phy = PHY_1M,
        .conn_param = CONN_PARAM
    },
};

static scan_phy_config_t scan_configs[] =
{
    {
        .phy = PHY_1M,
        .type = SCAN_PASSIVE,
        .interval = 200,
        .window = 200
    },
};

static void user_msg_handler(uint32_t msg_id, void *data, uint16_t size)
{
    switch (msg_id)
    {
    case USER_MSG_START_ADV:
        gap_set_ext_adv_enable(1, sizeof(adv_sets_en) / sizeof(adv_sets_en[0]), adv_sets_en);
        printf("adv started\n");
        break;
    case USER_MSG_STOP_ADV:
        gap_set_ext_adv_enable(0, 0, NULL);
        printf("adv stopped\n");
        break;
    case USER_MSG_CONN_TO_SLAVE:
        printf("create connection...\n");
        int ret = gap_ext_create_connection(INITIATING_ADVERTISER_FROM_PARAM,
                                                  BD_ADDR_TYPE_LE_RANDOM,           // Own_Address_Type,
                                                  slave_addr_type,                  // Peer_Address_Type,
                                                  slave_addr,                       // Peer_Address,
                                                  sizeof(phy_configs) / sizeof(phy_configs[0]),
                                                  phy_configs);
        printf("user_msg_handler: ret = %d\n", ret);
        break;
    case USER_MSG_CONN_CANCEL:
        printf("create connection cancelled.\n");
        gap_create_connection_cancel();
        break;
    case USER_MSG_SUB_TO_CHAR:
        {
            uint16_t config = GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NOTIFICATION;
            char_node_t *c = find_char(size);
            desc_node_t *d = find_config_desc(c);
            if (NULL == d)
            {
                iprintf("CHAR/DESC not found: %d\n", size);
                break;
            }

            if (c->notification == NULL)
            {
                c->notification = (gatt_client_notification_t *)malloc(sizeof(gatt_client_notification_t));
                gatt_client_listen_for_characteristic_value_updates(
                    c->notification, output_notification_handler,
                    mas_conn_handle, c->chara.value_handle);
            }

            gatt_client_write_characteristic_descriptor_using_descriptor_handle(
                write_characteristic_descriptor_callback,
                mas_conn_handle,
                d->desc.handle,
                sizeof(config),
                (uint8_t *)&config);
#if AoA_MANAUL_SETUP_ARRAY
            g_checkNewFlg = 1;
#endif
            xTimerStart(AoA_iqReportProcessTimer, 0);
        }
        break;
    case USER_MSG_UNSUB_TO_CHAR:
        {
            uint16_t config = 0;
            char_node_t *c = find_char(size);
            desc_node_t *d = find_config_desc(c);
            if (NULL == d)
            {
                iprintf("CHAR/DESC not found: %d\n", size);
                break;
            }

            gatt_client_write_characteristic_descriptor_using_descriptor_handle(
                write_characteristic_descriptor_callback,
                mas_conn_handle,
                d->desc.handle,
                sizeof(config),
                (uint8_t *)&config);

            xTimerStop(AoA_iqReportProcessTimer, 0);
        }
        break;
    default:
        ;
    }
}

void start_adv()
{
    btstack_push_user_msg(USER_MSG_START_ADV, NULL, 0);
}

void stop_adv()
{
    btstack_push_user_msg(USER_MSG_STOP_ADV, NULL, 0);
}

void conn_to_slave()
{
    btstack_push_user_msg(USER_MSG_CONN_TO_SLAVE, NULL, 0);
}

void cancel_create_conn()
{
    btstack_push_user_msg(USER_MSG_CONN_CANCEL, NULL, 0);
}

void sub_to_char(int handle)
{
    btstack_push_user_msg(USER_MSG_SUB_TO_CHAR, NULL, (uint16_t)handle);
}

void unsub_to_char(int handle)
{
    btstack_push_user_msg(USER_MSG_UNSUB_TO_CHAR, NULL, (uint16_t)handle);
}


int is_new_advertiser(const uint8_t *addr)
{
    int i;
    for (i = 0; i < advertiser_num; i++)
    {
        if (memcmp(scaned_advertisers[i], addr, BD_ADDR_LEN) == 0)
            return 0;
    }
    if (i >= MAX_ADVERTISERS) return 0;
    memcpy(scaned_advertisers[i], addr, BD_ADDR_LEN);
    advertiser_num++;
    return 1;
}

#pragma pack (push, 1)
typedef struct read_remote_version
{
    uint8_t Status;
    uint16_t Connection_Handle;
    uint8_t Version;
    uint16_t Manufacturer_Name;
    uint16_t Subversion;
} read_remote_version_t;
#pragma pack (pop)

static void print_features(const le_meta_event_read_remote_feature_complete_t * complete)
{
    static const char features[][48] =
    {
        "LE Encryption",
        "Connection Parameters Request",
        "Extended Reject Indication",
        "Slave-initiated Features Exchange",
        "LE Ping",
        "LE Data Packet Length Extension",
        "LL Privacy",
        "Extended Scanner Filter Policies",
        "LE 2M PHY",
        "Stable Modulation Index - Transmitter",
        "Stable Modulation Index - Receiver",
        "LE Coded PHY",
        "LE Extended Advertising",
        "LE Periodic Advertising",
        "Channel Selection Algorithm #2",
        "LE Power Class 1",
        "Minimum Number of Used Channels Procedure",
        "Connection CTE Request",
        "Connection CTE Response",
        "Connectionless CTE Transmitter",
        "Connectionless CTE Receiver",
        "Antenna Switching During CTE Transmission (AoD)",
        "Antenna Switching During CTE Reception (AoA)",
        "Receiving Constant Tone Extensions",
        "Periodic Advertising Sync Transfer - Sender",
        "Periodic Advertising Sync Transfer - Recipient",
        "Sleep Clock Accuracy Updates",
        "Remote Public Key Validation",
        "Connected Isochronous Stream - Master",
        "Connected Isochronous Stream - Slave",
        "Isochronous Broadcaster",
        "Synchronized Receiver",
        "Isochronous Channels (Host Support)",
        "LE Power Control Request",
        "LE Power Control Request",
        "LE Path Loss Monitoring",
        "Periodic Advertising ADI",
        "Connection Subrating",
        "Connection Subrating (Host Support)",
        "Channel Classification",        
    };
    int i;
    iprintf("Features of peer #%d (status %d)\n", complete->handle, complete->status);
    for (i = 0; i < sizeof(features) / sizeof(features[0]); i++)
    {
        int B_i = i >> 3;
        int b_i = i & 0x7;
        iprintf("[%c] %s\n", complete->features[B_i] & (1 << b_i) ? '*' : ' ', features[i]);
    }
    iprintf("\n");
}

static const char *decode_version(int ver)
{
    switch (ver)
    {
        case 6: return "4.0";
        case 7:  return "4.1";
        case 8:  return "4.2";
        case 9:  return "5.0";
        case 10: return "5.1";
        case 11: return "5.2";
        case 12: return "5.3";
        case 13: return "5.4";
        case 14: return "5.5";
        default: return "??";
    }
}

static void user_packet_handler(uint8_t packet_type, uint16_t channel, const uint8_t *packet, uint16_t size)
{
    uint8_t event = hci_event_packet_get_type(packet);
    const btstack_user_msg_t *p_user_msg;
    if (packet_type != HCI_EVENT_PACKET) return;

    switch (event)
    {
    case BTSTACK_EVENT_STATE:
        if (btstack_event_state_get_state(packet) != HCI_STATE_WORKING)
            break;
        
        platform_config(PLATFORM_CFG_LL_LEGACY_ADV_INTERVAL, 1500);
        
        gap_set_random_device_address(sm_persistent.identity_addr);
        gap_set_adv_set_random_addr(0, sm_persistent.identity_addr);
        gap_set_ext_adv_para(0,
                                CONNECTABLE_ADV_BIT | SCANNABLE_ADV_BIT | LEGACY_PDU_BIT,
                                0x00a1, 0x00a1,            // Primary_Advertising_Interval_Min, Primary_Advertising_Interval_Max
                                PRIMARY_ADV_ALL_CHANNELS,  // Primary_Advertising_Channel_Map
                                BD_ADDR_TYPE_LE_RANDOM,    // Own_Address_Type
                                BD_ADDR_TYPE_LE_PUBLIC,    // Peer_Address_Type (ignore)
                                NULL,                      // Peer_Address      (ignore)
                                ADV_FILTER_ALLOW_ALL,      // Advertising_Filter_Policy
                                0x00,                      // Advertising_Tx_Power
                                PHY_1M,                    // Primary_Advertising_PHY
                                0,                         // Secondary_Advertising_Max_Skip
                                PHY_1M,                    // Secondary_Advertising_PHY
                                0x00,                      // Advertising_SID
                                0x00);                     // Scan_Request_Notification_Enable
        do_set_data();
        gap_set_ext_scan_response_data(0, sizeof(scan_data), (uint8_t*)scan_data);
        break;

    case HCI_EVENT_LE_META:
        switch (hci_event_le_meta_get_subevent_code(packet))
        {
        case HCI_SUBEVENT_LE_EXTENDED_ADVERTISING_REPORT:
            {
                const le_ext_adv_report_t *report = decode_hci_le_meta_event(packet, le_meta_event_ext_adv_report_t)->reports;
                if (is_targeted_scan)
                {
                    if ((report->evt_type & HCI_EXT_ADV_PROP_SCAN_RSP) == 0)
                    {
                        uint64_t now = platform_get_us_time();
                        if (last_seen != 0)
                            platform_printf("Interval: %d ms\n", (int)((now - last_seen) / 1000));
                        last_seen = now;
                    }
                    
                    platform_printf("ADV %02X:%02X:%02X:%02X:%02X:%02X (%s) %ddBm\n"
                                "Type: 0x%02x\n",
                                report->address[5], report->address[4], report->address[3],
                                report->address[2], report->address[1], report->address[0],
                                report->addr_type ? "RANDOM" : "PUBLIC",
                                report->rssi, report->evt_type);
                }
                else
                {
                    if (!is_new_advertiser(report->address)) break;
                    platform_printf("No. %d:\n"
                                "ADV %02X:%02X:%02X:%02X:%02X:%02X (%s) %ddBm\n"
                                "Type: 0x%02x\n",
                                advertiser_num,
                                report->address[5], report->address[4], report->address[3],
                                report->address[2], report->address[1], report->address[0],
                                report->addr_type ? "RANDOM" : "PUBLIC",
                                report->rssi, report->evt_type);
                }
                    
                
                print_hex_table(report->data, report->data_len, print_fun);
                platform_printf("\n");
            }
            break;
        case HCI_SUBEVENT_LE_ENHANCED_CONNECTION_COMPLETE:
            {
                const le_meta_event_enh_create_conn_complete_t * complete =
                    decode_hci_le_meta_event(packet, le_meta_event_enh_create_conn_complete_t);

                if (HCI_ROLE_SLAVE == complete->role)
                {
                    sla_conn_handle = complete->handle;
                    att_set_db(sla_conn_handle, profile_data);
                }
                else
                {
                    mas_conn_handle = complete->handle;
                    gap_read_remote_info(mas_conn_handle);
                }
            }
            break;
        case HCI_SUBEVENT_LE_READ_REMOTE_USED_FEATURES_COMPLETE:
            {
                const le_meta_event_read_remote_feature_complete_t * complete =
                    decode_hci_le_meta_event(packet, le_meta_event_read_remote_feature_complete_t);
                print_features(complete);
                if (complete->handle == mas_conn_handle)
                {
                    if (0 == bonding_flag)
                    {
                        iprintf("discovering...\n");
                        discoverer = gatt_client_util_discover_all(mas_conn_handle, gatt_client_util_dump_profile, NULL);
                    }
                }
            }
            break;
        default:
            break;
        }
        break;

    case HCI_EVENT_ENCRYPTION_CHANGE:
        {
            const hci_encryption_change_event_t *complete =
                decode_hci_event(packet, hci_encryption_change_event_t);
            if (complete->conn_handle == mas_conn_handle)
            {
                iprintf("discovering...\n");
                discoverer = gatt_client_util_discover_all(mas_conn_handle, gatt_client_util_dump_profile, NULL);
            }
        }
        break;

    case HCI_EVENT_DISCONNECTION_COMPLETE:
        {
            const event_disconn_complete_t *complete = decode_hci_event_disconn_complete(packet);
            iprintf("disconnected\n");
            if (complete->conn_handle == sla_conn_handle)
            {
                sla_conn_handle = INVALID_HANDLE;
            }
            if (complete->conn_handle == mas_conn_handle)
            {
                mas_conn_handle = INVALID_HANDLE;
                if (discoverer)
                {
                    gatt_client_util_free(discoverer);
                    discoverer = NULL;
                }
            }
#if !AoA_MANAUL_SETUP_ARRAY
            g_deviceDisonnected = 1;
#endif
        }
        break;

    case HCI_EVENT_COMMAND_COMPLETE:
        {
            const uint8_t *returns = hci_event_command_complete_get_return_parameters(packet);
            if (*returns != 0)
                platform_printf("COMMAND_COMPLETE: 0x%02x for OPCODE %04X\n",
                    *returns, hci_event_command_complete_get_command_opcode(packet));
        }
        break;
    case HCI_EVENT_COMMAND_STATUS:
        {
            const uint8_t status = hci_event_command_status_get_status(packet);
            if (status != 0)
                platform_printf("COMMAND_STATUS: 0x%02x for OPCODE %04X\n",
                    status, hci_event_command_status_get_command_opcode(packet));
        }
        break;
    case HCI_EVENT_READ_REMOTE_VERSION_INFORMATION_COMPLETE:
        {
            const read_remote_version_t *version = decode_event_offset(packet, read_remote_version_t, 2);
            if (version->Status == 0)
            {
                platform_printf("Remote version\n"
                "Version          : %d (%s)\n"
                "Manufacturer Name: 0x%04X\n"
                "Subversion       : 0x%04X\n",
                version->Version, decode_version(version->Version),
                version->Manufacturer_Name, version->Subversion);
            }
            if (version->Connection_Handle == mas_conn_handle)
                gap_read_remote_used_features(mas_conn_handle);
#if !AoA_MANAUL_SETUP_ARRAY
            g_deviceConnected = 1;
#endif
        }
        break;

    case GATT_EVENT_MTU:
        iprintf("GATT client MTU updated: %d\n", gatt_event_mtu_get_mtu(packet));
        break;
    case ATT_EVENT_CAN_SEND_NOW:
        // add your code
        break;

    case BTSTACK_EVENT_USER_MSG:
        p_user_msg = hci_event_packet_get_user_msg(packet);
        user_msg_handler(p_user_msg->msg_id, p_user_msg->data, p_user_msg->len);
        break;

    default:
        break;
    }
}



uint32_t setup_profile(void *data, void *user_data)
{
	printf("setup_profile\n");
    att_server_init(att_read_callback, att_write_callback);
    hci_event_callback_registration.callback = &user_packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);
    att_server_register_packet_handler(&user_packet_handler);
    gatt_client_register_handler(&user_packet_handler);
    AoA_ServiceInit();
    return 0;
}

