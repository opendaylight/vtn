/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.vnode;

import java.util.Map;

import com.google.common.collect.ImmutableMap;

import org.opendaylight.vtn.manager.internal.util.inventory.MacVlan;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier.IdentifiableItem;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier.PathArgument;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.config.DeniedHosts;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.mappable.MacMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.vtn.vlan.mappable.VlanMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.vtn.vlan.mappable.VlanMapKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.Vtn;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.VtnKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.vlan.host.desc.set.VlanHostDescList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.vlan.host.desc.set.VlanHostDescListKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.VbridgeKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.list.Vinterface;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.list.VinterfaceKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.Vterminal;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.VterminalKey;

/**
 * {@code VNodePathConverter} is a utility class that converts YANG
 * instance identifier into {@link VNodeIdentifier} instance.
 */
public final class VNodePathConverter {
    /**
     * A map which keeps path argument parsers.
     */
    private static final Map<Class<?>, ArgumentParser>  PARSERS;

    /**
     * The name of the VTN found in the specified instance identifier.
     */
    private VnodeName  tenantName;

    /**
     * The name of the vBridge found in the specified instance identifier.
     */
    private VnodeName  bridgeName;

    /**
     * The name of the vTerminal found in the specified instance identifier.
     */
    private VnodeName  terminalName;

    /**
     * The name of the virtual interface found in the specified instance
     * identifier.
     */
    private VnodeName  interfaceName;

    /**
     * The VLAN map identifier found in the specified instance identifier.
     */
    private String  vlanMapId;

    /**
     * The host mapped by the MAC mapping found in the specified instance
     * identifier.
     */
    private MacVlan  macMappedHost;

    /**
     * A {@link ArgumentParser} instance that parsed the last path argument
     * in the specified instance identifier.
     */
    private ArgumentParser  lastParser;

    /**
     * A {@link VNodeIdentifier} constructed from the specified instance
     * identifier.
     */
    private final VNodeIdentifier<?>  identifier;

    /**
     * Initialize static fields.
     */
    static {
        PARSERS = ImmutableMap.<Class<?>, ArgumentParser>builder().
            put(Vtn.class, new VTenantParser()).
            put(Vbridge.class, new VBridgeParser()).
            put(Vterminal.class, new VTerminalParser()).
            put(Vinterface.class, new VInterfaceParser()).
            put(VlanMap.class, new VlanMapParser()).
            put(MacMap.class, new MacMapParser()).
            put(VlanHostDescList.class, new MacMapHostParser()).
            build();
    }

    /**
     * Describes an interface to fetch value from path arguments in the
     * specified instance identifier.
     */
    private interface ArgumentParser {
        /**
         * Fetch value from the given path argument, and set value into
         * {@code VNodePathConverter} instance.
         *
         * @param conv  A {@link VNodePathConverter} instance.
         * @param arg   A path argument in the specified instance identifier.
         * @throws RpcException  Failed to fetch value.
         */
        void fetch(VNodePathConverter conv, PathArgument arg)
            throws RpcException;

        /**
         * Create a new {@link VNodeIdentifier} instance.
         *
         * <p>
         *   This method is invoked if this instance parsed the last argument
         *   in the specified instance identifier.
         * </p>
         *
         * @param conv  A {@link VNodePathConverter} instance.
         * @return  A {@link VNodeIdentifier} instance.
         */
        VNodeIdentifier<?> newIdentifier(VNodePathConverter conv);
    }

    /**
     * Parser for the VTN name.
     */
    private static final class VTenantParser implements ArgumentParser {
        /**
         * Fetch the VTN name from the given path argument, and set it into
         * {@code VNodePathConverter} instance.
         *
         * @param conv  A {@link VNodePathConverter} instance.
         * @param arg   A path argument which contains the VTN name.
         */
        @Override
        public void fetch(VNodePathConverter conv, PathArgument arg) {
            @SuppressWarnings("unchecked")
            IdentifiableItem<Vtn, VtnKey> item =
                (IdentifiableItem<Vtn, VtnKey>)arg;
            conv.tenantName = item.getKey().getName();
        }

        /**
         * Create a new {@link VTenantIdentifier} instance.
         *
         * <p>
         *   This method is invoked if this instance parsed the last argument
         *   in the specified instance identifier.
         * </p>
         *
         * @param conv  A {@link VNodePathConverter} instance.
         * @return  A {@link VTenantIdentifier} instance.
         */
        @Override
        public VTenantIdentifier newIdentifier(VNodePathConverter conv) {
            return new VTenantIdentifier(conv.tenantName);
        }
    }

