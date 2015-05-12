//     Copyright (c) 2013-2014 NEC Corporation
//     All rights reserved.
//     This program and the accompanying materials  are   made
//     available under the terms of the Eclipse Public License
//     v1.0  which  accompanies  this  distribution,  and   is
//     available at  http://www.eclipse.org/legal/epl-v10.html

using System;
using System.Globalization;
using System.IO;
using System.Reflection;
using System.Runtime.Serialization;
using System.Threading;
using ODL.VSEMProvider.VSEMEvents;

namespace ODL.VSEMProvider.Libraries.Common {
    /// <summary>
    /// ConfigManagerBase class.
    /// This class is the base class of ConfigManager that corresponds to each config file one-to-one.
    /// This class has the functions which maintain Entity info of the config file temporarily and operate it.
    /// </summary>
    [Serializable]
    [DataContract]
    public abstract class ConfigManagerBase : IDisposable {
        /// <summary>
        /// Retry interval(msec) of file access.
        /// </summary>
        private const int RETRY_INTERVAL = 500;

        /// <summary>
        /// Retry count of file access.
        /// </summary>
        private const int RETRY_COUNT = 70;

        /// <summary>
        /// Error code when the config file is locked.
        /// </summary>
        private const uint FILE_LOCK_FAILED_CODE = 0x80070020;

        /// <summary>
        /// FileStream info.
        /// </summary>
        [NonSerializedAttribute]
        private FileStream FileStreamInfo = null;

        /// <summary>
        /// Config file path.
        /// </summary>
        private string Path = null;

        /// <summary>
        /// Property of config file path.
        /// </summary>
        public string ConfigFilePath {
            get {
                return this.Path;
            }
        }

        /// <summary>
        /// Initialize of ConfigManager.
        /// </summary>
        /// <param name="mode">Open mode of config file.</param>
        public void Initialize(TransactionManager.OpenMode mode) {
            ConfigManagerBase config = null;

            if (mode == TransactionManager.OpenMode.WriteMode) {
                FileInfo info = new FileInfo(this.Path);
                if (info.Directory.Exists == false) {
                    info.Directory.Create();
                }
            }

            // Open the config file.
            // (Retry max RETRY_COUNT times at RETRY_INTERVAL msec intervals)
            for (var cnt = 1; cnt <= RETRY_COUNT; cnt++) {
                try {
                    if (mode == TransactionManager.OpenMode.WriteMode) {
                        // Open the config file by WriteMode.
                        this.FileStreamInfo = new FileStream(this.Path,
                            FileMode.OpenOrCreate,
                            FileAccess.ReadWrite,
                            FileShare.None);
                    } else if (mode == TransactionManager.OpenMode.ReadMode) {
                        // Open the config file by ReadMode.
                        this.FileStreamInfo = new FileStream(this.Path,
                            FileMode.Open,
                            FileAccess.Read,
                            FileShare.Read);
                    }

                    ODLVSEMETW.EventWriteConfigManagerFileOpen(this.Path, (uint)mode);
                    break;
                } catch (Exception e) {
                    // If the config file is locked, retry the open.
                    if (e.HResult != unchecked((int)FILE_LOCK_FAILED_CODE) || cnt == RETRY_COUNT) {
                        ODLVSEMETW.EventWriteConfigManagerFileIOError(MethodBase.GetCurrentMethod().Name, e.Message);
                        throw;
                    }
                    Thread.Sleep(RETRY_INTERVAL);
                }
            }

            try {
                // If the config file is not empty, get the config info.
                if (this.FileStreamInfo.Length != 0) {
                    var dcs = new DataContractSerializer(this.GetType());
                    config = (ConfigManagerBase)dcs.ReadObject(this.FileStreamInfo);
                }

                this.SetEntity(config);
            } catch (Exception e) {
                // The config file closed because failed to get the config info.
                ODLVSEMETW.EventWriteConfigManagerFileIOError(MethodBase.GetCurrentMethod().Name, e.Message);
                this.FileStreamInfo.Close();
                throw;
            }
        }

