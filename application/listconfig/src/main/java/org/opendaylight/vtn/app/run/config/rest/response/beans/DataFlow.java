/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.app.run.config.rest.response.beans;

import java.util.ArrayList;
import java.util.List;

import org.opendaylight.vtn.app.run.config.json.annotations.JsonArray;
import org.opendaylight.vtn.app.run.config.json.annotations.JsonElement;
import org.opendaylight.vtn.app.run.config.json.annotations.JsonObject;
import org.opendaylight.vtn.app.run.config.json.annotations.JsonObjectRef;

/**
 * DataFlow - Bean Representaion for DataFlow object from the JSON Response.
 *
 */
@JsonObject
public class DataFlow {

    /**
     * Unique id for data flow.
     */
    @JsonElement(name = "id")
    private long id = 0;

    /**
     * Flow created time.
     */
    @JsonElement(name = "creationTime")
    private long creationTime = 1234567;

    /**
     * Idle timeout in flow.
     */
    @JsonElement(name = "idleTimeout")
    private int idleTimeout = 300;

    /**
     * Timeout in flow.
     */
    @JsonElement(name = "hardTimeout")
    private int hardTimeout = 0;

    /**
     * IngressNode in flow.
     */
    @JsonObjectRef(name = "IngressNode")
    private IngressNode inNode = new IngressNode();

    /**
     * ingressPort in flow.
     */
    @JsonObjectRef(name = "ingressPort")
    private IngressPort inportobj = new IngressPort();

    /**
     * egressNode in flow.
     */
    @JsonObjectRef(name = "egressNode")
    private EgressNode enobj = new EgressNode();

    /**
     * egressPort in flow.
     */
    @JsonObjectRef(name = "egressPort")
    private EgressPort gpobj = new EgressPort();

    /**
     * List of matches for flow.
     */
    @JsonObjectRef(name = "match")
    private Match matchobj = new Match();

    /**
     * List of actions for flow.
     */
    @JsonArray(name = "actions")
    private List<Action> actobj = new ArrayList<Action>();

    /**
     * List of vnoderoute for flow.
     */
    @JsonArray(name = "vnoderoute")
    private List<VNodeRoute> vnrobj = new ArrayList<VNodeRoute>();

    /**
     * List of noderoute for flow.
     */
    @JsonArray(name = "noderoute")
    private List<NodeRoute> nrobj = new ArrayList<NodeRoute>();

    /**
     * Reference of statistics.
     */
    @JsonObjectRef(name = "statistics")
    private Statistics staobj = new Statistics();

    /**
     * getId - function to get the id.
     *
     * @return The identifier of the data flow.
     */
    public long getId() {
        return id;
    }

    /**
     * setId - function to set the id.
     *
     * @param id
     */
    public void setId(long id) {
        this.id = id;
    }

    /**
     * getCreationTime - function to get the creationTime.
     *
     * @return The system time when the data flow was created.
     */
    public long getCreationTime() {
        return creationTime;
    }

    /**
     * setId - function to set the creationTime.
     *
     * @param creationTime
     */
    public void setCreationTime(long creationTime) {
        this.creationTime = creationTime;
    }

    /**
     * getIdleTimeout - function to get the idleTimeout.
     *
     * @return The idle timeout value.
     */
    public int getIdleTimeout() {
        return idleTimeout;
    }

    /**
     * setIdleTimeout - function to set the idleTimeout.
     *
     * @param idleTimeout
     */
    public void setIdleTimeout(int idleTimeout) {
        this.idleTimeout = idleTimeout;
    }

    /**
     * getHardTimeout - function to get the hardTimeout.
     *
     * @return The hard timeout value.
     */
    public int getHardTimeout() {
        return hardTimeout;
    }

    /**
     * setHardTimeout - function to set the hardTimeout.
     *
     * @param hardTimeout
     */
    public void setHardTimeout(int hardTimeout) {
        this.hardTimeout = hardTimeout;
    }

    /**
     * getInNode - function to get the inNode.
     *
     * @return {@link IngressNode}
     */
    public IngressNode getInNode() {
        return inNode;
    }

    /**
     * setInNode - function to get the inNode.
     *
     * @param inNode
     */
    public void setInNode(IngressNode inNode) {
        this.inNode = inNode;
    }

    /**
     * getInportobj - function to get the inportobj.
     *
     * @return {@link IngressPort}
     */
    public IngressPort getInportobj() {
        return inportobj;
    }

    /**
     * setInportobj - function to get the inportobj.
     *
     * @param inportobj
     */
    public void setInportobj(IngressPort inportobj) {
        this.inportobj = inportobj;
    }

    /**
     * getEnobj - function to get the enobj.
     *
     * @return {@link EgressNode}
     */
    public EgressNode getEnobj() {
        return enobj;
    }

    /**
     * setEnobj - function to set the enobj.
     *
     * @param enobj
     */
    public void setEnobj(EgressNode enobj) {
        this.enobj = enobj;
    }

    /**
     * EgressPort - function to get the gpobj.
     *
     * @return {@link EgressPort}
     */
    public EgressPort getGpobj() {
        return gpobj;
    }

    /**
     * setGpobj - function to get the gpobj.
     *
     * @param gpobj
     */
    public void setGpobj(EgressPort gpobj) {
        this.gpobj = gpobj;
    }

    /**
     * getId - function to get the id.
     *
     * @return {@link Match}
     */
    public Match getMatchobj() {
        return matchobj;
    }

    /**
     * setMatchobj - function to get the matchobj.
     *
     * @param matchobj {@link Match}
     */
    public void setMatchobj(Match matchobj) {
        this.matchobj = matchobj;
    }

    /**
     * getActobj - function to get the Actions.
     *
     * @return {@link List}
     */
    public List<Action> getActobj() {
        return actobj;
    }

    /**
     * setActobj - function to set the Actions.
     *
     * @param actobj
     */
    public void setActobj(List<Action> actobj) {
        this.actobj = actobj;
    }

    /**
     * getVnrobj - function to get list of VNodeRoute.
     *
     * @return {@link List}
     */
    public List<VNodeRoute> getVnrobj() {
        return vnrobj;
    }

    /**
     * setVnrobj - function to set the VNodeRoute.
     *
     * @param vnrobj
     */
    public void setVnrobj(List<VNodeRoute> vnrobj) {
        this.vnrobj = vnrobj;
    }

    /**
     * getNrobj - function to get the NodeRoute.
     *
     * @return {@link List}
     */
    public List<NodeRoute> getNrobj() {
        return nrobj;
    }

    /**
     * setNrobj - function to set the NodeRoute.
     *
     * @param nrobj
     */
    public void setNrobj(List<NodeRoute> nrobj) {
        this.nrobj = nrobj;
    }

    /**
     * getStaobj - function to get the Statistics.
     *
     * @return {@link Statistics}
     */
    public Statistics getStaobj() {
        return staobj;
    }

    /**
     * setStaobj - function to set the Statistics.
     *
     * @param staobj
     */
    public void setStaobj(Statistics staobj) {
        this.staobj = staobj;
    }

    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "DataFlow [id = " + id + ", creationTime = " + creationTime
                + ", idleTimeout = " + idleTimeout + ", hardTimeout = "
                + hardTimeout + ", inNode = " + inNode + ", inportobj = "
                + inportobj + ", enobj = " + enobj + ", gpobj = " + gpobj
                + ", matchobj = " + matchobj + ", actobj = " + actobj + ", vnrobj = "
                + vnrobj + ", nrobj = " + nrobj + ", staobj = " + staobj + "]";
    }
}
