//     Copyright (c) 2015 NEC Corporation
//     All rights reserved.
//     This program and the accompanying materials  are   made
//     available under the terms of the Eclipse Public License
//     v1.0  which  accompanies  this  distribution,  and   is
//     available at  http://www.eclipse.org/legal/epl-v10.html

using System;
using System.Management.Automation;
using ODL.VSEMProvider.VSEMEvents;

namespace ODL.VSEMProvider.Cmdlets.Common {
    /// <summary>
    /// Cmdlet base class.
    /// </summary>
    public abstract class VSEMODLCmdletBase : PSCmdlet {
        /// <summary>
        /// Symbol for connecting keywords.
        /// </summary>
        private const string CMDLET_SEP = "-";

        /// <summary>
        /// Field variable for stocking a cmdlet name.
        /// </summary>
        private string cmdletName = null;

        /// <summary>
        /// Field variable for stocking a cmdlet name.
        /// </summary>
        public string CmdletName {
            get {
                return this.cmdletName;
            }

            set {
                this.cmdletName = value;
            }
        }

        /// <summary>
        /// Constructor.
        /// </summary>
        protected VSEMODLCmdletBase() {
            var attributes = Attribute.GetCustomAttributes(this.GetType());

            // Assemble keywords into a cmdlet(XXX-YYY).
            foreach (var attr in attributes) {
                if (attr is CmdletAttribute) {
                    CmdletAttribute cmdletAttribute = attr as CmdletAttribute;
                    this.CmdletName = cmdletAttribute.VerbName + CMDLET_SEP + cmdletAttribute.NounName;
                    break;
                }
            }

            // string.Empty : Avoiding the ETW error.
            if (this.CmdletName == null) {
                this.CmdletName = string.Empty;
            }
        }

        /// <summary>
        /// Execute the start process of Cmdlet.
        /// </summary>
        protected override sealed void BeginProcessing() {
            ODLVSEMETW.EventWriteStartCmdlet(this.CmdletName, string.Empty);
            this.BeginODLVSEMCmdlet();
        }

        /// <summary>
        /// Execute the main process of Cmdlet.
        /// </summary>
        protected override sealed void ProcessRecord() {
            this.DoODLVSEMCmdlet();
        }

        /// <summary>
        /// Execute the end process of Cmdlet.
        /// </summary>
        protected override sealed void EndProcessing() {
            this.EndODLVSEMCmdlet();
        }

        /// <summary>
        /// Execute the stop process of Cmdlet.
        /// </summary>
        protected override sealed void StopProcessing() {
            ODLVSEMETW.EventWriteEndCmdlet(this.CmdletName, string.Empty);
        }

        /// <summary>
        /// Start process : Preprocessing of a cmdlet process.
        /// </summary>
        protected virtual void BeginODLVSEMCmdlet() {
        }

        /// <summary>
        /// Main process : Execute a cmdlet process.
        /// </summary>
        protected abstract void DoODLVSEMCmdlet();

        /// <summary>
        /// End process : Postprocessing of a cmdlet process.
        /// </summary>
        protected virtual void EndODLVSEMCmdlet() {
        }
    }
}
