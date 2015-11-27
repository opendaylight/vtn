/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron.impl;

import static java.net.HttpURLConnection.HTTP_BAD_REQUEST;
import static java.net.HttpURLConnection.HTTP_CLIENT_TIMEOUT;
import static java.net.HttpURLConnection.HTTP_CONFLICT;
import static java.net.HttpURLConnection.HTTP_GONE;
import static java.net.HttpURLConnection.HTTP_INTERNAL_ERROR;
import static java.net.HttpURLConnection.HTTP_NOT_ACCEPTABLE;
import static java.net.HttpURLConnection.HTTP_NOT_FOUND;
import static java.net.HttpURLConnection.HTTP_OK;
import static java.net.HttpURLConnection.HTTP_UNAUTHORIZED;
import static java.net.HttpURLConnection.HTTP_UNAVAILABLE;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.reset;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;
import static org.mockito.MockitoAnnotations.initMocks;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.EnumMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.Future;

import com.google.common.base.Optional;
import com.google.common.util.concurrent.CheckedFuture;
import com.google.common.util.concurrent.Futures;

import org.junit.Test;

import org.mockito.Mock;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.ReadOnlyTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;
import org.opendaylight.controller.md.sal.common.api.data.ReadFailedException;
import org.opendaylight.controller.sal.binding.api.RpcConsumerRegistry;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;
import org.opendaylight.yangtools.yang.common.RpcError.ErrorType;
import org.opendaylight.yangtools.yang.common.RpcError;
import org.opendaylight.yangtools.yang.common.RpcResult;
import org.opendaylight.yangtools.yang.common.RpcResultBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.RemovePortMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.RemovePortMapInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.RemovePortMapOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.RemovePortMapOutputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.SetPortMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.SetPortMapInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.SetPortMapOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.SetPortMapOutputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.VtnPortMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.RemoveVtnInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.RemoveVtnInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.UpdateVtnInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.UpdateVtnInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.UpdateVtnOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.UpdateVtnOutputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.VtnService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.Vtns;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.Vtn;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.VtnBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.VtnKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeUpdateMode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateOperationType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.RemoveVbridgeInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.RemoveVbridgeInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.UpdateVbridgeInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.UpdateVbridgeInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.UpdateVbridgeOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.UpdateVbridgeOutputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.VtnVbridgeService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.info.VbridgeConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.info.VbridgeConfigBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.VbridgeBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.VbridgeKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.RemoveVinterfaceInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.RemoveVinterfaceInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.UpdateVinterfaceInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.UpdateVinterfaceInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.UpdateVinterfaceOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.UpdateVinterfaceOutputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.VtnVinterfaceService;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeId;

/**
 * JUnit test for {@link VTNManagerService}.
 */
public class VTNManagerServiceTest extends TestBase {
    /**
     * A map that keeps pairs of HTTP status code associated with VTN error
     * tag.
     */
    private static final Map<VtnErrorTag, Integer>  VTN_ERROR_CODES;

    /**
     * MD-SAL data broker.
     */
    @Mock
    private DataBroker  dataBroker;

    /**
     * RPC consumer registry.
     */
    @Mock
    private RpcConsumerRegistry  rpcRegistry;

    /**
     * RPC service for VTN management.
     */
    @Mock
    private VtnService  vtnService;

    /**
     * RPC service for vBridge management.
     */
    @Mock
    private VtnVbridgeService  vbridgeService;

    /**
     * RPC service for virtual interface management.
     */
    @Mock
    private VtnVinterfaceService  vinterfaceService;

    /**
     * RPC service for port mapping management.
     */
    @Mock
    private VtnPortMapService  portMapService;

    /**
     * Initialize statis field.
     */
    static {
        VTN_ERROR_CODES = new EnumMap<>(VtnErrorTag.class);
        VTN_ERROR_CODES.put(VtnErrorTag.BADREQUEST, HTTP_BAD_REQUEST);
        VTN_ERROR_CODES.put(VtnErrorTag.UNAUTHORIZED, HTTP_UNAUTHORIZED);
        VTN_ERROR_CODES.put(VtnErrorTag.NOTFOUND, HTTP_NOT_FOUND);
        VTN_ERROR_CODES.put(VtnErrorTag.NOTACCEPTABLE, HTTP_NOT_ACCEPTABLE);
        VTN_ERROR_CODES.put(VtnErrorTag.TIMEOUT, HTTP_CLIENT_TIMEOUT);
        VTN_ERROR_CODES.put(VtnErrorTag.CONFLICT, HTTP_CONFLICT);
        VTN_ERROR_CODES.put(VtnErrorTag.GONE, HTTP_GONE);
        VTN_ERROR_CODES.put(VtnErrorTag.NOSERVICE, HTTP_UNAVAILABLE);
        VTN_ERROR_CODES.put(VtnErrorTag.INTERNALERROR, HTTP_INTERNAL_ERROR);
    }

    /**
     * Test case for {@link VTNManagerService#getTenantPath(String)}.
     */
    @Test
    public void testGetTenantPath() {
        String[] tnames = {
            "vtn", "vtn_1", "vtn_2",
        };

        for (String tname: tnames) {
            VtnKey vtnKey = new VtnKey(new VnodeName(tname));
            InstanceIdentifier<Vtn> expath = InstanceIdentifier.
                builder(Vtns.class).
                child(Vtn.class, vtnKey).
                build();
            assertEquals(expath, VTNManagerService.getTenantPath(tname));
        }
    }

    /**
     * Test case for
     * {@link VTNManagerService#getBridgeConfigPath(String, String)}.
     */
    @Test
    public void testGetBridgeConfigPath() {
        String[] tnames = {
            "vtn", "vtn_1", "vtn_2",
        };
        String[] bnames = {
            "vbr", "vbridge_1", "vbridge_2",
        };

        for (String tname: tnames) {
            VtnKey vtnKey = new VtnKey(new VnodeName(tname));
            for (String bname: bnames) {
                VbridgeKey vbrKey = new VbridgeKey(new VnodeName(bname));
                InstanceIdentifier<VbridgeConfig> expath = InstanceIdentifier.
                    builder(Vtns.class).
                    child(Vtn.class, vtnKey).
                    child(Vbridge.class, vbrKey).
                    child(VbridgeConfig.class).
                    build();
                InstanceIdentifier<VbridgeConfig> path =
                    VTNManagerService.getBridgeConfigPath(tname, bname);
                assertEquals(expath, path);
            }
        }
    }

