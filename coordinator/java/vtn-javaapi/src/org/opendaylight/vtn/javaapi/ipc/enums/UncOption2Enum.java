/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.javaapi.ipc.enums;

/**
 * The Enum UncOption2Enum.
 */
public enum UncOption2Enum {

	UNC_OPT2_NONE, /* none */
	UNC_OPT2_L2DOMAIN, /* l2-domain */
	UNC_OPT2_MAC_ENTRY, /* mac-entry */
	UNC_OPT2_MAC_ENTRY_DYNAMIC, /* mac-entry-dynamic */
	UNC_OPT2_MAC_ENTRY_STATIC, /* mac-entry-static */
	UNC_OPT2_NEIGHBOR, /* neighbor */
	UNC_OPT2_ARP_ENTRY, /* arp-entry */
	UNC_OPT2_ARP_ENTRY_DYNAMIC, /* arp-entry-dynamic */
	UNC_OPT2_ARP_ENTRY_STATIC, /* arp-entry-static */
	UNC_OPT2_DHCP_RELAY, /* dhcp-relay */
	UNC_OPT2_IP_ROUTE, /* ip-route */
	UNC_OPT2_PING, /* ping */
	UNC_OPT2_CLEAR_ARPAGENT, /* clear arpagent */
	UNC_OPT2_MATCH_SWITCH1, /* match-switch1 */
	UNC_OPT2_MATCH_SWITCH2, /* match-switch2 */
	UNC_OPT2_MATCH_BOTH_SWITCH, /* match-both-switch */
	UNC_OPT2_SIBLING_ALL, /* return all sibling details */
	UNC_OPT2_NO_TRAVERSING,   /*Returns n flows without travering further*/
	UNC_OPT2_FDBENTRY,        /* switch fdn entries */
	UNC_OPT2_BOUNDARY,         /* to check the boundary presence*/
	UNC_OPT2_EXPAND        /* to read Unified vBridge*/
}
