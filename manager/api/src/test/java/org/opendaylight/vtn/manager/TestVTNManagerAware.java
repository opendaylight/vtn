/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.util.ArrayList;
import java.util.EnumMap;
import java.util.List;
import java.util.Map;

import org.opendaylight.controller.sal.core.UpdateType;

/**
 * Test class to keep track the call of {@link IVTNManagerAware} methods.
 */
public final class TestVTNManagerAware implements IVTNManagerAware {
    /**
     * Counter to keep track of the method calls.
     */
    private final Map<VTNListenerType, List<List<Object>>>  callCounter =
        new EnumMap<VTNListenerType, List<List<Object>>>(VTNListenerType.class);

    /**
     * Invoked when the information related to VTN inside the container
     * is changed.
     *
     * @param path     A {@link VTenantPath} object that specifies the
     *                 position of the VTN.
     * @param vtenant  A {@link VTenant} object which represents the VTN
     *                 information.
     * @param type     An {@link UpdateType} object which indicates the type
     *                 of modification is specified.
     */
    @Override
    public void vtnChanged(VTenantPath path, VTenant vtenant,
                           UpdateType type) {
        record(VTNListenerType.VTN, path, vtenant, type);
    }

    /**
     * Invoked when the information related to vBridge inside the container
     * is changed.
     *
     * @param path     A {@link VBridgePath} object that specifies the
     *                 position of the vBridge.
     * @param vbridge  A {@link VBridge} object which represents the vBridge
     *                 information.
     * @param type     An {@link UpdateType} object which indicates the type
     *                 of modification is specified.
     */
    @Override
    public void vBridgeChanged(VBridgePath path, VBridge vbridge,
                               UpdateType type) {
        record(VTNListenerType.VBRIDGE, path, vbridge, type);
    }

    /**
     * Invoked when the information related to vTerminal inside the container
     * is changed.
     *
     * @param path   A {@link VTerminalPath} object that specifies the
     *               position of the vTerminal.
     * @param vterm  A {@link VTerminal} object which represents the vTerminal
     *               information.
     * @param type   An {@link UpdateType} object which indicates the type of
     *               modification is specified.
     */
    @Override
    public void vTerminalChanged(VTerminalPath path, VTerminal vterm,
                                 UpdateType type) {
        record(VTNListenerType.VTERMINAL, path, vterm, type);
    }

    /**
     * Invoked when the information related to the virtual interface
     * configured in vBridge inside the container is changed.
     *
     * @param path    A {@link VBridgeIfPath} object that specifies the
     *                position of the vBridge interface.
     * @param viface  A {@link VInterface} object which represents the vBridge
     *                interface information.
     * @param type    An {@link UpdateType} object which indicates the type of
     *                modification is specified.
     */
    @Override
    public void vInterfaceChanged(VBridgeIfPath path, VInterface viface,
                                  UpdateType type) {
        record(VTNListenerType.VBRIDGE_IF, path, viface, type);
    }

    /**
     * Invoked when the information related to the virtual interface
     * configured in vTerminal inside the container is changed.
     *
     * @param path    A {@link VTerminalIfPath} object that specifies the
     *                position of the vTerminal interface.
     * @param viface  A {@link VInterface} object which represents the
     *                vTerminal interface information.
     * @param type    An {@link UpdateType} object which indicates the type of
     *                modification is specified.
     */
    @Override
    public void vInterfaceChanged(VTerminalIfPath path, VInterface viface,
                                  UpdateType type) {
        record(VTNListenerType.VTERMINAL_IF, path, viface, type);
    }

    /**
     * Invoked when the information related to the VLAN mapping configured in
     * vBridge inside the container is changed.
     *
     * @param path   A {@link VBridgePath} object that specifies the position
     *               of the VBridge.
     * @param vlmap  A {@link VlanMap} object which represents the VLAN mapping
     *               information.
     * @param type   An {@link UpdateType} object which indicates the type of
     *               modification is specified.
     */
    @Override
    public void vlanMapChanged(VBridgePath path, VlanMap vlmap,
                               UpdateType type) {
        record(VTNListenerType.VLANMAP, path, vlmap, type);
    }

    /**
     * Invoked when the information related to the port mapping configured in
     * vBridge interface inside the container is changed.
     *
     * @param path  A {@link VBridgeIfPath} object that specifies the position
     *              of the vBridge interface.
     * @param pmap  A {@link PortMap} object which represents the port mapping
     *              information.
     * @param type  An {@link UpdateType} object which indicates the type of
     *              modification is specified.
     */
    @Override
    public void portMapChanged(VBridgeIfPath path, PortMap pmap,
                               UpdateType type) {
        record(VTNListenerType.PORTMAP, path, pmap, type);
    }

    /**
     * Invoked when the information related to the port mapping configured in
     * vTerminal interface inside the container is changed.
     *
     * @param path  A {@link VTerminalIfPath} object that specifies the
     *              position of the vTerminal interface.
     * @param pmap  A {@link PortMap} object which represents the port mapping
     *              information.
     * @param type  An {@link UpdateType} object which indicates the type of
     *              modification is specified.
     */
    @Override
    public void portMapChanged(VTerminalIfPath path, PortMap pmap,
                               UpdateType type) {
        record(VTNListenerType.PORTMAP_VTERM, path, pmap, type);
    }

    /**
     * Invoked when the information related to the MAC mappping configured in
     * vBridge inside the container is changed.
     *
     * @param path    A {@link VBridgePath} object that specifies the position
     *                of the VBridge.
     * @param mcconf  A {@link MacMapConfig} object which represents the MAC
     *                mapping configuration information.
     * @param type    An {@link UpdateType} object which indicates the type of
     *                modification is specified.
     */
    @Override
    public void macMapChanged(VBridgePath path, MacMapConfig mcconf,
                              UpdateType type) {
        record(VTNListenerType.MACMAP, path, mcconf, type);
    }

    /**
     * Clear the method call counter.
     */
    public void clear() {
        callCounter.clear();
    }

    /**
     * Return a list of arguments passed to the listener method specified by
     * the given type.
     *
     * @param type  The type of listener method.
     * @return  A list of arguments passed to the specified method.
     *          {@code null} is returned if the specified method is never
     *          called.
     */
    public List<List<Object>> getArguments(VTNListenerType type) {
        List<List<Object>> list = callCounter.get(type);
        if (list != null) {
            list = new ArrayList<List<Object>>(list);
        }

        return list;
    }

    /**
     * Record the call of method.
     *
     * @param type  The type of listener method.
     * @param args  Arguments passed to the method call.
     */
    private void record(VTNListenerType type, Object ... args) {
        List<Object> argList = new ArrayList<Object>();
        if (args != null) {
            for (Object o: args) {
                argList.add(o);
            }
        }

        List<List<Object>> list = callCounter.get(type);
        if (list == null) {
            list = new ArrayList<List<Object>>();
            callCounter.put(type, list);
        }

        list.add(argList);
    }
}
