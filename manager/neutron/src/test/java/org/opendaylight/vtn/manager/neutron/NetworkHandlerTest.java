/**
 * Copyright (c) 2015 NEC Corporation and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron;

import java.util.List;
import java.util.ArrayList;
import java.util.Set;

import java.net.HttpURLConnection;
import java.net.InetAddress;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mockito;

import org.opendaylight.vtn.manager.DataLinkHost;
import org.opendaylight.vtn.manager.IVTNManager;
import org.opendaylight.vtn.manager.MacAddressEntry;
import org.opendaylight.vtn.manager.MacMap;
import org.opendaylight.vtn.manager.MacMapConfig;
import org.opendaylight.vtn.manager.PathMap;
import org.opendaylight.vtn.manager.PathPolicy;
import org.opendaylight.vtn.manager.PortLocation;
import org.opendaylight.vtn.manager.PortMap;
import org.opendaylight.vtn.manager.PortMapConfig;
import org.opendaylight.vtn.manager.VBridge;
import org.opendaylight.vtn.manager.VBridgeConfig;
import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VInterface;
import org.opendaylight.vtn.manager.VInterfaceConfig;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.VTenant;
import org.opendaylight.vtn.manager.VTenantConfig;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.VTerminal;
import org.opendaylight.vtn.manager.VTerminalConfig;
import org.opendaylight.vtn.manager.VTerminalPath;
import org.opendaylight.vtn.manager.VTerminalIfPath;
import org.opendaylight.vtn.manager.VlanMap;
import org.opendaylight.vtn.manager.VlanMapConfig;
import org.opendaylight.vtn.manager.flow.DataFlow;
import org.opendaylight.vtn.manager.flow.DataFlowFilter;
import org.opendaylight.vtn.manager.flow.cond.FlowCondition;
import org.opendaylight.vtn.manager.flow.cond.FlowMatch;
import org.opendaylight.vtn.manager.flow.filter.FlowFilter;
import org.opendaylight.vtn.manager.flow.filter.FlowFilterId;

import org.opendaylight.controller.hosttracker.hostAware.HostNodeConnector;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.packet.address.DataLinkAddress;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.networks.rev150712.networks.attributes.networks.Network;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.networks.rev150712.networks.attributes.networks.NetworkBuilder;
import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev130715.Uuid;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.DataFlowMode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnAclType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateOperationType;

/**
 * JUnit test for {@link NetworkHandler}
 */
public class NetworkHandlerTest extends TestBase {

    /**
     * String identifier to have default VBridge description.
     */
    final String defaultVBridgeDescription = "Bridge";

    /**
     * String identifier to have default Network description.
     */
    final String defaultNetworkDescription = "Network";

    /**
     * NetworkBuilder instance.
     */
    private NetworkBuilder networkBuilder;

    /**
     * NetworkHandler instance.
     */
    private NetworkHandler networkHandler;

    /**
     * Status instance.
     */
    private Status status;

    /**
     * integer values used in mocking the functionality in unit testing.
     */
    private int hitCount, statusCodeIndex;

    @Before
    public void setup() throws Exception {
        networkHandler = new NetworkHandler() {
            @Override
            public IVTNManager getVTNManager() {
                IVTNManager ivtnManager = Mockito.mock(IVTNManager.class);
                status = getStatus();
                Mockito.when(
                        ivtnManager.addTenant(Mockito.isA(VTenantPath.class),
                                (Mockito.isA(VTenantConfig.class)))).thenReturn(status);
                Mockito.when(
                        ivtnManager.removeTenant(Mockito.isA(VTenantPath.class))).thenReturn(status);
                Mockito.when(
                        ivtnManager.addBridge(Mockito.isA(VBridgePath.class),
                                (Mockito.isA(VBridgeConfig.class)))).thenReturn(status);
                Mockito.when(
                        ivtnManager.removeBridge(Mockito.isA(VBridgePath.class)))
                        .thenReturn(status);
                return ivtnManager;
            }
        };
    }

    private Status getStatus() {
        status = new Status(StatusCode.CREATED) {
            @Override
            public boolean isSuccess() {
                if (hitCount == 0) {
                    hitCount++;
                    return true;
                } else {
                    return false;
                }
            }
            @Override
            public StatusCode getCode() {
                if (statusCodeIndex != 3) {
                    statusCodeIndex++;
                    return StatusCode.CREATED;
                }
                return  (statusCodeIndex == 3) ?  StatusCode.NOTFOUND : (statusCodeIndex == 4) ?
                     StatusCode.CONFLICT : StatusCode.CREATED;
            }
        };
        return status;
    }

