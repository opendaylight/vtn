/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron;

import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentMap;
import java.io.File;
import java.io.FileInputStream;
import java.util.Properties;
import java.net.HttpURLConnection;
import java.net.InetAddress;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.SwitchPort;
import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.PortMapConfig;

import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.utils.NodeCreator;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

import org.opendaylight.neutron.spi.INeutronPortCRUD;
import org.opendaylight.neutron.spi.NeutronPort;

import org.opendaylight.ovsdb.lib.notation.OvsdbSet;
import org.opendaylight.ovsdb.lib.notation.UUID;
import org.opendaylight.ovsdb.lib.notation.Row;
import org.opendaylight.ovsdb.schema.openvswitch.Interface;
import org.opendaylight.ovsdb.schema.openvswitch.Port;
import org.opendaylight.ovsdb.schema.openvswitch.OpenVSwitch;
import org.opendaylight.ovsdb.schema.openvswitch.Bridge;
import org.opendaylight.ovsdb.compatibility.plugin.api.OvsdbConfigurationService;
import org.opendaylight.ovsdb.compatibility.plugin.api.OvsdbConnectionService;
import org.opendaylight.ovsdb.compatibility.plugin.api.OvsdbInventoryListener;
import org.opendaylight.ovsdb.compatibility.plugin.api.StatusWithUuid;

/**
 * Handle events from OVSDB Southbound plugin.
 */
public class OVSDBPluginEventHandler extends VTNNeutronUtils implements OvsdbInventoryListener{
    /**
     * Logger instance.
     */
    static final Logger LOG = LoggerFactory.getLogger(OVSDBPluginEventHandler.class);

    /**
     * The name of the configuration directory.
     */
    private static final String  CONFIG_DIR = "configuration";

    /**
     * The name of the configuration file.
     */
    private static final String  VTN_INI_NAME = "vtn.ini";

    /**
     * Interface object for CRUD of NB Port objects.
     */
    private INeutronPortCRUD neutronPortCRUD;

    /**
     * OvsdbConfigurationService object to utlilize the add,update and delete rows.
     */
    private volatile OvsdbConfigurationService ovsdbConfigService;

    /**
     * To connect to the given Node.
     */
    private OvsdbConnectionService connectionService;

    /**
     * identifier for neutron external-id.
     */
    private static final String KEY_OF_NEUTRON_PORT_IN_EXT_IDS = "iface-id";

    /**
     * VTN identifiers in neutron port object.
     */
    private static final int VTN_IDENTIFIERS_IN_PORT = 3;

    /**
     * identifier for the integration bridge name.
     */
    private String integrationBridgeName;

    /**
     * identifier to define failmode of integration bridge.
     */
    private String failmode;

    /**
     * identifier to define protocol of integration bridge.
     */
    private String protocols;

     /**
     * identifier to define portname of the integration bridge.
     */
    private String portname;

    /**
     * Default integration bridge name.
     */
    private static final String DEFAULT_INTEGRATION_BRIDGENAME = "br-int";

    /**
     * identifier for Default failmode.
     */
    private static final String DEFAULT_FAILMODE = "secure";

    /**
     * identifier for Default protocol.
     */
    private static final String DEFAULT_PROTOCOLS = "OpenFlow13";

    /**
     * identifier for Default portname.
     */
    private static final String DEFAULT_PORTNAME = "ens33";

    /**
     * identifier to read for integration bridge name from config file.
     */
    private static final String CONFIG_INTEGRATION_BRIDGENAME = "bridgename";

    /**
     * identifier to read failmode from config file.
     */
    private static final String CONFIG_FAILMODE = "failmode";

    /**
     * identifier to read protocol from config file.
     */
    private static final String CONFIG_PROTOCOLS = "protocols";

    /**
     * identifier to read portname from config file.
     */
    private static final String CONFIG_PORTNAME = "portname";

    /**
     * identifier to represent radix for String while retrieving value from Long Type.
     */
    private static final int RADIX_FOR_STRING = 16;

    /**
     * Set neutron port service.
     * @param service An Instance of INeutronPortCRUD to be set.
     */
    void setNeutronPortCRUD(INeutronPortCRUD service) {
        LOG.trace("Set INeutronPortCRUD: {}", service);
        this.neutronPortCRUD = service;
    }

