/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode.xml;

import static org.opendaylight.vtn.manager.util.NumberUtils.HASH_PRIME;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlElementWrapper;
import javax.xml.bind.annotation.XmlRootElement;

import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.flow.filter.FlowFilterList;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.vnode.BridgeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeType;
import org.opendaylight.vtn.manager.internal.util.vnode.VTNMacMapConfig;
import org.opendaylight.vtn.manager.internal.util.vnode.VTNVlanMapConfig;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.mappable.MacMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.vtn.vlan.mappable.VlanMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.VtnVbridgeInfo;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.info.VbridgeConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.info.VbridgeConfigBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.info.VbridgeInputFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.info.VbridgeInputFilterBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.info.VbridgeOutputFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.info.VbridgeOutputFilterBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.VbridgeBuilder;

/**
 * {@code XmlVBridge} provides XML binding to the data model for vBridge.
 */
@XmlRootElement(name = "vbridge")
@XmlAccessorType(XmlAccessType.NONE)
public final class XmlVBridge extends XmlAbstractBridge {
    /**
     * The number of seconds between MAC address table aging.
     */
    @XmlElement(name = "age-interval", required = true)
    private Integer  ageInterval;

    /**
     * A list of VLAN mapping configurations.
     */
    @XmlElementWrapper(name = "vlan-maps")
    @XmlElement(name = "vlan-map")
    private List<VTNVlanMapConfig>  vlanMaps;

    /**
     * MAC mapping.
     */
    @XmlElement(name = "mac-map")
    private VTNMacMapConfig  macMap;

    /**
     * A list of flow filters applied to packets received from this vBridge.
     */
    @XmlElement(name = "input-filters")
    private FlowFilterList  inputFilters;

    /**
     * A list of flow filters applied to packets transmitted from this vBridge.
     */
    @XmlElement(name = "output-filters")
    private FlowFilterList  outputFilters;

    /**
     * Private constructor only for JAXB.
     */
    @SuppressWarnings("unused")
    private XmlVBridge() {
    }

    /**
     * Construct a new instance.
     *
     * @param vbr  A {@link VtnVbridgeInfo} instance.
     * @throws RpcException
     *    The given instance is invalid.
     */
    public XmlVBridge(VtnVbridgeInfo vbr) throws RpcException {
        super(vbr.getName(), vbr.getVinterface());

        VbridgeConfig vbconf = vbr.getVbridgeConfig();
        setDescription(vbconf.getDescription());
        ageInterval = vbconf.getAgeInterval();

        List<VlanMap> vmaps = vbr.getVlanMap();
        if (vmaps != null && !vmaps.isEmpty()) {
            List<VTNVlanMapConfig> vlist = new ArrayList<>(vmaps.size());
            for (VlanMap vmap: vmaps) {
                vlist.add(new VTNVlanMapConfig(vmap.getVlanMapConfig()));
            }
            vlanMaps = vlist;
        }

        MacMap mmap = vbr.getMacMap();
        if (mmap != null) {
            macMap = new VTNMacMapConfig(mmap.getMacMapConfig());
        }

        inputFilters =
            FlowFilterList.create(vbr.getVbridgeInputFilter(), true);
        outputFilters =
            FlowFilterList.create(vbr.getVbridgeOutputFilter(), true);
    }

    /**
     * Return a list of VLAN mapping configurations.
     *
     * @return  A list of {@link VTNVlanMapConfig} instances or {@code null}.
     */
    public List<VTNVlanMapConfig> getVlanMaps() {
        return (vlanMaps == null || vlanMaps.isEmpty()) ? null : vlanMaps;
    }

    /**
     * Return a MAC mapping configuration if present.
     *
     * @return  A {@link VTNMacMapConfig} instance if MAC mapping is configured
     *          in this vBridge. Otherwise {@code null}.
     */
    public VTNMacMapConfig getMacMap() {
        return macMap;
    }

    /**
     * Return a list of input flow filters.
     *
     * @return  A {@link FlowFilterList} instance or {@code null}.
     */
    public FlowFilterList getInputFilters() {
        return inputFilters;
    }

    /**
     * Return a list of output flow filters.
     *
     * @return  A {@link FlowFilterList} instance or {@code null}.
     */
    public FlowFilterList getOutputFilters() {
        return outputFilters;
    }

    /**
     * Convert this instance into a {@link VbridgeBuilder} instance.
     *
     * <p>
     *   Note that returned instance contains only configuration for the
     *   vBridge. Other information needs to be resumed by the caller.
     * </p>
     * <ul>
     *   <li>Virtual interfaces</li>
     *   <li>MAC mapping</li>
     *   <li>MAC mapping</li>
     * </ul>
     *
     * @param xlogger  A {@link XmlLogger} instance.
     * @param vbrId    An identifier for this vBridge.
     * @return  A {@link VbridgeBuilder} instance.
     * @throws RpcException
     *    Failed to convert this instance.
     */
    public VbridgeBuilder toVbridgeBuilder(XmlLogger xlogger,
                                           BridgeIdentifier<Vbridge> vbrId)
        throws RpcException {
        VnodeName vname = checkName(VNodeType.VBRIDGE.toString());
        if (ageInterval == null) {
            throw RpcException.getBadArgumentException(
                vbrId.toString() + ": age-interval is missing");
        }

        VbridgeConfig vbrc = new VbridgeConfigBuilder().
            setDescription(getDescription()).
            setAgeInterval(ageInterval).
            build();

        VbridgeBuilder builder = new VbridgeBuilder().
            setName(vname).
            setVbridgeConfig(vbrc);

        List<VtnFlowFilter> vflist =
            toVtnFlowFilterList(xlogger, vbrId, inputFilters, false);
        if (vflist != null) {
            VbridgeInputFilter in = new VbridgeInputFilterBuilder().
                setVtnFlowFilter(vflist).build();
            builder.setVbridgeInputFilter(in);
        }

        vflist = toVtnFlowFilterList(xlogger, vbrId, outputFilters, true);
        if (vflist != null) {
            VbridgeOutputFilter out = new VbridgeOutputFilterBuilder().
                setVtnFlowFilter(vflist).build();
            builder.setVbridgeOutputFilter(out);
        }

        // Verify mapping configurations.
        if (vlanMaps != null) {
            for (VTNVlanMapConfig vmc: vlanMaps) {
                vmc.verify();
            }
        }

        if (macMap != null) {
            macMap.verify();
        }

        return builder;
    }

    // Object

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        boolean ret = (o == this);
        if (!ret && super.equals(o)) {
            XmlVBridge xvbr = (XmlVBridge)o;
            if (Objects.equals(ageInterval, xvbr.ageInterval) &&
                Objects.equals(macMap, xvbr.macMap) &&
                MiscUtils.equalsAsSet(vlanMaps, xvbr.vlanMaps)) {
                ret = (Objects.equals(inputFilters, xvbr.inputFilters) &&
                       Objects.equals(outputFilters, xvbr.outputFilters));
            }
        }

        return ret;
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = super.hashCode() * HASH_PRIME +
            Objects.hash(ageInterval, macMap, inputFilters, outputFilters);
        if (vlanMaps != null) {
            for (VTNVlanMapConfig vmc: vlanMaps) {
                h += vmc.hashCode();
            }
        }

        return h;
    }
}
