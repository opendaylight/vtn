/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.vnode;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;
import java.util.Objects;
import java.util.regex.Pattern;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;
import javax.xml.bind.annotation.XmlSeeAlso;

import com.google.common.base.Optional;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;
import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier.InstanceIdentifierBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.VtnFlowFilterList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilterKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.result.FlowFilterResult;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.result.FlowFilterResultBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.virtual.route.info.VirtualNodePath;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.virtual.route.info.VirtualNodePathBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.Vtns;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtn.info.VtenantConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.Vtn;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.VtnKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodePathFields;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * {@code VNodeIdentifier} describes an identifier for the virtual node.
 *
 * @param <T>  The type of the target data model.
 */
@XmlRootElement(name = "vnode-path")
@XmlAccessorType(XmlAccessType.NONE)
@XmlSeeAlso({VBridgeIfIdentifier.class, VTerminalIfIdentifier.class})
public abstract class VNodeIdentifier<T extends DataObject> {
    /**
     * A bit which indicates a VTN or a virtual node inside VTN.
     */
    public static final int  TYPEBIT_VTN = 0x1;

    /**
     * A bit which indicates a vBridge or a virtual node inside vBridge.
     */
    public static final int  TYPEBIT_VBRIDGE = 0x2;

    /**
     * A bit which indicates a vTerminal or a virtual node inside
     * vTerminal.
     */
    public static final int  TYPEBIT_VTERMINAL = 0x4;

    /**
     * A bit which indicates a virtual interface.
     */
    public static final int  TYPEBIT_VINTERFACE = 0x8;

    /**
     * A bit which indicates a VLAN mapping configured in a vBridge.
     */
    public static final int  TYPEBIT_VLANMAP = 0x10;

    /**
     * A bit which indicates a MAC mapping configured in a vBridge.
     */
    public static final int  TYPEBIT_MACMAP = 0x20;

    /**
     * A bit which indicates a host mapped by a MAC mapping configured in
     * a vBridge.
     */
    public static final int  TYPEBIT_MACMAP_HOST = 0x40;

    /**
     * A pseudo path component which represents a {@code null}.
     */
    static final String  COMP_NULL = "<null>";

    /**
     * A separator between virtual node type and path components.
     */
    private static final String  SEP_TYPE = ":";

    /**
     * A separator for path components.
     */
    private static final String  SEP_COMPONENT = "/";

    /**
     * A regular expression which matches {@link #SEP_COMPONENT}.
     */
    private static final Pattern  SEP_COMPONENT_PAT =
        Pattern.compile(SEP_COMPONENT);

    /**
     * The name of the VTN.
     */
    @XmlElement(name = "tenant-name", required = true)
    private VnodeName  tenantName;

    /**
     * The name of the bridge, vBridge or vTerminal.
     *
     * <p>
     *   {@code null} is set if this instance does not specify the virtual
     *   bridge inside VTN.
     * </p>
     */
    @XmlElement(name = "bridge-name")
    private VnodeName  bridgeName;

    /**
     * The name of the virtual interface.
     *
     * <p>
     *   {@code null} is set if this instance does not specify the virtual
     *   interface.
     * </p>
     */
    @XmlElement(name = "interface-name")
    private VnodeName  interfaceName;

    /**
     * Cache for instance identifier.
     */
    private InstanceIdentifier<T>  targetPath;

    /**
     * Cache for a string returned by {@link #toString()}.
     */
    private String  identString;

    /**
     * Cache for a path component list returned by {@link #getComponents()}.
     */
    private List<String>  pathComponents;

    /**
     * Cache for a hash code of this instance.
     */
    private int  hashValue;

    /**
     * Construct a new instance from a string representation of
     * {@link VNodeIdentifier} instance.
     *
     * @param str  A string representation of {@link VNodeIdentifier} instance
     *             returned by the call of {@link VNodeIdentifier#toString()}.
     * @return  A {@link VNodeIdentifier} instance.
     *          Note that {@code null} is returned if {@code str} is
     *          {@code null}.
     * @throws IllegalArgumentException
     *    An invalid string is passed to {@code str}.
     */
    public static final VNodeIdentifier<?> create(String str) {
        if (str == null) {
            return null;
        }

        int typeIdx = str.indexOf(SEP_TYPE);
        Throwable cause = null;
        if (typeIdx > 0) {
            try {
                String typeStr = str.substring(0, typeIdx);
                VNodeType type = VNodeType.checkedForName(typeStr);
                String compStr = str.substring(typeIdx + 1);
                String[] comps = SEP_COMPONENT_PAT.split(compStr, -1);
                if (comps.length == type.getComponentSize()) {
                    // The given string should be equal to a string
                    // representation of VNodeIdentifier.
                    VNodeIdentifier<?> ident = type.newIdentifier(comps);
                    ident.identString = str;

                    return ident;
                }
            } catch (RuntimeException e) {
                cause = e;
            }
        }

        throw new IllegalArgumentException(
            "Invalid identifier format: " + str, cause);
    }

