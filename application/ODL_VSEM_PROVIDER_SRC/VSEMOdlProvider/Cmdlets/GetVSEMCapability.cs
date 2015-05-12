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
using System.Web.Script.Serialization;
using Microsoft.SystemCenter.NetworkService;
using ODL.VSEMProvider.Cmdlets.Common;
using ODL.VSEMProvider.Libraries;
using ODL.VSEMProvider.Libraries.Common;
using ODL.VSEMProvider.VSEMEvents;
using VSEM.Cmdlets.Common;

namespace ODL.VSEMProvider.Cmdlets {
    /// <summary>
    /// This class represents a cmdlet. This cmdlet is used to retrieves the virtual switch 
    /// extension manager system information.
    /// </summary>
    [Cmdlet(VerbsCommon.Get, "Odl.VSEMCapability")]
    [OutputType(typeof(NetworkServiceSystemInformation))]
    [ManagedName("Microsoft.SystemCenter.NetworkService.GetDeviceCapability")]
    public sealed class GetVSEMCapability : VSEMODLCmdletBase {
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
        /// This function is responsible to retrieve the SystemInfo for the connection.
        /// </summary>
        protected override void DoODLVSEMCmdlet() {
            var JavaScriptSerializer = new JavaScriptSerializer();
            JavaScriptSerializer.MaxJsonLength = int.MaxValue;
            ODLVSEMETW.EventWriteDoCmdlet(this.CmdletName,
                "Retrieving system information.",
                string.Empty);
            NetworkServiceSystemInformation info = null;
            TransactionManager txnMng = new TransactionManager();
            txnMng.StartTransaction();
            var operation = TransactionManager.Operation.None;
            try {
                info = this.conn.GetVSEMSystemInfo(txnMng);
                ODLVSEMETW.EventWriteReturnLibrary("System information is retrieved.", string.Empty);

                if (info == null) {
                    ODLVSEMETW.EventWriteProcessCmdletWarning(this.CmdletName,
                        "System information not found.");
                    this.WriteWarning("System information not found.");
                } else {
                    this.WriteObject(info);
                }
            } catch (Exception ex) {
                Exception exception = VSEMODLExceptionUtil.ConvertExceptionToVSEMException(ex);
                ODLVSEMETW.EventWriteFailedCmdlet(this.CmdletName, exception.GetType() + " : " + ex.Message);
                operation = TransactionManager.Operation.Rollback;
                throw exception;
            } finally {
                txnMng.EndTransaction(operation);
            string output = "\"NetworkServiceSystemInformation\":" + JavaScriptSerializer.Serialize(info);
            ODLVSEMETW.EventWriteEndCmdlet(this.CmdletName, output);
            }
        }
    }
}