    /**
     * Test case for
     * {@link VTNManagerService#updateTenant(String, VnodeUpdateMode)}.
     */
    @Test
    public void testUpdateTenant() {
        VTNManagerService vtnMgr = getVTNManagerService();
        String[] tnames = {
            "vtn", "vtn_1", "vtn_2",
        };

        // In case of successful completion.
        VtnUpdateType[] utypes = {
            null, VtnUpdateType.CREATED, VtnUpdateType.CHANGED,
        };
        for (String tname: tnames) {
            for (VnodeUpdateMode mode: VnodeUpdateMode.values()) {
                UpdateVtnInput input = new UpdateVtnInputBuilder().
                    setTenantName(tname).
                    setUpdateMode(mode).
                    setOperation(VtnUpdateOperationType.SET).
                    build();

                for (VtnUpdateType utype: utypes) {
                    UpdateVtnOutput output = new UpdateVtnOutputBuilder().
                        setStatus(utype).build();
                    reset(vtnService);
                    when(vtnService.updateVtn(input)).
                        thenReturn(getRpcFuture(output));
                    assertEquals(HTTP_OK, vtnMgr.updateTenant(tname, mode));
                    verify(vtnService).updateVtn(input);
                    verifyNoVtnInteractions();
                }
            }
        }

        String tname = "vtn_fail";
        UpdateVtnInput input = new UpdateVtnInputBuilder().
            setTenantName(tname).
            setOperation(VtnUpdateOperationType.SET).
            build();

        // In case of failure.
        for (VtnErrorTag vtag: VtnErrorTag.values()) {
            int expected = VTN_ERROR_CODES.get(vtag).intValue();
            Future<RpcResult<UpdateVtnOutput>> future =
                getFailureFuture(vtag, "Test failure");
            reset(vtnService);
            when(vtnService.updateVtn(input)).thenReturn(future);
            assertEquals(expected, vtnMgr.updateTenant(tname, null));
            verify(vtnService).updateVtn(input);
            verifyNoVtnInteractions();
        }

        // Future throws an exception.
        IllegalStateException ise = new IllegalStateException("Unexpected");
        Future<RpcResult<UpdateVtnOutput>> future =
            Futures.immediateFailedFuture(ise);
        reset(vtnService);
        when(vtnService.updateVtn(input)).thenReturn(future);
        assertEquals(HTTP_INTERNAL_ERROR, vtnMgr.updateTenant(tname, null));
        verify(vtnService).updateVtn(input);
        verifyNoVtnInteractions();

        // Future returns null.
        RpcResult<UpdateVtnOutput> result = null;
        future = Futures.immediateFuture(result);
        reset(vtnService);
        when(vtnService.updateVtn(input)).thenReturn(future);
        assertEquals(HTTP_INTERNAL_ERROR, vtnMgr.updateTenant(tname, null));
        verify(vtnService).updateVtn(input);
        verifyNoVtnInteractions();

        // RpcResult does not contain output.
        future = getRpcFuture((UpdateVtnOutput)null);
        reset(vtnService);
        when(vtnService.updateVtn(input)).thenReturn(future);
        assertEquals(HTTP_INTERNAL_ERROR, vtnMgr.updateTenant(tname, null));
        verify(vtnService).updateVtn(input);
        verifyNoVtnInteractions();

        // No RpcError in RpcResult.
        result = mock(RpcResult.class);
        when(result.isSuccessful()).thenReturn(false);
        when(result.getErrors()).thenReturn(null);
        future = Futures.immediateFuture(result);
        reset(vtnService);
        when(vtnService.updateVtn(input)).thenReturn(future);
        assertEquals(HTTP_INTERNAL_ERROR, vtnMgr.updateTenant(tname, null));
        verify(vtnService).updateVtn(input);
        verifyNoVtnInteractions();
        verify(result).isSuccessful();
        verify(result).getErrors();
        verifyNoMoreInteractions(result);

        Collection<RpcError> errors = Collections.<RpcError>emptySet();
        result = mock(RpcResult.class);
        when(result.isSuccessful()).thenReturn(false);
        when(result.getErrors()).thenReturn(errors);
        future = Futures.immediateFuture(result);
        reset(vtnService);
        when(vtnService.updateVtn(input)).thenReturn(future);
        assertEquals(HTTP_INTERNAL_ERROR, vtnMgr.updateTenant(tname, null));
        verify(vtnService).updateVtn(input);
        verifyNoVtnInteractions();
        verify(result).isSuccessful();
        verify(result).getErrors();
        verifyNoMoreInteractions(result);

        // Unexpected error information.
        IllegalArgumentException iae = new IllegalArgumentException("Too big");
        result = RpcResultBuilder.<UpdateVtnOutput>failed().
            withError(ErrorType.APPLICATION, "in-use",
                      "Unknown error 1", null, null, null).
            withError(ErrorType.APPLICATION, "too-big",
                      "Unknown error 2", "unknown", null, iae).
            build();
        future = Futures.immediateFuture(result);
        reset(vtnService);
        when(vtnService.updateVtn(input)).thenReturn(future);
        assertEquals(HTTP_INTERNAL_ERROR, vtnMgr.updateTenant(tname, null));
        verify(vtnService).updateVtn(input);
        verifyNoVtnInteractions();
    }

    /**
     * Test case for {@link VTNManagerService#removeTenant(String)}.
     */
    @Test
    public void testRemoveTenant() {
        VTNManagerService vtnMgr = getVTNManagerService();
        String[] tnames = {
            "vtn", "vtn_1", "vtn_2",
        };

        // In case of successful completion.
        for (String tname: tnames) {
            RemoveVtnInput input = new RemoveVtnInputBuilder().
                setTenantName(tname).
                build();
            reset(vtnService);
            when(vtnService.removeVtn(input)).
                thenReturn(getRpcFuture((Void)null));
            assertEquals(HTTP_OK, vtnMgr.removeTenant(tname));
            verify(vtnService).removeVtn(input);
            verifyNoVtnInteractions();
        }

        String tname = "vtn_fail";
        RemoveVtnInput input = new RemoveVtnInputBuilder().
            setTenantName(tname).
            build();

        // In case of failure.
        for (VtnErrorTag vtag: VtnErrorTag.values()) {
            int expected = VTN_ERROR_CODES.get(vtag).intValue();
            Future<RpcResult<Void>> future =
                getFailureFuture(vtag, "Test failure");
            reset(vtnService);
            when(vtnService.removeVtn(input)).thenReturn(future);
            assertEquals(expected, vtnMgr.removeTenant(tname));
            verify(vtnService).removeVtn(input);
            verifyNoVtnInteractions();
        }

        // Future throws an exception.
        IllegalStateException ise = new IllegalStateException("Unexpected");
        Future<RpcResult<Void>> future = Futures.immediateFailedFuture(ise);
        reset(vtnService);
        when(vtnService.removeVtn(input)).thenReturn(future);
        assertEquals(HTTP_INTERNAL_ERROR, vtnMgr.removeTenant(tname));
        verify(vtnService).removeVtn(input);
        verifyNoVtnInteractions();

        // Future returns null.
        RpcResult<Void> result = null;
        future = Futures.immediateFuture(result);
        reset(vtnService);
        when(vtnService.removeVtn(input)).thenReturn(future);
        assertEquals(HTTP_INTERNAL_ERROR, vtnMgr.removeTenant(tname));
        verify(vtnService).removeVtn(input);
        verifyNoVtnInteractions();

        // No RpcError in RpcResult.
        result = mock(RpcResult.class);
        when(result.isSuccessful()).thenReturn(false);
        when(result.getErrors()).thenReturn(null);
        future = Futures.immediateFuture(result);
        reset(vtnService);
        when(vtnService.removeVtn(input)).thenReturn(future);
        assertEquals(HTTP_INTERNAL_ERROR, vtnMgr.removeTenant(tname));
        verify(vtnService).removeVtn(input);
        verifyNoVtnInteractions();
        verify(result).isSuccessful();
        verify(result).getErrors();
        verifyNoMoreInteractions(result);

        Collection<RpcError> errors = Collections.<RpcError>emptySet();
        result = mock(RpcResult.class);
        when(result.isSuccessful()).thenReturn(false);
        when(result.getErrors()).thenReturn(errors);
        future = Futures.immediateFuture(result);
        reset(vtnService);
        when(vtnService.removeVtn(input)).thenReturn(future);
        assertEquals(HTTP_INTERNAL_ERROR, vtnMgr.removeTenant(tname));
        verify(vtnService).removeVtn(input);
        verifyNoVtnInteractions();
        verify(result).isSuccessful();
        verify(result).getErrors();
        verifyNoMoreInteractions(result);

        // Unexpected error information.
        IllegalArgumentException iae = new IllegalArgumentException("Too big");
        result = RpcResultBuilder.<Void>failed().
            withError(ErrorType.APPLICATION, "in-use",
                      "Unknown error 1", null, null, null).
            withError(ErrorType.APPLICATION, "too-big",
                      "Unknown error 2", "unknown", null, iae).
            build();
        future = Futures.immediateFuture(result);
        reset(vtnService);
        when(vtnService.removeVtn(input)).thenReturn(future);
        assertEquals(HTTP_INTERNAL_ERROR, vtnMgr.removeTenant(tname));
        verify(vtnService).removeVtn(input);
        verifyNoVtnInteractions();
    }