    /**
     * Parser for the vBridge name.
     */
    private static final class VBridgeParser implements ArgumentParser {
        /**
         * Fetch the vBridge name from the given path argument, and set it
         * into {@code VNodePathConverter} instance.
         *
         * @param conv  A {@link VNodePathConverter} instance.
         * @param arg   A path argument which contains the vBridge name.
         */
        @Override
        public void fetch(VNodePathConverter conv, PathArgument arg) {
            @SuppressWarnings("unchecked")
            IdentifiableItem<Vbridge, VbridgeKey> item =
                (IdentifiableItem<Vbridge, VbridgeKey>)arg;
            conv.bridgeName = item.getKey().getName();
        }

        /**
         * Create a new {@link VBridgeIdentifier} instance.
         *
         * <p>
         *   This method is invoked if this instance parsed the last argument
         *   in the specified instance identifier.
         * </p>
         *
         * @param conv  A {@link VNodePathConverter} instance.
         * @return  A {@link VBridgeIdentifier} instance.
         */
        @Override
        public VBridgeIdentifier newIdentifier(VNodePathConverter conv) {
            return new VBridgeIdentifier(conv.tenantName, conv.bridgeName);
        }
    }

    /**
     * Parser for the vTerminal name.
     */
    private static final class VTerminalParser implements ArgumentParser {
        /**
         * Fetch the vTerminal name from the given path argument, and set it
         * into {@code VNodePathConverter} instance.
         *
         * @param conv  A {@link VNodePathConverter} instance.
         * @param arg   A path argument which contains the vTerminal name.
         */
        @Override
        public void fetch(VNodePathConverter conv, PathArgument arg) {
            @SuppressWarnings("unchecked")
            IdentifiableItem<Vterminal, VterminalKey> item =
                (IdentifiableItem<Vterminal, VterminalKey>)arg;
            conv.terminalName = item.getKey().getName();
        }

        /**
         * Create a new {@link VTerminalIdentifier} instance.
         *
         * <p>
         *   This method is invoked if this instance parsed the last argument
         *   in the specified instance identifier.
         * </p>
         *
         * @param conv  A {@link VNodePathConverter} instance.
         * @return  A {@link VTerminalIdentifier} instance.
         */
        @Override
        public VTerminalIdentifier newIdentifier(VNodePathConverter conv) {
            return new VTerminalIdentifier(conv.tenantName, conv.terminalName);
        }
    }

    /**
     * Parser for the virtual interface name.
     */
    private static final class VInterfaceParser implements ArgumentParser {
        /**
         * Fetch the virtual interface name from the given path argument,
         * and set it into {@code VNodePathConverter} instance.
         *
         * @param conv  A {@link VNodePathConverter} instance.
         * @param arg   A path argument which contains the virtual interface
         *              name.
         */
        @Override
        public void fetch(VNodePathConverter conv, PathArgument arg) {
            @SuppressWarnings("unchecked")
            IdentifiableItem<Vinterface, VinterfaceKey> item =
                (IdentifiableItem<Vinterface, VinterfaceKey>)arg;
            conv.interfaceName = item.getKey().getName();
        }

        /**
         * Create a new {@link VInterfaceIdentifier} instance.
         *
         * <p>
         *   This method is invoked if this instance parsed the last argument
         *   in the specified instance identifier.
         * </p>
         *
         * @param conv  A {@link VNodePathConverter} instance.
         * @return  A {@link VInterfaceIdentifier} instance.
         */
        @Override
        public VInterfaceIdentifier newIdentifier(VNodePathConverter conv) {
            VnodeName tname = conv.tenantName;
            VnodeName bname = conv.bridgeName;
            VnodeName iname = conv.interfaceName;
            return (bname == null)
                ? new VTerminalIfIdentifier(tname, conv.terminalName, iname)
                : new VBridgeIfIdentifier(tname, bname, iname);
        }
    }

    /**
     * Parser for the VLAN mapping ID.
     */
    private static final class VlanMapParser implements ArgumentParser {
        /**
         * Fetch the VLAN mapping ID from the given path argument,
         * and set it into {@code VNodePathConverter} instance.
         *
         * @param conv  A {@link VNodePathConverter} instance.
         * @param arg   A path argument which contains the VLAN mapping ID.
         */
        @Override
        public void fetch(VNodePathConverter conv, PathArgument arg) {
            @SuppressWarnings("unchecked")
            IdentifiableItem<VlanMap, VlanMapKey> item =
                (IdentifiableItem<VlanMap, VlanMapKey>)arg;
            conv.vlanMapId = item.getKey().getMapId();
        }

