/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.net.InetAddress;
import java.util.List;
import java.util.Set;

import org.opendaylight.vtn.manager.flow.DataFlow;
import org.opendaylight.vtn.manager.flow.DataFlowFilter;
import org.opendaylight.vtn.manager.flow.cond.FlowCondition;
import org.opendaylight.vtn.manager.flow.cond.FlowMatch;
import org.opendaylight.vtn.manager.flow.filter.FlowFilter;
import org.opendaylight.vtn.manager.flow.filter.FlowFilterId;

import org.opendaylight.controller.hosttracker.hostAware.HostNodeConnector;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.packet.address.DataLinkAddress;
import org.opendaylight.controller.sal.utils.Status;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.DataFlowMode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnAclType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateOperationType;

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
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
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
     *       This service is associated with a non-default container.
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
     *       This service is associated with a non-default container.
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
     *       This service is associated with a non-default container.
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
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
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
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
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
     *       This service is associated with a non-default container.
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
     *       This service is associated with a non-default container.
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
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     */
    Status removeBridge(VBridgePath path);

    /**
     * Return a list of {@link VTerminal} objects corresponding to all the
     * {@linkplain <a href="package-summary.html#vTerminal">vTerminal</a>}
     * inside the specified
     * {@linkplain <a href="package-summary.html#VTN">VTN</a>}.
     *
     * @param path
     *   A {@link VTenantPath} object that specifies the position of the VTN.
     * @return  A list of {@link VTerminal} objects corresponding to all the
     *          vTerminals present inside the VTN specified by {@code path}.
     *          An empty list is returned if no vTerminal is present inside
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
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    List<VTerminal> getTerminals(VTenantPath path) throws VTNException;

    /**
     * Return information about the specified
     * {@linkplain <a href="package-summary.html#vTerminal">vTerminal</a>}
     * in the {@linkplain <a href="package-summary.html#VTN">VTN</a>}.
     *
     * @param path  A {@link VTerminalPath} object that specifies the position
     *              of the vTerminal.
     * @return  A {@link VTerminal} object which represents information about
     *          the vTerminal specified by {@code path}.
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
     *           {@linkplain VTerminalPath#getTerminalName() vTerminal name}.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>
     *       {@linkplain <a href="package-summary.html#VTN">VTN</a>} or
     *       vTerminal specified by {@code path} does not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    VTerminal getTerminal(VTerminalPath path) throws VTNException;

    /**
     * Create a new
     * {@linkplain <a href="package-summary.html#vTerminal">vTerminal</a>}
     * inside the specified
     * {@linkplain <a href="package-summary.html#VTN">VTN</a>}.
     *
     * @param path
     *   A {@link VTerminalPath} object that specifies the position of new
     *   vTerminal.
     *   <p style="margin-left: 1em;">
     *     The name of the vTerminal inside {@link VTerminalPath} must
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
     * @param vtconf
     *   A {@link VTerminalConfig} object which specifies the vTerminal
     *   configuration information.
     *   <ul>
     *     <li>
     *       The description of the vTerminal is not registered if {@code null}
     *       is configured in {@code vtconf} for the
     *       {@linkplain VTerminalConfig#getDescription() description}.
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
     *           {@code null} is passed to {@code path} or {@code vtconf}.
     *         </li>
     *         <li>
     *           {@code null} is configured in {@code path} for the
     *           {@linkplain VTenantPath#getTenantName() VTN name}.
     *         </li>
     *         <li>
     *           Incorrect
     *           {@linkplain VTerminalPath#getTerminalName() vTerminal name}
     *           is configured in {@code path}.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>{@linkplain <a href="package-summary.html#VTN">VTN</a>}
     *         specified by {@code path} does not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.CONFLICT}
     *     <dd>vTerminal specified by {@code path} already exists.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    Status addTerminal(VTerminalPath path, VTerminalConfig vtconf);

    /**
     * Modify the configuration of the existing
     * {@linkplain <a href="package-summary.html#vTerminal">vTerminal</a>}
     * in the specified
     * {@linkplain <a href="package-summary.html#VTN">VTN</a>}.
     *
     * @param path    A {@link VTerminalPath} that specifies the position
     *                of the vTerminal to be modified.
     * @param vtconf  A {@link VTerminalConfig} object which contains the
     *                vTerminal configuration information to be applied.
     * @param all
     *   A boolean value to determine the treatment of attributes for which
     *   value has not been configured inside {@code vtconf}.
     *   <ul>
     *     <li>
     *       If {@code true} is specified, all the attributes related to
     *       vTerminal are modified.
     *       <ul>
     *         <li>
     *           The description of the vTerminal will be deleted if
     *           {@code null} is configured in {@code vtconf} for the
     *           {@linkplain VTerminalConfig#getDescription() description}.
     *         </li>
     *       </ul>
     *     </li>
     *     <li>
     *       If {@code false} is specified, attributes with no value
     *       configured in {@code vtconf} is not modified.
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
     *           {@code null} is passed to {@code path} or {@code vtconf}.
     *         </li>
     *         <li>
     *           {@code null} is configured in {@code path} for the
     *           {@linkplain VTenantPath#getTenantName() VTN name} or
     *           {@linkplain VTerminalPath#getTerminalName() vTerminal name}.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>
     *       {@linkplain <a href="package-summary.html#VTN">VTN</a>} or
     *       vTerminal specified by {@code path} does not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    Status modifyTerminal(VTerminalPath path, VTerminalConfig vtconf,
                          boolean all);

    /**
     * Remove the specified
     * {@linkplain <a href="package-summary.html#vTerminal">vTerminal</a>}
     * from the {@linkplain <a href="package-summary.html#VTN">VTN</a>}.
     *
     * @param path  A {@link VTerminalPath} object that specifies the position
     *              of the vTerminal to be removed.
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
     *           {@linkplain VTerminalPath#getTerminalName() vTerminal name}.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>
     *       {@linkplain <a href="package-summary.html#VTN">VTN</a>} or
     *       vTerminal specified by {@code path} does not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    Status removeTerminal(VTerminalPath path);

    /**
     * Return a list of {@link VInterface} objects corresponding to all the
     * {@linkplain <a href="package-summary.html#vInterface">virtual interfaces</a>}
     * inside the specified
     * {@linkplain <a href="package-summary.html#vBridge">vBridge</a>}.
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
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    List<VInterface> getInterfaces(VBridgePath path) throws VTNException;

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
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    VInterface getInterface(VBridgeIfPath path) throws VTNException;

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
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    Status addInterface(VBridgeIfPath path, VInterfaceConfig iconf);

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
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    Status modifyInterface(VBridgeIfPath path, VInterfaceConfig iconf,
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
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    Status removeInterface(VBridgeIfPath path);

    /**
     * Return a list of {@link VInterface} objects corresponding to all the
     * {@linkplain <a href="package-summary.html#vInterface">virtual interfaces</a>}
     * inside the specified
     * {@linkplain <a href="package-summary.html#vTerminal">vTerminal</a>}.
     *
     * @param path
     *   A {@link VTerminalPath} object that specifies the position of the
     *   {@linkplain <a href="package-summary.html#vTerminal">vTerminal</a>}.
     * @return  A list of {@link VInterface} objects corresponding to all the
     *          virtual interfaces present inside the vTerminal specified by
     *          {@code path}.
     *          An empty list is returned if no virtual interface is present
     *          inside the vTerminal.
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
     *           {@linkplain VTerminalPath#getTerminalName() vTerminal name}.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>{@linkplain <a href="package-summary.html#VTN">VTN</a>} or
     *         vTerminal specified by {@code path} does not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    List<VInterface> getInterfaces(VTerminalPath path) throws VTNException;

    /**
     * Return information about the specified
     * {@linkplain <a href="package-summary.html#vInterface">virtual interface</a>}
     * in the {@linkplain <a href="package-summary.html#vTerminal">vTerminal</a>}.
     *
     * @param path  A {@link VTerminalIfPath} object that specifies the
     *              position of the vTerminal interface.
     * @return  A {@link VInterface} object which represents information about
     *          the vTerminal interface specified by {@code path}.
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
     *           {@linkplain VTerminalPath#getTerminalName() vTerminal name} or
     *           {@linkplain VTerminalIfPath#getInterfaceName() interface name}.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>
     *       {@linkplain <a href="package-summary.html#VTN">VTN</a>} or
     *       vTerminal or vTerminal interface specified by {@code path} does
     *       not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    VInterface getInterface(VTerminalIfPath path) throws VTNException;

    /**
     * Create a new
     * {@linkplain <a href="package-summary.html#vInterface">virtual interface</a>}
     * inside the specified
     * {@linkplain <a href="package-summary.html#vTerminal">vTerminal</a>}.
     *
     * @param path
     *   A {@link VTerminalIfPath} object that specifies the position of new
     *   vTerminal interface.
     *   <p style="margin-left: 1em;">
     *     The name of the vTerminal interface inside {@link VTerminalIfPath}
     *     must be a string that meets the following conditions.
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
     *   A {@link VInterfaceConfig} object which specifies the vTerminal
     *   interface configuration information.
     *   <ul>
     *     <li>
     *       The description of the vTerminal interface is not registered if
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
     *           {@linkplain VTerminalPath#getTerminalName() vTerminal name}.
     *         </li>
     *         <li>
     *           Incorrect
     *           {@linkplain VTerminalIfPath#getInterfaceName() interface name}
     *           is configured in {@code path}.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>{@linkplain <a href="package-summary.html#VTN">VTN</a>} or
     *         vTerminal specified by {@code path} does not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.CONFLICT}
     *     <dd>A vTerminal interface already exists in the vTerminal specified
     *         by {@code path}.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    Status addInterface(VTerminalIfPath path, VInterfaceConfig iconf);

    /**
     * Modify the configuration of the existing
     * {@linkplain <a href="package-summary.html#vInterface">virtual interface</a>}
     * in the specified
     * {@linkplain <a href="package-summary.html#vTerminal">vTerminal</a>}.
     *
     * @param path   A {@link VTerminalIfPath} that specifies the position
     *               of the vTerminal interface to be modified.
     * @param iconf  A {@link VInterfaceConfig} object which contains the
     *               vTerminal interface configuration information to be
     *               applied.
     * @param all
     *   A boolean value to determine the treatment of attributes for which
     *   value has not been configured inside {@code iconf}.
     *   <ul>
     *     <li>
     *       If {@code true} is specified, all the attributes related to
     *       vTerminal interface are modified.
     *       <ul>
     *         <li>
     *           The description of the vTerminal interface will be deleted if
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
     *           {@linkplain VTerminalPath#getTerminalName() vTerminal name} or
     *           {@linkplain VTerminalIfPath#getInterfaceName() interface name}.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>
     *       {@linkplain <a href="package-summary.html#VTN">VTN</a>} or
     *       vTerminal or vTerminal interface specified by {@code path} does
     *       not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    Status modifyInterface(VTerminalIfPath path, VInterfaceConfig iconf,
                           boolean all);

    /**
     * Remove the specified
     * {@linkplain <a href="package-summary.html#vInterface">virtual interface</a>}
     * from the
     * {@linkplain <a href="package-summary.html#vTerminal">vTerminal</a>}.
     *
     * @param path  A {@link VTerminalIfPath} object that specifies the
     *              position of the vTerminal interface to be removed.
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
     *           {@linkplain VTerminalPath#getTerminalName() vTerminal name} or
     *           {@linkplain VTerminalIfPath#getInterfaceName() interface name}.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>
     *       {@linkplain <a href="package-summary.html#VTN">VTN</a>} or
     *       vTerminal or vTerminal interface specified by {@code path} does
     *       not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    Status removeInterface(VTerminalIfPath path);

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
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
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
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
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
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
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
     *       This service is associated with a non-default container.
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
     *       This service is associated with a non-default container.
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
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     */
    PortMap getPortMap(VBridgeIfPath path) throws VTNException;

    /**
     * Return information about the
     * {@linkplain <a href="package-summary.html#port-map">port mapping</a>}
     * configured in the specified
     * {@linkplain <a href="package-summary.html#vInterface">virtual interface</a>}
     * in the
     * {@linkplain <a href="package-summary.html#vTerminal">vTerminal</a>}.
     *
     * @param path  A {@link VTerminalIfPath} object that specifies the
     *              position of the vTerminal interface.
     * @return  A {@link PortMap} object which contains the port mapping
     *          information is returned if the port mapping is configured
     *          in the vTerminal interface specified by {@code path}.
     *          {@code null} is returned if the port mapping is not configured
     *          in the specified vTerminal interface.
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
     *           {@linkplain VTerminalPath#getTerminalName() vTerminal name} or
     *           {@linkplain VTerminalIfPath#getInterfaceName() interface name}.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>
     *       {@linkplain <a href="package-summary.html#VTN">VTN</a>} or
     *       vTerminal or vTerminal interface specified by {@code path} does
     *       not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    PortMap getPortMap(VTerminalIfPath path) throws VTNException;

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
     *             virtual interface.
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
     *       to another virtual interface.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     */
    Status setPortMap(VBridgeIfPath path, PortMapConfig pmconf);

    /**
     * Configure
     * {@linkplain <a href="package-summary.html#port-map">port mapping</a>}
     * in the specified
     * {@linkplain <a href="package-summary.html#vInterface">virtual interface</a>}
     * in the
     * {@linkplain <a href="package-summary.html#vTerminal">vTerminal</a>}.
     *
     * <p>
     *   If port mapping is already configured in the specified vTerminal
     *   interface, the specified settings are applied after old configuration
     *   is deleted.
     * </p>
     * <p>
     *   If a non-{@code null} value is passed to {@code pmconf}, then
     *   ethernet frame that flows through the port of the physical switch
     *   specified by {@code pmconf} will get mapped to the vTerminal interface
     *   specified by {@code path}.
     * </p>
     * <ul>
     *   <li>
     *     {@code pmconf} must contain a
     *     {@link org.opendaylight.controller.sal.core.Node} object
     *     corresponding to the physical switch that you want to map to the
     *     vTerminal interface.
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
     *     map to the vTerminal interface.
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
     *             virtual interface.
     *           </li>
     *         </ul>
     *       </li>
     *     </ul>
     *   </li>
     *   <li>
     *     The VLAN network is mapped to the vTerminal interface according to
     *     the VLAN ID configured in {@code pmconf}.
     *     <ul>
     *       <li>
     *         If a value between <strong>1</strong> or more and
     *         <strong>4095</strong> or less is configured, then the ethernet
     *         frames that have this VLAN ID configured will get mapped to
     *         the vTerminal interface.
     *       </li>
     *       <li>
     *         If <strong>0</strong> is configured, untagged ethernet frames
     *         will get mapped to the vTerminal interface.
     *       </li>
     *     </ul>
     *   </li>
     *   <li>
     *     This method does nothing if port mapping with the same configuration
     *     information as {@code pmconf} is already configured in the vTerminal
     *     interface specified by {@code path}.
     *   </li>
     * </ul>
     *
     * @param path    A {@link VTerminalIfPath} object that specifies the
     *                position of the vTerminal interface.
     * @param pmconf
     *   A {@link PortMapConfig} object which contains the port mapping
     *   configuration information.
     *   <p style="margin-left: 1em;">
     *     If {@code null} is specified, port mapping in the specified
     *     vTerminal interface is removed. In this case this method will
     *     succeed even if no port mapping is configured to the specified
     *     vTerminal interface.
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
     *           {@linkplain VTerminalPath#getTerminalName() vTerminal name} or
     *           {@linkplain VTerminalIfPath#getInterfaceName() interface name}.
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
     *       {@linkplain <a href="package-summary.html#VTN">VTN</a>} or
     *       vTerminal or vTerminal interface specified by {@code path} does
     *       not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.CONFLICT}
     *     <dd>
     *       Physical switch port specified by {@code pmconf} exists, and
     *       the specified combination of physical port and VLAN ID is mapped
     *       to another virtual interface.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    Status setPortMap(VTerminalIfPath path, PortMapConfig pmconf);

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
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
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
     *   {@link VtnAclType} instance.
     * </p>
     *
     * @param path     A {@link VBridgePath} object that specifies the position
     *                 of the vBridge.
     * @param aclType  The type of access control list.
     *   <dl style="margin-left: 1em;">
     *     <dt>{@link VtnAclType#ALLOW}
     *     <dd>
     *       Return all host information configured in
     *       {@linkplain <a href="package-summary.html#MAC-map.allow">Map Allow list</a>}.
     *
     *     <dt>{@link VtnAclType#DENY}
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
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    Set<DataLinkHost> getMacMapConfig(VBridgePath path, VtnAclType aclType)
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
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
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
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
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
     *   A {@link VtnUpdateOperationType} instance which indicates how to
     *   change the MAC mapping configuration.
     *   <dl style="margin-left: 1em;">
     *     <dt>{@link VtnUpdateOperationType#SET}
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
     *     <dt>{@link VtnUpdateOperationType#ADD}
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
     *     <dt>{@link VtnUpdateOperationType#REMOVE}
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
     *       Configuration of existing MAC mapping was not changed.
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
     *           {@link VtnUpdateOperationType#SET} or
     *           {@link VtnUpdateOperationType#ADD} is passed to {@code op},
     *           one of the following conditions is met.
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
     *           {@link VtnUpdateOperationType#SET} or
     *           {@link VtnUpdateOperationType#ADD} is passed to {@code op},
     *           and host information configured inside Map Allow list in
     *           MAC mapping of another vBridge is configured in Map Allow
     *           list of {@code mcconf}.
     *         </li>
     *         <li>
     *           {@link VtnUpdateOperationType#ADD} is passed to {@code op},
     *           and host information having the same MAC address and different
     *           VLAN ID when compared to the host information in Map Allow
     *           list of {@code mcconf} is already configured in Map Allow list
     *           of MAC mapping.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    UpdateType setMacMap(VBridgePath path, VtnUpdateOperationType op,
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
     *    A {@link VtnUpdateOperationType} instance which indicates how to
     *   change the access control list.
     *   <dl style="margin-left: 1em;">
     *     <dt>{@link VtnUpdateOperationType#SET}
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
     *     <dt>{@link VtnUpdateOperationType#ADD}
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
     *     <dt>{@link VtnUpdateOperationType#REMOVE}
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
     *     <dt>{@link VtnAclType#ALLOW}
     *     <dd>
     *       Modify host information configured in
     *       {@linkplain <a href="package-summary.html#MAC-map.allow">Map Allow list</a>}.
     *
     *     <dt>{@link VtnAclType#DENY}
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
     *       Configuration of existing MAC mapping was not changed.
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
     *           {@link VtnUpdateOperationType#SET} or
     *           {@link VtnUpdateOperationType#ADD} is passed to {@code op},
     *           one of the following conditions is met.
     *           <ul>
     *             <li>
     *               {@link VtnAclType#ALLOW} is passed to {@code aclType},
     *               and multiple {@link EthernetHost} instances with the
     *               same MAC address are specified in {@code dlhosts}.
     *             </li>
     *             <li>
     *               {@link VtnAclType#DENY} is passed to {@code aclType},
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
     *       {@link VtnAclType#ALLOW} is passed to {@code aclType}, and
     *       one of the following conditions is met.
     *       <ul style="padding-left: 1em;">
     *         <li>
     *           {@link VtnUpdateOperationType#SET} or
     *           {@link VtnUpdateOperationType#ADD} is passed to {@code op},
     *           and host information configured inside the access control
     *           list in MAC mapping of another vBridge is configured in
     *           {@code dlhosts}.
     *         </li>
     *         <li>
     *           {@link VtnUpdateOperationType#ADD} is passed to {@code op},
     *           and host information having the same MAC address and different
     *           VLAN ID when compared to the host information in
     *           {@code dlhosts} is already configured in access control list
     *           of MAC mapping.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    UpdateType setMacMap(VBridgePath path, VtnUpdateOperationType op,
                         VtnAclType aclType,
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
     * <p>
     *   Note that this method does nothing if this service is associated
     *   with a non-default container.
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
     *     <li>
     *       If this service is associated with a non-default container.
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
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
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
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
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
     *       This service is associated with a non-default container.
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
     *       This service is associated with a non-default container.
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
     * @param path
     *   A {@link VTenantPath} object that specifies the position of the VTN.
     * @param mode
     *    A {@link DataFlowMode} instance which specifies behavior of this
     *    method.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@link DataFlowMode#SUMMARY}
     *     <dd>
     *       Indicates that summarized information is required.
     *       <p>
     *         If this mode is specified, the following attributes in
     *         {@link DataFlow} are omitted.
     *       </p>
     *       <ul>
     *         <li>
     *           The flow condition configured in the ingress flow entry.
     *           ({@link DataFlow#getMatch()})
     *         </li>
     *         <li>
     *           Actions to applied to the packet by the egress flow entry.
     *           ({@link DataFlow#getActions()})
     *         </li>
     *         <li>
     *           The route of the packet in the virtual network.
     *           ({@link DataFlow#getVirtualRoute()})
     *         </li>
     *         <li>
     *           The route of the packet in the physical network.
     *           ({@link DataFlow#getPhysicalRoute()})
     *         </li>
     *         <li>
     *           Statistics information. ({@link DataFlow#getStatistics()})
     *         </li>
     *         <li>
     *           Averaged statistics information.
     *           ({@link DataFlow#getAveragedStatistics()})
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@link DataFlowMode#DETAIL}
     *     <dd>
     *       Indicates that detailed information is required.
     *       <p>
     *         If this mode is specified, all attributes in {@link DataFlow}
     *         are filled if available. {@link DataFlow#getStatistics()}
     *         returns statistics cached in the statistics manager, which is
     *         updated every 10 seconds.
     *         {@link DataFlow#getAveragedStatistics()} returns averaged
     *         statistics per second if available.
     *       </p>
     *
     *     <dt style="font-weight: bold;">{@link DataFlowMode#UPDATESTATS}
     *     <dd>
     *       Same as {@link DataFlowMode#DETAIL}, but always make requests to
     *       physical switches to get flow statistics.
     *   </dl>
     * @param filter
     *    If a {@link DataFlowFilter} instance is specified, only data flows
     *    that meet the condition specified by {@link DataFlowFilter} instance
     *    is returned.
     *    All data flows in the VTN is returned if {@code null} is specified.
     * @param interval
     *    Time interval in seconds for retrieving the average statistics.
     *    Specifying zero or a negative value is treated as if 10 is
     *    specified.
     *    Note that this value is just a hint for determining the measurement
     *    period. So the actual measurement period may differ from the
     *    specified value.
     *    This value is ignored if {@link DataFlowMode#SUMMARY} is specified
     *    to {@code mode}.
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
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    List<DataFlow> getDataFlows(VTenantPath path, DataFlowMode mode,
                                DataFlowFilter filter, int interval)
        throws VTNException;

    /**
     * Return information about the specified data flow in the
     * {@linkplain <a href="package-summary.html#VTN">VTN</a>}.
     *
     * @param path
     *    A {@link VTenantPath} object that specifies the position of the VTN.
     * @param flowId
     *    An identifier of the data flow.
     * @param mode
     *    A {@link DataFlowMode} instance which specifies behavior of this
     *    method.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@link DataFlowMode#SUMMARY}
     *     <dd>
     *       Indicates that summarized information is required.
     *       <p>
     *         If this mode is specified, the following attributes in
     *         {@link DataFlow} are omitted.
     *       </p>
     *       <ul>
     *         <li>
     *           The flow condition configured in the ingress flow entry.
     *           ({@link DataFlow#getMatch()})
     *         </li>
     *         <li>
     *           Actions to applied to the packet by the egress flow entry.
     *           ({@link DataFlow#getActions()})
     *         </li>
     *         <li>
     *           The route of the packet in the virtual network.
     *           ({@link DataFlow#getVirtualRoute()})
     *         </li>
     *         <li>
     *           The route of the packet in the physical network.
     *           ({@link DataFlow#getPhysicalRoute()})
     *         </li>
     *         <li>
     *           Statistics information. ({@link DataFlow#getStatistics()})
     *         </li>
     *         <li>
     *           Averaged statistics information.
     *           ({@link DataFlow#getAveragedStatistics()})
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@link DataFlowMode#DETAIL}
     *     <dd>
     *       Indicates that detailed information is required.
     *       <p>
     *         If this mode is specified, all attributes in {@link DataFlow}
     *         are filled if available. {@link DataFlow#getStatistics()}
     *         returns statistics cached in the statistics manager, which is
     *         updated every 10 seconds.
     *         {@link DataFlow#getAveragedStatistics()} returns averaged
     *         statistics per second if available.
     *       </p>
     *
     *     <dt style="font-weight: bold;">{@link DataFlowMode#SUMMARY}
     *     <dd>
     *       Same as {@link DataFlowMode#DETAIL}, but always make requests to
     *       physical switches to get flow statistics.
     *   </dl>
     * @param interval
     *    Time interval in seconds for retrieving the average statistics.
     *    Specifying zero or a negative value is treated as if 10 is
     *    specified.
     *    Note that this value is just a hint for determining the measurement
     *    period. So the actual measurement period may differ from the
     *    specified value.
     *    This value is ignored if {@link DataFlowMode#SUMMARY} is specified
     *    to {@code mode}.
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
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    DataFlow getDataFlow(VTenantPath path, long flowId, DataFlowMode mode,
                         int interval)
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
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    int getDataFlowCount(VTenantPath path) throws VTNException;

    /**
     * Return a list of flow conditions configured in the container.
     *
     * @return  A list of {@link FlowCondition} instances corresponding to
     *          all flow conditions configured in the container.
     *          An empty list is returned if no flow condition is configured.
     * @throws VTNException  An error occurred.
     *   The following are the main {@code StatusCode} set in {@link Status}
     *   delivered by the exception.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    List<FlowCondition> getFlowConditions() throws VTNException;

    /**
     * Return information about the flow condition specified by the name.
     *
     * @param name  The name of the flow condition.
     * @return  A {@link FlowCondition} instance which represents information
     *          about the flow condition specified by {@code name}.
     * @throws VTNException  An error occurred.
     *   The following are the main {@code StatusCode} set in {@link Status}
     *   delivered by the exception.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.BADREQUEST}
     *     <dd>{@code null} is passed to {@code name}.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>Flow condition specified by {@code name} does not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    FlowCondition getFlowCondition(String name) throws VTNException;

    /**
     * Create or modify the flow condition.
     *
     * <ul>
     *   <li>
     *     If the flow condition specified by {@code name} does not exist,
     *     a new flow condition will be associated with {@code name} in the
     *     container.
     *   </li>
     *   <li>
     *     If the flow condition specified by {@code name} already exists,
     *     it will be modified as specified by {@code fcond}.
     *   </li>
     * </ul>
     *
     * @param name
     *   The name of the flow condition.
     *   <p style="margin-left: 1em;">
     *     The name of the flow condition must be a string that meets the
     *     following conditions.
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
     * @param fcond
     *   A {@link FlowCondition} instance which specifies the configuration
     *   of the flow condition.
     *   <ul>
     *     <li>
     *       The name of the flow condition configured in {@code fcond} is
     *       always ignored. The name is determined by {@code name} argument.
     *     </li>
     *     <li>
     *       Each {@link FlowMatch} instance in {@code fcond} must have an
     *       unique match index.
     *     </li>
     *     <li>
     *       The flow condition will match every packet if {@code fcond} does
     *       not contain {@link FlowMatch} instance.
     *     </li>
     *     <li>
     *       {@code null} implies an empty flow condition that matches every
     *       packet.
     *     </li>
     *   </ul>
     * @return
     *   A {@link UpdateType} object which represents the result of the
     *   operation is returned.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code UpdateType.ADDED}
     *     <dd>
     *       Flow condition was newly configured in the container.
     *
     *     <dt style="font-weight: bold;">{@code UpdateType.CHANGED}
     *     <dd>
     *       Configuration of existing flow condition in the container was
     *       changed.
     *
     *     <dt style="font-weight: bold;">{@code null}
     *     <dd>
     *       Configuration of existing flow condition was not changed.
     *   </dl>
     * @throws VTNException  An error occurred.
     *   The following are the main {@code StatusCode} set in {@link Status}
     *   delivered by the exception.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.BADREQUEST}
     *     <dd>
     *       <ul style="padding-left: 1em;">
     *         <li>
     *           {@code null} is passed to {@code name}.
     *         </li>
     *         <li>
     *           Incorrect flow condition name is passed to {@code name}.
     *         </li>
     *         <li>
     *           A {@link FlowCondition} instance passed to {@code fcond}
     *           contains invalid configuration.
     *         </li>
     *         <li>
     *           Duplicate match index is configured in {@link FlowMatch}
     *           instance in {@code fcond}.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    UpdateType setFlowCondition(String name, FlowCondition fcond)
        throws VTNException;

    /**
     * Remove the flow condition specified by the name.
     *
     * @param name  The name of the flow condition to be removed.
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
     *     <dd>{@code null} is passed to {@code name}.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>Flow condition specified by {@code name} does not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    Status removeFlowCondition(String name);

    /**
     * Remove all the flow conditions.
     *
     * @return
     *   A {@link Status} object which represents the result of the operation
     *   is returned.
     *   <p>
     *     <strong>{@code StatusCode.SUCCESS}</strong> is set in a returned
     *     object if at least one flow condition was removed.
     *     {@code null} is returned if no flow condition is present.
     *     Otherwise a {@code StatusCode} which indicates the cause of error
     *     is set in a returned {@link Status} object.
     *     The following are the main {@code StatusCode} configured in
     *     {@link Status}.
     *   </p>
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Lithium
     */
    Status clearFlowCondition();

    /**
     * Return a {@link FlowMatch} instance configured in the flow condition
     * specified by the flow condition name and match index.
     *
     * @param name   The name of the flow condition.
     * @param index  The match index that specifies flow match condition
     *               in the flow condition.
     * @return  A {@link FlowMatch} instance which represents a flow match
     *          condition.
     *          {@code null} is returned if no flow match condition is
     *          configured at the specified match index.
     * @throws VTNException  An error occurred.
     *   The following are the main {@code StatusCode} set in {@link Status}
     *   delivered by the exception.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.BADREQUEST}
     *     <dd>{@code null} is passed to {@code name}.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>Flow condition specified by {@code name} does not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    FlowMatch getFlowConditionMatch(String name, int index)
        throws VTNException;

    /**
     * Configure a flow match condition into the flow condition specified
     * by the flow condition name and match index.
     *
     * <ul>
     *   <li>
     *     If no flow match condition is associated with the specified match
     *     index in the flow condition, a new flow match condition will be
     *     associated with the specified match index in the flow condition.
     *   </li>
     *   <li>
     *     If the flow match condition is already associated with the
     *     specified match index in the flow condition, the contents of the
     *     flow match condition will be modified as specified by {@code match}.
     *   </li>
     * </ul>
     *
     * @param name   The name of the flow condition.
     * @param index
     *   The match index that specifies flow match condition in the flow
     *   condition.
     *   <ul>
     *     <li>
     *       The range of value that can be specified is from
     *       <strong>1</strong> to <strong>65535</strong>.
     *     </li>
     *   </ul>
     * @param match
     *   A {@link FlowMatch} instance which represents a flow match condition
     *   to be configured.
     *   <ul>
     *     <li>
     *       The match index configured in {@code match} is always ignored.
     *       The match index is determined by {@code index} argument.
     *     </li>
     *     <li>
     *       {@code null} implies an empty flow match condition that matches
     *       every packet.
     *     </li>
     *   </ul>
     * @return
     *   A {@link UpdateType} object which represents the result of the
     *   operation is returned.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code UpdateType.ADDED}
     *     <dd>
     *       Flow match condition was newly configured in the flow condition.
     *
     *     <dt style="font-weight: bold;">{@code UpdateType.CHANGED}
     *     <dd>
     *       Configuration of existing flow match condition in the flow
     *       condition was changed.
     *
     *     <dt style="font-weight: bold;">{@code null}
     *     <dd>
     *       Configuration of existing flow match condition was not changed.
     *   </dl>
     * @throws VTNException  An error occurred.
     *   The following are the main {@code StatusCode} set in {@link Status}
     *   delivered by the exception.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.BADREQUEST}
     *     <dd>
     *       <ul style="padding-left: 1em;">
     *         <li>
     *           {@code null} is passed to {@code name}.
     *         </li>
     *         <li>
     *           Match index specified by {@code index} is out of valid range.
     *         </li>
     *         <li>
     *           A {@link FlowMatch} instance passed to {@code match} contains
     *           invalid configuration.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    UpdateType setFlowConditionMatch(String name, int index, FlowMatch match)
        throws VTNException;

    /**
     * Remove the flow match condition specified by the flow condition name
     * and match index.
     *
     * @param name   The name of the flow condition.
     * @param index  The match index that specifies flow match condition
     *               in the flow condition.
     * @return
     *   A {@link Status} object which represents the result of the operation
     *   is returned.
     *   <p>
     *     Upon successful completion,
     *     <strong>{@code StatusCode.SUCCESS}</strong> is set in a returned
     *     object. {@code null} is returned if the flow match condition
     *     specified by {@code index} does not exist in the flow condition
     *     specified by {@code name}.
     *     Otherwise a {@code StatusCode} which indicates the cause
     *     of error is set in a returned {@link Status} object.
     *     The following are the main {@code StatusCode} configured in
     *     {@link Status}.
     *   </p>
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.BADREQUEST}
     *     <dd>{@code null} is passed to {@code name}.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>Flow condition specified by {@code name} does not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    Status removeFlowConditionMatch(String name, int index);

    /**
     * Return a list of path policy identifiers present in the container.
     *
     * @return  A list of {@link Integer} instances corresponding to all the
     *          path policies present in the container.
     * @throws VTNException  An error occurred.
     *   The following are the main {@code StatusCode} set in {@link Status}
     *   delivered by the exception.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    List<Integer> getPathPolicyIds() throws VTNException;

    /**
     * Return the configuration of the specified path policy.
     *
     * @param id  The identifier of the path policy.
     * @return  A {@link PathPolicy} instance which contains the configuration
     *          of the specified path policy.
     * @throws VTNException  An error occurred.
     *   The following are the main {@code StatusCode} set in {@link Status}
     *   delivered by the exception.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>Path policy specified by {@code id} does not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    PathPolicy getPathPolicy(int id) throws VTNException;

    /**
     * Create or modify the path policy.
     *
     * <ul>
     *   <li>
     *     If the path policy specified by {@code id} does not exist,
     *     a new path policy will be associated with {@code id} in the
     *     container.
     *   </li>
     *   <li>
     *     If the path policy specified by {@code id} already exists,
     *     it will be modified as specified by {@code policy}.
     *   </li>
     * </ul>
     *
     * @param id
     *   The identifier of the path policy.
     *   <p style="margin-left: 1em;">
     *     The range of value that can be specified is from
     *     <strong>1</strong> to <strong>3</strong>.
     *   </p>
     * @param policy
     *   A {@link PathPolicy} instance which specifies the configuration of
     *   the path policy.
     *   <ul>
     *     <li>
     *       The identifier of the path policy configured in {@code policy} is
     *       always ignored. The identifier is determined by {@code id}
     *       argument.
     *     </li>
     *   </ul>
     * @return
     *   A {@link UpdateType} object which represents the result of the
     *   operation is returned.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code UpdateType.ADDED}
     *     <dd>
     *       Path policy was newly configured in the container.
     *
     *     <dt style="font-weight: bold;">{@code UpdateType.CHANGED}
     *     <dd>
     *       Configuration of existing path policy in the container was
     *       changed.
     *
     *     <dt style="font-weight: bold;">{@code null}
     *     <dd>
     *       Configuration of existing path policy was not changed.
     *   </dl>
     * @throws VTNException  An error occurred.
     *   The following are the main {@code StatusCode} set in {@link Status}
     *   delivered by the exception.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.BADREQUEST}
     *     <dd>
     *       <ul style="padding-left: 1em;">
     *         <li>
     *           Invalid identifier is passed to {@code id}.
     *         </li>
     *         <li>
     *           {@code null} is passed to {@code policy}.
     *         </li>
     *         <li>
     *           A {@link PathPolicy} instance passed to {@code policy}
     *           contains invalid configuration.
     *         </li>
     *         <li>
     *           Duplicate {@link PortLocation} instance is configured in
     *           {@link PathCost} instance in {@code policy}.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    UpdateType setPathPolicy(int id, PathPolicy policy) throws VTNException;

    /**
     * Remove the path policy specified by the identifier.
     *
     * @param id  The identifier of the path policy to be removed.
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
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>Path policy specified by {@code id} does not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    Status removePathPolicy(int id);

    /**
     * Remove all the path policies.
     *
     * @return
     *   A {@link Status} object which represents the result of the operation
     *   is returned.
     *   <p>
     *     <strong>{@code StatusCode.SUCCESS}</strong> is set in a returned
     *     object if at least one path policy was removed.
     *     {@code null} is returned if no path policy is present.
     *     Otherwise a {@code StatusCode} which indicates the cause of error
     *     is set in a returned {@link Status} object.
     *     The following are the main {@code StatusCode} configured in
     *     {@link Status}.
     *   </p>
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Lithium
     */
    Status clearPathPolicy();

    /**
     * Return the default link cost configured in the specified path policy.
     *
     * @param id  The identifier of the path policy.
     * @return    The default link cost configured in the specified path policy
     *            is returned. {@link PathPolicy#COST_UNDEF} means that the
     *            default cost should be determined by link speed.
     * @throws VTNException  An error occurred.
     *   The following are the main {@code StatusCode} set in {@link Status}
     *   delivered by the exception.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>Path policy specified by {@code id} does not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    long getPathPolicyDefaultCost(int id) throws VTNException;

    /**
     * Change the default link cost for the specified path policy.
     *
     * @param id    The identifier of the path policy.
     * @param cost
     *   The default cost value to be set.
     *   <ul>
     *     <li>
     *       The value must be {@link PathPolicy#COST_UNDEF} or a positive
     *       value.
     *     </li>
     *     <li>
     *       Specifying {@link PathPolicy#COST_UNDEF} means that the default
     *       cost should be determined by link speed.
     *       If {@link PathPolicy#COST_UNDEF} is specified, the default cost
     *       is calculated by the following formula.
     *       <blockquote>
     *         10,000,000,000,000 / (<i>link speed (bps)</i>)
     *       </blockquote>
     *     </li>
     *   </ul>
     * @return  {@code true} if the default cost was changed.
     *          {@code false} if not changed.
     * @throws VTNException  An error occurred.
     *   The following are the main {@code StatusCode} set in {@link Status}
     *   delivered by the exception.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.BADREQUEST}
     *     <dd>Invalid cost value is passed to {@code cost}.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>Path policy specified by {@code id} does not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    boolean setPathPolicyDefaultCost(int id, long cost) throws VTNException;

    /**
     * Return the cost of transmitting a packet from the specified switch port
     * configured in the specified path policy.
     *
     * <p>
     *   This method searches for the link cost associated with the specified
     *   {@link PortLocation} instance in the specified path policy.
     * </p>
     *
     * @param id    The identifier of the path policy.
     * @param ploc  A {@link PortLocation} instance which specifies the
     *              location of the physical switch port.
     * @return  The cost of transmitting a packet from the specified physical
     *          switch port. {@link PathPolicy#COST_UNDEF} is returned if
     *          {@code ploc} is not configured in the specified path policy.
     * @throws VTNException  An error occurred.
     *   The following are the main {@code StatusCode} set in {@link Status}
     *   delivered by the exception.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>Path policy specified by {@code id} does not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    long getPathPolicyCost(int id, PortLocation ploc) throws VTNException;

    /**
     * Associate the cost of transmitting a packet with the specified switch
     * port in the specified path policy.
     *
     * <p>
     *   The specified cost value is used when a packet is transmitted from the
     *   switch port specified by a {@link PortLocation} instance.
     * </p>
     *
     * @param id    The identifier of the path policy to be removed.
     * @param ploc  A {@link PortLocation} instance which specifies the
     *              location of the physical switch port.
     * @param cost
     *   The cost of transmitting a packet.
     *   <ul>
     *     <li>
     *       The cost value must be greater than zero.
     *     </li>
     *   </ul>
     * @return
     *   A {@link UpdateType} object which represents the result of the
     *   operation is returned.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code UpdateType.ADDED}
     *     <dd>
     *       A {@link PortLocation} instance passed to {@code ploc} was
     *       newly configured in the specified path policy.
     *
     *     <dt style="font-weight: bold;">{@code UpdateType.CHANGED}
     *     <dd>
     *       The cost associated with {@code ploc} in the specified path policy
     *       was changed.
     *
     *     <dt style="font-weight: bold;">{@code null}
     *     <dd>
     *       The cost associated with {@code ploc} in the specified path policy
     *       was not changed.
     *   </dl>
     * @throws VTNException  An error occurred.
     *   The following are the main {@code StatusCode} set in {@link Status}
     *   delivered by the exception.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.BADREQUEST}
     *     <dd>
     *       <ul style="padding-left: 1em;">
     *         <li>
     *           {@code null} is passed to {@code ploc}.
     *         </li>
     *         <li>
     *           A {@link PortLocation} instance passed to {@code ploc}
     *           contains invalid configuration.
     *         </li>
     *         <li>
     *           A value less than 1 is passed to {@code cost}.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    UpdateType setPathPolicyCost(int id, PortLocation ploc, long cost)
        throws VTNException;

    /**
     * Remove the cost associated with the specified switch port in the
     * specified path policy.
     *
     * @param id    The identifier of the path policy to be removed.
     * @param ploc  A {@link PortLocation} instance which specifies the
     *              location of the physical switch port.
     * @return
     *   A {@link Status} object which represents the result of the operation
     *   is returned.
     *   <p>
     *     Upon successful completion,
     *     <strong>{@code StatusCode.SUCCESS}</strong> is set in a returned
     *     object. {@code null} is returned if the switch port location
     *     specified {@code ploc} is not configured in the path policy
     *     specified by {@code id}.
     *     Otherwise a {@code StatusCode} which indicates the cause
     *     of error is set in a returned {@link Status} object.
     *     The following are the main {@code StatusCode} configured in
     *     {@link Status}.
     *   </p>
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.BADREQUEST}
     *     <dd>{@code null} is passed to {@code ploc}.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>Path policy specified by {@code id} does not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    Status removePathPolicyCost(int id, PortLocation ploc);

    /**
     * Return a list of container path maps configured in the container.
     *
     * @return  A list of {@link PathMap} instances corresponding to all
     *          container path maps configured in the container.
     *          An empty list is returned if no container path map is
     *          configured.
     * @throws VTNException  An error occurred.
     *   The following are the main {@code StatusCode} set in {@link Status}
     *   delivered by the exception.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    List<PathMap> getPathMaps() throws VTNException;

    /**
     * Return information about the container path map specified by the index
     * number.
     *
     * @param index  The index value which specifies the path map in the
     *               container.
     * @return  A {@link PathMap} instance corresponding to the specified
     *          container path map is returned.
     *          {@code null} is returned if the specified path map does not
     *          exist in the container.
     * @throws VTNException  An error occurred.
     *   The following are the main {@code StatusCode} set in {@link Status}
     *   delivered by the exception.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    PathMap getPathMap(int index) throws VTNException;

    /**
     * Create or modify the container path map specified by the index number.
     *
     * <ul>
     *   <li>
     *     If the container path map specified by {@code index} does not exist,
     *     a new container path map will be associated with {@code index} in
     *     the container.
     *   </li>
     *   <li>
     *     If the container path map specified by {@code index} already exists,
     *     it will be modified as specified by {@code pmap}.
     *   </li>
     * </ul>
     *
     * @param index
     *   The index value which specifies the path map in the container.
     *   <ul>
     *     <li>
     *       The range of value that can be specified is from
     *       <strong>1</strong> to <strong>65535</strong>.
     *     </li>
     *     <li>
     *       This value is used to determine order of evaluation.
     *       Path maps in the container path map list are evaluated in
     *       ascending order of indices assigned to path maps.
     *     </li>
     *   </ul>
     * @param pmap
     *   A {@link PathMap} instance which specifies the configuration of the
     *   path map.
     *   <ul>
     *     <li>
     *       The index of the path map configured in {@code pmap} is always
     *       ignored. The index number is determined by {@code index} argument.
     *     </li>
     *     <li>
     *       Note that this API does not check whether the
     *       {@link PathMap#getFlowConditionName() flow condition}
     *       configured in {@code pmap} actually exists or not.
     *       The path map will be invalidated if the specified flow condition
     *       does not exist.
     *     </li>
     *     <li>
     *     <li>
     *       Note that this API does not check whether the
     *       {@link PathMap#getPathPolicyId() path policy} configured in
     *       {@code pmap} actually exists or not.
     *       The path map will be invalidated if the specified path policy
     *       does not exist.
     *     </li>
     *   </ul>
     * @return
     *   A {@link UpdateType} object which represents the result of the
     *   operation is returned.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code UpdateType.ADDED}
     *     <dd>
     *       Path map was newly configured in the container.
     *
     *     <dt style="font-weight: bold;">{@code UpdateType.CHANGED}
     *     <dd>
     *       Configuration of existing path map in the container was
     *       changed.
     *
     *     <dt style="font-weight: bold;">{@code null}
     *     <dd>
     *       Configuration of existing path map was not changed.
     *   </dl>
     * @throws VTNException  An error occurred.
     *   The following are the main {@code StatusCode} set in {@link Status}
     *   delivered by the exception.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.BADREQUEST}
     *     <dd>
     *       <ul style="padding-left: 1em;">
     *         <li>{@code null} is passed to {@code pmap}.</li>
     *         <li>
     *           The index number specified by {@code index} is out of valid
     *           range.
     *         </li>
     *         <li>
     *           A {@link PathMap} instance passed to {@code pmap}
     *           contains invalid configuration.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    UpdateType setPathMap(int index, PathMap pmap) throws VTNException;

    /**
     * Remove the container path map specified by the index number.
     *
     * @param index  The index value which specifies the path map in the
     *               container.
     * @return
     *   A {@link Status} object which represents the result of the operation
     *   is returned.
     *   <p>
     *     Upon successful completion,
     *     <strong>{@code StatusCode.SUCCESS}</strong> is set in a returned
     *     object. {@code null} is returned if the specified path map does not
     *     exist. Otherwise a {@code StatusCode} which indicates the cause
     *     of error is set in a returned {@link Status} object.
     *     The following are the main {@code StatusCode} configured in
     *     {@link Status}.
     *   </p>
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    Status removePathMap(int index);

    /**
     * Remove all the container path maps.
     *
     * @return
     *   A {@link Status} object which represents the result of the operation
     *   is returned.
     *   <p>
     *     <strong>{@code StatusCode.SUCCESS}</strong> is set in a returned
     *     object if at least one container path map was removed.
     *     {@code null} is returned if no path map is present in the container
     *     path map list.
     *     Otherwise a {@code StatusCode} which indicates the cause of error
     *     is set in a returned {@link Status} object.
     *     The following are the main {@code StatusCode} configured in
     *     {@link Status}.
     *   </p>
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Lithium
     */
    Status clearPathMap();

    /**
     * Return a list of VTN path maps configured in the VTN.
     *
     * @param path  A {@link VTenantPath} object that specifies the position
     *              of the VTN.
     * @return  A list of {@link PathMap} instances corresponding to all
     *          VTN path maps configured in the VTN.
     *          An empty list is returned if no VTN path map is configured.
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
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    List<PathMap> getPathMaps(VTenantPath path) throws VTNException;

    /**
     * Return information about the VTN path map specified by the index number.
     *
     * @param path   A {@link VTenantPath} object that specifies the position
     *               of the VTN.
     * @param index  The index value which specifies the path map in the VTN.
     * @return  A {@link PathMap} instance corresponding to the specified
     *          VTN path map is returned.
     *          {@code null} is returned if the specified path map does not
     *          exist in the VTN.
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
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    PathMap getPathMap(VTenantPath path, int index) throws VTNException;

    /**
     * Create or modify the VTN path map specified by the index number.
     *
     * <ul>
     *   <li>
     *     If the VTN path map specified by {@code index} does not exist,
     *     a new VTN path map will be associated with {@code index} in the VTN.
     *   </li>
     *   <li>
     *     If the VTN path map specified by {@code index} already exists,
     *     it will be modified as specified by {@code pmap}.
     *   </li>
     * </ul>
     *
     * @param path   A {@link VTenantPath} object that specifies the position
     *               of the VTN.
     * @param index
     *   The index value which specifies the path map in the VTN.
     *   <ul>
     *     <li>
     *       The range of value that can be specified is from
     *       <strong>1</strong> to <strong>65535</strong>.
     *     </li>
     *     <li>
     *       This value is used to determine order of evaluation.
     *       Path maps in the VTN path map list are evaluated in ascending
     *       order of indices assigned to path maps.
     *     </li>
     *   </ul>
     * @param pmap
     *   A {@link PathMap} instance which specifies the configuration of the
     *   path map.
     *   <ul>
     *     <li>
     *       The index of the path map configured in {@code pmap} is always
     *       ignored. The index number is determined by {@code index} argument.
     *     </li>
     *     <li>
     *       Note that this API does not check whether the
     *       {@link PathMap#getFlowConditionName() flow condition}
     *       configured in {@code pmap} actually exists or not.
     *       The path map will be invalidated if the specified flow condition
     *       does not exist.
     *     </li>
     *     <li>
     *     <li>
     *       Note that this API does not check whether the
     *       {@link PathMap#getPathPolicyId() path policy} configured in
     *       {@code pmap} actually exists or not.
     *       The path map will be invalidated if the specified path policy
     *       does not exist.
     *     </li>
     *   </ul>
     * @return
     *   A {@link UpdateType} object which represents the result of the
     *   operation is returned.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code UpdateType.ADDED}
     *     <dd>
     *       Path map was newly configured in the VTN.
     *
     *     <dt style="font-weight: bold;">{@code UpdateType.CHANGED}
     *     <dd>
     *       Configuration of existing path map in the VTN was changed.
     *
     *     <dt style="font-weight: bold;">{@code null}
     *     <dd>
     *       Configuration of existing path map was not changed.
     *   </dl>
     * @throws VTNException  An error occurred.
     *   The following are the main {@code StatusCode} set in {@link Status}
     *   delivered by the exception.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.BADREQUEST}
     *     <dd>
     *       <ul style="padding-left: 1em;">
     *         <li>{@code null} is passed to {@code path} or {@code pmap}.</li>
     *         <li>
     *           {@code null} is configured in {@code path} for the
     *           {@linkplain VTenantPath#getTenantName() VTN name}.
     *         </li>
     *         <li>
     *           The index number specified by {@code index} is out of valid
     *           range.
     *         </li>
     *         <li>
     *           A {@link PathMap} instance passed to {@code pmap}
     *           contains invalid configuration.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    UpdateType setPathMap(VTenantPath path, int index, PathMap pmap)
        throws VTNException;

    /**
     * Remove the VTN path map specified by the index number.
     *
     * @param path   A {@link VTenantPath} object that specifies the position
     *               of the VTN.
     * @param index  The index value which specifies the path map in the VTN.
     * @return
     *   A {@link Status} object which represents the result of the operation
     *   is returned.
     *   <p>
     *     Upon successful completion,
     *     <strong>{@code StatusCode.SUCCESS}</strong> is set in a returned
     *     object. {@code null} is returned if the specified path map does not
     *     exist. Otherwise a {@code StatusCode} which indicates the cause
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
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    Status removePathMap(VTenantPath path, int index);

    /**
     * Remove all the VTN path maps configured in the specified VTN.
     *
     * @param path   A {@link VTenantPath} object that specifies the position
     *               of the VTN.
     * @return
     *   A {@link Status} object which represents the result of the operation
     *   is returned.
     *   <p>
     *     <strong>{@code StatusCode.SUCCESS}</strong> is set in a returned
     *     object if at least one VTN path map was removed.
     *     {@code null} is returned if no path map is present in the VTN
     *     path map list.
     *     Otherwise a {@code StatusCode} which indicates the cause of error
     *     is set in a returned {@link Status} object.
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
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Lithium
     */
    Status clearPathMap(VTenantPath path);

    /**
     * Return a list of
     * {@linkplain <a href="flow/filter/package-summary.html">flow filters</a>}
     * configured in the specified flow filter list.
     *
     * @param fid  A {@link FlowFilterId} instance which specifies the
     *             flow filter list in the virtual node.
     * @return  A list of {@link FlowFilter} instances corresponding to all
     *          flow filters configured in the list specified by {@code fid}.
     * @throws VTNException  An error occurred.
     *   The following are the main {@code StatusCode} set in {@link Status}
     *   delivered by the exception.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.BADREQUEST}
     *     <dd>
     *       <ul style="padding-left: 1em;">
     *         <li>{@code null} is passed to {@code fid}.</li>
     *         <li>
     *           The location of the target virtual node is not configured
     *           in {@code fid}.
     *         </li>
     *         <li>
     *           The location of the target virtual node configured in
     *           {@code fid} contains invalid value.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>
     *       The target virtual node configured in {@code fid} does not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    List<FlowFilter> getFlowFilters(FlowFilterId fid) throws VTNException;

    /**
     * Return information about the
     * {@linkplain <a href="flow/filter/package-summary.html">flow filter</a>}
     * specified by the index number.
     *
     * @param fid    A {@link FlowFilterId} instance which specifies the
     *               flow filter list in the virtual node.
     * @param index  The index value which specifies the flow filter in the
     *               flow filter list specified by {@code fid}.
     * @return  A {@link FlowFilter} instance corresponding to the flow filter
     *          specified by {@code fid} and {@code index}.
     *          {@code null} is returned if the specified flow filter does not
     *          exist.
     * @throws VTNException  An error occurred.
     *   The following are the main {@code StatusCode} set in {@link Status}
     *   delivered by the exception.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.BADREQUEST}
     *     <dd>
     *       <ul style="padding-left: 1em;">
     *         <li>{@code null} is passed to {@code fid}.</li>
     *         <li>
     *           The location of the target virtual node is not configured
     *           in {@code fid}.
     *         </li>
     *         <li>
     *           The location of the target virtual node configured in
     *           {@code fid} contains invalid value.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>
     *       The target virtual node configured in {@code fid} does not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    FlowFilter getFlowFilter(FlowFilterId fid, int index) throws VTNException;

    /**
     * Create or modify the
     * {@linkplain <a href="flow/filter/package-summary.html">flow filter</a>}
     * specified by the index number.
     *
     * <ul>
     *   <li>
     *     If the flow filter specified by {@code index} does not exist in the
     *     flow filter list specified by {@code fid}, a new flow filter will
     *     be associated with {@code index} in the specified list.
     *   </li>
     *   <li>
     *     If the flow filter specified by {@code index} already exists in the
     *     flow filter list specified by {@code fid}, it will be modified as
     *     specified by {@code filter}.
     *   </li>
     * </ul>
     *
     * @param fid    A {@link FlowFilterId} instance which specifies the
     *               flow filter list in the virtual node.
     * @param index
     *   The index value which specifies the flow filter in the flow filter
     *   list.
     *   <ul>
     *     <li>
     *       The range of value that can be specified is from
     *       <strong>1</strong> to <strong>65535</strong>.
     *     </li>
     *     <li>
     *       This value is used to determine order of evaluation.
     *       Flow filters in the list specified by {@code fid} are evaluated in
     *       ascending order of indices assigned to flow filters.
     *     </li>
     *   </ul>
     * @param filter
     *   A {@link FlowFilter} instance which specifies the configuration of the
     *   flow filter.
     *   <ul>
     *     <li>
     *       The index of the flow filter configured in {@code filter} is
     *       always ignored. The index number is determined by {@code index}
     *       argument.
     *     </li>
     *     <li>
     *       Note that this API does not check whether the
     *       {@link FlowFilter#getFlowConditionName() flow condition}
     *       configured in {@code filter} actually exists or not.
     *       The flow filter will be invalidated if the specified flow
     *       condition does not exist.
     *     </li>
     *     <li>
     *     <li>
     *       Note that this API does not check whether the destination
     *       {@linkplain <a href="package-summary.html#vInterface">virtual interface</a>}
     *       specified by the
     *       {@linkplain <a href="flow/filter/package-summary.html#type.REDIRECT">REDIRECT flow filter</a>}
     *       configured in {@code filter} actually exists or not.
     *       Note that every packet redirected by the flow filter is discarded
     *       if the destination virtual interface specified by {@code filter}
     *       does not exist.
     *     </li>
     *   </ul>
     * @return
     *   A {@link UpdateType} object which represents the result of the
     *   operation is returned.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code UpdateType.ADDED}
     *     <dd>
     *       Flow filter was newly configured in the specified flow filter
     *       list.
     *
     *     <dt style="font-weight: bold;">{@code UpdateType.CHANGED}
     *     <dd>
     *       Configuration of existing flow filter in the specified
     *       flow filter list was changed.
     *
     *     <dt style="font-weight: bold;">{@code null}
     *     <dd>
     *       Configuration of existing flow filter was not changed.
     *   </dl>
     * @throws VTNException  An error occurred.
     *   The following are the main {@code StatusCode} set in {@link Status}
     *   delivered by the exception.
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.BADREQUEST}
     *     <dd>
     *       <ul style="padding-left: 1em;">
     *         <li>{@code null} is passed to {@code fid} or {@code filter}</li>
     *         <li>
     *           The location of the target virtual node is not configured
     *           in {@code fid}.
     *         </li>
     *         <li>
     *           The location of the target virtual node configured in
     *           {@code fid} contains invalid value.
     *         </li>
     *         <li>
     *           The index number specified by {@code index} is out of valid
     *           range.
     *         </li>
     *         <li>
     *           A {@link FlowFilter} instance passed to {@code pmap}
     *           contains invalid configuration.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>
     *       The target virtual node configured in {@code fid} does not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    UpdateType setFlowFilter(FlowFilterId fid, int index, FlowFilter filter)
        throws VTNException;

    /**
     * Remove the
     * {@linkplain <a href="flow/filter/package-summary.html">flow filter</a>}
     * specified by the index number.
     *
     * @param fid    A {@link FlowFilterId} instance which specifies the
     *               flow filter list in the virtual node.
     * @param index  The index value which specifies the flow filter in the
     *               flow filter list specified by {@code fid}.
     * @return
     *   A {@link Status} object which represents the result of the operation
     *   is returned.
     *   <p>
     *     Upon successful completion,
     *     <strong>{@code StatusCode.SUCCESS}</strong> is set in a returned
     *     object. {@code null} is returned if the specified flow filter does
     *     not exist. Otherwise a {@code StatusCode} which indicates the cause
     *     of error is set in a returned {@link Status} object.
     *     The following are the main {@code StatusCode} configured in
     *     {@link Status}.
     *   </p>
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.BADREQUEST}
     *     <dd>
     *       <ul style="padding-left: 1em;">
     *         <li>{@code null} is passed to {@code fid}.</li>
     *         <li>
     *           The location of the target virtual node is not configured
     *           in {@code fid}.
     *         </li>
     *         <li>
     *           The location of the target virtual node configured in
     *           {@code fid} contains invalid value.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>
     *       The target virtual node configured in {@code fid} does not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    Status removeFlowFilter(FlowFilterId fid, int index);

    /**
     * Remove all the
     * {@linkplain <a href="flow/filter/package-summary.html">flow filters</a>}
     * present in the specified flow filter list.
     *
     * @param fid  A {@link FlowFilterId} instance which specifies the
     *             flow filter list in the virtual node.
     * @return
     *   A {@link Status} object which represents the result of the operation
     *   is returned.
     *   <p>
     *     <strong>{@code StatusCode.SUCCESS}</strong> is set in a returned
     *     object if at least one flow filter in the list was removed.
     *     {@code null} is returned if no flow filter is present in the
     *     specified flow filter list.
     *     Otherwise a {@code StatusCode} which indicates the cause of error
     *     is set in a returned {@link Status} object.
     *     The following are the main {@code StatusCode} configured in
     *     {@link Status}.
     *   </p>
     *   <dl style="margin-left: 1em;">
     *     <dt style="font-weight: bold;">{@code StatusCode.BADREQUEST}
     *     <dd>
     *       <ul style="padding-left: 1em;">
     *         <li>{@code null} is passed to {@code fid}.</li>
     *         <li>
     *           The location of the target virtual node is not configured
     *           in {@code fid}.
     *         </li>
     *         <li>
     *           The location of the target virtual node configured in
     *           {@code fid} contains invalid value.
     *         </li>
     *       </ul>
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTFOUND}
     *     <dd>
     *       The target virtual node configured in {@code fid} does not exist.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.NOTACCEPTABLE}
     *     <dd>
     *       This service is associated with a non-default container.
     *
     *     <dt style="font-weight: bold;">{@code StatusCode.INTERNALERROR}
     *     <dd>Fatal internal error occurred in the VTN Manager.
     *   </dl>
     * @since  Helium
     */
    Status clearFlowFilter(FlowFilterId fid);
}
