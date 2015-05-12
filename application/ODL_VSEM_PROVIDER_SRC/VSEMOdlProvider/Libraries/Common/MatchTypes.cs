//     Copyright (c) 2015 NEC Corporation
//     All rights reserved.
//     This program and the accompanying materials  are   made
//     available under the terms of the Eclipse Public License
//     v1.0  which  accompanies  this  distribution,  and   is
//     available at  http://www.eclipse.org/legal/epl-v10.html


namespace ODL.VSEMProvider.Libraries.Common {
    /// <summary>
    /// Match types for the searching.
    /// </summary>
    public enum MatchTypes {
        /// <summary>
        /// Exact match.
        /// </summary>
        ExactMatch,

        /// <summary>
        /// Wild card match, where “*” is referred as a string of 0 or more characters.
        /// </summary>
        WildCardMatch
    }
}
