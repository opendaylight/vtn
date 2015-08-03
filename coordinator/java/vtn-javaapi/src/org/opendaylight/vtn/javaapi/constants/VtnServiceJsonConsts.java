/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.javaapi.constants;

import java.math.BigInteger;

/**
 * The Interface VtnServiceJsonConsts.
 */
public final class VtnServiceJsonConsts {
	public static final String SLASH = "/";
	public static final String URI = "";
	public static final String ENABLE = "enable";
	public static final String DISABLE = "disable";

	public static final String TYPEIPVERSION = "typeip_version";
	public static final String IP = "ip";
	public static final String IPV6 = "ipv6";
	public static final String TARGETDB = "targetdb";
	public static final String STARTUP = "startup";
	public static final String CANDIDATE = "candidate";
	public static final String RUNNING = "running";
	public static final String STATE = "state";
	public static final String OP = "op";
	public static final String NONE = "none";
	// API
	public static final String SHOW = "show"; // webui specific, not related to
	// VTN API
	public static final String LIST = "list";
	public static final String DETAIL = "detail";
	public static final String SHOWDETAIL = "showdetail";
	public static final String COUNT = "count";
	public static final String INDEX = "index";
	public static final String MAX = "max_repetition";

	public static final String CONFIGURATION = "configuration";
	public static final String OPERATION = "operation";
	public static final String TARGETPL = "targetpl";
	public static final String RANGE = "range";
	public static final String CONFIGMODE = "configmode";
	public static final String CONFIGID = "config_id";
	public static final String FORCE = "force";
	public static final String REAL_NETWORK_AUDIT = "real-network_audit";
	public static final String READLOCK = "readlock";
	public static final String READLOCKID = "readlock_id";
	public static final String COMMIT = "commit";
	public static final String ABORT = "abort";
	public static final String SAVE = "save";
	public static final String AUTOSAVE = "autosave";
	public static final String AUTOSAVESTATUS = "auto_save_status";

	public static final String SESSIONS = "sessions";
	public static final String SESSION = "session";
	public static final String SESSIONID = "session_id";
	public static final String USERS = "users";
	public static final String USERNAME = "username";
	public static final String PASSWORD = "password";
	public static final String ADMIN = "admin";
	public static final String OPER = "oper";
	public static final String WEBAPI = "webapi";
	public static final String WEBUI = "webui";

	// String IPADDRESS = "ipaddress";
	public static final String IPADDRESS = "ipaddress";
	public static final String LOGINNAME = "login_name";
	public static final String MODE = "mode";
	public static final String MODESTATUS = "mode_status";

	// Physical
	public static final String CONTROLLERS = "controllers";
	public static final String CONTROLLERID = "controller_id";
	public static final String CONTROLLERNAME = "controller_name";
	public static final String CONTROLLERNAME1 = "controller_name1";
	public static final String CONTROLLERNAME2 = "controller_name2";
	public static final String LOGICALPORTID1 = "logical_port_id1";
	public static final String LOGICALPORTID2 = "logical_port_id2";
	public static final String DOMAINNAME1 = "domain_name1";
	public static final String DOMAINNAME2 = "domain_name2";
	public static final String DOMAINS = "domains";
	public static final String DOMAINID = "domain_id";

	public static final String SWITCHES = "switches";
	public static final String SWITCHEID = "switch_id";

	public static final String PORTS = "ports";
	public static final String PORTNAME = "port_name";

	public static final String LINKS = "links";
	public static final String LINKNAME = "link_name";

	public static final String TRUNKPORTS = "trunkports";
	public static final String TRUNKPORTNAME = "trunk_port_name";
	public static final String BOUNDARIES = "boundaries";
	public static final String BOUNDARYID = "boundary_id";
	public static final String L4PORT = "l4port";

	// Logical
	public static final String VTNS = "vtns";
	public static final String VTNNAME = "vtn_name";
	public static final String VBRIDGES = "vbridges";
	public static final String VBRIDGE_NAME = "vbridge_name";
	public static final String VBRIDGENAME = "vbr_name";
	public static final String VINTERFACES = "interfaces";
	public static final String VIFNAME = "if_name";
	public static final String VNODEIFNAME = "vnode_if_name";

	public static final String VROUTERS = "vrouters";
	public static final String VROUTERNAME = "vrt_name";
	public static final String VLINKS = "vlinks";
	public static final String VLINKNAME = "vlk_name";
	public static final String VLINK_NAME = "vlink_name";
	
	public static final String UNIFIED_NW = "unified_network";
	public static final String UNIFIED_NW_NAME = "unified_network_name";
	public static final String UNIFIED_NWS = "unified_networks";
	public static final String ROUTING_TYPE = "routing_type";


	// flow filter types
	public static final String IN = "in";
	public static final String OUT = "out";

	// mac entry types
	public static final String STATIC = "static";
	public static final String DYNAMIC = "dynamic";

	// Flow Lists
	public static final String FLOWLIST = "flowlist";
	public static final String FLOWLISTNAME = "flowlist_name";
	public static final String FLOWLISTS = "flowlists";
	public static final String FLOWLISTKEY = "flowlist_key";
	public static final String FLOWLISTENTRY = "flowlistentry";
	public static final String FLOWLISTENTRIES = "flowlistentries";
	public static final String SEQNUM = "seqnum";
	public static final String SEQUENCENUM = "sequence_num";
	public static final String MACVLANPRIORITY = "macvlanpriority";
	public static final String IPDSTADDRPREFIX = "ipdstaddrprefix";
	public static final String IPSRCADDRPREFIX = "ipsrcaddrprefix";
	public static final String IPV6DSTADDR = "ipv6dstaddr";
	public static final String IPV6DSTADDRPREFIX = "ipv6dstaddrprefix";
	public static final String IPV6SRCADDR = "ipv6srcaddr";
	public static final String IPV6SRCADDRPREFIX = "ipv6srcaddrprefix";
	public static final String IPPROTO = "ipproto";
	public static final String IPDSCP = "ipdscp";
	public static final String L4DSTPORT = "l4dstport";
	public static final String L4DSTENDPORT = "l4dstendport";
	public static final String L4SRCPORT = "l4srcport";
	public static final String L4SRCENDPORT = "l4srcendport";
	public static final String ICMPTYPENUM = "icmptypenum";
	public static final String IPV6ICMPTYPENUM = "ipv6icmptypenum";
	public static final String IPV6ICMPCODENUM = "ipv6icmpcodenum";

