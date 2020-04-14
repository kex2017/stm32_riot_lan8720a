APPLICATION = kl-ftu

RIOTBASE ?= $(CURDIR)/../RIOT

BOARD ?= kl-ftu
QUIET ?= 0

CFLAGS += -DDEVELHELP 
ROM_START_ADDR ?=0x08000000

# FEATURES_REQUIRED += periph_uart
# FEATURES_OPTIONAL += periph_uart_modecfg
# FEATURES_REQUIRED += periph_gpio
# FEATURES_REQUIRED += periph_timer
# FEATURES_REQUIRED += periph_spi
# FEATURES_REQUIRED += periph_dma

# USEMODULE += cgpdov_fpga
# USEMODULE += periph_gpio_irq
# USEMODULE += isrpipe
# USEMODULE += isrpipe_read_timeout
USEMODULE += xtimer
USEMODULE += printf_float
# USEMODULE += fmt
# USEMODULE += shell
# USEMODULE += shell_commands
# USEMODULE += ps

USEMODULE += stm32_eth

USEMODULE += ipv4_addr
USEMODULE += lwip_arp
USEMODULE += lwip_ipv4
# USEMODULE += lwip_sock
# USEMODULE += lwip_dhcp_auto
CFLAGS += -DETHARP_SUPPORT_STATIC_ENTRIES=1


# including lwip_ipv6_mld would currently break this test on at86rf2xx radios
USEMODULE += lwip lwip_sock_ip lwip_netdev
USEMODULE += lwip_udp lwip_sock_udp
USEMODULE += lwip_tcp lwip_sock_tcp
USEMODULE += lwip_sock_async
USEMODULE += sock_async_event
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps
USEMODULE += od
USEMODULE += netdev_default


USEMODULE += $(DRIVER)

include $(RIOTBASE)/Makefile.include
