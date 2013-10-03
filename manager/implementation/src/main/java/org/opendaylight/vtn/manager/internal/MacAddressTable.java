/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.Iterator;
import java.util.List;
import java.util.ArrayList;
import java.util.Map;
import java.util.Set;
import java.util.HashSet;
import java.util.TreeMap;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.TimeUnit;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.MacAddressEntry;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.internal.cluster.MacTableEntry;
import org.opendaylight.vtn.manager.internal.cluster.MacTableEntryId;

import org.opendaylight.controller.hosttracker.hostAware.HostNodeConnector;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.packet.address.DataLinkAddress;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.controller.sal.utils.HexEncode;
import org.opendaylight.controller.sal.utils.NetUtils;
import org.opendaylight.controller.sal.utils.Status;

/**
 * {@code MacAddressTable} class represents a MAC address table in a virtual
 * L2 bridge.
 */
public class MacAddressTable {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(MacAddressTable.class);

    /**
     * Threshold of aging interval to determine whether a canceled task should
     * be purged from the timer task queue or not.
     *
     * <p>
     *   If the value of {@link #ageInterval} is greater than this value,
     *   an aging task will be purged when it is canceled.
     * </p>
     */
    private static final int  AGING_PURGE_THRESHOLD = 60;

    /**
     * Path to the virtual L2 bridge.
     */
    private final VBridgePath  bridgePath;

    /**
     * VTN Manager service.
     */
    private final VTNManagerImpl vtnManager;

    /**
     * Interval in seconds between aging task.
     */
    private int  ageInterval;

    /**
     * MAC address table.
     */
    private Map<Long, MacTableEntry>  macAddressTable =
        new TreeMap<Long, MacTableEntry>();

    /**
     * Current MAC address table aging task.
     */
    private transient MacTableAgingTask  agingTask;

    /**
     * Periodic timer task used for MAC address table aging.
     */
    private final class MacTableAgingTask extends TimerTask {
        /**
         * Scan MAC address table for aging.
         */
        @Override
        public void run() {
            if (LOG.isTraceEnabled()) {
                LOG.trace("{}: Aging task called", getTableName());
            }
            age();
        }
    }

    /**
     * Remove MAC address table entries that match the specified condition.
     */
    protected class EntryRemover {
        /**
         * Remove MAC address table entries.
         *
         * @param table  A map which keeps MAC address table entries.
         */
        private void remove(Map<Long, MacTableEntry> table) {
            Set<MacTableEntryId> removed = new HashSet<MacTableEntryId>();
            for (Iterator<MacTableEntry> it = table.values().iterator();
                 it.hasNext();) {
                MacTableEntry tent = it.next();
                if (match(tent)) {
                    if (LOG.isTraceEnabled()) {
                        logRemoved(tent);
                    }
                    removed.add(tent.getEntryId());
                    it.remove();
                }
            }

            if (!removed.isEmpty()) {
                removeFromCache(removed);
            }
        }

        /**
         * Remove the given MAC address table entries from the cluster cache.
         *
         * @param idSet  A set of identifiers of MAC address table entries.
         */
        protected void removeFromCache(Set<MacTableEntryId> idSet) {
            vtnManager.removeMacTableEntries(idSet);
        }

        /**
         * Record a trace log that indicates a MAC address table entry
         * was removed.
         *
         * @param tent  A MAC address table entry.
         */
        protected void logRemoved(MacTableEntry tent) {
            LOG.trace("{}: MAC address removed: {}", getTableName(), tent);
        }

        /**
         * Determine whether the given entry should be removed or not.
         *
         * @param tent  A MAC address table entry.
         * @return  {@code true} is returned if the given entry should be
         *          removed. Otherwise {@code fales} is returned.
         */
        protected boolean match(MacTableEntry tent) {
            // Remove all entries.
            return true;
        }
    }

    /**
     * Remove MAC address table entries associated with the given node.
     */
    protected class NodeEntryRemover extends EntryRemover {
        /**
         * A node associated with SDN switch.
         */
        private final Node  node;

        /**
         * Construct a new entry remover.
         *
         * @param node  A node.
         */
        protected NodeEntryRemover(Node node) {
            this.node = node;
        }

        /**
         * Return the target node.
         *
         * @return  The target node.
         */
        protected Node getNode() {
            return node;
        }