	// Policing Profile

	public static final String POLICINGMAP = "policingmap";
	public static final String PROFILE = "profile";
	public static final String PROFILES = "profiles";
	public static final String PROFILE_NAME = "profile_name";
	public static final String POLICINGPROFILEENTRIES = "policingprofileentries";

	public static final String FLNAME = "fl_name";
	public static final String GREENACTION = "greenaction";
	public static final String GREEN_ACTION = "green_action";
	public static final String GAPRIORITY = "ga_priority";
	public static final String GADSCP = "ga_dscp";
	public static final String GADROPPRECEDENCE = "ga_drop_precedence";
	public static final String YELLOWACTION = "yellowaction";
	public static final String YELLOW_ACTION = "yellow_action";
	public static final String YAPRIORITY = "ya_priority";
	public static final String YADSCP = "ya_dscp";
	public static final String YADROPPRECEDENCE = "ya_drop_precedence";
	public static final String REDACTION = "redaction";
	public static final String RED_ACTION = "red_action";
	public static final String RAPRIORITY = "ra_priority";
	public static final String RADSCP = "ra_dscp";
	public static final String RADROPPRECEDENCE = "ra_drop_precedence";

	// Policing Profile Entry
	public static final String KBPS = "kbps";

	// VTN Station
	public static final String VTNSTATIONS = "vtnstations";
	public static final String MACADDR = "macaddr";
	public static final String IPADDR = "ipaddr";
	public static final String IPV6ADDR = "ipv6addr";
	public static final String SWITCHID = "switch_id";
	public static final String VLANID = "vlan_id";
	public static final String VEXNAME = "vex_name";
	public static final String STATIONID = "station_id";
	public static final String CREATEDTIME = "createdtime";
	public static final String IPADDRS = "ipaddrs";
	public static final String IPV6ADDRS = "ipv6addrs";
	public static final String MAPTYPE = "maptype";
	public static final String MAPSTATUS = "mapstatus";
	public static final String VBRNAME = "vbr_name";
	public static final String INTERFACE = "interface";
	public static final String IFNAME = "if_name";
	public static final String OPERSTATUS = "operstatus";
	public static final String STATISTICS = "statistics";
	public static final String OPENFLOWCONTROLLER = "openflow_controller";
	public static final String ALLRX = "all_rx";
	public static final String PACKETS = "packets";
	public static final String OCTETS = "octets";
	public static final String ALLTX = "all_tx";
	public static final String OPENFLOWNW = "openflow_nw";
	public static final String EXISTINGRX = "existing_rx";
	public static final String EXISTINGTX = "existing_tx";
	public static final String EXPIREDRX = "expired_rx";
	public static final String EXPIREDTX = "expired_tx";
	public static final String ALLDROPRX = "all_drop_rx";
	public static final String EXISTINGDROPRX = "existing_drop_rx";
	public static final String EXPIREDDROPRX = "expired_drop_rx";
	public static final String VTN = "vtn";
	public static final String DESCRIPTION = "description";
	public static final String LASTCOMMITTEDTIME = "lastcommittedtime";

	// Flow Filter
	public static final String FLOWFILTER = "flowfilter";
	public static final String FLOWFILTERS = "flowfilters";
	public static final String FFTYPE = "ff_type";

	// Flow Filter Entry
	public static final String FLOWFILTERENTRY = "flowfilterentry";
	public static final String FLOWFILTERENTRIES = "flowfilterentries";
	public static final String ACTIONTYPE = "action_type";
	public static final String NMGNAME = "nmg_name";
	public static final String PRIORITY = "priority";
	public static final String DSCP = "dscp";
	public static final String SOFTWARE = "software";
	public static final String EXISTINGFLOW = "existingflow";
	public static final String EXPIREDFLOW = "expiredflow";
	public static final String TOTAL = "total";
	public static final String PASS = "pass";
	public static final String DROP = "drop";
	public static final String REDIRECT = "redirect";
	public static final String PENALTY = "penalty";
	// Policing Map
	public static final String OFSES = "ofses";
	public static final String DPID = "dp_id";
	public static final String POLICER = "policer";
	public static final String ID = "id";
	public static final String VEXTERNAL = "vexternal";
	public static final String PORT = "port";
	public static final String STATUS = "status";

	// vBridge
	public static final String VBRIDGE = "vbridge";
	public static final String INTERFACES = "interfaces";
	public static final String IFINDEX = "ifindex";
	public static final String ADMINSTATUS = "adminstatus";
	public static final String MTU = "mtu";

	// Host Address
	public static final String HOST_ADDR = "hostaddr";
	public static final String PREFIX = "prefix";

	// L2 Domain
	public static final String L2DOMAINS = "l2domains";
	public static final String L2DOMAINMEMBER = "l2domain_member";
	public static final String L2DOMAINMEMBERS = "l2domain_members";
	public static final String L2DOMAINID = "l2domain_id";

	// MAC Entry
	public static final String MACENTRIES = "macentries";

	// Ping
	public static final String DFBIT = "dfbit";
	public static final String INTERVAL = "interval";
	public static final String PACKETSIZE = "packetsize";
	public static final String TIMEOUT = "timeout";

	// VLAN Map
	public static final String VLANMAPS = "vlanmaps";
	public static final String VLANMAP = "vlanmap";
	public static final String VLANMAPID = "vlanmap_id";

	// Network Monitor Group
	public static final String NETMONGROUPS = "netmongroups";
	public static final String NETMONGROUP = "netmongroup";
	public static final String NAME = "name";
	public static final String HOSTS = "hosts";
	public static final String HEALTHINTERVAL = "health_interval";
	public static final String RECOVERYINTERVAL = "recovery_interval";
	public static final String WAITTIME = "wait_time";
	public static final String FAILURECOUNTS = "failure_counts";
	public static final String RECOVERYCOUNTS = "recovery_counts";
	public static final String PINGSEND = "pingsend";
	public static final String PINGRECV = "pingrecv";
	public static final String PINGERR = "pingerr";
	public static final String PINGTRBL = "pingtrbl";

