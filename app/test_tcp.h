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
 * @defgroup    net_sock_tcp    TCP sock API
 * @ingroup     net_sock
 * @brief       Sock submodule for TCP
 *
 * How To Use
 * ----------
 * First you need to @ref including-modules "include" a module that implements
 * this API in your application's Makefile. For example the implementation for
 * @ref net_gnrc "GNRC" is called `gnrc_sock_tcp`.
 *
 * ### A Simple TCP Echo Server
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.c}
 * #include "net/sock/tcp.h"
 *
 * #define SOCK_QUEUE_LEN  (1U)
 *
 * sock_tcp_t sock_queue[SOCK_QUEUE_LEN];
 * uint8_t buf[128];
 *
 * int main(void)
 * {
 *     sock_tcp_ep_t local = SOCK_IPV6_EP_ANY;
 *     sock_tcp_queue_t queue;
 *
 *     local.port = 12345;
 *
 *     if (sock_tcp_listen(&queue, &local, sock_queue, SOCK_QUEUE_LEN, 0) < 0) {
 *         puts("Error creating listening queue");
 *         return 1;
 *     }
 *     puts("Listening on port 12345");
 *     while (1) {
 *         sock_tcp_t *sock;
 *
 *         if (sock_tcp_accept(&queue, &sock) < 0) {
 *             puts("Error accepting new sock");
 *         }
 *         else {
 *             int read_res = 0;
 *
 *             puts("Reading data");
 *             while (read_res >= 0) {
 *                 read_res = sock_tcp_read(sock, &buf, sizeof(buf),
 *                                          SOCK_NO_TIMEOUT);
 *                 if (read_res < 0) {
 *                     puts("Disconnected");
 *                     break;
 *                 }
 *                 else {
 *                     int write_res;
 *                     printf("Read: \"");
 *                     for (int i = 0; i < read_res; i++) {
 *                         printf("%c", buf[i]);
 *                     }
 *                     puts("\"");
 *                     if ((write_res = sock_tcp_write(sock, &buf,
 *                                                     read_res)) < 0) {
 *                         puts("Errored on write, finished server loop");
 *                         break;
 *                     }
 *                 }
 *             }
 *             sock_tcp_disconnect(sock);
 *         }
 *     }
 *     sock_tcp_stop_listen(queue);
 *     return 0;
 * }
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * Above you see a simple TCP echo server. Don't forget to also
 * @ref including-modules "include" the IPv6 module of your networking
 * implementation (e.g. `gnrc_ipv6_default` for @ref net_gnrc GNRC) and at least
 * one network device.
 *
 *
 * After including header files for the @ref net_af "address families" and
 * the @ref net_sock_tcp "TCP `sock`s and `queue`s" themselves, we create an
 * array of @ref sock_tcp_t "sock" objects `sock_queue` as our listen queue (for
 * simplicity of length 1 in our example) and some buffer space `buf` to store
 * the data received by the server:
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.c}
 * #include "net/af.h"
 * #include "net/sock/tcp.h"
 *
 * #define SOCK_QUEUE_LEN  (1U)
 *
 * sock_tcp_t sock_queue[SOCK_QUEUE_LEN];
 * uint8_t buf[128];
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * We want to listen for incoming connections on a specific port, so we set a
 * local end point with that port (`12345` in this case).
 *
 * We then proceed to creating the listen queue `queue`. Since it is bound to
 * `local` it waits for incoming connections to port `12345`. We don't need any
 * further configuration so we set the flags to 0. In case of an error we stop
 * the program:
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.c}
 *     sock_tcp_ep_t local = SOCK_IPV6_EP_ANY;
 *     sock_tcp_queue_t queue;
 *
 *     local.port = 12345;
 *
 *     if (sock_tcp_listen(&queue, &local, sock_queue, SOCK_QUEUE_LEN, 0) < 0) {
 *         puts("Error creating listening queue");
 *         return 1;
 *     }
 *     puts("Listening on port 12345");
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * The application then waits indefinitely for an incoming connection with
 * `sock_tcp_accept()`. If we want to timeout this wait period we could
 * alternatively set the `timeout` parameter of @ref sock_tcp_accept() to a
 * value != @ref SOCK_NO_TIMEOUT. If an error occurs during that we print an
 * error message but proceed waiting.
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.c}
 *     while (1) {
 *         sock_tcp_t *sock;
 *
 *         if (sock_tcp_accept(&queue, &sock, SOCK_NO_TIMEOUT) < 0) {
 *             puts("Error accepting new sock");
 *         }
 *         else {
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * On successful connection establishment with a client we get a connected
 * `sock` object and we try to read the incoming stream into `buf` using
 * `sock_tcp_read()` on that `sock`. Again, we could use another timeout period
 * than @ref SOCK_NO_TIMEOUT with this function. If we error we break the read
 * loop and disconnect the `sock`.
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.c}
 *             int read_res = 0;
 *
 *             puts("Reading data");
 *             while (read_res >= 0) {
 *                 read_res = sock_tcp_read(sock, &buf, sizeof(buf),
 *                                          SOCK_NO_TIMEOUT);
 *                 if (read_res < 0) {
 *                     puts("Disconnected");
 *                     break;
 *                 }
 *                 else {
 *                     ...
 *                 }
 *             }
 *             sock_tcp_disconnect(sock);
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * Otherwise, we print the received message and write it back to the connected
 * `sock` (an again breaking the loop on error).
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.c}
 *                     int write_res;
 *                     printf("Read: \"");
 *                     for (int i = 0; i < read_res; i++) {
 *                         printf("%c", buf[i]);
 *                     }
 *                     puts("\"");
 *                     if ((write_res = sock_tcp_write(sock, &buf,
 *                                                     read_res)) < 0) {
 *                         puts("Errored on write, finished server loop");
 *                         break;
 *                     }
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * In the case of we somehow manage to break the infinite accepting loop we stop
 * the listening queue appropriately.
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.c}
 *     sock_tcp_stop_listen(queue);
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * ### A Simple TCP Echo Client
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.c}
 * #include "net/af.h"
 * #include "net/ipv6/addr.h"
 * #include "net/sock/tcp.h"
 *
 * uint8_t buf[128];
 * sock_tcp_t sock;
 *
 * int main(void)
 * {
 *     int res;
 *     sock_tcp_ep_t remote = SOCK_IPV6_EP_ANY;
 *
 *     remote.port = 12345;
 *     ipv6_addr_from_str((ipv6_addr_t *)&remote.addr,
 *                        "fe80::d8fa:55ff:fedf:4523");
 *     if (sock_tcp_connect(&sock, &remote, 0, 0) < 0) {
 *         puts("Error connecting sock");
 *         return 1;
 *     }
 *     puts("Sending \"Hello!\"");
 *     if ((res = sock_tcp_write(&sock, "Hello!", sizeof("Hello!"))) < 0) {
 *         puts("Errored on write");
 *     }
 *     else {
 *         if ((res = sock_tcp_read(&sock, &buf, sizeof(buf),
 *                                  SOCK_NO_TIMEOUT)) < 0) {
 *             puts("Disconnected");
 *         }
 *         printf("Read: \"");
 *         for (int i = 0; i < res; i++) {
 *             printf("%c", buf[i]);
 *         }
 *         puts("\"");
 *     }
 *     sock_tcp_disconnect(&sock);
 *     return res;
 * }
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * Above you see a simple TCP echo client. Again: Don't forget to also
 * @ref including-modules "include" the IPv6 module of your networking
 * implementation (e.g. `gnrc_ipv6_default` for @ref net_gnrc "GNRC") and at
 * least one network device. Ad0)ditionally, for the IPv6 address parsing you need
 * the @ref net_ipv6_addr "IPv6 address module".
 *
 * This time instead of creating a listening queue we create a connected `sock`
 * object directly. To connect it to a port at a host we setup a remote
 * end-point first (with port `12345` and address `fe80::d8fa:55ff:fedf:4523` in
 * this case; your IP address may differ of course) and connect to it using
 * `sock_tcp_connect()`. We neither care about the local port nor additional
 * configuration so we set both the `local_port` and `flags` parameter of
 * `sock_tcp_connect()` to `0`:
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.c}
 *     sock_tcp_ep_t remote = SOCK_IPV6_EP_ANY;
 *
 *     remote.port = 12345;
 *     ipv6_addr_from_str((ipv6_addr_t *)&remote.addr,
 *                        "fe80::d8fa:55ff:fedf:4523");
 *     if (sock_tcp_connect(&sock, &remote, 0, 0) < 0) {
 *         puts("Error connecting sock");
 *         return 1;
 *     }
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * On error we just terminate the program, on success we send a message
 * (`Hello!`) and again terminate the program on error:
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.c}
 *     if ((res = sock_tcp_write(&sock, "Hello!", sizeof("Hello!"))) < 0) {
 *         puts("Errored on write");
 *     }
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * Otherwise, we wait for the reply and print it in case of success (and
 * terminate in case of error):
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ {.c}
 *     else {
 *         if ((res = sock_tcp_read(&sock, &buf, sizeof(buf),
 *                                  SOCK_NO_TIMEOUT)) < 0) {
 *             puts("Disconnected");
 *         }
 *         printf("Read: \"");
 *         for (int i = 0; i < res; i++) {
 *             printf("%c", buf[i]);
 *         }
 *         puts("\"");
 *     }
 *     sock_tcp_disconnect(&sock);
 *     return res;
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * @{
 *
 * @file
 * @brief   TCP sock definitions
 *
 * @author  Alexander Aring <aar@pengutronix.de>
 * @author  Simon Brummer <simon.brummer@haw-hamburg.de>
 * @author  Cenk Gündoğan <mail@cgundogan.de>
 * @author  Peter Kietzmann <peter.kietzmann@haw-hamburg.de>
 * @author  Martine Lenders <m.lenders@fu-berlin.de>
 * @author  Kaspar Schleiser <kaspar@schleiser.de>
 */
