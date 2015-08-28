/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.inventory.xml;

import static org.opendaylight.vtn.manager.internal.inventory.xml.XmlStaticSwitchLink.isValidPortId;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Objects;
import java.util.Set;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.VtnStaticTopologyBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology.StaticEdgePorts;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology.StaticEdgePortsBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static.edge.ports.StaticEdgePort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static.edge.ports.StaticEdgePortBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;

/**
 * {@code XmlStaticEdgePorts} provides XML binding to a set of static edge
 * port configurations.
 */
@XmlRootElement(name = "static-edge-ports")
@XmlAccessorType(XmlAccessType.NONE)
public final class XmlStaticEdgePorts
    extends XmlStaticTopologyConfig<StaticEdgePorts> {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(XmlStaticEdgePorts.class);

    /**
     * The key associated with the configuration file for static edge ports.
     */
    private static final String  KEY_EDGE = "static-edge";

    /**
     * A set of switch ports to be treated as edge port.
     */
    @XmlElement(name = "static-edge-port")
    private Set<String>  edgePorts;

    /**
     * Construct an empty instance.
     */
    public XmlStaticEdgePorts() {
    }

    /**
     * Construct a new instance.
     *
     * @param edges  A {@link StaticEdgePorts} instance.
     */
    public XmlStaticEdgePorts(StaticEdgePorts edges) {
        edgePorts = toEdgePort(edges);
    }

    /**
     * Return a set of port ID strings configured in the given
     * {@link StaticEdgePorts} instances.
     *
     * @param edges  A {@link StaticEdgePorts} instance.
     * @return  A set of port ID strings configured in {@code edges} or
     *          {@code null}.
     */
    private Set<String> toEdgePort(StaticEdgePorts edges) {
        if (edges == null) {
            return null;
        }

        List<StaticEdgePort> eplist = edges.getStaticEdgePort();
        if (eplist != null) {
            Set<String> ports = new HashSet<>();
            for (StaticEdgePort ep: eplist) {
                NodeConnectorId ncId = ep.getPort();
                if (ncId != null) {
                    String id = ncId.getValue();
                    if (isValidPortId(id)) {
                        ports.add(id);
                        continue;
                    }
                }

                LOG.warn("Ignore invalid static edge port: {}", ep);
            }

            if (!ports.isEmpty()) {
                return ports;
            }
        }

        return null;
    }

    /**
     * Return a list of {@link StaticEdgePort} instances configured in this
     * instance.
     *
     * @return  A list of {@link StaticEdgePort} instances or {@code null}.
     */
    private List<StaticEdgePort> getStaticEdgePort() {
        if (edgePorts != null) {
            List<StaticEdgePort> edges = new ArrayList<>();
            for (String id: edgePorts) {
                if (isValidPortId(id)) {
                    try {
                        NodeConnectorId ncId = new NodeConnectorId(id);
                        StaticEdgePort edge = new StaticEdgePortBuilder().
                            setPort(ncId).build();
                        edges.add(edge);
                    } catch (IllegalArgumentException e) {
                        LOG.warn("Ignore invalid static edge port specified " +
                                 "by XML: {}: {}", e.getMessage(), id);
                    } catch (RuntimeException e) {
                        LOG.warn("Ignore invalid static edge port specified " +
                                 "by XML: " + id, e);
                    }
                } else {
                    LOG.warn("Ignore invalid static edge port specified by " +
                             "XML: {}", id);
                }
            }

            if (!edges.isEmpty()) {
                return edges;
            }
        }

        return null;
    }

    // XmlStaticTopologyConfig

    /**
     * {@inheritDoc}
     */
    @Override
    public Class<StaticEdgePorts> getContainerType() {
        return StaticEdgePorts.class;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Class<XmlStaticEdgePorts> getXmlType() {
        return XmlStaticEdgePorts.class;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public String getXmlConfigKey() {
        return KEY_EDGE;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setConfig(VtnStaticTopologyBuilder builder,
                          StaticEdgePorts conf) {
        builder.setStaticEdgePorts(conf);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public StaticEdgePorts getConfig() {
        return new StaticEdgePortsBuilder().
            setStaticEdgePort(getStaticEdgePort()).
            build();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public XmlStaticEdgePorts newInstance(StaticEdgePorts conf) {
        return new XmlStaticEdgePorts(conf);
    }

    // Object

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        if (o == null || !getClass().equals(o.getClass())) {
            return false;
        }

        XmlStaticEdgePorts xedges = (XmlStaticEdgePorts)o;
        return Objects.equals(edgePorts, xedges.edgePorts);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return Objects.hash(getClass(), edgePorts);
    }
}

