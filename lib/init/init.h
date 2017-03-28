#ifndef INIT_H_
#define INIT_H_

int get_ipv6_addr_from_ll(ipv6_addr_t* my_addr, kernel_pid_t radio_pid);
kernel_pid_t singlehop_init(void);

void dutycycling_init(void);

void default_init(void);

#endif