	// Policing Map
	public static final String TOTALPACKETS = "total_packets";
	public static final String TOTALBYTES = "total_bytes";
	public static final String GREENYELLOWPACKETS = "green_yellow_packets";
	public static final String GREENYELLOWBYTES = "green_yellow_bytes";
	public static final String REDPACKETS = "red_packets";
	public static final String REDBYTES = "red_bytes";

	// Flow Filter Entry
	public static final String REDIRECTDST = "redirectdst";
	public static final String VNODENAME = "vnode_name";
	public static final String IPVERSION = "ip_version";
	public static final String ICMPCODENUM = "icmpcodenum";

	// Interfaces
	public static final String NEIGHBOR = "neighbor";

	// Port Map
	public static final String PORTMAP = "portmap";
	public static final String VLANTAG = "vlantag";
	public static final String PORTMAPS = "portmaps";
	public static final String PORTMAP_NAME = "portmap_name";
	public static final String ANY_VLAN_ID = "any_vlan_id";

	// vRouter
	public static final String VROUTER = "vrouter";
	public static final String VRTNAME = "vrt_name";

	// Ping
	public static final String PING = "ping";
	public static final String IPDSTADDR = "ipdstaddr";
	public static final String IPSRCADDR = "ipsrcaddr";

	// Static IP Route
	public static final String STATIC_IPROUTE = "static_iproute";
	public static final String STATIC_IPROUTES = "static_iproutes";
	public static final String NEXTHOPADDR = "nexthopaddr";
	public static final String GROUPMETRIC = "groupmetric";
	public static final String STATICIPROUTEID = "static_iproute_id";

	// ARP Entry
	public static final String ARPENTRIES = "arpentries";
	public static final String TYPE = "type";

	// DHCP Relay
	public static final String DHCPRELAY = "dhcprelay";
	public static final String DHCPRELAYSTATUS = "dhcp_relay_status";
	public static final String SERVER = "server";
	public static final String SERVERS = "servers";

	// vByPass
	public static final String VBYPASS = "vbypass";
	public static final String VBYPASS_NAME = "vbypass_name";
	public static final String VBYPASSES = "vbypasses";

	// vTep
	public static final String VTEP = "vtep";
	public static final String VTEPNAME = "vtep_name";
	public static final String VTEPS = "vteps";

	// vTep Group
	public static final String VTEPGROUP = "vtepgroup";
	public static final String VTEPGROUPNAME = "vtepgroup_name";
	public static final String MEMBERVTEPS = "member_vteps";
	public static final String VTEPGROUPS = "vtepgroups";
	public static final String VTEPMEMBERNAME = "vtepmember_name";

	// vTunnel
	public static final String VTUNNEL = "vtunnel";
	public static final String VTUNNELNAME = "vtunnel_name";
	public static final String LABEL = "label";
	public static final String VTUNNELS = "vtunnels";

	// vLink
	public static final String VLINK = "vlink";
	public static final String VLKNAME = "vlk_name";
	public static final String VNODE1NAME = "vnode1_name";
	public static final String IF1NAME = "if1_name";
	public static final String VNODE2NAME = "vnode2_name";
	public static final String IF2NAME = "if2_name";
	public static final String BOUNDARYMAP = "boundary_map";

	// Resource
	public static final String RESOURCES = "resources";
	public static final String TIMESTAMP = "timestamp";
	public static final String CPUUTILIZATION = "cpu_utilization";
	public static final String SYSTEM = "system";
	public static final String CURRENT = "current";
	public static final String ONEMINAVG = "1minavg";
	public static final String TENMINAVG = "10minavg";
	public static final String SIXTYMINAVG = "60minavg";
	public static final String PFCPROCESS = "pfc_process";
	public static final String PROCESSNAME = "process_name";
	public static final String MEMORYUTILIZATION = "memory_utilization";
	public static final String DISKUTILIZATION = "disk_utilization";
	public static final String DEVICENAME = "device_name";
	public static final String UTILIZATION = "utilization";

	// Process
	public static final String PROCESSES = "processes";
	public static final String STARTTIME = "start_time";
	public static final String PROCESSESHISTORY = "processes_history";
	public static final String ACTION = "action";
	public static final String CAUSE = "cause";
	public static final String HISTORY = "history";
	public static final String ALL = "all";

	public static final String UNC = "unc";

	// Version
	public static final String VERSION = "version";
	public static final String VERSIONNO = "version_no";
	public static final String PATCHES = "patches";
	public static final String PATCHNO = "patch_no";

	// Alarm
	public static final String ALARMS = "alarms";
	public static final String ALARMNUMBER = "alarmnumber";
	public static final String ALARMNO = "alarm_no";
	public static final String SEVERITY = "severity";
	public static final String SUMMARY = "summary";
	public static final String MESSAGE = "message";
	public static final String ALARMINFO = "alarminfo";
	public static final String ALARMSHISTORY = "alarms_history";
	public static final String EMERGENCY = "emergency";
	public static final String ALERT = "alert";
	public static final String CRITICAL = "critical";
	public static final String WARNING = "warning";
	public static final String NOTICE = "notice";
	public static final String INFORMATION = "information";
	public static final String DEBUG = "debug";
	public static final String OCCURRED = "occurred";
	public static final String RECOVERED = "recovered";

	// SNMP Agent
	public static final String SNMPAGENT = "snmpagent";
	public static final String CONTACT = "contact";
	public static final String LOCATION = "location";
	public static final String DEFAULTSEVERITY = "default_severity";

	// Community
	public static final String COMMUNITY = "community";
	public static final String COMMUNITYNAME = "community_name";
	public static final String ACCESS = "access";
	public static final String COMMUNITIES = "communities";
	public static final String HOST = "host";
	public static final String HOSTID = "host_id";
	public static final String HOSTNAME = "hostname";

