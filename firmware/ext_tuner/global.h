

//MIT License
//
//Copyright (c) 2018 tvelliott
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.


#ifndef global_var_h
#define global_var_h

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define TUNER_CMD_SET_FREQ 0x01
#define TUNER_CMD_GET_FREQ 0x02
#define TUNER_CMD_SET_IF 0x03
#define TUNER_CMD_GET_IF 0x04
#define TUNER_CMD_SET_ANT 0x05
#define TUNER_CMD_GET_ANT 0x06
#define TUNER_CMD_PLL_STATUS 0x07
#define TUNER_CMD_SET_MUTE 0x08
#define TUNER_CMD_GET_LOW_FREQ 0x09
#define TUNER_CMD_GET_HIGH_FREQ 0x0a
#define TUNER_CMD_GET_LOW_IF 0x0b
#define TUNER_CMD_GET_HIGH_IF 0x0c

#define BUFFERSIZE 256
#define ANT_SW GPIO_Pin_3
#define MUTE_N GPIO_Pin_8
#define SYNTH_CS GPIO_Pin_4
#define TUNER_SLAVE_CS GPIO_Pin_7

#define htons(A) ((((uint16_t)(A) & 0xff00) >> 8) | \
                    (((uint16_t)(A) & 0x00ff) << 8))
#define htonl(A) ((((uint32_t)(A) & 0xff000000) >> 24) | \
                    (((uint32_t)(A) & 0x00ff0000) >> 8)  | \
                    (((uint32_t)(A) & 0x0000ff00) << 8)  | \
                    (((uint32_t)(A) & 0x000000ff) << 24))

