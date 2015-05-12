/**
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.app.ate.beans.executionengine;

import java.util.ArrayList;
import java.util.List;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

/**
 * Test Mapping class for mapping java class to each test case.
 */
@SuppressWarnings("restriction")
@XmlRootElement(name = "TestMappingClass")
@XmlAccessorType(XmlAccessType.NONE)
public class TestMappingClass {

    @XmlElement(name = "TestSuite")
    private List<TestSuite> testSuiteList = new ArrayList<TestSuite>();

    public TestMappingClass() {
    }

    public List<TestSuite> getTestSuiteList() {
        return testSuiteList;
    }

    public void setTestSuiteList(List<TestSuite> testSuiteList) {
        this.testSuiteList = testSuiteList;
    }
}
