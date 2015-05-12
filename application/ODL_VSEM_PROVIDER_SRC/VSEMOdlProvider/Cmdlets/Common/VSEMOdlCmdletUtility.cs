//     Copyright (c) 2015 NEC Corporation
//     All rights reserved.
//     This program and the accompanying materials  are   made
//     available under the terms of the Eclipse Public License
//     v1.0  which  accompanies  this  distribution,  and   is
//     available at  http://www.eclipse.org/legal/epl-v10.html


using Microsoft.SystemCenter.NetworkService;
using ODL.VSEMProvider.Libraries;
using ODL.VSEMProvider.VSEMEvents;

namespace VSEM.Cmdlets.Common {
    /// <summary>
    /// VSEMODLCmdletUtility class.
    /// </summary>
    public static class VSEMODLCmdletUtility {
        /// <summary>
        /// Validates the connection object.
        /// </summary>
        /// <param name="connection">Connection object.</param>
        /// <param name="cmdletName">Name of the calling cmdlet.</param>
        /// <returns>VSEMConnection object.</returns>
        public static VSEMConnection ValidateConnectionObject(IConnection connection, string cmdletName) {
            VSEMConnection conn;
            if (connection == null) {
                ODLVSEMETW.EventWriteValidateCmdletParameter(cmdletName,
                    "Microsoft.SystemCenter.NetworkService.NSPluginArgumentException : Mandatory parameter(s) not provided.");
                throw new NSPluginArgumentException("Mandatory parameter(s) not provided.");
            }
            conn = connection as VSEMConnection;
            if (conn == null) {
                ODLVSEMETW.EventWriteValidateConnectionObjectError(cmdletName,
                    "Microsoft.SystemCenter.NetworkService.NSPluginArgumentException : Connection object is NULL.");
                throw new NSPluginArgumentException("Invalid connection object.");
            }

            if (string.IsNullOrEmpty(conn.ConnectionString)) {
                ODLVSEMETW.EventWriteValidateVSEMRepositoryError(cmdletName,
                    "Microsoft.SystemCenter.NetworkService.NSPluginInvalidOperationException : Connection is already closed.");
                throw new NSPluginInvalidOperationException("Connection is already closed.");
            }
            return conn;
        }
    }
}
