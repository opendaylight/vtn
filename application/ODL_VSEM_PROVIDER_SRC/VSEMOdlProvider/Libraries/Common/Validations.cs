//     Copyright (c) 2015 NEC Corporation
//     All rights reserved.
//
//     This program and the accompanying materials are made available under the
//     terms of the Eclipse Public License v1.0 which accompanies this
//     distribution, and is available at http://www.eclipse.org/legal/epl-v10.html


using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Net;
using System.Reflection;
using System.Text;
using System.Text.RegularExpressions;
using Microsoft.SystemCenter.NetworkService;
using ODL.VSEMProvider.VSEMEvents;

namespace ODL.VSEMProvider.Libraries.Common {
    /// <summary>
    /// Provides the methods for validations.
    /// </summary>
    public static class Validations {
        /// <summary>
        /// Error message for the IP string validations.
        /// </summary>
        private static string ipStringValidationMessage =
            "IP address must be in format x.x.x.x where value of x should be between 0 and 255.";

        /// <summary>
        /// Error message for the IP string with port number validations.
        /// </summary>
        private static string ipStringWithPortValidationMessage =
            string.Format(CultureInfo.CurrentCulture,
            "{0}\n{1}\n1. {2}\n2. {3}\n",
            "Format of IPAddressSubnet is invalid.",
            "Possible reasons could be:",
            ipStringValidationMessage,
            "Subnet mask is invalid. It should be a number between 0 and 31.");

        /// <summary>
        /// Method to validate IP address.
        /// </summary>
        /// <param name="value">Value to be validated.</param>
        /// <returns>True if value is valid.</returns>
        public static bool IsIPAddressValid(string value) {
            if (!Regex.IsMatch(value, RegularExpressions.IP_PATTERN)) {
                return false;
            }
            IPAddress address;
            if (IPAddress.TryParse(value, out address)) {
                if (address.AddressFamily == System.Net.Sockets.AddressFamily.InterNetwork) {
                    return true;
                }
            }
            return false;
        }

        /// <summary>
        /// Method to validate IP address with subnet.
        /// </summary>
        /// <param name="value">Value to be validated.</param>
        /// <returns>True if value is valid.</returns>
        public static bool IsIPAddressWithSubnetValid(string value) {
            bool isvalid = false;
            if (!Regex.IsMatch(value, RegularExpressions.IP_PATTERN_WITH_SUBNET)) {
                return false;
            }
            IPAddress address;
            var parts = value.Split('/');
            if (IPAddress.TryParse(parts[0], out address)) {
                if (address.AddressFamily == System.Net.Sockets.AddressFamily.InterNetwork) {
                    isvalid = true;
                }
            }

            return isvalid;
        }

        /// <summary>
        /// Method to validate IP address with port.
        /// </summary>
        /// <param name="value">Value to be validated.</param>
        /// <returns>IP address if value is valid.</returns>
        public static string IsIPAddressWithPortValid(string value) {
            string output = string.Empty;
            if (!Regex.IsMatch(value, RegularExpressions.IP_PATTERN_WITH_PORT)) {
                return output;
            }
            var parts = value.Split(':');
            var ipnumbers = parts[0].Split('.').ToList();
            for (int ctr = 0; ctr < 4; ctr++) {
                ipnumbers[ctr] = ipnumbers[ctr].TrimStart('0');
                if (string.IsNullOrEmpty(ipnumbers[ctr])) {
                    ipnumbers[ctr] = "0";
                }
                output += ipnumbers[ctr] + ".";
            }
            output = output.Remove(output.Length - 1);
            output = output + ((parts.Count() == 2) ? ":" + parts[1] : string.Empty);

            return output;
        }

