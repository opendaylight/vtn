/*
 * Copyright (c) 2013, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.adsal;

import java.util.concurrent.Future;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.IVTNGlobal;

import org.opendaylight.vtn.manager.internal.VTNManagerProvider;

import org.opendaylight.yangtools.yang.common.RpcResult;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.version.rev150901.GetManagerVersionOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.version.rev150901.VtnVersionService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.version.rev150901.get.manager.version.output.BundleVersion;

/**
 * {@code GlobalResourceManager} class manages global resources used by the
 * VTN Manager.
 */
public class GlobalResourceManager implements IVTNGlobal {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(GlobalResourceManager.class);

    /**
     * MD-SAL VTN Manager service provider.
     */
    private VTNManagerProvider  vtnProvider;

    /**
     * Cached API version of the VTN Manager.
     */
    private int  apiVersion;

    /**
     * Cached OSGi bundle version of the VTN Manager.
     */
    private org.opendaylight.vtn.manager.BundleVersion  bundleVersion;

    /**
     * Invoked when a VTN Manager provider is registered.
     *
     * @param provider  VTN Manager provider service.
     */
    void setVTNProvider(VTNManagerProvider provider) {
        LOG.trace("Set VTN Manager provider: {}", provider);
        vtnProvider = provider;
    }

    /**
     * Invoked when a VTN Manager provider is unregistered.
     *
     * @param provider  VTN Manager provider service.
     */
    void unsetVTNProvider(VTNManagerProvider provider) {
        if (provider != null && provider.equals(vtnProvider)) {
            LOG.trace("Unset VTN Manager provider: {}", provider);
            vtnProvider = null;
        }
    }

    /**
     * Get VTN Manager's version information and cache it.
     */
    private void cacheManagerVersion() {
        if (bundleVersion != null) {
            // Already cached.
            return;
        }

        Throwable cause = null;
        String errmsg = null;
        try {
            VtnVersionService rpc =
                vtnProvider.getVtnRpcService(VtnVersionService.class);
            Future<RpcResult<GetManagerVersionOutput>> f =
                rpc.getManagerVersion();
            RpcResult<GetManagerVersionOutput> res = f.get();
            if (res.isSuccessful()) {
                GetManagerVersionOutput output = res.getResult();
                apiVersion = output.getApiVersion().intValue();
                BundleVersion bv = output.getBundleVersion();
                if (bv != null) {
                    bundleVersion = new org.opendaylight.vtn.manager.
                        BundleVersion(bv.getMajor().intValue(),
                                      bv.getMinor().intValue(),
                                      bv.getMicro().intValue(),
                                      bv.getQualifier());
                }

                return;
            } else {
                // This should never happen.
                errmsg = "get-manager-version RPC failed";
                LOG.error("{}: {}", errmsg, res.getErrors());
            }
        } catch (Exception e) {
            // This should never happen.
            errmsg = "get-manager-exception RPC failed due to exception.";
            LOG.error(errmsg, e);
            cause = e;
        }

        throw new IllegalStateException(errmsg, cause);
    }

    // IVTNGlobal

    /**
     * Return the API version of the VTN Manager.
     *
     * <p>
     *   The API version will be incremented when changes which breaks
     *   compatibility is made to the API of VTN Manager.
     * </p>
     *
     * @return  The API version of the VTN Manager.
     */
    @Override
    public int getApiVersion() {
        cacheManagerVersion();
        return apiVersion;
    }

    /**
     * Return the version information of the OSGi bundle which implements
     * the VTN Manager.
     *
     * @return  A {@link org.opendaylight.vtn.manager.BundleVersion} object
     *          which represents the version of the OSGi bundle which
     *          implements the VTN Manager.
     *          {@code null} is returned if the VTN Manager is not loaded by
     *          an OSGi bundle class loader.
     */
    @Override
    public org.opendaylight.vtn.manager.BundleVersion getBundleVersion() {
        cacheManagerVersion();
        return bundleVersion;
    }
}