/*
#define DHCPS_LIGHT_MAX_CLIENTS 32
#define VWR_INT_MTU 1500
#define MAX_BUFSIZE (VWR_INT_MTU+64)

#define LITTLE_ENDIAN 1

#if defined(BIG_ENDIAN) && !defined(LITTLE_ENDIAN)

#define htons(A) (A)
#define htonl(A) (A)
#define ntohs(A) (A)
#define ntohl(A) (A)

#elif defined(LITTLE_ENDIAN) && !defined(BIG_ENDIAN)

#define htons(A) ((((uint16_t)(A) & 0xff00) >> 8) | \
                    (((uint16_t)(A) & 0x00ff) << 8))
#define htonl(A) ((((uint32_t)(A) & 0xff000000) >> 24) | \
                    (((uint32_t)(A) & 0x00ff0000) >> 8)  | \
                    (((uint32_t)(A) & 0x0000ff00) << 8)  | \
                    (((uint32_t)(A) & 0x000000ff) << 24))

#define ntohs  htons
#define ntohl  htonl

#else

#error "Must define one of BIG_ENDIAN or LITTLE_ENDIAN"

#endif

#define TCP_FIN 0x01
#define TCP_SYN 0x02
#define TCP_RST 0x04
#define TCP_PSH 0x08
#define TCP_ACK 0x10
#define TCP_URG 0x20
#define TCP_CTL 0x3f

#define TCP_OPT_END     0
#define TCP_OPT_NOOP    1
#define TCP_OPT_MSS     2
#define TCP_OPT_WIN_SCALE 3
#define TCP_OPT_SACK  4
#define TCP_OPT_TIMESTAMP 8

#define IP_PROTO_ICMP  0x01
#define IP_PROTO_TCP   0x06
#define IP_PROTO_UDP   0x11
#define ETHERNET_MTU 1500
#define TCP_MAX_OPTIONS_LEN 20

#define ICMP_ECHO_REPLY 0
#define ICMP_ECHO_REQUEST 8

//TCP/IP structs
typedef struct __attribute__((packed)) {
        uint8_t   dst_mac[6];
        uint8_t   src_mac[6];
        uint16_t     eth_type;
} eth_hdr;

typedef struct __attribute__((packed)) {
        uint8_t     version;
        uint8_t        tos;
        uint16_t       len;
        uint16_t       id;
        uint16_t       offset;
        uint8_t        ttl;
        uint8_t        proto;
        uint16_t       chksum;
        uint32_t    src_ip;
        uint32_t    dst_ip;
} ip_hdr;

typedef struct __attribute__((packed)) {
        uint16_t  src_port;
        uint16_t  dst_port;
        uint32_t     tcp_seq;
        uint32_t     tcp_ack;
        uint8_t      hdr_len;
        uint8_t      tcp_flags;
        uint16_t     win_size;
        uint16_t     chksum;
        uint16_t     urgent;
        uint8_t      payload[0];
} tcp_hdr;

typedef struct __attribute__((packed)) {
        uint16_t     src_port;
        uint16_t     dst_port;
        uint16_t     len;
        uint16_t     chksum;
        uint8_t      payload[0];
} udp_hdr;
typedef struct __attribute__((packed)) {
    uint8_t type;
    uint8_t code;
    uint16_t     chksum;
    uint8_t      payload[0];
} icmp_hdr;

typedef struct __attribute__((packed)) {
        uint16_t hardware_type;
        uint16_t protocol_type;
        uint8_t  hardware_size;
        uint8_t  protocol_size;
        uint16_t opcode;
        uint8_t  sender_mac[6];
        uint32_t sender_ip;
        uint8_t  target_mac[6];
        uint32_t target_ip;
} arp_hdr;

typedef struct __attribute__((packed)) {
        eth_hdr ethhdr;
        arp_hdr arphdr;

} eth_arp_frame;


typedef struct __attribute__((packed)) {
        eth_hdr ethhdr;
        ip_hdr iphdr;

        union __attribute__((packed)) {
                tcp_hdr tcphdr;
                udp_hdr udphdr;
                icmp_hdr icmphdr;
        } ip_proto_payload;

} eth_frame;




//dhcp defines
#define BOOTP_BROADCAST 0x8000

#define DHCP_REQUEST        1
#define DHCP_REPLY          2
#define DHCP_HTYPE_ETHERNET 1
#define DHCP_HLEN_ETHERNET  6
#define DHCP_MSG_LEN      236

#define DHCPS_SERVER_PORT  67
#define DHCPS_CLIENT_PORT  68

#define DHCPDISCOVER  1
#define DHCPOFFER     2
#define DHCPREQUEST   3
#define DHCPDECLINE   4
#define DHCPACK       5
#define DHCPNAK       6
#define DHCPRELEASE   7

#define DHCP_OPTION_SUBNET_MASK   1
#define DHCP_OPTION_ROUTER        3
#define DHCP_OPTION_DNS_SERVER    6
#define DHCP_OPTION_REQ_IPADDR   50
#define DHCP_OPTION_LEASE_TIME   51
#define DHCP_OPTION_MSG_TYPE     53
#define DHCP_OPTION_SERVER_ID    54
#define DHCP_OPTION_INTERFACE_MTU 26
#define DHCP_OPTION_PERFORM_ROUTER_DISCOVERY 31
#define DHCP_OPTION_BROADCAST_ADDRESS 28
#define DHCP_OPTION_REQ_LIST     55
#define DHCP_OPTION_END         255

#define DHCP_MAGIC 0x63825363

typedef struct __attribute__((packed)) {
        uint8_t  op;
        uint8_t  htype;
        uint8_t  hlen;
        uint8_t  hops;
        uint32_t  xid;
        uint16_t  secs;
        uint16_t  flags;
        uint32_t  ciaddr;
        uint32_t  yiaddr;
        uint32_t  siaddr;
        uint32_t  giaddr;
        uint32_t  chaddr1;
        uint32_t  chaddr2;
        uint32_t  chaddr3;
        uint32_t  chaddr4;
        uint8_t   bootp_legacy[192];
        uint32_t  magic_cookie;
        uint8_t   options[0];
} dhcp_record;

*/

typedef struct __attribute__((packed)) {
  uint32_t intf_eth_dst_ip;
  uint32_t intf_eth_src_ip;
  uint8_t intf_eth_dst_mac48[6];
  uint8_t intf_eth_src_mac48[6];
} tcl_intf_eth_hdr;

enum {
  ETH_INTF,
  ACM_INTF
};
extern tcl_intf_eth_hdr tcl_intf_eth;
void __attribute__((naked)) DelayClk3(unsigned long ulCount);
void Delay(volatile uint32_t nTime);
void LwIP_Init(void);
void LwIP_Pkt_Handle(volatile uint8_t *buffer, int16_t len);
void LwIP_Periodic_Handle(long time);

#endif
