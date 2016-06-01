/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron.impl;

import static java.net.HttpURLConnection.HTTP_BAD_REQUEST;
import static java.net.HttpURLConnection.HTTP_OK;

import static org.opendaylight.vtn.manager.neutron.impl.VTNNeutronUtils.convertUUIDToKey;

import static org.mockito.Matchers.any;
import static org.mockito.Matchers.isA;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.verifyZeroInteractions;
import static org.mockito.Mockito.when;
import static org.mockito.MockitoAnnotations.initMocks;

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

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeUpdateMode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.RemoveVinterfaceInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.RemoveVinterfaceInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.UpdateVinterfaceInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.UpdateVinterfaceInputBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.ports.rev150712.ports.attributes.Ports;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.ports.rev150712.ports.attributes.ports.Port;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.ports.rev150712.ports.attributes.ports.PortBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.ports.rev150712.ports.attributes.ports.PortKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.rev150712.Neutron;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev130715.Uuid;

/**
 * JUnit test for {@link PortDataChangeListener}.
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest({VTNManagerService.class, MdsalUtils.class})
public class PortDataChangeListenerTest extends TestBase {
    /**
     * Mock-up of {@link DataBroker}.
     */
    @Mock
    private DataBroker  dataBroker;

    /**
     * Registration to be associated with {@link PortDataChangeListener}.
     */
    @Mock
    private ListenerRegistration<DataTreeChangeListener<Port>>  listenerReg;

    /**
     * A {@link PortDataChangeListener} instance for test.
     */
    private PortDataChangeListener  portListener;

    /**
     * VTNManagerService instance.
     */
    private VTNManagerService  vtnService;

    /**
     * Set up test environment.
     */
    @Before
    public void setUp() {
        initMocks(this);
        vtnService = PowerMockito.mock(VTNManagerService.class);
        Class<DataTreeIdentifier> idtype = DataTreeIdentifier.class;
        Class<DataTreeChangeListener> ltype = DataTreeChangeListener.class;
        when(dataBroker.registerDataTreeChangeListener(
                 (DataTreeIdentifier<Port>)any(idtype),
                 (DataTreeChangeListener<Port>)isA(ltype))).
            thenReturn(listenerReg);
        portListener = new PortDataChangeListener(dataBroker, vtnService);
    }
    /**
     * Test case for
     * {@link PortDataChangeListener#PortDataChangeListener(DataBroker,VTNManagerService)}.
     */
    @Test
    public void testConstructor() {
        // Ensure that PortDataChangeListener has been registered as data tree
        // change listener.
        LogicalDatastoreType cfg = LogicalDatastoreType.CONFIGURATION;
        DataTreeIdentifier<Port> ident =
            new DataTreeIdentifier<>(cfg, getPath());
        verify(dataBroker).registerDataTreeChangeListener(ident, portListener);
        verifyNoMoreInteractions(dataBroker, listenerReg);
    }

    /**
     * Test case for {@link PortDataChangeListener#close()}.
     */
    @Test
    public void testClose() {
        verifyZeroInteractions(listenerReg);

        // Close the listener.
        portListener.close();
        verify(listenerReg).close();
        verifyNoMoreInteractions(listenerReg);
    }

    /**
     * Test case for
     * {@link PortDataChangeListener#onDataTreeChanged(Collection)}
     *
     */
    @Test
    public void testOnDataTreeChanged() {
        List<DataTreeModification<Port>> changes = new ArrayList<>();
        List<UpdateVinterfaceInput> updateInputs = new ArrayList<>();
        List<RemoveVinterfaceInput> removeInputs = new ArrayList<>();

        // 1 port has been created.
        Port port = newPort("9096ad8f-fd79-42be-9019-a4932e5e5302",
                            "fdcfc6dd-d723-4868-928b-b2785f727270",
                            "8ff08212-964b-4879-8621-9d15c297f83e", "port-1",
                            true);
        changes.add(newModification(null, port));
        UpdateVinterfaceInput uinput = toUpdateVinterfaceInput(port);
        when(vtnService.updateInterface(uinput)).thenReturn(HTTP_OK);
        updateInputs.add(uinput);

        // 1 port has been deleted.
        port = newPort("3519e427-ad0e-3d14-aa70-151d4ff57c0e",
                       "40ef87c5-a5ee-41d5-b8a8-9afe0e85b75a",
                       "f4ac9a1a-8b3b-4760-8afe-2aadc6e8cdb4", "port-2", true);
        changes.add(newModification(port, null));
        RemoveVinterfaceInput rinput = toRemoveVinterfaceInput(port);
        when(vtnService.removeInterface(rinput)).thenReturn(HTTP_OK);
        removeInputs.add(rinput);

        // 1 more port has been created, but update-vinterface will fail.
        port = newPort("f3deba73-889c-34fc-a19c-03a4d63f37a1",
                       "361c9778-3a88-30b7-b25b-6a20f46f266b",
                       "37956e2d-a4ad-4722-8111-55d1e7af592b", "port-3", false);
        changes.add(newModification(null, port));
        uinput = toUpdateVinterfaceInput(port);
        when(vtnService.updateInterface(uinput)).thenReturn(HTTP_BAD_REQUEST);
        updateInputs.add(uinput);

        // 1 more port has been deleted, but remove-vinterface will fail.
        port = newPort("690ae9b6-9922-4923-91dc-1f4bd4e326bc",
                       "d6c8da9b-aee0-37ba-a58f-380f121abd65",
                       "06c6593a-d096-4cfe-b9ed-59ee6261db9d", "port-4", false);
        changes.add(newModification(port, null));
        rinput = toRemoveVinterfaceInput(port);
        when(vtnService.removeInterface(rinput)).thenReturn(HTTP_BAD_REQUEST);
        removeInputs.add(rinput);

        // Null port should be ignored.
        String uuid = "af092e3c-9fc6-33cd-8a9c-43d1401af43b";
        PortKey pkey = new PortKey(new Uuid(uuid));
        InstanceIdentifier<Port> path = InstanceIdentifier.
            builder(Neutron.class).
            child(Ports.class).
            child(Port.class, pkey).
            build();
        DataObjectModification<Port> mod = newKeyedModification(
            Port.class, ModificationType.WRITE, pkey, null, null, null);
        LogicalDatastoreType cfg = LogicalDatastoreType.CONFIGURATION;
        changes.add(newTreeModification(path, cfg, mod));

        mod = newKeyedModification(Port.class, ModificationType.DELETE, pkey,
                                   null, null, null);
        changes.add(newTreeModification(path, cfg, mod));

        mod = newKeyedModification(Port.class, ModificationType.DELETE, pkey,
                                   null, port, null);
        changes.add(newTreeModification(path, cfg, mod));

        // 1 more port has been created.
        port = newPort("2376e159-b2f3-30b9-ad7d-8e6e95317167",
                       "e390a7b9-a6bd-4ee0-a600-390c4c3ad2aa",
                       "f5fba412-bef2-3b6f-b5cb-350465c1605c", "port-5", false);
        changes.add(newModification(null, port));
        uinput = toUpdateVinterfaceInput(port);
        when(vtnService.updateInterface(uinput)).thenReturn(HTTP_OK);
        updateInputs.add(uinput);

        // 1 more port has been deleted.
        port = newPort("3bdf5b98-fdc3-4e69-b322-10c1d22b9094",
                       "ccdf4c1e-56c4-357d-ba89-02dea2346171",
                       "064423e7-3cb4-4797-b161-b83bf0a51236", "port-6", false);
        changes.add(newModification(port, null));
        rinput = toRemoveVinterfaceInput(port);
        when(vtnService.removeInterface(rinput)).thenReturn(HTTP_OK);
        removeInputs.add(rinput);

        // Notify changes.
        portListener.onDataTreeChanged(changes);

        for (UpdateVinterfaceInput uin: updateInputs) {
            verify(vtnService).updateInterface(uin);
        }
        for (RemoveVinterfaceInput rin: removeInputs) {
            verify(vtnService).removeInterface(rin);
        }
        verifyNoMoreInteractions(vtnService);

        // Empty collection should be ignored.
        changes.clear();
        portListener.onDataTreeChanged(changes);
        verifyNoMoreInteractions(vtnService);
    }

    /**
     * Return a wildcard path to the MD-SAL data model to listen.
     *
     * @return  A wildcard path to the MD-SAL data model to listen.
     */
    private InstanceIdentifier<Port> getPath() {
        return InstanceIdentifier.
            builder(Neutron.class).
            child(Ports.class).
            child(Port.class).
            build();
    }

    /**
     * Construct a new neutron port instance.
     *
     * @param tuuid  A string representation of tenant UUID.
     * @param nuuid  A string representation of network UUID.
     * @param puuid  A string representation of port UUID.
     * @param name   The name of the port.
     * @param up     Administrative state of the port.
     * @return  A new neutron port instance.
     */
    private Port newPort(String tuuid, String nuuid, String puuid, String name,
                         Boolean up) {
        return new PortBuilder().
            setTenantId(newUuid(tuuid)).
            setNetworkId(newUuid(nuuid)).
            setUuid(newUuid(puuid)).
            setName(name).
            setAdminStateUp(up).
            build();
    }

    /**
     * Convert the given neutron port to input for update-vinterface RPC.
     *
     * @param port  A neutron port instance.
     * @return  An {@link UpdateVinterfaceInput} instance.
     */
    private UpdateVinterfaceInput toUpdateVinterfaceInput(Port port) {
        return new UpdateVinterfaceInputBuilder().
            setTenantName(convertUUIDToKey(port.getTenantId())).
            setBridgeName(convertUUIDToKey(port.getNetworkId())).
            setInterfaceName(convertUUIDToKey(port.getUuid())).
            setDescription(port.getName()).
            setEnabled(port.isAdminStateUp()).
            setUpdateMode(VnodeUpdateMode.CREATE).
            build();
    }

    /**
     * Convert the given neutron port to input for remove-vinterface RPC.
     *
     * @param port  A neutron port instance.
     * @return  An {@link RemoveVinterfaceInput} instance.
     */
    private RemoveVinterfaceInput toRemoveVinterfaceInput(Port port) {
        return new RemoveVinterfaceInputBuilder().
            setTenantName(convertUUIDToKey(port.getTenantId())).
            setBridgeName(convertUUIDToKey(port.getNetworkId())).
            setInterfaceName(convertUUIDToKey(port.getUuid())).
            build();
    }

    /**
     * Create a new data tree modification that notifies changed neutron port.
     *
     * @param before  A neutron port instance before modification.
     * @param after   A neutron port instance after modification.
     * @return  A {@link DataTreeModification} instance.
     */
    private DataTreeModification<Port> newModification(
        Port before, Port after) {
        Port port = (before == null) ? after : before;
        InstanceIdentifier<Port> path = InstanceIdentifier.
            builder(Neutron.class).
            child(Ports.class).
            child(Port.class, port.getKey()).
            build();
        DataObjectModification<Port> mod =
            newKeyedModification(before, after, null);
        LogicalDatastoreType cfg = LogicalDatastoreType.CONFIGURATION;
        return newTreeModification(path, cfg, mod);
    }
}
