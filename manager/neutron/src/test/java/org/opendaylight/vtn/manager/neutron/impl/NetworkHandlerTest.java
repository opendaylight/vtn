/**
 * Copyright (c) 2015 NEC Corporation and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron.impl;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.opendaylight.yangtools.yang.model.api.Status;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev130715.Uuid;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.networks.rev150712.networks.attributes.networks.Network;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.networks.rev150712.networks.attributes.networks.NetworkBuilder;

/**
 * JUnit test for {@link NetworkHandler}
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest({ NetworkHandler.class, VTNManagerService.class })
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
     * VTNManagerService instance.
     */
    private VTNManagerService  vtnManager;
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

    @Before
    public void setUp() throws Exception {
        vtnManager = PowerMockito.mock(VTNManagerService.class);
        networkHandler = PowerMockito.spy(new NetworkHandler(vtnManager));
    }

    /**
     * Test method for
     * {@link NetworkHandler#neutronNetworkCreated(Network)}.
     */
    @Test
    public void testNeutronNetworkCreated() {

        Network network = null;

        // Success Case - Sending already existing Tenants
        networkBuilder = new NetworkBuilder();
        networkBuilder.setTenantId(new Uuid(tenantUuid1));
        networkBuilder.setUuid(new Uuid(NEUTRON_UUID_ARRAY[0]));
        network = networkBuilder.build();
        networkHandler.neutronNetworkCreated(network);

        // Failure Case - CreateTenant() recieves HTTP_CONFLICT(CreateTenant failed for tenant)
        networkBuilder = new NetworkBuilder();
        networkBuilder.setTenantId(new Uuid(tenantUuid3));
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
        networkBuilder.setTenantId(new Uuid(tenantUuid2));
        networkBuilder.setUuid(new Uuid(NEUTRON_UUID_ARRAY[3]));
        network = networkBuilder.build();
        networkHandler.neutronNetworkCreated(network);

        // Success Case - Sending already existing Tenants
        networkBuilder = new NetworkBuilder();
        networkBuilder.setTenantId(new Uuid(tenantUuid1));
        networkBuilder.setUuid(new Uuid(bridgeUuid2));
        networkBuilder.setName(defaultNetworkDescription);
        network = networkBuilder.build();
        networkHandler.neutronNetworkCreated(network);

        // Failure Case - CreateBridge receives BAD_REQUEST(Sending already existing Tenant which should not be deleted)
        networkBuilder = new NetworkBuilder();
        networkBuilder.setTenantId(new Uuid(tenantUuid4));
        networkBuilder.setUuid(new Uuid(bridgeUuid2));
        networkBuilder.setName(defaultNetworkDescription);
        network = networkBuilder.build();
        networkHandler.neutronNetworkCreated(network);

        // Failure Case - CreateBridge receives BAD_REQUEST(Created New Tenant which has to be deleted in deleteTenant())
        networkBuilder = new NetworkBuilder();
        networkBuilder.setTenantId(new Uuid(tenantUuid5));
        networkBuilder.setUuid(new Uuid(bridgeUuid2));
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
        networkBuilder.setTenantId(new Uuid(tenantUuid1));
        networkBuilder.setUuid(new Uuid(bridgeUuid4));
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
        Network network = null;

        // Failure Case - Getting HTTP_BAD_REQUEST from isBridgeExist().
        networkBuilder = new NetworkBuilder();
        networkBuilder.setTenantId(new Uuid(tenantUuid2));
        networkBuilder.setUuid(new Uuid(bridgeUuid2));
        network = networkBuilder.build();
        networkHandler.neutronNetworkUpdated(network);

        // Failure Case - Setting network.shared to TRUE and will get HTTP_NOT_ACCEPTABLE.
        networkBuilder = new NetworkBuilder();
        networkBuilder.setTenantId(new Uuid(tenantUuid1));
        networkBuilder.setUuid(new Uuid(bridgeUuid1));
        networkBuilder.setShared(true);
        network = networkBuilder.build();
        networkHandler.neutronNetworkUpdated(network);

        // Failure Case - canModifyBridge() getting failed.
        networkBuilder = new NetworkBuilder();
        networkBuilder.setTenantId(new Uuid(tenantUuid1));
        networkBuilder.setUuid(new Uuid(bridgeUuid1));
        network = networkBuilder.build();
        networkHandler.neutronNetworkUpdated(network);
    }

    /**
     * Test method for
     * {@link NetworkHandler#neutronNetworkDeleted(NeutronNetwork)}.
     */
    @Test
    public void testNeutronNetworkDeleted() {
        Network network = null;

        // Success Case - isBridgeExist() returns HTTP_NOT_FOUND and deleteBridge() returns HTTP_OK.
        networkBuilder = new NetworkBuilder();
        networkBuilder.setTenantId(new Uuid(tenantUuid1));
        networkBuilder.setUuid(new Uuid(bridgeUuid1));
        network = networkBuilder.build();
        networkHandler.neutronNetworkDeleted(network);

        // Success Case - deleteBridge() returns HTTP_CONFLICT.
        networkBuilder = new NetworkBuilder();
        networkBuilder.setTenantId(new Uuid(tenantUuid2));
        networkBuilder.setUuid(new Uuid(bridgeUuid1));
        network = networkBuilder.build();
        networkHandler.neutronNetworkDeleted(network);

        // Failure Case - List is NULL and returns false from isVbridgesInTenant() and deleteTenant() returns HTTP_CONFLICT.
        networkBuilder = new NetworkBuilder();
        networkBuilder.setTenantId(new Uuid(tenantUuid3));
        networkBuilder.setUuid(new Uuid(bridgeUuid1));
        network = networkBuilder.build();
        networkHandler.neutronNetworkDeleted(network);

        // Success Case - List is empty and returns false from isVbridgesInTenant() and deleteTenant() returns HTTP_OK.
        networkBuilder = new NetworkBuilder();
        networkBuilder.setTenantId(new Uuid(tenantUuid1));
        networkBuilder.setUuid(new Uuid(bridgeUuid1));
        network = networkBuilder.build();
        networkHandler.neutronNetworkDeleted(network);

        // Success Case - List has elements and returns true from isVbridgesInTenant().
        networkBuilder = new NetworkBuilder();
        networkBuilder.setTenantId(new Uuid(tenantUuid1));
        networkBuilder.setUuid(new Uuid(bridgeUuid1));
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