    /**
     * Test case for {@link VTNManagerService#hasBridge(String)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testContainsBridge() throws Exception {
        VTNManagerService vtnMgr = getVTNManagerService();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        String[] tnames = {
            "vtn", "vtn_1", "vtn_2",
        };

        for (String tname: tnames) {
            VtnKey vtnKey = new VtnKey(new VnodeName(tname));
            InstanceIdentifier<Vtn> path = InstanceIdentifier.
                builder(Vtns.class).
                child(Vtn.class, vtnKey).
                build();

            // In case where the specified VTN is not present.
            int txConut = 0;
            Vtn vtn = null;
            ReadOnlyTransaction rtx = mock(ReadOnlyTransaction.class);
            reset(dataBroker);
            when(dataBroker.newReadOnlyTransaction()).thenReturn(rtx);
            when(rtx.read(oper, path)).thenReturn(getReadResult(vtn));
            assertEquals(false, vtnMgr.hasBridge(tname));
            verify(dataBroker, times(++txConut)).newReadOnlyTransaction();
            verify(rtx).read(oper, path);
            verify(rtx).close();
            verifyNoMoreInteractions(dataBroker, rtx);
            verifyNoVtnInteractions();

            // In case where the specified VTN contains null vBridge list.
            VnodeName vtnName = new VnodeName(tname);
            vtn = new VtnBuilder().setName(vtnName).build();
            reset(rtx);
            when(rtx.read(oper, path)).thenReturn(getReadResult(vtn));
            assertEquals(false, vtnMgr.hasBridge(tname));
            verify(dataBroker, times(++txConut)).newReadOnlyTransaction();
            verify(rtx).read(oper, path);
            verify(rtx).close();
            verifyNoMoreInteractions(dataBroker, rtx);
            verifyNoVtnInteractions();

            // In case where the specified VTN contains an empty vBridge list.
            List<Vbridge> vbridges = Collections.<Vbridge>emptyList();
            vtn = new VtnBuilder().
                setName(vtnName).
                setVbridge(vbridges).
                build();
            reset(rtx);
            when(rtx.read(oper, path)).thenReturn(getReadResult(vtn));
            assertEquals(false, vtnMgr.hasBridge(tname));
            verify(dataBroker, times(++txConut)).newReadOnlyTransaction();
            verify(rtx).read(oper, path);
            verify(rtx).close();
            verifyNoMoreInteractions(dataBroker, rtx);
            verifyNoVtnInteractions();

            // In case of unexpected error.
            reset(rtx);
            IllegalStateException ise =
                new IllegalStateException("Read failed");
            when(rtx.read(oper, path)).
                thenReturn(getReadFailure(Vtn.class, ise));
            assertEquals(false, vtnMgr.hasBridge(tname));
            verify(dataBroker, times(++txConut)).newReadOnlyTransaction();
            verify(rtx).read(oper, path);
            verify(rtx).close();
            verifyNoMoreInteractions(dataBroker, rtx);
            verifyNoVtnInteractions();

            // In case of read timeout.
            CheckedFuture<Optional<Vtn>, ReadFailedException> future =
                getReadTimeoutFailure(Vtn.class);
            reset(rtx);
            when(rtx.read(oper, path)).thenReturn(future);
            assertEquals(false, vtnMgr.hasBridge(tname));
            verify(dataBroker, times(++txConut)).newReadOnlyTransaction();
            verify(rtx).read(oper, path);
            verify(rtx).close();
            verifyNoMoreInteractions(dataBroker, rtx);
            verifyNoVtnInteractions();

            // In case where the specified VTN contains one or more vBridges.
            for (int count = 1; count <= 5; count++) {
                vbridges = new ArrayList<>();
                for (int i = 0; i < count; i++) {
                    vbridges.add(new VbridgeBuilder().build());
                }
                vtn = new VtnBuilder().
                    setName(vtnName).
                    setVbridge(vbridges).
                    build();
                reset(rtx);
                when(rtx.read(oper, path)).thenReturn(getReadResult(vtn));
                assertEquals(true, vtnMgr.hasBridge(tname));
                verify(dataBroker, times(++txConut)).newReadOnlyTransaction();
                verify(rtx).read(oper, path);
                verify(rtx).close();
                verifyNoMoreInteractions(dataBroker, rtx);
                verifyNoVtnInteractions();
            }
        }
    }

    /**
     * Test case for
     * {@link VTNManagerService#updateBridge(String, String, String, VnodeUpdateMode)}.
     */
    @Test
    public void testUpdateBridge() {
        VTNManagerService vtnMgr = getVTNManagerService();
        String[] tnames = {
            "vtn", "vtn_1", "vtn_2",
        };
        String[] bnames = {
            "vbr", "bridge_1", "bridge_2",
        };
        String[] descriptions = {
            null, "desc",
        };

        // In case of successful completion.
        List<UpdateVbridgeInput> inputs = new ArrayList<>();
        for (String tname: tnames) {
            for (String bname: bnames) {
                for (String desc: descriptions) {
                    for (VnodeUpdateMode mode: VnodeUpdateMode.values()) {
                        UpdateVbridgeInput input =
                            new UpdateVbridgeInputBuilder().
                            setTenantName(tname).
                            setBridgeName(bname).
                            setDescription(desc).
                            setUpdateMode(mode).
                            setOperation(VtnUpdateOperationType.SET).
                            build();
                        inputs.add(input);
                    }
                }
            }
        }

        VtnUpdateType[] utypes = {
            null, VtnUpdateType.CREATED, VtnUpdateType.CHANGED,
        };
        for (UpdateVbridgeInput input: inputs) {
            for (VtnUpdateType utype: utypes) {
                UpdateVbridgeOutput output = new UpdateVbridgeOutputBuilder().
                    setStatus(utype).build();
                reset(vbridgeService);
                when(vbridgeService.updateVbridge(input)).
                    thenReturn(getRpcFuture(output));

                String tname = input.getTenantName();
                String bname = input.getBridgeName();
                String desc = input.getDescription();
                VnodeUpdateMode mode = input.getUpdateMode();
                assertEquals(HTTP_OK,
                             vtnMgr.updateBridge(tname, bname, desc, mode));
                verify(vbridgeService).updateVbridge(input);
                verifyNoVtnInteractions();
            }
        }

        String tname = "vtn_fail";
        String bname = "vbr_fail";
        UpdateVbridgeInput input = new UpdateVbridgeInputBuilder().
            setTenantName(tname).
            setBridgeName(bname).
            setOperation(VtnUpdateOperationType.SET).
            build();

        // In case of failure.
        for (VtnErrorTag vtag: VtnErrorTag.values()) {
            int expected = VTN_ERROR_CODES.get(vtag).intValue();
            Future<RpcResult<UpdateVbridgeOutput>> future =
                getFailureFuture(vtag, "Test failure");
            reset(vbridgeService);
            when(vbridgeService.updateVbridge(input)).thenReturn(future);
            assertEquals(expected,
                         vtnMgr.updateBridge(tname, bname, null, null));
            verify(vbridgeService).updateVbridge(input);
            verifyNoVtnInteractions();
        }

        // Future throws an exception.
        IllegalStateException ise = new IllegalStateException("Unexpected");
        Future<RpcResult<UpdateVbridgeOutput>> future =
            Futures.immediateFailedFuture(ise);
        reset(vbridgeService);
        when(vbridgeService.updateVbridge(input)).thenReturn(future);
        assertEquals(HTTP_INTERNAL_ERROR,
                     vtnMgr.updateBridge(tname, bname, null, null));
        verify(vbridgeService).updateVbridge(input);
        verifyNoVtnInteractions();

        // Future returns null.
        RpcResult<UpdateVbridgeOutput> result = null;
        future = Futures.immediateFuture(result);
        reset(vbridgeService);
        when(vbridgeService.updateVbridge(input)).thenReturn(future);
        assertEquals(HTTP_INTERNAL_ERROR,
                     vtnMgr.updateBridge(tname, bname, null, null));
        verify(vbridgeService).updateVbridge(input);
        verifyNoVtnInteractions();

        // RpcResult does not contain output.
        future = getRpcFuture((UpdateVbridgeOutput)null);
        reset(vbridgeService);
        when(vbridgeService.updateVbridge(input)).thenReturn(future);
        assertEquals(HTTP_INTERNAL_ERROR,
                     vtnMgr.updateBridge(tname, bname, null, null));
        verify(vbridgeService).updateVbridge(input);
        verifyNoVtnInteractions();

        // No RpcError in RpcResult.
        result = mock(RpcResult.class);
        when(result.isSuccessful()).thenReturn(false);
        when(result.getErrors()).thenReturn(null);
        future = Futures.immediateFuture(result);
        reset(vbridgeService);
        when(vbridgeService.updateVbridge(input)).thenReturn(future);
        assertEquals(HTTP_INTERNAL_ERROR,
                     vtnMgr.updateBridge(tname, bname, null, null));
        verify(vbridgeService).updateVbridge(input);
        verifyNoVtnInteractions();
        verify(result).isSuccessful();
        verify(result).getErrors();
        verifyNoMoreInteractions(result);

        Collection<RpcError> errors = Collections.<RpcError>emptySet();
        result = mock(RpcResult.class);
        when(result.isSuccessful()).thenReturn(false);
        when(result.getErrors()).thenReturn(errors);
        future = Futures.immediateFuture(result);
        reset(vbridgeService);
        when(vbridgeService.updateVbridge(input)).thenReturn(future);
        assertEquals(HTTP_INTERNAL_ERROR,
                     vtnMgr.updateBridge(tname, bname, null, null));
        verify(vbridgeService).updateVbridge(input);
        verifyNoVtnInteractions();
        verify(result).isSuccessful();
        verify(result).getErrors();
        verifyNoMoreInteractions(result);

        // Unexpected error information.
        IllegalArgumentException iae = new IllegalArgumentException("Too big");
        result = RpcResultBuilder.<UpdateVbridgeOutput>failed().
            withError(ErrorType.APPLICATION, "in-use",
                      "Unknown error 1", null, null, null).
            withError(ErrorType.APPLICATION, "too-big",
                      "Unknown error 2", "unknown", null, iae).
            build();
        future = Futures.immediateFuture(result);
        reset(vbridgeService);
        when(vbridgeService.updateVbridge(input)).thenReturn(future);
        assertEquals(HTTP_INTERNAL_ERROR,
                     vtnMgr.updateBridge(tname, bname, null, null));
        verify(vbridgeService).updateVbridge(input);
        verifyNoVtnInteractions();
    }

