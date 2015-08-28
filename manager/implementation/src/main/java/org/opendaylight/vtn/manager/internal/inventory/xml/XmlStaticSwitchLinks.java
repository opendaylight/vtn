/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.inventory.xml;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.VtnStaticTopologyBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology.StaticSwitchLinks;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology.StaticSwitchLinksBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static._switch.links.StaticSwitchLink;

/**
 * {@code XmlStaticSwitchLinks} provides XML binding to a set of static
 * inter-switch link configurations.
 */
@XmlRootElement(name = "static-switch-links")
@XmlAccessorType(XmlAccessType.NONE)
public final class XmlStaticSwitchLinks
    extends XmlStaticTopologyConfig<StaticSwitchLinks> {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(XmlStaticSwitchLinks.class);

    /**
     * The key associated with the configuration file for static inter-switch
     * links.
     */
    private static final String  KEY_LINK = "static-link";

    /**
     * A list of static inter-switch links.
     */
    private Map<String, XmlStaticSwitchLink>  switchLinks;

    /**
     * Construct an empty instance.
     */
    public XmlStaticSwitchLinks() {
    }

    /**
     * Construct a new instance.
     *
     * @param swlinks  A {@link StaticSwitchLinks} instance.
     */
    public XmlStaticSwitchLinks(StaticSwitchLinks swlinks) {
        switchLinks = toXmlStaticSwitchLink(swlinks);
    }

    /**
     * Return a list of {@link XmlStaticSwitchLink} instances configured in
     * this instance.
     *
     * <p>
     *   This method is provided only for JAXB.
     * </p>
     *
     * @return  A list of {@link XmlStaticSwitchLink} instances or
     *          {@code null}.
     */
    @XmlElement(name = "static-switch-link")
    public List<XmlStaticSwitchLink> getXmlStaticSwitchLink() {
        return (switchLinks == null)
            ? null : new ArrayList<XmlStaticSwitchLink>(switchLinks.values());
    }

    /**
     * Set a list of {@link XmlStaticSwitchLink} instances to this instance.
     *
     * <p>
     *   This method is provided only for JAXB.
     * </p>
     *
     * @param xlinks  A list of {@link XmlStaticSwitchLink} instance.
     */
    public void setXmlStaticSwitchLink(List<XmlStaticSwitchLink> xlinks) {
        if (xlinks == null) {
            switchLinks = null;
            return;
        }

        Map<String, XmlStaticSwitchLink> map = new HashMap<>();
        for (XmlStaticSwitchLink xlink: xlinks) {
            if (xlink.isValid()) {
                XmlStaticSwitchLink old = map.put(xlink.getSource(), xlink);
                if (old != null) {
                    LOG.warn("Ignore duplicate static link specified by XML" +
                             ": {}", old);
                }
            } else {
                LOG.warn("Ignore invalid static link specified by XML: {}",
                         xlink);
            }
        }

        if (!map.isEmpty()) {
            switchLinks = map;
        }
    }

    /**
     * Convert the given list of {@link StaticSwitchLink} instances into a
     * list of {@link XmlStaticSwitchLink} instances.
     *
     * @param swlinks  A {@link StaticSwitchLinks} instance.
     * @return  A map which keeps {@link XmlStaticSwitchLink} instances or
     *          {@code null}.
     */
    private Map<String, XmlStaticSwitchLink> toXmlStaticSwitchLink(
        StaticSwitchLinks swlinks) {
        if (swlinks == null) {
            return null;
        }

        List<StaticSwitchLink> links = swlinks.getStaticSwitchLink();
        if (links != null) {
            Map<String, XmlStaticSwitchLink> xlinks = new HashMap<>();
            for (StaticSwitchLink link: links) {
                XmlStaticSwitchLink xlink;
                try {
                    xlink = new XmlStaticSwitchLink(link);
                } catch (IllegalArgumentException e) {
                    LOG.warn("Ignore invalid static link: {}: {}",
                             e.getMessage(), link);
                    continue;
                } catch (RuntimeException e) {
                    LOG.warn("Ignore invalid static link: " + link, e);
                    continue;
                }

                String src = xlink.getSource();
                XmlStaticSwitchLink old = xlinks.put(src, xlink);
                if (old != null) {
                    LOG.warn("Ignore duplicate static link: ", old);
                }
            }

            if (!xlinks.isEmpty()) {
                return xlinks;
            }
        }

        return null;
    }

    /**
     * Return a list of {@link StaticSwitchLink} instances configured in this
     * instance.
     *
     * @return  A list of {@link StaticSwitchLink} instances or {@code null}.
     */
    private List<StaticSwitchLink> getStaticSwitchLinks() {
        if (switchLinks != null) {
            List<StaticSwitchLink> links = new ArrayList<>();
            for (XmlStaticSwitchLink xlink: switchLinks.values()) {
                try {
                    links.add(xlink.toStaticSwitchLink());
                } catch (RuntimeException e) {
                    // This should never happen because static links are
                    // already verified by setXmlStaticSwitchLink().
                    LOG.warn("Ignore invalid static link specified by XML: " +
                             xlink, e);
                }
            }

            if (!links.isEmpty()) {
                return links;
            }
        }

        return null;
    }

    // XmlStaticTopologyConfig

    /**
     * {@inheritDoc}
     */
    @Override
    public Class<StaticSwitchLinks> getContainerType() {
        return StaticSwitchLinks.class;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Class<XmlStaticSwitchLinks> getXmlType() {
        return XmlStaticSwitchLinks.class;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public String getXmlConfigKey() {
        return KEY_LINK;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setConfig(VtnStaticTopologyBuilder builder,
                          StaticSwitchLinks conf) {
        builder.setStaticSwitchLinks(conf);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public StaticSwitchLinks getConfig() {
        return new StaticSwitchLinksBuilder().
            setStaticSwitchLink(getStaticSwitchLinks()).
            build();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public XmlStaticSwitchLinks newInstance(StaticSwitchLinks conf) {
        return new XmlStaticSwitchLinks(conf);
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

        XmlStaticSwitchLinks xswlinks = (XmlStaticSwitchLinks)o;
        return Objects.equals(switchLinks, xswlinks.switchLinks);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return Objects.hash(getClass(), switchLinks);
    }
}