        /// <summary>
        /// Validates IP subnet.
        /// A VM Subnet can never have more than one IP address pools with same IP address family(IPv4/IPv6).
        /// A VM Subnet can never have more than one IP address pools with zero/empty ID and same IP address family(IPv4/IPv6).
        /// </summary>
        /// <param name="ipSubnets">IP address pool list to validate.</param>
        /// <param name="checkFormat">IP address pool to validate the format.</param>
        /// <returns>Empty if IP address pool is valid.</returns>
        public static string IsIPSubnetListValid(List<IPSubnet> ipSubnets,
            Guid checkFormat) {
            StringBuilder isvalid = new StringBuilder();
            if (ipSubnets == null || ipSubnets.Count == 0) {
                return isvalid.ToString();
            }

            if (checkFormat == Guid.Empty) {
                foreach (var subnet in ipSubnets) {
                    isvalid.Append(IsIPSubnetValid(subnet, Guid.Empty));
                    if (!string.IsNullOrEmpty(isvalid.ToString())) {
                        return isvalid.ToString();
                    }
                }
            } else {
                isvalid.Append(IsIPSubnetValid(ipSubnets.FirstOrDefault(pool =>
                    pool.IPAddressPools != null
                        && pool.IPAddressPools.Any(ipaddresspool => ipaddresspool.Id == checkFormat)),
                    checkFormat));
                if (!string.IsNullOrEmpty(isvalid.ToString())) {
                    return isvalid.ToString();
                }
            }
            return isvalid.ToString();
        }

        /// <summary>
        /// Returns the IP address family of th IP address given string form.
        /// </summary>
        /// <param name="value">IP address given string form.</param>
        /// <returns>IP address family.</returns>
        public static AddressFamily? ConverStringToIPAddressFamily(string value) {
            if (!Regex.IsMatch(value, RegularExpressions.IP_PATTERN)) {
                return null;
            }
            IPAddress address;
            if (IPAddress.TryParse(value, out address)) {
                if (address.AddressFamily == System.Net.Sockets.AddressFamily.InterNetwork) {
                    return AddressFamily.IPv4;
                }
                if (address.AddressFamily == System.Net.Sockets.AddressFamily.InterNetworkV6) {
                    return AddressFamily.IPv6;
                }
            }
            return null;
        }

        /// <summary>
        /// Converts the string into IP address.
        /// </summary>
        /// <param name="value">IP address in string form.</param>
        /// <returns>IP address.</returns>
        public static IPAddress ConverStringToIPAddress(string value) {
            if (!Regex.IsMatch(value, RegularExpressions.IP_PATTERN)) {
                return null;
            }
            IPAddress address;
            if (IPAddress.TryParse(value, out address)) {
                return address;
            }
            return null;
        }

        /// <summary>
        /// Validates the IP Subnet.
        /// AddressRangeStart less than AddressRangeEnd.
        /// AddressRangeStart,AddressRangeEnd,networkGateway.IPAddress lie in subnet logically.
        /// All of them exists and are valid IP Addresses.
        /// networkGateway is optional.
        /// </summary>
        /// <param name="subnet">IP address pool to validate.</param>
        /// <param name="id">IP address pool ID.</param>
        /// <returns>Error message.</returns>
        private static string IsIPSubnetValid(IPSubnet subnet, Guid id) {
            StringBuilder isvalid = new StringBuilder();
            if (subnet == null
                || subnet.IPAddressPools == null
                || subnet.IPAddressPools.Count() == 0) {
                return isvalid.ToString();
            }

            if (id == Guid.Empty) {
                foreach (IPAddressPool pool in subnet.IPAddressPools) {
                    isvalid.Append(IsIPAddressPoolValid(pool));
                    if (!string.IsNullOrEmpty(isvalid.ToString())) {
                        return isvalid.ToString();
                    }
                    if (subnet.IPAddressPools.Any(poolCheck => pool.Id != poolCheck.Id
                        && AreIPAddressPoolsOverlapping(pool, poolCheck))) {
                        isvalid.Append("The range specified for the static IP address pool overlaps with the range of address managed by an existing static IP address pool.");
                        return isvalid.ToString();
                    }
                }
            } else {
                isvalid.Append(IsIPAddressPoolValid(subnet.IPAddressPools.FirstOrDefault(pool =>
                    pool.Id == id)));
                if (!string.IsNullOrEmpty(isvalid.ToString())) {
                    return isvalid.ToString();
                }
                if (subnet.IPAddressPools.Any(poolCheck => AreIPAddressPoolsOverlapping(
                    subnet.IPAddressPools.FirstOrDefault(pool => pool.Id == id),
                    poolCheck)
                    && id != poolCheck.Id)) {
                    isvalid.Append("The range specified for the static IP address pool overlaps with the range of address managed by an existing static IP address pool.");
                    return isvalid.ToString();
                }
            }
            return isvalid.ToString();
        }