    /**
     * Test case for {@link VTNManagerService#removeBridge(String, String)}.
     */
    @Test
    public void testRemoveBridge() {
        VTNManagerService vtnMgr = getVTNManagerService();
        String[] tnames = {
            "vtn", "vtn_1", "vtn_2",
        };
        String[] bnames = {
            "vbr", "bridge_1", "bridge_2",
        };

        // In case of successful completion.
        for (String tname: tnames) {
            for (String bname: bnames) {
                RemoveVbridgeInput input = new RemoveVbridgeInputBuilder().
                    setTenantName(tname).
                    setBridgeName(bname).
                    build();
                reset(vbridgeService);
                when(vbridgeService.removeVbridge(input)).
                    thenReturn(getRpcFuture((Void)null));
                assertEquals(HTTP_OK, vtnMgr.removeBridge(tname, bname));
                verify(vbridgeService).removeVbridge(input);
                verifyNoVtnInteractions();
            }
        }

        String tname = "vtn_fail";
        String bname = "vbr_fail";
        RemoveVbridgeInput input = new RemoveVbridgeInputBuilder().
            setTenantName(tname).
            setBridgeName(bname).
            build();

        // In case of failure.
        for (VtnErrorTag vtag: VtnErrorTag.values()) {
            int expected = VTN_ERROR_CODES.get(vtag).intValue();
            Future<RpcResult<Void>> future =
                getFailureFuture(vtag, "Test failure");
            reset(vbridgeService);
            when(vbridgeService.removeVbridge(input)).thenReturn(future);
            assertEquals(expected, vtnMgr.removeBridge(tname, bname));
            verify(vbridgeService).removeVbridge(input);
            verifyNoVtnInteractions();
        }

        // Future throws an exception.
        IllegalStateException ise = new IllegalStateException("Unexpected");
        Future<RpcResult<Void>> future = Futures.immediateFailedFuture(ise);
        reset(vbridgeService);
        when(vbridgeService.removeVbridge(input)).thenReturn(future);
        assertEquals(HTTP_INTERNAL_ERROR, vtnMgr.removeBridge(tname, bname));
        verify(vbridgeService).removeVbridge(input);
        verifyNoVtnInteractions();

        // Future returns null.
        RpcResult<Void> result = null;
        future = Futures.immediateFuture(result);
        reset(vbridgeService);
        when(vbridgeService.removeVbridge(input)).thenReturn(future);
        assertEquals(HTTP_INTERNAL_ERROR, vtnMgr.removeBridge(tname, bname));
        verify(vbridgeService).removeVbridge(input);
        verifyNoVtnInteractions();

        // No RpcError in RpcResult.
        result = mock(RpcResult.class);
        when(result.isSuccessful()).thenReturn(false);
        when(result.getErrors()).thenReturn(null);
        future = Futures.immediateFuture(result);
        reset(vbridgeService);
        when(vbridgeService.removeVbridge(input)).thenReturn(future);
        assertEquals(HTTP_INTERNAL_ERROR, vtnMgr.removeBridge(tname, bname));
        verify(vbridgeService).removeVbridge(input);
        verifyNoVtnInteractions();
        verify(result).isSuccessful();
        verify(result).getErrors();
        verifyNoMoreInteractions(result);

        Collection<RpcError> errors = Collections.<RpcError>emptySet();
        result = mock(RpcResult.class);
        when(result.isSuccessful()).thenReturn(false);
        when(result.getErrors()).thenReturn(errors);
        future = Futures.immediateFuture(result);
        reset(vbridgeService);
        when(vbridgeService.removeVbridge(input)).thenReturn(future);
        assertEquals(HTTP_INTERNAL_ERROR, vtnMgr.removeBridge(tname, bname));
        verify(vbridgeService).removeVbridge(input);
        verifyNoVtnInteractions();
        verify(result).isSuccessful();
        verify(result).getErrors();
        verifyNoMoreInteractions(result);

        // Unexpected error information.
        IllegalArgumentException iae = new IllegalArgumentException("Too big");
        result = RpcResultBuilder.<Void>failed().
            withError(ErrorType.APPLICATION, "in-use",
                      "Unknown error 1", null, null, null).
            withError(ErrorType.APPLICATION, "too-big",
                      "Unknown error 2", "unknown", null, iae).
            build();
        future = Futures.immediateFuture(result);
        reset(vbridgeService);
        when(vbridgeService.removeVbridge(input)).thenReturn(future);
        assertEquals(HTTP_INTERNAL_ERROR, vtnMgr.removeBridge(tname, bname));
        verify(vbridgeService).removeVbridge(input);
        verifyNoVtnInteractions();
    }

