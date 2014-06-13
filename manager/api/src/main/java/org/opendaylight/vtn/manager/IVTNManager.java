/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.net.InetAddress;
import java.util.List;
import java.util.Set;

import org.opendaylight.vtn.manager.flow.DataFlow;
import org.opendaylight.vtn.manager.flow.DataFlowFilter;

import org.opendaylight.controller.hosttracker.hostAware.HostNodeConnector;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.packet.address.DataLinkAddress;
import org.opendaylight.controller.sal.utils.Status;

/**
 * {@code IVTNManager} is an interface that defines OSGi service for
 * operating the VTN Manager.
 *
 * <p>
 *   {@code IVTNManager} service is instantiated for each container and it is
 *   registered in the OSGi service registry as a container specific service.
 *   It is possible to give instructions to the VTN Manager inside the
 *   container by calling the necessary methods after acquiring the
 *   {@code IVTNManager} service instance from the OSGi service registry.
 * </p>
 */
public interface IVTNManager {
    /**
     * Determine whether the virtual networking provided by the VTN Manager
     * is operating inside the container.
     *
     * <p>
     *   If at least one {@linkplain <a href="package-summary.html#VTN">VTN</a>}
     *   exists inside the container, then it is judged that the virtual
     *   networking provided by the VTN Manager is operating. However,
     *   if a container other than the default container is present, then
     *   the VTN Manager inside the default container gets disabled.
     *   In this case, the call of this method against the {@code IVTNManager}
     *   service for the default container will always return {@code false}.
     * </p>
     *
     * @return  {@code true} is returned if the virtual networking provided
     *          by the VTN Manager is operating inside the container.
     *          Otherwise {@code false} is returned.
     */
    boolean isActive();

    /**
     * Return a list of {@linkplain <a href="package-summary.html#VTN">VTNs</a>}
     * present in the container.
     *
     * @return  A list of {@link VTenant} objects corresponding to all the
     *          VTNs present inside the container.
     *          An empty list is returned if no VTN is present inside the
     *          container.
     * @throws VTNException  An error occurred.
     *   The following are the main {@code StatusCode} set in {@link Status}
     *   delivered by the exception.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     */
    List<VTenant> getTenants() throws VTNException;

    /**
     * Return information about the specified
     * {@linkplain <a href="package-summary.html#VTN">VTN</a>}.
     *
     * @param path  A {@link VTenantPath} object that specifies the position
     *              of the VTN.
     * @return  A {@link VTenant} object which represents information about
     *          the VTN specified by {@code path}.
     * @throws VTNException  An error occurred.
     *   The following are the main {@code StatusCode} set in {@link Status}
     *   delivered by the exception.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.BADREQUEST}
     *     <dd>
     *       <ul style="padding-left: 1em;">
     *         <li>{@code null} is passed to {@code path}.</li>
     *         <li>
     *           {@code null} is configured in {@code path} for the
     *           {@linkplain VTenantPath#getTenantName() VTN name}.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>VTN specified by {@code path} does not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     */
    VTenant getTenant(VTenantPath path) throws VTNException;

    /**
     * Create a new {@linkplain <a href="package-summary.html#VTN">VTN</a>}
     * inside the container.
     *
     * @param path
     *   A {@link VTenantPath} object that specifies the position of new VTN.
     *   <p style="margin-left: 1em;">
     *     The name of the VTN inside {@link VTenantPath} must be a string
     *     that meets the following conditions.
     *   </p>
     *   <ul>
     *     <li>
     *       The length of the name must be greater than <strong>0</strong>
     *       and less than <strong>32</strong>.
     *     </li>
     *     <li>
     *       The name must consist of US-ASCII alphabets, numbers, and
     *       underscore ({@code '_'}).
     *     </li>
     *     <li>
     *       The name must start with an US-ASCII alphabet or number.
     *     </li>
     *   </ul>
     * @param tconf
     *   A {@link VTenantConfig} object which specifies the VTN configuration
     *   information.
     *   <ul>
     *     <li>
     *       The description of the VTN is not registered if {@code null}
     *       is configured in {@code tconf} for the
     *       {@linkplain VTenantConfig#getDescription() description}.
     *     </li>
     *     <li>
     *       {@link VTenantConfig#getIdleTimeout() idle_timeout} of flow entry
     *       will be treated as <strong>300</strong> if it is not configured
     *       in {@code tconf}.
     *     </li>
     *     <li>
     *       {@link VTenantConfig#getHardTimeout() hard_timeout} of flow entry
     *       will be treated as <strong>0</strong> if it is not configured
     *       in {@code tconf}.
     *     </li>
     *   </ul>
     * @return
     *   A {@link Status} object which represents the result of the operation
     *   is returned.
     *   <p>
     *     Upon successful completion,
     *     <strong>{@code StatusCode.SUCCESS}</strong> is set in a returned
     *     object. Otherwise a {@code StatusCode} which indicates the cause
     *     of error is set in a returned {@link Status} object.
     *     The following are the main {@code StatusCode} configured in
     *     {@link Status}.
     *   </p>
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.BADREQUEST}
     *     <dd>
     *       <ul style="padding-left: 1em;">
     *         <li>
     *           {@code null} is passed to {@code path} or {@code tconf}.
     *         </li>
     *         <li>
     *           Incorrect {@linkplain VTenantPath#getTenantName() VTN name} is
     *           configured in {@code tconf}.
     *         </li>
     *         <li>
     *           Incorrect value is configured in {@code tconf} for
     *           {@link VTenantConfig#getIdleTimeout() idle_timeout} or
     *           {@link VTenantConfig#getHardTimeout() hard_timeout}.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.CONFLICT}
     *     <dd>The VTN specified by {@code path} already exists.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with the default container, and
     *       a container other than the default container is present.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     */
    Status addTenant(VTenantPath path, VTenantConfig tconf);

    /**
     * Modify the configuration of the existing
     * {@linkplain <a href="package-summary.html#VTN">VTN</a>}.
     *
     * @param path   A {@link VTenantPath} object that specifies the position
     *               of the VTN to be modified.
     * @param tconf  A {@link VTenantConfig} object which contains the VTN
     *               configuration information to be applied.
     * @param all
     *   A boolean value to determine the treatment of attributes for which
     *   value has not been configured inside {@code tconf}.
     *   <ul>
     *     <li>
     *       If {@code true} is specified, all the attributes related to VTN
     *       are modified.
     *       <ul>
     *         <li>
     *           The description of the VTN will be deleted if {@code null} is
     *           configured in {@code tconf} for the
     *           {@linkplain VTenantConfig#getDescription() description}.
     *         </li>
     *         <li>
     *           {@link VTenantConfig#getIdleTimeout() idle_timeout} of flow
     *           entry will be treated as <strong>300</strong> if it is not
     *           configured in {@code tconf}.
     *         </li>
     *         <li>
     *           {@link VTenantConfig#getHardTimeout() hard_timeout} of flow
     *           entry will be treated as <strong>0</strong> if it is not
     *           configured in {@code tconf}.
     *         </li>
     *       </ul>
     *     </li>
     *     <li>
     *       If {@code false} is specified, attributes with no value
     *       configured in {@code tconf} is not modified.
     *     </li>
     *   </ul>
     * @return
     *   A {@link Status} object which represents the result of the operation
     *   is returned.
     *   <p>
     *     Upon successful completion,
     *     <strong>{@code StatusCode.SUCCESS}</strong> is set in a returned
     *     object. Otherwise a {@code StatusCode} which indicates the cause
     *     of error is set in a returned {@link Status} object.
     *     The following are the main {@code StatusCode} configured in
     *     {@link Status}.
     *   </p>
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.BADREQUEST}
     *     <dd>
     *       <ul style="padding-left: 1em;">
     *         <li>
     *           {@code null} is passed to {@code path} or {@code tconf}.
     *         </li>
     *         <li>
     *           {@code null} is configured in {@code path} for the
     *           {@linkplain VTenantPath#getTenantName() VTN name}.
     *         </li>
     *         <li>
     *           Incorrect value is configured in {@code tconf} for
     *           {@link VTenantConfig#getIdleTimeout() idle_timeout} or
     *           {@link VTenantConfig#getHardTimeout() hard_timeout}.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>VTN specified by {@code path} does not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with the default container, and
     *       a container other than the default container is present.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     */
    Status modifyTenant(VTenantPath path, VTenantConfig tconf, boolean all);

    /**
     * Remove the specified
     * {@linkplain <a href="package-summary.html#VTN">VTN</a>}.
     *
     * <p>
     *   All the virtual networking node in the specified VTN, such as
     *   {@linkplain <a href="package-summary.html#vBridge">vBridge</a>},
     *   will also be removed.
     * </p>
     *
     * @param path  A {@link VTenantPath} object that specifies the position
     *              of the VTN to be removed.
     * @return
     *   A {@link Status} object which represents the result of the operation
     *   is returned.
     *   <p>
     *     Upon successful completion,
     *     <strong>{@code StatusCode.SUCCESS}</strong> is set in a returned
     *     object. Otherwise a {@code StatusCode} which indicates the cause
     *     of error is set in a returned {@link Status} object.
     *     The following are the main {@code StatusCode} configured in
     *     {@link Status}.
     *   </p>
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.BADREQUEST}
     *     <dd>
     *       <ul style="padding-left: 1em;">
     *         <li>{@code null} is passed to {@code path}.</li>
     *         <li>
     *           {@code null} is configured in {@code path} for the
     *           {@linkplain VTenantPath#getTenantName() VTN name}.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>VTN specified by {@code path} does not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with the default container, and
     *       a container other than the default container is present.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     */
    Status removeTenant(VTenantPath path);

