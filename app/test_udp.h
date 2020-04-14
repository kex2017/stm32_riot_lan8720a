/*
 * Copyright (C) 2016 Alexander Aring <aar@pengutronix.de>
 *                    Freie Universität Berlin
 *                    HAW Hamburg
 *                    Kaspar Schleiser <kaspar@schleiser.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_sock_udp    UDP sock API
 * @ingroup     net_sock
 * @brief       Sock submodule for UDP
 *
 * How To Use
 * ----------
 * First you need to @ref including-modules "include" a module that implements
 * this API in your application's Makefile. For example the implementation for
 * @ref net_gnrc "GNRC" is called `gnrc_sock_udp`.
 *
 * ### A Simple UDP Echo Server
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.c}
 * #include <stdio.h>
 *
 * #include "net/sock/udp.h"
 *
 * uint8_t buf[128];
 *
 * int main(void)
 * {
 *     sock_udp_ep_t local = SOCK_IPV6_EP_ANY;
 *     sock_udp_t sock;
 *
 *     local.port = 12345;
 *
 *     if (sock_udp_create(&sock, &local, NULL, 0) < 0) {
 *         puts("Error creating UDP sock");
 *         return 1;
 *     }
 *
 *     while (1) {
 *         sock_udp_ep_t remote;
 *         ssize_t res;
 *
 *         if ((res = sock_udp_recv(&sock, buf, sizeof(buf), SOCK_NO_TIMEOUT,
 *                                  &remote)) >= 0) {
 *             puts("Received a message");
 *             if (sock_udp_send(&sock, buf, res, &remote) < 0) {
 *                 puts("Error sending reply");
 *             }
 *         }
 *     }
 *
 *     return 0;
 * }
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * Above you see a simple UDP echo server. Don't forget to also
 * @ref including-modules "include" the IPv6 module of your networking
 * implementation (e.g. `gnrc_ipv6_default` for @ref net_gnrc GNRC) and at least
 * one network device.
 *
 * After including the header file for @ref net_sock_udp "UDP sock", we create some
 * buffer space `buf` to store the data received by the server:
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.c}
 * #include "net/sock/udp.h"
 *
 * uint8_t buf[128];
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * To be able to listen for incoming packets we bind the `sock` by setting a
 * local end point with a port (`12345` in this case).
 *
 * We then proceed to create the `sock`. It is bound to `local` and thus listens
 * for UDP packets with @ref udp_hdr_t::dst_port "destination port" `12345`.
 * Since we don't need any further configuration we set the flags to 0.
 * In case of an error we stop the program:
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.c}
 *     sock_udp_ep_t local = SOCK_IPV6_EP_ANY;
 *     sock_udp_t sock;
 *
 *     local.port = 12345;
 *
 *     if (sock_udp_create(&sock, &local, NULL, 0) < 0) {
 *         puts("Error creating UDP sock");
 *         return 1;
 *     }
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * The application then waits indefinitely for an incoming message in `buf`
 * from `remote`. If we want to timeout this wait period we could alternatively
 * set the `timeout` parameter of @ref sock_udp_recv() to a value != @ref
 * SOCK_NO_TIMEOUT. If an error occurs on receive we just ignore it and
 * continue looping.
 *
 * If we receive a message we use its `remote` to reply. In case of an error on
 * send we print an according message:
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.c}
 *     while (1) {
 *         sock_udp_ep_t remote;
 *         ssize_t res;
 *
 *         if ((res = sock_udp_recv(&sock, buf, sizeof(buf), SOCK_NO_TIMEOUT,
 *                                  &remote)) >= 0) {
 *             puts("Received a message");
 *             if (sock_udp_send(&sock, buf, res, &remote) < 0) {
 *                 puts("Error sending reply");
 *             }
 *         }
 *     }
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * ### A Simple UDP Echo Client
 * There are two kinds of clients. Those that do expect a reply and those who
 * don't. A client that does not require a reply is very simple to implement in
 * one line:
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.c}
 * res = sock_udp_send(NULL, data, data_len, &remote);
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * With `data` being the data sent, `data_len` the length of `data` and `remote`
 * the remote end point the packet that is is to be sent.
 *
 * To see some other capabilities we look at a more complex example in form of
 * the counter of the echo server above:
 *
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.c}
 * #include <stdio.h>
 *
 * #include "net/af.h"
 * #include "net/protnum.h"
 * #include "net/ipv6/addr.h"
 * #include "net/sock/udp.h"
 * #include "xtimer.h"
 *
 * uint8_t buf[7];
 *
 * int main(void)
 * {
 *     sock_udp_ep_t local = SOCK_IPV6_EP_ANY;
 *     sock_udp_t sock;
 *
 *     local.port = 0xabcd;
 *
 *     if (sock_udp_create(&sock, &local, NULL, 0) < 0) {
 *         puts("Error creating UDP sock");
 *         return 1;
 *     }
 *
 *
 *     while (1) {
 *         sock_udp_ep_t remote = { .family = AF_INET6 };
 *         ssize_t res;
 *
 *         remote.port = 12345;
 *         ipv6_addr_set_all_nodes_multicast((ipv6_addr_t *)&remote.addr.ipv6,
 *                                           IPV6_ADDR_MCAST_SCP_LINK_LOCAL);
 *         if (sock_udp_send(&sock, "Hello!", sizeof("Hello!"), &remote) < 0) {
 *             puts("Error sending message");
 *             sock_udp_close(&sock);
 *             return 1;
 *         }
 *         if ((res = sock_udp_recv(&sock, buf, sizeof(buf), 1 * US_PER_SEC,
 *                                 NULL)) < 0) {
 *             if (res == -ETIMEDOUT) {
 *                 puts("Timed out");
 *             }
 *             else {
 *                 puts("Error receiving message");
 *             }
 *         }
 *         else {
 *             printf("Received message: \"");
 *             for (int i = 0; i < res; i++) {
 *                 printf("%c", buf[i]);
 *             }
 *             printf("\"\n");
 *         }
 *         xtimer_sleep(1);
 *     }
 *
 *     return 0;
 * }
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * Again: Don't forget to also @ref including-modules "include" the IPv6 module
 * of your networking implementation (e.g. `gnrc_ipv6_default` for
 * @ref net_gnrc "GNRC") and at least one network device.
 *
 * We first create again a `sock` with a local end point bound to any IPv6
 * address and some port. Note that we also could specify the remote here and
 * not use it with @ref sock_udp_send().
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.c}
 *     sock_udp_ep_t local = SOCK_IPV6_EP_ANY;
 *     sock_udp_t sock;
 *
 *     local.port = 0xabcd;
 *
 *     if (sock_udp_create(&sock, &local, NULL, 0) < 0) {
 *         puts("Error creating UDP sock");
 *         return 1;
 *     }
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * We then create a remote end point with the [link-local all nodes multicast
 * address](https://tools.ietf.org/html/rfc4291#page-16) (`ff02::1`) and port
 * `12345` and send a "Hello!" message to that end point.
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.c}
 *         sock_udp_ep_t remote = { .family = AF_INET6 };
 *         ssize_t res;
 *
 *         remote.port = 12345;
 *         ipv6_addr_set_all_nodes_multicast((ipv6_addr_t *)&remote.addr.ipv6,
 *                                           IPV6_ADDR_MCAST_SCP_LINK_LOCAL);
 *         if (sock_udp_send(&sock, "Hello!", sizeof("Hello!"), &remote) < 0) {
 *             puts("Error sending message");
 *             sock_udp_close(&sock);
 *             return 1;
 *         }
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * We then wait a second for a reply and print it when it is received.
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.c}
 *         if ((res = sock_udp_recv(&sock, buf, sizeof(buf), 1 * US_PER_SEC,
 *                                 NULL)) < 0) {
 *             if (res == -ETIMEDOUT) {
 *                 puts("Timed out");
 *             }
 *             else {
 *                 puts("Error receiving message");
 *             }
 *         }
 *         else {
 *             printf("Received message: \"");
 *             for (int i = 0; i < res; i++) {
 *                 printf("%c", buf[i]);
 *             }
 *             printf("\"\n");
 *         }
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * Finally, we wait a second before sending out the next "Hello!" with
 * `xtimer_sleep(1)`.
 *
 * @{
 *
 * @file
 * @brief   UDP sock definitions
 *
 * @author  Alexander Aring <aar@pengutronix.de>
 * @author  Simon Brummer <simon.brummer@haw-hamburg.de>
 * @author  Cenk Gündoğan <mail@cgundogan.de>
 * @author  Peter Kietzmann <peter.kietzmann@haw-hamburg.de>
 * @author  Martine Lenders <m.lenders@fu-berlin.de>
 * @author  Kaspar Schleiser <kaspar@schleiser.de>
 */
