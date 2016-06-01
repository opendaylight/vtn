/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron.impl;

import static org.mockito.Matchers.any;
import static org.mockito.Matchers.isA;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.verifyZeroInteractions;
import static org.mockito.Mockito.when;
import static org.mockito.MockitoAnnotations.initMocks;

import static org.opendaylight.vtn.manager.neutron.impl.NeutronNetworkChangeListener.isChanged;

import java.util.ArrayList;
import java.util.List;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

import org.mockito.Mock;

import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.DataObjectModification.ModificationType;
import org.opendaylight.controller.md.sal.binding.api.DataObjectModification;
import org.opendaylight.controller.md.sal.binding.api.DataTreeChangeListener;
import org.opendaylight.controller.md.sal.binding.api.DataTreeIdentifier;
import org.opendaylight.controller.md.sal.binding.api.DataTreeModification;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.concepts.ListenerRegistration;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.networks.rev150712.networks.attributes.Networks;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.networks.rev150712.networks.attributes.networks.Network;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.networks.rev150712.networks.attributes.networks.NetworkBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.networks.rev150712.networks.attributes.networks.NetworkKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.provider.ext.rev150712.NetworkProviderExtension;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.provider.ext.rev150712.NetworkProviderExtensionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.rev150712.Neutron;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev130715.Uuid;

