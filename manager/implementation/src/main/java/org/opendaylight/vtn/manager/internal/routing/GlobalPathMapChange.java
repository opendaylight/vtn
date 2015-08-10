/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.routing;

import org.opendaylight.vtn.manager.PathMap;

import org.opendaylight.vtn.manager.internal.util.ConfigFileUpdater;
import org.opendaylight.vtn.manager.internal.util.XmlConfigFile;

/**
 * {@code GlobalPathMapChange} describes changes to the global path map to be
 * applied to the configuration file.
 *
 * <p>
 *   Note that this class is not synchronized.
 * </p>
 */
final class GlobalPathMapChange extends ConfigFileUpdater<Integer, PathMap> {
    /**
     * Construct a new instance.
     */
    GlobalPathMapChange() {
        super(XmlConfigFile.Type.PATHMAP, "Global path map");
    }
}
