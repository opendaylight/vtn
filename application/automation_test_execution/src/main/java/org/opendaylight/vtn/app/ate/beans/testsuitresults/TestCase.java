/**
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.app.ate.beans.testsuitresults;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlAttribute;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlTransient;

/**
 * Test case object for Test suite result page.
 */
@XmlAccessorType(XmlAccessType.NONE)
public class TestCase {

    @XmlTransient
    public final String testSuccess = "SUCCESS";

    @XmlTransient
    public final String testFailed = "FAILED";

    @XmlTransient
    public final String testNotExecuted = "NOT_EXECUTED";

    @XmlAttribute(name = "Name")
    private String name;

    @XmlElement(name = "Status")
    private String status;

    @XmlElement(name = "Error")
    private String error;

    public TestCase() {
    }

    public TestCase(String name, String status, String error) {
        this.name = name;
        this.status = status;
        this.error = error;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getStatus() {
        return status;
    }

    public void setStatus(String status) {
        this.status = status;
    }

    public String getError() {
        return error;
    }

    public void setError(String error) {
        this.error = error;
    }
}
