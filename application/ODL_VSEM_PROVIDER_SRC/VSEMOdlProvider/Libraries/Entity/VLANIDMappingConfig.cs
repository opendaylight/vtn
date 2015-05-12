//     Copyright (c) 2015 NEC Corporation
//     All rights reserved.
//     This program and the accompanying materials  are   made
//     available under the terms of the Eclipse Public License
//     v1.0  which  accompanies  this  distribution,  and   is
//     available at  http://www.eclipse.org/legal/epl-v10.html

using System;
using System.Collections.Generic;
using System.Globalization;
using System.Runtime.Serialization;
using ODL.VSEMProvider.Libraries.Common;

namespace ODL.VSEMProvider.Libraries.Entity {
    /// <summary>
    /// VLANIDMappingConfig class.
    /// This class has the function to manage the VLANIDMapping.config file.
    /// </summary>
    [Serializable, DataContract]
    public class VLANIDMappingConfig : ConfigManagerBase {
        /// <summary>
        /// Config file name.
        /// </summary>
        private const string CONFIG_FILE_NAME = "VLANIDMapping.config";

        /// <summary>
        /// Entity(List of VSEMVLANIDMapping).
        /// </summary>
        [DataMember]
        public List<VSEMVLANIDMapping> VLANIDMapping {
            get;
            set;
        }

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="parentFolder">Name of VTNCoordinator host name as parent folder.</param>
        public VLANIDMappingConfig(string parentFolder) {
            // Initialize.
            this.SetPath(string.Format(CultureInfo.CurrentCulture,
                @"{0}\{1}",
                parentFolder,
                VLANIDMappingConfig.CONFIG_FILE_NAME));
        }

        /// <summary>
        /// Store the config info to field member(Entity).
        /// </summary>
        /// <param name="config">ConfigManager object.</param>
        protected override void SetEntity(ConfigManagerBase config) {
            if (config == null) {
                this.VLANIDMapping = new List<VSEMVLANIDMapping>();
            } else {
                this.VLANIDMapping = ((VLANIDMappingConfig)config).VLANIDMapping;
            }
        }
    }
}
