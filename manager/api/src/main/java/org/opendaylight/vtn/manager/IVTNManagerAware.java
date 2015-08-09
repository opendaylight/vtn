/*
 * Copyright (c) 2013, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import org.opendaylight.controller.sal.core.UpdateType;

/**
 * {@code IVTNManagerAware} defines the listener interface that monitors
 * the status change of the VTN Manager inside the container.
 *
 * <p>
 *   Once an OSGi service which implements {@code IVTNManagerAware} is
 *   registered in the OSGi service registry, corresponding method is called
 *   when the management information about the VTN Manager is changed.
 * </p>
 */
public interface IVTNManagerAware {
    /**
     * Invoked when the information related to
     * {@linkplain <a href="package-summary.html#VTN">VTN</a>} inside the
     * container is changed.
     *
     * <p>
     *   If at least one VTN exists in the container at the time of registering
     *   {@code IVTNManagerAware} listener in the VTN Manager, then this
     *   method is called with information about each VTN so that the
     *   existence of these can be notified to listener.
     *   {@code type} is specified as {@link UpdateType#ADDED} in such cases.
     * </p>
     *
     * @param path     A {@link VTenantPath} object that specifies the
     *                 position of the VTN.
     * @param vtenant  A {@link VTenant} object which represents the VTN
     *                 information.
     * @param type
     *   An {@link UpdateType} object which indicates the type of modification
     *   is specified.
     *   <ul>
     *     <li>
     *       {@link UpdateType#ADDED} is specified if a new VTN has been
     *       created.
     *       <ul>
     *         <li>
     *           The position of the newly created VTN is passed to
     *           {@code path}.
     *         </li>
     *         <li>
     *           Information about the newly created VTN is passed to
     *           {@code vtenant}.
     *         </li>
     *       </ul>
     *     </li>
     *     <li>
     *       {@link UpdateType#CHANGED} is specified if information about the
     *       specified VTN has been modified.
     *       <ul>
     *         <li>
     *           The position of the modified VTN is passed to {@code path}.
     *         </li>
     *         <li>
     *           Information about the updated VTN is passed to
     *           {@code vtenant}.
     *         </li>
     *         <li>
     *           Change is notified in the following cases.
     *           <ul>
     *             <li>The VTN configuration has been changed.</li>
     *           </ul>
     *         </li>
     *       </ul>
     *     </li>
     *     <li>
     *       {@link UpdateType#REMOVED} is specified if the specified VTN
     *       has been removed.
     *       <ul>
     *         <li>
     *           The position of the removed VTN is passed to {@code path}.
     *         </li>
     *         <li>
     *           Information about the VTN just prior to its removal is
     *           passed to {@code vtenant}.
     *         </li>
     *       </ul>
     *     </li>
     *   </ul>
     */
    void vtnChanged(VTenantPath path, VTenant vtenant, UpdateType type);

    /**
     * Invoked when the information related to
     * {@linkplain <a href="package-summary.html#vBridge">vBridge</a>} inside
     * the container is changed.
     *
     * <p>
     *   If at least one vBridge exists in the container at the time of
     *   registering {@code IVTNManagerAware} listener in the VTN Manager,
     *   then this method is called with specifying information about each
     *   vBridge so that the existence of these can be notified to listener.
     * </p>
     * <ul>
     *   <li>
     *     It is guaranteed that
     *     {@link #vtnChanged(VTenantPath, VTenant, UpdateType)}, which
     *     notifies the existence of the
     *     {@linkplain <a href="package-summary.html#VTN">VTN</a>} to which
     *     this vBridge belongs, is called first.
     *   </li>
     *   <li>
     *     In that case {@link UpdateType#ADDED} is passed to {@code type}.
     *   </li>
     * </ul>
     *
     * @param path     A {@link VBridgePath} object that specifies the
     *                 position of the vBridge.
     * @param vbridge  A {@link VBridge} object which represents the vBridge
     *                 information.
     * @param type
     *   An {@link UpdateType} object which indicates the type of modification
     *   is specified.
     *   <ul>
     *     <li>
     *       {@link UpdateType#ADDED} is specified if a new vBridge has been
     *       created.
     *       <ul>
     *         <li>
     *           The position of the newly created vBridge is passed to
     *           {@code path}.
     *         </li>
     *         <li>
     *           Information about the newly created vBridge is passed to
     *           {@code vbridge}.
     *         </li>
     *       </ul>
     *     </li>
     *     <li>
     *       {@link UpdateType#CHANGED} is specified if information about the
     *       specified vBridge has been modified.
     *       <ul>
     *         <li>
     *           The position of the modified vBridge is passed to {@code path}.
     *         </li>
     *         <li>
     *           Information about the updated vBridge is passed to
     *           {@code vbridge}.
     *         </li>
     *         <li>
     *           Change is notified in the following cases.
     *           <ul>
     *             <li>The vBridge configuration has been changed.</li>
     *             <li>The status of the vBridge has been changed.</li>
     *           </ul>
     *         </li>
     *       </ul>
     *     </li>
     *     <li>
     *       {@link UpdateType#REMOVED} is specified if the specified vBridge
     *       has been removed.
     *       <ul>
     *         <li>
     *           The position of the removed vBridge is passed to {@code path}.
     *         </li>
     *         <li>
     *           Information about the vBridge just prior to its removal is
     *           passed to {@code vbridge}.
     *         </li>
     *       </ul>
     *     </li>
     *   </ul>
     */
    void vBridgeChanged(VBridgePath path, VBridge vbridge, UpdateType type);