    /**
     * Return a list of {@link VBridge} objects corresponding to all the
     * {@linkplain <a href="package-summary.html#vBridge">vBridges</a>} inside
     * the specified {@linkplain <a href="package-summary.html#VTN">VTN</a>}.
     *
     * @param path  A {@link VTenantPath} object that specifies the position
     *              of the VTN.
     * @return  A list of {@link VBridge} objects corresponding to all the
     *          vBridges present inside the VTN specified by {@code path}.
     *          An empty list is returned if no vBridge is present inside
     *          the VTN.
     * @throws VTNException  An error occurred.
     *   The following are the main {@code StatusCode} set in {@link Status}
     *   delivered by the exception.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.BADREQUEST}
     *     <dd>
     *       <ul style="padding-left: 1em;">
     *         <li>{@code null} is passed to {@code path}.</li>
     *         <li>
     *           {@code null} is configured in {@code path} for the
     *           {@linkplain VTenantPath#getTenantName() VTN name}.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>VTN specified by {@code path} does not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     */
    List<VBridge> getBridges(VTenantPath path) throws VTNException;

    /**
     * Return information about the specified
     * {@linkplain <a href="package-summary.html#vBridge">vBridge</a>}.
     *
     * @param path  A {@link VBridgePath} object that specifies the position
     *              of the vBridge.
     * @return  A {@link VBridge} object which represents information about
     *          the vBridge specified by {@code path}.
     * @throws VTNException  An error occurred.
     *   The following are the main {@code StatusCode} set in {@link Status}
     *   delivered by the exception.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.BADREQUEST}
     *     <dd>
     *       <ul style="padding-left: 1em;">
     *         <li>{@code null} is passed to {@code path}.</li>
     *         <li>
     *           {@code null} is configured in {@code path} for the
     *           {@linkplain VTenantPath#getTenantName() VTN name} or
     *           {@linkplain VBridgePath#getBridgeName() vBridge name}.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>{@linkplain <a href="package-summary.html#VTN">VTN</a>} or
     *         vBridge specified by {@code path} does not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     */
    VBridge getBridge(VBridgePath path) throws VTNException;

    /**
     * Create a new
     * {@linkplain <a href="package-summary.html#vBridge">vBridge</a>} inside
     * the specified {@linkplain <a href="package-summary.html#VTN">VTN</a>}.
     *
     * @param path
     *   A {@link VBridgePath} object that specifies the position of new
     *   vBridge.
     *   <p style="margin-left: 1em;">
     *     The name of the vBridge inside {@link VBridgePath} must be a string
     *     that meets the following conditions.
     *   </p>
     *   <ul>
     *     <li>
     *       The length of the name must be greater than <strong>0</strong>
     *       and less than <strong>32</strong>.
     *     </li>
     *     <li>
     *       The name must consist of US-ASCII alphabets, numbers, and
     *       underscore ({@code '_'}).
     *     </li>
     *     <li>
     *       The name must start with an US-ASCII alphabet or number.
     *     </li>
     *   </ul>
     * @param bconf
     *   A {@link VBridgeConfig} object which specifies the vBridge
     *   configuration information.
     *   <ul>
     *     <li>
     *       The description of the vBridge is not registered if {@code null}
     *       is configured in {@code bconf} for the
     *       {@linkplain VBridgeConfig#getDescription() description}.
     *     </li>
     *     <li>
     *       {@linkplain VBridgeConfig#getAgeInterval() Aging interval} of
     *       {@linkplain <a href="package-summary.html#macTable">MAC address table</a>}
     *       is treated as <strong>600</strong> if it is not configured in
     *       {@code bconf}.
     *     </li>
     *   </ul>
     * @return
     *   A {@link Status} object which represents the result of the operation
     *   is returned.
     *   <p>
     *     Upon successful completion,
     *     <strong>{@code StatusCode.SUCCESS}</strong> is set in a returned
     *     object. Otherwise a {@code StatusCode} which indicates the cause
     *     of error is set in a returned {@link Status} object.
     *     The following are the main {@code StatusCode} configured in
     *     {@link Status}.
     *   </p>
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.BADREQUEST}
     *     <dd>
     *       <ul style="padding-left: 1em;">
     *         <li>
     *           {@code null} is passed to {@code path} or {@code bconf}.
     *         </li>
     *         <li>
     *           {@code null} is configured in {@code path} for the
     *           {@linkplain VTenantPath#getTenantName() VTN name}.
     *         </li>
     *         <li>
     *           Incorrect {@linkplain VBridgePath#getBridgeName() vBridge name}
     *           is configured in {@code path}.
     *         </li>
     *         <li>
     *           Incorrect value is configured in {@code bconf} for
     *           {@linkplain VBridgeConfig#getAgeInterval() aging interval} of
     *           MAC address table.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>{@linkplain <a href="package-summary.html#VTN">VTN</a>}
     *         specified by {@code path} does not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.CONFLICT}
     *     <dd>vBridge specified by {@code path} already exists.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with the default container, and
     *       a container other than the default container is present.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     */
    Status addBridge(VBridgePath path, VBridgeConfig bconf);

    /**
     * Modify the configuration of the existing
     * {@linkplain <a href="package-summary.html#vBridge">vBridge</a>}.
     *
     * @param path   A {@link VBridgePath} object that specifies the position
     *               of the vBridge to be modified.
     * @param bconf  A {@link VBridgeConfig} object which contains the vBridge
     *               configuration information to be applied.
     * @param all
     *   A boolean value to determine the treatment of attributes for which
     *   value has not been configured inside {@code bconf}.
     *   <ul>
     *     <li>
     *       If {@code true} is specified, all the attributes related to
     *       vBridge are modified.
     *       <ul>
     *         <li>
     *           The description of the vBridge will be deleted if {@code null}
     *           is configured in {@code bconf} for the
     *           {@linkplain VBridgeConfig#getDescription() description}.
     *         </li>
     *         <li>
     *           {@linkplain VBridgeConfig#getAgeInterval() Aging interval} of
     *           {@linkplain <a href="package-summary.html#macTable">MAC address table</a>}
     *           is treated as <strong>600</strong> if it is not configured in
     *           {@code bconf}.
     *         </li>
     *       </ul>
     *     </li>
     *     <li>
     *       If {@code false} is specified, attributes with no value
     *       configured in {@code bconf} is not modified.
     *     </li>
     *   </ul>
     * @return
     *   A {@link Status} object which represents the result of the operation
     *   is returned.
     *   <p>
     *     Upon successful completion,
     *     <strong>{@code StatusCode.SUCCESS}</strong> is set in a returned
     *     object. Otherwise a {@code StatusCode} which indicates the cause
     *     of error is set in a returned {@link Status} object.
     *     The following are the main {@code StatusCode} configured in
     *     {@link Status}.
     *   </p>
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.BADREQUEST}
     *     <dd>
     *       <ul style="padding-left: 1em;">
     *         <li>
     *           {@code null} is passed to {@code path} or {@code bconf}.
     *         </li>
     *         <li>
     *           {@code null} is configured in {@code path} for the
     *           {@linkplain VTenantPath#getTenantName() VTN name} or
     *           {@linkplain VBridgePath#getBridgeName() vBridge name}.
     *         </li>
     *         <li>
     *           Incorrect value is configured in {@code bconf} for
     *           {@linkplain VBridgeConfig#getAgeInterval() aging interval} of
     *           MAC address table.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>{@linkplain <a href="package-summary.html#VTN">VTN</a>} or
     *         vBridge specified by {@code path} does not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with the default container, and
     *       a container other than the default container is present.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     */
    Status modifyBridge(VBridgePath path, VBridgeConfig bconf, boolean all);

    /**
     * Remove the specified
     * {@linkplain <a href="package-summary.html#vBridge">vBridge</a>}.
     *
     * <p>
     *   All the
     *   {@linkplain <a href="package-summary.html#vInterface">virtual interfaces</a>}
     *   inside the specified vBridge will also be removed.
     * </p>
     *
     * @param path  A {@link VBridgePath} object that specifies the position
     *              of the vBridge to be removed.
     * @return
     *   A {@link Status} object which represents the result of the operation
     *   is returned.
     *   <p>
     *     Upon successful completion,
     *     <strong>{@code StatusCode.SUCCESS}</strong> is set in a returned
     *     object. Otherwise a {@code StatusCode} which indicates the cause
     *     of error is set in a returned {@link Status} object.
     *     The following are the main {@code StatusCode} configured in
     *     {@link Status}.
     *   </p>
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.BADREQUEST}
     *     <dd>
     *       <ul style="padding-left: 1em;">
     *         <li>{@code null} is passed to {@code path}.</li>
     *         <li>
     *           {@code null} is configured in {@code path} for the
     *           {@linkplain VTenantPath#getTenantName() VTN name} or
     *           {@linkplain VBridgePath#getBridgeName() vBridge name}.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>{@linkplain <a href="package-summary.html#VTN">VTN</a>} or
     *         vBridge specified by {@code path} does not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with the default container, and
     *       a container other than the default container is present.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     */
    Status removeBridge(VBridgePath path);

