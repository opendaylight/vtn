/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.inventory.xml;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.XmlConfigFile;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.VtnStaticTopology;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.VtnStaticTopologyBuilder;

import org.opendaylight.yangtools.yang.binding.ChildOf;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

/**
 * {@code XmlStaticTopologyConfig} describes a XML file which specifies the
 * static network topology configuration.
 *
 * @param <T>  The type of MD-SAL container associated with this class.
 */
public abstract class XmlStaticTopologyConfig
    <T extends ChildOf<VtnStaticTopology>> {
    /**
     * Construct a new instance.
     */
    protected XmlStaticTopologyConfig() {
    }

    /**
     * Load the static network topology configuration from XML file.
     *
     * <p>
     *   This method loads the configuration from the XML file, and store
     *   loaded configuration into the MD-SAL config DS and the given
     *   {@link VtnStaticTopologyBuilder} instance.
     * </p>
     *
     * @param builder  A {@link VtnStaticTopologyBuilder} instance.
     * @param tx       A {@link ReadWriteTransaction} instance.
     * @throws VTNException  An error occurred.
     */
    public final void load(VtnStaticTopologyBuilder builder,
                           ReadWriteTransaction tx) throws VTNException {
        XmlConfigFile.Type ctype = XmlConfigFile.Type.TOPOLOGY;
        LogicalDatastoreType cstore = LogicalDatastoreType.CONFIGURATION;

        // Load the configuration file.
        XmlStaticTopologyConfig<T> xconf =
            XmlConfigFile.load(ctype, getXmlConfigKey(), getXmlType());
        T conf;
        InstanceIdentifier<T> path = InstanceIdentifier.
            builder(VtnStaticTopology.class).
            child(getContainerType()).
            build();
        if (xconf == null) {
            // Delete all the configuration from the config DS.
            DataStoreUtils.delete(tx, cstore, path);
            conf = null;
        } else {
            // Resume the configuration.
            conf = xconf.getConfig();
            tx.put(cstore, path, conf, true);
        }

        setConfig(builder, conf);
    }

    /**
     * Save the given static network topology configuration into the XML file.
     *
     * @param conf  A MD-SAL container which contains the configuration to be
     *              saved. {@code null} means that the configuration file
     *              should be deleted.
     */
    public final void save(T conf) {
        XmlConfigFile.Type type = XmlConfigFile.Type.TOPOLOGY;
        String key = getXmlConfigKey();
        if (conf == null) {
            XmlConfigFile.delete(type, key);
        } else {
            XmlStaticTopologyConfig<T> xconf = newInstance(conf);
            XmlConfigFile.save(type, key, xconf);
        }
    }

    /**
     * Return a class which indicates the type of MD-SAL container associated
     * with this class.
     *
     * @return  A class instance which specifies the MD-SAL container type.
     */
    public abstract Class<T> getContainerType();

    /**
     * Return a class which indicates the type of this class.
     *
     * @return  A class which indicates the type of this class.
     */
    public abstract Class<? extends XmlStaticTopologyConfig<T>> getXmlType();

    /**
     * Return a string which represents the configuration file key associated
     * with this class.
     *
     * @return  A string which represents the configuration file key.
     */
    public abstract String getXmlConfigKey();

    /**
     * Set the given configuration into the given MD-SAL container builder
     * instance.
     *
     * @param builder  A {@link VtnStaticTopologyBuilder} instance.
     * @param conf     A MD-SAL container which contains the configuration.
     */
    public abstract void setConfig(VtnStaticTopologyBuilder builder, T conf);

    /**
     * Return a MD-SAL container which contains the configuration in this
     * instance.
     *
     * @return  A MD-SAL container which contains the configuration.
     */
    public abstract T getConfig();

    /**
     * Construct a new instance that contains the given configuration.
     *
     * @param conf  A MD-SAL container which contains the configuration.
     * @return  An instance of this class.
     */
    public abstract XmlStaticTopologyConfig<T> newInstance(T conf);
}
