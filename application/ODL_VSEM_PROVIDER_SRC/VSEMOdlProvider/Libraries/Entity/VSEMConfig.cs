//     Copyright (c) 2015 NEC Corporation
//     All rights reserved.
//     This program and the accompanying materials  are   made
//     available under the terms of the Eclipse Public License
//     v1.0  which  accompanies  this  distribution,  and   is
//     available at  http://www.eclipse.org/legal/epl-v10.html

using System.Collections.Generic;
using System.Runtime.Serialization;
using Microsoft.SystemCenter.NetworkService;
using Microsoft.SystemCenter.NetworkService.VSEM;
using ODL.VSEMProvider.Libraries.Common;

namespace ODL.VSEMProvider.Libraries.Entity {
    /// <summary>
    /// VSEMConfig class.
    /// This class has the function to manage the VSEM.config file.
    /// </summary>
    public class VSEMConfig : ConfigManagerBase {
        /// <summary>
        /// Config file name.
        /// </summary>
        private const string CONFIG_FILE_NAME = "VSEM.config";

        /// <summary>
        /// Entity(VSEMInfo).
        /// </summary>
        [DataMember]
        public VSEMInfo Info {
            get;
            set;
        }

        /// <summary>
        /// Entity(VSEMSystemInfo).
        /// </summary>
        [DataMember]
        public NetworkServiceSystemInformation NetworkServiceSystemInformation {
            get;
            set;
        }

        /// <summary>
        /// Entity(VSEMController).
        /// </summary>
        [DataMember]
        public VSEMController Controller {
            get;
            set;
        }

        /// <summary>
        /// Entity(List of VSEMSwitchExtensionInfo).
        /// </summary>
        [DataMember]
        public List<VSEMSwitchExtensionInfo> SwitchExtensionInfo {
            get;
            set;
        }

        /// <summary>
        /// Constructor.
        /// </summary>
        public VSEMConfig() {
            // Initialize.
            this.SetPath(VSEMConfig.CONFIG_FILE_NAME);
        }

        /// <summary>
        /// Store the config info to field member(Entity).
        /// </summary>
        /// <param name="config">ConfigManager object.</param>
        protected override void SetEntity(ConfigManagerBase config) {
            if (config == null) {
                this.Info = new VSEMInfo();
                this.NetworkServiceSystemInformation = new NetworkServiceSystemInformation();
                this.Controller = new VSEMController();
                this.SwitchExtensionInfo = new List<VSEMSwitchExtensionInfo>();
            } else {
                this.Info = ((VSEMConfig)config).Info;
                this.NetworkServiceSystemInformation = ((VSEMConfig)config).NetworkServiceSystemInformation;
                this.Controller = ((VSEMConfig)config).Controller;
                this.SwitchExtensionInfo = ((VSEMConfig)config).SwitchExtensionInfo;
            }
        }
    }
}
