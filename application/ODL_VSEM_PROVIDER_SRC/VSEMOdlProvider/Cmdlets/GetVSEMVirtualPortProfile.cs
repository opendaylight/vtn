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
    /// This class represents a cmdlet.
    /// This cmdlet is used to retrieves the information about the
    /// virtual port profile(s) that are managed by this Extension Manager.
    /// </summary>
    [Cmdlet(VerbsCommon.Get, "Odl.VSEMVirtualPortProfile")]
    [OutputType(typeof(List<VSEMVirtualPortProfile>))]
    [ManagedName("Microsoft.SystemCenter.NetworkService.GetVSEMVirtualPortProfile")]
    public sealed class GetVSEMVirtualPortProfile : VSEMODLCmdletBase {
        /// <summary>
        /// Id of virtual port profile that is managed by this Extension Manager.
        /// </summary>
        private Guid iD;

        /// <summary>
        /// Id of virtual port profile that is managed by this Extension Manager.
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
        /// This function is responsible to retrieve the information about the virtual port
        /// profile(s) for the connection.
        /// </summary>
        protected override void DoODLVSEMCmdlet() {
            var JavaScriptSerializer = new JavaScriptSerializer();
            JavaScriptSerializer.MaxJsonLength = int.MaxValue;
            StringBuilder json = new StringBuilder(" \"ID\":" + JavaScriptSerializer.Serialize(this.ID));
            ODLVSEMETW.EventWriteDoCmdlet(this.CmdletName,
                "Retrieving virtual port profile(s).",
                json.ToString());

            List<VSEMVirtualPortProfile> profiles = null;
            TransactionManager txnMng = new TransactionManager();
            txnMng.StartTransaction();
            var operation = TransactionManager.Operation.None;
            try {
                if (this.ID != Guid.Empty) {
                    profiles = new List<VSEMVirtualPortProfile>();
                    profiles.Add(this.conn.GetVSEMVirtualPortProfileById(txnMng, this.ID));
                    ODLVSEMETW.EventWriteReturnLibrary(string.Format(CultureInfo.CurrentCulture,
                        "Virtual port profile is retrieved by ID: {0}",
                        this.ID.ToString("B")),
                        string.Empty);

                    if (profiles[0] == null) {
                        ODLVSEMETW.EventWriteProcessCmdletWarning(this.CmdletName,
                        string.Format(CultureInfo.CurrentCulture,
                        "ID {0} not found.",
                        this.ID.ToString("B")));
                        this.WriteWarning(string.Format(CultureInfo.CurrentCulture,
                            "ID {0} not found.",
                            this.ID.ToString("B")));
                    } else {
                        this.WriteObject(profiles);
                    }
                } else {
                    profiles = this.conn.GetVSEMVirtualPortProfile(txnMng);
                    ODLVSEMETW.EventWriteReturnLibrary(
                        "Virtual port profile(s) retrieved.",
                        string.Empty);
                    if (profiles == null) {
                        ODLVSEMETW.EventWriteProcessCmdletWarning(this.CmdletName,
                        "Virtual profile(s) not found.");
                        this.WriteWarning("Virtual profile(s) not found.");
                    } else {
                        this.WriteObject(profiles, true);
                    }
                }
            } catch (Exception ex) {
                Exception exception = VSEMODLExceptionUtil.ConvertExceptionToVSEMException(ex);
                ODLVSEMETW.EventWriteFailedCmdlet(this.CmdletName, exception.GetType() + " : " + ex.Message);
                operation = TransactionManager.Operation.Rollback;
                throw exception;
            } finally {
                txnMng.EndTransaction(operation);
                string output = "\"VSEMVirtualPortProfile\":" + JavaScriptSerializer.Serialize(profiles);
                ODLVSEMETW.EventWriteEndCmdlet(this.CmdletName, output);
            }
        }
    }
}