        /**
         * Determine whether the given entry should be removed or not.
         *
         * @param tent  A MAC address table entry.
         * @return  {@code true} is returned if the given entry should be
         *          removed. Otherwise {@code fales} is returned.
         */
        @Override
        protected boolean match(MacTableEntry tent) {
            Node tnode = tent.getPort().getNode();
            return tnode.equals(node);
        }
    }

    /**
     * Remove MAC address table entries associated with the given node and
     * the VLAN ID.
     */
    private final class NodeVlanEntryRemover extends NodeEntryRemover {
        /**
         * The target VLAN ID.
         */
        private final short  vlan;

        /**
         * Construct a new entry remover.
         *
         * @param node  A node. Specifying {@code null} means wildcard.
         * @param vlan  VLAN ID.
         */
        private NodeVlanEntryRemover(Node node, short vlan) {
            super(node);
            this.vlan = vlan;
        }

        /**
         * Determine whether the given entry should be removed or not.
         *
         * @param tent  A MAC address table entry.
         * @return  {@code true} is returned if the given entry should be
         *          removed. Otherwise {@code fales} is returned.
         */
        @Override
        protected boolean match(MacTableEntry tent) {
            Node node = getNode();
            return (tent.getVlan() == vlan &&
                    (node == null || node.equals(tent.getPort().getNode())));
        }
    }

    /**
     * Remove MAC address table entries associated with the given node
     * connector.
     */
    protected class PortEntryRemover extends EntryRemover {
        /**
         * A node connector associated with SDN switch port.
         */
        private final NodeConnector  nodeConnector;

        /**
         * Construct a new entry remover.
         *
         * @param nc  A node connector.
         */
        protected PortEntryRemover(NodeConnector nc) {
            nodeConnector = nc;
        }

        /**
         * Determine whether the given entry should be removed or not.
         *
         * @param tent  A MAC address table entry.
         * @return  {@code true} is returned if the given entry should be
         *          removed. Otherwise {@code fales} is returned.
         */
        @Override
        protected boolean match(MacTableEntry tent) {
            return tent.getPort().equals(nodeConnector);
        }
    }

    /**
     * Remove MAC address table entries associated with the given node
     * connector and the VLAN ID.
     */
    private final class PortVlanEntryRemover extends PortEntryRemover {
        /**
         * The target VLAN ID.
         */
        private final short  vlan;

        /**
         * Construct a new entry remover.
         *
         * @param nc    A node connector.
         * @param vlan  VLAN ID.
         */
        protected PortVlanEntryRemover(NodeConnector nc, short vlan) {
            super(nc);
            this.vlan = vlan;
        }

        /**
         * Determine whether the given entry should be removed or not.
         *
         * @param tent  A MAC address table entry.
         * @return  {@code true} is returned if the given entry should be
         *          removed. Otherwise {@code fales} is returned.
         */
        @Override
        protected boolean match(MacTableEntry tent) {
            return (tent.getVlan() == vlan && super.match(tent));
        }
    }

    /**
     * Age MAC address table entries, and remove unused entries.
     */
    private final class AgedEntryRemover extends EntryRemover {
        /**
         * Record a debugging log that indicates a MAC address table entry
         * was removed.
         *
         * @param tent  A MAC address table entry.
         */
        @Override
        protected void logRemoved(MacTableEntry tent) {
            LOG.trace("{}: MAC address aged out: {}", getTableName(), tent);
        }

        /**
         * Determine whether the given entry should be removed or not.
         *
         * @param tent  A MAC address table entry.
         * @return  {@code true} is returned if the given entry should be
         *          removed. Otherwise {@code fales} is returned.
         */
        @Override
        protected boolean match(MacTableEntry tent) {
            // Do nothing if the specified entry is not created by this
            // controller.
            MacTableEntryId id = tent.getEntryId();
            return (id.isLocal() && !tent.clearUsed());
        }
    }

    /**
     * Remove MAC address table entries created by the given controller.
     */
    private final class ControllerEntryRemover extends EntryRemover {
        /**
         * A set of IP addresses of target controllers.
         */
        private final Set<InetAddress>  controllerAddresses;

        /**
         * Construct a new entry remover.
         *
         * @param addrs  A list of IP addresses of controllers.
         */
        private ControllerEntryRemover(Set<InetAddress> addrs) {
            controllerAddresses = addrs;
        }

