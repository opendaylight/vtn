/**
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.app.run.config.rest.response.beans;

import java.util.List;

import org.opendaylight.vtn.app.run.config.rest.output.format.beans.VBridgeBean;

/**
 * VTNConfigurationBean - Bean Representaion for VTNConfiguration object from
 * the JSON Response.
 *
 */
public class VTNConfigurationBean {
    /**
     * name for vtn configuration.
     */
    String name = null;

    /**
     * description for vtn configuration.
     */
    String description = null;

    /**
     * idleTimeout for vtn configuration.
     */
    int idleTimeout = 0;

    /**
     * hardTimeout for vtn configuration.
     */
    int hardTimeout = 0;

    /**
     * vbridge list for vtn configuration.
     */
    private List<VBridgeBean> vbridge = null;

    /**
     * DataFlowList for vtn configuration.
     */
    private DataFlowList dataflow = null;
    /**
     * Default Constructor.
     */
    public VTNConfigurationBean() {
    }

    /**
     * getDataflow - function to get the list .
     *
     * @return {@link List<DataFlowList> }
     */
    public DataFlowList getDataflow() {
        return dataflow;
    }

    /**
     * setDataflow - function to set the list .
     *
     * @param dataflow
     */
    public void setDataflow(DataFlowList dataflow) {
        this.dataflow = dataflow;
    }

    /**
     * getPath - function to get the list .
     *
     * @return {@link List<FlowFilter> }
     */
    public List<VBridgeBean> getVbridge() {
        return vbridge;
    }


    /**
     * setVbridge - function to set the list .
     *
     * @param vbridge
     */
    public void setVbridge(List<VBridgeBean> vbridge) {
        this.vbridge = vbridge;
    }

    /**
     * getPath - function to get the list .
     *
     * @return {@link List<FlowFilter> }
     */
    public String getName() {
        return name;
    }

    /**
     * setName - function to set the name .
     *
     * @param name
     */
    public void setName(String name) {
        this.name = name;
    }

    /**
     * getPath - function to get the description .
     *
     * @return {@link String }
     */
    public String getDescription() {
        return description;
    }

    /**
     * setDescription - function to set the description .
     *
     * @param description
     */
    public void setDescription(String description) {
        this.description = description;
    }

    /**
     * getPath - function to get the IdleTimeout .
     *
     * @return {@link int }
     */
    public int getIdleTimeout() {
        return idleTimeout;
    }

    /**
     * setIdleTimeout - function to set the idleTimeout for object
     *
     * @param idleTimeout
     */
    public void setIdleTimeout(int idleTimeout) {
        this.idleTimeout = idleTimeout;
    }

    /**
     * getHardTimeout - function to get the hardTimeout .
     *
     * @return {@link List<FlowFilter> }
     */
    public int getHardTimeout() {
        return hardTimeout;
    }

    /**
     * setHardTimeout - function to set the hardTimeout for object
     *
     * @param hardTimeout
     */
    public void setHardTimeout(int hardTimeout) {
        this.hardTimeout = hardTimeout;
    }

    /**
     * String representation of this object.
     */
    @Override
    public String toString() {
        return "VTN RUN CONFIG [name:" + name + ",description:" + description
                + ",idleTimeout:" + idleTimeout + ",hardTimeout:" + hardTimeout
                + "]";
    }
}