    /**
     * Return a list of {@link VInterface} objects corresponding to all the
     * {@linkplain <a href="package-summary.html#vInterface">virtual interfaces</a>}
     * inside the specified vBridge.
     *
     * @param path
     *   A {@link VBridgePath} object that specifies the position of the
     *   {@linkplain <a href="package-summary.html#vBridge">vBridge</a>}.
     * @return  A list of {@link VInterface} objects corresponding to all the
     *          virtual interfaces present inside the vBridge specified by
     *          {@code path}.
     *          An empty list is returned if no virtual interface is present
     *          inside the vBridge.
     * @throws VTNException  An error occurred.
     *   The following are the main {@code StatusCode} set in {@link Status}
     *   delivered by the exception.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.BADREQUEST}
     *     <dd>
     *       <ul style="padding-left: 1em;">
     *         <li>{@code null} is passed to {@code path}.</li>
     *         <li>
     *           {@code null} is configured in {@code path} for the
     *           {@linkplain VTenantPath#getTenantName() VTN name} or
     *           {@linkplain VBridgePath#getBridgeName() vBridge name}.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>{@linkplain <a href="package-summary.html#VTN">VTN</a>} or
     *         vBridge specified by {@code path} does not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     */
    List<VInterface> getBridgeInterfaces(VBridgePath path)
        throws VTNException;

    /**
     * Return information about the specified
     * {@linkplain <a href="package-summary.html#vInterface">virtual interface</a>}
     * in the {@linkplain <a href="package-summary.html#vBridge">vBridge</a>}.
     *
     * @param path  A {@link VBridgeIfPath} object that specifies the position
     *              of the vBridge interface.
     * @return  A {@link VInterface} object which represents information about
     *          the vBridge interface specified by {@code path}.
     * @throws VTNException  An error occurred.
     *   The following are the main {@code StatusCode} set in {@link Status}
     *   delivered by the exception.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.BADREQUEST}
     *     <dd>
     *       <ul style="padding-left: 1em;">
     *         <li>{@code null} is passed to {@code path}.</li>
     *         <li>
     *           {@code null} is configured in {@code path} for the
     *           {@linkplain VTenantPath#getTenantName() VTN name} or
     *           {@linkplain VBridgePath#getBridgeName() vBridge name} or
     *           {@linkplain VBridgeIfPath#getInterfaceName() interface name}.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>
     *       {@linkplain <a href="package-summary.html#VTN">VTN</a>} or
     *       vBridge or vBridge interface specified by {@code path} does not
     *       exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     */
    VInterface getBridgeInterface(VBridgeIfPath path) throws VTNException;

    /**
     * Create a new
     * {@linkplain <a href="package-summary.html#vInterface">virtual interface</a>}
     * inside the specified
     * {@linkplain <a href="package-summary.html#vBridge">vBridge</a>}.
     *
     * @param path
     *   A {@link VBridgeIfPath} object that specifies the position of new
     *   vBridge interface.
     *   <p style="margin-left: 1em;">
     *     The name of the vBridge interface inside {@link VBridgeIfPath} must
     *     be a string that meets the following conditions.
     *   </p>
     *   <ul>
     *     <li>
     *       The length of the name must be greater than <strong>0</strong>
     *       and less than <strong>32</strong>.
     *     </li>
     *     <li>
     *       The name must consist of US-ASCII alphabets, numbers, and
     *       underscore ({@code '_'}).
     *     </li>
     *     <li>
     *       The name must start with an US-ASCII alphabet or number.
     *     </li>
     *   </ul>
     * @param iconf
     *   A {@link VInterfaceConfig} object which specifies the vBridge
     *   interface configuration information.
     *   <ul>
     *     <li>
     *       The description of the vBridge interface is not registered if
     *       {@code null} is configured in {@code iconf} for the
     *       {@linkplain VInterfaceConfig#getDescription() description}.
     *     </li>
     *     <li>
     *       {@linkplain VInterfaceConfig#getEnabled() Enable/disable configuration}
     *       is treated as {@link Boolean#TRUE} if it is not configured in
     *       {@code iconf}.
     *     </li>
     *   </ul>
     * @return
     *   A {@link Status} object which represents the result of the operation
     *   is returned.
     *   <p>
     *     Upon successful completion,
     *     <strong>{@code StatusCode.SUCCESS}</strong> is set in a returned
     *     object. Otherwise a {@code StatusCode} which indicates the cause
     *     of error is set in a returned {@link Status} object.
     *     The following are the main {@code StatusCode} configured in
     *     {@link Status}.
     *   </p>
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.BADREQUEST}
     *     <dd>
     *       <ul style="padding-left: 1em;">
     *         <li>
     *           {@code null} is passed to {@code path} or {@code iconf}.
     *         </li>
     *         <li>
     *           {@code null} is configured in {@code path} for the
     *           {@linkplain VTenantPath#getTenantName() VTN name} or
     *           {@linkplain VBridgePath#getBridgeName() vBridge name}.
     *         </li>
     *         <li>
     *           Incorrect
     *           {@linkplain VBridgeIfPath#getInterfaceName() interface name}
     *           is configured in {@code path}.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>{@linkplain <a href="package-summary.html#VTN">VTN</a>} or
     *         vBridge specified by {@code path} does not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.CONFLICT}
     *     <dd>vBridge interface specified by {@code path} already exists.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with the default container, and
     *       a container other than the default container is present.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     */
    Status addBridgeInterface(VBridgeIfPath path, VInterfaceConfig iconf);

    /**
     * Modify the configuration of the existing
     * {@linkplain <a href="package-summary.html#vInterface">virtual interface</a>}
     * in the specified
     * {@linkplain <a href="package-summary.html#vBridge">vBridge</a>}.
     *
     * @param path   A {@link VBridgeIfPath} that specifies the position
     *               of the vBridge interface to be modified.
     * @param iconf  A {@link VInterfaceConfig} object which contains the
     *               vBridge interface configuration information to be applied.
     * @param all
     *   A boolean value to determine the treatment of attributes for which
     *   value has not been configured inside {@code iconf}.
     *   <ul>
     *     <li>
     *       If {@code true} is specified, all the attributes related to
     *       vBridge interface are modified.
     *       <ul>
     *         <li>
     *           The description of the vBridge interface will be deleted if
     *           {@code null} is configured in {@code iconf} for the
     *           {@linkplain VInterfaceConfig#getDescription() description}.
     *         </li>
     *         <li>
     *           {@linkplain VInterfaceConfig#getEnabled() Enable/disable configuration}
     *           is treated as {@link Boolean#TRUE} if it is not configured
     *           in {@code iconf}.
     *         </li>
     *       </ul>
     *     </li>
     *     <li>
     *       If {@code false} is specified, attributes with no value
     *       configured in {@code iconf} is not modified.
     *     </li>
     *   </ul>
     * @return
     *   A {@link Status} object which represents the result of the operation
     *   is returned.
     *   <p>
     *     Upon successful completion,
     *     <strong>{@code StatusCode.SUCCESS}</strong> is set in a returned
     *     object. Otherwise a {@code StatusCode} which indicates the cause
     *     of error is set in a returned {@link Status} object.
     *     The following are the main {@code StatusCode} configured in
     *     {@link Status}.
     *   </p>
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.BADREQUEST}
     *     <dd>
     *       <ul style="padding-left: 1em;">
     *         <li>
     *           {@code null} is passed to {@code path} or {@code iconf}.
     *         </li>
     *         <li>
     *           {@code null} is configured in {@code path} for the
     *           {@linkplain VTenantPath#getTenantName() VTN name} or
     *           {@linkplain VBridgePath#getBridgeName() vBridge name} or
     *           {@linkplain VBridgeIfPath#getInterfaceName() interface name}.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>
     *       {@linkplain <a href="package-summary.html#VTN">VTN</a>} or vBridge
     *       or vBridge interface specified by {@code path} does not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with the default container, and
     *       a container other than the default container is present.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     */
    Status modifyBridgeInterface(VBridgeIfPath path, VInterfaceConfig iconf,
                                 boolean all);

    /**
     * Remove the specified
     * {@linkplain <a href="package-summary.html#vInterface">virtual interface</a>}
     * from the {@linkplain <a href="package-summary.html#vBridge">vBridge</a>}.
     *
     * @param path  A {@link VBridgeIfPath} object that specifies the position
     *              of the vBridge interface to be removed.
     * @return
     *   A {@link Status} object which represents the result of the operation
     *   is returned.
     *   <p>
     *     Upon successful completion,
     *     <strong>{@code StatusCode.SUCCESS}</strong> is set in a returned
     *     object. Otherwise a {@code StatusCode} which indicates the cause
     *     of error is set in a returned {@link Status} object.
     *     The following are the main {@code StatusCode} configured in
     *     {@link Status}.
     *   </p>
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.BADREQUEST}
     *     <dd>
     *       <ul style="padding-left: 1em;">
     *         <li>{@code null} is passed to {@code path}.</li>
     *         <li>
     *           {@code null} is configured in {@code path} for the
     *           {@linkplain VTenantPath#getTenantName() VTN name} or
     *           {@linkplain VBridgePath#getBridgeName() vBridge name} or
     *           {@linkplain VBridgeIfPath#getInterfaceName() interface name}.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>
     *       {@linkplain <a href="package-summary.html#VTN">VTN</a>} or
     *       vBridge or vBridge interface specified by {@code path}
     *       does not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with the default container, and
     *       a container other than the default container is present.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     */
    Status removeBridgeInterface(VBridgeIfPath path);

