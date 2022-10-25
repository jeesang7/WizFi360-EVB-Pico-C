/**
 * Copyright (c) 2022 WIZnet Co.,Ltd
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * ----------------------------------------------------------------------------------------------------
 * Includes
 * ----------------------------------------------------------------------------------------------------
 */
#include <stdio.h>
#include <string.h>

#include "port_common.h"
#include "timer.h"
#include "iot_socket.h"
#include "pico/stdlib.h"

/**
 * ----------------------------------------------------------------------------------------------------
 * Macros
 * ----------------------------------------------------------------------------------------------------
 */
/* Buffer */
#define ETHERNET_BUF_MAX_SIZE (1024 * 2)

/* TCP */
#define TARGET_PORT 6000
#define RECV_TIMEOUT (10 * 1000)

/**
 * ----------------------------------------------------------------------------------------------------
 * Variables
 * ----------------------------------------------------------------------------------------------------
 */

/* TCP */
static uint8_t g_tcp_send_buf[ETHERNET_BUF_MAX_SIZE] = {
    0,
};
static uint8_t g_tcp_recv_buf[ETHERNET_BUF_MAX_SIZE] = {
    0,
};
static uint8_t g_tcp_target_ip[4] = {192, 168, 0, 27};

/* Timer  */
static volatile uint32_t g_msec_cnt = 0;

const uint BTN_PIN_1 = 13;
const uint LED_PIN_1 = 14;
const uint BTN_PIN_2 = 12;
const uint LED_PIN_2 = 15;
const uint BTN_PIN_3 = 9;
const uint LED_PIN_3 = 10;

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */

/* TCP */
int recv_timeout(void *ctx, unsigned char *buf, size_t len, uint32_t timeout);

/* Timer  */
static void repeating_timer_callback(void);
static time_t millis(void);

/**
 * ----------------------------------------------------------------------------------------------------
 * demo
 * ----------------------------------------------------------------------------------------------------
 */
int demo( void )
{
    int retval = 0;
    int32_t af;
    uint32_t ip_len = 4;
    uint8_t sock;
    uint32_t send_cnt = 0;

    wizchip_1ms_timer_initialize(repeating_timer_callback);

    af = IOT_SOCKET_AF_INET;

    sock = iotSocketCreate (af, IOT_SOCKET_SOCK_STREAM, IOT_SOCKET_IPPROTO_TCP);
    if (sock < 0){
        printf("iotSocketCreate failed %d\r\n", sock);
        return sock;
    }

    retval = iotSocketConnect (sock, g_tcp_target_ip, 4, TARGET_PORT);
    if (retval < 0){
        iotSocketClose (sock);
        printf("iotSocketConnect failed %d\r\n", retval);
        return retval;
    }

    while(1)
    {   
        sprintf(g_tcp_send_buf, "Hello WizFi360-EVB-Pico-C TCP_Client_Demo send_cnt = %d\r\n", send_cnt++);
        retval = iotSocketSend(sock, g_tcp_send_buf, strlen(g_tcp_send_buf));
        printf("iotSocketSend retval = %d\r\n", retval);

        retval = recv_timeout(sock, g_tcp_recv_buf, ETHERNET_BUF_MAX_SIZE, RECV_TIMEOUT);
        if(retval > 0){
            printf("recv_timeout retval = %d\r\n", retval);
            printf("%.*s\r\n", retval, g_tcp_recv_buf);

            if (strncmp(g_tcp_recv_buf, "ledon1", 6) == 0) {
                printf("receive ledon1 then gpio led1 on!\n");
                gpio_put(LED_PIN_1, 1);
            }

            if (strncmp(g_tcp_recv_buf, "ledon2", 6) == 0) {
                printf("receive ledon2 then gpio led2 on!\n");
                gpio_put(LED_PIN_2, 1);
            }

            if (strncmp(g_tcp_recv_buf, "ledon3", 6) == 0) {
                printf("receive ledon3 then gpio led3 on!\n");
                gpio_put(LED_PIN_3, 1);
            }
        }
    }
}

int gpio_handler(void)
{
    // #1
    gpio_init(BTN_PIN_1);
    gpio_set_dir(BTN_PIN_1, GPIO_IN);
    // We are using the button to pull down to 0v when pressed, so ensure that when
    // unpressed, it uses internal pull ups. Otherwise when unpressed, the input will
    // be floating.
    gpio_pull_up(BTN_PIN_1);

    gpio_init(LED_PIN_1);
    gpio_set_dir(LED_PIN_1, GPIO_OUT);
    gpio_put(LED_PIN_1, 0);
    sleep_ms(1000);
    gpio_put(LED_PIN_1, 1);

    // #2
    gpio_init(BTN_PIN_2);
    gpio_set_dir(BTN_PIN_2, GPIO_IN);
    gpio_pull_up(BTN_PIN_2);

    gpio_init(LED_PIN_2);
    gpio_set_dir(LED_PIN_2, GPIO_OUT);
    gpio_put(LED_PIN_2, 0);
    sleep_ms(1000);
    gpio_put(LED_PIN_2, 1);

    // #3
    gpio_init(BTN_PIN_3);
    gpio_set_dir(BTN_PIN_3, GPIO_IN);
    gpio_pull_up(BTN_PIN_3);

    gpio_init(LED_PIN_3);
    gpio_set_dir(LED_PIN_3, GPIO_OUT);
    gpio_put(LED_PIN_3, 0);
    sleep_ms(1000);
    gpio_put(LED_PIN_3, 1);


    while (1)
    {
        if (gpio_get(BTN_PIN_1)) {
            printf("button1 pushed\n");
            gpio_put(LED_PIN_1, 0);
        }

        if (gpio_get(BTN_PIN_2)) {
            printf("button2 pushed\n");
            gpio_put(LED_PIN_2, 0);
        }

        if (gpio_get(BTN_PIN_3)) {
            printf("button3 pushed\n");
            gpio_put(LED_PIN_3, 0);
        }

        sleep_ms(250);
    }
}

/* Timer */
static void repeating_timer_callback(void)
{
    g_msec_cnt++;
}

static time_t millis(void)
{
    return g_msec_cnt;
}

/* TCP */
int recv_timeout(void *ctx, unsigned char *buf, size_t len, uint32_t timeout)
{
    int32_t ret;
    uint32_t n = timeout;

    ret = iotSocketSetOpt (((uint8_t)ctx), IOT_SOCKET_SO_RCVTIMEO, &n, sizeof(n));
    if (ret < 0)
        return (-1);

    ret = iotSocketRecv ((uint8_t)ctx, buf, len);
    if (ret < 0)
        return (0);

    return ret;
}