	// Trap Destination
	public static final String TRAPDST = "trapdst";
	public static final String TRAPDSTID = "trapdst_id";
	public static final String DEFAULTFILTER = "default_filter";
	public static final String TRAPDSTS = "trapdsts";
	public static final String TRAPFILTER = "trapfilter";
	public static final String FILTERENABLEID = "filter_enable_id";
	public static final String CATEGORY = "category";
	public static final String TRAPNAME = "trap_name";
	public static final String TRAPFILTERS = "trapfilters";
	public static final String FILTERDISABLEID = "filter_disable_id";
	public static final String TRAPSHISTORY = "traps_history";
	public static final String TRAPOID = "trap_oid";
	public static final String SOURCE = "source";
	public static final String VARBINDS = "varbinds";
	public static final String OID = "oid";
	public static final String VALUE = "value";
	public static final String DESTINATIONS = "destinations";

	// Syslog
	public static final String SYSLOG = "syslog";
	public static final String ROTATEGENERATION = "rotate_generation";
	public static final String ROTATETERM = "rotate_term";
	public static final String ROTATESIZE = "rotate_size";

	// Tracelog
	public static final String SEVERITIES = "severities";
	public static final String APLNAME = "aplname";

	// Resource Monitor
	public static final String RESOURCEMONITOR = "resourcemonitor";
	public static final String DISKNAME = "disk_name";
	public static final String RISINGTHRESHOLD = "rising_threshold";
	public static final String RISINGINTERVAL = "rising_interval";
	public static final String FALLINGTHRESHOLD = "falling_threshold";
	public static final String FALLINGINTERVAL = "falling_interval";

	// NTP Server
	public static final String NTPSERVER = "ntpserver";
	public static final String NTPSERVERID = "ntpserver_id";
	public static final String NTPSERVERS = "ntpservers";

	// Controller
	public static final String CONTROLLER = "controller";
	public static final String AUDITSTATUS = "auditstatus";
	public static final String MULTIPATH = "multipath";
	public static final String VLANCONNECTSTATUS = "vlan_connect_status";
	public static final String PORTNAMEVALIDATION = "port_name_validation";

	// Traffic Monitor
	public static final String TRAFFICMONITOR = "trafficmonitor";
	public static final String ADMINSET = "adminset";
	public static final String MONITORING = "monitoring";
	public static final String RATE = "rate";
	public static final String FALLIJNGTHRESHOLD = "fallijng_threshold";
	public static final String POLLINGINTERVAL = "polling_interval";
	public static final String CONGESTIONPORTS = "congestion_ports";
	public static final String IFSPEED = "ifspeed";
	public static final String RX = "rx";
	public static final String RECEPTIONBAND = "reception_band";
	public static final String USEDBANDWIDTH = "used_bandwidth";

	// Extended VLAN
	public static final String EXTENDEDVLAN = "extended_vlan";
	public static final String ENABLECONFLICT = "enable_conflict";
	public static final String DISABLECONFLICT = "disable_conflict";

	// License
	public static final String LICENSES = "licenses";
	public static final String INSTALLEDLICENSES = "installed_licenses";
	public static final String ALLOCATEDLICENSES = "allocated_licenses";
	public static final String EXPIREDLICENSES = "expired_licenses";
	public static final String INSTALLEDUCODES = "installed_ucodes";
	public static final String DISABLEDUCODES = "disabled_ucodes";
	public static final String EXPIREDUCODES = "expired_ucodes";
	public static final String OLDESTEXPIRYDATE = "oldest_expirydate";
	public static final String EXPIRYDATE = "expirydate";

	// QoS Flow Entry
	public static final String QOSFLOWENTRIES = "qosflowentries";
	public static final String MATCH = "match";
	public static final String MACDSTADDR = "macdstaddr";
	public static final String MACSRCADDR = "macsrcaddr";
	public static final String MACETHERTYPE = "macethertype";
	public static final String MACVLANTAG = "macvlantag";
	public static final String POLICERID = "policer_id";

	// Unknown Unicast Flooding
	public static final String FLOODING = "flooding";
	public static final String PPS = "pps";
	public static final String FLOODPACKETS = "flood_packets";
	public static final String DROPPACKETS = "drop_packets";
	public static final String LASTTIME = "lasttime";

	// VLAN Map
	public static final String FLOODENTRIES = "floodentries";

	// Flow Entry List
	public static final String FLOWENTRYLIST = "flowentrylist";
	public static final String LISTNAME = "list_name";
	public static final String FLOWENTRYLISTS = "flowentrylists";

	// Flow Entry List Entry
	public static final String FLOWENTRYLISTENTRY = "flowentrylistentry";

	// Flow Entry Info
	public static final String FLOWENTRYINFO = "flowentryinfo";
	public static final String INFONAME = "info_name";
	public static final String TABLE = "table";
	public static final String FLOWENTRYINFOS = "flowentryinfos";
	public static final String FLOWENTRYACTION = "flowentryaction";
	public static final String ACTIONNAME = "action_name";
	public static final String STRIPMACVLAN = "stripmacvlan";
	public static final String FLOWENTRYACTIONS = "flowentryactions";
	public static final String POLCINGPROFILEENTRY = "policingprofileentry";

	// VTerminal
	public static final String VTERMINALNAME = "vterminal_name";

