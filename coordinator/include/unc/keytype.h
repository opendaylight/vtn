/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_UNC_KEYTYPE_H
#define	_UNC_KEYTYPE_H

#include <unc/base.h>

UNC_C_BEGIN_DECL

/* Key type represents the kind of resource. */
typedef enum {
	UNC_KT_ROOT = 0,
	/* UPLL: 0x001~0x1ff */
        UNC_KT_UNIFIED_NETWORK = 0x001,
        UNC_KT_UNW_LABEL,
        UNC_KT_UNW_LABEL_RANGE,
        UNC_KT_UNW_SPINE_DOMAIN,
	UNC_KT_FLOWLIST, // = 0x001,
	UNC_KT_FLOWLIST_ENTRY,
	UNC_KT_POLICING_PROFILE,
	UNC_KT_POLICING_PROFILE_ENTRY,
	UNC_KT_PATHMAP_ENTRY,
	UNC_KT_PATHMAP_CONTROLLER,
	UNC_KT_VTNSTATION_CONTROLLER,
	UNC_KT_VTN,
        UNC_KT_VTN_UNIFIED,
	UNC_KT_VTN_MAPPING_CONTROLLER,
	UNC_KT_VTN_DATAFLOW_CONTROLLER,
	UNC_KT_VBRIDGE,
	UNC_KT_VBR_PORTMAP,
        UNC_KT_VBR_VLANMAP,
	UNC_KT_VBR_NWMONITOR,
	UNC_KT_VBR_NWMONITOR_HOST,
	UNC_KT_VBR_POLICINGMAP,
	UNC_KT_VBR_POLICINGMAP_ENTRY,
	UNC_KT_VBR_FLOWFILTER,
	UNC_KT_VBR_FLOWFILTER_ENTRY,
	UNC_KT_VBR_IF,
	UNC_KT_IF_MACENTRY,
	UNC_KT_VBRIF_POLICINGMAP,
	UNC_KT_VBRIF_POLICINGMAP_ENTRY,
	UNC_KT_VBRIF_FLOWFILTER,
	UNC_KT_VBRIF_FLOWFILTER_ENTRY,
	UNC_KT_VROUTER,
	UNC_KT_VRT_IF,
	UNC_KT_VRTIF_FLOWFILTER,
	UNC_KT_VRTIF_FLOWFILTER_ENTRY,
	UNC_KT_VRT_IPROUTE,
	UNC_KT_DHCPRELAY_SERVER,
	UNC_KT_DHCPRELAY_IF,
	UNC_KT_IF_ARPENTRY,
    UNC_KT_VTERMINAL,
    UNC_KT_VTERM_IF,
    UNC_KT_VTERMIF_POLICINGMAP,
    UNC_KT_VTERMIF_POLICINGMAP_ENTRY,
    UNC_KT_VTERMIF_FLOWFILTER,
    UNC_KT_VTERMIF_FLOWFILTER_ENTRY,
	UNC_KT_VUNKNOWN,
	UNC_KT_VUNK_IF,
	UNC_KT_VTEP,
	UNC_KT_VTEP_IF,
	UNC_KT_VTEP_GRP,
	UNC_KT_VTEP_GRP_MEMBER,
	UNC_KT_VTUNNEL,
	UNC_KT_VTUNNEL_IF,
	UNC_KT_VLINK,
	UNC_KT_VTN_POLICINGMAP,
	UNC_KT_VTN_POLICINGMAP_CONTROLLER,
	UNC_KT_VTN_FLOWFILTER,
	UNC_KT_VTN_FLOWFILTER_ENTRY,
	UNC_KT_VTN_FLOWFILTER_CONTROLLER,
	UNC_KT_VTN_PATHMAP_ENTRY,
	UNC_KT_VTN_PATHMAP_CONTROLLER,
	UNC_KT_VTN_DATAFLOW,
  UNC_KT_VTN_CONTROLLER,
  UNC_KT_VTN_FLOODING_PATH,
	/* UPPL: 0x200~0x3ff */
	UNC_KT_CONTROLLER = 0x200,
	UNC_KT_SWITCH,
	UNC_KT_PORT,
	UNC_KT_PORT_NEIGHBOR,
	UNC_KT_LINK,
	UNC_KT_CTR_DOMAIN,
	UNC_KT_LOGICAL_PORT,
	UNC_KT_LOGICAL_MEMBER_PORT,
	UNC_KT_BOUNDARY,
	UNC_KT_DATAFLOW,
	UNC_KT_CTR_DATAFLOW,
	UNC_KT_DATAFLOW_V2
} unc_key_type_t;