    /**
     * Construct a new instance from a string representation of
     * {@link VNodeIdentifier} instance with type checking.
     *
     * @param str   A string representation of {@link VNodeIdentifier} instance
     *              returned by the call of {@link VNodeIdentifier#toString()}.
     *              Unlike {@link #create(String)}, {@code null} cannot be
     *              passed.
     * @param type  A class which specifies the expected type of
     *              {@link VNodeIdentifier}.
     * @param <T>   The type of {@link VNodeIdentifier}.
     * @return  A {@link VNodeIdentifier} instance.
     * @throws IllegalArgumentException
     *    An invalid string is passed to {@code str}.
     * @throws IllegalStateException
     *    The type of the VNode identifier constructed from {@code str} does
     *    not match the specified type.
     */
    public static final <T extends VNodeIdentifier<?>> T create(
        String str, Class<T> type) {
        VNodeIdentifier<?> ident = create(str);
        if (type.isInstance(ident)) {
            return type.cast(ident);
        }

        throw new IllegalStateException(
            "Unexpected VNode identifier type: " + ident);
    }

    /**
     * Construct a new instance from the given {@link VnodePathFields}
     * instance.
     *
     * @param vpath  A {@link VnodePathFields} instance.
     * @param find   {@code true} indicates the given name is used for
     *               finding existing virtual node.
     * @return  A {@link VNodeIdentifier} instance if {@code vpath} is not
     *          {@code null}. {@code null} if {@code vpath} is {@code null}.
     * @throws RpcException
     *    {@code vpath} contains invalid value.
     */
    public static VNodeIdentifier<?> create(
        VnodePathFields vpath, boolean find) throws RpcException {
        VNodeIdentifier<?> ident;
        if (vpath == null) {
            ident = null;
        } else {
            String tname = vpath.getTenantName();
            String iname = vpath.getInterfaceName();
            String bname = vpath.getBridgeName();
            if (bname != null) {
                ident = (iname == null)
                    ? VBridgeIdentifier.create(tname, bname, find)
                    : VBridgeIfIdentifier.create(tname, bname, iname, find);
            } else {
                String vtname = vpath.getTerminalName();
                if (vtname == null) {
                    ident = createTenantIdentifier(tname, iname, find);
                } else {
                    ident = (iname == null)
                        ? VTerminalIdentifier.create(tname, vtname, find)
                        : VTerminalIfIdentifier.create(tname, vtname, iname,
                                                       find);
                }
            }
        }

        return ident;
    }

    /**
     * Return an exception that notifies an invalid virtual node path.
     *
     * @param o  An object which indicates the cause of error.
     * @return  An {@link RpcException} instance.
     */
    public static final RpcException getBadPathException(Object o) {
        return RpcException.getBadArgumentException(
            "Unexpected virtual node path: " + o);
    }

    /**
     * Return an exception that indicates the virtual node does not support
     * flow filter.
     *
     * @param ident  A {@link VNodeIdentifier} instance.
     * @return  An {@link RpcException} instance.
     */
    public static final RpcException getFlowFilterUnsupportedException(
        VNodeIdentifier<?> ident) {
        String msg = ident.toString() + ": " +
            ident.getType().getDescription() +
            " does not support flow filter.";
        return RpcException.getBadArgumentException(msg);
    }

