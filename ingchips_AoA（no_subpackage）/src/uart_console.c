#include "uart_console.h"
#include "ingsoc.h"
#include <stdio.h>
#include <string.h>
#include "platform_api.h"
#include "bluetooth.h"
#include "AoA_service.h"

typedef void (*f_cmd_handler)(const char *param);

typedef struct
{
    const char *cmd;
    f_cmd_handler handler;
} cmd_t;

const static char error[] = "error";

static void tx_data(const char *d, const uint16_t len);

static const char help[] =  "commands:\n"
                            "h/?                        show this\n"
                            "conn   xx:xx:xx:xx:xx:xx   connect to dev and discover services\n"
                            "cancel                     cancel create connection\n"
                            "ascan  xx:xx:xx:xx:xx:xx   active scan for adv from a device\n"
                            "sub    handle              subscribe to a characteristic\n"
                            "unsub  handle              unsubscribe\n"
                            "8266                       setup 8266 to MQTT\n"
                            ;

void cmd_help(const char *param)
{
    tx_data(help, strlen(help) + 1);
}

extern uint8_t slave_addr[];

void conn_to_slave(void);
void cancel_create_conn(void);
void sub_to_char(int handle);
void unsub_to_char(int handle);

int parse_addr(uint8_t *output, const char *param)
{
    int addr[6];
    int i;
    if (sscanf(param, "%2x:%2x:%2x:%2x:%2x:%2x", &addr[0], &addr[1], &addr[2], &addr[3], &addr[4], &addr[5]) != 6)
    {
        tx_data(error, strlen(error) + 1);
        return -1;
    }
    for (i = 0; i < 6; i++) output[i] = addr[i];
    return 0;
}
void cmd_conn(const char *param)
{
    if (0 == parse_addr(slave_addr, param)) {
        conn_to_slave();
#if AoA_MANAUL_SETUP_ARRAY
        AoA_SetAddrStrMapByUart(slave_addr);
#endif
    }
}

void cmd_conn_cancel(const char *param)
{
    cancel_create_conn();
}

void cmd_sub_char(const char *param)
{
    int t = 0;
    if (sscanf(param, "%d", &t) != 1)
    {
        tx_data(error, strlen(error) + 1);
        return;
    }
    sub_to_char(t);
}

void AoA_MQTT_Setup_8266(void);
void cmd_setup_8266(const char *param)
{
    AoA_MQTT_Setup_8266();
}

void cmd_unsub_char(const char *param)
{
    int t = 0;
    if (sscanf(param, "%d", &t) != 1)
    {
        tx_data(error, strlen(error) + 1);
        return;
    }
    unsub_to_char(t);
}

static cmd_t cmds[] =
{
    {
        .cmd = "h",
        .handler = cmd_help
    },
    {
        .cmd = "?",
        .handler = cmd_help
    },
    {
        .cmd = "conn",
        .handler = cmd_conn
    },
    {
        .cmd = "cancel",
        .handler = cmd_conn_cancel
    },
    {
        .cmd = "sub",
        .handler = cmd_sub_char
    },
    {
        .cmd = "unsub",
        .handler = cmd_unsub_char
    },
    {
        .cmd = "8266",
        .handler = cmd_setup_8266
    },
};

void handle_command(char *cmd_line)
{
    static const char unknow_cmd[] =  "unknown command\n";
    char *param = cmd_line;
    int i;
    while ((*param != ' ') && (*param != '\0')) param++;
    *param = '\0'; param++;

    for (i = 0; i < sizeof(cmds) / sizeof(cmds[0]); i++)
    {
        if (strcasecmp(cmds[i].cmd, cmd_line) == 0)
            break;
    }
    if (i >= sizeof(cmds) / sizeof(cmds[0]))
        goto show_help;

    cmds[i].handler(param);
    return;

show_help:
    tx_data(unknow_cmd, strlen(unknow_cmd) + 1);
    cmd_help(NULL);
}

typedef struct
{
    uint16_t size;
    char buf[712];
} str_buf_t;

str_buf_t input = {0};
str_buf_t output = {0};

static void append_data(str_buf_t *buf, const char *d, const uint16_t len)
{
    if (buf->size + len > sizeof(buf->buf))
        buf->size = 0;

    if (buf->size + len <= sizeof(buf->buf))
    {
        memcpy(buf->buf + buf->size, d, len);
        buf->size += len;
    }
}

void console_rx_data(const char *d, uint8_t len)
{
    if (0 == input.size)
    {
        while ((len > 0) && ((*d == '\r') || (*d == '\n')))
        {
            d++;
            len--;
        }
    }
    if (len == 0) return;

    append_data(&input, d, len);

    if ((input.size > 0) &&
        ((input.buf[input.size - 1] == '\r') || (input.buf[input.size - 1] == '\r')))
    {
        int16_t t = input.size - 2;
        while ((t > 0) && ((input.buf[t] == '\r') || (input.buf[t] == '\n'))) t--;
        input.buf[t + 1] = '\0';
        handle_command(input.buf);
        input.size = 0;
    }
}

static void tx_data(const char *d, const uint16_t len)
{
    if ((output.size == 0) && (d[len - 1] == '\0'))
    {
        puts(d);
        return;
    }

    append_data(&output, d, len);

    if ((output.size > 0) && (output.buf[output.size - 1] == '\0'))
    {
        puts(output.buf);
        output.size = 0;
    }
}