/* Operation */
typedef enum {
	UNC_OP_INVALID = 0,
	UNC_OP_CREATE,
	UNC_OP_DELETE,
	UNC_OP_UPDATE,
	UNC_OP_CONTROL,
	UNC_OP_RENAME,
	UNC_OP_READ,
	UNC_OP_READ_NEXT,
	UNC_OP_READ_BULK,
	UNC_OP_READ_SIBLING,
	UNC_OP_READ_SIBLING_BEGIN,
	UNC_OP_READ_SIBLING_COUNT
} unc_keytype_operation_t;

/*
 * option1
 * Value dependent on Operation.
 */
typedef enum {
	UNC_OPT1_NORMAL = 0,	/* normal */
	UNC_OPT1_DETAIL,	    /* detail */
	UNC_OPT1_COUNT,       /* count */
  UNC_OPT1_AUDIT		    /* read_bulk for audit */
} unc_keytype_option1_t;

/*
 * option2
 * Value dependent on key type.
 */
typedef enum {
	UNC_OPT2_NONE = 0,		/* none */
	UNC_OPT2_L2DOMAIN,		/* l2-domain */
	UNC_OPT2_MAC_ENTRY,		/* mac-entry */
	UNC_OPT2_MAC_ENTRY_DYNAMIC,	/* mac-entry-dynamic */
	UNC_OPT2_MAC_ENTRY_STATIC,	/* mac-entry-static */
	UNC_OPT2_NEIGHBOR,		/* neighbor */
	UNC_OPT2_ARP_ENTRY,		/* arp-entry */
	UNC_OPT2_ARP_ENTRY_DYNAMIC,	/* arp-entry-dynamic */
	UNC_OPT2_ARP_ENTRY_STATIC,	/* arp-entry-static */
	UNC_OPT2_DHCP_RELAY,		/* dhcp-relay */
	UNC_OPT2_IP_ROUTE,		/* ip-route */
	UNC_OPT2_PING,			/* ping */
	UNC_OPT2_CLEAR_ARPAGENT,	/* clear arpagent */
	UNC_OPT2_MATCH_SWITCH1,		/* match-switch1 */
	UNC_OPT2_MATCH_SWITCH2,		/* match-switch2 */
	UNC_OPT2_MATCH_BOTH_SWITCH,	/* match-both-switch */
	UNC_OPT2_SIBLING_ALL,	    /* return all sibling details */
        UNC_OPT2_NO_TRAVERSING,   /*Returns n flows without travering further*/
        UNC_OPT2_FDBENTRY,        /* switch fdn entries */
        UNC_OPT2_BOUNDARY,         /* to check the boundary presence*/
        UNC_OPT2_EXPAND        /* to read Unified vBridge*/
} unc_keytype_option2_t;

/* Data type indicates the data storage. */
typedef enum {
	UNC_DT_INVALID = 0,
	UNC_DT_STATE,		/* Entity database (State and Statistics) */
	UNC_DT_CANDIDATE,	/* Candidate configuration */
	UNC_DT_RUNNING,		/* Running configuration */
	UNC_DT_STARTUP,		/* Startup configuration */
	UNC_DT_IMPORT,		/* Import configuration */
	UNC_DT_AUDIT
} unc_keytype_datatype_t;

/* Enumeration for Configuration Status. */
typedef enum {
	UNC_CS_UNKNOWN= 0,
	UNC_CS_APPLIED,
	UNC_CS_PARTIALLY_APPLIED,
	UNC_CS_NOT_APPLIED,
	UNC_CS_INVALID,
	UNC_CS_NOT_SUPPORTED
} unc_keytype_configstatus_t;

/* Enumeration for Valid flag. */
typedef enum {
	UNC_VF_INVALID= 0,
	UNC_VF_VALID,
	UNC_VF_VALID_NO_VALUE,
	UNC_VF_NOT_SUPPORTED,
    UNC_VF_VALUE_NOT_MODIFIED
} unc_keytype_validflag_t;

/* Controller type enum. */
typedef enum {
  UNC_CT_UNKNOWN = 0,
  UNC_CT_PFC,
  UNC_CT_VNP,
  UNC_CT_POLC,
  UNC_CT_VAN,
  UNC_CT_ODC
} unc_keytype_ctrtype_t;

/* Audit types */
typedef enum {
  UNC_AT_INVALID = 0,
  UNC_AT_NORMAL,
  UNC_AT_SIMPLIFIED,
  UNC_AT_REALNETWORK
} unc_keytype_audittype_t;

UNC_C_END_DECL

#endif /* _UNC_KEYTYPE_H */
