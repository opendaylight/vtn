/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow;

import java.io.Serializable;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlAttribute;
import javax.xml.bind.annotation.XmlRootElement;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

/**
 * This class describes statistics information of a flow entry or a data flow
 * installed by the VTN Manager.
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"packets": 7,
 * &nbsp;&nbsp;"bytes": 638,
 * &nbsp;&nbsp;"duration": 36510
 * &nbsp;&nbsp;"packets-per-sec": 1,
 * &nbsp;&nbsp;"bytes-per-sec": 98,
 * &nbsp;&nbsp;"interval": 10000
 * }</pre>
 *
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "flowstats")
@XmlAccessorType(XmlAccessType.NONE)
public final class FlowStats implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -1666392947101292135L;

    /**
     * The number of packets forwarded by the flow entry.
     */
    @XmlAttribute(name = "packets")
    private long  packetCount;

    /**
     * The total number of bytes in packets forwarded by the flow entry.
     */
    @XmlAttribute(name = "bytes")
    private long  byteCount;

    /**
     * Duration of the flow entry, in milliseconds.
     */
    @XmlAttribute
    private long  duration;

    /**
     * The averaged over the statistics interval number of packets forwarded by the flow entry.
     */
    @XmlAttribute(name = "packets-per-sec")
    private long  packetsPerSec;

    /**
     * The averaged over the statistics interval number of bytes in packets forwarded by the flow entry.
     */
    @XmlAttribute(name = "bytes-per-sec")
    private long  bytesPerSec;

    /**
     * The statistics interval, in milliseconds.
     */
    @XmlAttribute
    private long  interval;

    /**
     * Private constructor only for JAXB.
     */
    private FlowStats() {
    }

    /**
     * Construct a new instance.
     *
     * @param packets        The number of packets forwarded by the flow entry.
     * @param bytes          The total number of bytes in packets forwarded by the
     *                       flow entry.
     * @param duration       Duration of the flow entry, in milliseconds.
     * @param packetsPerSec  The averaged over the statistics interval number of packets
     *                       forwarded by the flow entry.
     * @param bytesPerSec    The averaged over the statistics interval number of bytes in packets
     *                       forwarded by the flow entry.
     * @param interval       The statistics interval, in milliseconds.
     */
    public FlowStats(long packets, long bytes, long duration, long  packetsPerSec, long  bytesPerSec, long  interval) {
        this.packetCount = packets;
        this.byteCount = bytes;
        this.duration = duration;
        this.packetsPerSec = packetsPerSec;
        this.bytesPerSec = bytesPerSec;
        this.interval = interval;
    }

    /**
     * Return the number of packets forwarded by the flow entry.
     *
     * @return  The number of packets forwarded by the flow entry.
     */
    public long getPacketCount() {
        return packetCount;
    }

    /**
     * Return the total number of bytes in packets forwarded by the flow entry.
     *
     * @return  The total number of bytes in packets forwarded by the
     *          flow entry.
     */
    public long getByteCount() {
        return byteCount;
    }

    /**
     * Return the duration of the flow entry.
     *
     * @return  The duration of the flow entry, in milliseconds.
     */
    public long getDuration() {
        return duration;
    }

    /**
     * Return the averaged over the statistics interval number of packets forwarded by the flow entry.
     *
     * @return  The averaged over the statistics interval number of packets forwarded by the flow entry.
     */
    public long getPacketsPerSec() {
        return packetsPerSec;
    }

    /**
     * Return the averaged over the statistics interval number of bytes in packets forwarded by the flow entry.
     *
     * @return  The averaged over the statistics interval number of bytes in packets forwarded by the flow entry.
     */
    public long getBytesPerSec() {
        return bytesPerSec;
    }

    /**
     * Return the statistics interval.
     *
     * @return  The statistics interval, in milliseconds.
     */
    public long getInterval() {
        return interval;
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        if (o == this) {
            return true;
        }
        if (!(o instanceof FlowStats)) {
            return false;
        }

        FlowStats stats = (FlowStats)o;
        return (packetCount == stats.packetCount &&
                byteCount == stats.byteCount &&
                duration == stats.duration &&
                packetsPerSec == stats.packetsPerSec &&
                bytesPerSec == stats.bytesPerSec &&
                interval == stats.interval);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return (int)(packetCount + byteCount + duration + packetsPerSec + bytesPerSec + interval);
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("FlowStats[packets=");
        builder.append(packetCount).append(",bytes=").append(byteCount).
            append(",duration=").append(duration).append(",packets-per-sec=").
            append(packetsPerSec).append(",bytes-per-sec=").append(bytesPerSec).
            append(",interval=").append(interval).append(']');

        return builder.toString();
    }
}