	// Flood Policing
	public static final String TWORATETHREECOLOR = "tworatethreecolor";
	public static final String METER = "meter";
	public static final String RATEUNIT = "rateunit";
	public static final String CIR = "cir";
	public static final String CBS = "cbs";
	public static final String PIR = "pir";
	public static final String PBS = "pbs";
	public static final String FLOWENTRY = "flowentry";
	public static final String FLOWID = "flowid";
	public static final String OUTPUTTYPE = "output_type";
	public static final String OUTPUTPORTNAME = "output_port_name";
	public static final String NORMAL = "normal";
	public static final String FLOWENTRIES = "flowentries";
	public static final String INGRESS = "ingress";
	public static final String CORE = "core";
	public static final String EGRESS = "egress";
	public static final String OTHER = "other";
	public static final String DURATION = "duration";
	public static final String MIRRORENTRY = "mirrorentry";
	public static final String MIRRORENTRYID = "mirrorentry_id";
	public static final String FELNAME = "fel_name";
	public static final String MIRRORENTRIES = "mirrorentries";
	public static final String FLOODENTRY = "floodentry";
	public static final String FLOODENTRYID = "floodentry_id";
	public static final String SWITCH = "switch";
	public static final String MODEL = "model";
	public static final String AVOIDSTATUS = "avoidstatus";
	public static final String CONNECTEDTIME = "connectedtime";
	public static final String CONNECTEDFOR = "connectedfor";
	public static final String MANUFACTURE = "manufacture";
	public static final String HARDWARE = "hardware";
	public static final String ALARMOCCURRING = "alarmoccurring";
	public static final String FEATURE = "feature";
	public static final String CAPABILITY = "capability";
	public static final String VLANCAPABILITY = "vlan_capability";
	public static final String EXTERNALPORT = "external_port";
	public static final String PLDPDMAC = "pldp_dmac";
	public static final String PLDPKEEPALIVE = "pldp_keepalive";
	public static final String SPEED = "speed";
	public static final String BCMCSPTUSE = "bcmc_spt_use";
	public static final String WEIGHT = "weight";
	public static final String DUPLEX = "duplex";
	public static final String LAGCHANNELGRP = "lagchannelgrp";

	// Trunk Port
	public static final String TRUNKPORT = "trunkport";
	public static final String MEMBERPORTS = "member_ports";
	public static final String STACKLINKPORTS = "stack_link_ports";
	public static final String DIRECTION = "direction";
	public static final String LOADBALANCE = "loadbalance";
	public static final String DESIGNATED = "designated";
	public static final String PORT1NAME = "port1_name";
	public static final String TRUNKPORT1NAME = "trunk_port1_name";
	public static final String SWITCH2ID = "switch2_id";
	public static final String PORT2NAME = "port2_name";
	public static final String TRUNKPORT2NAME = "trunk_port2_name";
	public static final String DOMAIN = "domain";
	public static final String MEMBERSWITCHES = "member_switches";
	public static final String BOUNDARY = "boundary";

	// Link
	public static final String LINK = "link";
	public static final String CONTROLLER1ID = "controller1_id";
	public static final String SWITCH1ID = "switch1_id";
	public static final String CONTROLLER2ID = "controller2_id";

	// Session
	public static final String INFO = "info";
	public static final String USERTYPE = "usertype";
	public static final String LOGINTIME = "login_time";
	public static final String CONFIGSTATUS = "configstatus";

	// validation requirements
	public static final String APIVERSION = "api_version";
	public static final String PORTTYPE = "port_type";
	public static final String TRUNK = "trunk";
	public static final String TAGGED = "tagged";
	public static final String TRUE = "true";
	public static final String FALSE = "false";
	public static final String CODE = "err_code";
	public static final String MSG = "err_msg";
	public static final String INFOS = "err_infos";
	public static final String ERROR = "error";
	public static final String MPLS = "mpls";
	public static final String VNI = "vni";
	public static final String VSID = "vsid";

	// INTEGER
	public static final int LEN_15 = 15;
	public static final int LEN_19 = 19;
	public static final int LEN_24 = 24;
	public static final int LEN_31 = 31;
	public static final int LEN_32 = 32;
	public static final int LEN_63 = 63;
	public static final int LEN_64 = 64;
	public static final int LEN_72 = 72;
	public static final int LEN_127 = 127;
	public static final int LEN_255 = 255;
	public static final int LEN_319 = 319;

	public static final int VAL_0 = 0;
	public static final int VAL_1 = 1;
	public static final int VAL_5 = 5;
	public static final int VAL_7 = 7;
	public static final int VAL_15 = 15;
	public static final int VAL_16 = 16;
	public static final int VAL_32 = 32;
	public static final int VAL_30 = 30;
	public static final int VAL_63 = 63;
	public static final int VAL_128 = 128;
	public static final int VAL_255 = 255;
	public static final int VAL_4000 = 4000;
	public static final int VAL_4095 = 4095;
	public static final int VAL_32766 = 32766;
	public static final int VAL_65535 = 65535;
	public static final int VAL_FFFF = 0xFFFF;
	public static final int VAL_FFFE = 0xFFFE;
	public static final BigInteger BIG_VAL0 = new BigInteger("0");
	public static final BigInteger BIG_VAL1 = new BigInteger("1");
	public static final BigInteger BIG_VAL_4294967040 = new BigInteger(
			"4294967040");
	public static final BigInteger BIG_VAL_4294967295 = new BigInteger(
			"4294967295");
	public static final BigInteger BIG_VAL_18446744073709551999 = new BigInteger(
			"18446744073709551999");
	public static final BigInteger BIG_VAL_9999999999999999999 = new BigInteger(
			"9999999999999999999");
	public static final BigInteger BIG_VAL_18446744073709551615 = new BigInteger(
			"18446744073709551615");
	public static final long LONG_VAL_0 = 0L;
	public static final long LONG_VAL_1 = 1L;
	public static final long LONG_VAL_4294967295 = 4294967295L;
	public static final String LOGIN_NAME = "login_name";
	public static final String LOGIN_TIME = "login_time";
	public static final String VBRDESCRIPTION = "vbr_description";
	public static final String ADMIN_STATUS = "admin_status";
	public static final String LABEL_TYPE = "label_type";
	public static final String NMG_NAME = "nmg_name";
	public static final String SWID = "swid";
	public static final String NOSWID = "no_swid";
	public static final String VTEPGROUPMEMBERNAME = "member_vteps";
	public static final String UP = "up";
	public static final String DOWN = "down";

	public static final String NO_VLAN_ID = "no_vlan_id";
	public static final String NWMNAME = "nwm_name";
	public static final String STATICIPROUTE = "static_iproute_id";
	public static final String PREFIXLEN = "prefixlen";
	public static final String ACTIVE = "active";
	public static final String INACTIVE = "inactive";
	public static final String STARTING = "starting";
	public static final String WAITING = "waiting";

