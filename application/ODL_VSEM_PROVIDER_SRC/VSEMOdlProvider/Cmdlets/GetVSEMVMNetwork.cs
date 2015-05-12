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
using System.Management.Automation;
using System.Management.Instrumentation;
using System.Text;
using System.Web.Script.Serialization;
using Microsoft.SystemCenter.NetworkService;
using ODL.VSEMProvider.Cmdlets.Common;
using ODL.VSEMProvider.Libraries;
using ODL.VSEMProvider.Libraries.Common;
using ODL.VSEMProvider.VSEMEvents;
using VSEM.Cmdlets.Common;

namespace ODL.VSEMProvider.Cmdlets {
    /// <summary>
    /// VM network retrieve cmdlet.
    /// </summary>
    [Cmdlet(VerbsCommon.Get, "Odl.VSEMVMNetwork")]
    [OutputType(typeof(List<VMNetwork>))]
    [ManagedName("Microsoft.SystemCenter.NetworkService.GetVMNetwork")]
    public sealed class GetVSEMVMNetwork : VSEMODLCmdletBase {
        /// <summary>
        /// Id of vmnetwork that is managed by this Extension Manager.
        /// </summary>
        private Guid iD;

        /// <summary>
        /// Id of vmnetwork that is managed by this Extension Manager.
        /// </summary>
        [Parameter(Mandatory = false)]
        public Guid ID {
            get {
                return this.iD;
            }

            set {
                this.iD = value;
            }
        }

        /// <summary>
        /// Connection object returned by the Open Connection cmdlet with the target extension
        /// manager. This is a mandatory parameter.
        /// </summary>
        private IConnection connection;

        /// <summary>
        /// This parameter specifies Connection object.
        /// </summary>
        [Parameter(ValueFromPipeline = true)]
        public IConnection Connection {
            get {
                return this.connection;
            }

            set {
                this.connection = value;
            }
        }

        /// <summary>
        /// The connection object parsed to VSEMConnection.
        /// </summary>
        private VSEMConnection conn;

        /// <summary>
        /// This function is responsible for validating the parameters.
        /// </summary>
        protected override void BeginODLVSEMCmdlet() {
            this.conn = VSEMODLCmdletUtility.ValidateConnectionObject(this.Connection, this.CmdletName);
        }

        /// <summary>
        /// This function is responsible to retrieve the information
        /// about the vm network(s) for the connection.
        /// </summary>
        protected override void DoODLVSEMCmdlet() {
            var JavaScriptSerializer = new JavaScriptSerializer();
            JavaScriptSerializer.MaxJsonLength = int.MaxValue;
            StringBuilder json = new StringBuilder(" \"ID\":" + JavaScriptSerializer.Serialize(this.ID));
            ODLVSEMETW.EventWriteDoCmdlet(this.CmdletName,
                "Retrieving VM network(s).",
                json.ToString());

            List<VMNetwork> vmNetworks = null;
            TransactionManager txnMng = new TransactionManager();
            txnMng.StartTransaction();
            var operation = TransactionManager.Operation.None;
            try {
                if (this.ID != Guid.Empty) {
                    vmNetworks = new List<VMNetwork>();
                    vmNetworks.Add(this.conn.GetVSEMVMNetworkById(txnMng, this.ID));
                    ODLVSEMETW.EventWriteReturnLibrary(string.Format(CultureInfo.CurrentCulture,
                        "VM network information is retrieved by ID: {0}",
                        this.ID.ToString("B")),
                        string.Empty);
                    if (vmNetworks[0] == null) {
                        ODLVSEMETW.EventWriteProcessCmdletWarning(this.CmdletName,
                        string.Format(CultureInfo.CurrentCulture,
                        "ID {0} not found.",
                        this.ID.ToString("B")));
                        this.WriteWarning(string.Format(CultureInfo.CurrentCulture,
                            "ID {0} not found.",
                            this.ID.ToString("B")));
                    } else {
                        if (vmNetworks.Count > 0 && vmNetworks[0].Id != Guid.Empty) {
                            GetVSEMVMNetwork.FilterIPAddressPool(vmNetworks);
                        }
                        this.WriteObject(vmNetworks, true);
                    }
                } else {
                    vmNetworks = new List<VMNetwork>();
                    vmNetworks = this.conn.GetVSEMVMNetwork(txnMng);
                    ODLVSEMETW.EventWriteReturnLibrary(
                        "VM network information is retrieved",
                        string.Empty);
                    if (vmNetworks == null) {
                        ODLVSEMETW.EventWriteProcessCmdletWarning(this.CmdletName,
                        "VM network not found.");
                        this.WriteWarning("VM network not found.");
                    } else {
                        if (vmNetworks.Count > 0) {
                            GetVSEMVMNetwork.FilterIPAddressPool(vmNetworks);
                        }
                        this.WriteObject(vmNetworks, true);
                    }
                }
            } catch (Exception ex) {
                Exception exception = VSEMODLExceptionUtil.ConvertExceptionToVSEMException(ex);
                ODLVSEMETW.EventWriteFailedCmdlet(this.CmdletName, exception.GetType() + " : " + ex.Message);
                operation = TransactionManager.Operation.Rollback;
                throw exception;
            } finally {
                txnMng.EndTransaction(operation);
                string output = "\"VMNetwork\":" + JavaScriptSerializer.Serialize(vmNetworks);
                ODLVSEMETW.EventWriteEndCmdlet(this.CmdletName, output);
            }
        }

        /// <summary>
        /// Get-Odl.VSEMVmNetwork returns poolID 0
        /// if other IP address pool does not exist. If exist, return it.
        /// </summary>
        /// <param name="vmnetworks">List of vmnetworks.</param>
        private static void FilterIPAddressPool(List<VMNetwork> vmnetworks) {
            foreach (var vmnet in vmnetworks) {
                foreach (var vmnetdef in vmnet.VMSubnets) {
                    foreach (var subnet in vmnetdef.IPSubnets) {
                        var temp = subnet.IPAddressPools.ToList();
                        temp.RemoveAll(pool => pool.Id == Guid.Empty);
                        subnet.IPAddressPools = temp.ToArray();
                    }
                }
            }
        }
    }
}
