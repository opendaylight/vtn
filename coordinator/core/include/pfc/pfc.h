/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _PFC_PFC_H
#define _PFC_PFC_H

#include <stdint.h>
#include <netinet/ether.h> /*struct ether_addr;*/
#include <pfc/base.h>


/*
 * Common definition for OpenFlow
 */

/* Config default value */
#define PFC_NUM_OFS   5000  /* ofs.num_ofs */
#define PFC_NUM_PORT    32  /* ofs.num_port */

/* Datapath ID type */
typedef uint64_t pfc_dpid_t;

/* Physical port type */
typedef uint32_t pfc_phy_port_t;


/*
 * Definition for TDB objects
 */

/* OFS type */
typedef enum {
	PFC_OFS_TYPE_REAL    = 1 << 0,
	PFC_OFS_TYPE_VIRTUAL = 1 << 1,
	PFC_OFS_TYPE_ALL     = PFC_OFS_TYPE_REAL | PFC_OFS_TYPE_VIRTUAL
} pfc_ofs_type_t;

/* OFS ID */
typedef struct {
	pfc_dpid_t dpid;
	pfc_ofs_type_t type;
} pfc_ofs_id_t;

/* Link ID */
typedef struct {
	pfc_ofs_id_t s_ofs;
	pfc_ofs_id_t d_ofs;
	pfc_phy_port_t s_port;
	pfc_phy_port_t d_port;
} pfc_link_id_t;


/*
 * Link state
 */
/*
 * State change model in cluster ACT side
 *
 *                      start
 *                        | LLDP recv.
 *                        | [ADD_LINK]
 *                        v
 *                  +-----------+
 *  +-------------> |  LINK_UP  |
 *  |               +-----------+
 *  | LLDP recv.,         | LLDP timeout, port down / del,
 *  | LOC Recovery        | vlanset disable, LOC
 *  |                     | [UPDATE_LINK]
 *  |                     v
 *  |               +-----------+
 *  +-------------- | LINK_DOWN |
 *                  +-----------+
 *                        | port down / del, vlanset disable
 *                        | * this trans is immediately occurred
 *                        | [DELETE_LINK]
 *                        v
 *                       end
 */
typedef enum {
	PFC_LINK_STATE_CHECK = 1,	 /* LSB */
	PFC_LINK_INACTIVE    = (1 << 1), /* Link status is inactive */
	PFC_LINK_DISCOVERY   = (PFC_LINK_INACTIVE | PFC_LINK_STATE_CHECK),
				/* Link discovery */
	PFC_LINK_DOWN        = (2 << 1), /* Link status is down (External) */
	PFC_LINK_RECOVERY    = (PFC_LINK_DOWN | PFC_LINK_STATE_CHECK),
				/* Link recovery */
	PFC_LINK_UP          = (3 << 1), /* Link status is up (Internal) */
	PFC_LINK_KEEPALIVE   = (PFC_LINK_UP | PFC_LINK_STATE_CHECK),
				/* Link keep-alive */
} pfc_link_state_t;
/* MEMO:
 *     Some states are used only in TM internally at present.
 *     (At very early version, they appeared on TDB)
 */ 


/*
 * OFS state
 */
/*
 * State change model in cluster ACT side
 *
 *              (*)
 *               |  Feature reply recv.
 *   [OFP V1.0]  |
 *  +------------+
 *  |            |  [OFP V1.3]
 *  |            v
 *  |   +------------------+  Multipart(PORT_DESC) reply error or timeout
 *  |   |MP_PORT_REPLY_WAIT|---->(*)
 *  |   +------------------+
 *  |Stats reply |  Multipart(PORT_DESC) reply recv.(more flag = 0)
 *  | recv.      |
 *  |            v
 *  |   +----------------+  Stats / Multipart(DESC) reply error or timeout
 *  +-->|STATS_REPLY_WAIT|---->(*)
 *      +----------------+
 *               |  Stats / Multipart(DESC) reply recv.
 *               v
 *         +-----------+
 *         | CONNECTED |
 *         +-----------+
 *       secchan |
 *       closed  |
 *               |
 *               v
 *        +------------+  Feature reply recv.
 *        |DISCONNECTED|---->(*)
 *        +------------+
 */