    /**
     * Test method for
     * {@link NetworkHandler#neutronNetworkCreated(Network)}.
     */
    @Test
    public void testNeutronNetworkCreated() {

        VTNManagerStubForNetworkHandler vtnManagerStubNH = VTNManagerStubForNetworkHandler.getInstance();
        networkHandler.setVTNManager(vtnManagerStubNH);
        Network network = null;

        // Success Case - Sending already existing Tenants
        networkBuilder = new NetworkBuilder();
        networkBuilder.setTenantId(new Uuid(vtnManagerStubNH.tenantUuid1));
        networkBuilder.setUuid(new Uuid(NEUTRON_UUID_ARRAY[0]));
        network = networkBuilder.build();
        networkHandler.neutronNetworkCreated(network);

        // Failure Case - CreateTenant() recieves HTTP_CONFLICT(CreateTenant failed for tenant)
        networkBuilder = new NetworkBuilder();
        networkBuilder.setTenantId(new Uuid(vtnManagerStubNH.tenantUuid3));
        networkBuilder.setUuid(new Uuid(NEUTRON_UUID_ARRAY[1]));
        network = networkBuilder.build();
        networkHandler.neutronNetworkCreated(network);

        // Success Case - Sending the Tenant which does not exist in the VTN
        networkBuilder = new NetworkBuilder();
        networkBuilder.setTenantId(new Uuid(TENANT_ID_ARRAY[2]));
        networkBuilder.setUuid(new Uuid(NEUTRON_UUID_ARRAY[2]));
        network = networkBuilder.build();
        networkHandler.neutronNetworkCreated(network);

        // Failure Case - Failed to create network(BAD_REQUEST from isTenantExist())
        networkBuilder = new NetworkBuilder();
        networkBuilder.setTenantId(new Uuid(vtnManagerStubNH.tenantUuid2));
        networkBuilder.setUuid(new Uuid(NEUTRON_UUID_ARRAY[3]));
        network = networkBuilder.build();
        networkHandler.neutronNetworkCreated(network);

        // Success Case - Sending already existing Tenants
        networkBuilder = new NetworkBuilder();
        networkBuilder.setTenantId(new Uuid(vtnManagerStubNH.tenantUuid1));
        networkBuilder.setUuid(new Uuid(vtnManagerStubNH.bridgeUuid2));
        networkBuilder.setName(defaultNetworkDescription);
        network = networkBuilder.build();
        networkHandler.neutronNetworkCreated(network);

        // Failure Case - CreateBridge receives BAD_REQUEST(Sending already existing Tenant which should not be deleted)
        networkBuilder = new NetworkBuilder();
        networkBuilder.setTenantId(new Uuid(vtnManagerStubNH.tenantUuid4));
        networkBuilder.setUuid(new Uuid(vtnManagerStubNH.bridgeUuid2));
        networkBuilder.setName(defaultNetworkDescription);
        network = networkBuilder.build();
        networkHandler.neutronNetworkCreated(network);

        // Failure Case - CreateBridge receives BAD_REQUEST(Created New Tenant which has to be deleted in deleteTenant())
        networkBuilder = new NetworkBuilder();
        networkBuilder.setTenantId(new Uuid(vtnManagerStubNH.tenantUuid5));
        networkBuilder.setUuid(new Uuid(vtnManagerStubNH.bridgeUuid2));
        networkBuilder.setName(defaultNetworkDescription);
        network = networkBuilder.build();
        networkHandler.neutronNetworkCreated(network);

        // Failure Case - DeleteTenant receives Success(HTTP_OK)
        networkBuilder = new NetworkBuilder();
        networkBuilder.setTenantId(new Uuid(TENANT_ID_ARRAY[1]));
        networkBuilder.setUuid(new Uuid(NEUTRON_UUID_ARRAY[1]));
        networkBuilder.setName(defaultNetworkDescription);
        network = networkBuilder.build();
        networkHandler.neutronNetworkCreated(network);

        // Success Case - CreateBridge receives Success(HTTP_OK)
        networkBuilder = new NetworkBuilder();
        networkBuilder.setTenantId(new Uuid(vtnManagerStubNH.tenantUuid1));
        networkBuilder.setUuid(new Uuid(vtnManagerStubNH.bridgeUuid4));
        networkBuilder.setName(defaultNetworkDescription);
        network = networkBuilder.build();
        networkHandler.neutronNetworkCreated(network);

        // Success Case
        networkBuilder = new NetworkBuilder();
        networkBuilder.setTenantId(new Uuid(TENANT_ID_ARRAY[0]));
        networkBuilder.setUuid(new Uuid(NEUTRON_UUID_ARRAY[0]));
        network = networkBuilder.build();
        networkHandler.neutronNetworkCreated(network);
    }

    /**
     * Test method for
     * {@link NetworkHandler#neutronNetworkUpdated(NeutronNetwork)}.
     */
    @Test
    public void testNeutronNetworkUpdated() {
        VTNManagerStubForNetworkHandler vtnManagerStubNH = VTNManagerStubForNetworkHandler.getInstance();
        networkHandler.setVTNManager(vtnManagerStubNH);
        Network network = null;

        // Failure Case - Getting HTTP_BAD_REQUEST from isBridgeExist().
        networkBuilder = new NetworkBuilder();
        networkBuilder.setTenantId(new Uuid(vtnManagerStubNH.tenantUuid2));
        networkBuilder.setUuid(new Uuid(vtnManagerStubNH.bridgeUuid2));
        network = networkBuilder.build();
        networkHandler.neutronNetworkUpdated(network);

        // Failure Case - Setting network.shared to TRUE and will get HTTP_NOT_ACCEPTABLE.
        networkBuilder = new NetworkBuilder();
        networkBuilder.setTenantId(new Uuid(vtnManagerStubNH.tenantUuid1));
        networkBuilder.setUuid(new Uuid(vtnManagerStubNH.bridgeUuid1));
        networkBuilder.setShared(true);
        network = networkBuilder.build();
        networkHandler.neutronNetworkUpdated(network);

        // Failure Case - canModifyBridge() getting failed.
        networkBuilder = new NetworkBuilder();
        networkBuilder.setTenantId(new Uuid(vtnManagerStubNH.tenantUuid1));
        networkBuilder.setUuid(new Uuid(vtnManagerStubNH.bridgeUuid1));
        network = networkBuilder.build();
        networkHandler.neutronNetworkUpdated(network);
    }

