#include "net/sock/tcp.h"
#include <lwip/sockets.h>
#include "xtimer.h"
#define SOCK_QUEUE_LEN (1U)

sock_tcp_t sock_queue[SOCK_QUEUE_LEN];
uint8_t buf[2 * 1024];
sock_tcp_t sock;

int test_tcp_server(void) //speed:23.0 Mbits/sec
{
    sock_tcp_ep_t local = SOCK_IPV4_EP_ANY;
    sock_tcp_queue_t queue;

    // int sock = -1;
    // sock = socket(AF_INET, SOCK_STREAM, 0);
    // if(sock < 0){
    //     printf("socket error\r\n");
    // }

    local.port = 12345;

    if (sock_tcp_listen(&queue, &local, sock_queue, SOCK_QUEUE_LEN, 0) < 0)
    {
        puts("Error creating listening queue");
        return 1;
    }
    puts("Listening on port 12345");
    while (1)
    {
        sock_tcp_t *sock;

        if (sock_tcp_accept(&queue, &sock, SOCK_NO_TIMEOUT) < 0)
        {
            puts("Error accepting new sock");
        }
        else
        {
            int read_res = 0;

            puts("Reading data");
            while (read_res >= 0)
            {
                read_res = sock_tcp_read(sock, &buf, sizeof(buf),
                                         SOCK_NO_TIMEOUT);
                if (read_res < 0)
                {
                    puts("Disconnected");
                    break;
                }
                else
                {
                    // int write_res;
                    // printf("Read: \"");
                    // for (int i = 0; i < read_res; i++)
                    // {
                    //     printf("%c", buf[i]);
                    // }
                    // puts("\"");
                    // if ((write_res = sock_tcp_write(sock, &buf,
                    //                                 read_res)) < 0)
                    // {
                    //     puts("Errored on write, finished server loop");
                    //     break;
                    // }
                }
            }
            sock_tcp_disconnect(sock);
        }
    }
    // sock_tcp_stop_listen(queue);
    return 0;
}

int test_tcp_client(void)//16.2855 Mbps
{
    int res;
    sock_tcp_ep_t remote = SOCK_IPV4_EP_ANY;
    remote.port = 12344;

    uint64_t sentlen = 0;
    uint32_t tick1 = 0, tick2 = 0;
    // for (int i = 0; i < 4 * 1024; i++)
    // {
    //     buf[i] = i & 0xff;
    // }
    memset(buf, 97, sizeof(buf));

    ipv4_addr_from_str((ipv4_addr_t *)&remote.addr,
                       "192.168.1.102");
    if (sock_tcp_connect(&sock, &remote, 0, 0) < 0)
    {
        puts("Error connecting sock");
        return 1;
    }
    puts("Sending \"Hello!\"");
    if ((res = sock_tcp_write(&sock, "Hello!", sizeof("Hello!"))) < 0)
    {
        puts("Errored on write");
    }
    else
    {
        tick1 = xtimer_now_usec();
        while (1)
        {
            tick2 = xtimer_now_usec();
            if (tick2 - tick1 >= 2000 * 1000)
            {
                float f = (float)sentlen * 8 * 1000 * 1000 / 1024 / 1024 / (tick2 - tick1);
                printf("send speed = %.4f Mbps!\r\n", f);
                tick1 = tick2;
                sentlen = 0;
            }
            if ((res = sock_tcp_write(&sock, buf, sizeof(buf))) < 0)
            {
                puts("Errored on write");
                break;
            }
            else{
                sentlen += sizeof(buf);
            }
        }
        // if ((res = sock_tcp_read(&sock, &buf, sizeof(buf),
        //                          SOCK_NO_TIMEOUT)) < 0)
        // {
        //     puts("Disconnected");
        // }
        // printf("Read: \"");
        // for (int i = 0; i < res; i++)
        // {
        //     printf("%c", buf[i]);
        // }
        // puts("\"");
    }
    sock_tcp_disconnect(&sock);
    return res;
}