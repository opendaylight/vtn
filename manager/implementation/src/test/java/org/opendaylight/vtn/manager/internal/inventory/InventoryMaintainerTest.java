/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.inventory;

import org.junit.Test;
import org.opendaylight.vtn.manager.internal.TestBase;
import java.util.List;
import java.util.ArrayList;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.mockito.Mockito;
import org.opendaylight.vtn.manager.internal.TxQueue;
import org.opendaylight.vtn.manager.internal.util.tx.TxQueueImpl;
import org.opendaylight.vtn.manager.internal.TxTask;
import org.opendaylight.vtn.manager.internal.util.tx.CompositeTxTask;
import org.opendaylight.controller.md.sal.binding.api.DataBroker;

/**
 * InventoryMaintainer test Class is to test whether InventoryMaintainer class maintains
 *  the VTN inventory data or not.
 *
 * JUnit test for {@link InventoryMaintainer}
 */

public class InventoryMaintainerTest extends TestBase {
    /**
     * create a mock object for DataBroker class
     */
    DataBroker broker = Mockito.mock(DataBroker.class);
    /**
     * create a mock object for VTNManagerProvider class
     */
    VTNManagerProvider provider = Mockito.mock(VTNManagerProvider.class);
    /**
     * create a transaction submit queue to test the VTN inventory data models.
     */
    TxQueue txQueue = new TxQueueImpl("QUEUE1", provider);
    /**
     * create a object for InventoryMaintainer
     */
    InventoryMaintainer inventoryMaintainer = new NodeConnectorListener(txQueue, broker);

    /**
     * Test method for
     * {@link InventoryMaintainer#submit}.
     * whether execute the given transaction task on the transaction queue or not.
     */
    @Test
    public void testSubmit() {

        List<String> taskList = new ArrayList<String>();
        taskList.add("Task1");
        taskList.add("Task2");
        taskList.add("Task3");
        TxTask txTask = new CompositeTxTask(taskList);
        inventoryMaintainer.submit(txTask);
    }

    /**
     * Test method for
     * {@link InventoryMaintainer#submitInitial}.
     * whether execute the given transaction task for initialization on the
     * transaction queue or not
     */
    @Test
    public void testSubmitInitial() {

        List<String> taskList = new ArrayList<String>();
        taskList.add("Task4");
        taskList.add("Task5");
        taskList.add("Task6");
        TxTask txTask = new CompositeTxTask(taskList);
        inventoryMaintainer.submitInitial(txTask);
    }

}
