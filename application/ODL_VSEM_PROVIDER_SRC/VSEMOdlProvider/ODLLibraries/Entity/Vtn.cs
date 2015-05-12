//     Copyright (c) 2015 NEC Corporation
//     All rights reserved.
//     This program and the accompanying materials  are   made
//     available under the terms of the Eclipse Public License
//     v1.0  which  accompanies  this  distribution,  and   is
//     available at  http://www.eclipse.org/legal/epl-v10.html


using System;
using System.Collections.Generic;
using System.Globalization;
using System.Management.Automation;
using System.Net;
using System.Reflection;
using System.Text;
using System.Web.Script.Serialization;
using ODL.VSEMProvider.CTRLibraries.Common;
using ODL.VSEMProvider.VSEMEvents;

namespace ODL.VSEMProvider.CTRLibraries.Entity {
    /// <summary>
    /// Represents Vtn on ODL.
    /// </summary>
    public class Vtn {
        /// <summary>
        /// Constructor to initialize fields.
        /// </summary>
        /// <param name="apiEndpoint">Endpoint for ODL API.</param>
        /// <param name="credential">Credential to use.</param>
        public Vtn(string apiEndpoint, PSCredential credential) {
            var JavaScriptSerializer = new JavaScriptSerializer();
            JavaScriptSerializer.MaxJsonLength = int.MaxValue;
            StringBuilder json = new StringBuilder("\"apiEndpoint\":" + JavaScriptSerializer.Serialize(apiEndpoint));
            ODLVSEMETW.EventWriteStartODLLibrary(
                MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                json.ToString());
            if (string.IsNullOrEmpty(apiEndpoint)) {
                throw new ArgumentException("No API endpoint specified.");
            }

            if (credential == null) {
                throw new ArgumentException("No credential provided.");
            }

            NetworkCredential cred = new NetworkCredential(credential.UserName,
                credential.Password);
            if (!apiEndpoint.StartsWith(@"https://", StringComparison.Ordinal)) {
                apiEndpoint = string.Format(CultureInfo.CurrentCulture,
                    @"https://{0}",
                    apiEndpoint);
            }
            if (apiEndpoint.EndsWith(@"/", StringComparison.Ordinal)) {
                apiEndpoint = apiEndpoint.Substring(0, apiEndpoint.Length - 1);
            }

            this.ApiEndpoint = apiEndpoint;
            this.Credential = cred;
            ODLVSEMETW.EventWriteEndODLLibrary(
                MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(), string.Empty);
        }

        /// <summary>
        /// Constructor to initialize fields.
        /// </summary>
        /// <param name="apiEndpoint">Endpoint for ODL API.</param>
        /// <param name="credential">Credential to use.</param>
        public Vtn(string apiEndpoint, NetworkCredential credential) {
            var JavaScriptSerializer = new JavaScriptSerializer();
            JavaScriptSerializer.MaxJsonLength = int.MaxValue;
            StringBuilder json = new StringBuilder("\"apiEndpoint\":" + JavaScriptSerializer.Serialize(apiEndpoint));
            ODLVSEMETW.EventWriteStartODLLibrary(
                MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                json.ToString());
            if (string.IsNullOrEmpty(apiEndpoint)) {
                throw new ArgumentException("No API endpoint specified.");
            }

            if (credential == null) {
                throw new ArgumentException("No credential provided.");
            }

            if (!apiEndpoint.StartsWith(@"https://", StringComparison.Ordinal)) {
                apiEndpoint = string.Format(CultureInfo.CurrentCulture,
                    @"https://{0}",
                    apiEndpoint);
            }
            if (apiEndpoint.EndsWith(@"/", StringComparison.Ordinal)) {
                apiEndpoint = apiEndpoint.Substring(0, apiEndpoint.Length - 1);
            }

            this.ApiEndpoint = apiEndpoint;
            this.Credential = credential;
            this.Vbridges = new List<Vbridge>();
            ODLVSEMETW.EventWriteEndODLLibrary(
                MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(), string.Empty);
        }

        /// <summary>
        /// ODL WebAPI endpoint.
        /// </summary>
        public string ApiEndpoint {
            get;
            private set;
        }

