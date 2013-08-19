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
import java.util.TreeMap;
import java.util.Timer;
import java.util.TimerTask;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.MacAddressEntry;
import org.opendaylight.vtn.manager.VTNException;

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
    private final static Logger  LOG =
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
    private final static int  AGING_PURGE_THRESHOLD = 60;

    /**
     * The name of the table.
     */
    private final String  tableName;

    /**
     * Interval in seconds between aging task.
     */
    private int  ageInterval;

    /**
     * MAC address table.
     */
    private final TreeMap<Long, MacTableEntry>  macAddressTable =
        new TreeMap<Long, MacTableEntry>();

    /**
     * Current MAC address table aging task.
     */
    private transient MacTableAgingTask  agingTask;

    /**
     * Periodic timer task used for MAC address table aging.
     */
    private class MacTableAgingTask extends TimerTask {
        /**
         * Scan MAC address table for aging.
         */
        @Override
        public void run() {
            LOG.trace("{}: Aging task called", tableName);
            age();
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
        return new Long(mac);
    }

    /**
     * Construct a new MAC address table.
     *
     * @param mgr   VTN Manager service.
     * @param name  The name of the table.
     * @param age   Interval in seconds between aging task.
     */
    public MacAddressTable(VTNManagerImpl mgr, String name, int age) {
        this.tableName = name;

        // Register an aging task to the global timer.
        IVTNResourceManager resMgr = mgr.getResourceManager();
        Timer timer = resMgr.getTimer();
        installAging(timer, age);
    }

    /**
     * Set interval between MAC address table aging task.
     *
     * @param mgr  VTN Manager service.
     * @param age  Interval in seconds between aging task.
     */
    public void setAgeInterval(VTNManagerImpl mgr, int age) {
        if (age != ageInterval) {
            IVTNResourceManager resMgr = mgr.getResourceManager();
            Timer timer = resMgr.getTimer();

            synchronized (this) {
                LOG.trace("{}: Aging interval changed: {} -> {}",
                          tableName, ageInterval, age);

                // Cancel current aging task.
                cancelAging(timer);

                // Install a new aging task.
                installAging(timer, age);
            }
        }
    }

    /**
     * Add a MAC address table entry if needed.
     *
     * @param mgr   VTN Manager service.
     * @param pctx  The context of the received packet.
     */
    public void add(VTNManagerImpl mgr, PacketContext pctx) {
        byte[] src = pctx.getSourceAddress();
        if (!NetUtils.isUnicastMACAddr(src)) {
            return;
        }

        // Determine attributes of a new MAC address table entry.
        Long key = getTableKey(src);
        NodeConnector port = pctx.getIncomingNodeConnector();
        short vlan = pctx.getVlan();
        byte[] sip = pctx.getSourceIpAddress();
        InetAddress ipaddr = null;
        if (sip != null) {
            try {
                ipaddr = InetAddress.getByAddress(sip);
            } catch (UnknownHostException e) {
                // This should never happen.
                if (LOG.isErrorEnabled()) {
                    String strip = HexEncode.bytesToHexStringFormat(sip);
                    LOG.error("{}: Invalid IP address: {}, ipaddr={}",
                              tableName, pctx.getDescription(port), strip);
                }
            }
        }

        boolean needProbe;
        synchronized (this) {
            // Search for a table entry.
            MacTableEntry tent = macAddressTable.get(key);
            if (tent != null) {
                // Check to see if the entry needs to be changed.
                NodeConnector curport = tent.getPort();
                short curvlan = tent.getVlan();
                boolean changed = false;
                if (vlan != curvlan || !port.equals(curport)) {
                    // The host was moved to other network.
                    pctx.addObsoleteEntry(key, tent);

                    // Replace the table entry.
                    tent = new MacTableEntry(port, vlan, ipaddr);
                    macAddressTable.put(key, tent);
                    changed = true;
                } else {
                    if (ipaddr != null) {
                        // Append IP address to this entry.
                        changed = tent.addInetAddress(ipaddr);
                    }

                    // Turn the used flag on.
                    tent.setUsed();
                }

                if (changed && LOG.isInfoEnabled()) {
                    String strmac = HexEncode.bytesToHexStringFormat(src);
                    LOG.info("{}: MAC address table entry changed: {}, {}",
                             tableName, strmac, tent);
                }
            } else {
                // Register a new table entry.
                tent = new MacTableEntry(port, vlan, ipaddr);
                if (LOG.isDebugEnabled()) {
                    String strmac = HexEncode.bytesToHexStringFormat(src);
                    LOG.debug("{}: New MAC address table entry: {}, {}",
                              tableName, strmac, tent);
                }
                macAddressTable.put(key, tent);
            }
            needProbe = tent.isProbeNeeded();
        }

        // Notify the host tracker of new host entry.
        if (ipaddr != null) {
            try {
                HostNodeConnector host =
                    new HostNodeConnector(src, ipaddr, port, vlan);
                if (LOG.isDebugEnabled()) {
                    LOG.debug("{}: Notify new host: ipaddr={}, host={}",
                              tableName, ipaddr.getHostAddress(), host);
                }
                mgr.notifyHost(host);
            } catch (Exception e) {
                LOG.error(tableName + ": Unable to create a host entry", e);
            }
        } else if (needProbe) {
            // Try to detect IP address of the host.
            pctx.probeInetAddress(mgr);
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
            MacTableEntry tent = macAddressTable.get(key);
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
            macAddressTable.remove(key);
        }
    }

    /**
     * Return a list of MAC address entries in this table.
     *
     * @return  A list of MAC address entries.
     * @throws VTNException  An error occurred.
     */
    public List<MacAddressEntry> getEntries() throws VTNException {
        ArrayList<MacAddressEntry> list =
            new ArrayList<MacAddressEntry>(macAddressTable.size());

        synchronized (this) {
            for (Iterator<Map.Entry<Long, MacTableEntry>> it =
                     macAddressTable.entrySet().iterator(); it.hasNext();) {
                Map.Entry<Long, MacTableEntry> entry = it.next();
                Long mac = entry.getKey();
                MacTableEntry tent = entry.getValue();
                list.add(tent.getEntry(mac.longValue()));
            }
        }

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
            MacTableEntry tent = macAddressTable.get(key);
            if (tent == null) {
                return null;
            }
            return tent.getEntry(key.longValue());
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
            tent = macAddressTable.remove(key);
        }

        if (tent == null) {
            return null;
        }

        if (LOG.isDebugEnabled()) {
            String strmac = HexEncode.longToHexString(key.longValue());
            LOG.debug("{}: removeEntry: MAC address removed: {}, {}",
                      tableName, strmac, tent);
        }
        return tent.getEntry(key.longValue());
    }

    /**
     * Flush all MAC address table entries.
     */
    public synchronized void flush() {
        LOG.debug("{}: MAC address table flushed", tableName);
        macAddressTable.clear();
    }

    /**
     * Flush all MAC address table entries associated with the given node.
     *
     * @param node  A node.
     */
    public synchronized void flush(Node node) {
        for (Iterator<Map.Entry<Long, MacTableEntry>> it =
                 macAddressTable.entrySet().iterator(); it.hasNext();) {
            Map.Entry<Long, MacTableEntry> entry = it.next();
            MacTableEntry tent = entry.getValue();
            Node tnode = tent.getPort().getNode();
            if (tnode.equals(node)) {
                if (LOG.isDebugEnabled()) {
                    long mac = entry.getKey().longValue();
                    String strmac = HexEncode.longToHexString(mac);
                    LOG.debug("{}: MAC address removed: {}, {}",
                              tableName, strmac, tent);
                }
                it.remove();
            }
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
        for (Iterator<Map.Entry<Long, MacTableEntry>> it =
                 macAddressTable.entrySet().iterator(); it.hasNext();) {
            Map.Entry<Long, MacTableEntry> entry = it.next();
            MacTableEntry tent = entry.getValue();
            if (tent.getVlan() == vlan &&
                (node == null || node.equals(tent.getPort().getNode()))) {
                if (LOG.isDebugEnabled()) {
                    long mac = entry.getKey().longValue();
                    String strmac = HexEncode.longToHexString(mac);
                    LOG.debug("{}: MAC address removed: {}, {}",
                              tableName, strmac, tent);
                }
                it.remove();
            }
        }
    }

    /**
     * Flush all MAC address table entries associated with the given node
     * connector.
     *
     * @param nc  A node connector.
     */
    public synchronized void flush(NodeConnector nc) {
        for (Iterator<Map.Entry<Long, MacTableEntry>> it =
                 macAddressTable.entrySet().iterator(); it.hasNext();) {
            Map.Entry<Long, MacTableEntry> entry = it.next();
            MacTableEntry tent = entry.getValue();
            NodeConnector tport = tent.getPort();
            if (tport.equals(nc)) {
                if (LOG.isDebugEnabled()) {
                    long mac = entry.getKey().longValue();
                    String strmac = HexEncode.longToHexString(mac);
                    LOG.debug("{}: MAC address removed: {}, {}",
                              tableName, strmac, tent);
                }
                it.remove();
            }
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
        for (Iterator<Map.Entry<Long, MacTableEntry>> it =
                 macAddressTable.entrySet().iterator(); it.hasNext();) {
            Map.Entry<Long, MacTableEntry> entry = it.next();
            MacTableEntry tent = entry.getValue();
            if (tent.getVlan() == vlan && nc.equals(tent.getPort())) {
                if (LOG.isDebugEnabled()) {
                    long mac = entry.getKey().longValue();
                    String strmac = HexEncode.longToHexString(mac);
                    LOG.debug("{}: MAC address removed: {}, {}",
                              tableName, strmac, tent);
                }
                it.remove();
            }
        }
    }

    /**
     * Destroy the MAC address table.
     *
     * @param mgr  VTN Manager service.
     *             If a non-{@code null} value is specified, this method tries
     *             to purge canceled tasks from the global timer task queue.
     */
    public void destroy(VTNManagerImpl mgr) {
        // Cancel the aging task.
        Timer timer;
        if (mgr != null) {
            IVTNResourceManager resMgr = mgr.getResourceManager();
            timer = resMgr.getTimer();
        } else {
            timer = null;
        }

        cancelAging(timer);
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
        long milli = (long)age * 1000L;
        timer.schedule(agingTask, milli, milli);
    }

    /**
     * Cancel the MAC address table aging task.
     *
     * @param timer  The global timer thread.
     *               If a non-{@code null} value is specified, this method
     *               tries to purge canceled tasks from the global timer task
     *               queue.
     */
    private synchronized void cancelAging(Timer timer) {
        // Cancel current aging task.
        agingTask.cancel();

        if (timer != null && ageInterval > AGING_PURGE_THRESHOLD) {
            // Purge canceled task.
            timer.purge();
        }
    }

    /**
     * Scan MAC address table entries, and eliminate unused entries.
     */
    private synchronized void age() {
        for (Iterator<Map.Entry<Long, MacTableEntry>> it =
                 macAddressTable.entrySet().iterator(); it.hasNext();) {
            Map.Entry<Long, MacTableEntry> entry = it.next();
            MacTableEntry tent = entry.getValue();
            // Turn the used flag off.
            if (!tent.clearUsed()) {
                // Remove this entry.
                if (LOG.isDebugEnabled()) {
                    long mac = entry.getKey().longValue();
                    String strmac = HexEncode.longToHexString(mac);
                    LOG.debug("{}: MAC address aged out: {}, {}",
                              tableName, strmac, tent);
                }
                it.remove();
            }
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
}
