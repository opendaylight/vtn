/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.vnode;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map.Entry;
import java.util.Map;
import java.util.Set;

import org.opendaylight.vtn.manager.it.util.VTNServices;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.VtnPortMappableBridge;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.port.mappable.bridge.BridgeStatus;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.VtnVinterfaceService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.list.Vinterface;

/**
 * {@code BridgeConfig} describes the configuration of a virtual bridge.
 *
 * @param <C>  The type of this instance.
 * @param <B>  The type of the virtual bridge.
 */
public abstract class BridgeConfig<C extends BridgeConfig,
                                   B extends VtnPortMappableBridge>
    extends VNodeConfig<C> {
    /**
     * Configuration of virtual interfaces.
     */
    private final Map<String, VInterfaceConfig>  vInterfaces = new HashMap<>();

    /**
     * Construct a new instance without specifying the description.
     */
    protected BridgeConfig() {
    }

    /**
     * Construct a new instance.
     *
     * @param desc  The description about the virtual node.
     */
    protected BridgeConfig(String desc) {
        super(desc);
    }

    /**
     * Add the given virtual interface configuration.
     *
     * @param name   The name of the virtual interface.
     * @param iconf  The virtual interface configuration.
     * @return  This instance.
     */
    public final C addInterface(String name, VInterfaceConfig iconf) {
        vInterfaces.put(name, iconf);
        return getThis();
    }

    /**
     * Remove the virtual interface configuration specified by the name.
     *
     * @param name  The name of the virtual interface.
     * @return  This instance.
     */
    public final C removeInterface(String name) {
        vInterfaces.remove(name);
        return getThis();
    }

    /**
     * Return the configuration of the virtual interface.
     *
     * @param name  The name of the virtual interface.
     * @return  A {@link VInterfaceConfig} if found.
     *          {@code null} if not found.
     */
    public final VInterfaceConfig getInterface(String name) {
        return vInterfaces.get(name);
    }

    /**
     * Return an unmodifiable map that contains all the virtual interfaces
     * in this virtual bridge.
     *
     * @return  An unmodifiable map that contains all the virtual interfaces.
     */
    public final Map<String, VInterfaceConfig> getInterfaces() {
        return Collections.unmodifiableMap(vInterfaces);
    }

    /**
     * Return the number of virtual interfaces in this virtual bridge.
     *
     * @return  The number of virtual interfaces.
     */
    public final int getInterfaceCount() {
        return vInterfaces.size();
    }

    /**
     * Verify the given virtual bridge.
     *
     * <p>
     *   Note that this method expects the specified virtual bridge does not
     *   contain faulted paths.
     * </p>
     *
     * @param rtx     A read-only MD-SAL datastore transaction.
     * @param ident   The identifier for the virtual bridge.
     * @param bridge  The virtual bridge to be verified.
     * @param state   The state of the virtual bridge determined by the
     *                virtual network mapping configured in the given bridge.
     */
    protected final void verify(ReadTransaction rtx, BridgeIdentifier<B> ident,
                                B bridge, VnodeState state) {
        VnodeState bstate = state;

        // Verify the configuration of the virtual interfaces.
        List<Vinterface> ifList = bridge.getVinterface();
        if (!vInterfaces.isEmpty()) {
            assertNotNull(ifList);
            Set<String> checked = new HashSet<>();
            for (Vinterface vif: ifList) {
                VnodeName vname = vif.getName();
                String name = vname.getValue();
                VInterfaceConfig iconf = vInterfaces.get(name);
                assertNotNull(iconf);
                VInterfaceIdentifier<B> ifId = ident.childInterface(vname);
                iconf.verify(rtx, ifId, vif);
                assertEquals(true, checked.add(name));

                if (!Boolean.FALSE.equals(iconf.isEnabled())) {
                    VnodeState ist = vif.getVinterfaceStatus().getState();
                    if (bstate != VnodeState.DOWN &&
                        ist != VnodeState.UNKNOWN) {
                        bstate = ist;
                    }
                }
            }
            assertEquals(checked, vInterfaces.keySet());
        } else if (ifList != null) {
            assertEquals(Collections.<Vinterface>emptyList(), ifList);
        }

        // Verify the bridge status.
        BridgeStatus bst = bridge.getBridgeStatus();
        assertEquals(bstate, bst.getState());
        assertEquals(0, bst.getPathFaults().intValue());
        assertEquals(null, bst.getFaultedPaths());
    }

    /**
     * Apply all the virtual interface configurations in this instance.
     *
     * @param service  A {@link VTNServices} instance.
     * @param ident    The identifier for the virtual bridge.
     * @param current  The current virtual bridge.
     */
    protected void applyInterfaces(
        VTNServices service, BridgeIdentifier<B> ident, B current) {
        Map<String, Vinterface> present =
            removeUnwanted(service.getVinterfaceService(), ident, current);

        for (Entry<String, VInterfaceConfig> entry: vInterfaces.entrySet()) {
            String name = entry.getKey();
            VInterfaceConfig iconf = entry.getValue();
            VnodeName vname = new VnodeName(name);
            VInterfaceIdentifier<B> ifId = ident.childInterface(vname);
            iconf.apply(service, ifId, present.get(name));
        }
    }

    /**
     * Remove all the virtual interfaces that are not associated with the
     * configuration in this instance.
     *
     * @param service  The vtn-vinterface RPC service.
     * @param ident    The identifier for the virtual bridge.
     * @param current  The current virtual bridge.
     * @return  A map that contains virtual interfaces present in the
     *          target virtual bridge.
     */
    private Map<String, Vinterface> removeUnwanted(
        VtnVinterfaceService service, BridgeIdentifier<B> ident, B current) {
        List<Vinterface> ifList = current.getVinterface();
        Map<String, Vinterface> present = new HashMap<>();
        if (ifList != null) {
            for (Vinterface vif: ifList) {
                VnodeName vname = vif.getName();
                String name = vname.getValue();
                assertEquals(null, present.put(name, vif));
                if (!vInterfaces.containsKey(vname.getValue())) {
                    VInterfaceIdentifier<B> ifId = ident.childInterface(vname);
                    VInterfaceConfig.removeVinterface(service, ifId);
                }
            }
        }

        return present;
    }
}
