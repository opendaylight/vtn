/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.rpc;

import static org.mockito.Mockito.any;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.verifyZeroInteractions;
import static org.mockito.Mockito.when;

import static org.opendaylight.yangtools.yang.common.RpcError.ErrorSeverity.ERROR;
import static org.opendaylight.yangtools.yang.common.RpcError.ErrorType.APPLICATION;

import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;

import org.junit.Test;

import org.slf4j.Logger;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yangtools.yang.common.RpcError;
import org.opendaylight.yangtools.yang.common.RpcResult;
import org.opendaylight.yangtools.yang.common.RpcResultBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.UpdateVtnOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateOperationType;

/**
 * JUnit test for {@link RpcUtils}.
 */
public class RpcUtilsTest extends TestBase {
    /**
     * Test case for {@link RpcUtils#getNullInputException()}.
     */
    @Test
    public void testGetNullInputException() {
        RpcException e = RpcUtils.getNullInputException();
        assertEquals("RPC input cannot be null", e.getMessage());
        assertEquals(RpcErrorTag.MISSING_ELEMENT, e.getErrorTag());
        assertEquals(VtnErrorTag.BADREQUEST, e.getVtnErrorTag());
    }

    /**
     * Test case for {@link RpcUtils#getInvalidOperationException(VtnUpdateOperationType)}.
     */
    @Test
    public void testGetInvalidOperationException() {
        for (VtnUpdateOperationType op: VtnUpdateOperationType.values()) {
            RpcException e = RpcUtils.getInvalidOperationException(op);
            assertEquals("Invalid operation type: " + op, e.getMessage());
            assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
            assertEquals(VtnErrorTag.BADREQUEST, e.getVtnErrorTag());
        }
    }

    /**
     * Test case for
     * {@link RpcUtils#getErrorBuilder(Class,RpcErrorTag,String,VtnErrorTag,String)}.
     */
    @Test
    public void testGetErrorBuilder1() {
        RpcErrorTag[] etags = RpcErrorTag.values();
        String[] msgs = {null, "test message 1", "test message 2"};
        VtnErrorTag[] vtags = VtnErrorTag.values();
        String[] information = {null, "info 1", "info 2"};
        for (RpcErrorTag et: etags) {
            String etag = et.getErrorTag();
            for (String msg: msgs) {
                for (VtnErrorTag vt: vtags) {
                    String vtag = vt.toString();
                    for (String info: information) {
                        RpcResult<UpdateVtnOutput> result = RpcUtils.
                            getErrorBuilder(UpdateVtnOutput.class, et, msg,
                                            vt, info).
                            build();
                        assertEquals(false, result.isSuccessful());
                        assertEquals(null, result.getResult());

                        Collection<RpcError> errors = result.getErrors();
                        assertEquals(1, errors.size());
                        RpcError err = errors.iterator().next();
                        assertEquals(ERROR, err.getSeverity());
                        assertEquals(etag, err.getTag());
                        assertEquals(vtag, err.getApplicationTag());
                        assertEquals(msg, err.getMessage());
                        assertEquals(info, err.getInfo());
                        assertEquals(null, err.getCause());
                        assertEquals(APPLICATION, err.getErrorType());
                    }
                }
            }
        }
    }

    /**
     * Test case for {@link RpcUtils#getErrorBuilder(Class,Exception)}.
     *
     * <ul>
     *   <li>In case of {@link VTNException}.</li>
     * </ul>
     */
    @Test
    public void testGetErrorBuilder2() {
        RpcErrorTag[] etags = RpcErrorTag.values();
        VtnErrorTag[] vtags = VtnErrorTag.values();
        String[] msgs = {null, "test message 1", "test message 2"};
        for (RpcErrorTag et: etags) {
            String etag = et.getErrorTag();
            for (VtnErrorTag vt: vtags) {
                String vtag = vt.toString();
                for (String msg: msgs) {
                    RpcException re = new RpcException(et, vt, msg);
                    RpcResult<UpdateVtnOutput> result = RpcUtils.
                        getErrorBuilder(UpdateVtnOutput.class, re).
                        build();
                    assertEquals(false, result.isSuccessful());
                    assertEquals(null, result.getResult());

                    Collection<RpcError> errors = result.getErrors();
                    assertEquals(1, errors.size());
                    RpcError err = errors.iterator().next();
                    assertEquals(ERROR, err.getSeverity());
                    assertEquals(etag, err.getTag());
                    assertEquals(vtag, err.getApplicationTag());
                    assertEquals(msg, err.getMessage());
                    assertEquals(null, err.getInfo());
                    assertEquals(null, err.getCause());
                    assertEquals(APPLICATION, err.getErrorType());
                }
            }
        }
    }

