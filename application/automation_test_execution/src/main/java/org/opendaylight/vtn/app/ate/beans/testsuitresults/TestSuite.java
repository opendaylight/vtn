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
import javax.xml.bind.annotation.XmlAttribute;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlElementWrapper;
import javax.xml.bind.annotation.XmlTransient;

/**
 * Test suite list for Test suite result page.
 */
@XmlAccessorType(XmlAccessType.NONE)
public class TestSuite {

    @XmlTransient
    public final String testSuccess = "SUCCESS";

    @XmlTransient
    public final String testFailed = "FAILED";

    @XmlAttribute(name = "Name")
    private String testSuiteName;

    @XmlElementWrapper(name = "TestCases")
    @XmlElement(name = "TestCase")
    private List<TestCase> testCases = new ArrayList<TestCase>();

    @XmlElement(name = "Status")
    private String testSuiteStatus;

    public TestSuite() {
    }

    public String getTestSuiteName() {
        return testSuiteName;
    }

    public void setTestSuiteName(String testSuiteName) {
        this.testSuiteName = testSuiteName;
    }

    public TestSuite(List<TestCase> testCases, String testSuiteStatus) {
        this.testCases = testCases;
        this.testSuiteStatus = testSuiteStatus;
    }

    public List<TestCase> getTestCases() {
        return testCases;
    }

    public void setTestCases(List<TestCase> testCases) {
        this.testCases = testCases;
    }

    public String getTestSuiteStatus() {
        return testSuiteStatus;
    }

    public void setTestSuiteStatus(String testSuiteStatus) {
        this.testSuiteStatus = testSuiteStatus;
    }
}