        /// <summary>
        /// Check if IP address pools overlaps.
        /// </summary>
        /// <param name="poolFirst">First IP address pool.</param>
        /// <param name="poolSecond">Second IP address pool.</param>
        /// <returns>True if IP address pols overlap.</returns>
        private static bool AreIPAddressPoolsOverlapping(IPAddressPool poolFirst, IPAddressPool poolSecond) {
            bool overlap = false;
            if (poolFirst.Id.CompareTo(Guid.Empty) == 0 || poolSecond.Id.CompareTo(Guid.Empty) == 0) {
                return overlap;
            }
            if ((IsLesser(poolFirst.AddressRangeStart, poolSecond.AddressRangeStart)
                && IsLesser(poolSecond.AddressRangeStart, poolFirst.AddressRangeEnd))
                || (IsLesser(poolFirst.AddressRangeStart, poolSecond.AddressRangeEnd)
                && IsLesser(poolSecond.AddressRangeEnd, poolFirst.AddressRangeEnd))
                || (IsLesser(poolSecond.AddressRangeStart, poolFirst.AddressRangeEnd)
                && IsLesser(poolFirst.AddressRangeEnd, poolSecond.AddressRangeEnd))) {
                overlap = true;
            }
            return overlap;
        }

        /// <summary>
        /// Check if IP address pool is valid.
        /// </summary>
        /// <param name="pool">IP address pool to check.</param>
        /// <returns>true if UP address pool is valid.</returns>
        private static string IsIPAddressPoolValid(IPAddressPool pool) {
            StringBuilder isvalid = new StringBuilder();
            if (pool == null) {
                return isvalid.ToString();
            }

            if (!string.IsNullOrEmpty(pool.AddressRangeStart)) {
                if (!Validations.IsIPAddressValid(pool.AddressRangeStart)) {
                    isvalid.Append("Format of IP address in AddressRangeStart is invalid.\n"
                        + ipStringValidationMessage + "\n");
                }
            }

            if (!string.IsNullOrEmpty(pool.AddressRangeEnd)) {
                if (!Validations.IsIPAddressValid(pool.AddressRangeEnd)) {
                    isvalid.Append("Format of IP address in AddressRangeEnd is invalid.\n"
                        + ipStringValidationMessage + "\n");
                }
            }

            if (!string.IsNullOrEmpty(pool.IPAddressSubnet)) {
                if (!Validations.IsIPAddressWithSubnetValid(
                    pool.IPAddressSubnet)) {
                    isvalid.Append(ipStringWithPortValidationMessage + "\n");
                }
            }

            if (!string.IsNullOrEmpty(isvalid.ToString())) {
                return isvalid.ToString();
            }

            if (!string.IsNullOrEmpty(pool.IPAddressSubnet)) {
                isvalid.Append(IsInRange(pool.IPAddressSubnet,
                    pool.AddressRangeStart,
                    "AddressRangeStart",
                    pool.AddressRangeEnd,
                    "AddressRangeEnd"));
            }

            if (!string.IsNullOrEmpty(isvalid.ToString())) {
                return isvalid.ToString();
            }

            if (!string.IsNullOrEmpty(pool.AddressRangeStart)
                && !string.IsNullOrEmpty(pool.AddressRangeEnd)) {
                if (!IsLesser(pool.AddressRangeStart,
                pool.AddressRangeEnd)) {
                    isvalid.Append(
                        "'AddressRangeStart' cannot be greater than 'AddressRangeEnd'.\n");
                }
            }

            if (pool.NetworkGateways != null) {
                foreach (var networkGateway in pool.NetworkGateways) {
                    if (!string.IsNullOrEmpty(networkGateway.IPAddress)) {
                        if (!Validations.IsIPAddressValid(networkGateway.IPAddress)) {
                            isvalid.Append(
                                "Format of one or more IP address(s) in NetworkGatewayInfo is invalid.\n"
                            + ipStringValidationMessage + "\n");
                            break;
                        }
                        isvalid.Append(IsInRange(pool.IPAddressSubnet,
                            networkGateway.IPAddress,
                            "IPAddress of NetworkGatewayInfo",
                            null,
                            null));
                    } else {
                        isvalid.Append(
                                "Value of one or more IP address(s) in NetworkGatewayInfo is null.\n"
                            + ipStringValidationMessage + "\n");
                        break;
                    }

                    if (networkGateway.Metric != null
                        && (networkGateway.Metric < 1
                        || networkGateway.Metric > 9999)) {
                        isvalid.Append(
                            "Value of one or more Metric(s) in NetworkGatewayInfo is invalid.\n"
                            + "Value of Metric in NetworkGatwayInfo should be between 1 and 9999.\n");
                        break;
                    }
                }
            }
            return isvalid.ToString();
        }

