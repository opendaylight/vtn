/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.app.run.config.rest.response.beans;

import java.util.List;

/**
 * VTNConfiguration - Bean Representaion for VTNConfiguration object from the JSON
 * Response.
 *
 */
public class VTNConfiguration {
    /**
     * version for the VTN Manager.
     */
    private VTNManagerVersion version = null;

    /**
     * ConditionList for the Flow.
     */
    private FlowconditionList flowconditions = null;
    /**
     * list of pathPolicies reference object.
     */
    private List<PathPolicy> pathPolicies = null;

    /**
     * ContainerPathMap for the VTN.
     */
    private ContainerPathMapNorthBoundList pathmap = null;

    /**
     * list of runConfiguration for the VTN.
     */
    private List<VTNConfigurationBean> vtn =  null;

    /**
     * getVersion - function to get the version value for this      object.
     *
     * @return {@link VTNManagerVersion }
     */
    public VTNManagerVersion getVersion() {
        return version;
    }
   /**
     * setVersion- function to get the version value for this    object.
     *
     * @param version
     */
    public void setVersion(VTNManagerVersion version) {
        this.version = version;
    }
    /**
     * getFlowconditions - function to get the flowconditions value for this object.
     *
     * @return {@link FlowconditionList }
     */
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
     * getVtn - function to get the vtn for this object.
     *
     * @return A list of VTNs.
     */
    public List<VTNConfigurationBean> getVtn() {
        return vtn;
    }

    /**
     * setVtn - function to set the vtn for this object.
     *
     * @param vtn
     */
    public void setVtn(List<VTNConfigurationBean> vtn) {
        this.vtn = vtn;
    }

    /**
     * getPathmap - function to get the pathmap for this object.
     *
     * @return pathmap
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

    /**
     * Default Constructor
     *
     */
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
