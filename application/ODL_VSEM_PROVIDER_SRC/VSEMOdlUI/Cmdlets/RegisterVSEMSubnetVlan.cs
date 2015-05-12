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
using System.Management.Instrumentation;
using System.Text;
using System.Text.RegularExpressions;
using System.Web.Script.Serialization;
using Microsoft.SystemCenter.NetworkService;
using ODL.VSEMProvider.Cmdlets.Common;
using ODL.VSEMProvider.Libraries;
using ODL.VSEMProvider.Libraries.Common;
using ODL.VSEMProvider.Libraries.Entity;
using ODL.VSEMProvider.VSEMEvents;

namespace ODL.VSEM.Cmdlets {
    /// <summary>
    /// This class represents a cmdlet. This cmdlet is used to register the 
    /// association information of VM sub network and VLAN ID. 
    /// </summary>
    [Cmdlet("Register", "Odl.VSEMSubnetVlan")]
    [OutputType(typeof(VSEMVLANIDMapping))]
    [ManagedName("Microsoft.SystemCenter.NetworkService.RegisterVSEMSubnetVlan")]
    public sealed class RegisterVSEMSubnetVlan : VSEMODLCmdletBase {
        /// <summary>
        /// Name of the VM network.
        /// </summary>
        private string vmNetworkName;

        /// <summary>
        /// Name of the VM network.
        /// </summary>
        [Parameter(Mandatory = false)]
        public string VMNetworkName {
            get {
                return this.vmNetworkName;
            }

            set {
                this.vmNetworkName = value;
            }
        }

        /// <summary>
        /// Name of the VM sub network.
        /// </summary>
        private string vmSubnetworkName;

        /// <summary>
        /// Name of the VM sub network.
        /// </summary>
        [Parameter(Mandatory = false)]
        public string VMSubnetworkName {
            get {
                return this.vmSubnetworkName;
            }

            set {
                this.vmSubnetworkName = value;
            }
        }

        /// <summary>
        /// Host name of the VTNCoordinator.
        /// </summary>
        private string VtnHostName;

        /// <summary>
        /// Host name of the VTNCoordinator
        /// </summary>
        [Parameter(Mandatory = false)]
        public string VtnCoHostName {
            get {
                return this.VtnHostName;
            }

            set {
                this.VtnHostName = value;
            }
        }

        /// <summary>
        /// VLAN ID range.
        /// </summary>
        private string vlanIdList;

        /// <summary>
        /// VLAN ID range.
        /// </summary>
        [Parameter(ValueFromPipeline = true)]
        public string VlanIdList {
            get {
                return this.vlanIdList;
            }

            set {
                this.vlanIdList = value;
            }
        }

        /// <summary>
        /// This function is responsible for validating the parameters.
        /// </summary>
        protected override void BeginODLVSEMCmdlet() {
            if (string.IsNullOrWhiteSpace(this.VlanIdList)) {
                ODLVSEMETW.EventWriteValidateCmdletParameter(this.CmdletName,
                    "NSPluginArgumentException : Mandatory parameter(s) not provided.");
                throw new NSPluginArgumentException("Mandatory parameter(s) not provided.");
            }

            // Check ConnectionParameter validity
            if (string.IsNullOrEmpty(this.VMNetworkName)) {
                this.VMNetworkName = "*";
            }
            if (string.IsNullOrEmpty(this.VMSubnetworkName)) {
                this.VMSubnetworkName = "*";
            }
            if (!Regex.IsMatch(this.VMNetworkName,
                RegularExpressions.VM_NETWORK_NAME_PATTERN_WITH_WILD_CARD)) {
                ODLVSEMETW.EventWriteFailedCmdlet(
                    this.CmdletName,
                    "NSPluginArgumentException : VMNetworkName is invalid.");
                throw new NSPluginArgumentException("VMNetworkName is invalid.");
            }

            if (!Regex.IsMatch(this.VMSubnetworkName,
                RegularExpressions.VM_NETWORK_NAME_PATTERN_WITH_WILD_CARD)) {
                ODLVSEMETW.EventWriteFailedCmdlet(
                    this.CmdletName,
                    "NSPluginArgumentException : VMSubnetworkName is invalid.");
                throw new NSPluginArgumentException("VMSubnetworkName is invalid.");
            }

            if (!Regex.IsMatch(this.VlanIdList, RegularExpressions.VLAN_ID_RANGE_PATTERN)) {
                ODLVSEMETW.EventWriteFailedCmdlet(
                    this.CmdletName,
                    "NSPluginArgumentException : VlanIdList is invalid.");
                throw new NSPluginArgumentException("VlanIdList is invalid.");
            }
            try {
                this.VlanIdList = SortVlanIdRange(this.VlanIdList);
            } catch (Exception ex) {
                ODLVSEMETW.EventWriteFailedCmdlet(this.CmdletName,
                    "NSPluginArgumentException : In VlanIdList, Range should be in format x-y, where x<y.\n" + ex.Message);
                throw new NSPluginArgumentException(
                    "In VlanIdList, Range should be in format x-y, where x<y.");
            }
        }

