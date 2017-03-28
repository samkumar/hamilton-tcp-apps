#include "net/gnrc/ipv6.h"
#include "net/gnrc/udp.h"
#include "net/gnrc/netapi.h"
#include "net/gnrc/netreg.h"
#include "net/gnrc/ipv6/autoconf_onehop.h"

int get_ipv6_addr_from_ll(ipv6_addr_t* my_addr, kernel_pid_t radio_pid) {
    ipv6_addr_t my_ipv6_addr;
    if (ipv6_addr_from_str(&my_ipv6_addr, HAMILTON_BORDER_ROUTER_ADDRESS) == NULL) {
        perror("invalid HAMILTON_BORDER_ROUTER_ADDRESS");
        return 1;
    }

    eui64_t my_ll_addr;
    gnrc_netapi_opt_t addr_req_opt;
    msg_t addr_req;
    msg_t addr_resp;

    addr_req.type = GNRC_NETAPI_MSG_TYPE_GET;
    addr_req.content.ptr = &addr_req_opt;

    addr_req_opt.opt = NETOPT_ADDRESS_LONG;
    addr_req_opt.data = &my_ll_addr;
    addr_req_opt.data_len = sizeof(eui64_t);

    msg_send_receive(&addr_req, &addr_resp, radio_pid);

    if (addr_resp.content.value != 8) {
        printf("Link layer address length is not 8 bytes (got %u)\n", (unsigned int) addr_resp.content.value);
        return 1;
    }

    if (gnrc_ipv6_autoconf_l2addr_to_ipv6(&my_ipv6_addr, &my_ll_addr) != 0) {
        printf("Could not convert link-layer address to IP address\n");
        return 1;
    }

    if (my_addr != NULL) {
        memcpy(my_addr, &my_ipv6_addr, sizeof(ipv6_addr_t));
    }

    return 0;
}

kernel_pid_t singlehop_init(void) {
    kernel_pid_t radio_pid = get_6lowpan_pid();
    assert(radio_pid != 0);

    ipv6_addr_t my_ipv6_addr;
    if (get_ipv6_addr_from_ll(&my_ipv6_addr, radio_pid) != 0) {
        printf("Could not set IPv6 address from link layer\n");
        return 1;
    }

    char ipbuf[IPV6_ADDR_MAX_STR_LEN + 1];
    char* ipstr = ipv6_addr_to_str(ipbuf, &my_ipv6_addr, sizeof(ipbuf));
    if (ipstr != ipbuf) {
        perror("inet_ntop");
        return 1;
    }

    printf("My IP address is %s\n", ipstr);

    gnrc_ipv6_netif_t* radio_if = gnrc_ipv6_netif_get(radio_pid);
    assert(radio_if != NULL);
    gnrc_ipv6_netif_add_addr(radio_pid, &my_ipv6_addr, 128, 0);
    gnrc_ipv6_netif_set_router(radio_if, false);

    return radio_pid;
}

void dutycycling_init(void) {
  /* Leaf nodes: Low power operation and auto duty-cycling */
  kernel_pid_t radio[GNRC_NETIF_NUMOF];
  uint8_t radio_num = gnrc_netif_get(radio);
  netopt_enable_t dutycycling;
  if (DUTYCYCLE_SLEEP_INTERVAL) {
    dutycycling = NETOPT_ENABLE;
  } else {
    dutycycling = NETOPT_DISABLE;
  }
  for (int i=0; i < radio_num; i++)
    gnrc_netapi_set(radio[i], NETOPT_DUTYCYCLE, 0, &dutycycling, sizeof(netopt_t));
}


void default_init(void) {
    (void) singlehop_init();

#ifdef LEAF_NODE
    dutycycling_init();
#endif
}
