/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.manager.internal.routing;

import java.util.Collections;
import java.util.List;
import java.util.Timer;
import java.util.concurrent.Future;

import org.mockito.Mockito;

import com.google.common.util.concurrent.FutureCallback;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.internal.FlowSelector;
import org.opendaylight.vtn.manager.internal.RouteResolver;
import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.TxTask;
import org.opendaylight.vtn.manager.internal.VTNConfig;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFuture;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.sal.packet.Packet;
import org.opendaylight.yangtools.yang.binding.Notification;
import org.opendaylight.yangtools.yang.binding.RpcService;

/**
  * VTNProviderStub.
  */
public class VTNProviderStub implements VTNManagerProvider {

    /**
     * @param command
     */
    @Override
    public void execute(Runnable command) {
        // TODO Auto-generated method stub

    }

    /**
     * @param <T>
     * @param task
     * @return
     */
    @Override
    public <T> VTNFuture<T> post(TxTask<T> task) {
        // TODO Auto-generated method stub
        return null;
    }

    /**
     * @param <T>
     * @param task
     * @return
     */
    @Override
    public <T> VTNFuture<T> postFirst(TxTask<T> task) {
        // TODO Auto-generated method stub
        return null;
    }

    /**
     * @param mgr
     */
    @Override
    public void setVTNManager(VTNManagerImpl mgr) {
        // TODO Auto-generated method stub

    }

    /**
     *  shutdown method.
     */
    @Override
    public void shutdown() {
        // TODO Auto-generated method stub

    }

    /**
     *  configLoaded method.
     */
    @Override
    public void configLoaded() {
        // TODO Auto-generated method stub

    }

    /**
     * @return null from getVTNConfig method.
     */
    @Override
    public VTNConfig getVTNConfig() {
        // TODO Auto-generated method stub
        return null;
    }

    /**
     * @return null
     */
    @Override
    public TxContext newTxContext() {
        // TODO Auto-generated method stub
        return null;
    }

    /**
     * @return null
     */
    @Override
    public Timer getTimer() {
        // TODO Auto-generated method stub
        return null;
    }

    /**
     * @param task
     * @return
     */
    @Override
    public boolean executeTask(Runnable task) {
        // TODO Auto-generated method stub
        return false;
    }

    /**
     * @return null value from getRouteResolver method.
     */
    @Override
    public RouteResolver getRouteResolver() {
        // TODO Auto-generated method stub
        return null;
    }

    /**
     * @param id
     * @return
     */
    @Override
    public RouteResolver getRouteResolver(Integer id) {
        // TODO Auto-generated method stub
        return null;
    }

    /**
     * @param selector
     * @return
     */
    @Override
    public List<VTNFuture<?>> removeFlows(FlowSelector selector) {
        // TODO Auto-generated method stub
        return Collections.<VTNFuture<?>>emptyList();
    }

    /**
     * @param tname
     * @param selector
     * @return
     */
    @Override
    public List<VTNFuture<?>> removeFlows(String tname, FlowSelector selector) {
        // TODO Auto-generated method stub
        return null;
    }

    /**
     * @param <T>
     * @param type
     * @return
     * @throws VTNException
     */
    @Override
    public <T extends RpcService> T getRpcService(Class<T> type)
        throws VTNException {
        // TODO Auto-generated method stub
        return null;
    }

    /**
     * @param <T>
     * @param type
     * @return
     * @throws VTNException
     */
    @Override
    public <T extends RpcService> T getVtnRpcService(Class<T> type)
        throws VTNException {
        // TODO Auto-generated method stub
        return null;
    }

    /**
     * @param <T>
     * @param task
     * @return
     */
    @Override
    public <T> VTNFuture<T> postSync(TxTask<T> task) {
        // TODO Auto-generated method stub
        return null;
    }

    /**
     * close method
     */
    @Override
    public void close() {
        // TODO Auto-generated method stub

    }

    /**
     * @param <T>
     * @param future
     * @param cb
     */
    @Override
    public <T> void setCallback(Future<T> future, FutureCallback<? super T> cb) {
        // TODO Auto-generated method stub

    }

    /**
     * @return
     */
    @Override
    public DataBroker getDataBroker() {
        // TODO Auto-generated method stub
        return Mockito.mock(DataBroker.class);
    }

    /**
     * @param egress
     * @param packet
     */
    @Override
    public void transmit(SalPort egress, Packet packet) {
        // TODO Auto-generated method stub

    }

    /**
     * @param n
     */
    @Override
    public void publish(Notification n) {
        // TODO Auto-generated method stub

    }

}