        /**
         * Remove the given MAC address table entries from the cluster cache.
         *
         * <p>
         *   This method removes MAC address table entries on the calling
         *   thread.
         * </p>
         *
         * @param idSet  A set of identifiers of MAC address table entries.
         */
        @Override
        protected void removeFromCache(Set<MacTableEntryId> idSet) {
            vtnManager.removeMacTableEntriesSync(idSet);
        }

        /**
         * Determine whether the given entry should be removed or not.
         *
         * @param tent  A MAC address table entry.
         * @return  {@code true} is returned if the given entry should be
         *          removed. Otherwise {@code fales} is returned.
         */
        @Override
        protected boolean match(MacTableEntry tent) {
            MacTableEntryId id = tent.getEntryId();
            InetAddress addr = id.getControllerAddress();
            return controllerAddresses.contains(addr);
        }
    }

    /**
     * Convert a MAC address into a key of the MAC address table.
     *
     * @param addr A byte array which represents a MAC address.
     * @return  A {@code Long} object which represents the given MAC address.
     */
    public static Long getTableKey(byte[] addr) {
        long mac = NetUtils.byteArray6ToLong(addr);
        return Long.valueOf(mac);
    }

    /**
     * Construct a new MAC address table.
     *
     * @param mgr   VTN Manager service.
     * @param path  Path to the virtual L2 bridge.
     * @param age   Interval in seconds between aging task.
     */
    public MacAddressTable(VTNManagerImpl mgr, VBridgePath path, int age) {
        vtnManager = mgr;
        bridgePath = path;

        // Register an aging task to the global timer.
        IVTNResourceManager resMgr = mgr.getResourceManager();
        Timer timer = resMgr.getTimer();
        installAging(timer, age);
    }

    /**
     * Set interval between MAC address table aging task.
     *
     * @param age  Interval in seconds between aging task.
     */
    public void setAgeInterval(int age) {
        if (age != ageInterval) {
            IVTNResourceManager resMgr = vtnManager.getResourceManager();
            Timer timer = resMgr.getTimer();

            synchronized (this) {
                if (LOG.isDebugEnabled()) {
                    LOG.debug("{}: Aging interval changed: {} -> {}",
                              getTableName(), ageInterval, age);
                }

                // Cancel current aging task.
                agingTask.cancel();
                if (ageInterval > AGING_PURGE_THRESHOLD) {
                    timer.purge();
                }

                // Install a new aging task.
                installAging(timer, age);
            }
        }
    }

    /**
     * Add a MAC address table entry if needed.
     *
     * @param pctx  The context of the received packet.
     */
    public void add(PacketContext pctx) {
        byte[] src = pctx.getSourceAddress();
        if (!NetUtils.isUnicastMACAddr(src)) {
            return;
        }

        // Determine attributes of a new MAC address table entry.
        Long key = getTableKey(src);
        NodeConnector port = pctx.getIncomingNodeConnector();
        short vlan = pctx.getVlan();
        InetAddress ipaddr = getSourceInetAddress(pctx);

        // Add a MAC address table entry.
        MacTableEntry tent = addEntry(pctx, port, vlan, key, ipaddr);
        if (tent == null) {
            return;
        }

        // Notify the host tracker of new host entry.
        boolean needProbe = tent.clearProbeNeeded();
        if (ipaddr != null) {
            try {
                HostNodeConnector host =
                    new HostNodeConnector(src, ipaddr, port, vlan);
                if (LOG.isDebugEnabled()) {
                    LOG.debug("{}: Notify new host: ipaddr={}, host={}",
                              getTableName(), ipaddr.getHostAddress(), host);
                }
                vtnManager.notifyHost(host);
            } catch (Exception e) {
                if (LOG.isErrorEnabled()) {
                    StringBuilder builder =
                        new StringBuilder(getTableName());
                    builder.append(": Unable to create host: src=").
                        append(HexEncode.bytesToHexStringFormat(src)).
                        append(", ipaddr=").append(ipaddr).
                        append(", port=").append(port.toString()).
                        append(", vlan=").append((int)vlan);
                    LOG.error(builder.toString(), e);
                }
            }
        } else if (needProbe) {
            // Try to detect IP address of the host.
            pctx.probeInetAddress(vtnManager);
        }
    }

    /**
     * Add the given MAC address table entry to this table.
     *
     * <p>
     *   Note that this method never puts the given entry to the cluster cache.
     * </p>
     *
     * @param tent  A MAC address table entry.
     */
    synchronized void add(MacTableEntry tent) {
        Map<Long, MacTableEntry> table = macAddressTable;
        if (table != null) {
            Long key = Long.valueOf(tent.getMacAddress());
            table.put(key, tent);
        }
    }

