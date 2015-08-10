/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.ArrayList;
import java.util.Deque;
import java.util.LinkedList;
import java.util.List;

import org.opendaylight.vtn.manager.IVTNManagerAware;
import org.opendaylight.vtn.manager.MacMapConfig;
import org.opendaylight.vtn.manager.PortMap;
import org.opendaylight.vtn.manager.VBridge;
import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VInterface;
import org.opendaylight.vtn.manager.VTenant;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.VTerminal;
import org.opendaylight.vtn.manager.VTerminalPath;
import org.opendaylight.vtn.manager.VTerminalIfPath;
import org.opendaylight.vtn.manager.VlanMap;

import org.opendaylight.controller.sal.core.UpdateType;

/**
 * An implementation of {@link IVTNManagerAware} which keeps received events
 * in order.
 */
public class OrderedVTNListener extends TestBase implements IVTNManagerAware {
    /**
     * Maximum number of milliseconds to wait for a event.
     */
    private static final long  EVENT_TIMEOUT = 5000L;

    /**
     * A list of arguments passed to listener methods.
     */
    private final Deque<List<Object>>  receivedEvents =
        new LinkedList<List<Object>>();

    /**
     * {@inheritDoc}
     */
    @Override
    public void vtnChanged(VTenantPath path, VTenant vtenant,
                           UpdateType type) {
        addEvent(VTNListenerType.VTN, path, vtenant, type);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void vBridgeChanged(VBridgePath path, VBridge vbridge,
                               UpdateType type) {
        addEvent(VTNListenerType.VBRIDGE, path, vbridge, type);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void vTerminalChanged(VTerminalPath path, VTerminal vterm,
                                 UpdateType type) {
        addEvent(VTNListenerType.VTERMINAL, path, vterm, type);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void vInterfaceChanged(VBridgeIfPath path, VInterface viface,
                                  UpdateType type) {
        addEvent(VTNListenerType.VBRIDGE_IF, path, viface, type);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void vInterfaceChanged(VTerminalIfPath path, VInterface viface,
                                  UpdateType type) {
        addEvent(VTNListenerType.VTERMINAL_IF, path, viface, type);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void vlanMapChanged(VBridgePath path, VlanMap vlmap,
                               UpdateType type) {
        addEvent(VTNListenerType.VLANMAP, path, vlmap, type);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void portMapChanged(VBridgeIfPath path, PortMap pmap,
                               UpdateType type) {
        addEvent(VTNListenerType.PORTMAP, path, pmap, type);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void portMapChanged(VTerminalIfPath path, PortMap pmap,
                               UpdateType type) {
        addEvent(VTNListenerType.PORTMAP_VTERM, path, pmap, type);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void macMapChanged(VBridgePath path, MacMapConfig mcconf,
                              UpdateType type) {
        addEvent(VTNListenerType.MACMAP, path, mcconf, type);
    }

    /**
     * Ensure that the specified listener method was called.
     *
     * @param type  An event type.
     * @param args  Arguments expected to be passed to listener method.
     */
    public void checkEvent(VTNListenerType type, Object... args) {
        List<Object> event = getEvent();
        assertEquals(args.length + 1, event.size());
        assertEquals(type, event.get(0));

        int idx = 1;
        for (Object arg: args) {
            assertEquals(arg, event.get(idx));
            idx++;
        }
    }

    /**
     * Ensure that no event is queued in this object.
     */
    public synchronized void checkEmtpy() {
        assertTrue(receivedEvents.isEmpty());
    }

    /**
     * Enqueue arguments passed to listener method.
     *
     * @param type  An event type.
     * @param args  A list of arguments passed to listener method.
     */
    private synchronized void addEvent(VTNListenerType type, Object... args) {
        ArrayList<Object> list = new ArrayList<Object>();
        list.add(type);
        for (Object arg: args) {
            list.add(arg);
        }
        list.trimToSize();
        receivedEvents.addLast(list);
        notify();
    }

    /**
     * Return the oldest event.
     *
     * @return  A list of arguments passed to listener method.
     */
    private synchronized List<Object> getEvent() {
        if (receivedEvents.isEmpty()) {
            long timeout = EVENT_TIMEOUT;
            long limit = System.currentTimeMillis() + timeout;
            while (true) {
                try {
                    wait(timeout);
                } catch (InterruptedException e) {
                    unexpected(e);
                }
                if (!receivedEvents.isEmpty()) {
                    break;
                }
                timeout = limit - System.currentTimeMillis();
                assertTrue(timeout > 0);
            }
        }

        return receivedEvents.removeFirst();
    }
}