    /**
     * Return a list of {@link VlanMap} objects corresponding to all the
     * {@linkplain <a href="package-summary.html#VLAN-map">VLAN mappings</a>}
     * configured in the specified
     * {@linkplain <a href="package-summary.html#vBridge">vBridge</a>}.
     *
     * @param path  A {@link VBridgePath} object that specifies the position
     *              of the vBridge.
     * @return  A list of {@link VlanMap} objects corresponding to all the
     *          VLAN mappings configured in the vBridge specified by
     *          {@code path}.
     *          An empty list is returned if no VLAN mapping is configured in
     *          the specified vBridge.
     * @throws VTNException  An error occurred.
     *   The following are the main {@code StatusCode} set in {@link Status}
     *   delivered by the exception.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.BADREQUEST}
     *     <dd>
     *       <ul style="padding-left: 1em;">
     *         <li>{@code null} is passed to {@code path}.</li>
     *         <li>
     *           {@code null} is configured in {@code path} for the
     *           {@linkplain VTenantPath#getTenantName() VTN name} or
     *           {@linkplain VBridgePath#getBridgeName() vBridge name}.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>{@linkplain <a href="package-summary.html#VTN">VTN</a>} or
     *         vBridge specified by {@code path} does not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     */
    List<VlanMap> getVlanMaps(VBridgePath path) throws VTNException;

    /**
     * Return information about the specified
     * {@linkplain <a href="package-summary.html#VLAN-map">VLAN mapping</a>} in the
     * {@linkplain <a href="package-summary.html#vBridge">vBridge</a>}.
     *
     * @param path  A {@link VBridgePath} object that specifies the position
     *              of the vBridge.
     * @param mapId  The identifier of the VLAN mapping assigned by the
     *               VTN Manager.
     * @return  A {@link VlanMap} object which represents information about
     *          the VLAN mapping specified by {@code path} and {@code mapId}.
     * @throws VTNException  An error occurred.
     *   The following are the main {@code StatusCode} set in {@link Status}
     *   delivered by the exception.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.BADREQUEST}
     *     <dd>
     *       <ul style="padding-left: 1em;">
     *         <li>
     *           {@code null} is passed to {@code path} or {@code mapId}.
     *         </li>
     *         <li>
     *           {@code null} is configured in {@code path} for the
     *           {@linkplain VTenantPath#getTenantName() VTN name} or
     *           {@linkplain VBridgePath#getBridgeName() vBridge name}.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>
     *       <ul style="padding-left: 1em;">
     *         <li>
     *           {@linkplain <a href="package-summary.html#VTN">VTN</a>} or
     *           vBridge specified by {@code path} does not exist.
     *         </li>
     *         <li>
     *           VLAN mapping specified by {@code mapId} is not configured
     *           in the vBridge specified by {@code path}.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     */
    VlanMap getVlanMap(VBridgePath path, String mapId) throws VTNException;

    /**
     * Search for a
     * {@linkplain <a href="package-summary.html#VLAN-map">VLAN mapping</a>}
     * with the specified configuration information in the specified
     * {@linkplain <a href="package-summary.html#vBridge">vBridge</a>}.
     *
     * <p>
     *   If any VLAN mapping with the configuration information that exactly
     *   matches with {@code vlconf} is present in the vBridge specified by
     *   {@code path}, information about that VLAN mapping is returned.
     * </p>
     *
     * @param path    A {@link VBridgePath} object that specifies the position
     *                of the vBridge.
     * @param vlconf  A {@link VlanMapConfig} object which contains the VLAN
     *                mapping configuration information.
     * @return  A {@link VlanMap} object which represents information about
     *          the VLAN mapping which matches the specified VLAN mapping
     *          configuration information.
     * @throws VTNException  An error occurred.
     *   The following are the main {@code StatusCode} set in {@link Status}
     *   delivered by the exception.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.BADREQUEST}
     *     <dd>
     *       <ul style="padding-left: 1em;">
     *         <li>
     *           {@code null} is passed to {@code path} or {@code vlconf}.
     *         </li>
     *         <li>
     *           {@code null} is configured in {@code path} for the
     *           {@linkplain VTenantPath#getTenantName() VTN name} or
     *           {@linkplain VBridgePath#getBridgeName() vBridge name}.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>
     *       <ul style="padding-left: 1em;">
     *         <li>
     *           {@linkplain <a href="package-summary.html#VTN">VTN</a>} or
     *           vBridge specified by {@code path} does not exist.
     *         </li>
     *         <li>
     *           There is no VLAN mapping in the vBridge specified by
     *           {@code path} where configuration information exactly matches
     *           with {@code vlconf}.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     */
    VlanMap getVlanMap(VBridgePath path, VlanMapConfig vlconf)
        throws VTNException;

    /**
     * Configure
     * {@linkplain <a href="package-summary.html#VLAN-map">VLAN mapping</a>}
     * in the specified
     * {@linkplain <a href="package-summary.html#vBridge">vBridge</a>}.
     *
     * <p>
     *   VLAN network specified by {@code vlconf} will get mapped to the
     *   vBridge specified by {@code path}.
     * </p>
     * <ul>
     *   <li>
     *     It is possible to configure a
     *     {@link org.opendaylight.controller.sal.core.Node} object
     *     corresponding to physical switch that is to be the target of
     *     the VLAN mapping.
     *     <ul>
     *       <li>
     *         Currently, it is possible to configure only the
     *         {@link org.opendaylight.controller.sal.core.Node} objects that
     *         correspond to OpenFlow switches as the target.
     *       </li>
     *       <li>
     *         VLAN mapping configuration will succeed even if the physical
     *         switch corresponding to the specified
     *         {@link org.opendaylight.controller.sal.core.Node} object does
     *         not exist. VLAN mapping will come into effect whenever,
     *         at a later point in time, the specified physical switch is
     *         found.
     *       </li>
     *       <li>
     *         All the physical switches managed by the OpenDaylight controller
     *         will be a target of VLAN mapping if the
     *         {@link org.opendaylight.controller.sal.core.Node} object is not
     *         configured.
     *       </li>
     *     </ul>
     *   </li>
     *   <li>
     *     The VLAN network is mapped to the vBridge according to the
     *     VLAN ID configured in {@code vlconf}.
     *     <ul>
     *       <li>
     *         If a value between <strong>1</strong> or more and
     *         <strong>4095</strong> or less is configured, then the ethernet
     *         frames that have this VLAN ID configured will get mapped to
     *         the vBridge.
     *       </li>
     *       <li>
     *         If <strong>0</strong> is configured, untagged ethernet frames
     *         will get mapped to the vBridge.
     *       </li>
     *     </ul>
     *   </li>
     * </ul>
     *
     * @param path    A {@link VBridgePath} object that specifies the position
     *                of the vBridge.
     * @param vlconf  A {@link VlanMapConfig} object which contains the VLAN
     *                mapping configuration information.
     * @return  A {@link VlanMap} object which contains information about the
     *          VLAN mapping that was configured.
     *          The identifier of new VLAN mapping can be obtained by the call
     *          of {@link VlanMap#getId()}.
     * @throws VTNException  An error occurred.
     *   The following are the main {@code StatusCode} set in {@link Status}
     *   delivered by the exception.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.BADREQUEST}
     *     <dd>
     *       <ul style="padding-left: 1em;">
     *         <li>
     *           {@code null} is passed to {@code path} or {@code vlconf}.
     *         </li>
     *         <li>
     *           {@code null} is configured in {@code path} for the
     *           {@linkplain VTenantPath#getTenantName() VTN name} or
     *           {@linkplain VBridgePath#getBridgeName() vBridge name}.
     *         </li>
     *         <li>
     *           Incorrect {@link org.opendaylight.controller.sal.core.Node}
     *           object is configured in {@code vlconf}.
     *         </li>
     *         <li>
     *           Incorrect VLAN ID is configured in {@code vlconf}.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>
     *       {@linkplain <a href="package-summary.html#VTN">VTN</a>} or vBridge
     *       specified by {@code path} does not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.CONFLICT}
     *     <dd>
     *       VLAN network specified by {@code vlconf} is already mapped to
     *       the specified vBridge or another vBridge.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with the default container, and
     *       a container other than the default container is present.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     */
    VlanMap addVlanMap(VBridgePath path, VlanMapConfig vlconf)
        throws VTNException;

    /**
     * Remove the specified
     * {@linkplain <a href="package-summary.html#VLAN-map">VLAN mapping</a>}
     * from the {@linkplain <a href="package-summary.html#vBridge">vBridge</a>}.
     *
     * @param path   A {@link VBridgePath} object that specifies the position
     *               of the vBridge.
     * @param mapId  The identifier of the VLAN mapping to be removed.
     * @return
     *   A {@link Status} object which represents the result of the operation
     *   is returned.
     *   <p>
     *     Upon successful completion,
     *     <strong>{@code StatusCode.SUCCESS}</strong> is set in a returned
     *     object. Otherwise a {@code StatusCode} which indicates the cause
     *     of error is set in a returned {@link Status} object.
     *     The following are the main {@code StatusCode} configured in
     *     {@link Status}.
     *   </p>
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.BADREQUEST}
     *     <dd>
     *       <ul style="padding-left: 1em;">
     *         <li>
     *           {@code null} is passed to {@code path} or {@code mapId}.
     *         </li>
     *         <li>
     *           {@code null} is configured in {@code path} for the
     *           {@linkplain VTenantPath#getTenantName() VTN name} or
     *           {@linkplain VBridgePath#getBridgeName() vBridge name}.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>
     *       <ul style="padding-left: 1em;">
     *         <li>
     *           {@linkplain <a href="package-summary.html#VTN">VTN</a>} or
     *           vBridge specified by {@code path} does not exist.
     *         </li>
     *         <li>
     *           VLAN mapping specified by {@code mapId} is not configured
     *           in the vBridge specified by {@code path}.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with the default container, and
     *       a container other than the default container is present.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     */
    Status removeVlanMap(VBridgePath path, String mapId);