typedef enum {
	PFC_OFS_STATS_REPLY_WAIT = 0,	/* OFS status is Stats reply wait */
					/*  [OFP V1.0]*/
					/* OFS status is Multipart(DESC) */
					/*  reply wait [OFP V1.3]*/
					/*  (TM OFS internal state) */
	PFC_OFS_CONNECTED        = 1,	/* OFS status is connected */
	PFC_OFS_DISCONNECTED     = 2,	/* OFS status is disconnected */
	PFC_OFS_MP_PORT_REPLY_WAIT = 3,	/* OFS status is Multipart(PORT_DESC) */
					/*  reply wait [OFP V1.3] */
					/*  (TM OFS internal state) */
} pfc_ofs_state_t;

/*
 * OFS sub-state
 */
/*
 * OFS's ports information state
 */
typedef enum {
	PFC_OFS_SS_PORT_UNKNOWN     = 0,	/* Ports info is unknown */
	PFC_OFS_SS_PORT_UNAVAILABLE = 1,	/* Ports info is unavailable */
	PFC_OFS_SS_PORT_READY       = 2,	/* Ports info is ready */
} pfc_ofs_ss_port_t;

/*
 * OFS ofp_capabilities
 */
enum pfc_capabilities {
	PFC_OFPC_FLOW_STATS   = 1 << 0,	/* Flow statistics. */
	PFC_OFPC_TABLE_STATS  = 1 << 1,	/* Table statistics. */
	PFC_OFPC_PORT_STATS   = 1 << 2,	/* Port statistics. */
	PFC_OFPC_GROUP_STATS  = 1 << 3,	/* Group statistics. */
	PFC_OFPC_STP          = 1 << 4,	/* 802.1d spanning tree. */
	PFC_OFPC_IP_REASM     = 1 << 5,	/* Can reassemble IP fragments. */
	PFC_OFPC_QUEUE_STATS  = 1 << 6,	/* Queue statistics. */
	PFC_OFPC_ARP_MATCH_IP = 1 << 7,	/* Match IP addresses in ARP pkts. */
	PFC_OFPC_PORT_BLOCKED = 1 << 8,	/* Switch will block looping ports. */
};

/* OFS ofp_actions */
#define PFC_OFPAT_INVALID	0xffffffff	/* ofp_action_type invalid */


/*
 * Port status
 */
/*
 * State change model in cluster ACT side
 *
 *                             start    end
 *                               |       ^
 *                    [ADD_PORT] |       | [DEL_PORT]
 *                               V       |
 *                           +---------------+
 *      +------------------> |   PORT_DOWN   | <-------------------+
 *      |                    +---------------+                     |
 *   [UPDATE_PORT]               |       ^                [UPDATE_PORT]
 *      |                        |       |                         |
 *      |               [UPDATE_PORT] [UPDATE_PORT]                |
 *      |                        |       |                         |
 *      |        set static      |       |      set static         |
 *      |        intport /       v       |      extport /          |
 *      |        LLDP recv.  +---------------+  LLDP timeout.      |
 *      |       +----------- |    PORT_UP    | ------------+       |
 *      |       |            +---------------+             |       |
 *      |   [UPDATE_PORT]                          [UPDATE_PORT]   |
 *      |       |                                          |       |
 *      |       v           set static intport /           v       |
 *   +---------------+        LLDP recv.  *1          +---------------+
 *   |  PORT_UP_INT. | <----------------------------- |  PORT_UP_EXT. |
 *   +---------------+          [UPDATE_PORT]         +---------------+
 * 
 *   *1: this trans is one way
 */
typedef enum {
	PFC_PORT_DOWN        = 1,	/* Port is down */
	PFC_PORT_UP          = 2,	/* Port is up */
	PFC_PORT_UP_INTERNAL = 3,	/* Port is up(Internal) */
	PFC_PORT_UP_EXTERNAL = 4,	/* Port is up(External) */
} pfc_port_state_t;

/* Port port_no */
#define PFC_OFPP_LOCAL	0xfffffffe	/* Local openflow "port". */

