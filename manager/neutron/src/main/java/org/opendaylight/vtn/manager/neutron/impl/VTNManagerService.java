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

import java.util.Collection;
import java.util.List;
import java.util.Map;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;

import com.google.common.base.Optional;
import com.google.common.collect.ImmutableMap;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.controller.sal.binding.api.RpcConsumerRegistry;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;
import org.opendaylight.yangtools.yang.common.RpcError;
import org.opendaylight.yangtools.yang.common.RpcResult;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.RemovePortMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.RemovePortMapOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.SetPortMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.SetPortMapOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.VtnPortMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.RemoveVtnInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.RemoveVtnInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.UpdateVtnInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.UpdateVtnInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.UpdateVtnOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.VtnService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.Vtns;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.Vtn;
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
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.VtnVbridgeService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.info.VbridgeConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.VbridgeKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.RemoveVinterfaceInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.UpdateVinterfaceInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.UpdateVinterfaceOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.VtnVinterfaceService;

/**
 * {@code VTNManagerService} provides interfaces to control VTN Manager.
 */
public final class VTNManagerService {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(VTNManagerService.class);

    /**
     * A map that keeps pairs of HTTP status code associated with VTN error
     * tag.
     */
    private static final Map<String, Integer>  VTN_ERROR_CODES;

    /**
     * The number of seconds to wait for completion of RPC.
     */
    private static final long  RPC_TIMEOUT = 60L;

    /**
     * MD-SAL utility service.
     */
    private final MdsalUtils  mdSal;

    /**
     * RPC service for VTN management.
     */
    private final VtnService  vtnService;

    /**
     * RPC service for vBridge management.
     */
    private final VtnVbridgeService  vbridgeService;

    /**
     * RPC service for virtual interface management.
     */
    private final VtnVinterfaceService  vinterfaceService;

    /**
     * RPC service for port mapping management.
     */
    private final VtnPortMapService  portMapService;

    /**
     * Initialize statis field.
     */
    static {
        VTN_ERROR_CODES = ImmutableMap.<String, Integer>builder().
            put(VtnErrorTag.BADREQUEST.toString(), HTTP_BAD_REQUEST).
            put(VtnErrorTag.UNAUTHORIZED.toString(), HTTP_UNAUTHORIZED).
            put(VtnErrorTag.NOTFOUND.toString(), HTTP_NOT_FOUND).
            put(VtnErrorTag.NOTACCEPTABLE.toString(), HTTP_NOT_ACCEPTABLE).
            put(VtnErrorTag.TIMEOUT.toString(), HTTP_CLIENT_TIMEOUT).
            put(VtnErrorTag.CONFLICT.toString(), HTTP_CONFLICT).
            put(VtnErrorTag.GONE.toString(), HTTP_GONE).
            put(VtnErrorTag.NOSERVICE.toString(), HTTP_UNAVAILABLE).
            put(VtnErrorTag.INTERNALERROR.toString(), HTTP_INTERNAL_ERROR).
            build();
    }

    /**
     * {@code VTNRpcResult} describes a result of VTN RPC invocation.
     *
     * @param <O>  The type of the RPC output.
     */
    private static final class VTNRpcResult<O> {
        /**
         * HTTP status code.
         */
        private final int  statusCode;

        /**
         * The output of the RPC.
         */
        private final O  output;

        /**
         * An error message.
         */
        private final String  errorMessage;

        /**
         * Construct a new instance that indicates successful completion.
         *
         * @param out  The output of the RPC.
         */
        private VTNRpcResult(O out) {
            statusCode = HTTP_OK;
            output = out;
            errorMessage = null;
        }

        /**
         * Construct a new instance that indicates an internal error.
         *
         * @param msg  An error message.
         */
        private VTNRpcResult(String msg) {
            statusCode = HTTP_INTERNAL_ERROR;
            output = null;
            errorMessage = msg;
        }