    /**
     * Test case for {@link RpcUtils#getErrorBuilder(Class,Exception)}.
     *
     * <ul>
     *   <li>In case of {@link RpcException}.</li>
     * </ul>
     */
    @Test
    public void testGetErrorBuilder3() {
        VtnErrorTag[] vtags = VtnErrorTag.values();
        String[] msgs = {null, "test message 1", "test message 2"};
        for (VtnErrorTag vt: vtags) {
            RpcErrorTag et;
            if (vt == VtnErrorTag.BADREQUEST) {
                et = RpcErrorTag.INVALID_VALUE;
            } else if (vt == VtnErrorTag.NOTFOUND) {
                et = RpcErrorTag.DATA_MISSING;
            } else if (vt == VtnErrorTag.CONFLICT) {
                et = RpcErrorTag.DATA_EXISTS;
            } else {
                et = RpcErrorTag.OPERATION_FAILED;
            }
            String vtag = vt.toString();
            String etag = et.getErrorTag();

            for (String msg: msgs) {
                VTNException ve = new VTNException(vt, msg);
                RpcResult<UpdateVtnOutput> result = RpcUtils.
                    getErrorBuilder(UpdateVtnOutput.class, ve).
                    build();
                assertEquals(false, result.isSuccessful());
                assertEquals(null, result.getResult());

                Collection<RpcError> errors = result.getErrors();
                assertEquals(1, errors.size());
                RpcError err = errors.iterator().next();
                assertEquals(ERROR, err.getSeverity());
                assertEquals(etag, err.getTag());
                assertEquals(vtag, err.getApplicationTag());
                assertEquals(msg, err.getMessage());
                assertEquals(null, err.getInfo());
                assertEquals(null, err.getCause());
                assertEquals(APPLICATION, err.getErrorType());
            }
        }
    }

    /**
     * Test case for {@link RpcUtils#getErrorBuilder(Class,Exception)}.
     *
     * <ul>
     *   <li>In case of {@link IllegalStateException}.</li>
     * </ul>
     */
    @Test
    public void testGetErrorBuilder4() {
        IllegalStateException ise = new IllegalStateException();
        RpcResult<UpdateVtnOutput> result = RpcUtils.
            getErrorBuilder(UpdateVtnOutput.class, ise).
            build();
        assertEquals(false, result.isSuccessful());
        assertEquals(null, result.getResult());

        Collection<RpcError> errors = result.getErrors();
        assertEquals(1, errors.size());
        RpcError err = errors.iterator().next();
        assertEquals(ERROR, err.getSeverity());
        assertEquals(RpcErrorTag.OPERATION_FAILED.getErrorTag(), err.getTag());
        assertEquals(VtnErrorTag.INTERNALERROR.toString(),
                     err.getApplicationTag());
        assertEquals("Caught unexpected exception: " + ise, err.getMessage());
        assertEquals(null, err.getInfo());
        assertEquals(null, err.getCause());
        assertEquals(APPLICATION, err.getErrorType());
    }