        /**
         * Create a new {@link VlanMapIdentifier} instance.
         *
         * <p>
         *   This method is invoked if this instance parsed the last argument
         *   in the specified instance identifier.
         * </p>
         *
         * @param conv  A {@link VNodePathConverter} instance.
         * @return  A {@link VlanMapIdentifier} instance.
         */
        @Override
        public VlanMapIdentifier newIdentifier(VNodePathConverter conv) {
            return new VlanMapIdentifier(conv.tenantName, conv.bridgeName,
                                         conv.vlanMapId);
        }
    }

    /**
     * Parser for the MAC mapping.
     */
    private static final class MacMapParser implements ArgumentParser {
        /**
         * This method does nothing.
         *
         * @param conv  A {@link VNodePathConverter} instance.
         * @param arg   A path argument which specifies the mac-map container.
         *              This method never uses this value.
         */
        @Override
        public void fetch(VNodePathConverter conv, PathArgument arg) {
        }

        /**
         * Create a new {@link MacMapIdentifier} instance.
         *
         * <p>
         *   This method is invoked if this instance parsed the last argument
         *   in the specified instance identifier.
         * </p>
         *
         * @param conv  A {@link VNodePathConverter} instance.
         * @return  A {@link MacMapIdentifier} instance.
         */
        @Override
        public MacMapIdentifier newIdentifier(VNodePathConverter conv) {
            return new MacMapIdentifier(conv.tenantName, conv.bridgeName);
        }
    }

    /**
     * Parser for the host mapped by the MAC mapping.
     */
    private static final class MacMapHostParser implements ArgumentParser {
        /**
         * Fetch the host information from the given path argument, and set it
         * into {@code VNodePathConverter} instance.
         *
         * @param conv  A {@link VNodePathConverter} instance.
         * @param arg   A path argument which contains the host mapped by the
         *              MAC mapping.
         * @throws RpcException
         *    An invalid host information is configured in {@code arg}.
         */
        @Override
        public void fetch(VNodePathConverter conv, PathArgument arg)
            throws RpcException {
            @SuppressWarnings("unchecked")
            IdentifiableItem<VlanHostDescList, VlanHostDescListKey> item =
                (IdentifiableItem<VlanHostDescList, VlanHostDescListKey>)arg;
            conv.macMappedHost = new MacVlan(item.getKey().getHost());
        }

        /**
         * Create a new {@link MacMapHostIdentifier} instance.
         *
         * <p>
         *   This method is invoked if this instance parsed the last argument
         *   in the specified instance identifier.
         * </p>
         *
         * @param conv  A {@link VNodePathConverter} instance.
         * @return  A {@link MacMapHostIdentifier} instance.
         */
        @Override
        public MacMapHostIdentifier newIdentifier(VNodePathConverter conv) {
            return new MacMapHostIdentifier(conv.tenantName, conv.bridgeName,
                                            conv.macMappedHost);
        }
    }

    /**
     * Construct a new instance.
     *
     * @param path  An {@link InstanceIdentifier} which specifies the data
     *              in the VTN tree.
     * @throws IllegalStateException
     *    The given path does not specify the data in the VTN tree.
     */
    public VNodePathConverter(InstanceIdentifier<?> path) {
        try {
            for (PathArgument arg: path.getPathArguments()) {
                Class<?> type = arg.getType();

                // Terminate conversion if denied-hosts container is detected.
                if (DeniedHosts.class.equals(type)) {
                    break;
                }

                ArgumentParser parser = PARSERS.get(type);
                if (parser != null) {
                    parser.fetch(this, arg);
                    lastParser = parser;
                }
            }
        } catch (RpcException | RuntimeException e) {
            throw new IllegalArgumentException(
                "Invalid instance identifier: " + path, e);
        }

        if (lastParser == null) {
            throw new IllegalArgumentException(
                "Unsupported instance identifier: " + path);
        }

        identifier = lastParser.newIdentifier(this);
    }

    /**
     * Return a {@link VNodeIdentifier} instance converted from the specified
     * instance identifier.
     *
     * @return  A {@link VNodeIdentifier} instance.
     */
    public VNodeIdentifier<?> getIdentifier() {
        return identifier;
    }

    /**
     * Return a {@link VNodeIdentifier} instance converted from the specified
     * instance identifier with type checking.
     *
     * @param type  A class which specifies the expected type of
     *              {@link VNodeIdentifier}.
     * @param <T>   The type of {@link VNodeIdentifier}.
     * @return  A {@link VNodeIdentifier} instance.
     * @throws IllegalStateException
     *    The type of the VNode identifier in this instance does not match
     *    the specified type.
     */
    public <T extends VNodeIdentifier<?>> T getIdentifier(Class<T> type) {
        if (type.isInstance(identifier)) {
            return type.cast(identifier);
        }

        throw new IllegalStateException(
            "Unexpected VNode identifier type: " + identifier);
    }
}