    /**
     * Test case for {@link VTNManagerService#getBridgeConfig(String, String)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void getGetBridgeConfig() throws Exception {
        VTNManagerService vtnMgr = getVTNManagerService();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        String[] tnames = {
            "vtn", "vtn_1", "vtn_2",
        };
        String[] bnames = {
            "vbr", "bridge_1", "bridge_2",
        };

        for (String tname: tnames) {
            VtnKey vtnKey = new VtnKey(new VnodeName(tname));
            for (String bname: bnames) {
                VbridgeKey vbrKey = new VbridgeKey(new VnodeName(bname));
                InstanceIdentifier<VbridgeConfig> path = InstanceIdentifier.
                    builder(Vtns.class).
                    child(Vtn.class, vtnKey).
                    child(Vbridge.class, vbrKey).
                    child(VbridgeConfig.class).
                    build();

                // In case where the specified vBridge is not present.
                int txConut = 0;
                VbridgeConfig config = null;
                ReadOnlyTransaction rtx = mock(ReadOnlyTransaction.class);
                reset(dataBroker);
                when(dataBroker.newReadOnlyTransaction()).thenReturn(rtx);
                when(rtx.read(oper, path)).thenReturn(getReadResult(config));
                assertEquals(null, vtnMgr.getBridgeConfig(tname, bname));
                verify(dataBroker, times(++txConut)).newReadOnlyTransaction();
                verify(rtx).read(oper, path);
                verify(rtx).close();
                verifyNoMoreInteractions(dataBroker, rtx);
                verifyNoVtnInteractions();

                // In case where the specified vBridge is present.
                config = new VbridgeConfigBuilder().build();
                reset(rtx);
                when(rtx.read(oper, path)).thenReturn(getReadResult(config));
                assertEquals(config, vtnMgr.getBridgeConfig(tname, bname));
                verify(dataBroker, times(++txConut)).newReadOnlyTransaction();
                verify(rtx).read(oper, path);
                verify(rtx).close();
                verifyNoMoreInteractions(dataBroker, rtx);
                verifyNoVtnInteractions();

                // In case of unexpected error.
                reset(rtx);
                IllegalStateException ise =
                    new IllegalStateException("Read failed");
                when(rtx.read(oper, path)).
                    thenReturn(getReadFailure(VbridgeConfig.class, ise));
                assertEquals(null, vtnMgr.getBridgeConfig(tname, bname));
                verify(dataBroker, times(++txConut)).newReadOnlyTransaction();
                verify(rtx).read(oper, path);
                verify(rtx).close();
                verifyNoMoreInteractions(dataBroker, rtx);
                verifyNoVtnInteractions();

                // In case of read timeout.
                CheckedFuture<Optional<VbridgeConfig>, ReadFailedException> future =
                    getReadTimeoutFailure(VbridgeConfig.class);
                reset(rtx);
                when(rtx.read(oper, path)).thenReturn(future);
                assertEquals(null, vtnMgr.getBridgeConfig(tname, bname));
                verify(dataBroker, times(++txConut)).newReadOnlyTransaction();
                verify(rtx).read(oper, path);
                verify(rtx).close();
                verifyNoMoreInteractions(dataBroker, rtx);
                verifyNoVtnInteractions();
            }
        }
    }

    /**
     * Test case for
     * {@link VTNManagerService#updateInterface(UpdateVinterfaceInput)}.
     */
    @Test
    public void testUpdateInterface() {
        VTNManagerService vtnMgr = getVTNManagerService();
        String[] tnames = {
            "vtn", "vtn_1",
        };
        String[] bnames = {
            "vbr", "bridge_1",
        };
        String[] inames = {
            "vif", "if_1",
        };

        // In case of successful completion.
        List<UpdateVinterfaceInput> inputs = new ArrayList<>();
        for (String tname: tnames) {
            for (String bname: bnames) {
                for (String iname: inames) {
                    for (VnodeUpdateMode mode: VnodeUpdateMode.values()) {
                        UpdateVinterfaceInput input =
                            new UpdateVinterfaceInputBuilder().
                            setTenantName(tname).
                            setBridgeName(bname).
                            setInterfaceName(iname).
                            setUpdateMode(mode).
                            build();
                        inputs.add(input);

                        input = new UpdateVinterfaceInputBuilder().
                            setTenantName(tname).
                            setBridgeName(bname).
                            setInterfaceName(iname).
                            setUpdateMode(mode).
                            setEnabled(true).
                            build();
                        inputs.add(input);

                        input = new UpdateVinterfaceInputBuilder().
                            setTenantName(tname).
                            setBridgeName(bname).
                            setInterfaceName(iname).
                            setUpdateMode(mode).
                            setEnabled(false).
                            setDescription("virtual interface").
                            build();
                        inputs.add(input);
                    }
                }
            }
        }

        VtnUpdateType[] utypes = {
            null, VtnUpdateType.CREATED, VtnUpdateType.CHANGED,
        };
        for (UpdateVinterfaceInput input: inputs) {
            for (VtnUpdateType utype: utypes) {
                UpdateVinterfaceOutput output =
                    new UpdateVinterfaceOutputBuilder().
                    setStatus(utype).build();
                reset(vinterfaceService);
                when(vinterfaceService.updateVinterface(input)).
                    thenReturn(getRpcFuture(output));
                assertEquals(HTTP_OK, vtnMgr.updateInterface(input));
                verify(vinterfaceService).updateVinterface(input);
                verifyNoVtnInteractions();
            }
        }

        String tname = "vtn_fail";
        String bname = "vbr_fail";
        String iname = "vif_fail";
        UpdateVinterfaceInput input = new UpdateVinterfaceInputBuilder().
            setTenantName(tname).
            setBridgeName(bname).
            setInterfaceName(iname).
            build();

        // In case of failure.
        for (VtnErrorTag vtag: VtnErrorTag.values()) {
            int expected = VTN_ERROR_CODES.get(vtag).intValue();
            Future<RpcResult<UpdateVinterfaceOutput>> future =
                getFailureFuture(vtag, "Test failure");
            reset(vinterfaceService);
            when(vinterfaceService.updateVinterface(input)).thenReturn(future);
            assertEquals(expected, vtnMgr.updateInterface(input));
            verify(vinterfaceService).updateVinterface(input);
            verifyNoVtnInteractions();
        }

        // Future throws an exception.
        IllegalStateException ise = new IllegalStateException("Unexpected");
        Future<RpcResult<UpdateVinterfaceOutput>> future =
            Futures.immediateFailedFuture(ise);
        reset(vinterfaceService);
        when(vinterfaceService.updateVinterface(input)).thenReturn(future);
        assertEquals(HTTP_INTERNAL_ERROR, vtnMgr.updateInterface(input));
        verify(vinterfaceService).updateVinterface(input);
        verifyNoVtnInteractions();

        // Future returns null.
        RpcResult<UpdateVinterfaceOutput> result = null;
        future = Futures.immediateFuture(result);
        reset(vinterfaceService);
        when(vinterfaceService.updateVinterface(input)).thenReturn(future);
        assertEquals(HTTP_INTERNAL_ERROR, vtnMgr.updateInterface(input));
        verify(vinterfaceService).updateVinterface(input);
        verifyNoVtnInteractions();

        // RpcResult does not contain output.
        future = getRpcFuture((UpdateVinterfaceOutput)null);
        reset(vinterfaceService);
        when(vinterfaceService.updateVinterface(input)).thenReturn(future);
        assertEquals(HTTP_INTERNAL_ERROR, vtnMgr.updateInterface(input));
        verify(vinterfaceService).updateVinterface(input);
        verifyNoVtnInteractions();

        // No RpcError in RpcResult.
        result = mock(RpcResult.class);
        when(result.isSuccessful()).thenReturn(false);
        when(result.getErrors()).thenReturn(null);
        future = Futures.immediateFuture(result);
        reset(vinterfaceService);
        when(vinterfaceService.updateVinterface(input)).thenReturn(future);
        assertEquals(HTTP_INTERNAL_ERROR, vtnMgr.updateInterface(input));
        verify(vinterfaceService).updateVinterface(input);
        verifyNoVtnInteractions();
        verify(result).isSuccessful();
        verify(result).getErrors();
        verifyNoMoreInteractions(result);

        Collection<RpcError> errors = Collections.<RpcError>emptySet();
        result = mock(RpcResult.class);
        when(result.isSuccessful()).thenReturn(false);
        when(result.getErrors()).thenReturn(errors);
        future = Futures.immediateFuture(result);
        reset(vinterfaceService);
        when(vinterfaceService.updateVinterface(input)).thenReturn(future);
        assertEquals(HTTP_INTERNAL_ERROR, vtnMgr.updateInterface(input));
        verify(vinterfaceService).updateVinterface(input);
        verifyNoVtnInteractions();
        verify(result).isSuccessful();
        verify(result).getErrors();
        verifyNoMoreInteractions(result);

        // Unexpected error information.
        IllegalArgumentException iae = new IllegalArgumentException("Too big");
        result = RpcResultBuilder.<UpdateVinterfaceOutput>failed().
            withError(ErrorType.APPLICATION, "in-use",
                      "Unknown error 1", null, null, null).
            withError(ErrorType.APPLICATION, "too-big",
                      "Unknown error 2", "unknown", null, iae).
            build();
        future = Futures.immediateFuture(result);
        reset(vinterfaceService);
        when(vinterfaceService.updateVinterface(input)).thenReturn(future);
        assertEquals(HTTP_INTERNAL_ERROR, vtnMgr.updateInterface(input));
        verify(vinterfaceService).updateVinterface(input);
        verifyNoVtnInteractions();
    }