/**
 * JUnit test for {@link NeutronNetworkChangeListener}.
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest({NetworkHandler.class, MdsalUtils.class})
public class NeutronNetworkChangeListenerTest extends TestBase {
    /**
     * Mock-up of {@link DataBroker}.
     */
    @Mock
    private DataBroker  dataBroker;

    /**
     * Registration to be associated with {@link NeutronNetworkChangeListener}.
     */
    @Mock
    private ListenerRegistration<DataTreeChangeListener<Network>>  listenerReg;

    /**
     * A {@link NeutronNetworkChangeListener} instance for test.
     */
    private NeutronNetworkChangeListener  networkListener;

    /**
     * A {@link NetworkHandler} instance.
     */
    private NetworkHandler  networkHandler;

    /**
     * Set up test environment.
     */
    @Before
    public void setUp() {
        initMocks(this);
        networkHandler = PowerMockito.mock(NetworkHandler.class);
        Class<DataTreeIdentifier> idtype = DataTreeIdentifier.class;
        Class<DataTreeChangeListener> ltype = DataTreeChangeListener.class;
        when(dataBroker.registerDataTreeChangeListener(
                 (DataTreeIdentifier<Network>)any(idtype),
                 (DataTreeChangeListener<Network>)isA(ltype))).
            thenReturn(listenerReg);
        networkListener = new NeutronNetworkChangeListener(
            dataBroker, networkHandler);
    }

    /**
     * Test case for
     * {@link NeutronNetworkChangeListener#NeutronNetworkChangeListener(DataBroker,NetworkHandler) }.
     */
    @Test
    public void testConstructor() {
        // Ensure that NeutronNetworkChangeListener has been registered as
        // data tree change listener.
        LogicalDatastoreType cfg = LogicalDatastoreType.CONFIGURATION;
        DataTreeIdentifier<Network> ident =
            new DataTreeIdentifier<>(cfg, getPath());
        verify(dataBroker).
            registerDataTreeChangeListener(ident, networkListener);
        verifyNoMoreInteractions(dataBroker, listenerReg);
    }

    /**
     * Test case for {@link NeutronNetworkChangeListener#close()}.
     */
    @Test
    public void testClose() {
        verifyZeroInteractions(listenerReg);

        // Close the listener.
        networkListener.close();
        verify(listenerReg).close();
        verifyNoMoreInteractions(listenerReg);
    }

    /**
     * Test case for
     * {@link NeutronNetworkChangeListener#onDataTreeChanged(java.util.Collection)}
     */
    @Test
    public void testOnDataTreeChanged() {
        List<DataTreeModification<Network>> changes = new ArrayList<>();
        List<Network> createdNetworks = new ArrayList<>();
        List<Network> updatedNetworks = new ArrayList<>();
        List<Network> deletedNetworks = new ArrayList<>();

        // 1 network has been created.
        Network nw = newNetwork("6301f86c-ac14-39f1-80c5-5c7ed7f6b6c8",
                                "934d54be-b2ee-4047-8a15-0544f4b323c0",
                                "net-1", false, true);
        changes.add(newModification(null, nw));
        createdNetworks.add(nw);

        // 1 network has been updated.
        String tuuid = "7b1b6071-c7b7-3263-af02-dbd179df2778";
        String uuid = "419e4212-5de7-45db-9035-53e6b50e866c";
        Network before = newNetwork(tuuid, uuid, "net-2", false, true);
        Network after = newNetwork(tuuid, uuid, "net-2(1)", false, true);
        changes.add(newModification(before, after));
        updatedNetworks.add(after);

        // 1 network has been deleted.
        nw = newNetwork("1eb6e93a-5b26-434e-905d-2c6bdc3b2319",
                        "abd4f9f4-b92e-4241-9f35-58a35f4a6fea", "net-3",
                        false, false);
        changes.add(newModification(nw, null));
        deletedNetworks.add(nw);

        // Unchanged network should be ignored.
        tuuid = "fcb160c5-f889-3659-af01-6d59e8c43164";
        uuid = "cccfb6f3-77e5-4fc3-b6af-e09f8f5bad8d";
        before = newNetwork(tuuid, uuid, "net-4", true, false);
        after = newNetwork(tuuid, uuid, "net-4", true, false);
        changes.add(newModification(before, after));

        tuuid = "c3547f54-ce50-4bce-ad49-edccc6885b22";
        uuid = "53657b99-2f3f-3350-9ccf-187671dffee8";
        before = newNetwork(tuuid, uuid, "net-5", false, true);
        after = newNetwork(tuuid, uuid, "net-5", false, true);
        changes.add(newModification(before, after));

        // 1 more network has been created.
        nw = newNetwork("33345576-8b73-3861-b274-2a8dcfcca6b7",
                        "460118c4-4d6f-4d6e-b3fb-44a219e07e42", "net-6", false,
                        false);
        changes.add(newModification(null, nw));
        createdNetworks.add(nw);

        // 1 more network has been deleted.
        nw = newNetwork("297de2f1-3e2c-33db-9bfb-cf9b609c4a43",
                        "59a2a06d-676e-45f5-8539-30a7961d4752", "net-7", false,
                        true);
        changes.add(newModification(nw, null));
        deletedNetworks.add(nw);

        // Null network should be ignored.
        uuid = "a44dc362-04da-3e52-a341-3f699402ec1c";
        NetworkKey nkey = new NetworkKey(new Uuid(uuid));
        InstanceIdentifier<Network> path = InstanceIdentifier.
            builder(Neutron.class).
            child(Networks.class).
            child(Network.class, nkey).
            build();
        DataObjectModification<Network> mod = newKeyedModification(
            Network.class, ModificationType.WRITE, nkey, null, null, null);
        LogicalDatastoreType cfg = LogicalDatastoreType.CONFIGURATION;
        changes.add(newTreeModification(path, cfg, mod));

        mod = newKeyedModification(
            Network.class, ModificationType.SUBTREE_MODIFIED, nkey, before,
            null, null);
        changes.add(newTreeModification(path, cfg, mod));

        mod = newKeyedModification(
            Network.class, ModificationType.DELETE, nkey, null, null, null);
        changes.add(newTreeModification(path, cfg, mod));

        mod = newKeyedModification(
            Network.class, ModificationType.DELETE, nkey, null, after, null);
        changes.add(newTreeModification(path, cfg, mod));

        // 5 more networks have been updated.
        uuid = "1f275485-89cf-3c23-a12c-0184864f3318";
        before = newNetwork("620dfe46-7f3b-4e7c-bcdd-4286f18ef93a", uuid,
                            "net-8", false, false);
        after = newNetwork("620dfe46-7f3b-4e7c-bcdd-4286f18ef93b", uuid,
                           "net-8", false, false);
        changes.add(newModification(before, after));
        updatedNetworks.add(after);

        tuuid = "7e44c7ee-eff5-3fb5-9d1f-75b8b44073b7";
        before = newNetwork(tuuid, "28d26d70-38b2-4ed1-8de5-843b6d2b761e",
                            "net-9", true, true);
        after = newNetwork(tuuid, "38d26d70-38b2-4ed1-8de5-843b6d2b761e",
                            "net-9", true, true);
        changes.add(newModification(before, after));
        updatedNetworks.add(after);

        tuuid = "d8a83087-1cb0-3527-a313-fd61382b0a92";
        uuid = "a34f6fbf-bfd8-4bf7-8459-0b81bc767936";
        before = newNetwork(tuuid, uuid, "net-10", false, true);
        after = newNetwork(tuuid, uuid, null, false, true);
        changes.add(newModification(before, after));
        updatedNetworks.add(after);

        tuuid = "84b6055d-0564-34ae-b4b5-5ad3b1a146b6";
        uuid = "d5da322b-1e9d-49ce-884d-eb2cfdc46f73";
        before = newNetwork(tuuid, uuid, "net-11", true, true);
        after = newNetwork(tuuid, uuid, "net-11", false, true);
        changes.add(newModification(before, after));
        updatedNetworks.add(after);

        tuuid = "a9bb2258-9acd-4a0e-b990-fbbeaeb1e956";
        uuid = "8f586172-452e-3b41-b482-b2c3c54cebbd";
        before = newNetwork(tuuid, uuid, "net-12", true, false);
        after = newNetwork(tuuid, uuid, "net-12", false, true);
        changes.add(newModification(before, after));
        updatedNetworks.add(after);

        // Notify changes.
        networkListener.onDataTreeChanged(changes);

        for (Network n: createdNetworks) {
            verify(networkHandler).neutronNetworkCreated(n);
        }
        for (Network n: updatedNetworks) {
            verify(networkHandler).neutronNetworkUpdated(n);
        }
        for (Network n: deletedNetworks) {
            verify(networkHandler).neutronNetworkDeleted(n);
        }
        verifyNoMoreInteractions(networkHandler);

        // Empty collection should be ignored.
        changes.clear();
        networkListener.onDataTreeChanged(changes);
        verifyNoMoreInteractions(networkHandler);
    }

    /**
     * Test case for {@link NeutronNetworkChangeListener#isChanged(Network,Network)}.
     */
    @Test
    public void testIsUpdated() {
        NetworkBuilder before = new NetworkBuilder();
        NetworkBuilder after = new NetworkBuilder();
        assertEquals(false, isChanged(before.build(), after.build()));

        String tuuid = "6b25019d-18ea-30e7-b433-82508b4cc55f";
        before.setTenantId(newUuid(tuuid));
        assertEquals(true, isChanged(before.build(), after.build()));
        before.setTenantId(null);
        after.setTenantId(newUuid(tuuid));
        assertEquals(true, isChanged(before.build(), after.build()));
        before.setTenantId(newUuid(tuuid));
        assertEquals(false, isChanged(before.build(), after.build()));

        String uuid = "ced20e18-c82f-4c1f-acfe-a2576b639b4f";
        before.setUuid(newUuid(uuid));
        assertEquals(true, isChanged(before.build(), after.build()));
        before.setUuid(null);
        after.setUuid(newUuid(uuid));
        assertEquals(true, isChanged(before.build(), after.build()));
        before.setUuid(newUuid(uuid));
        assertEquals(false, isChanged(before.build(), after.build()));

        String name = "network-1";
        before.setName(name);
        assertEquals(true, isChanged(before.build(), after.build()));
        before.setName(null);
        after.setName(name);
        assertEquals(true, isChanged(before.build(), after.build()));
        before.setName(name);
        assertEquals(false, isChanged(before.build(), after.build()));

        Boolean shared = Boolean.TRUE;
        before.setShared(shared);
        assertEquals(true, isChanged(before.build(), after.build()));
        before.setShared(null);
        after.setShared(shared);
        assertEquals(true, isChanged(before.build(), after.build()));
        before.setShared(shared);
        assertEquals(false, isChanged(before.build(), after.build()));

        String tuuid1 = "35f547d8-82e6-3486-aeb6-8e7da2cd6fb6";
        before.setTenantId(newUuid(tuuid1));
        assertEquals(true, isChanged(before.build(), after.build()));
        before.setTenantId(newUuid(tuuid));
        after.setTenantId(newUuid(tuuid1));
        assertEquals(true, isChanged(before.build(), after.build()));
        before.setTenantId(newUuid(tuuid1));
        assertEquals(false, isChanged(before.build(), after.build()));

        String uuid1 = "44c65163-42bd-40c6-a043-0455fa8b9443";
        before.setUuid(newUuid(uuid1));
        assertEquals(true, isChanged(before.build(), after.build()));
        before.setUuid(newUuid(uuid));
        after.setUuid(newUuid(uuid1));
        assertEquals(true, isChanged(before.build(), after.build()));
        before.setUuid(newUuid(uuid1));
        assertEquals(false, isChanged(before.build(), after.build()));

        String name1 = "network-2";
        before.setName(name1);
        assertEquals(true, isChanged(before.build(), after.build()));
        before.setName(name);
        after.setName(name1);
        assertEquals(true, isChanged(before.build(), after.build()));
        before.setName(name1);
        assertEquals(false, isChanged(before.build(), after.build()));

        Boolean shared1 = Boolean.FALSE;
        before.setShared(shared1);
        assertEquals(true, isChanged(before.build(), after.build()));
        before.setShared(shared);
        after.setShared(shared1);
        assertEquals(true, isChanged(before.build(), after.build()));
        before.setShared(shared1);
        assertEquals(false, isChanged(before.build(), after.build()));

        // Other fields and augments should be ignored.
        before.setAdminStateUp(true).setStatus("status 1");
        after.setAdminStateUp(false).setStatus("status 2");
        assertEquals(false, isChanged(before.build(), after.build()));

        NetworkProviderExtension ext = new NetworkProviderExtensionBuilder().
            setPhysicalNetwork("net-1").
            setSegmentationId("12345").
            build();
        before.addAugmentation(NetworkProviderExtension.class, ext);
        assertEquals(false, isChanged(before.build(), after.build()));
    }

    /**
     * Return a wildcard path to the MD-SAL data model to listen.
     *
     * @return  A wildcard path to the MD-SAL data model to listen.
     */
    private InstanceIdentifier<Network> getPath() {
        return InstanceIdentifier.
            builder(Neutron.class).
            child(Networks.class).
            child(Network.class).
            build();
    }

    /**
     * Construct a new neutron network instance.
     *
     * @param tuuid   A string representation of tenant UUID.
     * @param uuid    A string representation of network UUID.
     * @param name    The name of the network.
     * @param shared  A shared network status.
     * @param up      Administrative state of the network.
     * @return  A neutron network instance.
     */
    private Network newNetwork(String tuuid, String uuid, String name,
                               Boolean shared, Boolean up) {
        return new NetworkBuilder().
            setTenantId(newUuid(tuuid)).
            setUuid(newUuid(uuid)).
            setName(name).
            setShared(shared).
            setAdminStateUp(up).
            build();
    }

    /**
     * Create a new data tree modification that notifies changed neutron
     * network.
     *
     * @param before  Neutron network instance before modification.
     * @param after   Neutron network instance after modification.
     * @return  A {@link DataTreeModification} instance.
     */
    private DataTreeModification<Network> newModification(
        Network before, Network after) {
        Network nw = (before == null) ? after : before;
        InstanceIdentifier<Network> path = InstanceIdentifier.
            builder(Neutron.class).
            child(Networks.class).
            child(Network.class, nw.getKey()).
            build();
        DataObjectModification<Network> mod =
            newKeyedModification(before, after, null);
        LogicalDatastoreType cfg = LogicalDatastoreType.CONFIGURATION;
        return newTreeModification(path, cfg, mod);
    }
}
