#include <string.h>
#include <stdint.h>

#include "jerry.h"

#include "os.h"
#include "net_init.h"
#include "network_init.h"
#include "wifi_api.h"
#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "lwip/sockets.h"
#include "ethernetif.h"
#include "lwip/sockets.h"
#include "netif/etharp.h"

#include "microlattice.h"

char udp_rcv_buf_old[100] = {0};

DELCARE_HANDLER(udpClient) {
  if (args_cnt == 2 && args_p[0].type == JERRY_API_DATA_TYPE_OBJECT) {
    /* ip */
    int ip_req_sz = jerry_api_string_to_char_buffer(args_p[0].v_string, NULL, 0);
    ip_req_sz *= -1;
    char ip_buffer [ip_req_sz + 1];
    ip_req_sz = jerry_api_string_to_char_buffer (args_p[0].v_string, (jerry_api_char_t *) ip_buffer, ip_req_sz);
    ip_buffer[ip_req_sz] = '\0';

    int s;
    int ret;
    struct sockaddr_in addr;
    int count = 0;
    int rcv_len, rlen;

    char rcv_buf[64] = {0};
    char send_data[] = "hello,server";
    printf("udp_client_test start\n");
    // struct netif *sta_if= netif_find("st2");

    os_memset(&addr, 0, sizeof(addr));
    addr.sin_len = sizeof(addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons((int)args_p[1].v_float32);
    addr.sin_addr.s_addr =inet_addr(ip_buffer);
    // addr.sin_port = lwip_htons(SOCK_UDP_SRV_PORT);
    // inet_addr_from_ipaddr(&addr.sin_addr, netif_ip4_addr(sta_if));

    /* create the socket */
    s = lwip_socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        printf("udp client create fail\n");
        goto idle;
    }

    /* connect */
    ret = lwip_connect(s, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        lwip_close(s);
        printf("udp client connect fail\n");
        goto idle;
    }

    for (;;) {
        /* write something */
      // ret = lwip_write(s, send_data, sizeof(send_data));
      // printf("udp client write:ret = %d\n", ret);

      // printf("udp client waiting for server data...\n");
      // rlen = lwip_read(s, rcv_buf, sizeof(rcv_buf) - 1);
      // rcv_buf[rlen] = 0;
      // printf("udp client received data:%s\n", rcv_buf);
      // count++;
      // vTaskDelay(2000);

      char rcv_buf[100] = {0};
      if (0 == count) {
        ret = lwip_write(s, send_data, sizeof(send_data));
        printf("tcp client write:ret = %s\n", ret);
      }
      printf("tcp client waiting for data...\n");
      rcv_len = 0;
      rlen = lwip_recv(s, &rcv_buf[rcv_len], sizeof(rcv_buf) - 1 - rcv_len, 0);
      rcv_len += rlen;

      printf("rcv_buf: %s\n", rcv_buf);

      if (strcmp(udp_rcv_buf_old, rcv_buf) != 0) {
        jerry_api_value_t params[0];
        params[0].type = JERRY_API_DATA_TYPE_STRING;
        params[0].v_string = jerry_api_create_string (rcv_buf);
        jerry_api_call_function(args_p[2].v_object, NULL, false, &params, 1);
        *udp_rcv_buf_old = "";
        strcpy(*udp_rcv_buf_old, rcv_buf);
        jerry_api_release_value(&params);
      }
      count++;
      vTaskDelay(1000);
    }

    /* close */
    // ret = lwip_close(s);
    // printf("udp client s close:ret = %d\n", ret);

idle:
    ret_val_p->type = JERRY_API_DATA_TYPE_BOOLEAN;
    ret_val_p->v_bool = true;

    return true;
  }
}

void ml_udp_init(void) {
  REGISTER_HANDLER(udpClient);
}
