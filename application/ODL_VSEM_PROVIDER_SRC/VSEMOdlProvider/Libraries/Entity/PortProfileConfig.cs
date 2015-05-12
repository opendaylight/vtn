//     Copyright (c) 2015 NEC Corporation
//     All rights reserved.
//     This program and the accompanying materials  are   made
//     available under the terms of the Eclipse Public License
//     v1.0  which  accompanies  this  distribution,  and   is
//     available at  http://www.eclipse.org/legal/epl-v10.html


using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Serialization;
using Microsoft.SystemCenter.NetworkService.VSEM;
using ODL.VSEMProvider.Libraries.Common;

namespace ODL.VSEMProvider.Libraries.Entity {
    /// <summary>
    /// PortProfileConfig class.
    /// This class has the function to manage the PortProfile.config file.
    /// </summary>
    public class PortProfileConfig : ConfigManagerBase {
        /// <summary>
        /// Config file name.
        /// </summary>
        private const string CONFIG_FILE_NAME = "PortProfile.config";

        /// <summary>
        /// Entity(List of VSEMUplinkPortProfile).
        /// </summary>
        [DataMember]
        public List<VSEMUplinkPortProfile> UplinkPortProfiles {
            get;
            set;
        }

        /// <summary>
        /// Entity(List of VSEMVirtualPortProfile).
        /// </summary>
        [DataMember]
        public List<VSEMVirtualPortProfile> VirtualPortProfiles {
            get;
            set;
        }

        /// <summary>
        /// Constructor.
        /// </summary>
        public PortProfileConfig() {
            // Initialize.
            this.SetPath(PortProfileConfig.CONFIG_FILE_NAME);
        }

        /// <summary>
        /// Get the uplink port profile with the specified ID.
        /// </summary>
        /// <param name="id">ID to find.</param>
        /// <returns>Uplink port profile if found, else null.</returns>
        public VSEMUplinkPortProfile GetUplinkPortProfileById(Guid id) {
            if (this.UplinkPortProfiles != null) {
                var profile = this.UplinkPortProfiles.FirstOrDefault(prof => prof.Id == id);
                return profile;
            } else {
                return null;
            }
        }

        /// <summary>
        /// Store the config info to field member(Entity).
        /// </summary>
        /// <param name="config">ConfigManager object.</param>
        protected override void SetEntity(ConfigManagerBase config) {
            if (config == null) {
                this.UplinkPortProfiles = new List<VSEMUplinkPortProfile>();
                this.VirtualPortProfiles = new List<VSEMVirtualPortProfile>();
            } else {
                this.UplinkPortProfiles = ((PortProfileConfig)config).UplinkPortProfiles;
                this.VirtualPortProfiles = ((PortProfileConfig)config).VirtualPortProfiles;
            }
        }
    }
}
