//     Copyright (c) 2013-2014 NEC Corporation
//     All rights reserved.
//     This program and the accompanying materials  are   made
//     available under the terms of the Eclipse Public License
//     v1.0  which  accompanies  this  distribution,  and   is
//     available at  http://www.eclipse.org/legal/epl-v10.html


using System;
using Microsoft.SystemCenter.NetworkService;

namespace ODL.VSEMProvider.Cmdlets.Common {
    /// <summary>
    /// VSEMODLExceptionUtil class.
    /// </summary>
    public static class VSEMODLExceptionUtil {
        /// <summary>
        /// Convert Exception to VSEMException.
        /// </summary>
        /// <param name="exception">Exception.</param>
        /// <returns>VSEMException.</returns>
        public static NSPluginException ConvertExceptionToVSEMException(Exception exception) {
            NSPluginException nspluginException = null;

            if (exception == null) {
                nspluginException = new NSPluginArgumentException("Exception to be converted is not specified.");
                return nspluginException;
            }

            // Convert Exception to VSEMException.
            switch (exception.GetType().ToString()) {
            case "System.ArgumentException":
            case "System.ArgumentNullException":
            nspluginException = new NSPluginArgumentException(exception.Message, exception);
                break;
            case "System.TimeoutException":
                nspluginException = new NSPluginClientTimeoutException(exception.Message, exception);
                break;
            case "System.IO.DirectoryNotFoundException":
            case "System.IO.FileNotFoundException":
            case "System.IO.IOException":
                nspluginException = new NSPluginResourceUnavailableException(exception.Message, exception);
                break;
            case "System.InvalidOperationException":
                nspluginException = new NSPluginInvalidOperationException(exception.Message, exception);
                break;
            case "System.Management.Automation.ItemNotFoundException":
                nspluginException = new NSPluginResourceUnavailableException(exception.Message, exception);
                break;
            case "System.Net.WebException":
                if (exception.Message.Contains("Unauthorized")) {
                    nspluginException = new NSPluginAccessDeniedException(exception.Message, exception);
                } else {
                    nspluginException = new NSPluginConnectionFailedException(exception.Message, exception);
                }
                break;
            case "System.UnauthorizedAccessException":
                nspluginException = new NSPluginAccessDeniedException(exception.Message, exception);
                break;
            case "System.DataMisalignedException":
                nspluginException = new NSPluginSynchronizationException(exception.Message, exception);
                break;
            case "System.InvalidCastException":
            default:
                if (exception.Message.Contains("Object reference not set to an instance of an object.")) {
                    nspluginException = new NSPluginException(
                        "\nConfig file(s) may have been tampered externally.\n" + exception.StackTrace);
                } else {
                    nspluginException = new NSPluginException(exception.Message, exception);
                }
                break;
            }

            return nspluginException;
        }
    }
}
