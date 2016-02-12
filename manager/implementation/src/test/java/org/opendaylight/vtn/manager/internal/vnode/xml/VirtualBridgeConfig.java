/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode.xml;

import java.util.Random;

import org.opendaylight.vtn.manager.internal.XmlNode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.port.mappable.bridge.BridgeStatus;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.port.mappable.bridge.BridgeStatusBuilder;

/**
 * {@code VirtualBridgeConfig} describes configuration about a virtual bridge
 * in a VTN.
 */
public abstract class VirtualBridgeConfig extends VNodeConfig {
    /**
     * A list of virtual interfaces.
     */
    private VInterfaceConfigList  interfaces;

    /**
     * Create a dummy bridge-status instance.
     *
     * @return A {@link BridgeStatus} instance.
     */
    public static final BridgeStatus newBridgeStatus() {
        return new BridgeStatusBuilder().
            setState(VnodeState.UP).
            setPathFaults(0).
            build();
    }

    /**
     * Construct a new instance.
     *
     * @param name  The name of the virtual bridge.
     */
    protected VirtualBridgeConfig(String name) {
        super(name, null);
    }

    /**
     * Construct a new instance.
     *
     * @param name  The name of the virtual bridge.
     * @param desc  Description about the virtual bridge.
     */
    protected VirtualBridgeConfig(String name, String desc) {
        super(name, desc);
    }

    /**
     * Construct a new instance using the given random number generator.
     *
     * @param name  The name of the virtual bridge.
     * @param desc  Description about the virtual bridge.
     * @param rand  A pseudo random number generator.
     */
    protected VirtualBridgeConfig(String name, String desc, Random rand) {
        super(name, desc);
        interfaces = new VInterfaceConfigList(rand);
    }

    /**
     * Return a list of virtual interfaces.
     *
     * @return  A {@link VInterfaceConfigList} instance.
     */
    public final VInterfaceConfigList getInterfaces() {
        return interfaces;
    }

    /**
     * Set a list of virtual interfaces.
     *
     * @param list  A {@link VInterfaceConfigList} instance.
     * @return  This instance.
     */
    public final VirtualBridgeConfig setInterfaces(VInterfaceConfigList list) {
        interfaces = list;
        return this;
    }

    /**
     * Set the virtual interface configuration in this instance into the
     * specified XML node.
     *
     * @param xnode   A {@link XmlNode} instance.
     */
    public final void setInterfacesAsXml(XmlNode xnode) {
        if (interfaces != null) {
            interfaces.setXml(xnode);
        }
    }
}
