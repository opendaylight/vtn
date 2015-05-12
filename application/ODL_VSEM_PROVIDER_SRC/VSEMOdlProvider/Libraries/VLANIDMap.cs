//     Copyright (c) 2015 NEC Corporation
//     All rights reserved.
//     This program and the accompanying materials  are   made
//     available under the terms of the Eclipse Public License
//     v1.0  which  accompanies  this  distribution,  and   is
//     available at  http://www.eclipse.org/legal/epl-v10.html

using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Text.RegularExpressions;
using System.Web.Script.Serialization;
using ODL.VSEMProvider.Libraries.Common;
using ODL.VSEMProvider.Libraries.Entity;
using ODL.VSEMProvider.VSEMEvents;

namespace ODL.VSEMProvider.Libraries {
    /// <summary>
    /// This class is responsible for the management of VLAN ID mapping information.
    /// </summary>
    public class VLANIDMap {
        /// <summary>
        /// Error message for the file IO exception.
        /// </summary>
        private static string configFileIOErrorValidationMessage =
            string.Format(CultureInfo.CurrentCulture,
            "{0}\n{1}\n1. {2}\n2. {3}\n3. {4}\n4. {5}",
            "VLANIDMapping.config file could not be accessed.",
            "Possible reasons could be:",
            "File is not yet created. A VLAN ID must be registered first.",
            "File is deleted.",
            "File is renamed.",
            "File is tampered.");

        /// <summary>
        /// This parameter is used to store the VLAN-map information.
        /// </summary>
        private List<VSEMVLANIDMapping> vlanMapInfo;

        /// <summary>
        /// This constructor is responsible for instantiating the vlanMapInfo.
        /// </summary>
        /// <param name="txnMng">Transaction manager object.</param>
        /// <param name="VtnHostName">Parent Folder name.</param>
        /// <param name="mode">Write or Read mode in which file to be opened.</param>
        public VLANIDMap(TransactionManager txnMng,
            string VtnHostName,
            TransactionManager.OpenMode mode) {
            var JavaScriptSerializer = new JavaScriptSerializer();
            JavaScriptSerializer.MaxJsonLength = int.MaxValue;
            StringBuilder json = new StringBuilder("\"vMNetworkName\":\"" + VtnHostName + "\"");
            json.Append(" \"TransactionManager\":" + JavaScriptSerializer.Serialize(txnMng));
            json.Append(" \"OpenMode\":\"" + mode + "\"");
            ODLVSEMETW.EventWriteStartLibrary(
                MethodBase.GetCurrentMethod().Name,
                json.ToString());
            if (txnMng == null) {
                throw new ArgumentException(
                    "Parameter 'txnMng' is null or invalid.");
            }
            if (string.IsNullOrWhiteSpace(VtnHostName)) {
                VSEMConfig vsemConfig = new VSEMConfig();
                txnMng.SetConfigManager(vsemConfig, TransactionManager.OpenMode.ReadMode);
                VtnHostName = vsemConfig.Info.ServerName;
            }
            if (string.IsNullOrWhiteSpace(VtnHostName)) {
                throw new ArgumentException(
                        "Parameter 'VTNCoordinatorHostName' is null or invalid.");
            }
            try {
                VLANIDMappingConfig vLANIDMappingConfig = new VLANIDMappingConfig(VtnHostName);
                txnMng.SetConfigManager(vLANIDMappingConfig, mode);
                this.vlanMapInfo = vLANIDMappingConfig.VLANIDMapping;
            } catch (System.IO.IOException) {
                ODLVSEMETW.EventWriteConfigManagerFileIOError(MethodBase.GetCurrentMethod().Name,
                    configFileIOErrorValidationMessage);
                throw new InvalidOperationException(
                    configFileIOErrorValidationMessage);
            }
        }

        /// <summary>
        /// This method is responsible for retrieving vlan Id corresponding to the exact match of
        /// the vMNetworkName and vMSubnetworkName in vlanMapInfo.
        /// </summary>
        /// <param name="vMNetworkName">Name of the VM network.</param>
        /// <param name="vMSubnetworkName">Name of the VM sub network.</param>
        /// <returns>Config entry corresponding to the exact match.</returns>
        public List<VSEMVLANIDMapping> SelectVlanIdExactMatch(string vMNetworkName,
            string vMSubnetworkName) {
            ODLVSEMETW.EventWriteSelectVlanIdExactMatch(
                "Retrieving vlan ID for Exact match of VM network name and VM Subnet name.",
                string.Empty);
            if (this.vlanMapInfo == null) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "VLAN map Information is null.");
                throw new ArgumentException("VLAN-map information is null.");
            }