        /// <summary>
        /// Credential to use.
        /// </summary>
        public NetworkCredential Credential {
            get;
            private set;
        }

        /// <summary>
        /// Name of the VTN.
        /// </summary>
        public string Name {
            get;
            set;
        }

        /// <summary>
        /// List of vBridges attached to the VTN.
        /// </summary>
        public List<Vbridge> Vbridges {
            get;
            set;
        }

        /// <summary>
        /// Create vtn on ODL.
        /// </summary>
        /// <param name="name">Name of vtn.</param>
        public void AddVtn(string name) {
            StringBuilder json = new StringBuilder("\"name\":\"" + name + "\"");
            ODLVSEMETW.EventWriteStartODLLibrary(MethodBase.GetCurrentMethod().Name,
                json.ToString());
            ODLVSEMETW.EventWriteCreateUri(MethodBase.GetCurrentMethod().Name,
                    "Creating URI for creating VTN on ODL.");
            if (string.IsNullOrEmpty(name)) {
                throw new ArgumentException("No vtn name is specified.");
            }
            string uri = string.Format(CultureInfo.CurrentCulture,
                @"{0}/vtn-webapi/vtns",
                this.ApiEndpoint);
            var request = (HttpWebRequest)WebRequest.Create(uri);
            request.Credentials = this.Credential;

            string xmlData = string.Format(CultureInfo.CurrentCulture,
                "<vtn vtn_name=\"{0}\"/>",
                name);

            using (var response = request.XmlRequest(Constants.RETRY_COUNT,
                Constants.TIMEOUT_SECONDS,
                Constants.METHOD_POST,
                xmlData)) {
                if (response.StatusCode != HttpStatusCode.Created) {
                    ODLVSEMETW.EventWriteFailedHttpRequestError(
                        MethodBase.GetCurrentMethod().Name,
                        string.Format(CultureInfo.CurrentCulture,
                        "Failed to create VTN: {0}",
                        response.StatusDescription));
                    throw new InvalidOperationException(string.Format(CultureInfo.CurrentCulture,
                        "Failed to create VTN: {0}",
                        response.StatusDescription));
                }
                ODLVSEMETW.EventWriteEndODLLibrary(MethodBase.GetCurrentMethod().Name, string.Empty);
            }
        }

        /// <summary>
        /// Delete the VTN on ODL.
        /// </summary>
        /// <param name="name">Vtn name.</param>
        public void RemoveVtn(string name) {
            StringBuilder json = new StringBuilder("\"name\":\"" + name + "\"");
            ODLVSEMETW.EventWriteStartODLLibrary(MethodBase.GetCurrentMethod().Name,
                json.ToString());
            ODLVSEMETW.EventWriteCreateUri(MethodBase.GetCurrentMethod().Name,
                    "Creating URI deleting VTN from ODL.");
            if (string.IsNullOrEmpty(name)) {
                throw new ArgumentException("No vtn name is specified.");
            }
            string uri = string.Format(CultureInfo.CurrentCulture,
                @"{0}/vtn-webapi/vtns/{1}",
                this.ApiEndpoint,
                name);
            var request = (HttpWebRequest)WebRequest.Create(uri);
            request.Credentials = this.Credential;

            using (var response = request.XmlRequest(Constants.RETRY_COUNT,
               Constants.TIMEOUT_SECONDS,
               Constants.METHOD_DELETE,
               null)) {
                if (response.StatusCode != HttpStatusCode.NoContent) {
                    ODLVSEMETW.EventWriteFailedVTNRemoval(
                        MethodBase.GetCurrentMethod().Name,
                        string.Format(CultureInfo.CurrentCulture,
                        "Failed to delete VTN: {0}",
                        response.StatusDescription));
                    throw new InvalidOperationException(string.Format(CultureInfo.CurrentCulture,
                        "Failed to delete VTN: {0}",
                        response.StatusDescription));
                }
                ODLVSEMETW.EventWriteEndODLLibrary(MethodBase.GetCurrentMethod().Name, string.Empty);
            }
        }
    }
}
