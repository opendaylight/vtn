//     Copyright (c) 2015 NEC Corporation
//     All rights reserved.
//
//     This program and the accompanying materials are made available under the
//     terms of the Eclipse Public License v1.0 which accompanies this
//     distribution, and is available at http://www.eclipse.org/legal/epl-v10.html


using System.Globalization;

namespace ODL.VSEMProvider.Libraries.Common {
    /// <summary>
    /// Regular expressions to validate the format of user inputs.
    /// </summary>
    public static class RegularExpressions {
        /// <summary>
        /// Valid format of DNS.
        /// </summary>
        public const string DNS_PATTERN = @"((?![0-9]+$)(?!.*-$)(?!-)[a-zA-Z0-9-\.]{1,63})";

        /// <summary>
        /// Valid format of port number.
        /// </summary>
        public const string PORT_NUMBER_PATTERN =
            @"0*([0-9]{1,4}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5])";

        /// <summary>
        /// Valid format of subnet number.
        /// </summary>
        public const string SUBNET_PATTERN = @"0*([0-9]|[1-2][0-9]|3[0-1])";

        /// <summary>
        /// Valid format of VM network name with wild card.
        /// </summary>
        public const string VM_NETWORK_NAME_PATTERN_WITH_WILD_CARD = @"^[a-zA-Z0-9_]*(\*)?$";

        /// <summary>
        /// Valid format of vlan id.
        /// </summary>
        public const string VLAN_ID_PATTERN =
            @"0*(([1-9]|[1-9][0-9]|[1-9][0-9][0-9])|[1-3][0-9]{3}|40[0-8][0-9]|409[0-4])";

        /// <summary>
        /// Valid format of VM network name.
        /// </summary>
        public const string VM_NETWORK_NAME_PATTERN = @"^[a-zA-Z0-9_]*$";

        /// <summary>
        /// Valid format of Ip address number.
        /// </summary>
        public const string IP_NUMBER_PATTERN =
            @"([0-9]|[0-9][0-9]|[0-1][0-9][0-9]|2[0-4][0-9]|25[0-5])";

        /// <summary>
        /// Valid format of the vlan id range pattern.
        /// </summary>
        public static readonly string VLAN_ID_RANGE_PATTERN =
            string.Format(CultureInfo.CurrentCulture,
            @"^(({0})|({0}-{0}))(,({0})|,({0}-{0}))*$",
            RegularExpressions.VLAN_ID_PATTERN);

        /// <summary>
        /// Valid format of connection string.
        /// </summary>
        public static readonly string CONNECTION_STRING_PATTERN =
            string.Format(CultureInfo.CurrentCulture,
            @"^(https://)?{0}:{1}$",
            RegularExpressions.DNS_PATTERN,
            RegularExpressions.PORT_NUMBER_PATTERN);

        /// <summary>
        /// Valid format of IP address with port number.
        /// </summary>
        public static readonly string IP_PATTERN_WITH_PORT =
            string.Format(CultureInfo.CurrentCulture,
            @"^({0}\.{0}\.{0}\.{0}(:{1})?)$",
            RegularExpressions.IP_NUMBER_PATTERN,
            RegularExpressions.PORT_NUMBER_PATTERN);

        /// <summary>
        /// Valid format of IP address with subnet.
        /// </summary>
        public static readonly string IP_PATTERN_WITH_SUBNET =
            string.Format(CultureInfo.CurrentCulture,
            @"^({0}\.{0}\.{0}\.{0}/{1})$",
            RegularExpressions.IP_NUMBER_PATTERN,
            RegularExpressions.SUBNET_PATTERN);

        /// <summary>
        /// Valid format of IP address.
        /// </summary>
        public static readonly string IP_PATTERN =
            string.Format(CultureInfo.CurrentCulture,
            @"^({0}\.{0}\.{0}\.{0})$",
            RegularExpressions.IP_NUMBER_PATTERN);
    }
}
