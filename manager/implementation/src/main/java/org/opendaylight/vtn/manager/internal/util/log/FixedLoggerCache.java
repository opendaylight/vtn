/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.log;

import java.util.EnumMap;
import java.util.HashMap;
import java.util.Map;

import org.slf4j.Logger;

/**
 * {@code FixedLoggerCache} describes a cache for {@link FixedLogger}
 * instances.
 */
public final class FixedLoggerCache {
    /**
     * A map which keeps cached logger instances.
     */
    private final Map<VTNLogLevel, Map<Logger, FixedLogger>>  loggerCache =
        new EnumMap<>(VTNLogLevel.class);

    /**
     * Return a {@link FixedLogger} instance that keeps the given logger and
     * logging level.
     *
     * <p>
     *   If a fixed logger for the specified logger and logging level is
     *   already cached, this method returns a cached instance.
     *   If not cached, this method creates a new fixed logger and return it.
     * </p>
     *
     * @param logger  A {@link Logger} instance.
     * @param level   A {@link VTNLogLevel} instance.
     * @return  A {@link FixedLogger} instance.
     */
    public FixedLogger get(Logger logger, VTNLogLevel level) {
        FixedLogger flogger;
        Map<Logger, FixedLogger> levelCache = loggerCache.get(level);
        if (levelCache == null) {
            // Create a new intermediate map.
            levelCache = new HashMap<>();
            loggerCache.put(level, levelCache);
        } else {
            flogger = levelCache.get(logger);
            if (flogger != null) {
                return flogger;
            }
        }

        // Create a new fixed logger.
        flogger = new FixedLogger(logger, level);
        levelCache.put(logger, flogger);

        return flogger;
    }
}