    /**
     * Return information about the
     * {@linkplain <a href="package-summary.html#port-map">port mapping</a>}
     * configured in the specified
     * {@linkplain <a href="package-summary.html#vInterface">virtual interface</a>}
     * in the {@linkplain <a href="package-summary.html#vBridge">vBridge</a>}.
     *
     * @param path  A {@link VBridgeIfPath} object that specifies the position
     *              of the vBridge interface.
     * @return  A {@link PortMap} object which contains the port mapping
     *          information is returned if the port mapping is configured
     *          in the vBridge interface specified by {@code path}.
     *          {@code null} is returned if the port mapping is not configured
     *          in the specified vBridge interface.
     * @throws VTNException  An error occurred.
     *   The following are the main {@code StatusCode} set in {@link Status}
     *   delivered by the exception.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.BADREQUEST}
     *     <dd>
     *       <ul style="padding-left: 1em;">
     *         <li>{@code null} is passed to {@code path}.</li>
     *         <li>
     *           {@code null} is configured in {@code path} for the
     *           {@linkplain VTenantPath#getTenantName() VTN name} or
     *           {@linkplain VBridgePath#getBridgeName() vBridge name} or
     *           {@linkplain VBridgeIfPath#getInterfaceName() interface name}.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>
     *       {@linkplain <a href="package-summary.html#VTN">VTN</a>} or vBridge or
     *       vBridge interface specified by {@code path}
     *       does not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     */
    PortMap getPortMap(VBridgeIfPath path) throws VTNException;

    /**
     * Configure
     * {@linkplain <a href="package-summary.html#port-map">port mapping</a>}
     * in the specified
     * {@linkplain <a href="package-summary.html#vInterface">virtual interface</a>}
     * in the {@linkplain <a href="package-summary.html#vBridge">vBridge</a>}.
     *
     * <p>
     *   If port mapping is already configured in the specified vBridge
     *   interface, the specified settings are applied after old configuration
     *   is deleted.
     * </p>
     * <p>
     *   If a non-{@code null} value is passed to {@code pmconf}, then
     *   ethernet frame that flows through the port of the physical switch
     *   specified by {@code pmconf} will get mapped to the vBridge interface
     *   specified by {@code path}.
     * </p>
     * <ul>
     *   <li>
     *     {@code pmconf} must contain a
     *     {@link org.opendaylight.controller.sal.core.Node} object
     *     corresponding to the physical switch that you want to map to the
     *     vBridge interface.
     *     <ul>
     *       <li>
     *         Currently, it is possible to configure only the
     *         {@link org.opendaylight.controller.sal.core.Node} objects that
     *         correspond to OpenFlow switches.
     *       </li>
     *     </ul>
     *   </li>
     *   <li>
     *     {@code pmconf} must contain a {@link SwitchPort} object that
     *     specifies the position of the physical switch port that you want to
     *     map to the vBridge interface.
     *     <ul>
     *       <li>
     *         Currently, it is possible to configure only the physical ports
     *         of OpenFlow switches.
     *       </li>
     *       <li>
     *         Port mapping configuration will succeed even if the specified
     *         physical port of the switch does not exist. Port mapping will
     *         come into effect whenever, at a later point in time, the
     *         specified physical port is found.
     *         <ul>
     *           <li>
     *             However, when mapping is actually done with the specified
     *             physical port, the port mapping will not be established if
     *             that physical port and VLAN ID are mapped to another
     *             vBridge interface.
     *           </li>
     *         </ul>
     *       </li>
     *     </ul>
     *   </li>
     *   <li>
     *     The VLAN network is mapped to the vBridge interface according to
     *     the VLAN ID configured in {@code pmconf}.
     *     <ul>
     *       <li>
     *         If a value between <strong>1</strong> or more and
     *         <strong>4095</strong> or less is configured, then the ethernet
     *         frames that have this VLAN ID configured will get mapped to
     *         the vBridge interface.
     *       </li>
     *       <li>
     *         If <strong>0</strong> is configured, untagged ethernet frames
     *         will get mapped to the vBridge interface.
     *       </li>
     *     </ul>
     *   </li>
     *   <li>
     *     This method does nothing if port mapping with the same configuration
     *     information as {@code pmconf} is already configured in the vBridge
     *     interface specified by {@code path}.
     *   </li>
     * </ul>
     *
     * @param path    A {@link VBridgeIfPath} object that specifies the
     *                position of the vBridge interface.
     * @param pmconf
     *   A {@link PortMapConfig} object which contains the port mapping
     *   configuration information.
     *   <p style="margin-left: 1em;">
     *     If {@code null} is specified, port mapping in the specified vBridge
     *     interface is removed. In this case this method will succeed even
     *     if no port mapping is configured to the specified vBridge interface.
     *   </p>
     * @return
     *   A {@link Status} object which represents the result of the operation
     *   is returned.
     *   <p>
     *     Upon successful completion,
     *     <strong>{@code StatusCode.SUCCESS}</strong> is set in a returned
     *     object. Otherwise a {@code StatusCode} which indicates the cause
     *     of error is set in a returned {@link Status} object.
     *     The following are the main {@code StatusCode} configured in
     *     {@link Status}.
     *   </p>
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.BADREQUEST}
     *     <dd>
     *       <ul style="padding-left: 1em;">
     *         <li>{@code null} is passed to {@code path}.</li>
     *         <li>
     *           {@code null} is configured in {@code path} for the
     *           {@linkplain VTenantPath#getTenantName() VTN name} or
     *           {@linkplain VBridgePath#getBridgeName() vBridge name} or
     *           {@linkplain VBridgeIfPath#getInterfaceName() interface name}.
     *         </li>
     *         <li>
     *           {@code pmconf} is not {@code null}, and
     *           {@link org.opendaylight.controller.sal.core.Node} or
     *           {@link SwitchPort} object is not configured in that.
     *         </li>
     *         <li>
     *           {@code pmconf} is not {@code null}, and
     *           incorrect value is configured in that for
     *           {@link org.opendaylight.controller.sal.core.Node} or
     *           {@link SwitchPort} object.
     *         </li>
     *         <li>
     *           {@code pmconf} is not {@code null}, and incorrect VLAN ID is
     *           configured in that.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>
     *       {@linkplain <a href="package-summary.html#VTN">VTN</a>} or vBridge
     *       or vBridge interface specified by {@code path} does not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.CONFLICT}
     *     <dd>
     *       Physical switch port specified by {@code pmconf} exists, and
     *       the specified combination of physical port and VLAN ID is mapped
     *       to another vBridge interface.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with the default container, and
     *       a container other than the default container is present.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     */
    Status setPortMap(VBridgeIfPath path, PortMapConfig pmconf);