    /**
     * Invoked when the information related to
     * {@linkplain <a href="package-summary.html#vTerminal">vTerminal</a>}
     * inside the container is changed.
     *
     * <p>
     *   If at least one vTerminal exists in the container at the time of
     *   registering {@code IVTNManagerAware} listener in the VTN Manager,
     *   then this method is called with specifying information about each
     *   vTerminal so that the existence of these can be notified to listener.
     * </p>
     * <ul>
     *   <li>
     *     It is guaranteed that
     *     {@link #vtnChanged(VTenantPath, VTenant, UpdateType)}, which
     *     notifies the existence of the
     *     {@linkplain <a href="package-summary.html#VTN">VTN</a>} to which
     *     this vTerminal belongs, is called first.
     *   </li>
     *   <li>
     *     In that case {@link UpdateType#ADDED} is passed to {@code type}.
     *   </li>
     * </ul>
     *
     * @param path   A {@link VTerminalPath} object that specifies the
     *               position of the vTerminal.
     * @param vterm  A {@link VTerminal} object which represents the vTerminal
     *               information.
     * @param type
     *   An {@link UpdateType} object which indicates the type of modification
     *   is specified.
     *   <ul>
     *     <li>
     *       {@link UpdateType#ADDED} is specified if a new vTerminal has been
     *       created.
     *       <ul>
     *         <li>
     *           The position of the newly created vTerminal is passed to
     *           {@code path}.
     *         </li>
     *         <li>
     *           Information about the newly created vTerminal is passed to
     *           {@code vterm}.
     *         </li>
     *       </ul>
     *     </li>
     *     <li>
     *       {@link UpdateType#CHANGED} is specified if information about the
     *       specified vTerminal has been modified.
     *       <ul>
     *         <li>
     *           The position of the modified vTerminal is passed to
     *           {@code path}.
     *         </li>
     *         <li>
     *           Information about the updated vTerminal is passed to
     *           {@code vterm}.
     *         </li>
     *         <li>
     *           Change is notified in the following cases.
     *           <ul>
     *             <li>The vTerminal configuration has been changed.</li>
     *             <li>The status of the vTerminal has been changed.</li>
     *           </ul>
     *         </li>
     *       </ul>
     *     </li>
     *     <li>
     *       {@link UpdateType#REMOVED} is specified if the specified vTerminal
     *       has been removed.
     *       <ul>
     *         <li>
     *           The position of the removed vTerminal is passed to {@code path}.
     *         </li>
     *         <li>
     *           Information about the vTerminal just prior to its removal is
     *           passed to {@code vterm}.
     *         </li>
     *       </ul>
     *     </li>
     *   </ul>
     * @since  Helium
     */
    void vTerminalChanged(VTerminalPath path, VTerminal vterm,
                          UpdateType type);

