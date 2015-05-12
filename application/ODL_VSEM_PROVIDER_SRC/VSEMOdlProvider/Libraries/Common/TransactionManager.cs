//     Copyright (c) 2013-2014 NEC Corporation
//     All rights reserved.
//     This program and the accompanying materials  are   made
//     available under the terms of the Eclipse Public License
//     v1.0  which  accompanies  this  distribution,  and   is
//     available at  http://www.eclipse.org/legal/epl-v10.html


using System;
using System.Collections.Generic;
using System.Reflection;
using System.Text;
using ODL.VSEMProvider.VSEMEvents;

namespace ODL.VSEMProvider.Libraries.Common {
    /// <summary>
    /// TransactionManager class.
    /// This class has the functions which manage the transaction of ConfigManager.
    /// </summary>
    public class TransactionManager {
        /// <summary>
        /// Operation type of transaction.
        /// </summary>
        public enum Operation {
            /// <summary>
            /// Commit operation.
            /// Specify "Commit" in the case of being opened in at least one WriteMode.
            /// </summary>
            Commit,

            /// <summary>
            /// Rollback operation.
            /// Specify "Rollback" in the case of being opened in at least one WriteMode.
            /// </summary>
            Rollback,

            /// <summary>
            /// Default operation.
            /// Specify "None" in the case of being opened in only ReadMode.
            /// </summary>
            None
        }

        /// <summary>
        /// Open mode of config file.
        /// </summary>
        public enum OpenMode {
            /// <summary>
            /// Write mode.
            /// Open the config file by Read / Write lock.
            /// </summary>
            WriteMode,

            /// <summary>
            /// Read mode.
            /// Open the config file by Write lock.
            /// </summary>
            ReadMode
        }

        /// <summary>
        /// ConfigManager list.
        /// </summary>
        private List<ConfigManagerBase> Managers;

        /// <summary>
        /// Start the transaction.
        /// </summary>
        public void StartTransaction() {
            ODLVSEMETW.EventWriteStartLibrary(MethodBase.GetCurrentMethod().Name, 
                string.Empty);

            // Initialize Managers.
            this.Managers = new List<ConfigManagerBase>();

            ODLVSEMETW.EventWriteEndLibrary(MethodBase.GetCurrentMethod().Name, string.Empty);
        }

        /// <summary>
        /// End the transaction.
        /// </summary>
        /// <param name="operation">Operation type of transaction.</param>
        public void EndTransaction(Operation operation) {
            StringBuilder json = new StringBuilder("\"operation\":\"" + operation + "\"");
            ODLVSEMETW.EventWriteStartLibrary(MethodBase.GetCurrentMethod().Name,
                json.ToString());

            try {
                // Parameter check.
                if (!Operation.IsDefined(typeof(Operation), operation)) {
                    ODLVSEMETW.EventWriteConfigManagerDiagError(MethodBase.GetCurrentMethod().Name,
                        "The parameter 'operation' is null or invalid.");
                    throw new ArgumentException(
                        "The parameter 'operation' is null or invalid.");
                }

                this.Managers.Reverse();

                // Run the operation against all of ConfigManagers.
                foreach (ConfigManagerBase config in this.Managers) {
                    switch (operation) {
                    case Operation.Commit:
                        config.Commit();
                        break;
                    default:
                        config.Rollback();
                        break;
                    }

                    ODLVSEMETW.EventWriteConfigManagerFileClose(config.ConfigFilePath, (uint)operation);
                }
            } finally {
                // Finalize Managers.
                this.Managers = null;
            }

            ODLVSEMETW.EventWriteEndLibrary(MethodBase.GetCurrentMethod().Name, string.Empty);
        }

        /// <summary>
        /// Set the ConfigManager to Managers.
        /// </summary>
        /// <param name="config">ConfigManager object.</param>
        /// <param name="mode">Open mode of config file.</param>
        public void SetConfigManager(ConfigManagerBase config, OpenMode mode) {
            // Parameter check.
            if (config == null) {
                ODLVSEMETW.EventWriteConfigManagerDiagError(MethodBase.GetCurrentMethod().Name,
                    "The parameter 'config' is null or invalid.");
                throw new ArgumentException(
                    "The parameter 'config' is null or invalid.");
            }
            if (!OpenMode.IsDefined(typeof(OpenMode), mode)) {
                ODLVSEMETW.EventWriteConfigManagerDiagError(MethodBase.GetCurrentMethod().Name,
                    "The parameter 'mode' is null or invalid.");
                throw new ArgumentException(
                    "The parameter 'mode' is null or invalid.");
            }

            // Initialize ConfigManager.
            config.Initialize(mode);

            // Set the Managers.
            this.Managers.Add(config);
        }

        /// <summary>
        /// Delete the config file.
        /// </summary>
        /// <param name="config">ConfigManager object.</param>
        public void DelConfig(ConfigManagerBase config) {
            // Parameter check.
            if (config == null) {
                ODLVSEMETW.EventWriteConfigManagerDiagError(MethodBase.GetCurrentMethod().Name,
                    "The parameter 'config' is null or invalid.");
                throw new ArgumentException(
                    "The parameter 'config' is null or invalid.");
            }

            // Delete the config file.
            config.Del();
        }
    }
}