    /**
     * Return the MAC address table entry associated with the destination
     * address of the received packet.
     *
     * @param pctx  The context of the received packet.
     * @return  A MAC address table entry if found. {@code null} if not fonud.
     */
    public MacTableEntry get(PacketContext pctx) {
        byte[] dst = pctx.getDestinationAddress();
        Long key = getTableKey(dst);

        synchronized (this) {
            Map<Long, MacTableEntry> table = macAddressTable;
            if (table == null) {
                return null;
            }

            MacTableEntry tent = table.get(key);
            if (tent != null) {
                // Turn the used flag on.
                tent.setUsed();
            }

            return tent;
        }
    }

    /**
     * Remove the MAC address table entry associated with the destination
     * address of the received packet.
     *
     * @param pctx  The context of the received packet.
     */
    public void remove(PacketContext pctx) {
        byte[] dst = pctx.getDestinationAddress();
        Long key = getTableKey(dst);

        synchronized (this) {
            Map<Long, MacTableEntry> table = macAddressTable;
            if (table != null) {
                MacTableEntry tent = table.remove(key);
                if (tent != null) {
                    vtnManager.removeMacTableEntry(tent.getEntryId());
                }
            }
        }
    }

    /**
     * Return a list of MAC address entries in this table.
     *
     * @return  A list of MAC address entries.
     * @throws VTNException  An error occurred.
     */
    public synchronized List<MacAddressEntry> getEntries() throws VTNException {
        Map<Long, MacTableEntry> table = macAddressTable;
        if (table == null) {
            return new ArrayList<MacAddressEntry>();
        }

        ArrayList<MacAddressEntry> list =
            new ArrayList<MacAddressEntry>(table.size());
        for (Iterator<Map.Entry<Long, MacTableEntry>> it =
                 table.entrySet().iterator(); it.hasNext();) {
            Map.Entry<Long, MacTableEntry> entry = it.next();
            Long mac = entry.getKey();
            MacTableEntry tent = entry.getValue();
            list.add(tent.getEntry());
        }
        list.trimToSize();

        return list;
    }

    /**
     * Return a MAC address entry associated with the specified MAC address.
     *
     * <p>
     *   This method never affects the used flag of the MAC address table
     *   entry.
     * </p>
     *
     * @param dladdr  A data link address.
     * @return  A MAC address entry if found. {@code null} if not fonud.
     * @throws VTNException  An error occurred.
     */
    public MacAddressEntry getEntry(DataLinkAddress dladdr)
        throws VTNException {
        Long key = getTableKey(dladdr);
        if (key == null) {
            return null;
        }

        synchronized (this) {
            Map<Long, MacTableEntry> table = macAddressTable;
            if (table == null) {
                return null;
            }

            MacTableEntry tent = table.get(key);
            if (tent == null) {
                return null;
            }
            return tent.getEntry();
        }
    }

    /**
     * Remove a MAC address entry from the MAC address table.
     *
     * @param dladdr  A data link address.
     * @return  A removed MAC address entry. {@code null} if not fonud.
     * @throws VTNException  An error occurred.
     */
    public MacAddressEntry removeEntry(DataLinkAddress dladdr)
        throws VTNException {
        Long key = getTableKey(dladdr);
        if (key == null) {
            return null;
        }

        MacTableEntry tent;
        synchronized (this) {
            Map<Long, MacTableEntry> table = macAddressTable;
            if (table == null) {
                return null;
            }

            tent = table.remove(key);
            if (tent == null) {
                return null;
            }
            vtnManager.removeMacTableEntry(tent.getEntryId());
        }

        if (LOG.isTraceEnabled()) {
            LOG.trace("{}: removeEntry: MAC address removed: {}",
                      getTableName(), tent);
        }
        return tent.getEntry();
    }

    /**
     * Flush all MAC address table entries.
     */
    public synchronized void flush() {
        Map<Long, MacTableEntry> table = macAddressTable;
        if (table != null) {
            if (LOG.isTraceEnabled()) {
                LOG.trace("{}: Flush MAC address table", getTableName());
            }
            EntryRemover remover = new EntryRemover();
            remover.remove(table);
        }
    }

    /**
     * Flush all MAC address table entries associated with the given node.
     *
     * @param node  A node.
     */
    public synchronized void flush(Node node) {
        Map<Long, MacTableEntry> table = macAddressTable;
        if (table != null) {
            EntryRemover remover = new NodeEntryRemover(node);
            remover.remove(table);
        }
    }