    /**
     * Test method for
     * {@link NetworkHandler#neutronNetworkDeleted(NeutronNetwork)}.
     */
    @Test
    public void testNeutronNetworkDeleted() {
        VTNManagerStubForNetworkHandler vtnManagerStubNH = VTNManagerStubForNetworkHandler.getInstance();
        networkHandler.setVTNManager(vtnManagerStubNH);
        Network network = null;

        // Success Case - isBridgeExist() returns HTTP_NOT_FOUND and deleteBridge() returns HTTP_OK.
        networkBuilder = new NetworkBuilder();
        networkBuilder.setTenantId(new Uuid(vtnManagerStubNH.tenantUuid1));
        networkBuilder.setUuid(new Uuid(vtnManagerStubNH.bridgeUuid1));
        network = networkBuilder.build();
        networkHandler.neutronNetworkDeleted(network);

        // Success Case - deleteBridge() returns HTTP_CONFLICT.
        networkBuilder = new NetworkBuilder();
        networkBuilder.setTenantId(new Uuid(vtnManagerStubNH.tenantUuid2));
        networkBuilder.setUuid(new Uuid(vtnManagerStubNH.bridgeUuid1));
        network = networkBuilder.build();
        networkHandler.neutronNetworkDeleted(network);

        // Failure Case - List is NULL and returns false from isVbridgesInTenant() and deleteTenant() returns HTTP_CONFLICT.
        vtnManagerStubNH.returnNullForGetBridges = true;
        networkBuilder = new NetworkBuilder();
        networkBuilder.setTenantId(new Uuid(vtnManagerStubNH.tenantUuid3));
        networkBuilder.setUuid(new Uuid(vtnManagerStubNH.bridgeUuid1));
        network = networkBuilder.build();
        networkHandler.neutronNetworkDeleted(network);

        // Success Case - List is empty and returns false from isVbridgesInTenant() and deleteTenant() returns HTTP_OK.
        vtnManagerStubNH.returnNullForGetBridges = true;
        networkBuilder = new NetworkBuilder();
        networkBuilder.setTenantId(new Uuid(vtnManagerStubNH.tenantUuid1));
        networkBuilder.setUuid(new Uuid(vtnManagerStubNH.bridgeUuid1));
        network = networkBuilder.build();
        networkHandler.neutronNetworkDeleted(network);

        // Success Case - List has elements and returns true from isVbridgesInTenant().
        networkBuilder = new NetworkBuilder();
        networkBuilder.setTenantId(new Uuid(vtnManagerStubNH.tenantUuid1));
        networkBuilder.setUuid(new Uuid(vtnManagerStubNH.bridgeUuid1));
        network = networkBuilder.build();
        networkHandler.neutronNetworkDeleted(network);
    }

    /**
     * Test method for
     * {@link NetworkHandler#canDeleteNetwork(NeutronNetwork)}.
     */
    @Test
    public void testCanDeleteNetwork() {
        VTNManagerStubForNetworkHandler vtnManagerStubNH = VTNManagerStubForNetworkHandler.getInstance();
        networkHandler.setVTNManager(vtnManagerStubNH);
        Network network = null;

        // Failure Case - Getting HTTP_BAD_REQUEST from isBridgeExist().
        networkBuilder = new NetworkBuilder();
        networkBuilder.setTenantId(new Uuid(vtnManagerStubNH.tenantUuid2));
        networkBuilder.setUuid(new Uuid(vtnManagerStubNH.bridgeUuid2));
        network = networkBuilder.build();
        assertEquals(HttpURLConnection.HTTP_NOT_FOUND,
                     networkHandler.canDeleteNetwork(network));
    }

    /**
     * Test method for
     * {@link NetworkHandler#createTenant(String)}.
     */
    @Test
    public void testCreateTenant() {
        VTNManagerStubForNetworkHandler vtnManagerStubNH = VTNManagerStubForNetworkHandler.getInstance();
        networkHandler.setVTNManager(vtnManagerStubNH);
        Network network = null;

        // Case - CreateTenant receives HTTP_CONFLICT
        networkBuilder = new NetworkBuilder();
        networkBuilder.setTenantId(new Uuid(vtnManagerStubNH.tenantUuid2));
        networkBuilder.setUuid(new Uuid(NEUTRON_UUID_ARRAY[0]));
        network = networkBuilder.build();
        networkHandler.neutronNetworkCreated(network);

        // Case - CreateTenant receives Success(HTTP_OK)
        networkBuilder = new NetworkBuilder();
        networkBuilder.setTenantId(new Uuid(vtnManagerStubNH.tenantUuid1));
        networkBuilder.setUuid(new Uuid(NEUTRON_UUID_ARRAY[1]));
        network = networkBuilder.build();
        networkHandler.neutronNetworkCreated(network);
    }

    /**
     * Test method for
     * {@link NetworkHandler#deleteTenant(String)}.
     */
    @Test
    public void testDeleteTenant() {
        VTNManagerStubForNetworkHandler vtnManagerStubNH = VTNManagerStubForNetworkHandler.getInstance();
        networkHandler.setVTNManager(vtnManagerStubNH);
        Network network = null;

        // Case - DeleteTenant receives BAD_REQUEST
        networkBuilder = new NetworkBuilder();
        networkBuilder.setTenantId(new Uuid(vtnManagerStubNH.tenantUuid4));
        networkBuilder.setUuid(new Uuid(vtnManagerStubNH.bridgeUuid2));
        networkBuilder.setName(defaultNetworkDescription);
        network = networkBuilder.build();
        networkHandler.neutronNetworkCreated(network);

        // Case - DeleteTenant receives Success(HTTP_OK)
        networkBuilder = new NetworkBuilder();
        networkBuilder.setTenantId(new Uuid(vtnManagerStubNH.tenantUuid5));
        networkBuilder.setUuid(new Uuid(vtnManagerStubNH.bridgeUuid2));
        networkBuilder.setName(defaultNetworkDescription);
        network = networkBuilder.build();
        networkHandler.neutronNetworkCreated(network);
    }

