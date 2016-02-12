/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
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

import org.opendaylight.vtn.manager.internal.routing.xml.XmlPathMap;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.flow.FlowUtils;
import org.opendaylight.vtn.manager.internal.util.flow.filter.FlowFilterList;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeType;
import org.opendaylight.vtn.manager.internal.util.vnode.VTenantIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.vtn.path.map.list.VtnPathMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.VtnInfo;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtn.info.VtenantConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtn.info.VtenantConfigBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtn.info.VtnInputFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtn.info.VtnInputFilterBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtn.info.VtnPathMaps;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.VtnBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.Vterminal;

/**
 * {@code XmlVTenant} provides XML binding to the data model for VTN.
 */
@XmlRootElement(name = "vtn")
@XmlAccessorType(XmlAccessType.NONE)
public final class XmlVTenant extends XmlVNode {
    /**
     * Idle timeout value for flow entries.
     */
    @XmlElement(name = "idle-timeout", required = true)
    private Integer  idleTimeout;

    /**
     * Hard timeout value for flow entries.
     */
    @XmlElement(name = "hard-timeout", required = true)
    private Integer  hardTimeout;

    /**
     * A list of vBridges.
     */
    @XmlElementWrapper(name = "vbridges")
    @XmlElement(name = "vbridge")
    private List<XmlVBridge>  vBridges;

    /**
     * A list of vTerminals.
     */
    @XmlElementWrapper(name = "vterminals")
    @XmlElement(name = "vterminal")
    private List<XmlVTerminal>  vTerminals;

    /**
     * A list of VTN path maps.
     */
    @XmlElementWrapper(name = "vtn-path-maps")
    @XmlElement(name = "vtn-path-map")
    private List<XmlPathMap>  pathMaps;

    /**
     * A list of flow filters applied to packets mapped to this VTN.
     */
    @XmlElement(name = "input-filters")
    private FlowFilterList  inputFilters;

    /**
     * Identifier for this VTN.
     */
    private VTenantIdentifier  tenantId;

    /**
     * Private constructor only for JAXB.
     */
    @SuppressWarnings("unused")
    private XmlVTenant() {
    }

    /**
     * Construct a new instance.
     *
     * @param vtn  A {@link VtnInfo} instance.
     * @throws RpcException  An error occurred.
     */
    public XmlVTenant(VtnInfo vtn) throws RpcException {
        super(vtn.getName());

        VtenantConfig vtconf = vtn.getVtenantConfig();
        setDescription(vtconf.getDescription());
        idleTimeout = vtconf.getIdleTimeout();
        hardTimeout = vtconf.getHardTimeout();

        List<Vbridge> vblist = vtn.getVbridge();
        if (vblist != null && !vblist.isEmpty()) {
            List<XmlVBridge> xvblist = new ArrayList<>(vblist.size());
            for (Vbridge vbr: vblist) {
                xvblist.add(new XmlVBridge(vbr));
            }
            vBridges = xvblist;
        }

        List<Vterminal> vtlist = vtn.getVterminal();
        if (vtlist != null && !vtlist.isEmpty()) {
            List<XmlVTerminal> xvtlist = new ArrayList<>(vtlist.size());
            for (Vterminal vtm: vtlist) {
                xvtlist.add(new XmlVTerminal(vtm));
            }
            vTerminals = xvtlist;
        }

        initPathMaps(vtn);
        inputFilters = FlowFilterList.create(vtn.getVtnInputFilter(), true);
    }

    /**
     * Return the identifier for this VTN.
     *
     * @return  A {@link VTenantIdentifier} instance.
     */
    public VTenantIdentifier getIdentifier() {
        VTenantIdentifier vtnId = tenantId;
        if (vtnId == null) {
            vtnId = new VTenantIdentifier(getName());
            tenantId = vtnId;
        }

        return vtnId;
    }

