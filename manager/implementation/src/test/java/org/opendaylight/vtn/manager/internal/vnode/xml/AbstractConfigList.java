/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode.xml;

import static org.junit.Assert.assertEquals;

import static org.opendaylight.vtn.manager.internal.TestBase.MAX_RANDOM;

import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Random;

import org.opendaylight.vtn.manager.internal.XmlNode;

import org.opendaylight.yangtools.yang.binding.DataObject;

/**
 * {@code AbstractConfigList} describes a list of data model configuration.
 *
 * @param <X>  The type of the test target class.
 * @param <C>  The type of the configuration class.
 * @param <D>  The type of the data model.
 * @param <T>  The actual type of this instance.
 */
public abstract class AbstractConfigList<X, C, D extends DataObject,
                                         T extends AbstractConfigList>
    implements Cloneable {
    /**
     * A list of configurations indexed by the key.
     */
    private Map<Object, C>  configMap = new LinkedHashMap<>();

    /**
     * Construct an empty instance.
     */
    protected AbstractConfigList() {
    }

    /**
     * Construct a new instance using the given random number generator.
     *
     * @param rand  A pseudo random number generator.
     */
    protected AbstractConfigList(Random rand) {
        add(rand);
    }

    /**
     * Construct a new instance that contains the given number of
     * configurations using the given random number generator.
     *
     * @param rand   A pseudo random number generator.
     * @param count  The number of configurations to add.
     */
    protected AbstractConfigList(Random rand, int count) {
        add(rand, count);
    }

    /**
     * Add the given configuration to this instance.
     *
     * @param conf  An instance that specifies the configuration.
     * @return  This instance.
     */
    public final T add(C conf) {
        if (conf != null) {
            assertEquals(null, configMap.put(getKey(conf), conf));
        }
        return getThis();
    }

    /**
     * Add random configurations using the given random number generator.
     *
     * @param rand  A pseudo random number generator.
     * @return  This instance.
     */
    public final T add(Random rand) {
        return add(rand, rand.nextInt(MAX_RANDOM));
    }

    /**
     * Add the given number of configurations using the given random number
     * generator.
     *
     * @param rand   A pseudo random number generator.
     * @param count  The number of configurations to add.
     * @return  This instance.
     */
    public final T add(Random rand, int count) {
        if (count > 0) {
            int newSize = configMap.size() + count;
            do {
                C conf = newConfig(rand);
                Object key = getKey(conf);
                C old = configMap.put(key, conf);
                if (old != null) {
                    configMap.put(key, old);
                }
            } while (configMap.size() < newSize);
        }

        return getThis();
    }

    /**
     * Return all the configurations in this instance.
     *
     * @return  A list of configurations.
     */
    public final List<C> getAll() {
        return new ArrayList<C>(configMap.values());
    }

    /**
     * Convert this instance into a list of data objects.
     *
     * @return  A list of data objects.
     */
    public final List<D> toDataObjects() {
        List<D> list = new ArrayList<>(configMap.size());
        for (C conf: configMap.values()) {
            list.add(toDataObject(conf));
        }

        return list;
    }

    /**
     * Ensure that the given instances are identical to this instance.
     *
     * @param list  A list of XML bindings to be verified.
     */
    public final void verify(List<X> list) {
        verify(list, false);
    }

    /**
     * Return a map that contains all the configurations in this instance.
     *
     * @return  A map that contains all the configurations in this instance.
     */
    protected final Map<Object, C> getConfigMap() {
        return configMap;
    }

    /**
     * Set the configuratio map.
     *
     * @param cfMap  A map that contains indexed configurations.
     */
    protected final void setConfigMap(Map<Object, C> cfMap) {
        configMap = new LinkedHashMap<>(cfMap);
    }

    /**
     * Set the configuration in this instance into the specified XML node.
     *
     * @param xnode  A {@link XmlNode} instance.
     */
    public abstract void setXml(XmlNode xnode);

    /**
     * Ensure that the given instances are identical to this instance.
     *
     * @param list  A list of XML bindings to be verified.
     * @param jaxb  {@code true} indicates {@code xvifs} was deserialized
     *              from XML.
     */
    public abstract void verify(List<X> list, boolean jaxb);

    /**
     * Return this instance casted to the actual type.
     *
     * @return  This instance.
     */
    protected abstract T getThis();

    /**
     * Construct a new configuration using the given random number generator.
     *
     * @param rand  A pseudo random number generator.
     * @return  A new configuration.
     */
    protected abstract C newConfig(Random rand);

    /**
     * Return the key of the given configuration.
     *
     * @param conf  An instance that specifies the configuration.
     * @return  The key of the given configuration.
     */
    protected abstract Object getKey(C conf);

    /**
     * Convert the given configuration into a data object.
     *
     * @param conf  An instance that specifies the configuration.
     * @return  A data object.
     */
    protected abstract D toDataObject(C conf);

    // Object

    /**
     * Return a shallow copy of this instance.
     *
     * @return  A shallow copy of this instance.
     */
    @Override
    public final T clone() {
        try {
            @SuppressWarnings("unchecked")
            T list = (T)super.clone();
            list.setConfigMap(configMap);
            return list;
        } catch (CloneNotSupportedException e) {
            // This should never happen.
            throw new IllegalStateException("clone() failed.", e);
        }
    }
}
