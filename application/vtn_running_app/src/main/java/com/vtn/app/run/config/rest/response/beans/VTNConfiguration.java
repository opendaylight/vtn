/**
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package com.vtn.app.run.config.rest.response.beans;

import java.util.List;

public class VTNConfiguration {

    /**
     * VTNConfiguration - Bean Representaion for VTNConfiguration object from the JSON
     * Response.
     *
     */
    private VTNManagerVersion version = null;

    private FlowconditionList flowconditions = null;

    private List<PathPolicy> pathPolicies = null;

    private ContainerPathMapNorthBoundList pathmap = null;

    private List<VTNConfigurationBean> runConfig =  null;

    public VTNManagerVersion getVersion() {
        return version;
    }

    public void setVersion(VTNManagerVersion version) {
        this.version = version;
    }

    public FlowconditionList getFlowconditions() {
        return flowconditions;
    }

     /**returns path policy**/
    public List<PathPolicy> getPathPolicies() {
        return pathPolicies;
    }

    /**
     * setPathPolicies - function to set the pathPolicies for this object.
     *
     * @param pathPolicies
     */
    public void setPathPolicies(List<PathPolicy> pathPolicies) {
        this.pathPolicies = pathPolicies;
    }
    /**
     * setFlowconditions - function to set the flowconditions for this object.
     *
     * @param flowconditions
     */
    public void setFlowconditions(FlowconditionList flowconditions) {
        this.flowconditions = flowconditions;
    }
    /**
     * getRunConfig - function to get the runConfig for this object.
     *
     * @return {@link runConfig }
     */
    public runConfig getRunConfig() {
        return runConfig;
    }
    /**
     * setRunConfig - function to set the runConfig for this object.
     *
     * @param runConfig
     */
    public void setRunConfig(List<VTNConfigurationBean> runConfig) {
        this.runConfig = runConfig;
    }

    /**
     * getPathmap - function to get the pathmap for this object.
     *
     * @return {@link pathmap }
     */
    public ContainerPathMapNorthBoundList getPathmap() {
        return pathmap;
    }

    /**
     * setPathmap - function to set the pathmap for this object.
     *
     * @param pathmap
     */
    public void setPathmap(ContainerPathMapNorthBoundList pathmap) {
        this.pathmap = pathmap;
    }

    public VTNConfiguration() {}

    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "";
    }
}
