/*
 * Copyright (c) 2010, 2011, 2013, 2015 Nicira, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef NETDEV_VPORT_PRIVATE_H
#define NETDEV_VPORT_PRAVITE_H 1

#include <stdbool.h>
#include <stddef.h>
#include "compiler.h"
#include "netdev.h"
#include "netdev-provider.h"
#include "ovs-atomic.h"
#include "ovs-thread.h"

struct netdev_vport {
    struct netdev up;

    OVSRCU_TYPE(const struct netdev_tunnel_config *) tnl_cfg;

    /* Sequence number for outgoing GRE packets. */
    atomic_count gre_seqno;

    /* DEBUG (bsc#1283902): canaries around 'mutex' to detect overwrites. */
    uint64_t dbg_canary_pre;

    /* Protects all members below. */
    struct ovs_mutex mutex;

    uint64_t dbg_canary_post;

    /* DEBUG (bsc#1283902): OVS-level owner of 'mutex' (Linux TID, 0 if
     * unlocked) and a ring of the most recent lock/unlock events. */
    atomic_int dbg_owner_tid;
    const char *dbg_owner_where;
    atomic_count dbg_idx;
    struct vport_dbg_event {
        unsigned int seq;       /* Value of 'dbg_idx' for this event. */
        int op;                 /* enum vport_dbg_op. */
        int tid;                /* Linux TID. */
        unsigned int lock_word; /* glibc __lock at time of event. */
        int glibc_owner;        /* glibc __owner at time of event. */
        const char *where;
    } dbg_ring[64];

    struct eth_addr etheraddr;
    struct netdev_stats stats;

    /* Tunnels. */
    char egress_iface[IFNAMSIZ];
    bool carrier_status;

    /* Patch Ports. */
    char *peer;
};

int netdev_vport_construct(struct netdev *);

static bool
is_vport_class(const struct netdev_class *class)
{
    return class->construct == netdev_vport_construct;
}

static inline struct netdev_vport *
netdev_vport_cast(const struct netdev *netdev)
{
    ovs_assert(is_vport_class(netdev_get_class(netdev)));
    return CONTAINER_OF(netdev, struct netdev_vport, up);
}

#endif
