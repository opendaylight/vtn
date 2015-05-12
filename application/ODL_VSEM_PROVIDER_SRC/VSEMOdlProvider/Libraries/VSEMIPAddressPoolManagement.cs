//     Copyright (c) 2013-2014 NEC Corporation
//     All rights reserved.
//     This program and the accompanying materials  are   made
//     available under the terms of the Eclipse Public License
//     v1.0  which  accompanies  this  distribution,  and   is
//     available at  http://www.eclipse.org/legal/epl-v10.html


using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Management.Automation;
using System.Net;
using System.Reflection;
using System.Text;
using System.Web.Script.Serialization;
using Microsoft.SystemCenter.NetworkService;
using ODL.VSEMProvider.Libraries.Common;
using ODL.VSEMProvider.Libraries.Entity;
using ODL.VSEMProvider.CTRLibraries;
using ODL.VSEMProvider.VSEMEvents;

namespace ODL.VSEMProvider.Libraries {
    /// <summary>
    /// The classes manages IPAddressPool.
    /// </summary>
    public class VSEMIPAddressPoolManagement {
        /// <summary>
        /// Connection string to use.
        /// </summary>
        public string ConnectionString {
            get;
            private set;
        }

        /// <summary>
        /// Credential to use.
        /// </summary>
        public PSCredential Credential {
            get;
            private set;
        }

        /// <summary>
        /// Initializes the fileds.
        /// </summary>
        /// <param name="connectionString">Connection string of ODL.</param>
        /// <param name="credential">Creadentials of ODL.</param>
        public VSEMIPAddressPoolManagement(string connectionString,
            PSCredential credential) {
            // Verify arguments
            if (string.IsNullOrEmpty(connectionString)) {
                throw new ArgumentException(
                    "The parameter 'connectionString' is null or invalid.");
            }

            if (credential == null) {
                throw new ArgumentException("The parameter 'credential' is null or invalid.");
            }

            this.ConnectionString = connectionString;
            this.Credential = credential;
        }

