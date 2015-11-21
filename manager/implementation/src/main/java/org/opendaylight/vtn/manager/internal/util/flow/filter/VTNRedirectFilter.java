/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.filter;

import static org.opendaylight.vtn.manager.util.NumberUtils.HASH_PRIME;

import java.util.Objects;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlElements;
import javax.xml.bind.annotation.XmlRootElement;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.ErrorVNodePath;
import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VInterfacePath;
import org.opendaylight.vtn.manager.VTerminalIfPath;
import org.opendaylight.vtn.manager.flow.filter.FlowFilter;
import org.opendaylight.vtn.manager.flow.filter.RedirectFilter;

import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.log.VTNLogLevel;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIfIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VInterfaceIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeType;
import org.opendaylight.vtn.manager.internal.util.vnode.VTerminalIfIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.VtnFlowFilterConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.VtnRedirectFilterCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.VtnRedirectFilterCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.vtn.redirect.filter._case.VtnRedirectFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.vtn.redirect.filter._case.VtnRedirectFilterBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;

/**
 * {@code VTNRedirectFilter} describes configuration information about flow
 * filter which forwards packets to another virtual interface.
 */
@XmlRootElement(name = "vtn-redirect-filter")
@XmlAccessorType(XmlAccessType.NONE)
public final class VTNRedirectFilter extends VTNFlowFilter {
    /**
     * Logger instance.
     */
    static final Logger  LOG =
        LoggerFactory.getLogger(VTNRedirectFilter.class);

    /**
     * The location of the destination virtual interface.
     */
    @XmlElements({
        @XmlElement(name = "vbridge-if", type = VBridgeIfIdentifier.class),
        @XmlElement(name = "vterminal-if", type = VTerminalIfIdentifier.class)})
    private VInterfaceIdentifier<?>  destination;

    /**
     * Determine the direction of the packet redirection.
     *
     * {@code true} means that the packet is redirected as outgoing packet.
     */
    @XmlElement
    private boolean  output;

    /**
     * Private constructor only for JAXB.
     */
    private VTNRedirectFilter() {
    }

    /**
     * Construct a new instance.
     *
     * @param vffc  A {@link VtnFlowFilterConfig} instance which contains
     *              flow filter configuration.
     * @param vrfc  A {@link VtnRedirectFilterCase} instance configured in
     *              {@code vffc}.
     * @throws RpcException
     *    {@code vffc} contains invalid value.
     */
    VTNRedirectFilter(VtnFlowFilterConfig vffc, VtnRedirectFilterCase vrfc)
        throws RpcException {
        super(vffc);

        VtnRedirectFilter vrf = vrfc.getVtnRedirectFilter();
        if (vrf == null) {
            throw RpcException.getNullArgumentException("vtn-redirect-filter");
        }

        destination =
            VInterfaceIdentifier.create(vrf.getRedirectDestination());
        output = Boolean.TRUE.equals(vrf.isOutput());
    }

    /**
     * Construct a new instance.
     *
     * @param idx     An index number to be assigned to the flow filter.
     * @param filter  A {@link FlowFilter} instance which contains flow filter
     *                configuration.
     * @param rdf     A {@link RedirectFilter} instance configured in
     *                {@code filter}.
     * @throws RpcException
     *    {@code vffc} contains invalid value.
     */
    VTNRedirectFilter(int idx, FlowFilter filter, RedirectFilter rdf)
        throws RpcException {
        super(idx, filter);

        VInterfacePath path = rdf.getDestination();
        if (path == null) {
            throw RpcException.getNullArgumentException(
                "RedirectFilter: Destination");
        }

        if (path instanceof ErrorVNodePath) {
            ErrorVNodePath epath = (ErrorVNodePath)path;
            throw RpcException.getBadArgumentException(epath.getError());
        }

        // Ignore VTN name in the destination because the VTN name is always
        // determined by the virtual node path.
        VnodeName bname;
        VnodeName iname;
        try {
            bname = MiscUtils.checkName("Virtual node",
                                        path.getTenantNodeName());
            iname = MiscUtils.checkName("Interface", path.getInterfaceName());
        } catch (RpcException e) {
            RpcException re = RpcException.getBadArgumentException(
                "RedirectFilter: Invalid destination: " + e.getMessage());
            re.initCause(e);
            throw re;
        }

        destination = (path instanceof VBridgeIfPath)
            ? new VBridgeIfIdentifier(null, bname, iname)
            : new VTerminalIfIdentifier(null, bname, iname);
        output = rdf.isOutput();
    }