    /**
     * Flush all MAC address table entries associated with the give node and
     * the VLAN ID.
     *
     * @param node  A node. Specifying {@code null} means wildcard.
     * @param vlan  VLAN ID.
     */
    public synchronized void flush(Node node, short vlan) {
        Map<Long, MacTableEntry> table = macAddressTable;
        if (table != null) {
            EntryRemover remover = new NodeVlanEntryRemover(node, vlan);
            remover.remove(table);
        }
    }

    /**
     * Flush all MAC address table entries associated with the given node
     * connector.
     *
     * @param nc  A node connector.
     */
    public synchronized void flush(NodeConnector nc) {
        Map<Long, MacTableEntry> table = macAddressTable;
        if (table != null) {
            EntryRemover remover = new PortEntryRemover(nc);
            remover.remove(table);
        }
    }

    /**
     * Flush all MAC address table entries associated with the given node
     * connector and the VLAN ID.
     *
     * @param nc    A node connector.
     * @param vlan  VLAN ID.
     */
    public synchronized void flush(NodeConnector nc, short vlan) {
        Map<Long, MacTableEntry> table = macAddressTable;
        if (table != null) {
            EntryRemover remover = new PortVlanEntryRemover(nc, vlan);
            remover.remove(table);
        }
    }

    /**
     * Flush all MAC address table entries created by the specified
     * controllers.
     *
     * <p>
     *   Note that this method removes MAC address table entries from the
     *   cluster cache on the calling thread.
     * </p>
     *
     * @param addrs  A set of IP addresses of controllers.
     */
    public synchronized void flush(Set<InetAddress> addrs) {
        Map<Long, MacTableEntry> table = macAddressTable;
        if (table != null) {
            EntryRemover remover = new ControllerEntryRemover(addrs);
            remover.remove(table);
        }
    }

    /**
     * Destroy the MAC address table.
     *
     * @param purge  If {@code true} is passed, purge all canceled aging
     *               tasks in the global timer task queue.
     */
    public synchronized void destroy(boolean purge) {
        // Invalidate MAC address table.
        Map<Long, MacTableEntry> table = macAddressTable;
        if (table == null) {
            return;
        }
        macAddressTable = null;

        // Remove all MAC address table entries from the cluster cache.
        EntryRemover remover = new EntryRemover();
        remover.remove(table);

        // Cancel the aging task.
        agingTask.cancel();
        if (purge) {
            vtnManager.getResourceManager().getTimer().purge();
        }
    }

    /**
     * Invoked when a MAC address table entry is updated by remote cluster
     * node.
     *
     * @param tent  A MAC address table entry.
     */
    synchronized void entryUpdated(MacTableEntry tent) {
        Map<Long, MacTableEntry> table = macAddressTable;
        if (table == null) {
            return;
        }

        Long key = Long.valueOf(tent.getMacAddress());
        MacTableEntry old = table.put(key, tent);
        if (LOG.isTraceEnabled()) {
            String tname = getTableName();
            if (old != null) {
                LOG.trace("{}: MAC address was changed by another controller" +
                          ": {} -> {}", tname, old, tent);
            } else {
                LOG.trace("{}: MAC address was added by another controller" +
                          ": {}", tname, tent);
            }
        }
    }

    /**
     * Invoked when a MAC address table entry is deleted by remote cluster
     * node.
     *
     * @param id  An identifier of a MAC address table entry.
     */
    synchronized void entryDeleted(MacTableEntryId id) {
        Map<Long, MacTableEntry> table = macAddressTable;
        if (table == null) {
            return;
        }

        Long key = Long.valueOf(id.getMacAddress());
        MacTableEntry tent = table.remove(key);
        if (tent == null) {
            return;
        }

        if (!tent.getEntryId().equals(id)) {
            // Another entry is mapped to this MAC address.
            table.put(key, tent);
        } else if (LOG.isTraceEnabled()) {
            LOG.trace("{}: MAC address was removed by another controller: {}",
                      getTableName(), tent);
        }
    }

    /**
     * Install a new MAC address table aging task.
     *
     * @param timer  The global timer thread.
     * @param age    Interval in seconds between aging task.
     */
    private synchronized void installAging(Timer timer, int age) {
        ageInterval = age;
        agingTask = new MacTableAgingTask();
        long milli = TimeUnit.SECONDS.toMillis((long)age);
        timer.schedule(agingTask, milli, milli);
    }