        /// <summary>
        /// Check if the specified input1 and input2 are in correct format .
        /// according to subnet masking in subnet.
        /// </summary>
        /// <param name="subnet">Subnet mask.</param>
        /// <param name="input1">First IP address to check.</param>
        /// <param name="name1">Name of field of first IP address to check.</param>
        /// <param name="input2">Second IP address to check.</param>
        /// <param name="name2">Name of field of second IP address to check.</param>
        /// <returns>Error message.</returns>
        private static string IsInRange(string subnet,
            string input1,
            string name1,
            string input2,
            string name2) {
            StringBuilder message = new StringBuilder();
            IPAddress ipAddress = null;
            var parts = subnet.Split('/');
            IPAddress.TryParse(parts[0], out ipAddress);
            int bitsForMaking = Convert.ToInt16(parts[1], CultureInfo.CurrentCulture);

            uint subnetMask = ~(uint.MaxValue >> bitsForMaking);

            // Convert IP address into bytes.
            byte[] ipAddressBytes = ipAddress.GetAddressBytes();

            byte[] subnetMaskBytes = BitConverter.GetBytes(subnetMask).Reverse().ToArray();

            byte[] startIPAddressBytes = new byte[ipAddressBytes.Length];
            byte[] endIPAddressBytes = new byte[ipAddressBytes.Length];

            // Calculate bytes of start & end IP addresses.
            for (int cntr = 0; cntr < ipAddressBytes.Length; cntr++) {
                startIPAddressBytes[cntr] = (byte)(ipAddressBytes[cntr] & subnetMaskBytes[cntr]);
                endIPAddressBytes[cntr] = (byte)(ipAddressBytes[cntr] | ~subnetMaskBytes[cntr]);
            }

            IPAddress startIPAddress = new IPAddress(startIPAddressBytes);
            IPAddress endIPAddress = new IPAddress(endIPAddressBytes);
            var startIPNum = IPAddressToLongBackwards(startIPAddress.ToString());
            var endIPNum = IPAddressToLongBackwards(endIPAddress.ToString());
            if (!string.IsNullOrEmpty(input1)) {
                var input1Num = IPAddressToLongBackwards(input1);
                if (input1Num <= startIPNum || input1Num >= endIPNum) {
                    message.Append(string.Format(CultureInfo.CurrentCulture,
                        "Field '" + name1 + "' must be an IP in the range {0}, {1} (excluding the boundary values).\n",
                        startIPAddress,
                        endIPAddress));
                }
            }
            if (!string.IsNullOrEmpty(input2)) {
                var input2Num = IPAddressToLongBackwards(input2);
                if (input2Num <= startIPNum || input2Num >= endIPNum) {
                    message.Append(string.Format(CultureInfo.CurrentCulture,
                        "Field '" + name2 + "' must be an IP in the range {0}, {1} (excluding the boundary values).\n",
                        startIPAddress,
                        endIPAddress));
                }
            }
            return message.ToString();
        }

        /// <summary>
        /// Check if the input1 IP address is lesser than input2 IP address.
        /// </summary>
        /// <param name="input1">First IP address.</param>
        /// <param name="input2">Second IP address.</param>
        /// <returns>True if first IP address is lesser than the second IP address.</returns>
        private static bool IsLesser(string input1, string input2) {
            return IPAddressToLongBackwards(input1) <=
            IPAddressToLongBackwards(input2);
        }

        /// <summary>
        /// Converts IP address to number.
        /// </summary>
        /// <param name="IPAddr">IP address.</param>
        /// <returns>IP address in number format.</returns>
        private static uint IPAddressToLongBackwards(string IPAddr) {
            System.Net.IPAddress oIP = System.Net.IPAddress.Parse(IPAddr);
            byte[] byteIP = oIP.GetAddressBytes();

            uint ip = (uint)byteIP[0] << 24;
            ip += (uint)byteIP[1] << 16;
            ip += (uint)byteIP[2] << 8;
            ip += (uint)byteIP[3];

            return ip;
        }
    }
}