        /**
         * Construct a new instance.
         *
         * @param msg   An error message.
         * @param code  A HTTP status code that indicates the cause of error.
         */
        private VTNRpcResult(String msg, int code) {
            statusCode = code;
            output = null;
            errorMessage = msg;
        }

        /**
         * Return the HTTP status code that indicates the result of the RPC
         * invocation.
         *
         * @return  A HTTP status code.
         *          {@link java.net.HttpURLConnection#HTTP_OK} indicates
         *          successful completion.
         */
        private int getStatusCode() {
            return statusCode;
        }

        /**
         * Return the output of the RPC invocation.
         *
         * @return  The RPC output.
         */
        private O getOutput() {
            return output;
        }

        /**
         * Return an error message.
         *
         * @return  An error message.
         */
        private String getErrorMessage() {
            return errorMessage;
        }
    }

    /**
     * Return an instance identifier that specifies the VTN.
     *
     * @param tname  The name of the VTN.
     * @return  An {@link InstanceIdentifier} instance.
     */
    public static InstanceIdentifier<Vtn> getTenantPath(String tname) {
        VtnKey vtnKey = new VtnKey(new VnodeName(tname));
        return InstanceIdentifier.builder(Vtns.class).
            child(Vtn.class, vtnKey).
            build();
    }

    /**
     * Return an instance identifier that specifies the vBridge configuration.
     *
     * @param tname  The name of the VTN.
     * @param bname  The name of the vBridge.
     * @return  An {@link InstanceIdentifier} instance.
     */
    public static InstanceIdentifier<VbridgeConfig> getBridgeConfigPath(
        String tname, String bname) {
        VtnKey vtnKey = new VtnKey(new VnodeName(tname));
        VbridgeKey vbrKey = new VbridgeKey(new VnodeName(bname));
        return InstanceIdentifier.builder(Vtns.class).
            child(Vtn.class, vtnKey).
            child(Vbridge.class, vbrKey).
            child(VbridgeConfig.class).
            build();
    }

    /**
     * Wait for completion of the RPC task associated with the given future.
     *
     * @param f    A {@link Future} instance associated with the RPC task.
     * @param <O>  The type of the RPC output.
     * @return  A {@link VTNRpcResult} instance that contains the result of
     *          the RPC invocation.
     */
    private static <O> VTNRpcResult<O> getRpcResult(Future<RpcResult<O>> f) {
        return getRpcResult(f, false);
    }

    /**
     * Wait for completion of the RPC task associated with the given future.
     *
     * @param f         A {@link Future} instance associated with the RPC task.
     * @param nillable  Set {@code true} if the result can be null.
     * @param <O>       The type of the RPC output.
     * @return  A {@link VTNRpcResult} instance that contains the result of
     *          the RPC invocation.
     */
    private static <O> VTNRpcResult<O> getRpcResult(Future<RpcResult<O>> f,
                                                    boolean nillable) {
        RpcResult<O> result;
        try {
            result = f.get(RPC_TIMEOUT, TimeUnit.SECONDS);
        } catch (Exception e) {
            String msg =
                "Caught an exception while waiting for RPC completion";
            LOG.error(msg, e);
            return new VTNRpcResult<O>(msg + ": " + e.toString());
        }

        VTNRpcResult<O> vres;
        if (result == null) {
            // This should never happen.
            vres = new VTNRpcResult<O>("RPC did not set result.");
        } else if (result.isSuccessful()) {
            O res = result.getResult();
            if (!nillable && res == null) {
                // This should never happen.
                vres = new VTNRpcResult<O>("RPC did not set output.");
            } else {
                vres = new VTNRpcResult<O>(res);
            }
        } else {
            vres = getRpcErrorResult(result);
        }

        return vres;
    }