    /**
     * Return information about
     * {@linkplain <a href="package-summary.html#MAC-map">MAC mapping</a>}.
     * configured in the specified
     * {@linkplain <a href="package-summary.html#vBridge">vBridge</a>}.
     *
     * @param path  A {@link VBridgePath} object that specifies the position
     *              of the vBridge.
     * @return  A {@link MacMap} object which represents MAC mapping
     *          information configured in the vBridge specified by
     *          {@code path}.
     *          {@code null} is returned if MAC mapping is not configured
     *          in the specified vBridge.
     * @throws VTNException  An error occurred.
     *   The following are the main {@code StatusCode} set in {@link Status}
     *   delivered by the exception.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.BADREQUEST}
     *     <dd>
     *       <ul style="padding-left: 1em;">
     *         <li>{@code null} is passed to {@code path}.</li>
     *         <li>
     *           {@code null} is configured in {@code path} for the
     *           {@linkplain VTenantPath#getTenantName() VTN name} or
     *           {@linkplain VBridgePath#getBridgeName() vBridge name}.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>
     *       {@linkplain <a href="package-summary.html#VTN">VTN</a>} or vBridge
     *       specified by {@code path} does not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    MacMap getMacMap(VBridgePath path) throws VTNException;

    /**
     * Return a list of host information configured in access control list
     * of
     * {@linkplain <a href="package-summary.html#MAC-map">MAC mapping</a>}.
     *
     * <p>
     *   This method returns a set which contains all host information
     *   configured in the access control list specified by
     *   {@link MacMapAclType} instance.
     * </p>
     *
     * @param path     A {@link VBridgePath} object that specifies the position
     *                 of the vBridge.
     * @param aclType  The type of access control list.
     *   <dl style="margin-left: 1em;">
     *     <dt>{@link MacMapAclType#ALLOW}
     *     <dd>
     *       Return all host information configured in
     *       {@linkplain <a href="package-summary.html#MAC-map.allow">Map Allow list</a>}.
     *
     *     <dt>{@link MacMapAclType#DENY}
     *     <dd>
     *       Return all host information configured in
     *       {@linkplain <a href="package-summary.html#MAC-map.deny">Map Deny list</a>}.
     *   </dl>
     * @return  A set of {@link DataLinkHost} instances which contains host
     *          information in the specified access control list is returned.
     *          An empty set is returned if no host is configured in the
     *          specified access control list.
     *          {@code null} is returned if MAC mapping is not configured in
     *          the specified vBridge.
     * @throws VTNException  An error occurred.
     *   The following are the main {@code StatusCode} set in {@link Status}
     *   delivered by the exception.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.BADREQUEST}
     *     <dd>
     *       <ul style="padding-left: 1em;">
     *         <li>
     *           {@code null} is passed to {@code path} or {@code aclType}.
     *         </li>
     *         <li>
     *           {@code null} is configured in {@code path} for the
     *           {@linkplain VTenantPath#getTenantName() VTN name} or
     *           {@linkplain VBridgePath#getBridgeName() vBridge name}.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>
     *       {@linkplain <a href="package-summary.html#VTN">VTN</a>} or vBridge
     *       specified by {@code path} does not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    Set<DataLinkHost> getMacMapConfig(VBridgePath path, MacMapAclType aclType)
        throws VTNException;

    /**
     * Return a list of
     * {@linkplain <a href="package-summary.html#MAC-map.activate">hosts where mapping is actually active</a>}
     * based on
     * {@linkplain <a href="package-summary.html#MAC-map">MAC mapping</a>}
     * configured in the specified
     * {@linkplain <a href="package-summary.html#vBridge">vBridge</a>}.
     *
     * @param path  A {@link VBridgePath} object that specifies the position
     *              of the vBridge.
     * @return
     *   A list of {@link MacAddressEntry} instances that shows the
     *   information of hosts where MAC mapping is active with the vBridge
     *   specified by {@code path}.
     *   <ul>
     *     <li>
     *       An empty list is returned if there are no hosts where MAC mapping
     *       is active.
     *     </li>
     *     <li>
     *       {@code null} is returned if MAC mapping is not configured in the
     *       specified vBridge.
     *     </li>
     *   </ul>
     * @throws VTNException  An error occurred.
     *   The following are the main {@code StatusCode} set in {@link Status}
     *   delivered by the exception.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.BADREQUEST}
     *     <dd>
     *       <ul style="padding-left: 1em;">
     *         <li>{@code null} is passed to {@code path}.</li>
     *         <li>
     *           {@code null} is configured in {@code path} for the
     *           {@linkplain VTenantPath#getTenantName() VTN name} or
     *           {@linkplain VBridgePath#getBridgeName() vBridge name}.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>
     *       {@linkplain <a href="package-summary.html#VTN">VTN</a>} or vBridge
     *       specified by {@code path} does not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    List<MacAddressEntry> getMacMappedHosts(VBridgePath path)
        throws VTNException;

    /**
     * Determine whether, in the specified
     * {@linkplain <a href="package-summary.html#vBridge">vBridge</a>},
     * the
     * {@linkplain <a href="package-summary.html#MAC-map.activate">mapping is actually active</a>}
     * based on the
     * {@linkplain <a href="package-summary.html#MAC-map">MAC mapping</a>} on
     * the specified MAC address.
     *
     * @param path  A {@link VBridgePath} object that specifies the position
     *              of the vBridge.
     * @param addr  A {@link DataLinkAddress} instance which represents the
     *              MAC address.
     *   <ul>
     *     <li>
     *        Currently the VTN Manager handles only ethernet frame.
     *        Thus, in reality, an
     *        {@link org.opendaylight.controller.sal.packet.address.EthernetAddress}
     *        object needs to be specified.
     *     </li>
     *   </ul>
     * @return
     *   If the MAC mapping is active between the vBridge specified by
     *   {@code path} and the MAC address specified by {@code addr},
     *   A {@link MacAddressEntry} instance which shows the host information
     *   corresponding to {@code addr} is returned.
     *   {@code null} is returned in the following cases.
     *   <ul>
     *     <li>
     *       MAC mapping is not configured in the vBridge specified by
     *       {@code path}.
     *     </li>
     *     <li>
     *       MAC mapping is not activated with the MAC address specified by
     *       {@code addr}.
     *     </li>
     *   </ul>
     * @throws VTNException  An error occurred.
     *   The following are the main {@code StatusCode} set in {@link Status}
     *   delivered by the exception.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.BADREQUEST}
     *     <dd>
     *       <ul style="padding-left: 1em;">
     *         <li>{@code null} is passed to {@code path}.</li>
     *         <li>
     *           {@code null} is configured in {@code path} for the
     *           {@linkplain VTenantPath#getTenantName() VTN name} or
     *           {@linkplain VBridgePath#getBridgeName() vBridge name}.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>
     *       {@linkplain <a href="package-summary.html#VTN">VTN</a>} or vBridge
     *       specified by {@code path} does not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    MacAddressEntry getMacMappedHost(VBridgePath path, DataLinkAddress addr)
        throws VTNException;

    /**
     * Configure
     * {@linkplain <a href="package-summary.html#MAC-map">MAC mapping</a>}.
     * in the specified
     * {@linkplain <a href="package-summary.html#vBridge">vBridge</a>}.
     *
     * <p>
     *   This method changes the MAC addresses to be mapped and not to be
     *   mapped at the same time.
     * </p>
     *
     * @param path    A {@link VBridgePath} object that specifies the position
     *                of the vBridge.
     * @param op
     *   A {@link UpdateOperation} instance which indicates how to change
     *   the MAC mapping configuration.
     *   <dl style="margin-left: 1em;">
     *     <dt>{@link UpdateOperation#SET}
     *     <dd>
     *       Change the configuration of the MAC mapping exactly the same
     *       as the configuration specified by {@code mcconf}.
     *       <p>
     *         MAC mapping is removed in the following cases.
     *       </p>
     *       <ul>
     *         <li>
     *           {@code null} is specified to {@code mcconf}.
     *         </li>
     *         <li>
     *           {@code mcconf} does not contain any {@link DataLinkHost}
     *           instance.
     *         </li>
     *       </ul>
     *
     *     <dt>{@link UpdateOperation#ADD}
     *     <dd>
     *       Append the host information configured in {@code mcconf} to the
     *       access controll lists configured in MAC mapping.
     *       <ul>
     *         <li>
     *           No change is made if {@code null} is specified to
     *           {@code mcconf}.
     *         </li>
     *         <li>
     *           {@linkplain <a href="package-summary.html#MAC-map.allow">Map Allow list</a>}
     *           of the MAC mapping is not modified if the Map Allow list in
     *           {@code mcconf} is empty.
     *         </li>
     *         <li>
     *           {@linkplain <a href="package-summary.html#MAC-map.deny">Map Deny list</a>}
     *           of the MAC mapping is not modified if the Map Deny list in
     *           {@code mcconf} is empty.
     *         </li>
     *         <li>
     *           It will be ignored if one tries to add host information which
     *           is already present in the access control list.
     *         </li>
     *       </ul>
     *
     *     <dt>{@link UpdateOperation#REMOVE}
     *     <dd>
     *       Remove the host information configured in {@code mcconf} from the
     *       access controll lists configured in the MAC mapping.
     *       <ul>
     *         <li>
     *           No change is made if {@code null} is specified to
     *           {@code mcconf}.
     *         </li>
     *         <li>
     *           {@linkplain <a href="package-summary.html#MAC-map.allow">Map Allow list</a>}
     *           of the MAC mapping is not modified if the Map Allow list in
     *           {@code mcconf} is empty.
     *         </li>
     *         <li>
     *           {@linkplain <a href="package-summary.html#MAC-map.deny">Map Deny list</a>}
     *           of the MAC mapping is not modified if the Map Deny list in
     *           {@code mcconf} is empty.
     *         </li>
     *         <li>
     *           It will be ignored if one tries to remove host information
     *           which is not present in the access control list.
     *         </li>
     *         <li>
     *           MAC mappin will be removed if both Map Allow and Map Deny list
     *           become empty.
     *         </li>
     *       </ul>
     *   </dl>
     * @param mcconf  A {@link MacMapConfig} instance which contains the MAC
     *                mapping configuration information.
     * @return
     *   A {@link UpdateType} object which represents the result of the
     *   operation is returned.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code UpdateType.ADDED}
     *     <dd>
     *       MAC mapping was newly configured in the specified vBridge.
     *
     *     <dt style="font-weight: bold;">{@code UpdateType.REMOVED}
     *     <dd>
     *       MAC mapping was removed from the specified vBridge.
     *
     *     <dt style="font-weight: bold;">{@code UpdateType.CHANGED}
     *     <dd>
     *       Configuration of existing MAC mapping in the specified vBridge
     *       was changed.
     *
     *     <dt style="font-weight: bold;">{@code null}
     *     <dd>
     *       Configuration of existing MAC mapping is not changed.
     *   </dl>
     * @throws VTNException  An error occurred.
     *   The following are the main {@code StatusCode} set in {@link Status}
     *   delivered by the exception.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.BADREQUEST}
     *     <dd>
     *       <ul style="padding-left: 1em;">
     *         <li>{@code null} is passed to {@code path} or {@code op}.</li>
     *         <li>
     *           {@code null} is configured in {@code path} for the
     *           {@linkplain VTenantPath#getTenantName() VTN name} or
     *           {@linkplain VBridgePath#getBridgeName() vBridge name}.
     *         </li>
     *         <li>
     *           {@code null} or invalid {@link DataLinkHost} instance is
     *           configured in {@code mcconf}.
     *         </li>
     *         <li>
     *           {@link UpdateOperation#SET} or {@link UpdateOperation#ADD}
     *           is passed to {@code op}, one of the following conditions is
     *           met.
     *           <ul>
     *             <li>
     *               Multiple {@link EthernetHost} instances with the same
     *               MAC address are specified in Map Allow list of
     *               {@code mcconf}.
     *             </li>
     *             <li>
     *               {@link EthernetHost} instance without MAC address is
     *               configured in Map Deny list of {@code mcconf}.
     *             </li>
     *           </ul>
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>
     *       {@linkplain <a href="package-summary.html#VTN">VTN</a>} or
     *       vBridge specified by {@code path} does not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.CONFLICT}
     *     <dd>
     *       <ul style="padding-left: 1em;">
     *         <li>
     *           {@link UpdateOperation#SET} or {@link UpdateOperation#ADD} is
     *           passed to {@code op}, and host information configured inside
     *           Map Allow list in MAC mapping of another vBridge is configured
     *           in map Allow list of {@code mcconf}.
     *         </li>
     *         <li>
     *           {@link UpdateOperation#ADD} is passed to {@code op}, and
     *           host information having the same MAC address and different
     *           VLAN ID when compared to the host information in Map Allow
     *           list of {@code mcconf} is already configured in Map Allow list
     *           of MAC mapping.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with the default container, and
     *       a container other than the default container is present.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    UpdateType setMacMap(VBridgePath path, UpdateOperation op,
                         MacMapConfig mcconf) throws VTNException;

    /**
     * Configure
     * {@linkplain <a href="package-summary.html#MAC-map">MAC mapping</a>}.
     * in the specified
     * {@linkplain <a href="package-summary.html#vBridge">vBridge</a>}.
     *
     * <p>
     *   This method changes the specified access control list of the
     *   MAC mapping.
     * </p>
     *
     * @param path      A {@link VBridgePath} object that specifies the
     *                  position of the vBridge.
     * @param op
     *    A {@link UpdateOperation} instance which indicates how to change
     *    the access control list.
     *   <dl style="margin-left: 1em;">
     *     <dt>{@link UpdateOperation#SET}
     *     <dd>
     *       Change the access control list specified by {@code aclType}
     *       exactly the same as {@code dlhosts}.
     *       <ul>
     *         <li>
     *           All the host information inside the specified access control
     *           list will be removed if {@code null} or empty list is
     *           specified to {@code dlhosts}.
     *         </li>
     *         <li>
     *           MAC mappin will be removed if both Map Allow and Map Deny list
     *           become empty.
     *         </li>
     *       </ul>
     *
     *     <dt>{@link UpdateOperation#ADD}
     *     <dd>
     *       Append the host information configured in {@code dlhosts} to the
     *       access control list specified by {@code aclType}.
     *       <ul>
     *         <li>
     *           No change is made if {@code dlhosts} is {@code null} or an
     *           empty set.
     *         </li>
     *         <li>
     *           It will be ignored if one tries to add host information which
     *           is already present in the access control list.
     *         </li>
     *       </ul>
     *
     *     <dt>{@link UpdateOperation#REMOVE}
     *     <dd>
     *       Remove the host information configured in {@code dlhosts} from the
     *       access control list specified by {@code aclType}.
     *       <ul>
     *         <li>
     *           No change is made if {@code dlhosts} is {@code null} or an
     *           empty set.
     *         </li>
     *         <li>
     *           It will be ignored if one tries to add host information which
     *           is already present in the access control list.
     *         </li>
     *         <li>
     *           MAC mappin will be removed if both Map Allow and Map Deny list
     *           become empty.
     *         </li>
     *       </ul>
     *   </dl>
     * @param aclType   The type of access control list.
     *   <dl style="margin-left: 1em;">
     *     <dt>{@link MacMapAclType#ALLOW}
     *     <dd>
     *       Modify host information configured in
     *       {@linkplain <a href="package-summary.html#MAC-map.allow">Map Allow list</a>}.
     *
     *     <dt>{@link MacMapAclType#DENY}
     *     <dd>
     *       Modify host information configured in
     *       {@linkplain <a href="package-summary.html#MAC-map.deny">Map Deny list</a>}.
     *   </dl>
     * @param dlhosts   A set of {@link DataLinkHost} instances.
     * @return
     *   A {@link UpdateType} object which represents the result of the
     *   operation is returned.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code UpdateType.ADDED}
     *     <dd>
     *       MAC mapping was newly configured in the specified vBridge.
     *
     *     <dt style="font-weight: bold;">{@code UpdateType.REMOVED}
     *     <dd>
     *       MAC mapping was removed from the specified vBridge.
     *
     *     <dt style="font-weight: bold;">{@code UpdateType.CHANGED}
     *     <dd>
     *       Configuration of existing MAC mapping in the specified vBridge
     *       was changed.
     *
     *     <dt style="font-weight: bold;">{@code null}
     *     <dd>
     *       Configuration of existing MAC mapping is not changed.
     *   </dl>
     * @throws VTNException  An error occurred.
     *   The following are the main {@code StatusCode} set in {@link Status}
     *   delivered by the exception.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.BADREQUEST}
     *     <dd>
     *       <ul style="padding-left: 1em;">
     *         <li>
     *           {@code null} is passed to {@code path} or {@code op} or
     *           {@code aclType}.
     *         </li>
     *         <li>
     *           {@code null} is configured in {@code path} for the
     *           {@linkplain VTenantPath#getTenantName() VTN name} or
     *           {@linkplain VBridgePath#getBridgeName() vBridge name}.
     *         </li>
     *         <li>
     *           {@code null} or invalid {@link DataLinkHost} instance is
     *           configured in {@code dlhosts}.
     *         </li>
     *         <li>
     *           {@link UpdateOperation#SET} or {@link UpdateOperation#ADD}
     *           is passed to {@code op}, one of the following conditions is
     *           met.
     *           <ul>
     *             <li>
     *               {@link MacMapAclType#ALLOW} is passed to {@code aclType},
     *               and multiple {@link EthernetHost} instances with the
     *               same MAC address are specified in {@code dlhosts}.
     *             </li>
     *             <li>
     *               {@link MacMapAclType#DENY} is passed to {@code aclType},
     *               and {@link EthernetHost} instance without MAC address
     *               is configured in {@code dlhosts}.
     *             </li>
     *           </ul>
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>
     *       {@linkplain <a href="package-summary.html#VTN">VTN</a>} or
     *       vBridge specified by {@code path} does not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.CONFLICT}
     *     <dd>
     *       {@link MacMapAclType#ALLOW} is passed to {@code aclType}, and
     *       one of the following conditions is met.
     *       <ul style="padding-left: 1em;">
     *         <li>
     *           {@link UpdateOperation#SET} or {@link UpdateOperation#ADD} is
     *           passed to {@code op}, and host information configured inside
     *           the access control list in MAC mapping of another vBridge
     *           is configured in {@code dlhosts}.
     *         </li>
     *         <li>
     *           {@link UpdateOperation#ADD} is passed to {@code op}, and
     *           host information having the same MAC address and different
     *           VLAN ID when compared to the host information in
     *           {@code dlhosts} is already configured in access control list
     *           of MAC mapping.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with the default container, and
     *       a container other than the default container is present.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    UpdateType setMacMap(VBridgePath path, UpdateOperation op,
                         MacMapAclType aclType,
                         Set<? extends DataLinkHost> dlhosts)
        throws VTNException;

    /**
     * Initiate the discovery of a host based on its IP address.
     *
     * <p>
     *   If the specified IP address is an IPv4 address, this method sends
     *   a broadcast ARP request to the specified vBridges.
     *   If a host is found, it is reported to {@code HostTracker} via
     *   {@code IfHostListener} interface.
     * </p>
     *
     * @param addr
     *   An {@link InetAddress} object which represents an IP address.
     *   <p style="margin-left: 1em;">
     *     Note that this method does nothing in the following cases.
     *   </p>
     *   <ul>
     *     <li>If {@code null} is passed to {@code addr}.</li>
     *     <li>If {@code addr} represents an IPv6 address.</li>
     *   </ul>
     * @param pathSet
     *   A set of {@link VBridgePath} objects which indicates the vBridge
     *   where an ARP request is to be sent.
     *   <ul>
     *     <li>
     *       An ARP request will be sent to only those vBridges whose position
     *       is contained in {@code pathSet}.
     *     </li>
     *     <li>
     *       If {@code null} is passed, then an ARP request will be sent to
     *       all the vBridges that are present inside the container.
     *     </li>
     *   </ul>
     */
    void findHost(InetAddress addr, Set<VBridgePath> pathSet);