    /**
     * Test method for
     * {@link NetworkHandler#createBridge(String, String, String)}.
     */
    @Test
    public void testCreateBridge() {
        VTNManagerStubForNetworkHandler vtnManagerStubNH = VTNManagerStubForNetworkHandler.getInstance();
        networkHandler.setVTNManager(vtnManagerStubNH);
        Network network = null;

        // Case - CreateBridge receives BAD_REQUEST
        networkBuilder = new NetworkBuilder();
        networkBuilder.setTenantId(new Uuid(vtnManagerStubNH.tenantUuid1));
        networkBuilder.setUuid(new Uuid(vtnManagerStubNH.bridgeUuid2));
        networkBuilder.setName(defaultNetworkDescription);
        network = networkBuilder.build();
        networkHandler.neutronNetworkCreated(network);

        // Case - CreateBridge receives Success(HTTP_OK)
        networkBuilder = new NetworkBuilder();
        networkBuilder.setTenantId(new Uuid(vtnManagerStubNH.tenantUuid1));
        networkBuilder.setUuid(new Uuid(NEUTRON_UUID_ARRAY[1]));
        networkBuilder.setName(defaultNetworkDescription);
        network = networkBuilder.build();
        networkHandler.neutronNetworkCreated(network);
    }

    /**
     * Test method for
     * {@link NetworkHandler#modifyBridge(String, String, String)}.
     */
    @Test
    public void testModifyBridge() {
        VTNManagerStubForNetworkHandler vtnManagerStubNH = VTNManagerStubForNetworkHandler.getInstance();
        networkHandler.setVTNManager(vtnManagerStubNH);
        Network network = null;

        // Case - Getting HTTP_CONFLICT
        networkBuilder = new NetworkBuilder();
        networkBuilder.setTenantId(new Uuid(vtnManagerStubNH.tenantUuid2));
        networkBuilder.setUuid(new Uuid(vtnManagerStubNH.bridgeUuid2));
        network = networkBuilder.build();
        networkHandler.neutronNetworkUpdated(network);

        // Case - Getting modified success
        networkBuilder = new NetworkBuilder();
        networkBuilder.setTenantId(new Uuid(vtnManagerStubNH.tenantUuid1));
        networkBuilder.setUuid(new Uuid(vtnManagerStubNH.bridgeUuid2));
        networkBuilder.setName(defaultNetworkDescription);
        network = networkBuilder.build();
        networkHandler.neutronNetworkUpdated(network);
    }

    /**
     * Test method for
     * {@link NetworkHandler#deleteBridge(String, String)}.
     */
    @Test
    public void testDeleteBridge() {
        VTNManagerStubForNetworkHandler vtnManagerStubNH = VTNManagerStubForNetworkHandler.getInstance();
        networkHandler.setVTNManager(vtnManagerStubNH);
        Network network = null;

        // Case - Failed in CreateVlanMap, Tenant not deleted(Tenant didnt create) and getting failure when deleting created Bridge.
        networkBuilder = new NetworkBuilder();
        networkBuilder.setTenantId(new Uuid(vtnManagerStubNH.tenantUuid1));
        networkBuilder.setUuid(new Uuid(vtnManagerStubNH.bridgeUuid4));
        networkBuilder.setName(defaultNetworkDescription);
        network = networkBuilder.build();
        networkHandler.neutronNetworkCreated(network);

        // Case - Failed in CreateVlanMap, getting failure when deleting created Tenant and successfully deleted created Bridge.
        networkBuilder = new NetworkBuilder();
        networkBuilder.setTenantId(new Uuid(vtnManagerStubNH.tenantUuid4));
        networkBuilder.setUuid(new Uuid(vtnManagerStubNH.bridgeUuid4));
        networkBuilder.setName(defaultNetworkDescription);
        network = networkBuilder.build();
        networkHandler.neutronNetworkCreated(network);
    }

    /**
     * Test method for
     * {@link NetworkHandler#isVbridgesInTenant(String)}.
     */
    @Test
    public void testIsVbridgesInTenant() {
        VTNManagerStubForNetworkHandler vtnManagerStubNH = VTNManagerStubForNetworkHandler.getInstance();
        networkHandler.setVTNManager(vtnManagerStubNH);
        Network network = null;

        // Case - List is NULL and returns false.
        vtnManagerStubNH.returnNullForGetBridges = true;
        networkBuilder = new NetworkBuilder();
        networkBuilder.setTenantId(new Uuid(vtnManagerStubNH.tenantUuid1));
        networkBuilder.setUuid(new Uuid(vtnManagerStubNH.bridgeUuid1));
        network = networkBuilder.build();
        networkHandler.neutronNetworkDeleted(network);

        // Case - List is empty and returns false.
        vtnManagerStubNH.returnEmptyForGetBridges = true;
        networkBuilder = new NetworkBuilder();
        networkBuilder.setTenantId(new Uuid(vtnManagerStubNH.tenantUuid1));
        networkBuilder.setUuid(new Uuid(vtnManagerStubNH.bridgeUuid1));
        network = networkBuilder.build();
        networkHandler.neutronNetworkDeleted(network);

        // Case - List has elements and returns true.
        networkBuilder = new NetworkBuilder();
        networkBuilder.setTenantId(new Uuid(vtnManagerStubNH.tenantUuid1));
        networkBuilder.setUuid(new Uuid(vtnManagerStubNH.bridgeUuid1));
        network = networkBuilder.build();
        networkHandler.neutronNetworkDeleted(network);
    }

    /**
     * This will make unwanted object eligible for garbage collection.
     */
    @After
    public void tearDown() {
        networkHandler = null;
    }
}

