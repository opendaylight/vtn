/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.inventory.xml;

import java.util.ArrayList;
import java.util.Collections;
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
    @XmlElement(name = "static-switch-link")
    private List<XmlStaticSwitchLink>  switchLinks;

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
     *   This method is provided only for testing.
     * </p>
     *
     * @return  A list of {@link XmlStaticSwitchLink} instances or
     *          {@code null}.
     */
    List<XmlStaticSwitchLink> getXmlStaticSwitchLink() {
        return (switchLinks == null)
            ? null
            : Collections.unmodifiableList(switchLinks);
    }

    /**
     * Convert the given list of {@link StaticSwitchLink} instances into a
     * list of {@link XmlStaticSwitchLink} instances.
     *
     * @param swlinks  A {@link StaticSwitchLinks} instance.
     * @return  A list of {@link XmlStaticSwitchLink} instances or {@code null}.
     */
    private List<XmlStaticSwitchLink> toXmlStaticSwitchLink(
        StaticSwitchLinks swlinks) {
        if (swlinks == null) {
            return null;
        }

        List<StaticSwitchLink> links = swlinks.getStaticSwitchLink();
        if (links != null) {
            List<XmlStaticSwitchLink> xlinks = new ArrayList<>(links.size());
            Set<String> srcSet = new HashSet<>();
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
                if (srcSet.add(src)) {
                    xlinks.add(xlink);
                } else {
                    LOG.warn("Ignore duplicate static link: ", xlink);
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
        List<StaticSwitchLink> links;
        if (switchLinks == null) {
            links = null;
        } else {
            links = new ArrayList<>();
            Set<String> srcSet = new HashSet<>();
            for (XmlStaticSwitchLink xlink: switchLinks) {
                if (xlink.isValid()) {
                    String src = xlink.getSource();
                    if (srcSet.add(src)) {
                        links.add(xlink.toStaticSwitchLink());
                    } else {
                        LOG.warn("Ignore duplicate static link specified " +
                                 "by XML: {}", xlink);
                    }
                } else {
                    LOG.warn("Ignore invalid static link specified by XML: {}",
                             xlink);
                }
            }

            if (links.isEmpty()) {
                links = null;
            }
        }

        return links;
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