    /**
     * Unset the neutron port service.
     * @param service Instance of INeutronPortCRUD service to be unset.
     */
    void unsetNeutronPortCRUD(INeutronPortCRUD service) {
        if (this.neutronPortCRUD == service) {
            this.neutronPortCRUD = null;
        }
    }

    /**
     * Set ovsdb configuration service.
     * @param service Instance of OvsdbConfigurationService to be set.
     */
    void setOVSDBConfigService(OvsdbConfigurationService service) {
        this.ovsdbConfigService = service;
    }

    /**
     * Unset OvsdbConfigurationService.
     * @param service Instance of OvsdbConfigurationService to be unset.
     */
    void unsetOVSDBConfigService(OvsdbConfigurationService service) {
        if (this.ovsdbConfigService == service) {
            this.ovsdbConfigService = null;
        }
    }

    /**
     * Set connection service.
     * @param service Instance of OvsdbConnectionService to be set.
     */
    public void setConnectionService(OvsdbConnectionService service) {
        LOG.trace("Set ConnectionServiceInternal: {}", service);
        this.connectionService = service;
    }

    /**
     * unset for connection service.
     * @param service Instance of OvsdbConnectionService to be unset.
     */
    public void unsetConnectionService(OvsdbConnectionService service) {
        if (service == this.connectionService) {
            LOG.trace("Unset ConnectionServiceInternal: {}", service);
            this.connectionService = null;
        }
    }

    /**
     * Get the name of Integration Bridge.
     * @return the name of the integrationBridge.
     */
    public String getIntegrationBridgeName() {
        return this.integrationBridgeName;
    }

    /**
     * Set the Integration Bridge Name.
     * @param integrationBridgeName name of the IntegrationBridge to be set.
     */
    public void setIntegrationBridgeName(String integrationBridgeName) {
        this.integrationBridgeName = integrationBridgeName;
    }

    /**
     * Get the  Failmode for Integration bridge.
     * @return the value of Failmode.
     */
    public String getFailmode() {
        return this.failmode;
    }

    /**
     * Set the Failmode for Integration bridge.
     * @param failmode failmode for IntegrationBridge to be set.
     */
    public void setFailmode(String failmode) {
        this.failmode = failmode;
    }

    /**
     * Get the Protocol for Integration bridge.
     * @return the value of Protocol.
     */
    public String getProtocols() {
        return this.protocols;
    }

    /**
     * Set the protocol for Integration bridge.
     * @param  protocols the protocol value to be set.
     */
    public void setProtocols(String protocols) {
        this.protocols = protocols;
    }

    /**
     * Get port name for Integration bridge.
     * @return the value of Portname.
     */
    public String getPortName() {
        return this.portname;
    }

    /**
     * Set the port name for Integration bridge.
     * @param portname the portname value to be set.
     */
    public void setPortName(String portname) {
        this.portname = portname;
    }

    /**
     * Method invoked when the open flow switch is Added.
     * @param node Instance of Node object to be added.
     * @param address Instance of InetAddress.
     * @param port int value.
     */
    @Override
    public void nodeAdded(Node node, InetAddress address, int port) {
        LOG.trace("nodeAdded() - {}", node.toString());
        processNodeUpdate(node);
    }

    /**
     * Method invoked when the open flow switch is Removed.
     * @param node Instance of Node object to be removed.
     */
    @Override
    public void nodeRemoved(Node node) {
        LOG.trace("nodeRemoved() - {}", node.toString());
    }

    /**
     * Method invoked when the open flow switch properties are Added.
     * @param node Instance of Node to be added.
     * @param tableName name of the table.
     * @param uuid unique identifier value.
     * @param row Instance of row to be added.
     */
    @Override
    public void rowAdded(Node node, String tableName, String uuid, Row row) {
        LOG.trace("rowAdded() - {}", node.toString());
        logRowEventInfo(tableName, uuid, row);
    }

    /**
     * Method invoked when the existing interface details are updated.
     * @param node Instance of Node to be updated.
     * @param tableName  Name of the table.
     * @param uuid unique identifier value.
     * @param oldRow Instance of oldRow of the Table.
     * @param newRow Instance of newRow of the Table to be updated.
     */
    @Override
    public void rowUpdated(Node node, String tableName, String uuid, Row oldRow, Row newRow) {
        LOG.trace("rowUpdated() - {}", node.toString());
        logRowEventInfo(tableName, uuid, newRow);
        if (this.isUpdateOfInterest(node, oldRow, newRow)) {
            processRowUpdated(node, tableName, uuid, newRow);
        }
    }

