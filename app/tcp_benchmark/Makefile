APPLICATION = tcp_benchmark
RIOTBASE ?= $(CURDIR)/../../RIOT-OS
BOARD = hamilton
USEMODULE += rtt_stdio
CFLAGS += -DRTT_STDIO_DISABLE_STDIN
CFLAGS += -DSOCK_HAS_IPV6

# This way you can include libraries
DIRS += $(CURDIR)/init
USEMODULE += hamilton_lib_init

# Radio
USEMODULE += at86rf233

# Prefix length is assumed to be 64
CFLAGS += -DHAMILTON_BORDER_ROUTER_ADDRESS=\"2001:470:4889:115:7b0a:fa74:734b:7679\"

# Unset this for the Border Router... though you also would need to use the
# Border Router kernel (which is needed to work properly with the samr21)
# CFLAGS += -DI_AM_HAMILTON_BORDER_ROUTER

USEMODULE += gnrc_dutymac
USEMODULE += gnrc_icmpv6_echo
USEMODULE += random
USEMODULE += xtimer

# networking
USEMODULE += gnrc_netdev_default
USEMODULE += auto_init_gnrc_netif
USEMODULE += gnrc_ipv6_autoconf_onehop

# Specify the mandatory networking modules for IPv6 and UDP
USEMODULE += gnrc_ipv6
USEMODULE += gnrc_udp
# Add also the shell, some shell commands
USEMODULE += netstats_l2
USEMODULE += netstats_ipv6

# TCP and sock
USEMODULE += gnrc_sock_udp
USEMODULE += gnrc_tcp_freebsd
USEMODULE += gnrc_sock_tcp_freebsd
USEMODULE += posix_sockets
USEMODULE += gnrc_sixlowpan_frag

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
CFLAGS += -DDEVELHELP

DEFAULT_CHANNEL = 26
CFLAGS += -DDEFAULT_CHANNEL=$(DEFAULT_CHANNEL)
CFLAGS += -DIEEE802154_DEFAULT_CHANNEL=$(DEFAULT_CHANNEL)

QUIET ?= 1

FEATURES_REQUIRED += periph_timer

include $(RIOTBASE)/Makefile.include