    /**
     * Create a {@link VtnBuilder} instance that contains the VTN
     * configuration.
     * <p>
     *   Note that returned instance contains only configuration for the
     *   VTN. Other information needs to be resumed by the caller.
     * </p>
     * <ul>
     *   <li>Runtime status</li>
     *   <li>vBridge</li>
     *   <li>vTerminal</li>
     *   <li>VTN path map</li>
     * </ul>
     *
     * @param xlogger  A {@link XmlLogger} instance.
     * @param name     The expected name of this VTN.
     * @return  A {@link VtnBuilder} instance.
     * @throws RpcException  An error occurred.
     */
    public VtnBuilder toVtnBuilder(XmlLogger xlogger, String name)
        throws RpcException {
        VnodeName vname = checkName(VNodeType.VTN.toString());
        String tname = vname.getValue();
        if (!name.equals(tname)) {
            throw RpcException.getBadArgumentException(
                name + ": Unexpected VTN name: " + tname);
        }

        FlowUtils.verifyFlowTimeout(idleTimeout, hardTimeout, true);
        VtenantConfig vtconf = new VtenantConfigBuilder().
            setDescription(getDescription()).
            setIdleTimeout(idleTimeout).
            setHardTimeout(hardTimeout).
            build();

        VtnBuilder builder = new VtnBuilder().
            setName(getName()).
            setVtenantConfig(vtconf);
        List<VtnFlowFilter> vflist =
            toVtnFlowFilterList(xlogger, getIdentifier(), inputFilters, false);
        if (vflist != null) {
            VtnInputFilter in = new VtnInputFilterBuilder().
                setVtnFlowFilter(vflist).build();
            builder.setVtnInputFilter(in);
        }

        return builder;
    }

    /**
     * Return a list of vBridges.
     *
     * @return  A list of {@link XmlVBridge} instances or {@code null}.
     */
    public List<XmlVBridge> getBridges() {
        return (vBridges == null || vBridges.isEmpty()) ? null : vBridges;
    }

    /**
     * Return a list of vTerminals.
     *
     * @return  A list of {@link XmlVTerminal} instances or {@code null}.
     */
    public List<XmlVTerminal> getTerminals() {
        return (vTerminals == null || vTerminals.isEmpty())
            ? null : vTerminals;
    }

    /**
     * Return a list of VTN path map configurations.
     *
     * @return  A list of {@link XmlPathMap} instances or {@code null}.
     */
    public List<XmlPathMap> getPathMaps() {
        return (pathMaps == null || pathMaps.isEmpty()) ? null : pathMaps;
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
     * Initialize VTN path map list.
     *
     * @param vtn  A {@link VtnInfo} instance.
     * @throws RpcException  An error occurred.
     */
    private void initPathMaps(VtnInfo vtn) throws RpcException {
        VtnPathMaps root = vtn.getVtnPathMaps();
        if (root == null) {
            return;
        }

        List<VtnPathMap> vlist = root.getVtnPathMap();
        if (vlist == null || vlist.isEmpty()) {
            return;
        }

        List<XmlPathMap> list = new ArrayList<>(vlist.size());
        pathMaps = list;
        for (VtnPathMap vpm: vlist) {
            list.add(new XmlPathMap(vpm));
        }
    }

    /**
     * Determine whether the given instance contains the same flow timeout
     * configuration as this instance.
     *
     * @param xvtn  A {@link XmlVTenant} instance to be compared.
     * @return  {@code true} only if the given instance contains the same
     *          flow timeout configuration.
     */
    private boolean equalsFlowTimeouts(XmlVTenant xvtn) {
        return (Objects.equals(idleTimeout, xvtn.idleTimeout) &&
                Objects.equals(hardTimeout, xvtn.hardTimeout));
    }

    /**
     * Determine whether the given instance contains the same vBridge and
     * vTerminal list as this instance.
     *
     * @param xvtn  A {@link XmlVTenant} instance to be compared.
     * @return  {@code true} only if the given instance contains the same
     *          vBridges and vTerminals.
     */
    private boolean equalsBridges(XmlVTenant xvtn) {
        return (MiscUtils.equalsAsSet(vBridges, xvtn.vBridges) &&
                MiscUtils.equalsAsSet(vTerminals, xvtn.vTerminals));
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
            XmlVTenant xvtn = (XmlVTenant)o;
            if (equalsFlowTimeouts(xvtn) && equalsBridges(xvtn)) {
                ret = (Objects.equals(pathMaps, xvtn.pathMaps) &&
                       Objects.equals(inputFilters, xvtn.inputFilters));
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
            Objects.hash(idleTimeout, hardTimeout, inputFilters);

        if (vBridges != null) {
            for (XmlVBridge xvbr: vBridges) {
                h += xvbr.hashCode();
            }
        }

        if (vTerminals != null) {
            for (XmlVTerminal xvtm: vTerminals) {
                h += xvtm.hashCode();
            }
        }

        return h;
    }
}