    /**
     * Invoked when the information related to the
     * {@linkplain <a href="package-summary.html#vInterface">virtual interface</a>}
     * configured in
     * {@linkplain <a href="package-summary.html#vBridge">vBridge</a>} inside
     * the container is changed.
     *
     * <p>
     *   If at least one vBridge interface exists in the container at the time
     *   of registering {@code IVTNManagerAware} listener in the VTN Manager,
     *   then this method is called with specifying information about each
     *   vBridge interface so that the existence of these can be notified to
     *   listener.
     * </p>
     * <ul>
     *   <li>
     *     It is guaranteed that
     *     {@link #vBridgeChanged(VBridgePath, VBridge, UpdateType)}, which
     *     notifies the existence of the vBridge to which this vBridge
     *     interface belongs, is called first.
     *   </li>
     *   <li>
     *     In that case {@link UpdateType#ADDED} is passed to {@code type}.
     *   </li>
     * </ul>
     *
     * @param path    A {@link VBridgeIfPath} object that specifies the
     *                position of the vBridge interface.
     * @param viface  A {@link VInterface} object which represents the vBridge
     *                interface information.
     * @param type
     *   An {@link UpdateType} object which indicates the type of modification
     *   is specified.
     *   <ul>
     *     <li>
     *       {@link UpdateType#ADDED} is specified if a new vBridge interface
     *       has been created.
     *       <ul>
     *         <li>
     *           The position of the newly created vBridge interface is
     *           passed to {@code path}.
     *         </li>
     *         <li>
     *           Information about the newly created vBridge interface is
     *           passed to {@code viface}.
     *         </li>
     *       </ul>
     *     </li>
     *     <li>
     *       {@link UpdateType#CHANGED} is specified if information about the
     *       specified vBridge interface has been modified.
     *       <ul>
     *         <li>
     *           The position of the modified vBridge interface is specified
     *           to {@code path}.
     *         </li>
     *         <li>
     *           Information about the updated vBridge interface is specified
     *           to {@code viface}.
     *         </li>
     *         <li>
     *           Change is notified in the following cases.
     *           <ul>
     *             <li>
     *               The vBridge interface configuration has been changed.
     *             </li>
     *             <li>
     *               The status of the vBridge interface has been changed.
     *             </li>
     *           </ul>
     *         </li>
     *       </ul>
     *     </li>
     *     <li>
     *       {@link UpdateType#REMOVED} is specified if the specified vBridge
     *       interface has been removed.
     *       <ul>
     *         <li>
     *           The position of the removed vBridge interface is passed to
     *           {@code path}.
     *         </li>
     *         <li>
     *           Information about the vBridge interface just prior to its
     *           removal is passed to {@code viface}.
     *         </li>
     *       </ul>
     *     </li>
     *   </ul>
     * @since  Helium
     */
    void vInterfaceChanged(VBridgeIfPath path, VInterface viface,
                           UpdateType type);

    /**
     * Invoked when the information related to the
     * {@linkplain <a href="package-summary.html#vInterface">virtual interface</a>}
     * configured in
     * {@linkplain <a href="package-summary.html#vTerminal">vTerminal</a>}
     * inside the container is changed.
     *
     * <p>
     *   If at least one vTerminal interface exists in the container at the
     *   time of registering {@code IVTNManagerAware} listener in the
     *   VTN Manager, then this method is called with specifying information
     *   about each vTerminal interface so that the existence of these can be
     *   notified to listener.
     * </p>
     * <ul>
     *   <li>
     *     It is guaranteed that
     *     {@link #vTerminalChanged(VTerminalPath, VTerminal, UpdateType)},
     *     which notifies the existence of the vTerminal to which this
     *     vTerminal interface belongs, is called first.
     *   </li>
     *   <li>
     *     In that case {@link UpdateType#ADDED} is passed to {@code type}.
     *   </li>
     * </ul>
     *
     * @param path    A {@link VTerminalIfPath} object that specifies the
     *                position of the vTerminal interface.
     * @param viface  A {@link VInterface} object which represents the
     *                vTerminal interface information.
     * @param type
     *   An {@link UpdateType} object which indicates the type of modification
     *   is specified.
     *   <ul>
     *     <li>
     *       {@link UpdateType#ADDED} is specified if a new vTerminal interface
     *       has been created.
     *       <ul>
     *         <li>
     *           The position of the newly created vTerminal interface is
     *           passed to {@code path}.
     *         </li>
     *         <li>
     *           Information about the newly created vTerminal interface is
     *           passed to {@code viface}.
     *         </li>
     *       </ul>
     *     </li>
     *     <li>
     *       {@link UpdateType#CHANGED} is specified if information about the
     *       specified vTerminal interface has been modified.
     *       <ul>
     *         <li>
     *           The position of the modified vTerminal interface is specified
     *           to {@code path}.
     *         </li>
     *         <li>
     *           Information about the updated vTerminal interface is specified
     *           to {@code viface}.
     *         </li>
     *         <li>
     *           Change is notified in the following cases.
     *           <ul>
     *             <li>
     *               The vTerminal interface configuration has been changed.
     *             </li>
     *             <li>
     *               The status of the vTerminal interface has been changed.
     *             </li>
     *           </ul>
     *         </li>
     *       </ul>
     *     </li>
     *     <li>
     *       {@link UpdateType#REMOVED} is specified if the specified vTerminal
     *       interface has been removed.
     *       <ul>
     *         <li>
     *           The position of the removed vTerminal interface is passed to
     *           {@code path}.
     *         </li>
     *         <li>
     *           Information about the vTerminal interface just prior to its
     *           removal is passed to {@code viface}.
     *         </li>
     *       </ul>
     *     </li>
     *   </ul>
     * @since  Helium
     */
    void vInterfaceChanged(VTerminalIfPath path, VInterface viface,
                           UpdateType type);

