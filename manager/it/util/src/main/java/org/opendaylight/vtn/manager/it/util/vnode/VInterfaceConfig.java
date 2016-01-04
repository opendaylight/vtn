/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.vnode;

import static org.junit.Assert.assertEquals;

import static org.opendaylight.vtn.manager.it.util.ModelDrivenTestBase.getRpcOutput;
import static org.opendaylight.vtn.manager.it.util.ModelDrivenTestBase.getRpcResult;

import java.util.Random;

import org.opendaylight.vtn.manager.it.util.VTNServices;
import org.opendaylight.vtn.manager.it.util.flow.filter.FlowFilterList;
import org.opendaylight.vtn.manager.it.util.inventory.InventoryUtils;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.vtn.port.mappable.PortMapConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeUpdateMode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateOperationType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.RemoveVinterfaceInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.RemoveVinterfaceInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.UpdateVinterfaceInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.UpdateVinterfaceInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.VtnVinterfaceService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.VinterfaceStatus;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.list.Vinterface;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.list.VinterfaceBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.vinterface.info.VinterfaceConfig;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;

/**
 * {@code VInterfaceConfig} describes the configuration of a virtual interface.
 */
public final class VInterfaceConfig extends VNodeConfig<VInterfaceConfig>
    implements FlowFilterNode {
    /**
     * The value of enabled.
     */
    private Boolean  enabled;

    /**
     * Port mapping configuration.
     */
    private VTNPortMapConfig  portMap;

    /**
     * A list of input flow filter.
     */
    private final FlowFilterList  inputFilter = new FlowFilterList();

    /**
     * A list of output flow filter.
     */
    private final FlowFilterList  outputFilter = new FlowFilterList();

    /**
     * The expected state of the virtual interface.
     */
    private VnodeState  ifState;

    /**
     * The expected state of the entity mapped to this virtual interface.
     */
    private VnodeState  entityState;

    /**
     * Remove the specified virtual interface.
     *
     * @param service  The vtn-vinterface RPC service.
     * @param ident    The identifier for the virtual interface.
     */
    public static void removeVinterface(
        VtnVinterfaceService service, VInterfaceIdentifier<?> ident) {
        RemoveVinterfaceInputBuilder builder =
            new RemoveVinterfaceInputBuilder();
        builder.fieldsFrom(ident.getVirtualNodePath());

        RemoveVinterfaceInput input = builder.build();
        assertEquals(null,
                     getRpcOutput(service.removeVinterface(input), true));
    }

    /**
     * Construct a new instance with default values.
     */
    public VInterfaceConfig() {
    }

    /**
     * Construct a new instance.
     *
     * @param desc  The description about the virtual interface.
     * @param en    The value of enabled.
     */
    public VInterfaceConfig(String desc, Boolean en) {
        super(desc);
        enabled = en;
    }

    /**
     * Return the value of enabled.
     *
     * @return  The value of enabled.
     */
    public Boolean isEnabled() {
        return enabled;
    }

    /**
     * Set the value of enabled.
     *
     * @param en  The value of enabled.
     * @return  This instance.
     */
    public VInterfaceConfig setEnabled(Boolean en) {
        enabled = en;
        return this;
    }

    /**
     * Return the port mapping configuration.
     *
     * @return  A {@link VTNPortMapConfig} instance if configured.
     *          {@code null} otherwise.
     */
    public VTNPortMapConfig getPortMap() {
        return portMap;
    }

    /**
     * Set the port mapping configuration.
     *
     * @param pmconf  The port mapping configuration.
     * @return  This instance.
     */
    public VInterfaceConfig setPortMap(VTNPortMapConfig pmconf) {
        portMap = pmconf;
        return this;
    }

    /**
     * Return the expected state of the virtual interface.
     *
     * @return  The expected state of the virtual interface.
     *          {@code null} is returned if not configured.
     */
    public VnodeState getState() {
        return ifState;
    }

    /**
     * Set the expected state of the virtual interface.
     *
     * @param st  The expected state of the virtual interface.
     *            If {@code null} is specified, the expected interface state
     *            is determined automatically.
     * @return  This instance.
     */
    public VInterfaceConfig setState(VnodeState st) {
        ifState = st;
        return this;
    }

    /**
     * Return the expected state of the entity mapped to the virtual interface.
     *
     * @return  The expected state of the entity mapped to the virtual
     *          interface. {@code null} is returned if not configured.
     */
    public VnodeState getEntityState() {
        return entityState;
    }

    /**
     * Set the expected state of the entity mapped to the virtual interface.
     *
     * @param st  The expected state of the entity mapped to the virtual
     *            interface. If {@code null} is specified, the expected
     *            entity state is determined automatically.
     * @return  This instance.
     */
    public VInterfaceConfig setEntityState(VnodeState st) {
        entityState = st;
        return this;
    }

    /**
     * Create a new input builder for update-vinterface RPC.
     *
     * @return  An {@link UpdateVinterfaceInputBuilder} instance.
     */
    public UpdateVinterfaceInputBuilder newInputBuilder() {
        return new UpdateVinterfaceInputBuilder().
            setDescription(getDescription()).
            setEnabled(enabled);
    }

    /**
     * Add random configuration to this virtual interface configuration
     * using the given random generator.
     *
     * @param rand  A pseudo random generator.
     * @return  This instance.
     */
    public VInterfaceConfig add(Random rand) {
        inputFilter.add(rand);
        outputFilter.add(rand);
        return this;
    }

    /**
     * Update the specified virtual interface.
     *
     * @param service  The vtn-vinterface service.
     * @param ident    The identifier for the virtual interface.
     * @param mode     A {@link VnodeUpdateMode} instance.
     * @param op       A {@link VtnUpdateOperationType} instance.
     * @return  A {@link VtnUpdateType} instance returned by the RPC.
     */
    public VtnUpdateType update(VtnVinterfaceService service,
                                VInterfaceIdentifier<?> ident,
                                VnodeUpdateMode mode,
                                VtnUpdateOperationType op) {
        UpdateVinterfaceInputBuilder builder = newInputBuilder().
            setUpdateMode(mode).
            setOperation(op);
        builder.fieldsFrom(ident.getVirtualNodePath());
        return getRpcResult(service.updateVinterface(builder.build()));
    }

    /**
     * Verify the given virtual interface.
     *
     * @param rtx    A read-only MD-SAL datastore transaction.
     * @param ident  The identifier for the virtual interface.
     * @param vif    The virtual interface to be verified.
     */
    public void verify(ReadTransaction rtx, VInterfaceIdentifier<?> ident,
                       Vinterface vif) {
        // Verify the configuration.
        VinterfaceConfig vic = vif.getVinterfaceConfig();
        assertEquals(getDescription(), vic.getDescription());
        Boolean en = enabled;
        if (en == null) {
            en = Boolean.TRUE;
        }
        assertEquals(en, vic.isEnabled());

        // Verify the status.
        PortMapConfig pmc = vif.getPortMapConfig();
        VinterfaceStatus ist = vif.getVinterfaceStatus();
        NodeConnectorId mapped = ist.getMappedPort();
        VnodeState state = VnodeState.UNKNOWN;
        if (portMap == null) {
            assertEquals(VnodeState.UNKNOWN, ist.getEntityState());
            assertEquals(null, pmc);
            assertEquals(null, mapped);
        } else {
            // Verify port mapping configuration.
            portMap.verify(pmc, mapped);

            VtnPort vport = portMap.findPort(rtx);
            if (vport == null) {
                assertEquals(VnodeState.UNKNOWN, ist.getEntityState());
                assertEquals(null, mapped);
                state = VnodeState.DOWN;
            } else {
                assertEquals(vport.getId(), mapped);
                VnodeState portState = (InventoryUtils.isEnabled(vport))
                    ? VnodeState.UP : VnodeState.DOWN;
                assertEquals(portState, ist.getEntityState());
                if (portState == VnodeState.UP &&
                    InventoryUtils.isEdge(vport)) {
                    state = VnodeState.UP;
                } else {
                    state = VnodeState.DOWN;
                }
            }
        }

        if (!en.booleanValue()) {
            state = VnodeState.DOWN;
        }
        assertEquals(state, ist.getState());

        if (ifState != null) {
            assertEquals(ifState, ist.getState());
        }
        if (entityState != null) {
            assertEquals(entityState, ist.getEntityState());
        }

        // Verify the flow filters.
        inputFilter.verify(vif.getVinterfaceInputFilter());
        outputFilter.verify(vif.getVinterfaceOutputFilter());
    }

    /**
     * Apply all the configurations in this instance.
     *
     * @param service  A {@link VTNServices} instance.
     * @param ident    The identifier for the virtual interface.
     * @param current  The current virtual interface.
     */
    public void apply(VTNServices service, VInterfaceIdentifier<?> ident,
                      Vinterface current) {
        // Create the virtual interface if not present.
        UpdateVinterfaceInputBuilder builder = newInputBuilder().
            setUpdateMode(VnodeUpdateMode.UPDATE).
            setOperation(VtnUpdateOperationType.SET);
        builder.fieldsFrom(ident.getVirtualNodePath());
        UpdateVinterfaceInput input = builder.build();
        getRpcOutput(service.getVinterfaceService().updateVinterface(input));

        Vinterface vif = (current == null)
            ? new VinterfaceBuilder().build()
            : current;

        // Apply the port mapping configuration.
        if (portMap == null) {
            VtnUpdateType expected = (vif.getPortMapConfig() == null)
                ? null : VtnUpdateType.REMOVED;
            VtnUpdateType result = VTNPortMapConfig.removePortMap(
                service.getPortMapService(), ident);
            assertEquals(expected, result);
        } else {
            portMap.apply(service, ident);
        }

        // Apply the flow filter configurations.
        inputFilter.apply(service, ident, false,
                          vif.getVinterfaceInputFilter());
        outputFilter.apply(service, ident, true,
                           vif.getVinterfaceOutputFilter());
    }

    // FlowFilterNode

    /**
     * {@inheritDoc}
     */
    @Override
    public FlowFilterList getInputFilter() {
        return inputFilter;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public FlowFilterList getOutputFilter() {
        return outputFilter;
    }
}
