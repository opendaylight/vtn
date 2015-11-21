/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.log;

import java.util.Set;
import java.util.HashSet;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.TestBase;

/**
 * JUnit test for {@link FixedLoggerCache}.
 */
public class FixedLoggerCacheTest extends TestBase {
    /**
     * Test case for {@link FixedLoggerCache#get(Logger, VTNLogLevel)}.
     */
    @Test
    public void testGet() {
        Logger[] loggers = {
            LoggerFactory.getLogger("TestLogger1"),
            LoggerFactory.getLogger("TestLogger2"),
            LoggerFactory.getLogger("TestLogger3"),
        };
        VTNLogLevel[] levels = VTNLogLevel.values();

        FixedLoggerCache cache = new FixedLoggerCache();
        Set<FixedLogger> fixedLoggers = new HashSet<>();
        for (Logger logger: loggers) {
            for (VTNLogLevel level: levels) {
                FixedLogger flogger = cache.get(logger, level);
                assertEquals(logger, flogger.getLogger());
                assertEquals(level, flogger.getLogLevel());
                assertTrue(fixedLoggers.add(flogger));

                // Created FixedLogger instances should be cached.
                assertSame(flogger, cache.get(logger, level));
            }
        }

        for (Logger logger: loggers) {
            for (VTNLogLevel level: levels) {
                FixedLogger flogger = cache.get(logger, level);
                assertEquals(logger, flogger.getLogger());
                assertEquals(level, flogger.getLogLevel());
                assertFalse(fixedLoggers.add(flogger));
            }
        }
    }
}
