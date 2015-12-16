/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.vnode;

import java.util.Arrays;
import java.util.List;

import org.opendaylight.vtn.manager.it.util.inventory.MacVlan;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier.InstanceIdentifierBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.virtual.route.info.VirtualNodePath;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.BridgeMapInfo;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.BridgeMapInfoBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.config.AllowedHosts;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.info.MacMapConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.mappable.MacMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.vlan.host.desc.set.VlanHostDescList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.vlan.host.desc.set.VlanHostDescListKey;

/**
 * {@code MacMapHostIdentifier} describes an identifier for a host mapped
 * by a MAC mapping configured in a vBridge.
 */
public final class MacMapHostIdentifier
    extends VBridgeMapIdentifier<VlanHostDescList> {
    /**
     * A pair of MAC address and VLAN ID which specifies the host mapped by
     * a MAC mapping.
     */
    private final MacVlan  mappedHost;

    /**
     * Return the L2 host mapped by the MAC mapping configured in the
     * specified virtual-node-path.
     *
     * @param vpath  A {@link VirtualNodePath} instance that specifies the
     *               virtual node.
     * @return  A {@link MacVlan} instance if {@code vpath} specifies the
     *          host mapped by MAC mapping. {@code null} otherwise}.
     */
    public static MacVlan getMappedHost(VirtualNodePath vpath) {
        MacVlan mv = null;
        if (vpath != null) {
            BridgeMapInfo bmi = vpath.getAugmentation(BridgeMapInfo.class);
            if (bmi != null) {
                Long host = bmi.getMacMappedHost();
                if (host != null) {
                    mv = new MacVlan(host.longValue());
                }
            }
        }

        return mv;
    }

    /**
     * Construct a new instance.
     *
     * @param tname  The name of the VTN.
     * @param bname  The name of the vBridge.
     * @param mvlan  A {@link MacVlan} instance which specifies the host mapped
     *               by a MAC mapping.
     */
    public MacMapHostIdentifier(VnodeName tname, VnodeName bname,
                                MacVlan mvlan) {
        super(tname, bname);
        mappedHost = mvlan;
    }

    /**
     * Construct a new instance.
     *
     * @param mapId  A {@link MacMapIdentifier} instance that specifies the
     *               MAC mapping configured in a vBridge.
     * @param mvlan  A {@link MacVlan} instance which specifies the host mapped
     *               by a MAC mapping.
     */
    public MacMapHostIdentifier(MacMapIdentifier mapId, MacVlan mvlan) {
        super(mapId.getTenantName(), mapId.getBridgeName());
        mappedHost = mvlan;
    }

    /**
     * Construct a new instance from the given path components.
     *
     * @param comps  An array of strings which represents the path components
     *               of identifier. Note that the caller must guarantee that
     *               {@code comps} contains valid MAC mapped host path
     *               components.
     */
    MacMapHostIdentifier(String[] comps) {
        super(comps[0], comps[1]);
        mappedHost = new MacVlan(Long.parseLong(comps[2]));
    }

    /**
     * Return information about a host mapped by a MAC mapping.
     *
     * @return  A {@link MacVlan} instance which specifies the host mapped
     *          by a MAC mapping.
     */
    public MacVlan getMappedHost() {
        return mappedHost;
    }

    // VNodeIdentifier

    /**
     * Return a {@link VNodeType} instance which indicates the type of the
     * virtual node.
     *
     * @return  {@link VNodeType#MACMAP_HOST}.
     */
    @Override
    public VNodeType getType() {
        return VNodeType.MACMAP_HOST;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public InstanceIdentifierBuilder<VlanHostDescList> getIdentifierBuilder() {
        VlanHostDescListKey key =
            new VlanHostDescListKey(mappedHost.getVlanHostDesc());
        return getVBridgeIdentifierBuilder().
            child(MacMap.class).
            child(MacMapConfig.class).
            child(AllowedHosts.class).
            child(VlanHostDescList.class, key);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected List<String> newComponents() {
        String host = (mappedHost == null)
            ? null
            : Long.toString(mappedHost.getEncodedValue());
        return Arrays.asList(getTenantNameString(), getBridgeNameString(),
                             host);
    }

    /**
     * Determine whether the MAC mapped host specified by this instance is
     * equal to the MAC mapped host specified by {@link VirtualNodePath}
     * instance.
     *
     * @param vpath  A {@link VirtualNodePath} instance that specifies the
     *               virtual node. {@code null} cannot be specified.
     * @return  {@code true} if the MAC mapped host specified by this instance
     *          is equal to the MAC mapped host specified by {@code vpath}.
     *          {@code false} otherwise.
     */
    @Override
    public boolean contains(VirtualNodePath vpath) {
        MacVlan host = getMappedHost(vpath);
        boolean ret;
        if (host == null) {
            ret = false;
        } else {
            ret = (super.contains(vpath) &&
                   host.getEncodedValue() == mappedHost.getEncodedValue());
        }

        return ret;
    }

    // VBridgeMapIdentifier

    /**
     * Return a {@link BridgeMapInfo} instance that indicates the MAC mapped
     * host specified by this instance.
     *
     * @return  A {@link BridgeMapInfo} instance.
     */
    @Override
    protected BridgeMapInfo getBridgeMapInfo() {
        return new BridgeMapInfoBuilder().
            setMacMappedHost(mappedHost.getEncodedValue()).build();
    }
}