        /// <summary>
        /// Commit of the config file operation.
        /// </summary>
        public void Commit() {
            // Write in the case of WriteMode open.
            if (this.FileStreamInfo.CanWrite) {
                var dcs = new DataContractSerializer(this.GetType());

                // Empty the config file.
                // SetLength is called just before WriteObject to suppress the risk
                // between SetLength and WriteObject as little as possible.
                // Risk means that the write fails after deleting the info in the file.
                this.FileStreamInfo.SetLength(0);

                // Write to the config file.
                dcs.WriteObject(this.FileStreamInfo, this);
            }

            // Close the config file.
            this.FileStreamInfo.Close();
        }

        /// <summary>
        /// Rollback of the config file operation.
        /// </summary>
        public void Rollback() {
            // Close the config file.
            this.FileStreamInfo.Close();

            FileStream FileStreamInfoTemp = new FileStream(this.Path,
                FileMode.Open,
                FileAccess.Read,
                FileShare.Read);

            if (FileStreamInfoTemp.Length == 0) {
                FileStreamInfoTemp.Close();
                File.Delete(this.Path);
            } else {
                FileStreamInfoTemp.Close();
            }

            string folder = this.Path;
            int slash = folder.LastIndexOf("\\", StringComparison.Ordinal);
            folder = folder.Substring(0, slash);
            if (folder.EndsWith("\\", StringComparison.Ordinal)) {
                folder = folder.Substring(0, folder.Length - 1);
            }
            if (Directory.Exists(folder) && Directory.GetFiles(folder).Length == 0) {
                Directory.Delete(folder);
            }
        }

        /// <summary>
        /// Delete the config file.
        /// </summary>
        public void Del() {
            // Delete the config file.
            // (Retry max RETRY_COUNT times at RETRY_INTERVAL msec intervals)
            for (var cnt = 1; cnt <= RETRY_COUNT; cnt++) {
                try {
                    File.Delete(this.Path);

                    ODLVSEMETW.EventWriteConfigManagerFileDelete(this.Path);
                    return;
                } catch (Exception e) {
                    // If the file is locked, retry the delete.
                    if (e.HResult != unchecked((int)FILE_LOCK_FAILED_CODE) || cnt == RETRY_COUNT) {
                        ODLVSEMETW.EventWriteConfigManagerFileIOError(MethodBase.GetCurrentMethod().Name, e.Message);
                        throw;
                    }
                    Thread.Sleep(RETRY_INTERVAL);
                }
            }
        }

        /// <summary>
        /// Performs application-defined tasks associated with freeing, releasing, or resetting unmanaged resources.
        /// </summary>
        public void Dispose() {
            this.Dispose(true);
            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Performs application-defined tasks associated with freeing, releasing, or resetting unmanaged resources.
        /// </summary>
        /// <param name="disposing">Indicates whether to dispose or not.</param>
        protected virtual void Dispose(bool disposing) {
            if (disposing) {
                // dispose managed resources
                this.FileStreamInfo.Close();
            }
        }

        /// <summary>
        /// Set the config file path.
        /// </summary>
        /// <param name="name">Config file name.</param>
        protected void SetPath(string name) {
            this.Path = string.Format(
                CultureInfo.CurrentCulture,
                @"{0}\{1}\{2}\{3}",
                Environment.GetFolderPath(Environment.SpecialFolder.CommonApplicationData),
                VSEMODLConstants.MANUFACTURE_NAME,
                VSEMODLConstants.CONFIG_FOLDER_NAME,
                name);
        }

        /// <summary>
        /// Store the config info to field member(Entity) of inheritance class.
        /// </summary>
        /// <param name="config">ConfigManager object.</param>
        protected abstract void SetEntity(ConfigManagerBase config);
    }
}
