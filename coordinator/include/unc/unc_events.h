/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * @brief   UNC Driver Event definitoins (common definition)
 * @file    unc_events.h 
 * @author  HCL
 * @date    Feb-2013
 * @version SYSTEM:UNC 1.0, MODULE:Driver
 *
 */

#ifndef _UNC_EVENTS_H_
#define _UNC_EVENTS_H_

/*
 * @brief      Event masks to be registered by platform layer
 */
typedef enum {
  UNC_PHYSICAL_EVENTS = 0, /* Physical Topology Events */
  UNC_ALARMS,              /* Alarm Events */
  UNC_UPPL_ALARMS,         /* Physical Alarm Events */
  UNC_UPLL_ALARMS,         /* Logical Alarm Events */
  UNC_CTLR_STATE_EVENTS    /* Controller State Events */
} unc_event_mask_t;

/*
 * @brief      Specifies the controller states
 */
typedef enum {
  CONTROLLER_OPER_DOWN = 0,
  CONTROLLER_OPER_UP,
  CONTROLLER_EVENTS_START,
  CONTROLLER_EVENTS_DONE
} unc_state_event_type_t;

/*
 * @brief      Physical Topology Events
 */
/*
typedef enum {
  UNC_OFS_ADD = 0,
  UNC_OFS_MOD,
  UNC_OFS_DEL,
  UNC_PORT_ADD,
  UNC_PORT_MOD,
  UNC_PORT_DEL,
  UNC_LINK_ADD,
  UNC_LINK_MOD,
  UNC_LINK_DEL,
  UNC_OFS_ADD_AUX,
  UNC_OFS_MOD_AUX,
  UNC_OFS_DEL_AUX,
  UNC_PORT_ADD_AUX,
  UNC_PORT_MOD_AUX,
  UNC_PORT_DEL_AUX,
  UNC_LINK_ADD_AUX,
  UNC_LINK_MOD_AUX,
  UNC_LINK_DEL_AUX,
} unc_physical_type_t;
*/

/*
 * @brief       Alarm type enumeration body
 */
typedef enum {
  UNC_PHYS_PATH_FAULT = 0, /*Physical Path Fault/Recovery*/
  UNC_COREDOMAIN_SPLIT,      /*Broadcast Domain Split/Recovery*/
  UNC_DOMAIN_SPLIT,      /* Domain Split/Recovery*/
  UNC_SUBDOMAIN_SPLIT,      /*Broadcast Domain Split/Recovery*/
  UNC_FLOW_ENT_FULL,       /*FlowEntry Full Occurred/Recovery*/
  UNC_DEFAULT_FLOW,        /*Default Flow Failure/Success*/
  UNC_POLICER_FULL,        /*Policer resource full Occurred/Recovery*/
  UNC_POLICER_FAIL,        /*Failure in a setup of Policer/Failed Policer was deleted*/
  UNC_NWMON_FAULT,         /*Network Monitor Group Fault/Recovery(Cancel)*/
  UNC_PORT_DIRECTION,      /*Port direction is inconsistent/consistent*/
  UNC_PORT_CONGES,         /*OFS Port Congestion Occurred/Recovered*/
  UNC_OFS_LACK_FEATURES,    /*OFS is lack of features/OFS has recovered lack of features*/
  UNC_OFS_DISABLED,    /*OFS is disabled/enabled */
  UNC_VTN_ID_EXHAUSTION /*VTN ID Exhaustion*/
}unc_alarm_type_t;

/*
 * @brief       Domain events
 */
typedef enum
{
  UNC_DOMAIN_ADD = 0,
  UNC_SUBDOMAIN_ADD,
  UNC_DOMAIN_DEL,
  UNC_SUBDOMAIN_DEL
}unc_domain_types_t;

/*
 * @brief       Operation status
 */
typedef enum
{
  UNC_OPERSTATUS_CRITERIA_ANY = 0,
  UNC_OPERSTATUS_CRITERIA_ALL
}unc_operstatus_criteria;

#endif