	// physical validators
	public static final String LOGICAL_PORT1_ID = "logical_port1_id";
	public static final String LOGICAL_PORT2_ID = "logical_port2_id";
	public static final String DOMAIN1_ID = "domain1_id";
	public static final String DOMAIN2_ID = "domain2_id";
	public static final String DEFAULT = "default";
	public static final String PORT_ID = "port_id";
	public static final String LINKSAPERATOR = "\\.";
	public static final String LINKCONCATENATOR = ".";
	public static final String UNKNOWN = "unknown";
	public static final String BYPASS = "bypass";
	public static final String PFC = "pfc";
	public static final String LEGACY = "legacy";
	public static final String OVERLAY = "overlay";
	// public static final String WAITINGAUDIT ="waiting_audit";
	public static final String AUDITING = "auditing";
	public static final String HYPHEN = "-";
	public static final int LEN_256 = 256;
	public static final int LEN_4 = 4;
	public static final int VAL_2 = 2;
	public static final int VAL_3 = 3;
	public static final int VAL_4 = 4;
	public static final int VAL_6 = 6;
	public static final int VAL_8 = 8;
	public static final int VAL_12 = 12;
	public static final int LEN_20 = 20;

	public static final String WAITING_AUDIT = "waiting_audit";
	public static final String MANUFACTURER = "manufacturer";
	public static final String ALARMSSTATUS = "alarmsstatus";
	public static final String TRUNK_ALLOWED_VLAN = "trunk_allowed_vlan";
	public static final String LOGICAL_PORT_ID = "logical_port_id";
	public static final String CONNECTED_PORT_ID = "port_name";
	public static final String CONNECTED_SWITCH_ID = "switch_id";
	public static final String PORT1_NAME = "port1_name";
	public static final String PORT2_NAME = "port2_name";
	public static final String ACTUALVERSION = "actual_version";
	public static final String LOGICALPORT1ID = "logical_port1_id";
	public static final String DOMAIN1ID = "domain1_id";
	public static final String LOGICALPORT2ID = "logical_port2_id";
	public static final String DOMAIN2ID = "domain2_id";
	public static final String DOMAINLPORT = "logicalport";
	public static final String DOMAINLPORTS = "logicalports";
	public static final String OPERDOWNCRITERIA = "operstatus_criteria";
	public static final String NMG_STATUS = "nmg_status";
	public static final String ROUTER = "router";
	public static final String BRIDGE = "bridge";

	public static final String DIRECTION_INTERNAL = "internal";
	public static final String DIRECTION_EXTERNAL = "external";
	public static final String DIRECTION_UNKNOWN = "unknown";
	// public static final String LOGICALPORT_ID = "logical_port_id";
	public static final String LOGICALPORT = "logical_port";
	public static final String LOGICALPORTS = "logical_ports";
	public static final String MEMBER_PORTS = "member_ports";
	public static final String SUBDOMAIN = "subdomain";
	public static final String OPERDOWN_CRITERIA = "operdown_criteria";
	public static final String ANY = "any";
	public static final String VLANMAPIDSEPERATOR = "-";
	public static final String LPID = "lpid";
	public static final String NOLPID = "no_lpid";
	public static final String IPROUTES = "iproutes";
	public static final String DSTADDR = "dstaddr";
	public static final String GATEWAY = "gateway";
	public static final String FLAGS = "flags";
	public static final String METRIC = "metric";
	public static final String USE = "use";
	public static final String IFKIND = "if_kind";
	public static final String BLANK = "";
	public static final String ONE = "1";
	public static final String OFS_MAP = "ofs-map";
	public static final String VLAN_MAP = "vlan-map";
	public static final String VALID = "valid";
	public static final String INVALID = "invalid";
	public static final String ALARMSTATUS = "alarmstatus";
	public static final String CLEAR = "clear";
	public static final String RAISE = "raise";
	public static final String TUNNEL_ENDPOINT = "tunnel_endpoint";
	public static final String PORT_GROUP = "port_group";
	public static final String DEFAULT_DOMAIN_ID = "(DEFAULT)";
	public static final String SWITCHID_NOT_FOUND = "switchIdNotFound";
	public static final String PORTID_NOT_FOUND = "portIdNotFound";
	public static final String VNP = "vnp";
	public static final String ODC = "odc";
	public static final String DIFF = "diff";
	public static final String DIFF_STATUS = "diff_status";
	public static final String TWO = "2";
	public static final String DEL = "del";
	public static final String V = "V";
	// Constatnts for portmap under overlay
	public static final String VTUNNEL_INTERFACE_PORTMAP = "VTunnelInterfacePortMap";
	public static final String VTEP_INTERFACE_PORTMAP = "VTepInterfacePortMap";
	public static final String VBRIDGE_INTERFACE_PORTMAP = "VBridgeInterfacePortMap";
	public static final String VTERMINAL_INTERFACE_PORTMAP = "VTerminalInterfacePortMap";
	public static final String DATAFLOWS = "dataflows";
	// vterminal
	public static final String VTERMINAL = "vterminal";
	public static final String VTERMINALS = "vterminals";
	public static final String VTERMINAL_NAME = "vterminal_name";
	public static final String VTERM_DESCRIPTION = "vterm_description";
	public static final String SRCMAC = "srcmac";
	public static final String RX_PACKETS = "rx_packets";
	public static final String TX_PACKETS = "tx_packets";
	public static final String TX_BYTES = "rx_bytes";
	public static final String RX_BYTES = "tx_bytes";
	public static final String RX_DROPPED = "rx_dropped";
	public static final String TX_DROPPED = "tx_dropped";
	public static final String RX_ERRORS = "rx_errors";
	public static final String TX_ERRORS = "tx_errors";
	public static final String RX_FRAME_ERR = "rx_frame_err";
	public static final String RX_OVER_ERR = "rx_over_err";
	public static final String RX_CRC_ERRS = "rx_crc_err";
	public static final String COLLISIONS = "collisions";

	// dataflow

