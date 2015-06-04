/**
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.app.ate.beans.testsuitresults;

import java.util.ArrayList;
import java.util.List;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlElementWrapper;
import javax.xml.bind.annotation.XmlRootElement;

/**
 * Test suite object for Test suite result page.
 */
@XmlRootElement(name = "TestResults")
@XmlAccessorType(XmlAccessType.NONE)
public class TestResult {

    @XmlElement(name = "ControllerIP")
    private String controllerIP;

    @XmlElement(name = "ContainerName")
    private String containerName;

    @XmlElement(name = "TotalTestCases")
    private int totalTestCases;

    @XmlElement(name = "TotalSucceededTestCases")
    private int totalSucceededTestCases;

    @XmlElement(name = "TestSuite")
    @XmlElementWrapper(name = "TestSuites")
    private List<TestSuite> testSuite = new ArrayList<TestSuite>();

    public TestResult() {
    }

    public String getControllerIP() {
        return controllerIP;
    }

    public void setControllerIP(String controllerIP) {
        this.controllerIP = controllerIP;
    }

    public String getContainerName() {
        return containerName;
    }

    public void setContainerName(String containerName) {
        this.containerName = containerName;
    }

    public int getTotalTestCases() {
        return totalTestCases;
    }

    public void setTotalTestCases(int totalTestCases) {
        this.totalTestCases = totalTestCases;
    }

    public int getTotalSucceededTestCases() {
        return totalSucceededTestCases;
    }

    public void setTotalSucceededTestCases(int totalSucceededTestCases) {
        this.totalSucceededTestCases = totalSucceededTestCases;
    }

    public TestResult(List<TestSuite> testSuite) {
        this.testSuite = testSuite;
    }

    public List<TestSuite> getTestSuite() {
        return testSuite;
    }

    public void setTestSuite(List<TestSuite> testSuite) {
        this.testSuite = testSuite;
    }
}
