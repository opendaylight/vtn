<?xml version="1.0" encoding="utf-8" standalone="yes"?>
<!--
  Copyright (c) 2012-2013 NEC Corporation
  All rights reserved.

  This program and the accompanying materials are made available under the
  terms of the Eclipse Public License v1.0 which accompanies this
  distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
-->

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:template match="*">
<xsl:copy>
<xsl:for-each select="@*|*[not(* or @*)]">
<xsl:attribute name="{name(.)}"><xsl:value-of select="."/>
</xsl:attribute>
</xsl:for-each>
<xsl:apply-templates select="*[* or @*]|text()"/>
</xsl:copy>
</xsl:template>
</xsl:stylesheet>
