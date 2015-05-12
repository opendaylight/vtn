//     Copyright (c) 2015 NEC Corporation
//     All rights reserved.
//     This program and the accompanying materials  are   made
//     available under the terms of the Eclipse Public License
//     v1.0  which  accompanies  this  distribution,  and   is
//     available at  http://www.eclipse.org/legal/epl-v10.html


using System;
using System.Runtime.Serialization;

namespace ODL.VSEMProvider.Libraries.Entity {
    /// <summary>
    /// This class describes the information about the controller to be managed by Virtual Switch
    /// Extension Manager.
    /// </summary>
    [Serializable, DataContract]
    public class VSEMController {
        /// <summary>
        /// This parameter consists of controller IP Address(es) and port number(s).
        /// </summary>
        [DataMember]
        public string ControllerInfo { get; set; }
    }
}
