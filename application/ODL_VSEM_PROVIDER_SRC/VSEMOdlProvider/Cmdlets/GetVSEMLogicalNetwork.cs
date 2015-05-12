//     Copyright (c) 2013-2014 NEC Corporation
//     All rights reserved.
//     This program and the accompanying materials  are   made
//     available under the terms of the Eclipse Public License
//     v1.0  which  accompanies  this  distribution,  and   is
//     available at  http://www.eclipse.org/legal/epl-v10.html


using System;
using System.Collections.Generic;
using System.Globalization;
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
    /// This class represents a cmdlet. This cmdlet is used to retrieves the information about the
    /// fabric network definition(s) that are managed by the virtual switch extension manager. 
    /// </summary>
    [Cmdlet(VerbsCommon.Get, "Odl.VSEMLogicalNetwork")]
    [OutputType(typeof(List<LogicalNetwork>))]
    [ManagedName("Microsoft.SystemCenter.NetworkService.GetLogicalNetwork")]
    public sealed class GetVSEMLogicalNetwork : VSEMODLCmdletBase {
        /// <summary>
        /// Logical Network Definition Id. 
        /// </summary>
        private Guid iD;

        /// <summary>
        /// Fabric Network Definition Id. 
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
        /// This function is responsible to retrieve the VSEMFabricNetworkDefinition information 
        /// for the connection.
        /// </summary>
        protected override void DoODLVSEMCmdlet() {
            var JavaScriptSerializer = new JavaScriptSerializer();
            JavaScriptSerializer.MaxJsonLength = int.MaxValue;
            StringBuilder json = new StringBuilder(" \"ID\":" + JavaScriptSerializer.Serialize(this.ID));
            ODLVSEMETW.EventWriteDoCmdlet(this.CmdletName,
                "Retrieving logical network definition.",
                json.ToString());
            List<LogicalNetwork> vmNw = null;
            TransactionManager txnMng = new TransactionManager();
            txnMng.StartTransaction();
            var operation = TransactionManager.Operation.None;
            try {
                if (this.ID != Guid.Empty) {
                    vmNw = new List<LogicalNetwork>();
                    vmNw.Add(this.conn.GetVSEMLogicalNetworkById(txnMng, this.ID));
                    ODLVSEMETW.EventWriteReturnLibrary(string.Format(CultureInfo.CurrentCulture,
                        "Logical network definition is retrieved by ID: {0}.",
                        this.ID.ToString("B")),
                        string.Empty);

                    if (vmNw[0] == null) {
                        ODLVSEMETW.EventWriteProcessCmdletWarning(this.CmdletName,
                        string.Format(CultureInfo.CurrentCulture,
                        "ID '{0}' not found.",
                        this.ID.ToString("B")));
                        this.WriteWarning(string.Format(CultureInfo.CurrentCulture,
                            "ID '{0}' not found.",
                            this.ID.ToString("B")));
                    } else {
                        var hnvmgmt = new HNVLogicalNetworkManagement(this.conn.GetVtnHostName());

                        // Validate HNV LogicalNetwork. Need not check return value.
                        hnvmgmt.IsLogicalNetworkValid(vmNw[0]);
                        this.WriteObject(vmNw, true);
                    }
                } else {
                    vmNw = this.conn.GetVSEMLogicalNetwork(txnMng);
                    ODLVSEMETW.EventWriteReturnLibrary(
                        "Logical networks are retrieved.", string.Empty);
                    if (vmNw == null) {
                        ODLVSEMETW.EventWriteProcessCmdletWarning(this.CmdletName,
                            "Logical network not found.");
                        this.WriteWarning("Logical network not found.");
                    } else {
                        var hnvmgmt = new HNVLogicalNetworkManagement(this.conn.GetVtnHostName());
                        for (int i = 0; i < vmNw.Count; i++) {
                            // Validate HNV LogicalNetworks. Need not check return value.
                            hnvmgmt.IsLogicalNetworkValid(vmNw[i]);
                        }
                        this.WriteObject(vmNw, true);
                    }
                }
            } catch (Exception ex) {
                Exception exception = VSEMODLExceptionUtil.ConvertExceptionToVSEMException(ex);
                ODLVSEMETW.EventWriteFailedCmdlet(this.CmdletName, exception.GetType() + " : " + ex.Message);
                operation = TransactionManager.Operation.Rollback;
                throw exception;
            } finally {
                txnMng.EndTransaction(operation);

                string output = "\"LogicalNetwork\":" + JavaScriptSerializer.Serialize(vmNw);
                ODLVSEMETW.EventWriteEndCmdlet(this.CmdletName, output);
            }
        }
    }
}