    /**
     * Create a new identifier for the VTN.
     *
     * <p>
     *   This method is called by {@link #create(VnodePathFields, boolean)}
     *   if the specified vnode-path-fields does not specify the name of the
     *   virtual bridge.
     * </p>
     *
     * @param tname  The name of the VTN specified by vnode-path-fields.
     * @param iname  The name of the virtual interface specified by
     *               vnode-path-fields.
     * @param find   {@code true} indicates the given name is used for
     *               finding existing virtual node.
     * @return  A {@link VTenantIdentifier} instance that specifies the
     *          location of the VTN.
     * @throws RpcException  An error occurred.
     */
    private static VTenantIdentifier createTenantIdentifier(
        String tname, String iname, boolean find) throws RpcException {
        VTenantIdentifier ident;
        if (iname == null) {
            ident = VTenantIdentifier.create(tname, find);
        } else {
            // No virtual bridge name is specified though virtual interface
            // name is specified. This should be treated as an invalid path.
            if (tname == null) {
                VNodeType.VTN.checkName(null, find);
            }

            throw RpcException.getNullArgumentException("Virtual bridge name");
        }

        return ident;
    }

    /**
     * Default constructor only for JAXB.
     */
    VNodeIdentifier() {
    }

    /**
     * Construct a new instance.
     *
     * @param tname  The name of the VTN.
     * @param bname  The name of the virtual bridge.
     * @param iname  The name of the virtual interface.
     */
    VNodeIdentifier(VnodeName tname, VnodeName bname, VnodeName iname) {
        tenantName = tname;
        bridgeName = bname;
        interfaceName = iname;
    }

    /**
     * Construct a new instance.
     *
     * @param tname  The name of the VTN.
     * @param bname  The name of the virtual bridge.
     * @param iname  The name of the virtual interface.
     */
    VNodeIdentifier(String tname, VnodeName bname, VnodeName iname) {
        tenantName = new VnodeName(tname);
        bridgeName = bname;
        interfaceName = iname;
    }

    /**
     * Return the name of the VTN configured in this instance.
     *
     * @return  A {@link VnodeName} instance which contains the VTN name.
     *          {@code null} if not configured.
     */
    public final VnodeName getTenantName() {
        return tenantName;
    }

    /**
     * Return the name of the VTN configured in this instance.
     *
     * @return  A string which indicates the VTN name.
     *          {@code null} if not configured.
     */
    public final String getTenantNameString() {
        return (tenantName == null) ? null : tenantName.getValue();
    }

    /**
     * Return the name of the virtual bridge configured in this instance.
     *
     * @return  A {@link VnodeName} instance which contains the virtual bridge
     *          name. {@code null} if not configured.
     */
    public final VnodeName getBridgeName() {
        return bridgeName;
    }

    /**
     * Return the name of the virtual bridge configured in this instance.
     *
     * @return  A string which indicates the virtual bridge name.
     *          {@code null} if not configured.
     */
    public final String getBridgeNameString() {
        return (bridgeName == null) ? null : bridgeName.getValue();
    }

    /**
     * Return the name of the virtual interface configured in this instance.
     *
     * @return  A {@link VnodeName} instance which contains the virtual
     *          interface name. {@code null} if not configured.
     */
    public final VnodeName getInterfaceName() {
        return interfaceName;
    }

    /**
     * Return the name of the virtual interface configured in this instance.
     *
     * @return  A string which indicates the virtual interface name.
     *          {@code null} if not configured.
     */
    public final String getInterfaceNameString() {
        return (interfaceName == null) ? null : interfaceName.getValue();
    }

    /**
     * Return an indentifier for the MD-SAL data model associated with this
     * instance.
     *
     * @return  An {@link InstanceIdentifier} instance.
     */
    public final InstanceIdentifier<T> getIdentifier() {
        InstanceIdentifier<T> path = targetPath;
        if (path == null) {
            path = getIdentifierBuilder().build();
            targetPath = path;
        }

        return path;
    }

    /**
     * Return a list which contains all path components configured in this
     * instance.
     *
     * @return  A list of path components.
     */
    public final List<String> getComponents() {
        List<String> comps = pathComponents;
        if (comps == null) {
            comps = Collections.unmodifiableList(newComponents());
            pathComponents = comps;
        }

        return comps;
    }

    /**
     * Determine whether the data model associated with this instance contains
     * the data model specified by the given instance.
     *
     * @param ident  A {@link VNodeIdentifier} instance.
     * @return  {@code true} only if the data model associated with this
     *          instance contains the data model specified by {@code ident}.
     */
    public final boolean contains(VNodeIdentifier<?> ident) {
        if (ident == null || !getType().contains(ident.getType())) {
            return false;
        }

        Iterator<String> it = ident.getComponents().iterator();
        for (String comp: getComponents()) {
            if (!it.hasNext()) {
                return false;
            }

            if (!comp.equals(it.next())) {
                return false;
            }
        }

        return true;
    }