    /**
     * Test case for
     * {@link VTNManagerService#removeInterface(RemoveVinterfaceInput)}.
     */
    @Test
    public void testRemoveInterface() {
        VTNManagerService vtnMgr = getVTNManagerService();
        String[] tnames = {
            "vtn", "vtn_1",
        };
        String[] bnames = {
            "vbr", "bridge_1",
        };
        String[] inames = {
            "vif", "if_1",
        };

        // In case of successful completion.
        for (String tname: tnames) {
            for (String bname: bnames) {
                for (String iname: inames) {
                    RemoveVinterfaceInput input =
                        new RemoveVinterfaceInputBuilder().
                        setTenantName(tname).
                        setBridgeName(bname).
                        setInterfaceName(iname).
                        build();
                    reset(vinterfaceService);
                    when(vinterfaceService.removeVinterface(input)).
                        thenReturn(getRpcFuture((Void)null));
                    assertEquals(HTTP_OK, vtnMgr.removeInterface(input));
                    verify(vinterfaceService).removeVinterface(input);
                    verifyNoVtnInteractions();
                }
            }
        }

        String tname = "vtn_fail";
        String bname = "vbr_fail";
        String iname = "vif_fail";
        RemoveVinterfaceInput input = new RemoveVinterfaceInputBuilder().
            setTenantName(tname).
            setBridgeName(bname).
            setInterfaceName(iname).
            build();

        // In case of failure.
        for (VtnErrorTag vtag: VtnErrorTag.values()) {
            int expected = VTN_ERROR_CODES.get(vtag).intValue();
            Future<RpcResult<Void>> future =
                getFailureFuture(vtag, "Test failure");
            reset(vinterfaceService);
            when(vinterfaceService.removeVinterface(input)).thenReturn(future);
            assertEquals(expected, vtnMgr.removeInterface(input));
            verify(vinterfaceService).removeVinterface(input);
            verifyNoVtnInteractions();
        }

        // Future throws an exception.
        IllegalStateException ise = new IllegalStateException("Unexpected");
        Future<RpcResult<Void>> future = Futures.immediateFailedFuture(ise);
        reset(vinterfaceService);
        when(vinterfaceService.removeVinterface(input)).thenReturn(future);
        assertEquals(HTTP_INTERNAL_ERROR, vtnMgr.removeInterface(input));
        verify(vinterfaceService).removeVinterface(input);
        verifyNoVtnInteractions();

        // Future returns null.
        RpcResult<Void> result = null;
        future = Futures.immediateFuture(result);
        reset(vinterfaceService);
        when(vinterfaceService.removeVinterface(input)).thenReturn(future);
        assertEquals(HTTP_INTERNAL_ERROR, vtnMgr.removeInterface(input));
        verify(vinterfaceService).removeVinterface(input);
        verifyNoVtnInteractions();

        // No RpcError in RpcResult.
        result = mock(RpcResult.class);
        when(result.isSuccessful()).thenReturn(false);
        when(result.getErrors()).thenReturn(null);
        future = Futures.immediateFuture(result);
        reset(vinterfaceService);
        when(vinterfaceService.removeVinterface(input)).thenReturn(future);
        assertEquals(HTTP_INTERNAL_ERROR, vtnMgr.removeInterface(input));
        verify(vinterfaceService).removeVinterface(input);
        verifyNoVtnInteractions();
        verify(result).isSuccessful();
        verify(result).getErrors();
        verifyNoMoreInteractions(result);

        Collection<RpcError> errors = Collections.<RpcError>emptySet();
        result = mock(RpcResult.class);
        when(result.isSuccessful()).thenReturn(false);
        when(result.getErrors()).thenReturn(errors);
        future = Futures.immediateFuture(result);
        reset(vinterfaceService);
        when(vinterfaceService.removeVinterface(input)).thenReturn(future);
        assertEquals(HTTP_INTERNAL_ERROR, vtnMgr.removeInterface(input));
        verify(vinterfaceService).removeVinterface(input);
        verifyNoVtnInteractions();
        verify(result).isSuccessful();
        verify(result).getErrors();
        verifyNoMoreInteractions(result);

        // Unexpected error information.
        IllegalArgumentException iae = new IllegalArgumentException("Too big");
        result = RpcResultBuilder.<Void>failed().
            withError(ErrorType.APPLICATION, "in-use",
                      "Unknown error 1", null, null, null).
            withError(ErrorType.APPLICATION, "too-big",
                      "Unknown error 2", "unknown", null, iae).
            build();
        future = Futures.immediateFuture(result);
        reset(vinterfaceService);
        when(vinterfaceService.removeVinterface(input)).thenReturn(future);
        assertEquals(HTTP_INTERNAL_ERROR, vtnMgr.removeInterface(input));
        verify(vinterfaceService).removeVinterface(input);
        verifyNoVtnInteractions();
    }