    /**
     * Invoked when the information related to the
     * {@linkplain <a href="package-summary.html#VLAN-map">VLAN mapping</a>}
     * configured in
     * {@linkplain <a href="package-summary.html#vBridge">vBridge</a>} inside
     * the container is changed.
     *
     * <p>
     *   If at least one VLAN mapping configured in vBridge exists in the
     *   container at the time of registering {@code IVTNManagerAware}
     *   listener in the VTN Manager, then this method is called with
     *   specifying information about each VLAN mapping so that the existence
     *   of these can be notified to listener.
     * </p>
     * <ul>
     *   <li>
     *     It is guaranteed that
     *     {@link #vBridgeChanged(VBridgePath, VBridge, UpdateType)}, which
     *     notifies the existence of the vBridge wherein this VLAN mapping is
     *     configured, is called first.
     *   </li>
     *   <li>
     *     In that case {@link UpdateType#ADDED} is passed to {@code type}.
     *   </li>
     * </ul>
     *
     * @param path   A {@link VBridgePath} object that specifies the position
     *               of the VBridge.
     * @param vlmap  A {@link VlanMap} object which represents the VLAN mapping
     *               information.
     * @param type
     *   An {@link UpdateType} object which indicates the type of modification
     *   is specified.
     *   <ul>
     *     <li>
     *       {@link UpdateType#ADDED} is specified if a new VLAN mapping has
     *       been configured.
     *       <ul>
     *         <li>
     *           The position of the vBridge in which the VLAN mapping is
     *           configured is passed to {@code path}.
     *         </li>
     *         <li>
     *           Information about the VLAN mapping is passed to {@code vlmap}.
     *         </li>
     *       </ul>
     *     </li>
     *     <li>
     *       {@link UpdateType#REMOVED} is specified if the specified VLAN
     *       mapping has been removed.
     *       <ul>
     *         <li>
     *           The position of the vBridge from which the VLAN mapping is
     *           removed is passed to {@code path}.
     *         </li>
     *         <li>
     *           Information about the VLAN mapping just prior to its removal
     *           is passed to {@code vlmap}.
     *         </li>
     *       </ul>
     *     </li>
     *     <li>
     *       {@link UpdateType#CHANGED} will never be passed because
     *       information of existing VLAN mapping cannot be changed.
     *     </li>
     *   </ul>
     */
    void vlanMapChanged(VBridgePath path, VlanMap vlmap, UpdateType type);

