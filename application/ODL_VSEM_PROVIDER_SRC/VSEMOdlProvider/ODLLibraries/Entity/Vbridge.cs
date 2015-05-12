//     Copyright (c) 2013-2014 NEC Corporation
//     All rights reserved.
//     This program and the accompanying materials  are   made
//     available under the terms of the Eclipse Public License
//     v1.0  which  accompanies  this  distribution,  and   is
//     available at  http://www.eclipse.org/legal/epl-v10.html

using System;
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
    /// It represents vBrige from ODL.
    /// </summary>
    public class Vbridge {
        /// <summary>
        /// Constructor to initialize fields.
        /// </summary>
        /// <param name="apiEndpoint">Endpoint for ODL API.</param>
        /// <param name="credential">Credential to use.</param>
        public Vbridge(string apiEndpoint, PSCredential credential) {
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
            ODLVSEMETW.EventWriteEndLibrary(
                MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                string.Empty);
        }

        /// <summary>
        /// Constructor to initialize fields.
        /// </summary>
        /// <param name="apiEndpoint">Endpoint for ODL API.</param>
        /// <param name="credential">Credential to use.</param>
        public Vbridge(string apiEndpoint, NetworkCredential credential) {
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
            ODLVSEMETW.EventWriteEndLibrary(
                MethodBase.GetCurrentMethod().DeclaringType.Name.ToString(),
                string.Empty);
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
        /// Name of the vBridge.
        /// </summary>
        public string Name {
            get;
            set;
        }

        /// <summary>
        /// VLAN ID associated with the vBridge.
        /// </summary>
        public long VlanId {
            get;
            set;
        }

        /// <summary>
        /// Add vBridge on ODL.
        /// </summary>
        /// <param name="name"> Name of vBridge.</param>
        /// <param name="vtnName">Nam eof vtn.</param>
        /// <param name="vlanId">Vlan id to associate.</param>
        public void AddVbridge(string name, string vtnName, long vlanId) {
            StringBuilder json = new StringBuilder("\"vtnName\":\"" + vtnName + "\"");
            json.Append("\"name\":\"" + name + "\"");
            json.Append("\"vlanId\":\"" + vlanId + "\"");
            ODLVSEMETW.EventWriteStartODLLibrary(MethodBase.GetCurrentMethod().Name,
                json.ToString());
            if (string.IsNullOrEmpty(name)) {
                throw new ArgumentException("No vBridge name is specified.");
            }

            if (string.IsNullOrEmpty(vtnName)) {
                throw new ArgumentException("No vtn name specified.");
            }

            if (vlanId == 0) {
                throw new ArgumentException("No vlan Id specified.");
            }
            string uri = string.Format(CultureInfo.CurrentCulture,
                @"{0}/vtn-webapi/vtns/{1}/vbridges", 
                this.ApiEndpoint, 
                vtnName);
            var request = (HttpWebRequest)WebRequest.Create(uri);
            request.Credentials = this.Credential;

            string xmlData = string.Format(CultureInfo.CurrentCulture,
                "<vbridge vbr_name=\"{0}\" controller_id=\"{1}\" domain_id=\"{2}\"/>", 
                name,
                Constants.CTR_NAME,
                Constants.DOMAIN);

            using (var response =
                request.XmlRequest(Constants.RETRY_COUNT,
                Constants.TIMEOUT_SECONDS,
                Constants.METHOD_POST,
                xmlData)) {
                    if (response.StatusCode != HttpStatusCode.Created)
                    {
                    ODLVSEMETW.EventWriteFailedHttpRequestError(
                        MethodBase.GetCurrentMethod().Name,
                        string.Format(CultureInfo.CurrentCulture,
                        "Failed to create vBridge: {0}",
                        response.StatusDescription));
                    throw new InvalidOperationException(string.Format(CultureInfo.CurrentCulture,
                        "Failed to create vBridge: {0}", 
                        response.StatusDescription));
                }
            }

            this.AssociateVlan(uri, name, vlanId);

            ODLVSEMETW.EventWriteReturnODLLibrary("Return from ODL Library.",
                string.Empty);
            ODLVSEMETW.EventWriteEndLibrary(MethodBase.GetCurrentMethod().Name,
                string.Empty);
        }

        /// <summary>
        /// Delete the vBridge on ODL.
        /// </summary>
        /// <param name="vtnName">Name of vtn.</param>
        /// <param name="vbrName">Name of vBridge.</param>
        public void RemoveVbridge(string vtnName, string vbrName) {
            StringBuilder json = new StringBuilder("\"vtnName\":\"" + vtnName + "\"");
            json.Append("\"vbrName\":\"" + vbrName + "\"");
            ODLVSEMETW.EventWriteStartODLLibrary(MethodBase.GetCurrentMethod().Name,
                json.ToString());
            if (string.IsNullOrEmpty(vbrName)) {
                throw new ArgumentException("No vBridge name is specified.");
            }

            if (string.IsNullOrEmpty(vtnName)) {
                throw new ArgumentException("No vtn name specified.");
            }
            string uri = string.Format(CultureInfo.CurrentCulture,
                @"{0}/vtn-webapi/vtns/{1}/vbridges/{2}",
                this.ApiEndpoint,
                vtnName,
                vbrName);
            ODLVSEMETW.EventWriteCreateUri(MethodBase.GetCurrentMethod().Name,
                    "Creating URI for removing vBridge from ODL");
            var request = (HttpWebRequest)WebRequest.Create(uri);
            request.Credentials = this.Credential;

            using (var response = request.XmlRequest(Constants.RETRY_COUNT,
               Constants.TIMEOUT_SECONDS,
               Constants.METHOD_DELETE,
               null)) {
                if (response.StatusCode != HttpStatusCode.NoContent) {
                    ODLVSEMETW.EventWriteFailedHttpRequestError(
                        MethodBase.GetCurrentMethod().Name,
                        string.Format(CultureInfo.CurrentCulture,
                        "Failed to delete vBridge: {0}",
                        response.StatusDescription));
                    throw new InvalidOperationException(string.Format(CultureInfo.CurrentCulture,
                        "Failed to delete vBridge: {0}.", 
                        response.StatusDescription));
                }
                ODLVSEMETW.EventWriteEndODLLibrary(MethodBase.GetCurrentMethod().Name,
                    string.Empty);
            }
        }

        /// <summary>
        /// Create vlan Mapping on ODL.
        /// </summary>
        /// <param name="uri"> Connection string.</param>
        /// <param name="name">Name of vBridge.</param>
        /// <param name="vlanId">Vlan id to associate.</param>
        private void AssociateVlan(string uri, string name, long vlanId) {
            StringBuilder json = new StringBuilder("\"uri\":\"" + uri + "\"");
            json.Append("\"name\":\"" + name + "\"");
            json.Append("\"vlanId\":\"" + vlanId + "\"");
            ODLVSEMETW.EventWriteStartODLLibrary(MethodBase.GetCurrentMethod().Name,
                json.ToString());
            if (string.IsNullOrEmpty(name)) {
                throw new ArgumentException("No vBridge name is specified.");
            }

            if (string.IsNullOrEmpty(uri)) {
                throw new ArgumentException("No API endpoint specified.");
            }

            if (vlanId == 0) {
                throw new ArgumentException("No vlan Id specified.");
            }

            string vlanuri = string.Format(CultureInfo.CurrentCulture,
                @"{0}/{1}/vlanmaps", 
                uri, 
                name);
            var request = (HttpWebRequest)WebRequest.Create(vlanuri);
            request.Credentials = this.Credential;

            string xmlData = string.Format(CultureInfo.CurrentCulture,
                "<vlanmap vlan_id=\"{0}\"/>", 
                vlanId);
            using (var response = request.XmlRequest(Constants.RETRY_COUNT,
                Constants.TIMEOUT_SECONDS,
                Constants.METHOD_POST,
                xmlData)) {
                if (response.StatusCode != HttpStatusCode.Created) {
                    ODLVSEMETW.EventWriteFailedHttpRequestError(
                        MethodBase.GetCurrentMethod().Name,
                        string.Format(CultureInfo.CurrentCulture,
                        "Failed to add VLAN mapping: {0}",
                        response.StatusDescription));
                    throw new ItemNotFoundException(string.Format(CultureInfo.CurrentCulture,
                        "Failed to add VLAN mapping: {0}", 
                        response.StatusDescription));
                }
                ODLVSEMETW.EventWriteEndODLLibrary(MethodBase.GetCurrentMethod().Name,
                    string.Empty);
            }
        }
    }
}