    /**
     * Test case for {@link VTNManagerService#setPortMap(SetPortMapInput)}.
     */
    @Test
    public void testSetPortMap() {
        VTNManagerService vtnMgr = getVTNManagerService();
        String[] tnames = {
            "vtn", "vtn_1",
        };
        String[] bnames = {
            "vbr", "bridge_1",
        };
        String[] inames = {
            "vif", "if_1",
        };

        // In case of successful completion.
        List<SetPortMapInput> inputs = new ArrayList<>();
        for (String tname: tnames) {
            for (String bname: bnames) {
                for (String iname: inames) {
                    for (VnodeUpdateMode mode: VnodeUpdateMode.values()) {
                        SetPortMapInput input =
                            new SetPortMapInputBuilder().
                            setTenantName(tname).
                            setBridgeName(bname).
                            setInterfaceName(iname).
                            setNode(new NodeId("openflow:1")).
                            setPortId("10").
                            build();
                        inputs.add(input);

                        input = new SetPortMapInputBuilder().
                            setTenantName(tname).
                            setBridgeName(bname).
                            setInterfaceName(iname).
                            setNode(new NodeId("openflow:2")).
                            setPortName("port-1").
                            build();
                        inputs.add(input);

                        input = new SetPortMapInputBuilder().
                            setTenantName(tname).
                            setBridgeName(bname).
                            setInterfaceName(iname).
                            setNode(new NodeId("openflow:12345")).
                            setPortName("port-3").
                            setPortId("3").
                            build();
                        inputs.add(input);
                    }
                }
            }
        }

        VtnUpdateType[] utypes = {
            null, VtnUpdateType.CREATED, VtnUpdateType.CHANGED,
        };
        for (SetPortMapInput input: inputs) {
            for (VtnUpdateType utype: utypes) {
                SetPortMapOutput output = new SetPortMapOutputBuilder().
                    setStatus(utype).build();
                reset(portMapService);
                when(portMapService.setPortMap(input)).
                    thenReturn(getRpcFuture(output));
                assertEquals(HTTP_OK, vtnMgr.setPortMap(input));
                verify(portMapService).setPortMap(input);
                verifyNoVtnInteractions();
            }
        }

        String tname = "vtn_fail";
        String bname = "vbr_fail";
        String iname = "vif_fail";
        SetPortMapInput input = new SetPortMapInputBuilder().
            setTenantName(tname).
            setBridgeName(bname).
            setInterfaceName(iname).
            setNode(new NodeId("openflow:1")).
            setPortId("2").
            build();

        // In case of failure.
        for (VtnErrorTag vtag: VtnErrorTag.values()) {
            int expected = VTN_ERROR_CODES.get(vtag).intValue();
            Future<RpcResult<SetPortMapOutput>> future =
                getFailureFuture(vtag, "Test failure");
            reset(portMapService);
            when(portMapService.setPortMap(input)).thenReturn(future);
            assertEquals(expected, vtnMgr.setPortMap(input));
            verify(portMapService).setPortMap(input);
            verifyNoVtnInteractions();
        }

        // Future throws an exception.
        IllegalStateException ise = new IllegalStateException("Unexpected");
        Future<RpcResult<SetPortMapOutput>> future =
            Futures.immediateFailedFuture(ise);
        reset(portMapService);
        when(portMapService.setPortMap(input)).thenReturn(future);
        assertEquals(HTTP_INTERNAL_ERROR, vtnMgr.setPortMap(input));
        verify(portMapService).setPortMap(input);
        verifyNoVtnInteractions();

        // Future returns null.
        RpcResult<SetPortMapOutput> result = null;
        future = Futures.immediateFuture(result);
        reset(portMapService);
        when(portMapService.setPortMap(input)).thenReturn(future);
        assertEquals(HTTP_INTERNAL_ERROR, vtnMgr.setPortMap(input));
        verify(portMapService).setPortMap(input);
        verifyNoVtnInteractions();

        // RpcResult does not contain output.
        future = getRpcFuture((SetPortMapOutput)null);
        reset(portMapService);
        when(portMapService.setPortMap(input)).thenReturn(future);
        assertEquals(HTTP_INTERNAL_ERROR, vtnMgr.setPortMap(input));
        verify(portMapService).setPortMap(input);
        verifyNoVtnInteractions();

        // No RpcError in RpcResult.
        result = mock(RpcResult.class);
        when(result.isSuccessful()).thenReturn(false);
        when(result.getErrors()).thenReturn(null);
        future = Futures.immediateFuture(result);
        reset(portMapService);
        when(portMapService.setPortMap(input)).thenReturn(future);
        assertEquals(HTTP_INTERNAL_ERROR, vtnMgr.setPortMap(input));
        verify(portMapService).setPortMap(input);
        verifyNoVtnInteractions();
        verify(result).isSuccessful();
        verify(result).getErrors();
        verifyNoMoreInteractions(result);

        Collection<RpcError> errors = Collections.<RpcError>emptySet();
        result = mock(RpcResult.class);
        when(result.isSuccessful()).thenReturn(false);
        when(result.getErrors()).thenReturn(errors);
        future = Futures.immediateFuture(result);
        reset(portMapService);
        when(portMapService.setPortMap(input)).thenReturn(future);
        assertEquals(HTTP_INTERNAL_ERROR, vtnMgr.setPortMap(input));
        verify(portMapService).setPortMap(input);
        verifyNoVtnInteractions();
        verify(result).isSuccessful();
        verify(result).getErrors();
        verifyNoMoreInteractions(result);

        // Unexpected error information.
        IllegalArgumentException iae = new IllegalArgumentException("Too big");
        result = RpcResultBuilder.<SetPortMapOutput>failed().
            withError(ErrorType.APPLICATION, "too-big",
                      "Unknown error 1", "unknown-error", null, iae).
            withError(ErrorType.APPLICATION, "in-use",
                      "Unknown error 2", null, null, null).
            build();
        future = Futures.immediateFuture(result);
        reset(portMapService);
        when(portMapService.setPortMap(input)).thenReturn(future);
        assertEquals(HTTP_INTERNAL_ERROR, vtnMgr.setPortMap(input));
        verify(portMapService).setPortMap(input);
        verifyNoVtnInteractions();
    }

