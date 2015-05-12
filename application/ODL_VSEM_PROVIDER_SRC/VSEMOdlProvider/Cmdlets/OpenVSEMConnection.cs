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
using System.Reflection;
using System.Text;
using System.Text.RegularExpressions;
using System.Web.Script.Serialization;
using Microsoft.SystemCenter.NetworkService;
using ODL.VSEMProvider.Cmdlets.Common;
using ODL.VSEMProvider.Libraries;
using ODL.VSEMProvider.Libraries.Common;
using ODL.VSEMProvider.VSEMEvents;

namespace ODL.VSEMProvider.Cmdlets {
    /// <summary>
    /// Open VSEM connection cmdlet.
    /// </summary>
    [Cmdlet(VerbsCommon.Open, "Odl.VSEMConnection")]
    [OutputType(typeof(IConnection))]
    [ManagedName("Microsoft.SystemCenter.NetworkService.OpenDeviceConnection")]
    public sealed class OpenVSEMConnection : VSEMODLCmdletBase {
        /// <summary>
        /// Error message for the connection string validations.
        /// </summary>
        private static string connectionStringValidationMessage =
            string.Format(CultureInfo.CurrentCulture,
            "{0}\n{1}\n1. {2}\n2. {3}\n",
            "Correct format of connection string is: HostName:PortNumber,IP1[:PortNumber][,IP2[:PortNumber]].\n Format of given connection string is invalid.",
            "Possible reasons could be:",
            "DNS name is invalid.",
            "Port number is invalid. It should be a number between 0 and 65535.");

        /// <summary>
        /// Error message for the controller IP string validations.
        /// </summary>
        private static string ipStringValidationMessage =
            string.Format(CultureInfo.CurrentCulture,
            "{0}\n{1}\n1. {2}\n2. {3}\n",
            "Format of one or more controller strings is invalid.",
            "Possible reasons could be:",
            "Format of IP Address is invalid. It must be in format x.x.x.x where value of x should be between 0 and 255.",
            "Port number is invalid. It should be a number between 0 and 65535.");

        /// <summary>
        /// Cmdlet parameters for connection credential.
        /// </summary>
        private ConnectionParams connectionParams;

        /// <summary>
        /// Cmdlet parameters for connection credential.
        /// </summary>
        [Parameter(ValueFromPipeline = true)]
        public ConnectionParams ConnectionParams {
            get {
                return this.connectionParams;
            }

            set {
                this.connectionParams = value;
            }
        }

        /// <summary>
        /// This function is responsible for validating the parameters.
        /// </summary>
        protected override void BeginODLVSEMCmdlet() {
            if (this.ConnectionParams == null) {
                ODLVSEMETW.EventWriteValidateCmdletParameter(this.CmdletName,
                    "NSPluginArgumentException : Mandatory parameter(s) not provided.");
                throw new NSPluginArgumentException("Mandatory parameter(s) not provided.");
            }

            if (string.IsNullOrEmpty(this.ConnectionParams.ConnectionString)) {
                ODLVSEMETW.EventWriteConnectionStringError(this.CmdletName,
                    "NSPluginArgumentException : No ConnectionString specified.");
                throw new NSPluginArgumentException("No ConnectionString specified.");
            }

            if (this.ConnectionParams.Credential == null) {
                ODLVSEMETW.EventWriteCredentialError(this.CmdletName,
                    "NSPluginArgumentException : User Name or Password not specified.");
                throw new NSPluginArgumentException("No credential specified.");
            }

            // Validate the format of the connection string
            if (this.ConnectionParams.ConnectionString.Length > 450) {
                ODLVSEMETW.EventWriteConnectionStringError(this.CmdletName,
                    "NSPluginArgumentException : Maximum length of connectionString is 450 characters.");
                throw new NSPluginArgumentException("ConnectionString is too long." +
                    " Maximum length of connectionString is 450 characters.");
            }
        }