    /**
     * Check to see if the specified host is still in the network.
     *
     * <p>
     *   This method sends an unicast ARP request to the host specified by
     *   {@code host}. If the specified host sends a ARP reply, it is reported
     *   to {@code HostTracker} via {@code IfHostListener} interface.
     * </p>
     *
     * @param host
     *   A {@link HostNodeConnector} object which represents a host that needs
     *   to be probed.
     *   <p style="margin-left: 1em;">
     *     An unicast ARP request will be sent to the physical switch port
     *     corresponding to the
     *     {@link org.opendaylight.controller.sal.core.NodeConnector} object
     *     configured in {@code host}.
     *   </p>
     * @return
     *   {@code true} is returned if an ARP request was actually sent.
     *   <p>
     *     Otherwise {@code false} is returned.
     *     The following are main reasons for returning {@code false}.
     *   </p>
     *   <ul>
     *     <li>If {@code null} is passed to {@code host}.</li>
     *     <li>
     *       If {@link org.opendaylight.controller.sal.core.NodeConnector}
     *       object is not configured in {@code host}.
     *     </li>
     *     <li>
     *       If the physical switch port corresponding to the
     *       {@link org.opendaylight.controller.sal.core.NodeConnector} object
     *       configured in {@code host} is not operating.
     *     </li>
     *     <li>
     *       If the vBridge to which the {@code host} belongs does not exist.
     *     </li>
     *   </ul>
     */
    boolean probeHost(HostNodeConnector host);

