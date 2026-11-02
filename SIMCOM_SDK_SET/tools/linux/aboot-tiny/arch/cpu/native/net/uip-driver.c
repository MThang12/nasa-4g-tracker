/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

/**
 * \file
 *         A brief description of what this file is
 * \author
 *         Niclas Finne <nfi@sics.se>
 *         Joakim Eriksson <joakime@sics.se>
 */

#include "net/netstack.h"
#include "net/ipv6/uip.h"
#include "net/ipv6/tcpip.h"
#include "dev/ethos.h"
#include "net/uip-driver.h"
#include "net/linkaddr.h"
#include <string.h>
#include <stdio.h>

static char send_buffer[2048];

/*--------------------------------------------------------------------*/
static uint8_t
uip_driver_send(const linkaddr_t *addr)
{
  struct uip_eth_hdr *hdr = (struct uip_eth_hdr *)send_buffer;
  struct uip_ip_hdr *ip_hdr = (struct uip_ip_hdr *)uip_buf;
  uip_ip6addr_t *ipaddr = (uip_ip6addr_t *)&ip_hdr->destipaddr;

  if(!ethos_get_mac(&hdr->dest)) {
    return 0;
  }
  memcpy(&hdr->src, &linkaddr_node_addr, sizeof(hdr->src));
  memcpy(send_buffer + sizeof(*hdr), uip_buf, uip_len);
  if(uip_is_addr_mcast(ipaddr)) {
    hdr->dest.addr[0] = 0x33;
    hdr->dest.addr[1] = 0x33;
    hdr->dest.addr[2] = ipaddr->u8[12];
    hdr->dest.addr[3] = ipaddr->u8[13];
    hdr->dest.addr[4] = ipaddr->u8[14];
    hdr->dest.addr[5] = ipaddr->u8[15];
  }
  hdr->type = uip_htons(UIP_ETHTYPE_IPV6);
  ethos_write_aboot_cmd((const uint8_t *)send_buffer, uip_len + sizeof(*hdr));

  return 1;
}
/*--------------------------------------------------------------------*/
static void
init(void)
{
}
/*--------------------------------------------------------------------*/
static void
input(void)
{
}
/*--------------------------------------------------------------------*/
const struct network_driver uip_driver = {
  "ether_uip",
  init,
  input,
  uip_driver_send
};
/*--------------------------------------------------------------------*/
