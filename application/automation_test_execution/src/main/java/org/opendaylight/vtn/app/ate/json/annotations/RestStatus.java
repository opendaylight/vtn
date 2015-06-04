/**
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.app.ate.json.annotations;

public abstract class RestStatus {

    @SuppressWarnings("unused")
    private String statusTring = null;

    public void setStatusString(String statusString) {
        this.statusTring = statusString;
    }

    public abstract String getStatus();

    @Override
    public String toString() {
        return "Status :" + getStatus();
    }
}