        /// <summary>
        /// Create an IP Address Pool and add it to the specified VM Subnet.
        /// </summary>
        /// <param name="txnMng">Transaction manager.</param>
        /// <param name="name">Name of IP address pool.</param>
        /// <param name="description">Description for IP address pool.</param>
        /// <param name="subnet">IP address subnet.</param>
        /// <param name="addressStart">Starting address for IP address pool.</param>
        /// <param name="addressEnd">Ending address for IP address pool.</param>
        /// <param name="gatewayInfo">Network gateway info.</param>
        /// <param name="vmNetworkDefinitionId">VM Subnet ID.</param>
        /// <returns>Newly created ip address pool.</returns>
        public IPAddressPool CreateIpAddressPool(TransactionManager txnMng,
            string name,
            string description,
            string subnet,
            string addressStart,
            string addressEnd,
            NetworkGatewayInfo[] gatewayInfo,
            Guid vmNetworkDefinitionId) {
            var JavaScriptSerializer = new JavaScriptSerializer();
            JavaScriptSerializer.MaxJsonLength = int.MaxValue;
            StringBuilder json = new StringBuilder("\"txnMng\":" + JavaScriptSerializer.Serialize(txnMng));
            json.Append(" \"gatewayInfo\":" + JavaScriptSerializer.Serialize(gatewayInfo));
            json.Append("\"name\":\"" + name + "\"");
            json.Append("\"description\":\"" + description + "\"");
            json.Append("\"subnet\":\"" + subnet + "\"");
            json.Append("\"addressStart\":\"" + addressStart + "\"");
            json.Append("\"addressEnd\":\"" + addressEnd + "\"");
            ODLVSEMETW.EventWriteStartLibrary(MethodBase.GetCurrentMethod().Name,
                json.ToString());
            if (txnMng == null) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'txnMng' is null or invalid.");
                throw new ArgumentException("The parameter 'txnMng' is null or invalid.");
            }
            if (string.IsNullOrEmpty(name)) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'name' is null or invalid.");
                throw new ArgumentException("The parameter 'name' is null or invalid.");
            }
            if (string.IsNullOrEmpty(subnet)) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'subnet' is null or invalid.");
                throw new ArgumentException("The parameter 'subnet' is null or invalid.");
            }
            if (string.IsNullOrEmpty(addressStart)) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'addressStart' is null or invalid.");
                throw new ArgumentException("The parameter 'addressStart' is null or invalid.");
            }
            if (string.IsNullOrEmpty(addressEnd)) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'addressEnd' is null or invalid.");
                throw new ArgumentException(
                    "The parameter 'addressEnd' is null or invalid.");
            }
            string VtnHostName = this.ConnectionString;

            if (VtnHostName.StartsWith(@"https://", StringComparison.Ordinal)) {
                VtnHostName = VtnHostName.Substring(8);
            }

            VtnHostName = VtnHostName.Split('.', ':').First();

            var vMNetworkConfig = new VMNetworkConfig(VtnHostName);
            txnMng.SetConfigManager(vMNetworkConfig, TransactionManager.OpenMode.WriteMode);
            if (vMNetworkConfig.VMNetwork.VMNetworkMappingInformation.VMNetworkInfo.Count == 0) {
                ODLVSEMETW.EventWriteConfigManagerFileIOError(
                    MethodBase.GetCurrentMethod().Name,
                    string.Format(CultureInfo.CurrentCulture,
                    "VMNetwork.config {0}",
                    configFileIOErrorValidationMessage));
                throw new InvalidOperationException(
                    string.Format(CultureInfo.CurrentCulture,
                    "VMNetwork.config {0}",
                    configFileIOErrorValidationMessage));
            }

            VMSubnet vmNetDef = null;
            foreach (var vmNet in vMNetworkConfig.VMNetwork.VmNetworks) {
                vmNetDef = vmNet.VMSubnets.FirstOrDefault(vmSubNet =>
                    vmSubNet.Id == vmNetworkDefinitionId);
                if (vmNetDef != null) {
                    break;
                }
            }

            if (vmNetDef == null) {
                ODLVSEMETW.EventWriteValidateVMNetDefinitionError(
                    MethodBase.GetCurrentMethod().Name,
                        string.Format(CultureInfo.CurrentCulture,
                        "VM Subnet '{0}' not found.",
                        vmNetworkDefinitionId.ToString("B")));
                throw new ArgumentException(string.Format(CultureInfo.CurrentCulture,
                    "VM Subnet '{0}' not found.",
                    vmNetworkDefinitionId.ToString("B")));
            }
            
            this.SyncVMSubnet(vmNetworkDefinitionId, vMNetworkConfig);

            IPAddressPool poolAdd = null;

            if (vmNetDef != null) {
                poolAdd = this.CreateIpAddressPool(name,
                    description,
                    subnet,
                    addressStart,
                    addressEnd,
                    gatewayInfo);

                if (poolAdd != null) {
                    AddressFamily addressFamily = AddressFamily.IPv4;
                    IPAddress address = null;
                    var parts = subnet.Split('/');
                    var parsingResult = IPAddress.TryParse(parts[0], out address);
                    if (parsingResult == false || address == null) {
                        ODLVSEMETW.EventWriteArgumentError(
                            MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                            MethodBase.GetCurrentMethod().Name,
                            "The parameter 'subnet' is null or invalid.");
                        throw new ArgumentException("The parameter 'subnet' is invalid.");
                    }
                    switch (address.AddressFamily) {
                        case System.Net.Sockets.AddressFamily.InterNetwork:
                            addressFamily = AddressFamily.IPv4;
                            break;
                        case System.Net.Sockets.AddressFamily.InterNetworkV6:
                            addressFamily = AddressFamily.IPv6;
                            break;
                        default:
                            ODLVSEMETW.EventWriteArgumentError(
                                MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                                MethodBase.GetCurrentMethod().Name,
                                "The parameter 'subnet' is null or invalid.");
                            throw new ArgumentException("The IP address family of subnet is invalid.");
                    }

                    IPSubnet ipsub = null;
                    if (vmNetDef.IPSubnets != null) {
                        ipsub = vmNetDef.IPSubnets.FirstOrDefault(x => x.AddressFamily == addressFamily);
                    } else {
                        vmNetDef.IPSubnets = new IPSubnet[0];
                    }
                    if (ipsub == null) {
                        IPSubnet ipSubnet = new IPSubnet();
                        ipSubnet.Id = Guid.NewGuid();
                        poolAdd.IPSubnetId = ipSubnet.Id;
                        ipSubnet.IPAddressPools = new IPAddressPool[] {
                        poolAdd};
                        ipSubnet.AddressFamily = addressFamily;
                        ipSubnet.LastModifiedTimeStamp = DateTime.Now;
                        ipSubnet.Subnet = subnet;
                        ipSubnet.SupportsDHCP = true;
                        List<IPSubnet> subnets = vmNetDef.IPSubnets.ToList();
                        subnets.Add(ipSubnet);
                        vmNetDef.IPSubnets = subnets.ToArray();
                    } else {
                        List<IPAddressPool> pools = ipsub.IPAddressPools.ToList();
                        pools.Add(poolAdd);
                        poolAdd.IPSubnetId = ipsub.Id;
                        ipsub.IPAddressPools = pools.ToArray();
                        ipsub.LastModifiedTimeStamp = DateTime.Now;
                    }
                    vmNetDef.LastModifiedTimeStamp = DateTime.Now;

                    string error = Validations.IsIPSubnetListValid(
                        vmNetDef.IPSubnets.ToList(), poolAdd.Id);
                    if (!string.IsNullOrEmpty(error)) {
                        ODLVSEMETW.EventWriteValidateVMNetDefinitionError(
                                MethodBase.GetCurrentMethod().Name,
                                error);
                        throw new ArgumentException(error);
                    }
                }
            }

            return poolAdd;
        }

        /// <summary>
        /// Remove the specified IP Address Pool.
        /// </summary>
        /// <param name="txnMng">Transaction manager.</param>
        /// <param name="id">ID of IP Address Pool to remove.</param>
        /// <param name="conn">Connecion object.</param>
        /// <returns>True if successful, else false.</returns>
        public bool RemoveIpAddressPool(TransactionManager txnMng,
            Guid id,
            VSEMConnection conn) {
            var JavaScriptSerializer = new JavaScriptSerializer();
            JavaScriptSerializer.MaxJsonLength = int.MaxValue;
            StringBuilder json = new StringBuilder("\"txnMng\":" + JavaScriptSerializer.Serialize(txnMng));
            json.Append("\"id\":\"" + id + "\"");
            ODLVSEMETW.EventWriteStartLibrary(MethodBase.GetCurrentMethod().Name,
                string.Empty);
            ODLVSEMETW.EventWriteRemoveIpAddressPool(MethodBase.GetCurrentMethod().Name,
                "Removing IP Address pool.");
            if (txnMng == null) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'txnMng' is null or invalid.");
                throw new ArgumentException("The parameter 'txnMng' is null or invalid.");
            }
            if (id == Guid.Empty) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'txnMng' is null or invalid.");
                throw new ArgumentException("The parameter 'txnMng' is null or invalid.");
            }
            if (conn == null) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'txnMng' is null or invalid.");
                throw new ArgumentException("The parameter 'txnMng' is null or invalid.");
            }

            string VtnHostName = this.ConnectionString;

            if (VtnHostName.StartsWith(@"https://", StringComparison.Ordinal)) {
                VtnHostName = VtnHostName.Substring(8);
            }

            VtnHostName = VtnHostName.Split('.', ':').First();

            var vMNetworkConfig = new VMNetworkConfig(VtnHostName);
            txnMng.SetConfigManager(vMNetworkConfig, TransactionManager.OpenMode.WriteMode);

            foreach (var vmNet in vMNetworkConfig.VMNetwork.VmNetworks) {
                foreach (var vmNetDef in vmNet.VMSubnets) {
                    if (vmNetDef.IPSubnets != null) {
                        foreach (var ipsunet in vmNetDef.IPSubnets) {
                            var pool = ipsunet.IPAddressPools.FirstOrDefault(p => p.Id == id);
                            if (pool != null) {
                                var pools = ipsunet.IPAddressPools.ToList();
                                pools.Remove(pool);
                                ipsunet.IPAddressPools = pools.ToArray();
                                ipsunet.LastModifiedTimeStamp = DateTime.Now;
                                string output = "\"output\":" + JavaScriptSerializer.Serialize(true);
                                ODLVSEMETW.EventWriteEndLibrary(MethodBase.GetCurrentMethod().Name,
                                    output);
                                return true;
                            }
                        }
                    }
                }
            }

            string outputLib = "\"output\":" + JavaScriptSerializer.Serialize(false);
            ODLVSEMETW.EventWriteEndLibrary(MethodBase.GetCurrentMethod().Name,
                outputLib);
            return false;
        }

        /// <summary>
        /// Error message for the file IO exception.
        /// </summary>
        private static string configFileIOErrorValidationMessage =
            string.Format(CultureInfo.CurrentCulture,
            "{0}\n{1}\n1. {2}\n2. {3}\n3. {4}",
            "file could not be accessed.",
            "Possible reasons could be:",
            "File is deleted.",
            "File is renamed.",
            "File is tampered.");

        /// <summary>
        /// Update the specified IP Address Pool with the specified info.
        /// </summary>
        /// <param name="txnMng">Transaction manager.</param>
        /// <param name="name">Name of IP Address Pool.</param>
        /// <param name="description">Description for IP address Pool.</param>
        /// <param name="subnet">IP address subnet.</param>
        /// <param name="addressStart">Starting address for IP Address Pool.</param>
        /// <param name="addressEnd">Ending address for IP Address Pool.</param>
        /// <param name="gatewayInfo">Network gateway info.</param>
        /// <param name="conn">VSEM connection object.</param>
        /// <param name="ipAddressPoolId">IP address pool.</param>
        /// <returns>Updated IP address pool.</returns>
        public IPAddressPool UpdateIpAddressPool(TransactionManager txnMng,
            string name,
            string description,
            string subnet,
            string addressStart,
            string addressEnd,
            NetworkGatewayInfo[] gatewayInfo,
            VSEMConnection conn,
            Guid ipAddressPoolId) {
            var JavaScriptSerializer = new JavaScriptSerializer();
            JavaScriptSerializer.MaxJsonLength = int.MaxValue;
            StringBuilder json = new StringBuilder("\"txnMng\":" + JavaScriptSerializer.Serialize(txnMng));
            json.Append(" \"gatewayInfo\":" + JavaScriptSerializer.Serialize(gatewayInfo));
            json.Append("\"name\":\"" + name + "\"");
            json.Append("\"description\":\"" + description + "\"");
            json.Append("\"subnet\":\"" + subnet + "\"");
            json.Append("\"addressStart\":\"" + addressStart + "\"");
            json.Append("\"addressEnd\":\"" + addressEnd + "\"");
            json.Append("\"ipAddressPoolId\":\"" + ipAddressPoolId + "\"");
            ODLVSEMETW.EventWriteStartLibrary(MethodBase.GetCurrentMethod().Name,
                json.ToString());
            if (txnMng == null) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'txnMng' is null or invalid.");
                throw new ArgumentException("The parameter 'txnMng' is null or invalid.");
            }
            if (conn == null) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'conn' is null or invalid.");
                throw new ArgumentException("The parameter 'conn' is null or invalid.");
            }
            if (name != null) {
                if (string.IsNullOrWhiteSpace(name)) {
                    ODLVSEMETW.EventWriteArgumentError(
                        MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                        MethodBase.GetCurrentMethod().Name,
                        "The parameter 'name' is invalid.");
                    throw new ArgumentException(
                        "The parameter 'name' is invalid.");
                }
            }

            string VtnHostName = this.ConnectionString;

            if (VtnHostName.StartsWith(@"https://", StringComparison.Ordinal)) {
                VtnHostName = VtnHostName.Substring(8);
            }

            VtnHostName = VtnHostName.Split('.', ':').First();

            var vMNetworkConfig = new VMNetworkConfig(VtnHostName);
            txnMng.SetConfigManager(vMNetworkConfig, TransactionManager.OpenMode.WriteMode);
            if (vMNetworkConfig.VMNetwork.VMNetworkMappingInformation.VMNetworkInfo.Count == 0) {
                ODLVSEMETW.EventWriteConfigManagerFileIOError(MethodBase.GetCurrentMethod().Name,
                    string.Format(CultureInfo.CurrentCulture,
                    "VMNetwork.config {0}",
                    configFileIOErrorValidationMessage));
                throw new InvalidOperationException(
                    string.Format(CultureInfo.CurrentCulture,
                    "VMNetwork.config {0}",
                    configFileIOErrorValidationMessage));
            }

            VMSubnet vmNetDefFrom = null;
            IPAddressPool pool = null;
            foreach (var vmNet in vMNetworkConfig.VMNetwork.VmNetworks) {
                foreach (var vmNetDef in vmNet.VMSubnets) {
                    if (vmNetDef.IPSubnets != null) {
                        foreach (var ipsunet in vmNetDef.IPSubnets) {
                            pool = ipsunet.IPAddressPools.FirstOrDefault(p => p.Id == ipAddressPoolId);
                            if (pool != null) {
                                vmNetDefFrom = vmNetDef;
                                break;
                            }
                        }
                    }
                    if (vmNetDefFrom != null) {
                        break;
                    }
                }
                if (vmNetDefFrom != null) {
                    break;
                }
            }

            if (vmNetDefFrom == null) {
                ODLVSEMETW.EventWriteValidateIPAddressPoolError(
                    MethodBase.GetCurrentMethod().Name,
                    string.Format(CultureInfo.CurrentCulture,
                    "IP Address Pool '{0}' not found.",
                    ipAddressPoolId));
                throw new ArgumentException(string.Format(CultureInfo.CurrentCulture,
                    "IP Address Pool '{0}' not found.",
                    ipAddressPoolId));
            }
            
            this.SyncVMSubnet(vmNetDefFrom.Id, vMNetworkConfig);

            if (pool != null) {
                // Update the IP Address Pool
                if (name != null) {
                    pool.Name = name;
                }

                if (description != null) {
                    pool.Description = description;
                }

                if (subnet != null) {
                    pool.IPAddressSubnet = subnet;
                }

                if (addressStart != null) {
                    pool.AddressRangeStart = addressStart;
                }

                if (addressEnd != null) {
                    pool.AddressRangeEnd = addressEnd;
                }

                if (gatewayInfo != null) {
                    pool.NetworkGateways = gatewayInfo;
                }
                string error = Validations.IsIPSubnetListValid(
                        vmNetDefFrom.IPSubnets.ToList(),
                        ipAddressPoolId);
                if (!string.IsNullOrEmpty(error)) {
                    ODLVSEMETW.EventWriteValidateVMNetDefinitionError(
                            MethodBase.GetCurrentMethod().Name,
                            error);
                    throw new ArgumentException(error);
                }
            }

            string output = "\"pool\":" + JavaScriptSerializer.Serialize(pool);
            ODLVSEMETW.EventWriteEndLibrary(MethodBase.GetCurrentMethod().Name, output);
            return pool;
        }

        /// <summary>
        /// Synchronize the VMSubnet.
        /// </summary>
        /// <param name="vmNetworkDefinitionId"> ID of the VMSubnet.</param>
        /// <param name="vMNetworkConfig">Instance of VMNetwor.config file.</param>
        private void SyncVMSubnet(Guid vmNetworkDefinitionId, VMNetworkConfig vMNetworkConfig) {
            string vtnName = string.Empty;
            string vbrName = string.Empty;
            VMNetworkInfo vmnetworkMappigInfoFound = null;
            foreach (var vmNet in
                vMNetworkConfig.VMNetwork.VMNetworkMappingInformation.VMNetworkInfo) {
                foreach (var vmSubNet in vmNet.VMSubnetInfo) {
                    if (vmSubNet.VMSubnetID == vmNetworkDefinitionId) {
                        vtnName = vmNet.VTNName;
                        vbrName = vmSubNet.VBridgeName;
                        vmnetworkMappigInfoFound = vmNet;
                        break;
                    }
                }
                if (!string.IsNullOrEmpty(vtnName)) {
                    break;
                }
            }
            VSEMSynchronization vSEMSynchronization =
                new VSEMSynchronization(this.ConnectionString, this.Credential);
            Controller odl = new Controller(this.ConnectionString, this.Credential);
            int odlStatus = odl.CheckVbridgeStatus(vtnName, vbrName);

            if (odlStatus == -2) {
                VSEMSynchronization.RemoveVMNetworkSync(vMNetworkConfig, vmnetworkMappigInfoFound);
                ODLVSEMETW.EventWriteValidateVMNetDefinitionError(
                    MethodBase.GetCurrentMethod().Name,
                        "Corresponding VTN is not found on ODL.");
                throw new DataMisalignedException("Corresponding VTN is not found on ODL.\nRefresh the Odl configuration and then retry.");
            }
            if (odlStatus == -1) {
                VSEMSynchronization.RemoveVMSubnetSync(vMNetworkConfig, vmNetworkDefinitionId, null, null);
                ODLVSEMETW.EventWriteValidateVMNetDefinitionError(
                    MethodBase.GetCurrentMethod().Name,
                        "Corresponding vBridge is not found on ODL.");
                throw new DataMisalignedException("Corresponding vBridge is not found on ODL.\nRefresh the Odl configuration and then retry.");
            }
        }

        /// <summary>
        /// Create a VSEM IP Address Pool.
        /// </summary>
        /// <param name="name">Name of IP address pool.</param>
        /// <param name="description">Description of IP address pool.</param>
        /// <param name="subnet">IP address subnet.</param>
        /// <param name="addressStart">Starting address for IP address pool.</param>
        /// <param name="addressEnd">Ending address for IP address pool.</param>
        /// <param name="gatewayInfo">Network gateway info.</param>
        /// <returns>New instance of VSEMIPAddressPool.</returns>
        private IPAddressPool CreateIpAddressPool(string name,
            string description,
            string subnet,
            string addressStart,
            string addressEnd,
            NetworkGatewayInfo[] gatewayInfo) {
            if (string.IsNullOrEmpty(name)) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'name' is null or invalid.");
                throw new ArgumentException("The parameter 'name' is null or invalid.");
            }
            if (string.IsNullOrEmpty(subnet)) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'subnet' is null or invalid.");
                throw new ArgumentException("The parameter 'subnet' is null or invalid.");
            }
            if (string.IsNullOrEmpty(addressStart)) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'addressStart' is null or invalid.");
                throw new ArgumentException("The parameter 'addressStart' is null or invalid.");
            }
            if (string.IsNullOrEmpty(addressEnd)) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "The parameter 'addressEnd' is null or invalid.");
                throw new ArgumentException(
                    "The parameter 'addressEnd' is null or invalid.");
            }
            IPAddressPool pool = new IPAddressPool();
            pool.Id = Guid.NewGuid();
            pool.Name = name;
            pool.Description = description;
            pool.IPAddressSubnet = subnet;
            pool.AddressRangeStart = addressStart;
            pool.AddressRangeEnd = addressEnd;
            pool.NetworkGateways = gatewayInfo;
            pool.LastModifiedTimeStamp = DateTime.Now;
            pool.IsNetBIOSEnabled = false;

            return pool;
        }
    }
}