    /**
     * Test case for {@link RpcUtils#checkResult(RpcRequest,RpcResult,Logger)}.
     *
     * <ul>
     *   <li>In case of successful completion</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCheckResult() throws Exception {
        RpcRequest req = mock(RpcRequest.class);
        Logger logger = mock(Logger.class);
        UpdateVtnOutput output = mock(UpdateVtnOutput.class);
        RpcResult<UpdateVtnOutput> result = RpcResultBuilder.
            success(output).build();
        assertSame(output, RpcUtils.checkResult(req, result, logger));
        verifyZeroInteractions(req, logger, output);
    }

    /**
     * Test case for {@link RpcUtils#checkResult(RpcRequest,RpcResult,Logger)}.
     *
     * <ul>
     *   <li>RPC result is {@code null}.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCheckResultNull() throws Exception {
        RpcRequest req = mock(RpcRequest.class);
        String name = "test-rpc-1";
        String input = "test input 1";
        when(req.getName()).thenReturn(name);
        when(req.getInputForLog()).thenReturn(input);

        String msg = "RPC did not return the result";
        Logger logger = mock(Logger.class);
        try {
            RpcUtils.checkResult(req, null, logger);
            unexpected();
        } catch (VTNException e) {
            assertEquals(msg, e.getMessage());
            assertEquals(VtnErrorTag.INTERNALERROR, e.getVtnErrorTag());
            assertEquals(null, e.getCause());
        }

        verify(logger).error("{}: {}: input={}", name, msg, input);
        verify(req).getName();
        verify(req).getInputForLog();
        verifyNoMoreInteractions(req, logger);
    }

    /**
     * Test case for {@link RpcUtils#checkResult(RpcRequest,RpcResult,Logger)}.
     *
     * <ul>
     *   <li>Result does not contain any error information.</li>
     *   <li>
     *     {@link RpcRequest#needErrorLog(Collection)} returns {@code true}.
     *   </li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCheckResultNoError1() throws Exception {
        RpcRequest req = mock(RpcRequest.class);
        String name = "test-rpc-2";
        String input = "test input 2";
        when(req.getName()).thenReturn(name);
        when(req.getInputForLog()).thenReturn(input);
        when(req.needErrorLog(any(Collection.class))).thenReturn(true);

        @SuppressWarnings("unchecked")
        RpcResult<UpdateVtnOutput> result = mock(RpcResult.class);
        when(result.isSuccessful()).thenReturn(false);

        String msg = "RPC returned error";
        Logger logger = mock(Logger.class);
        try {
            RpcUtils.checkResult(req, result, logger);
            unexpected();
        } catch (VTNException e) {
            assertEquals(msg, e.getMessage());
            assertEquals(VtnErrorTag.INTERNALERROR, e.getVtnErrorTag());
            assertEquals(null, e.getCause());
        }

        Collection<RpcError> errors = Collections.<RpcError>emptyList();
        verify(logger).
            error("{}: {}: input={}, errors={}", name, msg, input, errors);
        verify(req).getName();
        verify(req).getInputForLog();
        verify(req).needErrorLog(errors);
        verify(result).isSuccessful();
        verify(result).getErrors();
        verifyNoMoreInteractions(req, logger, result);
    }

    /**
     * Test case for {@link RpcUtils#checkResult(RpcRequest,RpcResult,Logger)}.
     *
     * <ul>
     *   <li>Result does not contain any error information.</li>
     *   <li>
     *     {@link RpcRequest#needErrorLog(Collection)} returns {@code false}.
     *   </li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCheckResultNoError2() throws Exception {
        RpcRequest req = mock(RpcRequest.class);
        String name = "test-rpc-3";
        String input = "test input 3";
        when(req.getName()).thenReturn(name);
        when(req.getInputForLog()).thenReturn(input);
        when(req.needErrorLog(any(Collection.class))).thenReturn(false);

        @SuppressWarnings("unchecked")
        RpcResult<UpdateVtnOutput> result = mock(RpcResult.class);
        when(result.isSuccessful()).thenReturn(false);

        String msg = "RPC returned error";
        Logger logger = mock(Logger.class);
        try {
            RpcUtils.checkResult(req, result, logger);
            unexpected();
        } catch (VTNException e) {
            assertEquals(msg, e.getMessage());
            assertEquals(VtnErrorTag.INTERNALERROR, e.getVtnErrorTag());
            assertEquals(null, e.getCause());
        }

        Collection<RpcError> errors = Collections.<RpcError>emptyList();
        verify(req).needErrorLog(errors);
        verify(result).isSuccessful();
        verify(result).getErrors();
        verifyNoMoreInteractions(req, logger, result);
    }

    /**
     * Test case for {@link RpcUtils#checkResult(RpcRequest,RpcResult,Logger)}.
     *
     * <ul>
     *   <li>Result does not contain {@link Throwable}.</li>
     *   <li>
     *     {@link RpcRequest#needErrorLog(Collection)} returns {@code true}.
     *   </li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCheckResultNoCause1() throws Exception {
        RpcRequest req = mock(RpcRequest.class);
        String name = "test-rpc-4";
        String input = "test input 4";
        when(req.getName()).thenReturn(name);
        when(req.getInputForLog()).thenReturn(input);
        when(req.needErrorLog(any(Collection.class))).thenReturn(true);

        Collection<RpcError> errors = Arrays.asList(
            mock(RpcError.class), mock(RpcError.class), mock(RpcError.class));

        @SuppressWarnings("unchecked")
        RpcResult<UpdateVtnOutput> result = mock(RpcResult.class);
        when(result.isSuccessful()).thenReturn(false);
        when(result.getErrors()).thenReturn(errors);

        String msg = "RPC returned error";
        Logger logger = mock(Logger.class);
        try {
            RpcUtils.checkResult(req, result, logger);
            unexpected();
        } catch (VTNException e) {
            assertEquals(msg, e.getMessage());
            assertEquals(VtnErrorTag.INTERNALERROR, e.getVtnErrorTag());
            assertEquals(null, e.getCause());
        }

        verify(logger).
            error("{}: {}: input={}, errors={}", name, msg, input, errors);
        verify(req).getName();
        verify(req).getInputForLog();
        verify(req).needErrorLog(errors);
        verify(result).isSuccessful();
        verify(result).getErrors();
        verifyNoMoreInteractions(req, logger, result);
    }

    /**
     * Test case for {@link RpcUtils#checkResult(RpcRequest,RpcResult,Logger)}.
     *
     * <ul>
     *   <li>Result does not contain {@link Throwable}.</li>
     *   <li>
     *     {@link RpcRequest#needErrorLog(Collection)} returns {@code false}.
     *   </li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCheckResultNoCause2() throws Exception {
        RpcRequest req = mock(RpcRequest.class);
        String name = "test-rpc-5";
        String input = "test input 5";
        when(req.getName()).thenReturn(name);
        when(req.getInputForLog()).thenReturn(input);
        when(req.needErrorLog(any(Collection.class))).thenReturn(false);

        Collection<RpcError> errors = Arrays.asList(
            mock(RpcError.class), mock(RpcError.class), mock(RpcError.class));

        @SuppressWarnings("unchecked")
        RpcResult<UpdateVtnOutput> result = mock(RpcResult.class);
        when(result.isSuccessful()).thenReturn(false);
        when(result.getErrors()).thenReturn(errors);

        String msg = "RPC returned error";
        Logger logger = mock(Logger.class);
        try {
            RpcUtils.checkResult(req, result, logger);
            unexpected();
        } catch (VTNException e) {
            assertEquals(msg, e.getMessage());
            assertEquals(VtnErrorTag.INTERNALERROR, e.getVtnErrorTag());
            assertEquals(null, e.getCause());
        }

        verify(req).needErrorLog(errors);
        verify(result).isSuccessful();
        verify(result).getErrors();
        verifyNoMoreInteractions(req, logger, result);
    }

    /**
     * Test case for {@link RpcUtils#checkResult(RpcRequest,RpcResult,Logger)}.
     *
     * <ul>
     *   <li>Result contains a {@link Throwable}.</li>
     *   <li>
     *     {@link RpcRequest#needErrorLog(Collection)} returns {@code true}.
     *   </li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCheckResultCause1() throws Exception {
        RpcRequest req = mock(RpcRequest.class);
        String name = "test-rpc-6";
        String input = "test input 6";
        when(req.getName()).thenReturn(name);
        when(req.getInputForLog()).thenReturn(input);
        when(req.needErrorLog(any(Collection.class))).thenReturn(true);

        IllegalStateException ise = new IllegalStateException();
        RpcError err = mock(RpcError.class);
        when(err.getCause()).thenReturn(ise);
        Collection<RpcError> errors = Arrays.asList(
            mock(RpcError.class), err, mock(RpcError.class));

        @SuppressWarnings("unchecked")
        RpcResult<UpdateVtnOutput> result = mock(RpcResult.class);
        when(result.isSuccessful()).thenReturn(false);
        when(result.getErrors()).thenReturn(errors);

        String msg = "RPC returned error";
        Logger logger = mock(Logger.class);
        try {
            RpcUtils.checkResult(req, result, logger);
            unexpected();
        } catch (VTNException e) {
            assertEquals(msg, e.getMessage());
            assertEquals(VtnErrorTag.INTERNALERROR, e.getVtnErrorTag());
            assertEquals(null, e.getCause());
        }

        String lmsg = name + ": " + msg + ": input=" + input +
            ", errors=" + errors;
        verify(logger).error(lmsg, ise);
        verify(req).getName();
        verify(req).getInputForLog();
        verify(req).needErrorLog(errors);
        verify(result).isSuccessful();
        verify(result).getErrors();
        verifyNoMoreInteractions(req, logger, result);
    }

    /**
     * Test case for {@link RpcUtils#checkResult(RpcRequest,RpcResult,Logger)}.
     *
     * <ul>
     *   <li>Result contains a {@link Throwable}.</li>
     *   <li>
     *     {@link RpcRequest#needErrorLog(Collection)} returns {@code false}.
     *   </li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCheckResultCause2() throws Exception {
        RpcRequest req = mock(RpcRequest.class);
        String name = "test-rpc-7";
        String input = "test input 7";
        when(req.getName()).thenReturn(name);
        when(req.getInputForLog()).thenReturn(input);
        when(req.needErrorLog(any(Collection.class))).thenReturn(false);

        IllegalStateException ise = new IllegalStateException();
        RpcError err = mock(RpcError.class);
        when(err.getCause()).thenReturn(ise);
        Collection<RpcError> errors = Arrays.asList(
            mock(RpcError.class), err, mock(RpcError.class));

        @SuppressWarnings("unchecked")
        RpcResult<UpdateVtnOutput> result = mock(RpcResult.class);
        when(result.isSuccessful()).thenReturn(false);
        when(result.getErrors()).thenReturn(errors);

        String msg = "RPC returned error";
        Logger logger = mock(Logger.class);
        try {
            RpcUtils.checkResult(req, result, logger);
            unexpected();
        } catch (VTNException e) {
            assertEquals(msg, e.getMessage());
            assertEquals(VtnErrorTag.INTERNALERROR, e.getVtnErrorTag());
            assertEquals(null, e.getCause());
        }

        verify(req).needErrorLog(errors);
        verify(result).isSuccessful();
        verify(result).getErrors();
        verifyNoMoreInteractions(req, logger, result);
    }
}