        /// <summary>
        /// This function is responsible for creating a connection object for this VSEM provider.
        /// </summary>
        protected override void DoODLVSEMCmdlet() {
            var JavaScriptSerializer = new JavaScriptSerializer();
            JavaScriptSerializer.MaxJsonLength = int.MaxValue;
            StringBuilder json = new StringBuilder("\"ConnectionString\":" + JavaScriptSerializer.Serialize(this.ConnectionParams.ConnectionString));
            ODLVSEMETW.EventWriteDoCmdlet(this.CmdletName,
                "Connecting to VSEM.",
                json.ToString());

            this.ValidateConnectionString();
            string controllers = this.ValidateControllers();
            string connectionString =
                this.ConnectionParams.ConnectionString.Split(',').FirstOrDefault();

            TransactionManager txnMng = new TransactionManager();
            txnMng.StartTransaction();
            var operation = TransactionManager.Operation.None;
            VSEMConnection conn = new VSEMConnection(connectionString,
                this.ConnectionParams.Credential,
                controllers);
            try {
                conn.UpdateConnection(txnMng);
                ODLVSEMETW.EventWriteReturnLibrary(string.Format(CultureInfo.CurrentCulture,
                    "Connected to {0} as user '{1}'.",
                    this.ConnectionParams.ConnectionString,
                    this.ConnectionParams.Credential.UserName),
                    string.Empty);

                operation = TransactionManager.Operation.Commit;
            } catch (Exception ex) {
                Exception exception = VSEMODLExceptionUtil.ConvertExceptionToVSEMException(ex);
                ODLVSEMETW.EventWriteFailedCmdlet(this.CmdletName, exception.GetType() + " : " + ex.Message);
                operation = TransactionManager.Operation.Rollback;
                throw exception;
            } finally {
                txnMng.EndTransaction(operation);
                ODLVSEMETW.EventWriteEndCmdlet(this.CmdletName, string.Empty);
                this.WriteObject(conn);
            }
        }

        /// <summary>
        /// Validates the connection string.
        /// </summary>
        private void ValidateConnectionString() {
            ODLVSEMETW.EventWriteValidateConnectionString(
                MethodBase.GetCurrentMethod().Name,
                "Validating connection string.");
            if (!Regex.IsMatch(this.ConnectionParams.ConnectionString.Split(',').First(),
                RegularExpressions.CONNECTION_STRING_PATTERN)) {
                ODLVSEMETW.EventWriteConnectionStringFormatError(
                    MethodBase.GetCurrentMethod().Name,
                    connectionStringValidationMessage);
                throw new NSPluginArgumentException(connectionStringValidationMessage);
            }
        }

        /// <summary>
        /// Validates the controllers string.
        /// </summary>
        /// <returns>Controllers.</returns>
        private string ValidateControllers() {
            ODLVSEMETW.EventWriteValidateControllerString(
                MethodBase.GetCurrentMethod().Name,
                "Validating controller in connection string.");
            int controllersIndex = this.ConnectionParams.ConnectionString.IndexOf(',');
            if (controllersIndex == -1) {
                ODLVSEMETW.EventWriteCountControllerIPError(
                    MethodBase.GetCurrentMethod().Name,
                    "It is mandatory to give at least one controller IP.");
                throw new NSPluginArgumentException(
                    "It is mandatory to give at least one controller IP.");
            }
            string controllers =
                this.ConnectionParams.ConnectionString.Substring(controllersIndex + 1);
            List<string> controllersToAdd =
                 controllers.Split(',').ToList();
            string ipaddress = string.Empty;
            for (int cntr = 0; cntr < controllersToAdd.Count; cntr++) {
                ipaddress = Validations.IsIPAddressWithPortValid(controllersToAdd[cntr]);
                if (string.IsNullOrEmpty(controllersToAdd[cntr])
                    || string.IsNullOrEmpty(ipaddress)) {
                    ODLVSEMETW.EventWriteControllerFormatError(
                        MethodBase.GetCurrentMethod().Name,
                        ipStringValidationMessage);
                    throw new NSPluginArgumentException(ipStringValidationMessage);
                } else {
                    if (ipaddress.Contains(':')) {
                        controllersToAdd[cntr] = ipaddress;
                    } else {
                        controllersToAdd[cntr] = ipaddress + ":6633";
                    }
                }
            }

            if (controllersToAdd.Count > 2) {
                ODLVSEMETW.EventWriteCountControllerIPError(
                    MethodBase.GetCurrentMethod().Name,
                    "It is not allowed to add more than two controller IPs.");
                throw new NSPluginArgumentException(
                    "It is not allowed to add more than two controller IPs.");
            }
            controllers = string.Empty;
            controllersToAdd.ForEach(cnt => controllers = controllers + cnt + ",");
            controllers = controllers.Remove(controllers.Length - 1);
            return controllers;
        }
    }
}