    /**
     * Invoked when the information related to the
     * {@linkplain <a href="package-summary.html#port-map">port mapping</a>}
     * configured in
     * {@linkplain <a href="package-summary.html#vInterface">vBridge interface</a>}
     * inside the container is changed.
     *
     * <p>
     *   If at least one port mapping configured in vBridge interface exists
     *   in the container at the time of registering {@code IVTNManagerAware}
     *   listener in the VTN Manager, then this method is called with
     *   specifying information about each port mapping so that the existence
     *   of these can be notified to listener.
     * </p>
     * <ul>
     *   <li>
     *     It is guaranteed that
     *     {@link #vInterfaceChanged(VBridgeIfPath, VInterface, UpdateType)},
     *     which notifies the existence of the vBridge interface wherein this
     *     port mapping is configured, is called first.
     *   </li>
     *   <li>
     *     In that case {@link UpdateType#ADDED} is passed to {@code type}.
     *   </li>
     * </ul>
     *
     * @param path  A {@link VBridgeIfPath} object that specifies the position
     *              of the vBridge interface.
     * @param pmap  A {@link PortMap} object which represents the port mapping
     *              information.
     * @param type
     *   An {@link UpdateType} object which indicates the type of modification
     *   is specified.
     *   <ul>
     *     <li>
     *       {@link UpdateType#ADDED} is specified if a new port mapping is
     *       configured.
     *       <ul>
     *         <li>
     *           The position of the vBridge interface in which the port
     *           mapping is configured is passed to {@code path}.
     *         </li>
     *         <li>
     *           Information about the port mapping is passed to {@code pmap}.
     *         </li>
     *       </ul>
     *     </li>
     *     <li>
     *       {@link UpdateType#CHANGED} is specified if information about the
     *       specified port mapping has been modified.
     *       <ul>
     *         <li>
     *           The position of the modified vBridge interface in which the
     *           port mapping is configured is passed to {@code path}.
     *         </li>
     *         <li>
     *           Information about the updated port mapping is specified to
     *           {@code pmap}.
     *         </li>
     *         <li>
     *           Change is notified in the following cases.
     *           <ul>
     *             <li>
     *               The port mapping configuration has been changed.
     *             </li>
     *             <li>
     *               The physical switch port actually mapped by the port
     *               mapping has been changed.
     *             </li>
     *           </ul>
     *         </li>
     *       </ul>
     *     </li>
     *     <li>
     *       {@link UpdateType#REMOVED} is specified if the specified port
     *       mapping has been removed.
     *       <ul>
     *         <li>
     *           The position of the vBridge interface from which the port
     *           mapping is removed is passed to {@code path}.
     *         </li>
     *         <li>
     *           Information about the port mapping just prior to its removal
     *           is passed to {@code pmap}.
     *         </li>
     *       </ul>
     *     </li>
     *   </ul>
     */
    void portMapChanged(VBridgeIfPath path, PortMap pmap, UpdateType type);

    /**
     * Invoked when the information related to the
     * {@linkplain <a href="package-summary.html#port-map">port mapping</a>}
     * configured in
     * {@linkplain <a href="package-summary.html#vInterface">vTerminal interface</a>}
     * inside the container is changed.
     *
     * <p>
     *   If at least one port mapping configured in vTerminal interface exists
     *   in the container at the time of registering {@code IVTNManagerAware}
     *   listener in the VTN Manager, then this method is called with
     *   specifying information about each port mapping so that the existence
     *   of these can be notified to listener.
     * </p>
     * <ul>
     *   <li>
     *     It is guaranteed that
     *     {@link #vInterfaceChanged(VTerminalIfPath, VInterface, UpdateType)},
     *     which notifies the existence of the vTerminal interface wherein this
     *     port mapping is configured, is called first.
     *   </li>
     *   <li>
     *     In that case {@link UpdateType#ADDED} is passed to {@code type}.
     *   </li>
     * </ul>
     *
     * @param path  A {@link VTerminalIfPath} object that specifies the
     *              position of the vTerminal interface.
     * @param pmap  A {@link PortMap} object which represents the port mapping
     *              information.
     * @param type
     *   An {@link UpdateType} object which indicates the type of modification
     *   is specified.
     *   <ul>
     *     <li>
     *       {@link UpdateType#ADDED} is specified if a new port mapping is
     *       configured.
     *       <ul>
     *         <li>
     *           The position of the vTerminal interface in which the port
     *           mapping is configured is passed to {@code path}.
     *         </li>
     *         <li>
     *           Information about the port mapping is passed to {@code pmap}.
     *         </li>
     *       </ul>
     *     </li>
     *     <li>
     *       {@link UpdateType#CHANGED} is specified if information about the
     *       specified port mapping has been modified.
     *       <ul>
     *         <li>
     *           The position of the modified vTerminal interface in which the
     *           port mapping is configured is passed to {@code path}.
     *         </li>
     *         <li>
     *           Information about the updated port mapping is specified to
     *           {@code pmap}.
     *         </li>
     *         <li>
     *           Change is notified in the following cases.
     *           <ul>
     *             <li>
     *               The port mapping configuration has been changed.
     *             </li>
     *             <li>
     *               The physical switch port actually mapped by the port
     *               mapping has been changed.
     *             </li>
     *           </ul>
     *         </li>
     *       </ul>
     *     </li>
     *     <li>
     *       {@link UpdateType#REMOVED} is specified if the specified port
     *       mapping has been removed.
     *       <ul>
     *         <li>
     *           The position of the vTerminal interface from which the port
     *           mapping is removed is passed to {@code path}.
     *         </li>
     *         <li>
     *           Information about the port mapping just prior to its removal
     *           is passed to {@code pmap}.
     *         </li>
     *       </ul>
     *     </li>
     *   </ul>
     * @since  Helium
     */
    void portMapChanged(VTerminalIfPath path, PortMap pmap, UpdateType type);

