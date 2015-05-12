//     Copyright (c) 2015 NEC Corporation
//     All rights reserved.
//     This program and the accompanying materials  are   made
//     available under the terms of the Eclipse Public License
//     v1.0  which  accompanies  this  distribution,  and   is
//     available at  http://www.eclipse.org/legal/epl-v10.html

using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
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
    /// This class retrieve the value of vlan-id for specific VTN from VSEMVTNVLANMapping
    /// configuration file.
    /// </summary>
    [Cmdlet(VerbsCommon.Get, "Odl.VSEMSubnetVlan")]
    [OutputType(typeof(Collection<VSEMVLANIDMapping>))]
    [ManagedName("Microsoft.SystemCenter.NetworkService.GetVSEMSubnetVlan")]
    public sealed class GetVSEMSubnetVlan : VSEMODLCmdletBase {
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
        private string VtncoHostName;

        /// <summary>
        /// Host name of the VTNCoordinator.
        /// </summary>
        [Parameter(Mandatory = false)]
        public string VTNCoHostName {
            get {
                return this.VtncoHostName;
            }

            set {
                this.VtncoHostName = value;
            }
        }

        /// <summary>
        /// This function is responsible for validating the parameters.
        /// </summary>
        protected override void BeginODLVSEMCmdlet() {
            if (string.IsNullOrEmpty(this.VMNetworkName)) {
                this.VMNetworkName = "*";
            }
            if (string.IsNullOrEmpty(this.VMSubnetworkName)) {
                this.VMSubnetworkName = "*";
            }

            // Check ConnectionParameter validity
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
        }

        /// <summary>
        /// This method is responsible to retrieve the VlanId map information.
        /// </summary>
        protected override void DoODLVSEMCmdlet() {
            StringBuilder json = new StringBuilder("\"VMNetworkName\":\"" + this.VMNetworkName + "\"");
            json.Append("\"VMSubnetworkName\":\"" + this.VMSubnetworkName + "\"");
            json.Append("\"VtncoHostName\":\"" + this.VTNCoHostName + "\"");
            ODLVSEMETW.EventWriteDoCmdlet(this.CmdletName,
                "Retrieving VlanId map information.",
                json.ToString());
            TransactionManager txnMng = new TransactionManager();
            txnMng.StartTransaction();
            var ope = TransactionManager.Operation.None;
            List<VSEMVLANIDMapping> vSEMVLANIDMapping = new List<VSEMVLANIDMapping>();
            try {
                VLANIDMap vLANIDMap = new VLANIDMap(txnMng,
                    this.VTNCoHostName,
                    TransactionManager.OpenMode.ReadMode);

                vSEMVLANIDMapping.AddRange(vLANIDMap.SelectVlanIdWildCardMatch(
                    this.VMNetworkName,
                    this.VMSubnetworkName));

                ODLVSEMETW.EventWriteReturnLibrary("VlanId map information is retrieved.", string.Empty);
                ope = TransactionManager.Operation.Commit;
            } catch (Exception ex) {
                Exception exception = VSEMODLExceptionUtil.ConvertExceptionToVSEMException(ex);
                ODLVSEMETW.EventWriteFailedCmdlet(this.CmdletName, exception.GetType() + " : " + ex.Message);
                ope = TransactionManager.Operation.Rollback;
                throw exception;
            } finally {
                txnMng.EndTransaction(ope);

                var JavaScriptSerializer = new JavaScriptSerializer();
                JavaScriptSerializer.MaxJsonLength = int.MaxValue;
                string output = "\"vSEMVLANIDMapping\":" + JavaScriptSerializer.Serialize(vSEMVLANIDMapping);
                ODLVSEMETW.EventWriteEndCmdlet(this.CmdletName, output);
            }
            if (vSEMVLANIDMapping == null || vSEMVLANIDMapping.Count == 0) {
                this.WriteWarning("Vlan-mapping not found.");
            } else {
                this.WriteObject(vSEMVLANIDMapping);
            }
        }
    }
}
