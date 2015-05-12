//     Copyright (c) 2013-2014 NEC Corporation
//     All rights reserved.
//     This program and the accompanying materials  are   made
//     available under the terms of the Eclipse Public License
//     v1.0  which  accompanies  this  distribution,  and   is
//     available at  http://www.eclipse.org/legal/epl-v10.html

namespace ODL.VSEMProvider.Libraries.Common {
    /// <summary>
    /// This class represents the range of inegers.
    /// </summary>
    public class Range {
        /// <summary>
        /// Range lower limit.
        /// </summary>
        private int start;

        /// <summary>
        /// Range lower limit.
        /// </summary>
        public int Start {
            get {
                return this.start;
            }

            set {
                this.start = value;
            }
        }

        /// <summary>
        /// Range upper limit.
        /// </summary>
        private int end;

        /// <summary>
        /// Range upper limit.
        /// </summary>
        public int End {
            get {
                return this.end;
            }

            set {
                this.end = value;
            }
        }
    }
}
