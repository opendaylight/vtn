/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.app.run.config.rest.response.beans;

import org.opendaylight.vtn.app.run.config.json.annotations.JsonElement;
import org.opendaylight.vtn.app.run.config.json.annotations.JsonObject;

/**
 * Statistics - Bean Representaion for Statistics object from the JSON Response.
 *
 */
@JsonObject
public class Statistics {
    /**
     * Number of bytes for a Dataflow.
     */
    @JsonElement(name = "bytes")
    private int bytes = 638;

    /**
     * Number of packets for a data flow.
     */
    @JsonElement(name = "packets")
    private int packets = 7;

    /**
     * Duration for each packet.
     */
    @JsonElement(name = "duration")
    private int duration = 36510;

    public Statistics() {
    }

    /**
     * getBytes - function to get the bytes for this object.
     *
     * @return The number of bytes.
     */
    public int getBytes() {
        return bytes;
    }

    /**
     * setBytes - function to set the bytes for this object.
     *
     * @param bytes  The number of bytes.
     */
    public void setBytes(int bytes) {
        this.bytes = bytes;
    }

    /**
     * getPackets - function to get the packets for this object.
     *
     * @return The number of packets.
     */
    public int getPackets() {
        return packets;
    }

    /**
     * setPackets - function to set the packets for this object.
     *
     * @param packets  The number of packets.
     */
    public void setPackets(int packets) {
        this.packets = packets;
    }

    /**
     * getDuration - function to get the duration for this object.
     *
     * @return The duration of the data flow.
     */
    public int getDuration() {
        return duration;
    }

    /**
     * setDuration - function to set the duration for this object.
     *
     * @param duration The duration of the data flow.
     */
    public void setDuration(int duration) {
        this.duration = duration;
    }

    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "VTN RUN CONFIG [packets:" + packets + ",duration:" + duration
                + "]";
    }
}