    /**
     * Construct a {@link VTNRpcResult} instance that indicates an error.
     *
     * @param result  An {@link RpcResult} instance that indicates an error.
     * @param <O>     The type of the RPC output.
     * @return  A {@link VTNRpcResult} instance that contains the result of
     *          the RPC invocation.
     */
    private static <O> VTNRpcResult<O> getRpcErrorResult(RpcResult<O> result) {
        VTNRpcResult<O> vres;
        Collection<RpcError> errors = result.getErrors();
        if (errors == null || errors.isEmpty()) {
            // This should never happen.
            String msg = "RPC failed without error information: " + result;
            vres = new VTNRpcResult<O>(msg);
        } else {
            // VTN RPC sets only one RpcError, and it contains encoded
            // VtnErrorTag value in application tag.
            RpcError rerr = errors.iterator().next();
            String appTag = rerr.getApplicationTag();
            Integer code = VTN_ERROR_CODES.get(appTag);
            if (code != null) {
                String msg = appTag + ": " + rerr.getMessage();
                vres = new VTNRpcResult<O>(msg, code.intValue());
            } else {
                // Unexpected error.
                int index = 0;
                for (RpcError re: errors) {
                    Throwable cause = re.getCause();
                    String msg = "RPC failed: error[" + index + "]=" + re;
                    if (cause == null) {
                        LOG.error(msg);
                    } else {
                        LOG.error(msg, cause);
                    }
                    index++;
                }

                vres = new VTNRpcResult<O>("Internal error");
            }
        }

        return vres;
    }

    /**
     * Construct a new instance.
     *
     * @param md   A {@link MdsalUtils} instance.
     * @param rpc  A {@link RpcConsumerRegistry} instance.
     */
    public VTNManagerService(MdsalUtils md, RpcConsumerRegistry rpc) {
        mdSal = md;
        vtnService = rpc.getRpcService(VtnService.class);
        vbridgeService = rpc.getRpcService(VtnVbridgeService.class);
        vinterfaceService = rpc.getRpcService(VtnVinterfaceService.class);
        portMapService = rpc.getRpcService(VtnPortMapService.class);
    }

    /**
     * Create or update a VTN with default parameters.
     *
     * @param name  The name of the VTN.
     * @param mode  A {@link VnodeUpdateMode} instance that specifies how to
     *              update the VTN.
     *              {@code null} implies {@link VnodeUpdateMode#UPDATE}.
     * @return  A HTTP status code that indicates the result.
     *          {@link java.net.HttpURLConnection#HTTP_OK} indicates
     *          successful completion.
     */
    public int updateTenant(String name, VnodeUpdateMode mode) {
        UpdateVtnInput input = new UpdateVtnInputBuilder().
            setTenantName(name).
            setUpdateMode(mode).
            setOperation(VtnUpdateOperationType.SET).
            build();
        VTNRpcResult<UpdateVtnOutput> result =
            getRpcResult(vtnService.updateVtn(input));
        int code = result.getStatusCode();
        if (code != HTTP_OK) {
            LOG.error("Failed to update VTN: input={}, err={}",
                      input, result.getErrorMessage());
        } else if (LOG.isDebugEnabled()) {
            VtnUpdateType utype = result.getOutput().getStatus();
            String msg;
            if (utype == VtnUpdateType.CREATED) {
                msg = "A VTN has been created";
            } else if (utype == VtnUpdateType.CHANGED) {
                msg = "A VTN has been changed";
            } else {
                assert utype == null;
                msg = "A VTN is present and not changed";
            }

            LOG.debug("{}: name={}", msg, name);
        }

        return code;
    }

    /**
     * Remove the specified VTN.
     *
     * @param name  The name of the VTN.
     * @return  A HTTP status code that indicates the result.
     *          {@link java.net.HttpURLConnection#HTTP_OK} indicates
     *          successful completion.
     */
    public int removeTenant(String name) {
        RemoveVtnInput input = new RemoveVtnInputBuilder().
            setTenantName(name).build();
        VTNRpcResult<?> result =
            getRpcResult(vtnService.removeVtn(input), true);
        int code = result.getStatusCode();
        if (code == HTTP_OK) {
            LOG.debug("A VTN has been removed: name={}", name);
        } else {
            LOG.error("Failed to remove VTN: name={}, err={}",
                      name, result.getErrorMessage());
        }

        return code;
    }