/**
 * Stub class only for Network Handler-unit test.
 *
 * This stub provides APIs implemented in
 * org.opendaylight.vtn.manager package.
 */
class VTNManagerStubForNetworkHandler implements IVTNManager {

    // VTNManagerStubForNetworkHandler object for NetworkHandler-UT.
    static VTNManagerStubForNetworkHandler vtnManagerStub = null;

    // Boolean identifier for throwing Exception in getTenants() method.
    boolean throwExceptionForGetTenants = false;

    // Boolean identifier for throwing Exception in getBridges() method.
    boolean throwExceptionForGetBridges = false;

    // Boolean identifier for throwing Exception in getVlanMap() method.
    boolean throwExceptionForGetVlanMap = false;

    // Boolean identifier for adding Bridge2 in getVlanMap() method.
    boolean addBridge2ForGetVlanMap = false;

    // Boolean identifier for returnig null in getBridges() method.
    boolean returnNullForGetBridges = false;

    // Boolean identifier for returnig empty in getBridges() method.
    boolean returnEmptyForGetBridges = false;

    // Integer identifier to have the default VlanID.
    final int defaultVlanId = 0;

    // Integer identifier to have the VlanID for HTTP_NOT_FOUND from list of added VlanMapIds.
    final int httpNotFoundForVlanId = 500;

    // Integer identifier to have the VlanID for HTTP_OK from list of added VlanMapIds.
    final int httpOkForVlanId = 600;

    // Integer identifier to have the VlanID for HTTP_BAD_REQUEST from list of added VlanMapIds.
    final int httpBadRequestForVlanId = 700;

    // Integer identifier to have the VlanID for adding VlanMap.
    int addVlanMapId = defaultVlanId;

    // String identifier to have default VBridge description.
    final String defaultVBridgeDescription = "Bridge";

    // An invalid UUID
    final String invalidUuid = "F6197D54-97A1-44D2-ABFB-6DFED030C30F-";

    // A tenant information which the stub class has.
    final String tenantUuid1 = "b9a13232-525e-4d8c-be21-cd65e3436035";
    final String tenantName1 = "b9a13232525e4d8cbe21cd65e3436035";
    final String tenantUuid2 = "4b99cbea-5fa7-450a-b40a-81929e40371e";
    final String tenantName2 = "4b99cbea5fa750ab40a81929e40371e";
    final String tenantUuid3 = "4b99cbea-5fa7-450a-b40a-81929e40371f";
    final String tenantName3 = "4b99cbea5fa750ab40a81929e40371f";
    final String tenantUuid4 = "4b99cbea-5fa7-450a-b40a-81929e403720";
    final String tenantName4 = "4b99cbea5fa750ab40a81929e403720";
    final String tenantUuid5 = "4b99cbea-5fa7-450a-b40a-81929e403721";
    final String tenantName5 = "4b99cbea5fa750ab40a81929e403721";

    // A vBridge information which the stub class has.
    final String bridgeUuid1 = "9CFF065F-44AC-47B9-9E81-5E51CA843307";
    final String bridgeName1 = "9CFF065F44AC7B99E815E51CA843307";
    final String bridgeUuid2 = "9CFF065F-44AC-47B9-9E81-5E51CA843308";
    final String bridgeName2 = "9CFF065F44AC7B99E815E51CA843308";
    final String bridgeUuid3 = "9CFF065F-44AC-47B9-9E81-5E51CA843309";
    final String bridgeName3 = "9CFF065F44AC7B99E815E51CA843309";
    final String bridgeUuid4 = "9CFF065F-44AC-47B9-9E81-5E51CA84330a";
    final String bridgeName4 = "9CFF065F44AC7B99E815E51CA84330a";

    // Following methods are used in UnitTest.
    private VTNManagerStubForNetworkHandler() {
    }

    public static synchronized VTNManagerStubForNetworkHandler getInstance() {
        if (vtnManagerStub == null) {
            vtnManagerStub = new VTNManagerStubForNetworkHandler();
        }
        return vtnManagerStub;
    }

    @Override
    public boolean isActive() {
        return true;
    }

    @Override
    public VTenant getTenant(VTenantPath path) throws VTNException {
        if (path ==  null) {
            Status status = new Status(StatusCode.BADREQUEST);
            throw new VTNException(status);
        }

        VTenantPath tenant1 = new VTenantPath(tenantName1);
        VTenantPath tenant2 = new VTenantPath(tenantUuid1);
        VTenantPath tenant3 = new VTenantPath(tenantName2);
        if (path.equals(tenant1)) {
            VTenantConfig conf = new VTenantConfig(null);
            VTenant tenant = new VTenant(tenantName1, conf);
            return tenant;
        } else if ((path.equals(tenant2)) || (path.equals(tenant3))) {
            Status status = new Status(StatusCode.BADREQUEST);
            throw new VTNException(status);
        }

        return null;
    }

    @Override
    public Status addTenant(VTenantPath path, VTenantConfig tconf) {
        if (path ==  null || tconf ==  null) {
            return new Status(StatusCode.BADREQUEST);
        }

        VTenantPath tenant1 = new VTenantPath(tenantName1);
        VTenantPath tenant2 = new VTenantPath(tenantName3);
        if ((path.equals(tenant1)) || (path.equals(tenant2))) {
            return new Status(StatusCode.CONFLICT);
        }

        return new Status(StatusCode.SUCCESS);
    }

