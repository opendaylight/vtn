/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron;

import java.math.BigInteger;
import java.util.Map;
import java.util.Set;
import java.net.InetAddress;
import java.net.HttpURLConnection;
import java.net.UnknownHostException;

import org.opendaylight.controller.networkconfig.neutron.INeutronPortCRUD;
import org.opendaylight.controller.networkconfig.neutron.NeutronPort;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.utils.NodeCreator;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;
import org.opendaylight.ovsdb.lib.notation.OvsDBSet;
import org.opendaylight.ovsdb.lib.notation.UUID;
import org.opendaylight.ovsdb.lib.table.Bridge;
import org.opendaylight.ovsdb.lib.table.Interface;
import org.opendaylight.ovsdb.lib.table.Open_vSwitch;
import org.opendaylight.ovsdb.lib.table.Port;
import org.opendaylight.ovsdb.lib.table.internal.Table;
import org.opendaylight.ovsdb.plugin.OVSDBConfigService;
import org.opendaylight.ovsdb.plugin.OVSDBInventoryListener;
import org.opendaylight.ovsdb.plugin.IConnectionServiceInternal;
import org.opendaylight.ovsdb.plugin.StatusWithUuid;

import org.opendaylight.vtn.manager.IVTNManager;
import org.opendaylight.vtn.manager.SwitchPort;
import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.PortMapConfig;


import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * Handle events from OVSDB Southbound plugin.
 */