    /**
     * Determine whether the specified VTN contains at least one vBridge or
     * not.
     *
     * @param tname  The name of the VTN.
     * @return  {@code true} if the specified VTN contains at least one
     *          vBridge. {@code false} otherwise.
     */
    public boolean hasBridge(String tname) {
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        Optional<Vtn> opt = mdSal.read(oper, getTenantPath(tname));
        boolean ret;
        if (opt.isPresent()) {
            List<Vbridge> vbridges = opt.get().getVbridge();
            ret = (vbridges != null && !vbridges.isEmpty());
        } else {
            ret = false;
        }

        return ret;
    }

    /**
     * Create or update the specified vBridge.
     *
     * @param tname  The name of the VTN.
     * @param bname  The name of the vBridge.
     * @param desc   A brief description about the vBridge.
     * @param mode   A {@link VnodeUpdateMode} instance that specifies how to
     *               update the vBridge.
     *               {@code null} implies {@link VnodeUpdateMode#UPDATE}.
     * @return  A HTTP status code that indicates the result.
     *          {@link java.net.HttpURLConnection#HTTP_OK} indicates
     *          successful completion.
     */
    public int updateBridge(String tname, String bname, String desc,
                            VnodeUpdateMode mode) {
        UpdateVbridgeInput input = new UpdateVbridgeInputBuilder().
            setTenantName(tname).
            setBridgeName(bname).
            setDescription(desc).
            setUpdateMode(mode).
            setOperation(VtnUpdateOperationType.SET).
            build();
        VTNRpcResult<UpdateVbridgeOutput> result =
            getRpcResult(vbridgeService.updateVbridge(input));
        int code = result.getStatusCode();
        if (code != HTTP_OK) {
            LOG.error("Failed to update vBridge: input={}, err={}",
                      input, result.getErrorMessage());
        } else if (LOG.isDebugEnabled()) {
            VtnUpdateType utype = result.getOutput().getStatus();
            String msg;
            if (utype == VtnUpdateType.CREATED) {
                msg = "A vBridge has been created";
            } else if (utype == VtnUpdateType.CHANGED) {
                msg = "A vBridge has been changed";
            } else {
                assert utype == null;
                msg = "A vBridge is present and not changed";
            }

            LOG.debug("{}: path={}/{}, desc={}", msg, tname, bname, desc);
        }

        return code;
    }

    /**
     * Remove the specified vBridge.
     *
     * @param tname  The name of the VTN.
     * @param bname  The name of the vBridge.
     * @return  A HTTP status code that indicates the result.
     *          {@link java.net.HttpURLConnection#HTTP_OK} indicates
     *          successful completion.
     */
    public int removeBridge(String tname, String bname) {
        RemoveVbridgeInput input = new RemoveVbridgeInputBuilder().
            setTenantName(tname).
            setBridgeName(bname).
            build();
        VTNRpcResult<?> result =
            getRpcResult(vbridgeService.removeVbridge(input), true);
        int code = result.getStatusCode();
        if (code == HTTP_OK) {
            LOG.debug("A vBridge has been removed: path={}/{}", tname, bname);
        } else {
            LOG.error("Failed to remove vBridge: path={}/{}, err={}",
                      tname, bname, result.getErrorMessage());
        }

        return code;
    }

    /**
     * Return the current configuration of the specified vBridge.
     *
     * @param tname  The name of the VTN.
     * @param bname  The name of the vBridge.
     * @return  A {@link VbridgeConfig} instance if the specified vBridge is
     *          present. {@code null} otherwise.
     */
    public VbridgeConfig getBridgeConfig(String tname, String bname) {
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        return mdSal.read(oper, getBridgeConfigPath(tname, bname)).orNull();
    }