    @Override
    public VBridge getBridge(VBridgePath path) throws VTNException {
        if (path ==  null) {
            Status status = new Status(StatusCode.BADREQUEST);
            throw new VTNException(status);
        }

        VBridgePath bridge1 = new VBridgePath(tenantName1,
                                              bridgeName1);

        VBridgePath bridge2 = new VBridgePath(tenantName2,
                                              bridgeName1);

        VBridgePath bridge3 = new VBridgePath(tenantName3,
                                              bridgeName1);

        VBridgePath bridge4 = new VBridgePath(tenantUuid1,
                                              bridgeName1);

        VBridgePath bridge5 = new VBridgePath(tenantName2,
                                              bridgeName2);
        if (path.equals(bridge1)) {
            VBridgeConfig bconf = new VBridgeConfig(null);
            VBridge bridge = new VBridge(bridgeName1,
                                         VnodeState.UNKNOWN,
                                         0,
                                         bconf);
            return bridge;
        } else if ((path.equals(bridge2)) || (path.equals(bridge3))) {
            VBridgeConfig bconf = new VBridgeConfig(defaultVBridgeDescription);
            VBridge bridge = new VBridge(bridgeName1,
                                         VnodeState.UNKNOWN,
                                         0,
                                         bconf);
            return bridge;
        } else if ((path.equals(bridge4)) || (path.equals(bridge5))) {
            Status status = new Status(StatusCode.BADREQUEST);
            throw new VTNException(status);
        }

        return null;
    }

    @Override
    public Status addBridge(VBridgePath path, VBridgeConfig bconf) {
        if (path ==  null || bconf ==  null) {
            return new Status(StatusCode.BADREQUEST);
        }

        VBridgePath bridge1 = new VBridgePath(tenantName1,
                                              bridgeName1);
        VBridgePath bridge2 = new VBridgePath(tenantName1,
                                              bridgeName2);
        VBridgePath bridge3 = new VBridgePath(tenantName4,
                                              bridgeName2);
        VBridgePath bridge4 = new VBridgePath(tenantName5,
                                              bridgeName2);
        if ((path.equals(bridge1)) || (path.equals(bridge2))
            || (path.equals(bridge3)) || (path.equals(bridge4))) {
            return new Status(StatusCode.CONFLICT);
        }

        return new Status(StatusCode.SUCCESS);
    }

    @Override
    public Status modifyBridge(VBridgePath path, VBridgeConfig bconf, boolean all) {
        VBridgePath bridge1 = new VBridgePath(tenantName2,
                                              bridgeName1);
        if (path.equals(bridge1)) {
            return new Status(StatusCode.CONFLICT);
        }

        return new Status(StatusCode.SUCCESS);
    }

    @Override
    public List<VTenant> getTenants() throws VTNException {
        if (throwExceptionForGetTenants) {
            throwExceptionForGetTenants = false;

            Status status = new Status(StatusCode.BADREQUEST);
            throw new VTNException(status);
        }

        List<VTenant> tenants = new ArrayList<VTenant>();
        // Tenant1
        VTenantConfig conf1 = new VTenantConfig(null);
        VTenant tenant1 = new VTenant(tenantName1, conf1);

        // Tenant2
        VTenantConfig conf2 = new VTenantConfig(null);
        VTenant tenant2 = new VTenant(tenantName2, conf2);

        tenants.add(tenant1);
        tenants.add(tenant2);

        return tenants;
    }

    @Override
    public List<VBridge> getBridges(VTenantPath path) throws VTNException {
        if (throwExceptionForGetBridges) {
            throwExceptionForGetBridges = false;

            Status status = new Status(StatusCode.BADREQUEST);
            throw new VTNException(status);
        }

        if (returnNullForGetBridges) {
            returnNullForGetBridges = false;
            return null;
        }

        List<VBridge> bridges = new ArrayList<VBridge>();

        if (returnEmptyForGetBridges) {
            returnEmptyForGetBridges = false;
            return bridges;
        }

        // Bridge1
        VBridgeConfig bconf1 = new VBridgeConfig(null);
        VBridge bridge1 = new VBridge(bridgeName1,
                                         VnodeState.UNKNOWN,
                                         0,
                                         bconf1);
        bridges.add(bridge1);

        // Bridge2
        if (addBridge2ForGetVlanMap) {
            VBridgeConfig bconf2 = new VBridgeConfig(null);
            VBridge bridge2 = new VBridge(bridgeName2,
                                             VnodeState.UNKNOWN,
                                             0,
                                             bconf2);
            bridges.add(bridge2);
            addBridge2ForGetVlanMap = false;
        }

        // Bridge3
        if (throwExceptionForGetVlanMap) {
            VBridgeConfig bconf3 = new VBridgeConfig(null);
            VBridge bridge3 = new VBridge(bridgeName3,
                                             VnodeState.UNKNOWN,
                                             0,
                                             bconf3);
            bridges.add(bridge3);
            throwExceptionForGetVlanMap = false;
        }
        return bridges;
    }

    @Override
    public VlanMap getVlanMap(VBridgePath path, VlanMapConfig vlconf) throws VTNException {
        VBridgePath bridge1 = new VBridgePath(tenantName1,
                                              bridgeName2);

        VBridgePath bridge2 = new VBridgePath(tenantName1,
                                              bridgeName3);
        if (path.equals(bridge1)) {
            VlanMap vlanMap = new VlanMap(Integer.toString(vlconf.getVlan()), vlconf.getNode(), vlconf.getVlan());

            return vlanMap;
        } else if (path.equals(bridge2)) {
            Status status = new Status(StatusCode.BADREQUEST);
            throw new VTNException(status);
        }

        return null;
    }

    @Override
    public Status removeTenant(VTenantPath path) {
        if (path ==  null) {
            return new Status(StatusCode.BADREQUEST);
        }

        VTenantPath tenant1 = new VTenantPath(tenantName3);
        VTenantPath tenant2 = new VTenantPath(tenantName4);
        if ((path.equals(tenant1)) || (path.equals(tenant2))) {
            return new Status(StatusCode.CONFLICT);
        }

        return new Status(StatusCode.SUCCESS);
    }

