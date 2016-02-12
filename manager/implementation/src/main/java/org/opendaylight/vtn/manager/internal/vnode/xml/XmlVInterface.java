/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode.xml;

import static org.opendaylight.vtn.manager.util.NumberUtils.HASH_PRIME;
import static org.opendaylight.vtn.manager.internal.util.vnode.VInterfaceIdentifier.DESCRIPTION;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

import org.opendaylight.vtn.manager.internal.util.flow.filter.FlowFilterList;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.vnode.VInterfaceIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VTNPortMapConfig;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.vtn.port.mappable.PortMapConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.VtnMappableVinterface;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.list.Vinterface;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.list.VinterfaceBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.vinterface.info.VinterfaceConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.vinterface.info.VinterfaceConfigBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.vinterface.info.VinterfaceInputFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.vinterface.info.VinterfaceInputFilterBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.vinterface.info.VinterfaceOutputFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.vinterface.info.VinterfaceOutputFilterBuilder;

/**
 * {@code XmlVInterface} provides XML binding to the data model for
 * virtual interface.
 */
@XmlRootElement(name = "vinterface")
@XmlAccessorType(XmlAccessType.NONE)
public final class XmlVInterface extends XmlVNode {
    /**
     * A boolean value which determines whether the virtual interface is to
     * be enabled or not.
     */
    @XmlElement
    private boolean  enabled;

    /**
     * Port mapping configuration.
     */
    @XmlElement(name = "port-map")
    private VTNPortMapConfig  portMap;

    /**
     * A list of flow filters applied to packets received from this virtual
     * interface.
     */
    @XmlElement(name = "input-filters")
    private FlowFilterList  inputFilters;

    /**
     * A list of flow filters applied to packets transmitted from this virtual
     * interface.
     */
    @XmlElement(name = "output-filters")
    private FlowFilterList  outputFilters;

    /**
     * Convert the given virtual interface list into a list of
     * {@link XmlVInterface} instances.
     *
     * @param list  A list of {@link Vinterface} instance.
     * @return  A list of {@link XmlVInterface} instance if the given list
     *          contains at least one virtual interface.
     *          {@code null} otherwise.
     * @throws RpcException
     *    The given list contains instance instance.
     */
    public static List<XmlVInterface> toList(List<Vinterface> list)
        throws RpcException {
        List<XmlVInterface> xlist;
        if (list == null || list.isEmpty()) {
            xlist = null;
        } else {
            xlist = new ArrayList<>(list.size());
            for (Vinterface vif: list) {
                xlist.add(new XmlVInterface(vif));
            }
        }

        return xlist;
    }

    /**
     * Private constructor only for JAXB.
     */
    @SuppressWarnings("unused")
    private XmlVInterface() {
        enabled = true;
    }

    /**
     * Construct a new instance.
     *
     * @param vif  A {@link VtnMappableVinterface} instance.
     * @throws RpcException
     *    The given instance is invalid.
     */
    public XmlVInterface(VtnMappableVinterface vif) throws RpcException {
        super(vif.getName());

        VinterfaceConfig vifc = vif.getVinterfaceConfig();
        setDescription(vifc.getDescription());

        Boolean en = vifc.isEnabled();
        enabled = (en == null || en.booleanValue());

        PortMapConfig pmconf = vif.getPortMapConfig();
        if (pmconf != null) {
            portMap = new VTNPortMapConfig(pmconf);
        }

        inputFilters =
            FlowFilterList.create(vif.getVinterfaceInputFilter(), true);
        outputFilters =
            FlowFilterList.create(vif.getVinterfaceOutputFilter(), true);
    }

    /**
     * Return a port mapping configuration if present.
     *
     * @return  A {@link VTNPortMapConfig} instance if port mapping is
     *          configured in this virtual interface.
     *          Otherwise {@code null}.
     */
    public VTNPortMapConfig getPortMap() {
        return portMap;
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
     * Convert this instance into a {@link VinterfaceBuilder} instance.
     *
     * <p>
     *   Note that returned instance contains only configuration.
     *   Runtime status needs to be resumed by the caller.
     * </p>
     *
     * @param xlogger  A {@link XmlLogger} instance.
     * @param ifId     A {@link VInterfaceIdentifier} instance which specifies
     *                 the virtual interface.
     * @return  A {@link VinterfaceBuilder} instance.
     * @throws RpcException
     *    Failed to convert this instance.
     */
    public VinterfaceBuilder toVinterfaceBuilder(XmlLogger xlogger,
                                                 VInterfaceIdentifier<?> ifId)
        throws RpcException {
        checkName(DESCRIPTION);

        VinterfaceConfig vifc = new VinterfaceConfigBuilder().
            setDescription(getDescription()).
            setEnabled(enabled).
            build();
        VinterfaceBuilder builder = new VinterfaceBuilder().
            setName(getName()).
            setVinterfaceConfig(vifc);

        VTNPortMapConfig pmc = portMap;
        if (pmc != null) {
            pmc.verify();
            builder.setPortMapConfig(pmc.toPortMapConfig());
        }

        List<VtnFlowFilter> vflist = toVtnFlowFilterList(
            xlogger, ifId, inputFilters, false);
        if (vflist != null) {
            VinterfaceInputFilter in = new VinterfaceInputFilterBuilder().
                setVtnFlowFilter(vflist).build();
            builder.setVinterfaceInputFilter(in);
        }

        vflist = toVtnFlowFilterList(xlogger, ifId, outputFilters, true);
        if (vflist != null) {
            VinterfaceOutputFilter out = new VinterfaceOutputFilterBuilder().
                setVtnFlowFilter(vflist).build();
            builder.setVinterfaceOutputFilter(out);
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
            XmlVInterface xvif = (XmlVInterface)o;
            ret = (enabled == xvif.enabled &&
                   Objects.equals(inputFilters, xvif.inputFilters) &&
                   Objects.equals(outputFilters, xvif.outputFilters) &&
                   Objects.equals(portMap, xvif.portMap));
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
        return super.hashCode() * HASH_PRIME +
            Objects.hash(enabled, inputFilters, outputFilters, portMap);
    }
}