    /**
     * Method invoked when the interface is removed.
     * @param node Instance of Node to be removed.
     * @param tableName Name of the table.
     * @param uuid unique identifier value.
     * @param row Instance of Table Row to be removed.
     * @param obj  Instance of Object.
     */
    @Override
    public void rowRemoved(Node node, String tableName, String uuid, Row row, Object obj) {
        LOG.trace("rowRemoved() - {}", node.toString());
        logRowEventInfo(tableName, uuid, row);
        processRowRemoved(node, tableName, uuid, row);
    }

    /**
     * Ignore unneccesary updates to be even considered for processing.
     * @param node A {@link Node} instance.
     * @param oldRow Instance of old Row Table.
     * @param newRow Instance of new Row Table.
     * @return true while the new row is added.
     */
    private boolean isUpdateOfInterest(Node node, Row oldRow, Row newRow) {
        if (oldRow == null) {
            return true;
        }
        if (newRow.getTableSchema().getName().equals(ovsdbConfigService.getTableName(node, Interface.class))) {
            // We are NOT interested in Stats only updates
            Interface oldIntf = ovsdbConfigService.getTypedRow(node, Interface.class, oldRow);
            if (oldIntf.getName() == null && oldIntf.getExternalIdsColumn() == null && oldIntf.getMacColumn() == null &&
                oldIntf.getOpenFlowPortColumn() == null && oldIntf.getOptionsColumn() == null && oldIntf.getOtherConfigColumn() == null &&
                oldIntf.getTypeColumn() == null) {
                LOG.trace("IGNORING Interface Update: node {}, row: {}", node, newRow);
                return false;
            }
        } else if (newRow.getTableSchema().getName().equals(ovsdbConfigService.getTableName(node, Port.class))) {
            // We are NOT interested in Stats only updates
            Port oldPort = ovsdbConfigService.getTypedRow(node, Port.class, oldRow);
            if (oldPort.getName() == null && oldPort.getExternalIdsColumn() == null && oldPort.getMacColumn() == null &&
                oldPort.getInterfacesColumn() == null && oldPort.getTagColumn() == null && oldPort.getTrunksColumn() == null) {
                LOG.trace("IGNORING Port Update: node {}, row: {}", node, newRow);
                return false;
            }
        } else if (newRow.getTableSchema().getName().equals(ovsdbConfigService.getTableName(node, OpenVSwitch.class))) {
            OpenVSwitch oldOpenvSwitch = ovsdbConfigService.getTypedRow(node, OpenVSwitch.class, oldRow);
            if (oldOpenvSwitch.getOtherConfigColumn() == null) {
                /* we are only interested in other_config field change */
                return false;
            }
        }
        return true;
    }

    /**
     * Create Port Mapping whan an interface is added to the switch.
     * @param node Instance of Node to be updated.
     * @param tableName name of the table.
     * @param uuid unique identifier value.
     * @param row Instance of table row to be updated.
     */
    private void processRowUpdated(Node node, String tableName, String uuid, Row row) {
        String intfName = ovsdbConfigService.getTableName(node, Interface.class);
        if (!intfName.equalsIgnoreCase(tableName)) {
            LOG.trace("Not an interface event ");
            return;
        }

        Interface intf =  ovsdbConfigService.getTypedRow(node, Interface.class, row);

        NeutronPort neutronPort = getNeutronPortFormInterface(intf);
        if (neutronPort == null) {
            LOG.error("No neutron port associated with interface {}", intf);
            return;
        }

        String switchId = getSwitchIdFromInterface(node, uuid);
        if (switchId == null) {
            LOG.error("No switch is associated with interface {}", uuid);
            return;
        }

        String ofPort = getOfPortFormInterface(intf);
        if (ofPort == null) {
            LOG.error("No OF port associated with interface {}", intf);
            return;
        }
        int result = setPortMapForInterface(intf, neutronPort, switchId, ofPort);
        if (result != HttpURLConnection.HTTP_OK) {
            LOG.error("Set Port mapping failed for interface {}", intf);
        }
        return;
    }