    /**
     * Create or update the specified virtual interface.
     *
     * @param input  A {@link UpdateVinterfaceInput} instance.
     * @return  A HTTP status code that indicates the result.
     *          {@link java.net.HttpURLConnection#HTTP_OK} indicates
     *          successful completion.
     */
    public int updateInterface(UpdateVinterfaceInput input) {
        VTNRpcResult<UpdateVinterfaceOutput> result =
            getRpcResult(vinterfaceService.updateVinterface(input));
        int code = result.getStatusCode();
        if (code != HTTP_OK) {
            LOG.error("Failed to update virtual interface: input={}, err={}",
                      input, result.getErrorMessage());
        } else if (LOG.isDebugEnabled()) {
            VtnUpdateType utype = result.getOutput().getStatus();
            String msg;
            if (utype == VtnUpdateType.CREATED) {
                msg = "A virtual interface has been created";
            } else if (utype == VtnUpdateType.CHANGED) {
                msg = "A virtual interface has been changed";
            } else {
                assert utype == null;
                msg = "A virtual interface is present and not changed";
            }

            LOG.debug("{}: input={}", msg, input);
        }

        return code;
    }

    /**
     * Remove the specified virtual interface.
     *
     * @param input  A {@link RemoveVinterfaceInput} instance.
     * @return  A HTTP status code that indicates the result.
     *          {@link java.net.HttpURLConnection#HTTP_OK} indicates
     *          successful completion.
     */
    public int removeInterface(RemoveVinterfaceInput input) {
        VTNRpcResult<?> result =
            getRpcResult(vinterfaceService.removeVinterface(input), true);
        int code = result.getStatusCode();
        if (code == HTTP_OK) {
            LOG.debug("A virtual interface has been removed: input={}",
                      input);
        } else {
            LOG.error("Failed to remove virtual interface: input={}, err={}",
                      input, result.getErrorMessage());
        }

        return code;
    }

    /**
     * Configure a port mapping into the virtual interface specified by the
     * given RPC input.
     *
     * @param input  A {@link SetPortMapInput} instance.
     * @return  A HTTP status code that indicates the result.
     *          {@link java.net.HttpURLConnection#HTTP_OK} indicates
     *          successful completion.
     */
    public int setPortMap(SetPortMapInput input) {
        VTNRpcResult<SetPortMapOutput> result =
            getRpcResult(portMapService.setPortMap(input));
        int code = result.getStatusCode();
        if (code != HTTP_OK) {
            LOG.error("Failed to set port mapping: input={}, err={}",
                      input, result.getErrorMessage());
        } else if (LOG.isDebugEnabled()) {
            VtnUpdateType utype = result.getOutput().getStatus();
            String msg;
            if (utype == VtnUpdateType.CREATED) {
                msg = "Port mapping has been created";
            } else if (utype == VtnUpdateType.CHANGED) {
                msg = "Port mapping has been changed";
            } else {
                assert utype == null;
                msg = "Port mapping is already configured";
            }

            LOG.debug("{}: input={}", msg, input);
        }

        return code;
    }

    /**
     * Remove the port mapping configuration from the specified virtual
     * interface.
     *
     * @param input  A {@link RemovePortMapInput} instance.
     * @return  A HTTP status code that indicates the result.
     *          {@link java.net.HttpURLConnection#HTTP_OK} indicates
     *          successful completion.
     */
    public int removePortMap(RemovePortMapInput input) {
        VTNRpcResult<RemovePortMapOutput> result =
            getRpcResult(portMapService.removePortMap(input));
        int code = result.getStatusCode();
        if (code != HTTP_OK) {
            LOG.error("Failed to remove port mapping: input={}, err={}",
                      input, result.getErrorMessage());
        } else if (LOG.isDebugEnabled()) {
            VtnUpdateType utype = result.getOutput().getStatus();
            String msg;
            if (utype == VtnUpdateType.REMOVED) {
                msg = "Port mapping has been removed";
            } else {
                assert utype == null;
                msg = "Port mapping is not configured";
            }

            LOG.debug("{}: input={}", msg, input);
        }

        return code;
    }
}