/*
 * Port ofp_curr, ofp_advertised, ofp_supported, ofp_peer
 *   (enum ofp_port_features)
 */
enum pfc_port_features {
	PFC_OFPPF_10MB_HD    = 1 << 0,	/* 10 Mb half-duplex rate support. */
	PFC_OFPPF_10MB_FD    = 1 << 1,	/* 10 Mb full-duplex rate support. */
	PFC_OFPPF_100MB_HD   = 1 << 2,	/* 100 Mb half-duplex rate support. */
	PFC_OFPPF_100MB_FD   = 1 << 3,	/* 100 Mb full-duplex rate support. */
	PFC_OFPPF_1GB_HD     = 1 << 4,	/* 1 Gb half-duplex rate support. */
	PFC_OFPPF_1GB_FD     = 1 << 5,	/* 1 Gb full-duplex rate support. */
	PFC_OFPPF_10GB_FD    = 1 << 6,	/* 10 Gb full-duplex rate support. */
	PFC_OFPPF_40GB_FD    = 1 << 7,	/* 40 Gb full-duplex rate support. */
	PFC_OFPPF_100GB_FD   = 1 << 8,	/* 100 Gb full-duplex rate support. */
	PFC_OFPPF_1TB_FD     = 1 << 9,	/* 1 Tb full-duplex rate support. */
	PFC_OFPPF_OTHER      = 1 << 10,	/* Other rate, not in the list. */
	PFC_OFPPF_COPPER     = 1 << 11,	/* Copper medium. */
	PFC_OFPPF_FIBER      = 1 << 12,	/* Fiber medium. */
	PFC_OFPPF_AUTONEG    = 1 << 13,	/* Auto-negotiation. */
	PFC_OFPPF_PAUSE      = 1 << 14,	/* Pause. */
	PFC_OFPPF_PAUSE_ASYM = 1 << 15,	/* Asymmetric pause. */
};

/*
 * Port ofp_config
 */
enum pfc_port_config {
	PFC_OFPPC_PORT_DOWN   = 1 << 0,	/* Port is administratively down. */
	PFC_OFPPC_NO_STP      = 1 << 1,	/* Disable 802.1D spanning tree on */
					/* port. */
	PFC_OFPPC_NO_RECV     = 1 << 2,	/* Drop all packets received by port. */
	PFC_OFPPC_NO_RECV_STP = 1 << 3,	/* Drop received 802.1D STP packets. */
	PFC_OFPPC_NO_FLOOD    = 1 << 4,	/* Do not include this port when */
					/* flooding. */
	PFC_OFPPC_NO_FWD      = 1 << 5,	/* Drop packets forwarded to port. */
	PFC_OFPPC_NO_PACKET_IN= 1 << 6,	/* Do not send packet-in msgs for */
					/* port. */
};

/*
 * Port ofp_state
 */
enum pfc_port_ofp_state {
	PFC_OFPPS_LINK_DOWN = 1 << 0,	/* No physical link present. */
	PFC_OFPPS_BLOCKED   = 1 << 1,	/* Port is blocked */
	PFC_OFPPS_LIVE      = 1 << 2,	/* Live for Fast Failover Group. */
};

/* OFS Port */
typedef struct {
	pfc_dpid_t	dpid;
	pfc_phy_port_t	port_no;
} pfc_ofs_port_t;


PFC_C_BEGIN_DECL

extern const char *pfc_dpid2s_r(pfc_dpid_t dpid, char *buf);
extern const char *pfc_dpid2s(pfc_dpid_t dpid);
extern const char *pfc_dpid2str_r(pfc_dpid_t dpid, char *buf);
extern const char *pfc_dpid2str(pfc_dpid_t dpid);
extern const char *pfc_mac2s_r(const void *addr, char *buf);
extern const char *pfc_mac2s(const void *addr);
extern const char *pfc_ether_ntoa_r(const struct ether_addr *addr, char *buf);
extern const char *pfc_ether_ntoa(const struct ether_addr *addr);

PFC_C_END_DECL

#endif /* #ifndef _PFC_PFC_H */
