/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.stats;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.statistics.rev130819.FlowAndStatisticsMap;

/**
 * {@code StatsReaderCallback} is a callback for accepting flow statistics
 * information sent by a switch.
 *
 * <p>
 *   This callback is always invoked on a global single thread.
 * </p>
 */
public interface StatsReaderCallback {
    /**
     * Invoked when a flow statistics information has been received.
     *
     * @param fstats  A {@link FlowAndStatisticsMap} instance which contains
     *                the flow statistics send by the switch.
     */
    void flowStatsReceived(FlowAndStatisticsMap fstats);

    /**
     * Invoked when the transaction for reading flow statistics has been
     * completed successfully.
     */
    void transactionCompleted();

    /**
     * Invoked when the transaction for reading flow statistics has been
     * canceled.
     */
    void transactionCanceled();
}
