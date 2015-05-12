//     Copyright (c) 2015 NEC Corporation
//     All rights reserved.
//     This program and the accompanying materials  are   made
//     available under the terms of the Eclipse Public License
//     v1.0  which  accompanies  this  distribution,  and   is
//     available at  http://www.eclipse.org/legal/epl-v10.html


using System;
using System.Globalization;
using System.Reflection;
using System.Text;
using System.Web.Script.Serialization;
using ODL.VSEMProvider.Libraries.Common;
using ODL.VSEMProvider.Libraries.Entity;
using ODL.VSEMProvider.VSEMEvents;

namespace ODL.VSEMProvider.Libraries {
    /// <summary>
    /// This class is responsible for the management of VSEM configurations.
    /// </summary>
    public static class VSEMConfiguration {
        /// <summary>
        /// Error message for the file IO exception.
        /// </summary>
        private static string configFileIOErrorValidationMessage =
            string.Format(CultureInfo.CurrentCulture,
            "{0}\n{1}\n1. {2}\n2. {3}\n3. {4}\n",
            "file could not be accessed.",
            "Possible reasons could be:",
            "File is deleted.",
            "File is renamed.",
            "File is tampered.");

        /// <summary>
        /// This method is responsible for extracting the list of VSEMVmNetwork
        /// from the VSEM repository for the corresponding connection.
        /// </summary>
        /// <param name="txnMng">Transaction manager.</param>
        /// <param name="VtnHostName">Host name of the VTNCoordinator.</param>
        /// <returns>List of VSEMVmNetwork instances attached with the connection.</returns>
        public static VMNetworkMappingInfo GetVSEMVMNetworkMappingInfo(TransactionManager txnMng, string VtnHostName) {
            var JavaScriptSerializer = new JavaScriptSerializer();
            JavaScriptSerializer.MaxJsonLength = int.MaxValue;
            StringBuilder json = new StringBuilder("\"TransactionManager\":" + JavaScriptSerializer.Serialize(txnMng));
            ODLVSEMETW.EventWriteStartLibrary(
                    MethodBase.GetCurrentMethod().Name,
                    json.ToString());
            ODLVSEMETW.EventWriteExtractVMNetworkInfolist(
                "Extracting list of VMNetwork Info.",
                string.Empty);
            if (string.IsNullOrWhiteSpace(VtnHostName)) {
                VSEMConfig vsemConfig = new VSEMConfig();
                try {
                    txnMng.SetConfigManager(vsemConfig, TransactionManager.OpenMode.ReadMode);
                } catch (Exception ex) {
                    ODLVSEMETW.EventWriteConfigManagerFileIOError(
                        MethodBase.GetCurrentMethod().Name,
                        string.Format(CultureInfo.CurrentCulture,
                        "VSEM.config {0}\n",
                        configFileIOErrorValidationMessage) +
                        ex.Message);
                    ODLVSEMETW.EventWriteEndLibrary(MethodBase.GetCurrentMethod().Name, string.Empty);
                    throw new InvalidOperationException(
                        string.Format(CultureInfo.CurrentCulture,
                        "Either the NetworkService is not added in SCVMM or VSEM.config {0}",
                        configFileIOErrorValidationMessage));
                }
                VtnHostName = vsemConfig.Info.ServerName;
            }
            if (string.IsNullOrWhiteSpace(VtnHostName)) {
                throw new ArgumentException(
                        "Parameter 'VTNCoordinatorHostName' is null or invalid.");
            }
            var vMNetworkConfig = new VMNetworkConfig(VtnHostName);
            try {
                txnMng.SetConfigManager(vMNetworkConfig, TransactionManager.OpenMode.ReadMode);
                var ret = vMNetworkConfig.VMNetwork.VMNetworkMappingInformation;
                string output = "\"VMNetwork\":" + JavaScriptSerializer.Serialize(ret);
                ODLVSEMETW.EventWriteEndLibrary(MethodBase.GetCurrentMethod().Name,
                    output);
                return ret;
            } catch (System.IO.FileNotFoundException) {
                // Ignore if the file is not yet created and return empty list.
                return new VMNetworkMappingInfo();
            } catch (Exception ex) {
                ODLVSEMETW.EventWriteConfigManagerFileIOError(
                    MethodBase.GetCurrentMethod().Name,
                    string.Format(CultureInfo.CurrentCulture,
                    "VMNetwork.config {0}\n{1}",
                    configFileIOErrorValidationMessage,
                    ex.Message));
                throw new InvalidOperationException(
                    string.Format(CultureInfo.CurrentCulture,
                    "VMNetwork.config {0}",
                    configFileIOErrorValidationMessage));
            }
        }
    }
}
