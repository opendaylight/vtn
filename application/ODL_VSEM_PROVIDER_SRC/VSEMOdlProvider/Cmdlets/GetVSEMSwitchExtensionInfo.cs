//     Copyright (c) 2015 NEC Corporation
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
using Microsoft.SystemCenter.NetworkService.VSEM;
using ODL.VSEMProvider.Cmdlets.Common;
using ODL.VSEMProvider.Libraries;
using ODL.VSEMProvider.Libraries.Common;
using ODL.VSEMProvider.VSEMEvents;
using VSEM.Cmdlets.Common;

namespace ODL.VSEMProvider.Cmdlets {
    /// <summary>
    /// This class represents a cmdlet. This cmdlet is used to retrieves the information
    /// about the switch extension(s) that are managed by the virtual switch extension manager.
    /// </summary>
    [Cmdlet(VerbsCommon.Get, "Odl.VSEMSwitchExtensionInfo")]
    [OutputType(typeof(List<VSEMSwitchExtensionInfo>))]
    [ManagedName("Microsoft.SystemCenter.NetworkService.GetVSEMSwitchExtensionInfo")]
    public sealed class GetVSEMSwitchExtensionInfo : VSEMODLCmdletBase {
        /// <summary>
        /// Id of Switch Extension that is managed by this Extension Manager.
        /// </summary>
        private Guid iD;

        /// <summary>
        /// Id of Switch Extension that is managed by this Extension Manager.
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
        /// This function is responsible to retrieve the SwitchExtensionInfo for the connection.
        /// </summary>
        protected override void DoODLVSEMCmdlet() {
            var JavaScriptSerializer = new JavaScriptSerializer();
            JavaScriptSerializer.MaxJsonLength = int.MaxValue;
            StringBuilder json = new StringBuilder(" \"ID\":" + JavaScriptSerializer.Serialize(this.ID));
            ODLVSEMETW.EventWriteDoCmdlet(this.CmdletName,
                "Retrieving switch extension information.",
                json.ToString());

            List<VSEMSwitchExtensionInfo> infos = null;
            TransactionManager txnMng = new TransactionManager();
            txnMng.StartTransaction();
            var operation = TransactionManager.Operation.None;
            try {
                if (this.ID != Guid.Empty) {
                    infos = new List<VSEMSwitchExtensionInfo>();
                    infos.Add(this.conn.GetVSEMSwitchExtensionInfoById(txnMng, this.ID));
                    ODLVSEMETW.EventWriteReturnLibrary(string.Format(CultureInfo.CurrentCulture,
                        "Switch extension info is retrieved by ID: {0}",
                        this.ID.ToString("B")),
                        string.Empty);

                    if (infos[0] == null) {
                        ODLVSEMETW.EventWriteProcessCmdletWarning(this.CmdletName,
                        string.Format(CultureInfo.CurrentCulture,
                        "ID {0} not found.",
                        this.ID.ToString("B")));
                        this.WriteWarning(string.Format(CultureInfo.CurrentCulture,
                            "ID {0} not found.",
                            this.ID.ToString("B")));
                    } else {
                        this.WriteObject(infos, true);
                    }
                } else {
                    infos = this.conn.GetVSEMSwitchExtensionInfo(txnMng);
                    ODLVSEMETW.EventWriteReturnLibrary(
                        "Switch extension info is retrieved.",
                        string.Empty);
                    if (infos == null) {
                        ODLVSEMETW.EventWriteProcessCmdletWarning(this.CmdletName,
                        "Switch extension info not found.");
                        this.WriteWarning("Switch extension info not found.");
                    } else {
                        this.WriteObject(infos, true);
                    }
                }
            } catch (Exception ex) {
                Exception exception = VSEMODLExceptionUtil.ConvertExceptionToVSEMException(ex);
                ODLVSEMETW.EventWriteFailedCmdlet(this.CmdletName, exception.GetType() + " : " + ex.Message);
                operation = TransactionManager.Operation.Rollback;
                throw exception;
            } finally {
                txnMng.EndTransaction(operation);
                string output = "\"VSEMSwitchExtensionInfo\":" + JavaScriptSerializer.Serialize(infos);
                ODLVSEMETW.EventWriteEndCmdlet(this.CmdletName, output);
            }
        }
    }
}
