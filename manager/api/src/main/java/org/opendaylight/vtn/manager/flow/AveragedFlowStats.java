/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow;

import java.io.Serializable;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlAttribute;
import javax.xml.bind.annotation.XmlRootElement;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

import org.opendaylight.vtn.manager.util.NumberUtils;

/**
 * This class describes average flow statistics information per second.
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"bytes": 345.32,
 * &nbsp;&nbsp;"packets": 3.8,
 * &nbsp;&nbsp;"start": 1428516341931,
 * &nbsp;&nbsp;"end": 1428516352034
 * }</pre>
 *
 * @since  Lithium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "averagedflowstats")
@XmlAccessorType(XmlAccessType.NONE)
public final class AveragedFlowStats implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 6824293799484238673L;

    /**
     * The averaged number of packets forwarded by the flow entry per second.
     */
    @XmlAttribute(name = "packets")
    private double  packetCount;

    /**
     * The averaged number of bytes in packets forwarded by the flow entry
     * per second.
     */
    @XmlAttribute(name = "bytes")
    private double  byteCount;

    /**
     * The system time when the measurement period begins.
     *
     * <p>
     *   The value in this field represents the number of milliseconds
     *   between the start time and 1970-01-01T00:00:00+0000.
     * </p>
     */
    @XmlAttribute(name = "start")
    private long  startTime;

    /**
     * The system time when the measurement period ends.
     *
     * <p>
     *   The value in this field represents the number of milliseconds
     *   between the end time and 1970-01-01T00:00:00+0000.
     * </p>
     */
    @XmlAttribute(name = "end")
    private long  endTime;

    /**
     * Private constructor only for JAXB.
     */
    private AveragedFlowStats() {
    }

    /**
     * Construct a new instance.
     *
     * @param packets  The averaged number of packets forwarded by the flow
     *                 entry per second.
     * @param bytes    The averaged number of bytes in packets forwarded by
     *                 the flow entry per second.
     * @param start    The start time of the measurement period.
     * @param end      The end time of the measurement period.
     */
    public AveragedFlowStats(double packets, double bytes, long start,
                             long end) {
        packetCount = packets;
        byteCount = bytes;
        startTime = start;
        endTime = end;
    }

    /**
     * Return the averaged number of packets forwarded by the flow entry
     * per second.
     *
     * @return  The averaged number of packets forwarded by the flow entry
     *          per second.
     */
    public double getPacketCount() {
        return packetCount;
    }

    /**
     * Return the averaged number of bytes in packets forwarded by the flow
     * entry per second.
     *
     * @return  The averaged number of bytes in packets forwarded by the
     *          flow entry per second.
     */
    public double getByteCount() {
        return byteCount;
    }

    /**
     * Return the start time of the measurement period.
     *
     * @return  The number of milliseconds between the start time of the
     *          measurement period and 1970-01-01T00:00:00+0000.
     */
    public long getStartTime() {
        return startTime;
    }

    /**
     * Return the end time of the measurement period.
     *
     * @return  The number of milliseconds between the end time of the
     *          measurement period and 1970-01-01T00:00:00+0000.
     */
    public long getEndTime() {
        return endTime;
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
        if (o == this) {
            return true;
        }
        if (!(o instanceof AveragedFlowStats)) {
            return false;
        }

        AveragedFlowStats stats = (AveragedFlowStats)o;
        return (NumberUtils.equalsDouble(packetCount, stats.packetCount) &&
                NumberUtils.equalsDouble(byteCount, stats.byteCount) &&
                startTime == stats.startTime &&
                endTime == stats.endTime);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = NumberUtils.hashCode(packetCount);
        h = h * NumberUtils.HASH_PRIME + NumberUtils.hashCode(byteCount);
        h = h * NumberUtils.HASH_PRIME + NumberUtils.hashCode(startTime);
        return h * NumberUtils.HASH_PRIME + NumberUtils.hashCode(endTime);
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("AveragedFlowStats[packets=");
        builder.append(packetCount).append(",bytes=").append(byteCount).
            append(",start=").append(startTime)
            .append(",end=").append(endTime).append(']');

        return builder.toString();
    }
}
