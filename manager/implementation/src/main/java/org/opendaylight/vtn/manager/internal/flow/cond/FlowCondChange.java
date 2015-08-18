/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.cond;

import org.opendaylight.vtn.manager.internal.util.ConfigFileUpdater;
import org.opendaylight.vtn.manager.internal.util.XmlConfigFile;
import org.opendaylight.vtn.manager.internal.util.flow.cond.VTNFlowCondition;

/**
 * {@code FlowCondChange} describes changes to the flow condition to be applied
 * to the configuration file.
 *
 * <p>
 *   Note that this class is not synchronized.
 * </p>
 */
final class FlowCondChange extends ConfigFileUpdater<String, VTNFlowCondition> {
    /**
     * Construct a new instance.
     */
    FlowCondChange() {
        super(XmlConfigFile.Type.FLOWCOND, "Flow condition");
    }
}
