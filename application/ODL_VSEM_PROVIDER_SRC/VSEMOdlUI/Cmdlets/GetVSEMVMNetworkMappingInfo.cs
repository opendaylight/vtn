//     Copyright (c) 2015 NEC Corporation
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
using System.Xml.Serialization;
using ODL.VSEMProvider.Cmdlets.Common;
using ODL.VSEMProvider.Libraries;
using ODL.VSEMProvider.Libraries.Common;
using ODL.VSEMProvider.Libraries.Entity;
using ODL.VSEMProvider.VSEMEvents;

namespace ODL.VSEM.Cmdlets {
    /// <summary>
    /// This class represents a cmdlet.
    /// This cmdlet is used to retrieve the mapping information about the VMNetworks.
    /// </summary>
    [Cmdlet("Get", "Odl.VSEMVMNetworkMappingInfo")]
    [OutputType(typeof(string))]
    [ManagedName("Microsoft.SystemCenter.NetworkService.GetVSEMVMNetworkMappingInfo")]
    public sealed class GetVSEMVMNetworkMappingInfo : VSEMODLCmdletBase {
        /// <summary>
        /// Host name of the VTNCoordinator.
        /// </summary>
        private string VtnHostName;

        /// <summary>
        /// Host name of the VTNCoordinator.
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
        /// This method is responsible to add specified vlan entry into the local variable.
        /// </summary>
        protected override void DoODLVSEMCmdlet() {
            var JavaScriptSerializer = new JavaScriptSerializer();
            JavaScriptSerializer.MaxJsonLength = int.MaxValue;
            StringBuilder json = new StringBuilder("\"VtnHostName\":" + JavaScriptSerializer.Serialize(this.VtnHostName));
            ODLVSEMETW.EventWriteDoCmdlet(this.CmdletName,
                "Retrieving VM network maping info.",
                json.ToString());
            TransactionManager txnMng = new TransactionManager();
            txnMng.StartTransaction();
            var operation = TransactionManager.Operation.None;
            VMNetworkMappingInfo vMNetworkMappingInfo = null;
            try {
                vMNetworkMappingInfo = VSEMConfiguration.GetVSEMVMNetworkMappingInfo(txnMng, this.VtnHostName);
                ODLVSEMETW.EventWriteReturnLibrary("VM network mapping information is retrieved.", string.Empty);
                if (vMNetworkMappingInfo == null) {
                    ODLVSEMETW.EventWriteProcessCmdletWarning(this.CmdletName,
                    "VM network mapping information not found.");
                    this.WriteWarning("VM network mapping information not found.");
                } else {
                    this.WriteObject(this.ToXML(vMNetworkMappingInfo), true);
                }
            } catch (Exception ex) {
                Exception exception = VSEMODLExceptionUtil.ConvertExceptionToVSEMException(ex);
                ODLVSEMETW.EventWriteFailedCmdlet(this.CmdletName, exception.GetType() + " : " + ex.Message);
                operation = TransactionManager.Operation.Rollback;
                throw exception;
            } finally {
                txnMng.EndTransaction(operation);
                string output = "\"vMNetworkMappingInfo\":" + JavaScriptSerializer.Serialize(vMNetworkMappingInfo);
                ODLVSEMETW.EventWriteEndCmdlet(this.CmdletName, output);
            }
        }

        /// <summary>
        /// Method to convert the objevt into XML string.
        /// </summary>
        /// <param name="obj">Object to parse.</param>
        /// <returns>Converted string.</returns>
        private string ToXML(VMNetworkMappingInfo obj) {
            var stringwriter = new System.IO.StringWriter();
            var serializer = new XmlSerializer(typeof(VMNetworkMappingInfo));
            serializer.Serialize(stringwriter, obj);
            return stringwriter.ToString();
        }
    }
}