    /**
     * Scan MAC address table entries, and eliminate unused entries.
     */
    private synchronized void age() {
        Map<Long, MacTableEntry> table = macAddressTable;
        if (table != null) {
            EntryRemover remover = new AgedEntryRemover();
            remover.remove(table);
        }
    }

    /**
     * Convert a data link address into a key of the MAC address table.
     *
     * @param dladdr  A data link address.
     * @return  A {@code Long} object which represents the given MAC address.
     *          {@code null} is returned if {@code dladdr} is not an
     *          ethernet address.
     * @throws VTNException  An error occurred.
     */
    private Long getTableKey(DataLinkAddress dladdr) throws VTNException {
        if (!(dladdr instanceof EthernetAddress)) {
            if (dladdr == null) {
                Status status = VTNManagerImpl.argumentIsNull("MAC address");
                throw new VTNException(status);
            }
            return null;
        }

        EthernetAddress ethAddr = (EthernetAddress)dladdr;
        return getTableKey(ethAddr.getValue());
    }

    /**
     * Add a MAC address table entry for a received packet.
     *
     * @param pctx    The context of the received packet.
     * @param port    A node connector associated with incoming switch port.
     * @param vlan    VLAN ID.
     * @param key     A long value which represents a MAC address.
     * @param ipaddr  IP address assigned to the given entry.
     * @return  A MAC address table entry associated with the received packet
     *          is returned.
     *          {@code null} is returned if this table is no longer available.
     */
    private synchronized MacTableEntry addEntry(PacketContext pctx,
                                                NodeConnector port, short vlan,
                                                Long key, InetAddress ipaddr) {
        Map<Long, MacTableEntry> table = macAddressTable;
        if (table == null) {
            return null;
        }

        // Search for a table entry mapped to the given MAC address.
        MacTableEntry tent = table.get(key);
        if (tent == null) {
            // Register a new table entry.
            tent = new MacTableEntry(bridgePath, key, port, vlan, ipaddr);
            if (LOG.isTraceEnabled()) {
                LOG.trace("{}: New MAC address table entry: {}",
                          getTableName(), tent);
            }
            table.put(key, tent);
            vtnManager.putMacTableEntry(tent);
            return tent;
        }

        // Check to see if the entry needs to be changed.
        NodeConnector curport = tent.getPort();
        short curvlan = tent.getVlan();
        boolean changed = false;
        MacTableEntry newEnt = tent;
        if (vlan != curvlan || !port.equals(curport)) {
            // The host was moved to other network.
            pctx.addObsoleteEntry(key, tent);
            vtnManager.removeMacTableEntry(tent.getEntryId());

            // Replace the table entry.
            newEnt = new MacTableEntry(bridgePath, key, port, vlan, ipaddr);
            table.put(key, newEnt);
            vtnManager.putMacTableEntry(newEnt);
            changed = true;
        } else {
            if (ipaddr != null) {
                // Append IP address to this entry.
                changed = tent.addInetAddress(ipaddr);
                if (changed) {
                    vtnManager.updateMacTableEntry(tent);
                }
            }

            // Turn the used flag on.
            tent.setUsed();
        }

        if (changed && LOG.isTraceEnabled()) {
            LOG.trace("{}: MAC address table entry changed: {}",
                      getTableName(), tent);
        }

        return newEnt;
    }

    /**
     * Return the source IP address of the received packet.
     *
     * @param pctx  The context of the received packet.
     * @return  The source IP address.
     *          {@code null} is returned if no IP header was found.
     */
    private InetAddress getSourceInetAddress(PacketContext pctx) {
        byte[] sip = pctx.getSourceIpAddress();
        if (sip != null) {
            try {
                return InetAddress.getByAddress(sip);
            } catch (UnknownHostException e) {
                // This should never happen.
                LOG.error("{}: Invalid IP address: {}, ipaddr={}",
                          getTableName(),
                          pctx.getDescription(pctx.getIncomingNodeConnector()),
                          HexEncode.bytesToHexStringFormat(sip));
            }
        }

        return null;
    }

    /**
     * Return the name of this table.
     *
     * @return  The name of this table.
     */
    private String getTableName() {
        StringBuilder builder =
            new StringBuilder(vtnManager.getContainerName());
        builder.append(':').append(bridgePath.toString());
        return builder.toString();
    }
}