    /**
     * Remove Port Mapping when the interface is removed.
     * @param node Instance of Node to be removed.
     * @param tableName name of the table.
     * @param uuid unique identifier value.
     * @param row Instance of Table row to be removed.
     */
    private void processRowRemoved(Node node, String tableName, String uuid, Row<?> row) {
        String intfName = ovsdbConfigService.getTableName(node, Interface.class);
        LOG.trace("processRowRemoved() - {} , tableNmae {}, UUID{}, ROw{}, intfName{}", node.toString(), tableName, uuid , row.toString(), intfName);
        if (!intfName.equalsIgnoreCase(tableName)) {
            LOG.error("Not an interface event ");
            return;
        }
        Interface intf = ovsdbConfigService.getTypedRow(node, Interface.class, row);

        NeutronPort neutronPort = getNeutronPortFormInterface(intf);
        if (neutronPort == null) {
            LOG.error("No neutron port associated with interface {}", intf);
            return;
        }

        int result = deletePortMapForInterface(neutronPort);
        if (result != HttpURLConnection.HTTP_OK) {
            LOG.error("Set Port mapping failed for interface {}", intf);
        }
        return;
    }

    /**
     * Get NeutronPort for the given interface object.
     * @param intf Instance of interface to get Neutron port.
     * @return Instance of NeutronPort.
     */
    private NeutronPort getNeutronPortFormInterface(Interface intf) {
        LOG.trace("getNeutronPortFormInterface for {}", intf);
        if (intf == null) {
            LOG.error("Interface is Null");
            return null;
        }

        Map<String, String> externalIds = intf.getExternalIdsColumn().getData();
        if (externalIds == null) {
            LOG.error("External id for interface {} is null", intf);
            return null;
        }

        String neutronPortId = externalIds.get(KEY_OF_NEUTRON_PORT_IN_EXT_IDS);
        if (neutronPortId == null) {
            LOG.error("Neutron port id is null for external id{}", externalIds);
            return null;
        }

        // Get Neutron Port object from NeutronPortCRUD service
        NeutronPort neutronPort = neutronPortCRUD.getPort(neutronPortId);
        LOG.trace("Neutron Port - {} associated with neutronPortId {}",
            neutronPort, neutronPortId);
        if (neutronPort == null) {
            LOG.error("Neutron Port is Null");
            return null;
        }

        return neutronPort;
    }

    /**
     * Get OF PortFormInterface for the given interface object.
     * @param intf Instance of interface to get OF Port From Interface.
     * @return OF port Form interface.
     */
    private String getOfPortFormInterface(Interface intf) {
        LOG.trace("getOFPortFormInterface for {}", intf);
        if (intf == null) {
            LOG.error("Interface is null");
            return null;
        }

        Set<Long> portSet = intf.getOpenFlowPortColumn().getData();
        if (portSet == null || portSet.isEmpty()) {
            LOG.error("No OF Ports configured for interface {}", intf);
            return null;
        }

        String port = portSet.toArray()[0].toString();
        LOG.trace("OF Port for interface {}", port);
        return port;
    }

    /**
     * Get SwitchId From Interface for the given node and interface uuid.
     * @param node Instance of Node to get switch id.
     * @param interfaceUuid unique identifier value.
     * @return switch Id.
     */
    private String getSwitchIdFromInterface(Node node, String interfaceUuid) {
        if ((node == null) || (interfaceUuid == null)) {
            LOG.error("Node or interface Uuid is Null");
            return null;
        }
        try {
            ConcurrentMap<String, Row> map = ovsdbConfigService.getRows(node, ovsdbConfigService.getTableName(node, Bridge.class));
            for (Row row: map.values()) {
                Bridge br = ovsdbConfigService.getTypedRow(node, Bridge.class, row);
                Set<UUID> ports = br.getPortsColumn().getData();
                for (UUID portId: ports) {
                    Port port = ovsdbConfigService.getTypedRow(node, Port.class, ovsdbConfigService.getRow(node, "Port", portId.toString()));
                    if (port == null) {
                        LOG.error("Port is Null");
                        continue;
                    }
                    if (LOG.isTraceEnabled()) {
                        LOG.trace("Node or Port getRow ={}", port.getRow());
                    }
                    Set<UUID> interfaces = port.getInterfacesColumn().getData();
                    for (UUID intfIds: interfaces) {
                        if (intfIds.toString().equals(interfaceUuid)) {
                            return getDataPathIdFromBridge(br);
                        }
                    }
                }
            }
            LOG.error("Failed to get switch Id related to the interface({}).", interfaceUuid);
        } catch (Exception e) {
            LOG.error("Exception while getting switch identifier", e);
        }
        return null;
    }