    /**
     * Return a list of {@link MacAddressEntry} objects corresponding to
     * all the MAC address information learned in the
     * {@linkplain <a href="package-summary.html#macTable">MAC address table</a>}
     * of the specified
     * {@linkplain <a href="package-summary.html#vBridge">vBridge</a>}.
     *
     * @param path  A {@link VBridgePath} object that specifies the position
     *              of the vBridge.
     * @return  A list of {@link MacAddressEntry} objects corresponding to all
     *          the MAC address information learned in the vBridge specified
     *          by {@code path}.
     *          An empty list is returned if no MAC address is learned in the
     *          specified vBridge.
     * @throws VTNException  An error occurred.
     *   The following are the main {@code StatusCode} set in {@link Status}
     *   delivered by the exception.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.BADREQUEST}
     *     <dd>
     *       <ul style="padding-left: 1em;">
     *         <li>{@code null} is passed to {@code path}.</li>
     *         <li>
     *           {@code null} is configured in {@code path} for the
     *           {@linkplain VTenantPath#getTenantName() VTN name} or
     *           {@linkplain VBridgePath#getBridgeName() vBridge name}.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>{@linkplain <a href="package-summary.html#VTN">VTN</a>} or
     *         vBridge specified by {@code path} does not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     */
    List<MacAddressEntry> getMacEntries(VBridgePath path) throws VTNException;

    /**
     * Search the
     * {@linkplain <a href="package-summary.html#macTable">MAC address table</a>}
     * of the {@linkplain <a href="package-summary.html#vBridge">vBridge</a>}
     * for the specified MAC address.
     *
     * @param path  A {@link VBridgePath} object that specifies the position
     *              of the vBridge.
     * @param addr
     *   A {@link DataLinkAddress} object which represents the MAC address.
     *   <ul>
     *     <li>
     *        Currently the VTN Manager handles only ethernet frame.
     *        Thus, in reality, an
     *        {@link org.opendaylight.controller.sal.packet.address.EthernetAddress}
     *        object needs to be specified.
     *     </li>
     *   </ul>
     * @return  A {@link MacAddressEntry} object which represents the MAC
     *          address information corresponding to {@code addr} is returned.
     *          {@code null} is returned if the MAC address specified by
     *          {@code addr} is not learned in the specified vBridge.
     * @throws VTNException  An error occurred.
     *   The following are the main {@code StatusCode} set in {@link Status}
     *   delivered by the exception.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.BADREQUEST}
     *     <dd>
     *       <ul style="padding-left: 1em;">
     *         <li>
     *           {@code null} is passed to {@code path} or {@code addr}.
     *         </li>
     *         <li>
     *           {@code null} is configured in {@code path} for the
     *           {@linkplain VTenantPath#getTenantName() VTN name} or
     *           {@linkplain VBridgePath#getBridgeName() vBridge name}.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>{@linkplain <a href="package-summary.html#VTN">VTN</a>} or
     *         vBridge specified by {@code path} does not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     */
    MacAddressEntry getMacEntry(VBridgePath path, DataLinkAddress addr)
        throws VTNException;

    /**
     * Remove the MAC address information from the
     * {@linkplain <a href="package-summary.html#macTable">MAC address table</a>}
     * in the specified
     * {@linkplain <a href="package-summary.html#vBridge">vBridge</a>}.
     *
     * @param path  A {@link VBridgePath} object that specifies the position
     *              of the vBridge.
     * @param addr
     *   A {@link DataLinkAddress} object which represents the MAC address
     *   to be removed.
     *   <ul>
     *     <li>
     *        Currently the VTN Manager handles only ethernet frame.
     *        Thus, in reality, an
     *        {@link org.opendaylight.controller.sal.packet.address.EthernetAddress}
     *        object needs to be specified.
     *     </li>
     *   </ul>
     * @return  A {@link MacAddressEntry} object which represents the MAC
     *          address information actually removed.
     *          {@code null} is returned if the MAC address specified by
     *          {@code addr} is not learned in the specified vBridge.
     * @throws VTNException  An error occurred.
     *   The following are the main {@code StatusCode} set in {@link Status}
     *   delivered by the exception.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.BADREQUEST}
     *     <dd>
     *       <ul style="padding-left: 1em;">
     *         <li>
     *           {@code null} is passed to {@code path} or {@code addr}.
     *         </li>
     *         <li>
     *           {@code null} is configured in {@code path} for the
     *           {@linkplain VTenantPath#getTenantName() VTN name} or
     *           {@linkplain VBridgePath#getBridgeName() vBridge name}.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>{@linkplain <a href="package-summary.html#VTN">VTN</a>} or
     *         vBridge specified by {@code path} does not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with the default container, and
     *       a container other than the default container is present.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     */
    MacAddressEntry removeMacEntry(VBridgePath path, DataLinkAddress addr)
        throws VTNException;

    /**
     * Remove all the MAC address information learned in the
     * {@linkplain <a href="package-summary.html#macTable">MAC address table</a>}
     * of the specified
     * {@linkplain <a href="package-summary.html#vBridge">vBridge</a>}.
     *
     * @param path  A {@link VBridgePath} object that specifies the position
     *              of the vBridge.
     * @return
     *   A {@link Status} object which represents the result of the operation
     *   is returned.
     *   <p>
     *     Upon successful completion,
     *     <strong>{@code StatusCode.SUCCESS}</strong> is set in a returned
     *     object. Otherwise a {@code StatusCode} which indicates the cause
     *     of error is set in a returned {@link Status} object.
     *     The following are the main {@code StatusCode} configured in
     *     {@link Status}.
     *   </p>
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.BADREQUEST}
     *     <dd>
     *       <ul style="padding-left: 1em;">
     *         <li>{@code null} is passed to {@code path}.</li>
     *         <li>
     *           {@code null} is configured in {@code path} for the
     *           {@linkplain VTenantPath#getTenantName() VTN name} or
     *           {@linkplain VBridgePath#getBridgeName() vBridge name}.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>{@linkplain <a href="package-summary.html#VTN">VTN</a>} or
     *         vBridge specified by {@code path} does not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with the default container, and
     *       a container other than the default container is present.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     */
    Status flushMacEntries(VBridgePath path);

    /**
     * Return information about all data flows present in the specified
     * {@linkplain <a href="package-summary.html#VTN">VTN</a>}.
     *
     * @param path    A {@link VTenantPath} object that specifies the position
     *                of the VTN.
     * @param mode    A {@link org.opendaylight.vtn.manager.flow.DataFlow.Mode}
     *                instance which specifies behavior of this method.
     * @param filter  If a {@link DataFlowFilter} instance is specified,
     *                only data flows that meet the condition specified by
     *                {@link DataFlowFilter} instance is returned.
     *                All data flows in the VTN is returned if {@code null}
     *                is specified.
     * @return  A list of {@link DataFlow} instances which represents
     *          information about data flows.
     * @throws VTNException  An error occurred.
     *   The following are the main {@code StatusCode} set in {@link Status}
     *   delivered by the exception.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.BADREQUEST}
     *     <dd>
     *       <ul style="padding-left: 1em;">
     *         <li>{@code null} is passed to {@code path} or {@code mode}.</li>
     *         <li>
     *           {@code null} is configured in {@code path} for the
     *           {@linkplain VTenantPath#getTenantName() VTN name}.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>VTN specified by {@code path} does not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    List<DataFlow> getDataFlows(VTenantPath path, DataFlow.Mode mode,
                                DataFlowFilter filter)
        throws VTNException;

    /**
     * Return information about the specified data flow in the
     * {@linkplain <a href="package-summary.html#VTN">VTN</a>}.
     *
     * @param path    A {@link VTenantPath} object that specifies the position
     *                of the VTN.
     * @param flowId  An identifier of the data flow.
     * @param mode    A {@link org.opendaylight.vtn.manager.flow.DataFlow.Mode}
     *                instance which specifies behavior of this method.
     * @return  A {@link DataFlow} instance which represents information
     *          about the specified data flow.
     *          {@code null} is returned if the specified data flow was not
     *          found.
     * @throws VTNException  An error occurred.
     *   The following are the main {@code StatusCode} set in {@link Status}
     *   delivered by the exception.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.BADREQUEST}
     *     <dd>
     *       <ul style="padding-left: 1em;">
     *         <li>
     *           {@code null} is passed to {@code path} or {@code flowId} or
     *           {@code mode}.
     *         </li>
     *         <li>
     *           An invalid flow identifier is passed to {@code flowId}.
     *         </li>
     *         <li>
     *           {@code null} is configured in {@code path} for the
     *           {@linkplain VTenantPath#getTenantName() VTN name}.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>VTN specified by {@code path} does not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    DataFlow getDataFlow(VTenantPath path, long flowId, DataFlow.Mode mode)
        throws VTNException;

    /**
     * Return the number of data flows present in the specified
     * {@linkplain <a href="package-summary.html#VTN">VTN</a>}.
     *
     * @param path  A {@link VTenantPath} object that specifies the position
     *              of the VTN.
     * @return  The number of data flows present in the specified VTN.
     * @throws VTNException  An error occurred.
     *   The following are the main {@code StatusCode} set in {@link Status}
     *   delivered by the exception.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.BADREQUEST}
     *     <dd>
     *       <ul style="padding-left: 1em;">
     *         <li>{@code null} is passed to {@code path}.</li>
     *         <li>
     *           {@code null} is configured in {@code path} for the
     *           {@linkplain VTenantPath#getTenantName() VTN name}.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>VTN specified by {@code path} does not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    int getDataFlowCount(VTenantPath path) throws VTNException;
}
