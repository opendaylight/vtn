//     Copyright (c) 2013-2014 NEC Corporation
//     All rights reserved.
//     This program and the accompanying materials  are   made
//     available under the terms of the Eclipse Public License
//     v1.0  which  accompanies  this  distribution,  and   is
//     available at  http://www.eclipse.org/legal/epl-v10.html

namespace ODL.VSEMProvider.CTRLibraries.Common {
    /// <summary>
    /// Constants of ODL Library.
    /// </summary>
    public static class Constants {
        /// <summary>
        /// Number of retries for web requests.
        /// </summary>
        public const int RETRY_COUNT = 3;

        /// <summary>
        /// Timeout for web requests in seconds.
        /// </summary>
        public const int TIMEOUT_SECONDS = 10;

        /// <summary>
        /// Timeout for web requests in seconds.
        /// </summary>
        public const int TIMEOUT_SECONDS_RETRIEVE_WEBAPI_VERSION = 5;

        /// <summary>
        /// HTTP Post method.
        /// </summary>
        public const string METHOD_POST = "POST";

        /// <summary>
        /// HTTP Delete method.
        /// </summary>
        public const string METHOD_DELETE = "DELETE";

        /// <summary>
        /// HTTP Put method.
        /// </summary>
        public const string METHOD_PUT = "PUT";

        /// <summary>
        /// Maximum length of name in WebAPI.
        /// </summary>
        public const int MAX_WEBAPI_NAME_LENGTH = 31;

        /// <summary>
        /// Controller Name.
        /// </summary>
        public const string CTR_NAME = "odc1";

        /// <summary>
        /// COntroller Type.
        /// </summary>
        public const string CTR_TYPE = "odc";

        /// <summary>
        /// COntroller Version.
        /// </summary>
        public const string CTR_VERSION = "1.0";

        /// <summary>
        /// COntroller AuditStatus.
        /// </summary>
        public const string CTR_AUDIT = "enable";

        /// <summary>
        /// Domain id.
        /// </summary>
        public const string DOMAIN = "(DEFAULT)";
    }
}