    /**
     * Get the DataPathId for a Bridge.
     * @param bridge An instance of bridge.
     * @return datapath id from Bridge.
     */
    private String getDataPathIdFromBridge(Bridge bridge) {
        if (bridge == null) {
            LOG.error("getDataPathIdFromBridge , bridge is Null");
            return null;
        }
        Set<String> dpids = bridge.getDatapathIdColumn().getData();
        if (dpids == null || dpids.isEmpty()) {
            LOG.error("data pathid is empty for bridge {}", bridge);
            return null;
        }

        String dpid = dpids.toArray()[0].toString();
        LOG.trace("datapath Id of Bridge - {} is {}", bridge, dpid);
        return dpid;
    }

    /**
     * Set PortMap for an Interface.
     *
     * @param intf An instance of Interface object.
     * @param neutronPort An instance of NeutronPort object.
     * @param switchId the switch id .
     * @param ofPort ofport value .
     * @return A HTTP status code to the PortMap request.
     */
    private int setPortMapForInterface(Interface intf,
                                       NeutronPort neutronPort,
                                       String switchId,
                                       String ofPort) {
        int result = HttpURLConnection.HTTP_BAD_REQUEST;
        short vlan = 0;
        LOG.trace("switch ID - {}", switchId);
        Node switchNode = NodeCreator.createOFNode(Long.valueOf(switchId, RADIX_FOR_STRING));
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
            LOG.trace("PortMap Created Sucessfully.");
            result = HttpURLConnection.HTTP_OK;
        } else {
            LOG.error("Failed to create PortMap. {}", status.getDescription());
            result = getException(status);
        }
        return result;
    }

    /**
     * Delete PortMap set for a NeutronPort.
     * @param neutronPort An instance of NeutronPort object.
     * @return A HTTP status code for delete PortMap request.
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
            LOG.trace("PortMap deleted Sucessfully.");
            result = HttpURLConnection.HTTP_OK;
        } else {
            LOG.error("Failed to delete PortMap. {}", status.getDescription());
            result = getException(status);
        }
        return result;
    }

    /**
     * Validate and Return VTN identifiers for the given NeutronPort object.
     * @param port An instance of Port object.
     * @param vtnIDs VTN identifiers.
     * @return A HTTP status code to the validation request.
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
     * Method to log the details of event information.
     * @param tableName name of the table.
     * @param uuid unique identifier value.
     * @param row Instance of table row.
     */
    private void logRowEventInfo(String tableName, String uuid, Row row) {
        LOG.trace("Event info - uuid: {}, table name: {}", uuid, tableName);
        LOG.trace("Row info: {}", row);
    }

    /**
     * Invoked when nodeAdded method is called.
     * @param node Instance of Node to be updated.
     */
    public void processNodeUpdate(Node node) {
        LOG.trace("processNodeUpdate() - node {}", node.toString());
        prepareInternalNetwork(node);
    }

    /**
     * Prepares the Internal Network for the node instance specified.
     * @param node Instance of Node Object.
     */
    public void prepareInternalNetwork(Node node) {
        try {
            this.createInternalNetworkForNeutron(node);
            LOG.trace("Process internal neutron network ,node {}", node);
        } catch (RuntimeException e) {
            LOG.error("Error while creating internal network for node " +  node,
                      e);
        }
    }

    /**
     * Create InternalNetwork if not already present.
     * @param node Instance of Node object.
     * @throws Exception while Neutron network create.
     */
    public void createInternalNetworkForNeutron(Node node) {
        getSystemProperties();
        String brInt = getIntegrationBridgeName();
        LOG.trace("createInternalNetworkForNeutron() - node ={}, integration bridge ={}", node.toString(), brInt);
        Status status = this.addInternalBridge(node, brInt);
        if (!status.isSuccess()) {
            LOG.trace("Integration Bridge Creation Status ={}", status.toString());
        }
    }

    /**
     * Read the paramaters configured in configuration file(vtn.ini).
     */
    public void getSystemProperties() {
        File f = new File(new File(CONFIG_DIR), VTN_INI_NAME);
        FileInputStream in = null;
        Properties prop = new Properties();
        try {
            in = new FileInputStream(f);
            prop.load(in);
            LOG.trace("loaded Integration bridge Configuartion details from vtn.ini");
        } catch (Exception e) {
            LOG.trace("Exception occured while reading SystemProperties", e);
        } finally {
            if (in != null) {
                try {
                    in.close();
                } catch (Exception e) {
                    LOG.warn("Exception occurred while closing file", e);
                }
                in = null;
            }
        }

        integrationBridgeName =
            prop.getProperty(CONFIG_INTEGRATION_BRIDGENAME,
                             DEFAULT_INTEGRATION_BRIDGENAME);
        failmode = prop.getProperty(CONFIG_FAILMODE, DEFAULT_FAILMODE);
        protocols = prop.getProperty(CONFIG_PROTOCOLS, DEFAULT_PROTOCOLS);
        portname = prop.getProperty(CONFIG_PORTNAME, DEFAULT_PORTNAME);
        LOG.trace("System properties values : {},{},{},{}:",
                  integrationBridgeName, failmode, protocols, portname);
    }

    /**
     * To add InternalBridge for the given Node object and bridgeName.
     * @param node Instance of Node object.
     * @param bridgeName name of the bridge.
     * @return A StatusCode from the defined enumeration list.
     */
    private Status addInternalBridge(Node node, String bridgeName) {
        LOG.trace("Added InternalBridge  bridgeName - {}, for the node - {}",
                  bridgeName, node);
        String bridgeUUID = this.getInternalBridgeUUID(node, bridgeName);
        Bridge bridgeobj = ovsdbConfigService.createTypedRow(node, Bridge.class);
        bridgeobj.setName(bridgeName);
        String failModeFile = getFailmode();
        OvsdbSet<String> failModeSet = new OvsdbSet<String>();
        failModeSet.add(failModeFile);
        bridgeobj.setFailMode(failModeSet);
        OvsdbSet<String> protocolset = new OvsdbSet<String>();
        String protocolfile = getProtocols();
        protocolset.add(protocolfile);
        bridgeobj.setProtocols(protocolset);
        if (bridgeUUID == null) {
            bridgeobj.setName(bridgeName);
            StatusWithUuid statusWithUuid = ovsdbConfigService.insertRow(node, bridgeobj.getSchema().getName(), null, bridgeobj.getRow());
            LOG.trace("Successfully inserted Bridge.NAME.getName() {} statusWithUuid ={}", bridgeobj.getSchema().getName(), statusWithUuid);
            if (!statusWithUuid.isSuccess()) {
                return statusWithUuid;
            }
            bridgeUUID = statusWithUuid.getUuid().toString();

            Port port = ovsdbConfigService.createTypedRow(node, Port.class);
            String portnamefile = getPortName();
            port.setName(portnamefile);
            String portName = port.getSchema().getName();
            Status status = ovsdbConfigService.insertRow(node, portName, bridgeUUID, port.getRow());
            LOG.trace("addInternalBridge Port : Inserting Bridge ={} with Table name ={} and status ={}", bridgeUUID, port.getSchema().getName(), status);

        } else {
            Status status = ovsdbConfigService.updateRow(node, bridgeobj.getSchema().getName(), null, bridgeUUID, bridgeobj.getRow());
            LOG.trace("addInternalBridge : Updating Bridge {} with protocols {} and status {}", bridgeUUID, protocols, status);
        }
        return new Status(StatusCode.SUCCESS);
    }

    /**
     * To get the Universally unique identifier for the bridge from the given Node object and bridgeName.
     * @param node Instance of Node.
     * @param bridgeName name of the bridge.
     * @return internal bridge unique identifier.
     */
    public String getInternalBridgeUUID(Node node, String bridgeName) {
        try {
            Map<String, Row> bridgeTable = ovsdbConfigService.getRows(node, ovsdbConfigService.getTableName(node, Bridge.class));
            if (bridgeTable == null) {
                return null;
            }
            for (Map.Entry<String, Row> entry: bridgeTable.entrySet()) {
                Row row = entry.getValue();
                Bridge bridge =
                    ovsdbConfigService.getTypedRow(node, Bridge.class, row);
                if (bridge.getName().equals(bridgeName)) {
                    return entry.getKey();
                }
            }
        } catch (Exception e) {
            LOG.error("Error while getting Bridge Identifier for " + node +
                      " / " + bridgeName, e);
        }
        return null;
    }
}