	public static final String MAPPINGID = "mapping_id";
	public static final String MAPPING = "mapping";
	public static final String MAPPINGS = "mappings";
	// VTN Data Flow
	public static final String REASON = "reason";
	public static final String CONTROLER_TYPE = "controller_type";
	public static final String FLOW_ID = "flow_id";
	public static final String FLOW_TYPE = "type";
	public static final String POLICY_INDEX = "policy_index";
	public static final String MATCH_TYPE = "match_type";
	public static final String IN_PORT = "inport";
	public static final String DL_ADDR = "dl_addr";
	public static final String V_MASK = "v_mask";
	public static final String DL_ADDR_MASK = "dl_addr_mask";
	public static final String DL_TYPE = "dl_type";
	public static final String VLAN_VID = "vlan_vid";
	public static final String VLAN_PCP = "vlan_pcp";
	public static final String IP_TOS = "ip_tos";
	public static final String IP_PROTO = "ipproto";
	public static final String IPV4_ADDR = "ipv4_addr";
	public static final String IPV4_ADDR_MASK = "ipv4_addr_mask";
	public static final String TP_PORT = "tp_port";
	public static final String TP_PORT_MASK = "tp_port_mask";
	public static final String IPV6_ADDR = "ipv6_addr";
	public static final String VTN_ID = "vtn_id";
	public static final String INGRESS_SWITCH_ID = "ingress_switch_id";
	public static final String INGRESS_STATION_ID = "ingress_station_id";
	public static final String EGRESS_SWITCH_ID = "egress_switch_id";
	public static final String EGRESS_STATION_ID = "egress_station_id";
	public static final String OUTPORT = "outport";
	public static final String ENQUEUE_ID = "enqueue_id";
	public static final String QUEUE_ID = "queue_id";
	public static final String ENQUEUEPORT = "enqueueport";
	public static final String MACSRC = "macsrc";
	public static final String MACDST = "macdst";
	public static final String ETHERNATETYPE = "ethertype";
	public static final String VLAN_ID = "vlan_id";
	public static final String IPSRC = "ipsrc";
	public static final String IPDST = "ipdst";
	public static final String IPV6DST = "ipv6src";
	public static final String IPV6SRC = "ipv6dst";
	public static final String TPSRC = "tpsrc";
	public static final String TSDST = "tpdst";
	public static final String IPTOS = "iptos";
	public static final String SWITCH_ID = "switch_id";
	public static final String INPORT = "inport";
	public static final String TPDST = "tpdst";
	public static final String PATHINFOS = "pathinfos";
	public static final String CONTROLLER_DATAFLOWS = "controller_dataflows";
	public static final String REASON_SUCCESS = "success";
	public static final String REASON_NOT_SUPP = "operation not supported";
	public static final String REASON_EXCD_LIM = "exceeds flow limit";
	public static final String REASON_CTRL_DISC = "controller disconnected";
	public static final String REASON_EXCD_HOP = "exceeds hop limit ";
	public static final String REASON_DST_NOT_REACHED = "dst not reached";
	public static final String REASON_FLOW_NOTFOUND = "flow not found";
	public static final String REASON_SYS_ERROR = "system error";
	public static final String RES_SUCCESS = "success";
	public static final String RES_EXCEEDS_FLOW_LIMIT = "exceeds flow limit";
	public static final String RES_EXCEEDS_HOP_LIMIT = "exceeds hop limit";
	public static final String RES_DST_NOT_REACHED = "dst_not_reached";
	public static final String RES_CTRLR_DISCONNECTED = "controller_disconnected";
	public static final String RES_OPERATION_NOT_SUPPORTED = "operation not supported";
	public static final String RES_FLOW_NOT_FOUND = "flow_not_found";
	public static final String RES_SYSTEM_ERROR = "system_error";
	public static final String CONTROLLER_DOMAIN_DATAFLOWS = "controller_domain_dataflows";
	public static final String IDLETIMEOUT = "idletimeout";
	public static final String HARDTIMEOUT = "hardtimeout";
	public static final String INGRESS_VNODE_NAME = "ingress_vnode_name";
	public static final String INGRESS_IF_NAME = "ingress_if_name";
	public static final String INGRESS_PORT_NAME = "ingress_port_name";
	public static final String INGRESS_LOGICAL_PORT_ID = "ingress_logical_port_id";
	public static final String INGRESS_DOMAIN_ID = "ingress_domain_id";
	public static final String EGRESS_VNODE_NAME = "egress_vnode_name";
	public static final String EGRESS_IF_NAME = "egress_if_name";
	public static final String EGRESS_PORT_NAME = "egress_port_name";
	public static final String EGRESS_LOGICAL_PORT_ID = "egress_logical_port_id";
	public static final String EGRESS_DOMAIN_ID = "egress_domain_id";
	public static final String IN_VNODE_NAME = "in_vnode_name";
	public static final String IN_IF_NAME = "in_if_name";
	public static final String OUT_VNODE_NAME = "out_vnode_name";
	public static final String OUT_IF_NAME = "out_if_name";
	public static final String VLINK_FLAG = "vlink_flag";
	public static final String COORDINATOR_VERSION = "coordinator_version";

	public static final String VLAN_PRIORITY = "vlan_priority";