    @Override
    public Status removeBridge(VBridgePath path) {
        if (path ==  null) {
            return new Status(StatusCode.BADREQUEST);
        }

        VBridgePath bridge1 = new VBridgePath(tenantName1,
                                                bridgeName4);
        VBridgePath bridge2 = new VBridgePath(tenantName2,
                                                bridgeName1);
        if ((path.equals(bridge1)) || (path.equals(bridge2))) {
            return new Status(StatusCode.CONFLICT);
        }

        return new Status(StatusCode.SUCCESS);
    }

    @Override
    public VlanMap addVlanMap(VBridgePath path, VlanMapConfig vlconf) throws VTNException {
        VBridgePath bridge1 = new VBridgePath(tenantName1,
                                              bridgeName3);
        VBridgePath bridge2 = new VBridgePath(tenantName1,
                                              bridgeName4);
        VBridgePath bridge3 = new VBridgePath(tenantName4,
                                              bridgeName4);
        VBridgePath bridge4 = new VBridgePath(tenantName5,
                                              bridgeName4);
        if (path.equals(bridge1)) {
            VlanMap vlanMap = new VlanMap(Integer.toString(vlconf.getVlan()), vlconf.getNode(), vlconf.getVlan());

            return vlanMap;
        } else if ((path.equals(bridge2)) || (path.equals(bridge2))
                    || (path.equals(bridge3)) || (path.equals(bridge4))) {
            Status status = new Status(StatusCode.BADREQUEST);
            throw new VTNException(status);
        }

        return null;
    }

    @Override
    public List<VlanMap> getVlanMaps(VBridgePath path) throws VTNException {
        List<VlanMap> vlanMaps = new ArrayList<VlanMap>();

        VBridgePath bridge1 = new VBridgePath(tenantName2,
                                              bridgeName1);
        if (path.equals(bridge1)) {
            Status status = new Status(StatusCode.BADREQUEST);
            throw new VTNException(status);
        }

        switch(addVlanMapId) {
        case defaultVlanId :
            addVlanMapId = defaultVlanId;
            return vlanMaps;

        case httpNotFoundForVlanId :
            addVlanMapId = defaultVlanId;
            vlanMaps.add(new VlanMap(Integer.toString(httpNotFoundForVlanId), null, (short)httpNotFoundForVlanId));
            return vlanMaps;

        case httpOkForVlanId :
            addVlanMapId = defaultVlanId;
            vlanMaps.add(new VlanMap(Integer.toString(httpOkForVlanId), null, (short)httpOkForVlanId));
            return vlanMaps;

        case httpBadRequestForVlanId :
            addVlanMapId = defaultVlanId;
            vlanMaps.add(new VlanMap(Integer.toString(httpBadRequestForVlanId), null, (short)httpBadRequestForVlanId));
            return vlanMaps;

        default :
            return null;
        }
    }

    @Override
    public Status removeVlanMap(VBridgePath path, String mapId) {
        switch(Integer.parseInt(mapId)) {
        case httpNotFoundForVlanId :
            return new Status(StatusCode.NOTFOUND);

        case httpOkForVlanId :
            return new Status(StatusCode.SUCCESS);

        case httpBadRequestForVlanId :
            return new Status(StatusCode.BADREQUEST);

        default :
            return new Status(StatusCode.CONFLICT);
        }
    }

    // Following methods are Unused in UnitTest.
    @Override
    public VInterface getInterface(VBridgeIfPath path) throws VTNException {
        return null;
    }

    @Override
    public Status addInterface(VBridgeIfPath path, VInterfaceConfig iconf) {
        return null;
    }

    @Override
    public Status modifyInterface(VBridgeIfPath path, VInterfaceConfig iconf, boolean all) {
        return null;
    }

    @Override
    public Status removeInterface(VBridgeIfPath path) {
        return null;
    }

    @Override
    public Status setPortMap(VBridgeIfPath path, PortMapConfig pmconf) {
        return null;
    }

    @Override
    public Status modifyTenant(VTenantPath path, VTenantConfig tconf, boolean all) {
        return null;
    }

    @Override
    public List<VTerminal> getTerminals(VTenantPath path) throws VTNException {
        return null;
    }

    @Override
    public VTerminal getTerminal(VTerminalPath path) throws VTNException {
        return null;
    }

    @Override
    public Status addTerminal(VTerminalPath path, VTerminalConfig vtconf) {
        return null;
    }

    @Override
    public Status modifyTerminal(VTerminalPath path, VTerminalConfig vtconf, boolean all) {
        return null;
    }

    @Override
    public Status removeTerminal(VTerminalPath path) {
        return null;
    }

    @Override
    public List<VInterface> getInterfaces(VBridgePath path) throws VTNException {
        return null;
    }


    @Override
    public List<VInterface> getInterfaces(VTerminalPath path) throws VTNException {
        return null;
    }

    @Override
    public VInterface getInterface(VTerminalIfPath path) throws VTNException {
        return null;
    }

    @Override
    public Status addInterface(VTerminalIfPath path, VInterfaceConfig iconf) {
        return null;
    }

    @Override
    public Status modifyInterface(VTerminalIfPath path, VInterfaceConfig iconf, boolean all) {
        return null;
    }

    @Override
    public Status removeInterface(VTerminalIfPath path) {
        return null;
    }

    @Override
    public VlanMap getVlanMap(VBridgePath path, String mapId) throws VTNException {
        return null;
    }

    @Override
    public PortMap getPortMap(VBridgeIfPath path) throws VTNException {
        return null;
    }

    @Override
    public PortMap getPortMap(VTerminalIfPath path) throws VTNException {
        return null;
    }

