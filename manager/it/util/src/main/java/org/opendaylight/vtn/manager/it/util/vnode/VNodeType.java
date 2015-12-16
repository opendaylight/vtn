/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.vnode;

import static org.opendaylight.vtn.manager.it.util.vnode.VNodeIdentifier.TYPEBIT_VTN;
import static org.opendaylight.vtn.manager.it.util.vnode.VNodeIdentifier.TYPEBIT_VBRIDGE;
import static org.opendaylight.vtn.manager.it.util.vnode.VNodeIdentifier.TYPEBIT_VTERMINAL;
import static org.opendaylight.vtn.manager.it.util.vnode.VNodeIdentifier.TYPEBIT_VINTERFACE;
import static org.opendaylight.vtn.manager.it.util.vnode.VNodeIdentifier.TYPEBIT_VLANMAP;
import static org.opendaylight.vtn.manager.it.util.vnode.VNodeIdentifier.TYPEBIT_MACMAP;
import static org.opendaylight.vtn.manager.it.util.vnode.VNodeIdentifier.TYPEBIT_MACMAP_HOST;

import java.util.Map;

import com.google.common.collect.ImmutableMap;

/**
 * {@code VNodeType} Describes a type of virtual node specified by
 * {@code VNodeIdentifier}.
 */
public enum VNodeType implements VNodeIdentifierFactory {
    /**
     * Indicates a VTN or a virtual node inside VTN.
     */
    VTN("VTN", 1, TYPEBIT_VTN) {
        @Override
        public VTenantIdentifier newIdentifier(String[] comps) {
            return new VTenantIdentifier(comps);
        }
    },

    /**
     * Indicates a vBridge or a virtual node inside vBridge.
     */
    VBRIDGE("vBridge", 2, TYPEBIT_VTN | TYPEBIT_VBRIDGE) {
        @Override
        public VBridgeIdentifier newIdentifier(String[] comps) {
            return new VBridgeIdentifier(comps);
        }
    },

    /**
     * Indicates a vTerminal or a virtual node inside vTerminal.
     */
    VTERMINAL("vTerminal", 2, TYPEBIT_VTN | TYPEBIT_VTERMINAL) {
        @Override
        public VTerminalIdentifier newIdentifier(String[] comps) {
            return new VTerminalIdentifier(comps);
        }
    },

    /**
     * Indicates a virtual interface attached to a vBridge.
     */
    VBRIDGE_IF("vBridge-IF", VInterfaceIdentifier.DESCRIPTION, 3,
               TYPEBIT_VTN | TYPEBIT_VBRIDGE | TYPEBIT_VINTERFACE) {
        @Override
        public VBridgeIfIdentifier newIdentifier(String[] comps) {
            return new VBridgeIfIdentifier(comps);
        }
    },

    /**
     * Indicates a virtual interface attached to a vTerminal.
     */
    VTERMINAL_IF("vTerminal-IF", VInterfaceIdentifier.DESCRIPTION, 3,
                 TYPEBIT_VTN | TYPEBIT_VTERMINAL | TYPEBIT_VINTERFACE) {
        @Override
        public VTerminalIfIdentifier newIdentifier(String[] comps) {
            return new VTerminalIfIdentifier(comps);
        }
    },

    /**
     * Indicates a VLAN mapping configured in a vBridge.
     */
    VLANMAP("VLAN-map", "VLAN mapping", 3,
            TYPEBIT_VTN | TYPEBIT_VBRIDGE | TYPEBIT_VLANMAP) {
        @Override
        public VlanMapIdentifier newIdentifier(String[] comps) {
            return new VlanMapIdentifier(comps);
        }
    },

    /**
     * Indicates a MAC mapping configured in a vBridge.
     */
    MACMAP("MAC-map", "MAC mapping", 2,
           TYPEBIT_VTN | TYPEBIT_VBRIDGE | TYPEBIT_MACMAP) {
        @Override
        public MacMapIdentifier newIdentifier(String[] comps) {
            return new MacMapIdentifier(comps);
        }
    },

    /**
     * Indicates a host mapped by MAC mapping configured in a vBridge.
     */
    MACMAP_HOST("MAC-map-host", "MAC mapped host", 3,
                TYPEBIT_VTN | TYPEBIT_VBRIDGE | TYPEBIT_MACMAP |
                TYPEBIT_MACMAP_HOST) {
        @Override
        public MacMapHostIdentifier newIdentifier(String[] comps) {
            return new MacMapHostIdentifier(comps);
        }
    };

    /**
     * A map which keeps pairs of virtual node type names and {@link VNodeType}
     * instances.
     */
    private static final Map<String, VNodeType>  TYPE_MAP;