            var found = this.vlanMapInfo.Where(vlanIdMap =>
               (string.IsNullOrEmpty(vMNetworkName)
               || string.Compare(vlanIdMap.VMNetworkName, vMNetworkName, StringComparison.Ordinal) == 0)
               && (string.IsNullOrEmpty(vMSubnetworkName)
               || string.Compare(vlanIdMap.VMSubNetworkName, vMSubnetworkName, StringComparison.Ordinal) == 0));
            return found.ToList();
        }

        /// <summary>
        /// This method is responsible for retrieving vla Id corresponding to the wild card match 
        /// of the vMNetworkName and vMSubnetworkName in vlanMapInfo.
        /// </summary>
        /// <param name="vMNetworkName">Name of the VM network.</param>
        /// <param name="vMSubnetworkName">Name of the VM sub network.</param>
        /// <returns>Config entry corresponding to the wild card match.</returns>
        public List<VSEMVLANIDMapping> SelectVlanIdWildCardMatch(string vMNetworkName,
            string vMSubnetworkName) {
                StringBuilder json = new StringBuilder("\"vMNetworkName\":\"" + vMNetworkName + "\"");
                json.Append(" \"vMSubnetworkName\":\"" + vMSubnetworkName + "\"");
            ODLVSEMETW.EventWriteStartLibrary(
                MethodBase.GetCurrentMethod().Name,
                json.ToString());
            ODLVSEMETW.EventWriteSelectVlanIdWildcardMatch(
                    "Retriving vlan ID for Wildcard match of vMNetworkName and vMSubnetworkName.", 
                    string.Empty);
            if (string.IsNullOrEmpty(vMNetworkName)) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "Parameter 'vMNetworkName' is null or invalid.");
                throw new ArgumentException(
                    "Parameter 'vMNetworkName' is null or invalid.");
            }
            if (string.IsNullOrEmpty(vMSubnetworkName)) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "Parameter 'vMSubnetworkName' is null or invalid.");
                throw new ArgumentException(
                    "Parameter 'vMSubnetworkName' is null or invalid.");
            }
            if (this.vlanMapInfo == null) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "Parameter 'vlanMapInfo' is null or invalid.");
                throw new ArgumentException(
                    "Parameter 'vlanMapInfo' is null or invalid.");
            }

            List<VSEMVLANIDMapping> found = new List<VSEMVLANIDMapping>();

            found.AddRange(this.vlanMapInfo.Where(vlanIdMap =>
                Regex.IsMatch(vlanIdMap.VMNetworkName.Replace("*", string.Empty),
                "^(" + vMNetworkName.Replace("*", ".*") + ")$")));

            found.RemoveAll(vlanIdMap =>
                !Regex.IsMatch(vlanIdMap.VMSubNetworkName.Replace("*", string.Empty),
                "^(" + vMSubnetworkName.Replace("*", ".*") + ")$"));

            return found;
        }

        /// <summary>
        /// This method is responsible for retrieving vlan Id corresponding to the exact match
        /// and partial match of the vMNetworkName and vMSubnetworkName in vlanMapInfo
        /// according to match priorities.
        /// </summary>
        /// <param name="vMNetworkName">Name of the VM network.</param>
        /// <param name="vMSubnetworkName">Name of the VM sub network.</param>
        /// <returns>Vlan Id corresponding to the specified parameters.</returns>
        public string GetVlanId(string vMNetworkName, string vMSubnetworkName) {
            if (string.IsNullOrEmpty(vMNetworkName)) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "Parameter 'vMNetworkName' is null or invalid.");
                throw new ArgumentException(
                    "Parameter 'vMNetworkName' is null or invalid.");
            }
            if (string.IsNullOrEmpty(vMSubnetworkName)) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "Parameter 'vMSubnetworkName' is null or invalid.");
                throw new ArgumentException(
                    "Parameter 'vMSubnetworkName' is null or invalid.");
            }
            if (this.vlanMapInfo == null) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "Parameter 'vlanMapInfo' is null or invalid.");
                throw new ArgumentException(
                        "Parameter 'vlanMapInfo' is null or invalid.");
            }
            string vlanid = string.Empty;
            ODLVSEMETW.EventWriteCreateVMsubNetwork(MethodBase.GetCurrentMethod().Name,
                    "Allocating Vlan ID to VM SubNetwork.");
            var vmnetwork =
               this.vlanMapInfo.FirstOrDefault(x => x.VMNetworkName
                   == vMNetworkName);
            if (vmnetwork != null) {
                var vmsubnetwork = this.StringMatch(vMSubnetworkName,
                      this.vlanMapInfo.Where(x => x.VMNetworkName ==
                          vmnetwork.VMNetworkName).Select(
                      x => x.VMSubNetworkName).ToList());
                if (!string.IsNullOrEmpty(vmsubnetwork)) {
                    vlanid = (from sub in this.vlanMapInfo
                              where sub.VMNetworkName == vmnetwork.VMNetworkName
                              && sub.VMSubNetworkName == vmsubnetwork
                              select sub.VlanId).FirstOrDefault();
                }
            }

            if (string.IsNullOrEmpty(vlanid)) {
                List<VSEMVLANIDMapping> mapping =
                    this.vlanMapInfo.Where(
                    map => (string.Compare(map.VMNetworkName, vMNetworkName, StringComparison.Ordinal) == 0
                        || map.VMNetworkName.Contains("*"))
                        && Regex.IsMatch(vMNetworkName,
                        map.VMNetworkName.Replace(
                        "*", ".*"))).ToList();

                mapping.Sort(new PartialMatchComparer(vMNetworkName));

                var results = from map in mapping
                              group map by map.VMNetworkName into g
                              select new { VMNets = g.Key, VMSubnets = g.ToList() };

                foreach (var result in results) {
                    var found = this.StringMatch(vMSubnetworkName,
                         result.VMSubnets.Select(x => x.VMSubNetworkName).ToList());

                    if (!string.IsNullOrEmpty(found)) {
                        vlanid = (from sub in result.VMSubnets
                                  where string.Compare(sub.VMSubNetworkName, found, StringComparison.Ordinal) == 0
                                  select sub.VlanId).FirstOrDefault();
                        break;
                    }
                }
            }
            return vlanid;
        }

        /// <summary>
        /// This method is responsible to register the VlanId map in VLANIDMapping.config file.
        /// </summary>
        /// <param name="vmNetworkName">Name of the VM network.</param>
        /// <param name="vmSubnetworkName">Name of the VM sub network.</param>
        /// <param name="vlanIdList">Vlan Id list to be registered.</param>
        /// <returns>Entry made in config file.</returns>
        public VSEMVLANIDMapping RegisterVlanId(string vmNetworkName,
            string vmSubnetworkName,
            string vlanIdList) {
            var JavaScriptSerializer = new JavaScriptSerializer();
            JavaScriptSerializer.MaxJsonLength = int.MaxValue;
                StringBuilder json = new StringBuilder("\"vmNetworkName\":\"" + vmNetworkName + "\"");
                json.Append(" \"vmSubnetworkName\":\"" + vmSubnetworkName + "\"");
                json.Append(" \"vlanIdList\":\"" + vlanIdList + "\"");
            ODLVSEMETW.EventWriteStartLibrary(
                MethodBase.GetCurrentMethod().Name,
                json.ToString());
            ODLVSEMETW.EventWriteRegisterVlan(
                "Registering VlanId map in VLANIDMapping.config.",
                string.Empty);
            if (string.IsNullOrEmpty(vmNetworkName)) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "Parameter 'vmNetworkName' is null or invalid.");
                throw new ArgumentException(
                    "Parameter 'vmNetworkName' is null or invalid.");
            }
            if (string.IsNullOrEmpty(vmSubnetworkName)) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "Parameter 'vmSubnetworkName' is null or invalid.");
                throw new ArgumentException(
                    "Parameter 'vmSubnetworkName' is null or invalid.");
            }
            if (string.IsNullOrEmpty(vlanIdList)) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "Parameter 'vlanIdList' is null or invalid.");
                throw new ArgumentException(
                    "Parameter 'vlanIdList' is null or invalid.");
            }
            if (this.vlanMapInfo == null) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "Parameter 'vlanMapInfo' is null or invalid.");
                throw new ArgumentException(
                    "Parameter 'vlanMapInfo' is null or invalid.");
            }

            var found = this.SelectVlanIdExactMatch(vmNetworkName, vmSubnetworkName);
            ODLVSEMETW.EventWriteReturnLibrary(string.Format(CultureInfo.CurrentCulture,
                "Vlan ID is retieved for VMNetworkName = {0} and VMSubnetworkName = {1}.",
                vmNetworkName,
                vmSubnetworkName),
                string.Empty);
            if (found == null || found.Count == 0) {
                found.Add(new VSEMVLANIDMapping {
                    VMNetworkName = vmNetworkName,
                    VMSubNetworkName = vmSubnetworkName,
                    VlanId = vlanIdList,
                    LastModifiedTimeStamp = DateTime.Now
                });
                this.vlanMapInfo.AddRange(found);
            } else {
                found.FirstOrDefault().VlanId = vlanIdList;
                found.FirstOrDefault().LastModifiedTimeStamp = DateTime.Now;
            }

            string output = "\"VSEMVLANIDMapping\":" + JavaScriptSerializer.Serialize(found.FirstOrDefault());
            ODLVSEMETW.EventWriteEndLibrary(MethodBase.GetCurrentMethod().Name, output);
            return found.FirstOrDefault();
        }

        /// <summary>
        /// This method is responsible to deregister the VlanId map in config file.
        /// </summary>
        /// <param name="vMNetworkName">Name of the VM network.</param>
        /// <param name="vMSubnetworkName">Name of the VM sub network.</param>
        /// <param name="matchType">Its value specifies which kind of search is to be done
        /// for deletion. “ExactMatch” or “WildCardMatch”.</param>
        public void DeregisterVlanId(string vMNetworkName,
            string vMSubnetworkName,
            string matchType) {
                StringBuilder json = new StringBuilder("\"vMNetworkName\":\"" + vMNetworkName + "\"");
                json.Append(" \"vMSubnetworkName\":\"" + vMSubnetworkName + "\"");
                json.Append(" \"matchType\":\"" + matchType + "\"");
            ODLVSEMETW.EventWriteStartLibrary(
                MethodBase.GetCurrentMethod().Name,
                json.ToString());

            if (string.IsNullOrEmpty(vMNetworkName)) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "Parameter 'vMNetworkName' is null or invalid.");
                throw new ArgumentException(
                    "Parameter 'vMNetworkName' is null or invalid.");
            }
            if (string.IsNullOrEmpty(vMSubnetworkName)) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "Parameter 'vMSubnetworkName' is null or invalid.");
                throw new ArgumentException(
                    "Parameter 'vMSubnetworkName' is null or invalid.");
            }
            ODLVSEMETW.EventWriteCheckMatchType(
                   "Checking Match Type From Input Parameter.",
                   string.Empty);
            if (string.IsNullOrEmpty(matchType)
                || (string.Compare(matchType, MatchTypes.ExactMatch.ToString(), StringComparison.Ordinal) == 0
                && string.Compare(matchType, MatchTypes.WildCardMatch.ToString(), StringComparison.Ordinal) == 0)) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    "Either matchType is not specified or its value is invalid.");
                throw new ArgumentException(
                    "Either matchType is not specified or its value is invalid.");
            }
            if (this.vlanMapInfo == null) {
                throw new ArgumentException("VLAN-map information is not null.");
            }

            List<VSEMVLANIDMapping> found = new List<VSEMVLANIDMapping>();
            if (string.Compare(matchType, MatchTypes.ExactMatch.ToString(), StringComparison.Ordinal) == 0) {
                found.AddRange(this.SelectVlanIdExactMatch(vMNetworkName, vMSubnetworkName));
            } else {
                found.AddRange(this.SelectVlanIdWildCardMatch(vMNetworkName, vMSubnetworkName));
            }

            if (found == null || found.Count == 0) {
                ODLVSEMETW.EventWriteArgumentError(
                    MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                    MethodBase.GetCurrentMethod().Name,
                    string.Format(CultureInfo.CurrentCulture,
                    "Mapping not found for VMNetworkName = {0} and VMSubnetName = {1}.",
                    vMNetworkName,
                    vMSubnetworkName));
                throw new InvalidOperationException(string.Format(CultureInfo.CurrentCulture,
                    "Mapping not found for VMNetworkName = {0} and VMSubnetName = {1}.",
                    vMNetworkName,
                    vMSubnetworkName));
            } else {
                found.ForEach(vlanIdMap => this.vlanMapInfo.Remove(vlanIdMap));
            }
            ODLVSEMETW.EventWriteEndLibrary(MethodBase.GetCurrentMethod().Name, string.Empty);
        }

        /// <summary>
        /// This method is responsible for searching the match of the "element"
        /// in the "pool" with the priority of exact match over partial match
        /// and the priority of the longer partial match over smaller partial match.
        /// </summary>
        /// <param name="element">Element to be searched in the pool.</param>
        /// <param name="pool">List of elements.</param>
        /// <returns>Element found in the pool.</returns>
        private string StringMatch(string element, List<string> pool) {
            if (string.IsNullOrEmpty(element)) {
                throw new ArgumentException(
                    "Parameter 'element' is null or invalid.");
            }
            if (pool == null) {
                throw new ArgumentException(
                    "Parameter 'pool' is null or invalid.");
            }

            string found = string.Empty;
            if (pool.Contains(element)) {
                found = element;
            } else {
                int match = 0;
                int matchFound = 0;
                foreach (var name in pool) {
                    if (name.Contains("*")) {
                        matchFound = Regex.Match(
                            element, "^" + name.Replace("*", string.Empty)).Length;
                        if (matchFound > match) {
                            found = name;
                            match = matchFound;
                        }
                    }
                }
                if (match == 0 && pool.Contains("*")) {
                    found = "*";
                }
            }
            return found;
        }
    }
}