    // VTNFlowFilter

    /**
     * Determine whether this flow filter can be configured into the specified
     * virtual node.
     *
     * @param ident  A {@link VNodeIdentifier} instance which specifies the
     *               target virtual node.
     * @throws RpcException
     *    This flow filter cannot be configured into the virtual node specified
     *    by {@code ident}.
     */
    @Override
    public void canSet(VNodeIdentifier<?> ident) throws RpcException {
        if (ident instanceof VInterfaceIdentifier<?>) {
            VnodeName vtnName = ident.getTenantName();
            VInterfaceIdentifier<?> dst =
                destination.replaceTenantName(vtnName);
            if (ident.contains(dst)) {
                throw RpcException.getBadArgumentException(
                    "Self redirection is not allowed: " + dst);
            }
        }
    }

    /**
     * Determine whether this flow filter supports packet flooding or not.
     *
     * <p>
     *   This method returns {@code false} because REDIRECT filter does not
     *   support broadcast packets.
     * </p>
     *
     * @return  {@code false}.
     */
    @Override
    public boolean isFloodingSuppoted() {
        return false;
    }

    /**
     * Verify the contents of this instance.
     *
     * @throws RpcException  Verification failed.
     */
    @Override
    protected void verifyImpl() throws RpcException {
        VInterfaceIdentifier<?> dest = destination;
        if (dest == null) {
            throw VInterfaceIdentifier.getNoRedirectDestinationException();
        }

        VNodeType btype = dest.getType().getBridgeType();
        MiscUtils.checkPresent(btype.toString(), dest.getBridgeName());
        MiscUtils.checkPresent(VInterfaceIdentifier.DESCRIPTION,
                               dest.getInterfaceName());

        if (dest.getTenantName() != null) {
            // Ignore VTN name in the destination.
            dest = dest.replaceTenantName(null);
            destination = dest;
        }
    }

    /**
     * Return a {@link org.opendaylight.vtn.manager.flow.filter.FilterType}
     * instance which indicates the type of this flow filter.
     *
     * @return  A {@link RedirectFilter} instance.
     */
    @Override
    protected RedirectFilter getFilterType() {
        String bname = destination.getBridgeNameString();
        String iname = destination.getInterfaceNameString();
        VInterfacePath ipath = (destination.getType() == VNodeType.VBRIDGE_IF)
            ? new VBridgeIfPath(null, bname, iname)
            : new VTerminalIfPath(null, bname, iname);

        return ipath.getRedirectFilter(output);
    }

    /**
     * Return a {@link org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.VtnFlowFilterType}
     * instance which indicates the type of this flow filter.
     *
     * @return  A {@link VtnRedirectFilterCase} instance.
     */
    @Override
    protected VtnRedirectFilterCase getVtnFlowFilterType() {
        VtnRedirectFilter vrf = new VtnRedirectFilterBuilder().
            setRedirectDestination(destination.toRedirectDestination()).
            setOutput(output).
            build();
        return new VtnRedirectFilterCaseBuilder().
            setVtnRedirectFilter(vrf).
            build();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected Logger getLogger() {
        return LOG;
    }

    /**
     * Apply this REDIRECT flow filter to the given packet.
     *
     * @param fctx   A flow filter context which contains the packet.
     * @param flid   A {@link FlowFilterListId} instance that specifies the
     *               location of this flow filter list.
     * @throws RedirectFlowException  Always thrown.
     */
    @Override
    protected void apply(FlowFilterContext fctx, FlowFilterListId flid)
        throws RedirectFlowException {
        String fpath = getPathString(flid);
        String cond = getCondition();
        VnodeName vtnName = flid.getIdentifier().getTenantName();
        VInterfaceIdentifier<?> dst = destination.replaceTenantName(vtnName);
        RedirectFlowException e =
            new RedirectFlowException(fpath, cond, dst, output);
        VTNLogLevel level = (fctx.setFirstRedirection(e))
            ? VTNLogLevel.DEBUG : VTNLogLevel.TRACE;
        if (level.isEnabled(LOG)) {
            level.log(LOG, "{}: Redirect packet: cond={}, to={}, " +
                      "direction={}, packet={}",
                      fpath, cond, dst,
                      FlowFilterListId.getFlowDirectionName(output),
                      fctx.getDescription());
        }

        throw e;
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
            VTNRedirectFilter vrdf = (VTNRedirectFilter)o;
            ret = (Objects.equals(destination, vrdf.destination) &&
                   output == vrdf.output);
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
            Objects.hash(destination, output);
    }
}