    /**
     * A human-readable string which indicates the type of the virtual node.
     */
    private final String  typeName;

    /**
     * A brief description about the virtual node.
     */
    private final String  description;

    /**
     * The number of path components.
     */
    private final int  componentSize;

    /**
     * A bit mask which indicates the type of the virtual node.
     */
    private final int  typeBits;

    /**
     * Initialize static fields.
     */
    static {
        ImmutableMap.Builder<String, VNodeType> builder =
            ImmutableMap.<String, VNodeType>builder();
        for (VNodeType type: VNodeType.values()) {
            builder.put(type.typeName, type);
        }

        TYPE_MAP = builder.build();
    }

    /**
     * Convert the given string into a {@link VNodeType} instance.
     *
     * @param str  A string representation of {@link VNodeType} instance.
     * @return  A {@link VNodeType} instance or a {@code null}.
     */
    public static VNodeType forName(String str) {
        return TYPE_MAP.get(str);
    }

    /**
     * Convert the given string into a {@link VNodeType} instance.
     *
     * @param str  A string representation of {@link VNodeType} instance.
     * @return  A {@link VNodeType} instance.
     * @throws IllegalArgumentException
     *    {@code str} could not be converted into {@link VNodeType}.
     */
    public static VNodeType checkedForName(String str) {
        VNodeType type = TYPE_MAP.get(str);
        if (type == null) {
            throw new IllegalArgumentException("Unknown VNode type: " + str);
        }

        return type;
    }

    /**
     * Construct a new instance.
     *
     * @param name  A human-readable string which indicates the type of the
     *              virtual node.
     * @param size  The number of path components.
     * @param bits  A bit mask which indicates the type of the virtual node.
     */
    private VNodeType(String name, int size, int bits) {
        this(name, name, size, bits);
    }

    /**
     * Construct a new instance.
     *
     * @param name  A human-readable string which indicates the type of the
     *              virtual node.
     * @param desc  A brief description about the virtual node.
     * @param size  The number of path components.
     * @param bits  A bit mask which indicates the type of the virtual node.
     */
    private VNodeType(String name, String desc, int size, int bits) {
        typeName = name;
        description = desc;
        componentSize = size;
        typeBits = bits;
    }

    /**
     * Return the description about the virtual node.
     *
     * @return  The description about the virtual node.
     */
    public String getDescription() {
        return description;
    }

    /**
     * Return the number of path components.
     *
     * @return  The number of path components.
     */
    public int getComponentSize() {
        return componentSize;
    }

    /**
     * Determine whether the virtual node specified by this instance contains
     * the virtual node specified by the given {@link VNodeType} instance.
     *
     * @param type  A {@link VNodeType} instance.
     * @return  {@code true} only if the virtual node specified by this
     *          instance contains the virtual node specified by {@code type}.
     */
    public boolean contains(VNodeType type) {
        boolean ret = (type != null);
        if (ret) {
            ret = ((typeBits & type.typeBits) == typeBits);
        }

        return ret;
    }

    /**
     * Return a {@link VNodeType} which indicates the type of virtual bridge.
     *
     * @return  {@link VNodeType#VBRIDGE} if this instance specifies the
     *          vBridge or virtual node inside vBridge.
     *          {@link VNodeType#VTERMINAL} if this instance specifies the
     *          vTerminal or virtual node inside vTerminal.
     *          {@code null} otherwise.
     */
    public VNodeType getBridgeType() {
        VNodeType type;
        if ((typeBits & TYPEBIT_VBRIDGE) != 0) {
            type = VNodeType.VBRIDGE;
        } else if ((typeBits & TYPEBIT_VTERMINAL) != 0) {
            type = VNodeType.VTERMINAL;
        } else {
            type = null;
        }

        return type;
    }

    /**
     * Determine whether this instance indicates the virtual interface
     * or not.
     *
     * @return  {@code true} if this instance indicates the virtual interface.
     *          {@code false} otherwise.
     */
    public boolean isInterface() {
        return ((typeBits & TYPEBIT_VINTERFACE) != 0);
    }

    /**
     * Determine whether this instance indicates the MAC mapping or a host
     * mapped by the MAC mapping.
     *
     * @return  {@code true} if this instance indicates the MAC mapping.
     *          {@code false} otherwise.
     */
    public boolean isMacMap() {
        return ((typeBits & TYPEBIT_MACMAP) != 0);
    }

    // Object

    /**
     * Return a string representation of this instance.
     *
     * @return  A string representation of this instance.
     */
    @Override
    public String toString() {
        return typeName;
    }
}