    /**
     * Invoked when the information related to the
     * {@linkplain <a href="package-summary.html#MAC-map">MAC mappping</a>}
     * configured in
     * {@linkplain <a href="package-summary.html#vBridge">vBridge</a>} inside
     * the container is changed.
     *
     * <p>
     *   If the MAC mapping configured in vBridge exists in the container
     *   at the time of registering {@code IVTNManagerAware} listener in the
     *   VTN Manager, then this method is called with specifying information
     *   about each MAC mapping so that the existence of these can be
     *   notified to listener.
     * </p>
     * <ul>
     *   <li>
     *     It is guaranteed that
     *     {@link #vBridgeChanged(VBridgePath, VBridge, UpdateType)}, which
     *     notifies the existence of the vBridge wherein this MAC mapping is
     *     configured, is called first.
     *   </li>
     *   <li>
     *     In that case {@link UpdateType#ADDED} is passed to {@code type}.
     *   </li>
     * </ul>
     * <p>
     *   Note that this method notifies only change of the MAC mapping
     *   configuration. This method will not be called even if a new mapping
     *   is established between host and the vBridge by MAC mapping.
     * </p>
     *
     * @param path    A {@link VBridgePath} object that specifies the position
     *                of the VBridge.
     * @param mcconf  A {@link MacMapConfig} object which represents the MAC
     *                mapping configuration information.
     * @param type
     *   An {@link UpdateType} object which indicates the type of modification
     *   is specified.
     *   <ul>
     *     <li>
     *       {@link UpdateType#ADDED} is specified if a new MAC mapping has
     *       been configured.
     *       <ul>
     *         <li>
     *           The position of the vBridge in which the MAC mapping is
     *           configured is passed to {@code path}.
     *         </li>
     *         <li>
     *           Information about the MAC mapping configuration is passed to
     *           {@code mcmap}.
     *         </li>
     *       </ul>
     *     </li>
     *     <li>
     *       {@link UpdateType#REMOVED} is specified if the specified MAC
     *       mapping has been removed.
     *       <ul>
     *         <li>
     *           The position of the vBridge from which the MAC mapping is
     *           removed is passed to {@code path}.
     *         </li>
     *         <li>
     *           Information about the MAC mapping configuration just prior
     *           to its removal is passed to {@code mcconf}.
     *         </li>
     *       </ul>
     *     </li>
     *     <li>
     *       {@link UpdateType#CHANGED} is specified if information about the
     *       MAC mapping has been modified.
     *       <ul>
     *         <li>
     *           The position of the modified vBridge in which the MAC mapping
     *           is configured is passed to {@code path}.
     *         </li>
     *         <li>
     *           Information about the updated MAC mapping configuration is
     *           specified to {@code mcconf}.
     *         </li>
     *       </ul>
     *     </li>
     *   </ul>
     * @since  Helium
     */
    void macMapChanged(VBridgePath path, MacMapConfig mcconf, UpdateType type);
}