        /// <summary>
        /// This method is responsible to add specified vlan entry into the local variable.
        /// </summary>
        protected override void DoODLVSEMCmdlet() {
            StringBuilder json = new StringBuilder("\"VMNetworkName\":\"" + this.VMNetworkName + "\"");
            json.Append("\"VMSubnetworkName\":\"" + this.VMSubnetworkName + "\"");
            json.Append("\"VlanIdList\":\"" + this.VlanIdList + "\"");
            json.Append("\"VtnHostName\":\"" + this.VtnCoHostName + "\"");
            ODLVSEMETW.EventWriteDoCmdlet(this.CmdletName,
                "Registering VlanId map information.",
                json.ToString());
            TransactionManager txnMng = new TransactionManager();
            txnMng.StartTransaction();
            var ope = TransactionManager.Operation.None;
            VSEMVLANIDMapping vSEMVLANIDMapping = null;
            try {
                if (string.IsNullOrWhiteSpace(this.VMNetworkName)) {
                    this.VMNetworkName = "*";
                }

                if (string.IsNullOrWhiteSpace(this.VMSubnetworkName)) {
                    this.VMSubnetworkName = "*";
                }

                VLANIDMap vLANIDMap = new VLANIDMap(txnMng,
                    this.VtnCoHostName,
                    TransactionManager.OpenMode.WriteMode);
                vSEMVLANIDMapping = vLANIDMap.RegisterVlanId(this.VMNetworkName,
                    this.VMSubnetworkName,
                    this.VlanIdList);
                ODLVSEMETW.EventWriteReturnLibrary("VlanId map information is registered.", string.Empty);
                ope = TransactionManager.Operation.Commit;
            } catch (Exception ex) {
                Exception exception = VSEMODLExceptionUtil.ConvertExceptionToVSEMException(ex);
                ODLVSEMETW.EventWriteFailedCmdlet(this.CmdletName, exception.GetType() + " : VlanId map information registration is failed. \n" + ex.Message);
                ope = TransactionManager.Operation.Rollback;
                throw exception;
            } finally {
                txnMng.EndTransaction(ope);
                var JavaScriptSerializer = new JavaScriptSerializer();
                JavaScriptSerializer.MaxJsonLength = int.MaxValue;
                string output = "\"vSEMVLANIDMapping\":" + JavaScriptSerializer.Serialize(vSEMVLANIDMapping);
                ODLVSEMETW.EventWriteEndCmdlet(this.CmdletName, output);
            }

            // Return the VSEMVLANIDMapping object
            this.WriteObject(vSEMVLANIDMapping);
        }

        /// <summary>
        /// This method is responsible to sort the range of Vlan Ids in the VlanIdList string.
        /// </summary>
        /// <param name="vlanIds">Unsorted list of range of Vlan Ids.</param>
        /// <returns>Sorted list of range of Vlan Ids.</returns>
        private static string SortVlanIdRange(string vlanIds) {
            List<Range> sortedVlanIds = new List<Range>();
            var vlanIdList = vlanIds.Split(',').ToList();
            int rangeStart = 0;
            int rangeEnd = 0;

            foreach (var vlanIdRange in vlanIdList) {
                rangeStart = int.Parse(vlanIdRange.Split('-')[0], CultureInfo.CurrentCulture);
                if (vlanIdRange.Contains("-")) {
                    rangeEnd = int.Parse(vlanIdRange.Split('-')[1], CultureInfo.CurrentCulture);
                } else {
                    rangeEnd = int.Parse(vlanIdRange.Split('-')[0], CultureInfo.CurrentCulture);
                }
                if (rangeStart > rangeEnd) {
                    throw new ArgumentException("'rangeStart' cannot be greater than 'rangeEnd'.");
                }
                sortedVlanIds.Add(new Range { Start = rangeStart, End = rangeEnd });
            }

            MergeVlanIdRange(sortedVlanIds);

            sortedVlanIds = sortedVlanIds.Distinct(new CustomEqualityRangeComparer()).ToList();
            sortedVlanIds.Sort(new CustomSortRangeComparer());
            StringBuilder vlanIdString = new StringBuilder();
            foreach (var range in sortedVlanIds) {
                if (range.Start == range.End) {
                    vlanIdString.Append(range.Start);
                } else {
                    vlanIdString.Append(range.Start + "-" + range.End);
                }
                vlanIdString.Append(",");
            }
            vlanIdString.Remove(vlanIdString.Length - 1, 1);
            return vlanIdString.ToString();
        }

        /// <summary>
        /// Merge the ranges.
        /// </summary>
        /// <param name="VlanIds">List of ranges of vlan Ids.</param>
        private static void MergeVlanIdRange(List<Range> VlanIds) {
            for (int outerCounter = 0; outerCounter < VlanIds.Count(); outerCounter++) {
                for (int innerCounter = 1; innerCounter < VlanIds.Count(); innerCounter++) {
                    var x = VlanIds.ElementAt(outerCounter);
                    var y = VlanIds.ElementAt(innerCounter);
                    if (x.End >= y.Start && y.End >= x.Start) {
                        if (x.Start < y.Start) {
                            y.Start = x.Start;

                            if (x.End < y.End) {
                                x.End = y.End;
                            } else if (x.End > y.End) {
                                y.End = x.End;
                            }
                        } else if (x.Start > y.Start) {
                            x.Start = y.Start;
                            if (x.End < y.End) {
                                x.End = y.End;
                            } else if (x.End > y.End) {
                                y.End = x.End;
                            }
                        } else {
                            if (x.End < y.End) {
                                x.End = y.End;
                            } else {
                                y.End = x.End;
                            }
                        }
                    }
                }
            }
        }
    }
}