	public static final String SRCMACADDR = "srcmacaddr";
	public static final String SETMACSRCADDR = "setmacsrcaddr";
	public static final String SETMACDSTADDR = "setmacdstaddr";
	public static final String SETVLAN_ID = "setvlan_id";
	public static final String SETVLAN_PRIORITY = "setvlan_priority";
	public static final String SETIPSRCADDR = "setipsrcaddr";
	public static final String SETIPDSTADDR = "setipdstaddr";
	public static final String SETIPTOS = "setiptos";
	public static final String SETL4SRCPORT_ICMPTYPE = "setl4srcport_icmptype";
	public static final String SETL4DSTPORT_ICMPTYPE = "setl4dstport_icmptype";
	public static final String SETIPV6DSTADDR = "setipv6dstaddr";
	public static final String SETIPV6SRCADDR = "setipv6srcaddr";
	public static final String OUTPUTPORT = "outputport";
	public static final String MACDSTADDR_MASK = "macdstaddr_mask";
	public static final String MACSRCADDR_MASK = "macsrcaddr_mask";
	public static final String IPSRCADDR_MASK = "ipsrcaddr_mask";
	public static final String IPDSTADDR_MASK = "ipdstaddr_mask";
	public static final String IPV6SRCADDR_MASK = "ipv6srcaddr_mask";
	public static final String IPV6DSTADDR_MASK = "ipv6dstaddr_mask";
	public static final String L4SRCPORT_ICMPTYPE = "l4srcport_icmptype";
	public static final String L4DSTPORT_ICMPTYPE = "l4dstport_icmptype";
	public static final String L4SRCPORT_ICMPTYPE_MASK = "l4srcport_icmptype_mask";
	public static final String L4DSTPORT_ICMPTYPE_MASK = "l4dstport_icmptype_mask";
	public static final String STATUS_INIT = "init";
	public static final String STATUS_ACTIVATING = "activating";
	public static final String STATUS_ACTIVE = "active";
	public static final String STATUS_SWITCHING = "switching";
	public static final String IN_PORT_NAME = "in_port_name";
	public static final String OUT_PORT_NAME = "out_port_name";
	public static final String FLOWCOUNT = "flowcount";
	public static final String RX_FRAMEERR = "rx_frameerr";
	public static final String RX_CRCERR = "rx_crcerr";
	public static final String RX_OVERERR = "rx_overerr";
	public static final String NOT_EXISTS = "not_exists";
	public static final String EXISTS = "exists";
	public static final String MAPPINGINFOS = "mappinginfos";
	public static final String VLAN_ID_65535 = "65535";
	public static final String EMPTY = "";
	public static final String STRIPVLAN = "stripvlan";
	public static final String AUDIT = "audit";
	public static final String GREEN_YELLOW = "green_yellow";
	public static final String RED = "red";
	public static final String UNIT = "unit";
	public static final String DROPPRECEDENCE = "drop_precedence";

	// Path policy
	public static final String POLICYID = "policy_id";
	public static final String PATHPOLICY = "pathpolicy";
	public static final String PATHPOLICIES = "pathpolicies";
	public static final String LINK_WEIGHTS = "link_weights";
	public static final String DISABLE_SWITCHES = "disable_switches";
	public static final String PATHMAPENTRIES = "pathmapentries";
	public static final String PATHMAPENTRY = "pathmapentry";
	public static final String ENTRYID = "entry_id";
	public static final String PATHPOLICYENTRY = "pathpolicyentry";
	public static final String PATHPOLICYENTRIES = "pathpolicyentries";
	public static final String AGEOUTTIME = "ageout_time";
	public static final String POLICYTYPE = "policy_type";
	public static final String POLICINGPROFILE = "policingprofile";
	public static final String POLICINGPROFILES = "policingprofiles";
	public static final String POLICINGPROFILEENTRY = "policingprofileentry";
	public static final String VNODETYPE = "vnode_type";

	// spine_domain
	public static final String UNIFIED_NETWORK_NAME = "unified_network_name";
	public static final String SPINE_DOMAIN = "spine_domain";
	public static final String SPINE_DOMAINS = "spine_domains";
	public static final String ASSIGNED_LABELS = "assigned_labels";
	public static final String ASSIGNED_LABEL = "assigned_label";
	public static final String SPINE_DOMAIN_NAME = "spine_domain_name";
	public static final String USED_COUNT = "used_count";
	public static final String ALARM_STATUS = "alarm_status";
	
	//commit and abort
	public static final String CANCEL_AUDIT = "cancel_audit";

	public static final String MODE_TYPE = "mode_type";
	public static final String REAL_MODE = "real";
	public static final String VIRTUAL_MODE = "virtual";
	public static final String VTN_MODE = "vtn";
	public static final String GLOBAL_MODE = "global";
	public static final String EXPANDING = "expanding";
	public static final String VNODES = "vnodes";
	public static final String LABELS = "labels";
	public static final String LABEL_NAME = "label_name";
	public static final String MAX_COUNT = "max_count";
	public static final String RISING_THRESHOLD = "rising_threshold";
	public static final String FALLING_THRESHOLD = "falling_threshold";
	public static final String RANGES = "ranges";
	public static final String RANGE_ID = "range_id";
	public static final String RANGE_MIN = "min";
	public static final String RANGE_MAX = "max";
	public static final String CONNECTED_VNODE_NAME = "connected_vnode_name";
	public static final String CONNECTED_IF_NAME = "connected_if_name";
	public static final String CONNECTED_VLK_NAME = "connected_vlk_name";
	public static final String CONTROLLER_VTN_NAME = "controller_vtn_name";
	public static final String CONTROLLER_VTN_LABEL = "controller_vtn_label";
	
	//domain type
	public static final String LEAF = "leaf";
	public static final String SPINE = "spine";

	//spine domain fdbusage
	public static final String FDBUSAGE = "fdbusage";
	public static final String TOTAL_MAX_SW_ID = "total_max_switch_id";
	public static final String TOTAL_MAX_COUNT = "total_max_count";
	public static final String TOTAL_MIN_SW_ID = "total_min_switch_id";
	public static final String TOTAL_MIN_COUNT = "total_min_count";
	public static final String TOTAL_AVG_COUNT = "total_average_count";
	public static final String NUM_OF_SW = "num_of_switches";
	public static final String VTN_FDBUSAGES = "vtn_fdbusages";
	public static final String MAX_SW_ID = "max_switch_id";
	public static final String MIN_SW_ID = "min_switch_id";
	public static final String MIN_COUNT = "min_count";
	public static final String AVG_COUNT = "average_count";
	//logical port
	public static final String MAPPING_GROUP = "mapping_group";
	public static final String BOUNDARY_CANDIDATE = "boundary_candidate";
	public static final String YES = "yes";
	public static final String NO = "no";
	public static final String CONNECTEDSWITCHID = "connected_switch_id";
	public static final String CONNECTEDPORTNAME = "connected_port_name";
	public static final String CONNECTEDCONTROLLERID = "connected_controller_id";
}