    /**
     * Determine whether the virtua node specified by this instance contains
     * the virtual node specified by {@link VirtualNodePath} instance.
     *
     * <p>
     *   Note that this method returns {@code true} if the given
     *   {@link VirtualNodePath} specifies the same virtual node as specified
     *   by this instance.
     * </p>
     * <p>
     *   This method in this class checks only the VTN name.
     *   Subclass may override this method.
     * </p>
     *
     * @param vpath  A {@link VirtualNodePath} instance that specifies the
     *               virtual node. {@code null} cannot be specified.
     * @return  {@code true} if the virtual node specified by this instance
     *          contains the virtual node specified by {@code vpath}.
     *          {@code false} otherwise.
     */
    public boolean contains(VirtualNodePath vpath) {
        return tenantName.getValue().equals(vpath.getTenantName());
    }

    /**
     * Return a {@link VirtualNodePath} instance that specifies the same
     * virtual node as this instance.
     *
     * @return  A {@link VirtualNodePath} instance.
     */
    public final VirtualNodePath getVirtualNodePath() {
        return getVirtualNodePathBuilder().build();
    }

    /**
     * Read the configuration for the VTN specified by this instance.
     *
     * @param rtx  A read transaction.
     * @return  An {@link Optional} instance that contains the result.
     * @throws VTNException  An error occurred.
     */
    public final Optional<VtenantConfig> readVtenantConfig(ReadTransaction rtx)
        throws VTNException {
        InstanceIdentifier<VtenantConfig> path = getVtnIdentifierBuilder().
            child(VtenantConfig.class).build();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        return DataStoreUtils.read(rtx, oper, path);
    }

    /**
     * Read the data object specified by this instance.
     *
     * @param rtx  A read transaction.
     * @return  An {@link Optional} instance that contains the result.
     * @throws VTNException  An error occurred.
     */
    public final Optional<T> read(ReadTransaction rtx) throws VTNException {
        InstanceIdentifier<T> path = getIdentifier();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        return DataStoreUtils.read(rtx, oper, path);
    }

    /**
     * Read the data object specified by this instance.
     *
     * <p>
     *   This method throws an {@link RpcException} if the data object is
     *   not present.
     * </p>
     *
     * @param rtx  A read transaction.
     * @return  The data object specified by this instance.
     * @throws VTNException  An error occurred.
     */
    public final T fetch(ReadTransaction rtx) throws VTNException {
        Optional<T> opt = read(rtx);
        if (opt.isPresent()) {
            return opt.get();
        }

        throw getNotFoundException();
    }

    /**
     * Return an instance identifier that specifies a flow filter.
     *
     * @param output  {@code true} indicates the output filter.
     *                {@code false} indicates the input filter.
     * @param index   A flow filter index.
     * @return  An instance identifier for the specified flow filter.
     * @throws RpcException
     *    The virtual node specified by this instance does not support
     *    flow filter.
     */
    public final InstanceIdentifier<VtnFlowFilter> getFlowFilterIdentifier(
        boolean output, Integer index) throws RpcException {
        return getFlowFilterListIdentifierBuilder(output).
            child(VtnFlowFilter.class, new VtnFlowFilterKey(index)).
            build();
    }

    /**
     * Delete all the flow filters in the specified flow filter list.
     *
     * @param tx      A MD-SAL datastore transaction.
     * @param output  {@code true} indicates the output filter list.
     *                {@code false} indicates the input filter list.
     * @return  A list of {@link FlowFilterResult} instances that contains
     *          all the removed flow filter indices.
     *          {@code null} if the specified flow filter list is empty.
     * @throws VTNException  An error occurred.
     */
    public final List<FlowFilterResult> clearFlowFilter(
        ReadWriteTransaction tx, boolean output) throws VTNException {
        InstanceIdentifier<? extends VtnFlowFilterList> path =
            getFlowFilterListIdentifierBuilder(output).build();

        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        Optional<? extends VtnFlowFilterList> opt =
            DataStoreUtils.read(tx, oper, path);
        List<FlowFilterResult> list = null;
        if (opt.isPresent()) {
            List<VtnFlowFilter> filters = opt.get().getVtnFlowFilter();
            if (!MiscUtils.isEmpty(filters)) {
                list = new ArrayList<>(filters.size());
                for (VtnFlowFilter vff: filters) {
                    FlowFilterResult res = new FlowFilterResultBuilder().
                        setIndex(vff.getIndex()).
                        setStatus(VtnUpdateType.REMOVED).
                        build();
                    list.add(res);
                }
            }

            tx.delete(oper, path);
        }

        return list;
    }