    @Override
    public Status setPortMap(VTerminalIfPath path, PortMapConfig pmconf) {
        return null;
    }

    @Override
    public MacMap getMacMap(VBridgePath path) throws VTNException {
        return null;
    }

    @Override
    public Set<DataLinkHost> getMacMapConfig(VBridgePath path,
                                             VtnAclType aclType)
        throws VTNException {
        return null;
    }

    @Override
    public List<MacAddressEntry> getMacMappedHosts(VBridgePath path) throws VTNException {
        return null;
    }

    @Override
    public MacAddressEntry getMacMappedHost(VBridgePath path, DataLinkAddress addr) throws VTNException {
        return null;
    }

    @Override
    public UpdateType setMacMap(VBridgePath path, VtnUpdateOperationType op,
                                MacMapConfig mcconf) throws VTNException {
        return null;
    }

    @Override
    public UpdateType setMacMap(VBridgePath path, VtnUpdateOperationType op,
                                VtnAclType aclType,
                                Set<? extends DataLinkHost> dlhosts)
        throws VTNException {
        return null;
    }

    @Override
    public void findHost(InetAddress addr, Set<VBridgePath> pathSet) {
    }

    @Override
    public boolean probeHost(HostNodeConnector host) {
        return false;
    }

    @Override
    public List<MacAddressEntry> getMacEntries(VBridgePath path) throws VTNException {
        return null;
    }

    @Override
    public MacAddressEntry getMacEntry(VBridgePath path, DataLinkAddress addr) throws VTNException {
        return null;
    }

    @Override
    public MacAddressEntry removeMacEntry(VBridgePath path, DataLinkAddress addr) throws VTNException {
        return null;
    }

    @Override
    public Status flushMacEntries(VBridgePath path) {
        return null;
    }

    @Override
    public List<DataFlow> getDataFlows(VTenantPath path, DataFlowMode mode,
            DataFlowFilter filter, int interval) throws VTNException {
        return null;
    }

    @Override
    public DataFlow getDataFlow(VTenantPath path, long flowId,
                                DataFlowMode mode, int interval)
        throws VTNException {
        return null;
    }

    @Override
    public int getDataFlowCount(VTenantPath path) throws VTNException {
        return 0;
    }

    @Override
    public List<FlowCondition> getFlowConditions() throws VTNException {
        return null;
    }

    @Override
    public FlowCondition getFlowCondition(String name) throws VTNException {
        return null;
    }

    @Override
    public UpdateType setFlowCondition(String name, FlowCondition fcond) throws VTNException {
        return null;
    }

    @Override
    public Status removeFlowCondition(String name) {
        return null;
    }

    @Override
    public Status clearFlowCondition() {
        return null;
    }

    @Override
    public FlowMatch getFlowConditionMatch(String name, int index)
        throws VTNException {
        return null;
    }

    @Override
    public UpdateType setFlowConditionMatch(String name, int index, FlowMatch match) throws VTNException {
        return null;
    }

    @Override
    public Status removeFlowConditionMatch(String name, int index) {
        return null;
    }

    @Override
    public List<Integer> getPathPolicyIds() throws VTNException {
        return null;
    }

    @Override
    public PathPolicy getPathPolicy(int id) throws VTNException {
        return null;
    }

    @Override
    public UpdateType setPathPolicy(int id, PathPolicy policy) throws VTNException {
        return null;
    }

    @Override
    public Status removePathPolicy(int id) {
        return null;
    }

    @Override
    public Status clearPathPolicy() {
        return null;
    }

    @Override
    public long getPathPolicyDefaultCost(int id) throws VTNException {
        return 0;
    }

    @Override
    public boolean setPathPolicyDefaultCost(int id, long cost)
        throws VTNException {
        return false;
    }

    @Override
    public long getPathPolicyCost(int id, PortLocation ploc)
        throws VTNException {
        return 0;
    }

    @Override
    public UpdateType setPathPolicyCost(int id,
            PortLocation ploc, long cost) throws VTNException {
        return null;
    }

    @Override
    public Status removePathPolicyCost(int id, PortLocation ploc) {
        return null;
    }

    @Override
    public List<PathMap> getPathMaps() throws VTNException {
        return null;
    }

    @Override
    public PathMap getPathMap(int index) throws VTNException {
        return null;
    }

    @Override
    public UpdateType setPathMap(int index, PathMap pmap) throws VTNException {
        return null;
    }

    @Override
    public Status removePathMap(int index) {
        return null;
    }

    @Override
    public Status clearPathMap() {
        return null;
    }

    @Override
    public List<PathMap> getPathMaps(VTenantPath path) throws VTNException {
        return null;
    }

    @Override
    public PathMap getPathMap(VTenantPath path, int index) throws VTNException {
        return null;
    }

    @Override
    public UpdateType setPathMap(VTenantPath path, int index, PathMap pmap) throws VTNException {
        return null;
    }

    @Override
    public Status removePathMap(VTenantPath path, int index) {
        return null;
    }

    @Override
    public Status clearPathMap(VTenantPath path) {
        return null;
    }

    @Override
    public List<FlowFilter> getFlowFilters(FlowFilterId fid) throws VTNException {
        return null;
    }

    @Override
    public FlowFilter getFlowFilter(FlowFilterId fid, int index) throws VTNException {
        return null;
    }

    @Override
    public UpdateType setFlowFilter(FlowFilterId fid, int index, FlowFilter filter) throws VTNException {
        return null;
    }

    @Override
    public Status removeFlowFilter(FlowFilterId fid, int index) {
        return null;
    }

    @Override
    public Status clearFlowFilter(FlowFilterId fid) {
        return null;
    }
}
