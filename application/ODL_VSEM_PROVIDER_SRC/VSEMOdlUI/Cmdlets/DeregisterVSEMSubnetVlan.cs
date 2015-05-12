//     Copyright (c) 2013-2014 NEC Corporation
//     All rights reserved.
//     This program and the accompanying materials  are   made
//     available under the terms of the Eclipse Public License
//     v1.0  which  accompanies  this  distribution,  and   is
//     available at  http://www.eclipse.org/legal/epl-v10.html


using System;
using System.Management.Automation;
using System.Management.Instrumentation;
using System.Text;
using System.Text.RegularExpressions;
using System.Web.Script.Serialization;
using Microsoft.SystemCenter.NetworkService;
using ODL.VSEMProvider.Cmdlets.Common;
using ODL.VSEMProvider.Libraries;
using ODL.VSEMProvider.Libraries.Common;
using ODL.VSEMProvider.VSEMEvents;

namespace ODL.VSEM.Cmdlets {
    /// <summary>
    /// This class represents a cmdlet. This cmdlet is used to de-register the association information of VM sub network and VLAN ID. 
    /// </summary>
    [Cmdlet("Unregister", "Odl.VSEMSubnetVlan")]
    [ManagedName("Microsoft.SystemCenter.NetworkService.DeregisterVSEMSubnetVlan")]
    public sealed class DeregisterVSEMSubnetVlan : VSEMODLCmdletBase {
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
        /// Its value specifies which kind of search is to be done for deletion. “ExactMatch” or
        /// “WildCardMatch”.
        /// </summary>
        private string matchType;

        /// <summary>
        /// Its value specifies which kind of search is to be done for deletion. “ExactMatch” or
        /// “WildCardMatch”.
        /// </summary>
        [Parameter(Mandatory = false)]
        public string MatchType {
            get { 
                return this.matchType; 
            }
           
            set { 
                this.matchType = value;
            }
        }

        /// <summary>
        /// This function is responsible for validating the parameters.
        /// </summary>
        protected override void BeginODLVSEMCmdlet() {
            if (string.IsNullOrWhiteSpace(this.VMNetworkName)
                || string.IsNullOrWhiteSpace(this.VMSubnetworkName)) {
                ODLVSEMETW.EventWriteValidateCmdletParameter(this.CmdletName,
                    "NSPluginArgumentException : Mandatory parameter(s) not provided.");
                throw new NSPluginArgumentException("Mandatory parameter(s) not provided.");
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
                        this.CmdletName, "NSPluginArgumentException : VMSubnetworkName is invalid.");
                    throw new NSPluginArgumentException("VMSubnetworkName is invalid.");
            }
            if (string.IsNullOrWhiteSpace(this.MatchType)) {
                this.MatchType = MatchTypes.WildCardMatch.ToString();
            }
            if (this.MatchType != MatchTypes.ExactMatch.ToString()
                && this.MatchType != MatchTypes.WildCardMatch.ToString()) {
                    ODLVSEMETW.EventWriteFailedCmdlet(
                        this.CmdletName,
                        "NSPluginArgumentException : MatchType is invalid.");
                throw new NSPluginArgumentException(
                    "MatchType is invalid. Following is the list of valid MatchType values: \n1. 'ExactMatch'\n2. 'WildCardMatch'");
            }
        }

        /// <summary>
        /// This method is responsible to dump the VlanId map information.
        /// </summary>
        protected override void DoODLVSEMCmdlet() {
            StringBuilder json = new StringBuilder("\"VMNetworkName\":\"" + this.VMNetworkName + "\"");
            json.Append("\"VMSubnetworkName\":\"" + this.VMSubnetworkName + "\"");
            json.Append("\"MatchType\":\"" + this.MatchType + "\"");
            json.Append("\"VTNcoHostName\":\"" + this.VTNCoHostName + "\"");
            ODLVSEMETW.EventWriteDoCmdlet(this.CmdletName,
                "Removing VlanId map information.",
                json.ToString());
            TransactionManager txnMng = new TransactionManager();
            txnMng.StartTransaction();
            var ope = TransactionManager.Operation.None;
            try {
                VLANIDMap vLANIDMap = new VLANIDMap(txnMng,
                    this.VTNCoHostName,
                    TransactionManager.OpenMode.WriteMode);
                vLANIDMap.DeregisterVlanId(this.VMNetworkName,
                    this.VMSubnetworkName,
                    this.MatchType);
                ODLVSEMETW.EventWriteReturnLibrary("VlanId map information is un-registered.", string.Empty);
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
                ODLVSEMETW.EventWriteEndCmdlet(this.CmdletName, string.Empty);
            }
        }
    }
}