    /**
     * Return an exception that indicates the virtual node specified by this
     * instance is not present.
     *
     * @return  An {@link RpcException} instance.
     */
    public final RpcException getNotFoundException() {
        return getType().getNotFoundException(toString());
    }

    /**
     * Return an exception that indicates the virtual node specified by this
     * instance is already present.
     *
     * @return  An {@link RpcException} instance.
     */
    public final RpcException getDataExistsException() {
        return getType().getDataExistsException(toString());
    }

    /**
     * Return an instance identifier builder which contains the VTN key.
     *
     * @return  An instance identifier builder.
     */
    protected final InstanceIdentifierBuilder<Vtn> getVtnIdentifierBuilder() {
        return InstanceIdentifier.builder(Vtns.class).
            child(Vtn.class, new VtnKey(tenantName));
    }

    /**
     * Create a new {@link VirtualNodePathBuilder} instance that contains the
     * path components configured in this instance.
     *
     * <p>
     *   This method in this class sets only the VTN name.
     *   Subclass may override this method.
     * </p>
     *
     * @return  A {@link VirtualNodePathBuilder} instance.
     */
    protected VirtualNodePathBuilder getVirtualNodePathBuilder() {
        return new VirtualNodePathBuilder().
            setTenantName(tenantName.getValue());
    }

    /**
     * Return an instance identifier builder rooted at the specified container
     * for the flow filter list.
     *
     * @param output  {@code true} indicates the output filter list.
     *                {@code false} indicates the input filter list.
     * @return  An instance identifier builder rooted at the specified
     *          container for the flow filter list.
     * @throws RpcException
     *    The virtual node specified by this instance does not support
     *    flow filter.
     */
    protected InstanceIdentifierBuilder<? extends VtnFlowFilterList> getFlowFilterListIdentifierBuilder(
        boolean output) throws RpcException {
        throw getFlowFilterUnsupportedException(this);
    }

    /**
     * Return a {@link VNodeType} instance which indicates the type of the
     * virtual node.
     *
     * @return  A {@link VNodeType} instance.
     */
    public abstract VNodeType getType();

    /**
     * Return an instance identifier builder rooted at the virtual node
     * specified by this instance.
     *
     * @return  An {@link InstanceIdentifierBuilder} instance.
     */
    public abstract InstanceIdentifierBuilder<T> getIdentifierBuilder();

    /**
     * Return a {@link VNodeIdentifier} that specifies a virtual network
     * element.
     *
     * <p>
     *   If this instance specifies a virtual network element, such as VTN and
     *   vBridge, this method returns this instance.
     *   If this instance specifies a virtual network mapping, such as VLAN
     *   mapping, this method returns a {@link VNodeIdentifier} instance
     *   that specfies a virtual network element in which contains that
     *   mapping.
     * </p>
     *
     * @return  A {@link VNodeIdentifier} instance that specifies a virtual
     *          network element.
     */
    public abstract VNodeIdentifier<?> getVNodeIdentifier();

    /**
     * Create a new list which contains all path components configured in this
     * instance.
     *
     * @return  A list of path components.
     */
    protected abstract List<String> newComponents();

    // Object

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public final boolean equals(Object o) {
        boolean ret = (o == this);
        if (!ret && o != null && getClass().equals(o.getClass())) {
            VNodeIdentifier<?> ident = (VNodeIdentifier<?>)o;
            ret = getComponents().equals(ident.getComponents());
        }

        return ret;
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public final int hashCode() {
        int h = hashValue;
        if (h == 0) {
            h = Objects.hash(getClass(), getComponents());
            hashValue = h;
        }

        return h;
    }

    /**
     * Return a string representation of this instance.
     *
     * @return  A string representation of this instance.
     */
    @Override
    public final String toString() {
        String str = identString;
        if (str == null) {
            StringBuilder builder = new StringBuilder(getType().toString()).
                append(SEP_TYPE);
            String sep = "";
            for (String comp: getComponents()) {
                if (comp == null) {
                    comp = COMP_NULL;
                }
                builder.append(sep).append(comp);
                sep = SEP_COMPONENT;
            }

            str = builder.toString();
            identString = str;
        }

        return str;
    }
}