public class OVSDBPluginEventHandler extends VTNNeutronUtils
                               implements OVSDBInventoryListener {
    /**
     * Logger instance.
     */
    static final Logger LOG = LoggerFactory.getLogger(OVSDBPluginEventHandler.class);

    /**
     * Interface object for CRUD of NB Port objects
     *
     */
    private INeutronPortCRUD neutronPortCRUD;

    /**
     *
     * ovsdbConfigService object to utlilize the add,update and delete rows
     */
    private OVSDBConfigService ovsdbConfigService;

    /**
     * To connect to the given Node
     */
    private IConnectionServiceInternal connectionService;

    /**
     * identifier for neutrom external-id
     */
    private static final String KEY_OF_NEUTRON_PORT_IN_EXT_IDS = "iface-id";

    /**
     * VTN identifiers in neutron port object.
     */
    private static final int VTN_IDENTIFIERS_IN_PORT = 3;

    private String integrationBridgeName;
    private String failmode;
    private String protocols;
    private String portname;
    private static String DEFAULT_INTEGRATION_BRIDGENAME = "br-int";
    private static String DEFAULT_FAILMDOE="secure";
    private static String DEFAULT_PROTOCOLS="OpenFlow10";
    private static String DEFAULT_PORTNAME="ens33";
    private static String CONFIG_INTEGRATION_BRIDGENAME = "integration_bridge";
    private static String CONFIG_FAILMODE ="fail_mode";
    private static String CONFIG_PROTOCOLS ="protocols";
    private static String CONFIG_PORTNAME ="portname";

    /**
     * Setter for neutron port service
     * @param service the { #service} to set
     */
    void setNeutronPortCRUD(INeutronPortCRUD service) {
        LOG.trace("Set INeutronPortCRUD: {}", service);
        this.neutronPortCRUD = service;
    }

    /**
     * Unset/remove the neutron port service
     * @param service the { #service} to set
     */
    void unsetNeutronPortCRUD(INeutronPortCRUD service) {
        if (this.neutronPortCRUD == service) {
            LOG.trace("Unset INeutronPortCRUD: {}", service);
            this.neutronPortCRUD = null;
        }
    }

    /**
     * Setter for OVSDBConfigservice
     */
    void setOVSDBConfigService(OVSDBConfigService service) {
        LOG.trace("Set OVSDBConfigService: {}", service);
        this.ovsdbConfigService = service;
    }

    /**
     * Unset/remove OVSDBConfigservice
     */
    void unsetOVSDBConfigService(OVSDBConfigService service) {
        if (this.ovsdbConfigService == service) {
            LOG.trace("Unset OVSDBConfigService: {}", service);
            this.ovsdbConfigService = null;
        }
    }

    /**
     * Setter for connection service
     */
    public void setConnectionService(IConnectionServiceInternal service) {
        LOG.trace("Set ConnectionServiceInternal: {}", service);
        this.connectionService = service;
    }

    /**
     * unset/remove for connection service
     */
    public void unsetConnectionService(IConnectionServiceInternal service) {
        if (service == this.connectionService) {
            LOG.trace("Unset ConnectionServiceInternal: {}", service);
            this.connectionService = null;
        }
    }

    // TODO: In VTN should a event handler thread be created similar to OVSDB
    // Need to discuss

    /**
     * while the node is Added
     */
    @Override
    public void nodeAdded(Node node) {
        // TBD
        LOG.trace("nodeAdded() - {}", node.toString());
        processNodeUpdate(node);
    }

    /**
     * while the Node is removed
     */
    @Override
    public void nodeRemoved(Node node) {
        // TBD
        LOG.trace("nodeRemoved() - {}", node.toString());
    }


    /**
     * Application calls method while the interface is added
     */
    @Override
    public void rowAdded(Node node, String tableName, String uuid, Table<?> row) {
        LOG.trace("rowAdded() - {}", node.toString());
        logRowEventInfo(tableName, uuid, row);
        /*
        * If OF port is received in Add event set port map accordingly
        */
        // processRowUpdated(node, tableName, uuid, row);
    }

    /**
     * Application calls the while the existing interface details are updated
     */
    @Override
    public void rowUpdated(Node node, String tableName, String uuid, Table<?> oldRow, Table<?> newRow) {
        LOG.trace("rowUpdated() - {}", node.toString());
        logRowEventInfo(tableName, uuid, newRow);
        if (this.isUpdateOfInterest(oldRow, newRow)) {
            processRowUpdated(node, tableName, uuid, newRow);
        }
    }

    /**
     * Application calls method while the interface is removed
     */
    @Override
    public void rowRemoved(Node node, String tableName, String uuid, Table<?> row) {
        // TBD
        LOG.trace("rowRemoved() - {}", node.toString());
        logRowEventInfo(tableName, uuid, row);
        processRowRemoved(node, tableName, uuid, row);
    }

    private boolean isUpdateOfInterest(Table<?> oldRow, Table<?> newRow) {
        if (oldRow == null) {
            return true;
        }
        if (newRow.getTableName().equals(Interface.NAME)) {
            // We are NOT interested in Stats only updates
            Interface oldIntf = (Interface)oldRow;
            if (oldIntf.getName() == null && oldIntf.getExternal_ids() == null && oldIntf.getMac() == null &&
                oldIntf.getOfport() == null && oldIntf.getOptions() == null && oldIntf.getOther_config() == null &&
                oldIntf.getType() == null) {
                LOG.trace("IGNORING Interface Update : "+newRow.toString());
                return false;
            }
        }
        return true;
    }

    /**
    *
    * @param node
    * @param tableName
    * @param uuid
    * @param row
    * @param action
    */
    private void processRowUpdated(Node node, String tableName, String uuid, Table<?> row) {
        if (!Interface.NAME.getName().equalsIgnoreCase(tableName)) {
            LOG.trace(" Not an interface event ");
            return;
        }

        Interface intf = (Interface) row;

        NeutronPort neutronPort = getNeutronPortFormInterface(intf);
        if (neutronPort == null) {
            LOG.trace(" No neutron port associated with interface {} ", intf);
            return;
        }

        String switchId = getSwitchIdFromInterface(node, uuid);
        if (switchId == null) {
            LOG.trace(" No switch is associated with interface {} ", uuid);
            return;
        }

        String ofPort = getOfPortFormInterface(intf);
        if (ofPort == null) {
            LOG.trace(" No OF port associated with interface {} ", intf);
            return;
        }
        int result = setPortMapForInterface(node, intf, neutronPort, switchId, ofPort);
        if (result != HttpURLConnection.HTTP_OK) {
            LOG.error(" Set Port mapping failed for interface {}", intf);
        }
        return;
    }

    /**
     * TODO: ADD COMMENTS!
     */
    private void processRowRemoved(Node node, String tableName, String uuid, Table<?> row) {
        if (!Interface.NAME.getName().equalsIgnoreCase(tableName)) {
            LOG.trace(" Not an interface event ");
            return;
        }
        Interface intf = (Interface) row;

        NeutronPort neutronPort = getNeutronPortFormInterface(intf);
        if (neutronPort == null) {
            LOG.trace(" No neutron port associated with interface {} ", intf);
            return;
        }

        int result = deletePortMapForInterface(neutronPort);
        if (result != HttpURLConnection.HTTP_OK) {
            LOG.error(" Set Port mapping failed for interface {}", intf);
        }
        return;
    }

    /**
     * TODO: ADD COMMENTS!
     */
    private NeutronPort getNeutronPortFormInterface(Interface intf) {
        LOG.trace("getNeutronPortFormInterface for {}", intf);
        if (intf == null) {
            return null;
        }

        Map<String, String> externalIds = intf.getExternal_ids();
        if (externalIds == null) {
            return null;
        }

        LOG.trace("interface {}, externalIds {}", intf, externalIds);

        String neutronPortId = externalIds.get(KEY_OF_NEUTRON_PORT_IN_EXT_IDS);
        if (neutronPortId == null) {
            return null;
        }

        // Get Neutron Port object from NeutronPortCRUD service
        NeutronPort neutronPort = neutronPortCRUD.getPort(neutronPortId);
        LOG.trace("Neutron Port - {} associated with neutronPortId {}",
            neutronPort, neutronPortId);
        if (neutronPort == null) {
            return null;
        }

        return neutronPort;
    }

    private String getOfPortFormInterface(Interface intf) {
        LOG.trace("getOFPortFormInterface for {}", intf);
        if (intf == null) {
            return null;
        }

        Set<BigInteger> setBigInt = intf.getOfport();
        if (setBigInt == null || setBigInt.isEmpty()) {
            LOG.trace(" No OF Ports configured for interface {} ", intf);
            return null;
        }
        LOG.trace( " OF Port for interface {} ", setBigInt.toArray()[0].toString());
        return setBigInt.toArray()[0].toString();
    }

    /**
     * TODO: ADD COMMENTS!
     */
    private String getSwitchIdFromInterface(Node node, String interfaceUuid) {
        if ((node == null) || (interfaceUuid == null)) {
            return null;
        }
        try {
            Map<String, Table<?>> map = ovsdbConfigService.getRows(node, "Bridge");
            for(Table<?> row: map.values()) {
                Bridge br = (Bridge) row;
                Set<UUID> ports = br.getPorts();
                for(UUID portId: ports) {
                    Port port = (Port)ovsdbConfigService.getRow(node, "Port", portId.toString());
                    if(port == null) {
                        continue;
                    }
                    Set<UUID> interfaces = port.getInterfaces();
                    for(UUID intfIds: interfaces) {
                        if (intfIds.toString().equals(interfaceUuid)) {
                            return getDataPathIdFromBridge(br);
                        }
                    }
                }
            }
            LOG.info("Failed to get switch Id related to the interface({}).", interfaceUuid);
        } catch (Exception e) {
            LOG.error(" Exception while getting switch identifier {}", e);
        }
        return null;
    }

    /**
     * TODO: ADD COMMENTS!
     */
    private String getDataPathIdFromBridge(Bridge bridge) {

        if (bridge == null) {
            return null;
        }
        Set<String> dpids = bridge.getDatapath_id();
        if (dpids == null || dpids.isEmpty() ) {
            return null;
        }
        LOG.trace(" datapath Id of Bridge - {} ", dpids.toArray()[0].toString());
        return dpids.toArray()[0].toString();
    }

    /**
     * TODO: ADD COMMENTS!
     */
    private int setPortMapForInterface(Node node,
                                       Interface intf,
                                       NeutronPort neutronPort,
                                       String switchId,
                                       String ofPort) {
        int result = HttpURLConnection.HTTP_BAD_REQUEST;
        short vlan = 0;
        LOG.trace(" switch ID - {}", switchId);
        Node switchNode = NodeCreator.createOFNode(Long.valueOf(switchId,16));
        SwitchPort switchPort = new SwitchPort(intf.getName(), "OF", ofPort);
        PortMapConfig portMapConfig = new PortMapConfig(switchNode, switchPort, vlan);
        String[] vtnIDs = new String[VTN_IDENTIFIERS_IN_PORT];
        result = getVTNIdentifiers(neutronPort, vtnIDs);
        if (result != HttpURLConnection.HTTP_OK) {
            LOG.error("processRowUpdated getVTNIdentifiers failed, result - {}",
                      result);
            return result;
        }
        String tenantID = vtnIDs[0];
        String bridgeID = vtnIDs[1];
        String portID = vtnIDs[2];

        VBridgeIfPath path = new VBridgeIfPath(tenantID, bridgeID, portID);
        Status status = getVTNManager().setPortMap(path, portMapConfig);
        if (status.isSuccess()) {
            LOG.info("PortMap Created Sucessfully.");
            result = HttpURLConnection.HTTP_OK;
        } else {
            LOG.error("Failed to create PortMap." + status.getDescription());
            result = getException(status);
        }
        return result;
    }

    /**
     * TODO: ADD COMMENTS!
     */
    private int deletePortMapForInterface(NeutronPort neutronPort) {
        int result = HttpURLConnection.HTTP_BAD_REQUEST;
        String[] vtnIDs = new String[VTN_IDENTIFIERS_IN_PORT];
        result = getVTNIdentifiers(neutronPort, vtnIDs);
        if (result != HttpURLConnection.HTTP_OK) {
            LOG.error("processRowUpdated getVTNIdentifiers failed, result - {}",
                      result);
            return result;
        }
        String tenantID = vtnIDs[0];
        String bridgeID = vtnIDs[1];
        String portID = vtnIDs[2];

        VBridgeIfPath path = new VBridgeIfPath(tenantID, bridgeID, portID);
        Status status = getVTNManager().setPortMap(path, null);
        if (status.isSuccess()) {
            LOG.info("PortMap deleted Sucessfully.");
            result = HttpURLConnection.HTTP_OK;
        } else {
            LOG.error("Failed to delete PortMap." + status.getDescription());
            result = getException(status);
        }
        return result;
    }

    /**
     * Validate and return VTN identifiers from the given neutron object.
     *
     * @param port   An instance of Port object.
     * @param vtnIDs VTN identifiers.
     * @return A HTTP status code to the creation request.
     */
    private int getVTNIdentifiers(NeutronPort port, String[] vtnIDs) {
        int result = HttpURLConnection.HTTP_BAD_REQUEST;
        /**
         * To basic validation of the request
         */
        if (port == null) {
            LOG.error("port object not specified");
            return result;
        }

        String tenantUUID = port.getTenantID();
        String bridgeUUID = port.getNetworkUUID();
        String portUUID = port.getID();

        if ((tenantUUID == null) || (bridgeUUID == null) || portUUID == null) {
            LOG.error("neutron identifiers not specified");
            return result;
        }

        String tenantID = convertNeutronIDToVTNKey(tenantUUID);
        if (tenantID == null) {
            LOG.error("Invalid tenant identifier");
            return result;
        }

        String bridgeID = convertNeutronIDToVTNKey(bridgeUUID);
        if (bridgeID == null) {
            LOG.error("Invalid bridge identifier");
            return result;
        }

        String portID = convertNeutronIDToVTNKey(portUUID);
        if (portID == null) {
            LOG.error("Invalid port identifier");
            return result;
        }

        vtnIDs[0] = tenantID;
        vtnIDs[1] = bridgeID;
        vtnIDs[2] = portID;

        return HttpURLConnection.HTTP_OK;
    }


    /**
     * TODO: ADD COMMENTS!
     */
    private void logRowEventInfo(String tableName, String uuid, Table<?> row) {
        if (!Interface.NAME.getName().equalsIgnoreCase(tableName) &&
            !Port.NAME.getName().equalsIgnoreCase(tableName) &&
            !Open_vSwitch.NAME.getName().equalsIgnoreCase(tableName)) {
            return;
        }
        LOG.trace("Event info - uuid: {}, table name: {}", uuid, tableName);
        LOG.trace("Row info: {}", row);
    }

    /**
     * To get the IntegrationBridgeName
     */
    public String getIntegrationBridgeName() {
        integrationBridgeName = System.getProperty(CONFIG_INTEGRATION_BRIDGENAME);
        if (integrationBridgeName == null) integrationBridgeName = DEFAULT_INTEGRATION_BRIDGENAME;
        LOG.trace("inside getIntegrationBridgeName: "+this.integrationBridgeName);
        return this.integrationBridgeName;
    }
    /**
     * To set the IntegrationBridgeName
     */

    public void setIntegrationBridgeName(String integrationBridgeName) {
        this.integrationBridgeName = integrationBridgeName;
    }
    /**
     * To get the Failmode
     */
    public String getFailmode() {
        failmode = System.getProperty(CONFIG_FAILMODE);
        if (failmode == null) failmode=DEFAULT_FAILMDOE;
             LOG.trace("inside getFailmode: "+this.failmode);
              return this.failmode;
    }
    /**
     * To set the Failmode
     */
    public void setFailmode(String failmode) {
        this.failmode = failmode;
    }
    /**
     * To get the Protocol
     */
    public String getProtocols() {
     protocols = System.getProperty(CONFIG_PROTOCOLS);
     if (protocols == null) protocols=DEFAULT_PROTOCOLS;
            LOG.trace("inside protocols: "+this.protocols);
          return this.protocols;
    }
    /**
     * To set the Protocol
     */
    public void setProtocols(String protocols) {
    this.protocols = protocols;
    }

    /**
     * To get the Portname
     */
    public String getPortName() {
     portname = System.getProperty(CONFIG_PORTNAME);
     if (portname == null) portname=DEFAULT_PORTNAME;
            LOG.trace("inside portname: "+this.portname);
          return this.portname;
    }
    /**
     * To set the Protocol
     */
    public void setPortName(String portname) {
    this.portname = portname;
    }
    /**
     *
     * @param node
     */
    public void processNodeUpdate(Node node) {
        LOG.trace("Process Node added { }", node);
        prepareInternalNetwork(node);
    }

    public void prepareInternalNetwork(Node node) {
        try {
            this.createInternalNetworkForNeutron(node);
            LOG.trace("Process internal neutron network { }", node);
        } catch (Exception e) {
              LOG.error("Error creating internal network"+node.toString(), e);
        }
        // TODO:
        //if Need initialize flow rules
    }

    /*
     * Lets create these if not already present :
     *
       Bridge br-int
            Port br-int
                Interface br-int
                    type: internal
     */
    public void createInternalNetworkForNeutron(Node node) throws Exception {
        LOG.trace("initializeInteBridgeName()");
        String brInt = getIntegrationBridgeName();
        LOG.trace("called getIntegrationBridgeName***brInt=="+brInt);
       Status status = this.addInternalBridge(node,brInt);
       LOG.trace("called addInternalBridge  status=="+status);
        if (!status.isSuccess()) {
           LOG.trace("Integration Bridge Creation Status : "+status.toString());
        }
    }

    private Status addInternalBridge (Node node, String bridgeName) throws Exception {
        LOG.trace( "node - {}, bridgeName - {}", node, bridgeName);
        String bridgeUUID = this.getInternalBridgeUUID(node, bridgeName);
        org.opendaylight.ovsdb.lib.table.Bridge bridge = new org.opendaylight.ovsdb.lib.table.Bridge();
        LOG.trace("set bridge name"+bridgeName);
        bridge.setName(bridgeName);
        LOG.trace("set failmode");
        String failMode_File = getFailmode();
        OvsDBSet<String> failMode = new OvsDBSet<String>();
        failMode.add(failMode_File);
        bridge.setFail_mode(failMode);
        LOG.trace("set protocols ");
        OvsDBSet<String> protocols = new OvsDBSet<String>();
        String protocol_file=getProtocols().toString();
        protocols.add(protocol_file);
        bridge.setProtocols(protocols);
    //    InetAddress ControllerIP = getControllerIPAddress();
    //    OvsDBSet<UUID> controllers = new OvsDBSet<UUID>();
    //    controllers.add(ControllerIP.toString());
    //    bridge.setController(controllers);
    //    bridge.getController();

    // TBD    bridge.controlleripAddress(127.0.0.1);

        if (bridgeUUID == null) {
            bridge.setName(bridgeName);
            LOG.debug("insertRow node= {}  Bridge.NAME.getName()={} bridge={}", node,Bridge.NAME.getName(),bridge);
            StatusWithUuid statusWithUuid = ovsdbConfigService.insertRow(node, Bridge.NAME.getName(), null, bridge);
            LOG.trace("*****Successfully inserted Bridge.NAME.getName()*****",statusWithUuid);
            if (!statusWithUuid.isSuccess()) return statusWithUuid;
            bridgeUUID = statusWithUuid.getUuid().toString();
            Port port = new Port();
            String portname = getPortName();
            port.setName(portname);
            LOG.trace("*****before insertROW  portname - {} ****** {}", portname, statusWithUuid);
            Status status = ovsdbConfigService.insertRow(node, Port.NAME.getName(), bridgeUUID, port);
            LOG.trace("addInternalBridge : Inserting Bridge {} with protocols {} and status {}", bridgeUUID, protocols, status);
        } else {
            Status status = ovsdbConfigService.updateRow(node, Bridge.NAME.getName(), null, bridgeUUID, bridge);
            LOG.trace("addInternalBridge : Updating Bridge {} with protocols {} and status {}", bridgeUUID, protocols, status);
        }

        connectionService.setOFController(node, bridgeUUID);

        return new Status(StatusCode.SUCCESS);
    }

    public String getInternalBridgeUUID (Node node, String bridgeName) {
        try {
            Map<String, Table<?>> bridgeTable = ovsdbConfigService.getRows(node, Bridge.NAME.getName());
            if (bridgeTable == null) return null;
            for (String key : bridgeTable.keySet()) {
                Bridge bridge = (Bridge)bridgeTable.get(key);
                if (bridge.getName().equals(bridgeName)) return key;
            }
        } catch (Exception e) {
            LOG.error("Error getting Bridge Identifier for {} / {}", node, bridgeName, e);
        }
        return null;
    }

    /**
     *
     *   This test method to get the controller ip address
     */
    private InetAddress getControllerIPAddress() {
        InetAddress controllerIP = null;
        String addressString = System.getProperty("controller.address");
        if (addressString != null) {
            try {
                controllerIP = InetAddress.getByName(addressString);
                if (controllerIP != null) {
                   LOG.trace("Host {} ip == ", addressString);
                    return controllerIP;
                }
            } catch (UnknownHostException e) {
                LOG.trace("Host {} is invalid", addressString);
            }
        }
        return controllerIP;
    }
}