    /**
     * Test case for
     * {@link VTNManagerService#removePortMap(RemovePortMapInput)}.
     */
    @Test
    public void testRemovePortMap() {
        VTNManagerService vtnMgr = getVTNManagerService();
        String[] tnames = {
            "vtn", "vtn_1",
        };
        String[] bnames = {
            "vbr", "bridge_1",
        };
        String[] inames = {
            "vif", "if_1",
        };

        // In case of successful completion.
        VtnUpdateType[] utypes = {
            null, VtnUpdateType.REMOVED,
        };
        for (String tname: tnames) {
            for (String bname: bnames) {
                for (String iname: inames) {
                    RemovePortMapInput input = new RemovePortMapInputBuilder().
                        setTenantName(tname).
                        setBridgeName(bname).
                        setInterfaceName(iname).
                        build();
                    for (VtnUpdateType utype: utypes) {
                        RemovePortMapOutput output =
                            new RemovePortMapOutputBuilder().
                            setStatus(utype).build();
                        reset(portMapService);
                        when(portMapService.removePortMap(input)).
                            thenReturn(getRpcFuture(output));
                        assertEquals(HTTP_OK, vtnMgr.removePortMap(input));
                        verify(portMapService).removePortMap(input);
                        verifyNoVtnInteractions();
                    }
                }
            }
        }

        String tname = "vtn_fail";
        String bname = "vbr_fail";
        String iname = "vif_fail";
        RemovePortMapInput input = new RemovePortMapInputBuilder().
            setTenantName(tname).
            setBridgeName(bname).
            setInterfaceName(iname).
            build();

        // In case of failure.
        for (VtnErrorTag vtag: VtnErrorTag.values()) {
            int expected = VTN_ERROR_CODES.get(vtag).intValue();
            Future<RpcResult<RemovePortMapOutput>> future =
                getFailureFuture(vtag, "Test failure");
            reset(portMapService);
            when(portMapService.removePortMap(input)).thenReturn(future);
            assertEquals(expected, vtnMgr.removePortMap(input));
            verify(portMapService).removePortMap(input);
            verifyNoVtnInteractions();
        }

        // Future throws an exception.
        IllegalStateException ise = new IllegalStateException("Unexpected");
        Future<RpcResult<RemovePortMapOutput>> future =
            Futures.immediateFailedFuture(ise);
        reset(portMapService);
        when(portMapService.removePortMap(input)).thenReturn(future);
        assertEquals(HTTP_INTERNAL_ERROR, vtnMgr.removePortMap(input));
        verify(portMapService).removePortMap(input);
        verifyNoVtnInteractions();

        // Future returns null.
        RpcResult<RemovePortMapOutput> result = null;
        future = Futures.immediateFuture(result);
        reset(portMapService);
        when(portMapService.removePortMap(input)).thenReturn(future);
        assertEquals(HTTP_INTERNAL_ERROR, vtnMgr.removePortMap(input));
        verify(portMapService).removePortMap(input);
        verifyNoVtnInteractions();

        // RpcResult does not contain output.
        future = getRpcFuture((RemovePortMapOutput)null);
        reset(portMapService);
        when(portMapService.removePortMap(input)).thenReturn(future);
        assertEquals(HTTP_INTERNAL_ERROR, vtnMgr.removePortMap(input));
        verify(portMapService).removePortMap(input);
        verifyNoVtnInteractions();

        // No RpcError in RpcResult.
        result = mock(RpcResult.class);
        when(result.isSuccessful()).thenReturn(false);
        when(result.getErrors()).thenReturn(null);
        future = Futures.immediateFuture(result);
        reset(portMapService);
        when(portMapService.removePortMap(input)).thenReturn(future);
        assertEquals(HTTP_INTERNAL_ERROR, vtnMgr.removePortMap(input));
        verify(portMapService).removePortMap(input);
        verifyNoVtnInteractions();
        verify(result).isSuccessful();
        verify(result).getErrors();
        verifyNoMoreInteractions(result);

        Collection<RpcError> errors = Collections.<RpcError>emptySet();
        result = mock(RpcResult.class);
        when(result.isSuccessful()).thenReturn(false);
        when(result.getErrors()).thenReturn(errors);
        future = Futures.immediateFuture(result);
        reset(portMapService);
        when(portMapService.removePortMap(input)).thenReturn(future);
        assertEquals(HTTP_INTERNAL_ERROR, vtnMgr.removePortMap(input));
        verify(portMapService).removePortMap(input);
        verifyNoVtnInteractions();
        verify(result).isSuccessful();
        verify(result).getErrors();
        verifyNoMoreInteractions(result);

        // Unexpected error information.
        IllegalArgumentException iae = new IllegalArgumentException("Too big");
        result = RpcResultBuilder.<RemovePortMapOutput>failed().
            withError(ErrorType.APPLICATION, "too-big",
                      "Unknown error 1", "unknown", null, iae).
            withError(ErrorType.APPLICATION, "in-use",
                      "Unknown error 2", null, null, null).
            build();
        future = Futures.immediateFuture(result);
        reset(portMapService);
        when(portMapService.removePortMap(input)).thenReturn(future);
        assertEquals(HTTP_INTERNAL_ERROR, vtnMgr.removePortMap(input));
        verify(portMapService).removePortMap(input);
        verifyNoVtnInteractions();
    }

    /**
     * Create a new VTN Manager service instance.
     *
     * @return  A {@link VTNManagerService} instance.
     */
    private VTNManagerService getVTNManagerService() {
        initMocks(this);
        MdsalUtils mdSal = new MdsalUtils(dataBroker);

        when(rpcRegistry.getRpcService(VtnService.class)).
            thenReturn(vtnService);
        when(rpcRegistry.getRpcService(VtnVbridgeService.class)).
            thenReturn(vbridgeService);
        when(rpcRegistry.getRpcService(VtnVinterfaceService.class)).
            thenReturn(vinterfaceService);
        when(rpcRegistry.getRpcService(VtnPortMapService.class)).
            thenReturn(portMapService);

        VTNManagerService vtn = new VTNManagerService(mdSal, rpcRegistry);
        verify(rpcRegistry).getRpcService(VtnService.class);
        verify(rpcRegistry).getRpcService(VtnVbridgeService.class);
        verify(rpcRegistry).getRpcService(VtnVinterfaceService.class);
        verify(rpcRegistry).getRpcService(VtnPortMapService.class);
        verifyNoMoreInteractions(rpcRegistry);

        return vtn;
    }

    /**
     * Ensure that all the VTN RPC mock-ups have no more interactions.
     */
    private void verifyNoVtnInteractions() {
        verifyNoMoreInteractions(vtnService, vbridgeService, vinterfaceService,
                                 portMapService);
    }

    /**
     * Return a future that contains RPC result that indicates failure.
     *
     * @param vtag  A {@link VtnErrorTag} instance.
     * @param msg   An error message.
     * @param <O>   The type of the RPC output;
     * @return  A future that contains the RPC result.
     */
    private <O> Future<RpcResult<O>> getFailureFuture(VtnErrorTag vtag,
                                                      String msg) {
        RpcResult<O> result = RpcResultBuilder.<O>failed().
            withError(ErrorType.APPLICATION, "operation-failed", msg,
                      String.valueOf(vtag), null, null).
            build();
        return Futures.immediateFuture(result);
    }
}
