//     Copyright (c) 2015 NEC Corporation
//     All rights reserved.
//     This program and the accompanying materials  are   made
//     available under the terms of the Eclipse Public License
//     v1.0  which  accompanies  this  distribution,  and   is
//     available at  http://www.eclipse.org/legal/epl-v10.html

using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Management.Automation;
using System.Net;
using System.Reflection;
using System.Text;
using System.Web.Script.Serialization;
using System.Xml.Linq;
using ODL.VSEMProvider.CTRLibraries.Common;
using ODL.VSEMProvider.CTRLibraries.Entity;
using ODL.VSEMProvider.VSEMEvents;

namespace ODL.VSEMProvider.CTRLibraries
{
    /// <summary>
    /// This class is used to handle the controller related operations on vtn-coordinator.
    /// </summary>
    public class Controller
    {
        /// <summary>
        /// Vtn-coordinator WebAPI endpoint.
        /// </summary>
        public string Base_uri
        {
            get;
            private set;
        }

        /// <summary>
        /// Controller IP address.
        /// </summary>
        public string Ip
        {
            get;
            private set;
        }

        /// <summary>
        /// Conection string to use.
        /// </summary>
        public string Conn_str
        {
            get;
            private set;
        }

        /// <summary>
        /// Controller type.
        /// </summary>
        public string Ctr_type
        {
            get;
            private set;
        }

        /// <summary>
        /// Controller version.
        /// </summary>
        public string Version
        {
            get;
            private set;
        }

        /// <summary>
        /// Controller audit status.
        /// </summary>
        public string Audit_status
        {
            get;
            private set;
        }

        /// <summary>
        /// Credential to use.
        /// </summary>
        public NetworkCredential Credential
        {
            get;
            private set;
        }

        /// <summary>
        /// Constructor to initialize filed.
        /// </summary>
        /// <param name="conn">Endpoint for VTN-coordinator API.</param>
        /// <param name="ip_addr">ODL controller IP.</param>
        /// <param name="credential">Credential to use.</param>
        public Controller(string conn, string ip_addr, PSCredential credential)
        {
            if (string.IsNullOrEmpty(conn))
            {
                throw new ArgumentException("connection string empty");
            }

            if (credential == null)
            {
                throw new ArgumentException("No credential provided.");
            }

            if (string.IsNullOrEmpty(ip_addr))
            {
                throw new ArgumentException("NO Controller IP provided");
            }
            if (!conn.StartsWith(@"https://", StringComparison.Ordinal))
            {
                conn = string.Format(CultureInfo.CurrentCulture,
                    @"https://{0}",
                    conn);
            }
            if (conn.EndsWith(@"/", StringComparison.Ordinal))
            {
                conn = conn.Substring(0, conn.Length - 1);
            }
            NetworkCredential cred = new NetworkCredential(credential.UserName,
                    credential.Password);
            this.Ip = ip_addr.Split(':').FirstOrDefault();
            this.Base_uri = conn;
            this.Credential = cred;
            this.Ctr_type = Constants.CTR_TYPE;
            this.Version = Constants.CTR_VERSION;
            this.Audit_status = Constants.CTR_AUDIT;
        }

        /// <summary>
        /// Constructor to initialize filed.
        /// </summary>
        /// <param name="conn">Endpoint for VTN-coordinator API.</param>
        /// <param name="credential">Credential to use.</param>
        public Controller(string conn, PSCredential credential)
        {
            if (string.IsNullOrEmpty(conn))
            {
                throw new ArgumentException("connection string empty");
            }

            if (credential == null)
            {
                throw new ArgumentException("No credential provided.");
            }
            if (!conn.StartsWith(@"https://", StringComparison.Ordinal))
            {
                conn = string.Format(CultureInfo.CurrentCulture,
                    @"https://{0}",
                    conn);
            }
            if (conn.EndsWith(@"/", StringComparison.Ordinal))
            {
                conn = conn.Substring(0, conn.Length - 1);
            }
            NetworkCredential cred = new NetworkCredential(credential.UserName,
                    credential.Password);
            this.Base_uri = conn;
            this.Credential = cred;
        }

        /// <summary>
        /// Create controller on vtn-coordinator.
        /// </summary>
        /// <param name="name">Name of Controller.</param>
        public void Create_ctr(string name)
        {
            StringBuilder json = new StringBuilder("\"name\":\"" + name + "\"");
            ODLVSEMETW.EventWriteStartODLLibrary(MethodBase.GetCurrentMethod().Name,
                json.ToString());
            ODLVSEMETW.EventWriteCreateUri(MethodBase.GetCurrentMethod().Name,
                    "Creating URI for Creating Controller on vtn-coordinator.");
            if (string.IsNullOrEmpty(name))
            {
                throw new ArgumentException("No controller name is specified.");
            }
            string uri = string.Format(CultureInfo.CurrentCulture,
                @"{0}/vtn-webapi/controllers",
                this.Base_uri);
            var request = (HttpWebRequest)WebRequest.Create(uri);
            request.Credentials = this.Credential;

            string xmlData = string.Format(CultureInfo.CurrentCulture,
                "<controller controller_id=\"{0}\"  ipaddr=\"{1}\" type=\"{2}\" auditstatus=\"{3}\" version=\"{4}\"/>",
                name,
                this.Ip,
                this.Ctr_type,
                this.Audit_status,
                this.Version);
            using (var response = request.XmlRequest(Constants.RETRY_COUNT,
                    Constants.TIMEOUT_SECONDS,
                    Constants.METHOD_POST,
                    xmlData))
            {
                if (response.StatusCode != HttpStatusCode.Created)
                {
                    ODLVSEMETW.EventWriteFailedHttpRequestError(
                        MethodBase.GetCurrentMethod().Name,
                        string.Format(CultureInfo.CurrentCulture,
                        "Failed to create Controller: {0}",
                        response.StatusDescription));
                    throw new InvalidOperationException(string.Format(CultureInfo.CurrentCulture,
                        "Failed to create Controller: {0}",
                        response.StatusDescription));
               }
                ODLVSEMETW.EventWriteEndODLLibrary(MethodBase.GetCurrentMethod().Name, string.Empty);
            }
        }

        /// <summary>
        /// Delete controller on vtn-coordinator.
        /// </summary>
        /// <param name="name">Name of Controller.</param>
        public void Delete_ctr(string name)
        {
            StringBuilder json = new StringBuilder("\"name\":\"" + name + "\"");
            ODLVSEMETW.EventWriteStartODLLibrary(MethodBase.GetCurrentMethod().Name,
                json.ToString());
            ODLVSEMETW.EventWriteCreateUri(MethodBase.GetCurrentMethod().Name,
                    "Creating URI for Deleting Controller on vtn-coordinator.");
            if (string.IsNullOrEmpty(name))
            {
                throw new ArgumentException("No controller name is specified.");
            }
            string uri = string.Format(CultureInfo.CurrentCulture,
                @"{0}/vtn-webapi/controllers/{1}",
                this.Base_uri,
                name);
            var request = (HttpWebRequest)WebRequest.Create(uri);
            request.Credentials = this.Credential;
            using (var response = request.XmlRequest(Constants.RETRY_COUNT,
                   Constants.TIMEOUT_SECONDS,
                   Constants.METHOD_DELETE,
                   null))
            {
                if (response.StatusCode != HttpStatusCode.NoContent)
                {
                    ODLVSEMETW.EventWriteFailedVTNRemoval(
                        MethodBase.GetCurrentMethod().Name,
                        string.Format(CultureInfo.CurrentCulture,
                        "Failed to delete Controller: {0}",
                        response.StatusDescription));
                    throw new InvalidOperationException(string.Format(CultureInfo.CurrentCulture,
                        "Failed to delete Controller: {0}",
                        response.StatusDescription));
                }
                ODLVSEMETW.EventWriteEndODLLibrary(MethodBase.GetCurrentMethod().Name, string.Empty);
            }
        }

        /// <summary>
        /// This function is responsible for get controller status
        /// from Vtn-coordinator.
        /// </summary>
        /// <param name="name">Name of Controller.</param>
        /// <returns>Returns true if controller status is UP. </returns>
        public bool Get_status(string name)
        {
            StringBuilder json = new StringBuilder("\"name\":\"" + name + "\"");
            ODLVSEMETW.EventWriteStartODLLibrary(
                    MethodBase.GetCurrentMethod().Name,
                    json.ToString());
            if (string.IsNullOrEmpty(name))
            {
                throw new ArgumentException("No controller name is specified.");
            }
            var JavaScriptSerializer = new JavaScriptSerializer();
            string status = null;
            ODLVSEMETW.EventWriteCreateUri(MethodBase.GetCurrentMethod().Name,
                        "Creating URI for retrieving the Controller info on Vtn-coordinator.");
            string requestUri = string.Format(CultureInfo.CurrentCulture,
                @"{0}/vtn-webapi/controllers/{1}.xml",
                this.Base_uri,
                name);
            HttpWebRequest request = (HttpWebRequest)WebRequest.Create(requestUri);
            request.Credentials = this.Credential;

            var response = request.GetResponse(Constants.RETRY_COUNT, Constants.TIMEOUT_SECONDS);

            var respStream = response.GetResponseStream();

            XDocument doc = XDocument.Load(respStream);

            ODLVSEMETW.EventWriteGetVbridgesforVtn(MethodBase.GetCurrentMethod().Name,
                    "Extracting controller info from VTN-coordinator.");
            var ctrs = doc.Elements("controller");
            foreach (var id in ctrs)
            {
                status = id.Attribute("operstatus").Value;
                break;
            }
            int result = string.Compare(status, "up");
            respStream.Close();
            response.Close();

            string output = "\"operstatus\":" + JavaScriptSerializer.Serialize(status);
            ODLVSEMETW.EventWriteEndODLLibrary(MethodBase.GetCurrentMethod().Name, output);

            if (result == 0)
            {
                return true;
            }
            return false;
        }

        /// <summary>
        /// This method is responsible for making the connection with ODL through
        /// http request and retrieve webApi version.
        /// </summary>
        /// <returns>Extracts VTNCoordinator_webapi_version from the response and return.</returns>
        public string RequestWebApiVersion() {
            var JavaScriptSerializer = new JavaScriptSerializer();
            JavaScriptSerializer.MaxJsonLength = int.MaxValue;
            ODLVSEMETW.EventWriteStartODLLibrary(
                MethodBase.GetCurrentMethod().Name,
                string.Empty);
            ODLVSEMETW.EventWriteCreateUri(MethodBase.GetCurrentMethod().Name,
                    "Creating URI for getting webapi version from VTNCoordinator.");
            string requestUri = string.Format(CultureInfo.CurrentCulture,
                {0}/vtn-webapi/api_version.xml",
                this.Base_uri);

            HttpWebRequest request = (HttpWebRequest)WebRequest.Create(requestUri);
            request.Credentials = this.Credential;

            var response = request.GetResponse(Constants.RETRY_COUNT,
                Constants.TIMEOUT_SECONDS_RETRIEVE_WEBAPI_VERSION);

            var respStream = response.GetResponseStream();

            XDocument doc = XDocument.Load(respStream);

            var versionElement = doc.Elements("api_version").FirstOrDefault();
            string webapiVersionFound = string.Empty;

            if (versionElement != null) {
                webapiVersionFound = versionElement.Attribute("version").Value;
            }

            respStream.Close();
            response.Close();
            string output = "\"webapiVersionFound\":" + JavaScriptSerializer.Serialize(webapiVersionFound);
            ODLVSEMETW.EventWriteEndODLLibrary(MethodBase.GetCurrentMethod().Name, output);
            return webapiVersionFound;
        }

        /// <summary>
        /// This method is responsible for making the connection with ODL through
        /// http request and retrieve the list of VTNs.
        /// </summary>
        /// <returns>Extracts list of vtns from the response and return.</returns>
        public List<string> GetVtnList() {
            var JavaScriptSerializer = new JavaScriptSerializer();
            JavaScriptSerializer.MaxJsonLength = int.MaxValue;
            ODLVSEMETW.EventWriteStartODLLibrary(
                MethodBase.GetCurrentMethod().Name,
                string.Empty);
            List<string> names = new List<string>();

            ODLVSEMETW.EventWriteCreateUri(MethodBase.GetCurrentMethod().Name,
                   "Creating URI for getting VTN from ODL.");
            string requestUri = string.Format(CultureInfo.CurrentCulture,
                "{0}/vtn-webapi/vtns.xml",
                this.Base_uri);

            HttpWebRequest request = (HttpWebRequest)WebRequest.Create(requestUri);
            request.Credentials = this.Credential;

            var response = request.GetResponse(Constants.RETRY_COUNT, Constants.TIMEOUT_SECONDS);

            var respStream = response.GetResponseStream();

            XDocument doc = XDocument.Load(respStream);
            ODLVSEMETW.EventWriteGetVtn(MethodBase.GetCurrentMethod().Name,
                    "Extracting list of VTN's from ODL.");
            var vtns = doc.Elements("vtns").Elements("vtn");
            foreach (var v in vtns) {
                names.Add(v.Attribute("vtn_name").Value);
            }

            respStream.Close();
            response.Close();
            string output = "\"names\":" + JavaScriptSerializer.Serialize(names);

            ODLVSEMETW.EventWriteEndODLLibrary(MethodBase.GetCurrentMethod().Name, output);
            return names;
        }

        /// <summary>
        /// This method is responsible for making the connection with ODL through
        /// http request and retrieve the list of VTNs.
        /// </summary>
        /// <param name="readVTN">Vtn name.</param>
        /// <returns>Extracts list of vtns from the response and return.</returns>
        public List<Vtn> ReadVTNObjects(string readVTN) {
            var JavaScriptSerializer = new JavaScriptSerializer();
            JavaScriptSerializer.MaxJsonLength = int.MaxValue;
            ODLVSEMETW.EventWriteStartODLLibrary(
                MethodBase.GetCurrentMethod().Name,
                string.Empty);
            List<Vtn> vtns = new List<Vtn>();
            List<string> vtnNames = this.GetVtnList();
            if (!string.IsNullOrEmpty(readVTN)) {
                vtnNames.RemoveAll(vtn => vtn.CompareTo(readVTN) != 0);
            }
            foreach (string vtnName in vtnNames) {
                var vtn = new Vtn(this.Base_uri, this.Credential) {
                    Name = vtnName
                };
                List<string> vbrNames = this.GetVbridgesListforVtn(vtnName);
                foreach (string vbrName in vbrNames) {
                    long vlanID = this.GetVLANIDforVbridge(vtnName, vbrName);
                    if (vlanID != 0) {
                        var vbr = new Vbridge(this.Base_uri, this.Credential) {
                            Name = vbrName,
                            VlanId = vlanID
                        };
                        vtn.Vbridges.Add(vbr);
                    }
                }
                if (vtn.Vbridges.Count() > 0) {
                    vtns.Add(vtn);
                }
            }

            string output = "\"vtns\":" + JavaScriptSerializer.Serialize(vtns.Select(vtn => vtn.Name));

            ODLVSEMETW.EventWriteEndODLLibrary(MethodBase.GetCurrentMethod().Name, output);
            return vtns;
        }

        /// <summary>
        /// Checks the status of vBridge on ODL.
        /// </summary>
        /// <param name="vtnName">Name of parent VTN.</param>
        /// <param name="vbrName">Name of parent vBridge.</param>
        /// <returns>VLAN ID.
        /// -2 if VTN is deleted.
        /// -1 if vBridge is deleted.
        /// </returns>
        public int CheckVbridgeStatus(string vtnName, string vbrName) {
            int status = -2;
            Vtn vtns = this.ReadVTNObjects(vtnName).FirstOrDefault();
            if (vtns != null) {
                Vbridge vbridge = vtns.Vbridges.FirstOrDefault(vbr => vbr.Name.CompareTo(vbrName) == 0);
                if (vbridge != null) {
                    long vlanID = vbridge.VlanId;
                    if (vlanID != 0) {
                        status = Convert.ToInt32(vlanID);
                    } else {
                        status = -1;
                    }
                } else {
                    status = -1;
                }
            } else {
                status = -2;
            }
            return status;
        }

        /// <summary>
        /// This method is responsible for making the connection with ODL through
        /// http request and retrieve the VLAN ID associated with a vBridge on ODL.
        /// </summary>
        /// <param name="vtnName">Vtn name.</param>
        /// <param name="vbrName">VBridge name.</param>
        /// <returns>Extracts VLANID of vBridges from the response and return.</returns>
        public long GetVLANIDforVbridge(string vtnName, string vbrName) {
            var JavaScriptSerializer = new JavaScriptSerializer();
            JavaScriptSerializer.MaxJsonLength = int.MaxValue;
            StringBuilder json = new StringBuilder("\"vtnName\":\"" + vtnName + "\"");
            json.Append("\"vbrName\":\"" + vbrName + "\"");
            ODLVSEMETW.EventWriteStartODLLibrary(
                MethodBase.GetCurrentMethod().Name,
                json.ToString());
            if (string.IsNullOrEmpty(vtnName)) {
                throw new ArgumentException("No vtn name is specified.");
            }
            if (string.IsNullOrEmpty(vbrName)) {
                throw new ArgumentException("No vBridge name is specified.");
            }

            long vlanID = 0;
            ODLVSEMETW.EventWriteCreateUri(MethodBase.GetCurrentMethod().Name,
                    "Creating URI for retrieving the VLAN ID associated with a vBridge on ODL.");
            string requestUri = string.Format(CultureInfo.CurrentCulture,
                "{0}/vtn-webapi/vtns/{1}/vbridges/{2}/vlanmaps/detail.xml",
                this.Base_uri,
                vtnName,
                vbrName);

            HttpWebRequest request = (HttpWebRequest)WebRequest.Create(requestUri);
            request.Credentials = this.Credential;

            var response = request.GetResponse(Constants.RETRY_COUNT, Constants.TIMEOUT_SECONDS);

            var respStream = response.GetResponseStream();

            XDocument doc = XDocument.Load(respStream);
            ODLVSEMETW.EventWriteGetVbridgesforVtn(MethodBase.GetCurrentMethod().Name,
                    "Extracting list of vBridges from Vtn-coordinator.");
            var ids = doc.Elements("vlanmaps").Elements("vlanmap");
            foreach (var id in ids) {
                int value = 0;
                int.TryParse(id.Attribute("vlan_id").Value, out value);
                if (value != 0) {
                    vlanID = value;
                    break;
                }
            }

            respStream.Close();
            response.Close();

            string output = "\"vlanID\":" + JavaScriptSerializer.Serialize(vlanID);
            ODLVSEMETW.EventWriteEndODLLibrary(MethodBase.GetCurrentMethod().Name, output);
            return vlanID;
        }

        /// <summary>
        /// This method is responsible for making the connection with ODL through
        /// http request and retrieve the list of vBridges under specified VTN.
        /// </summary>
        /// <param name="vtnName">Vtn name.</param>
        /// <returns>Extracts list of vBridges from the response and return.</returns>
        public List<string> GetVbridgesListforVtn(string vtnName) {
            var JavaScriptSerializer = new JavaScriptSerializer();
            JavaScriptSerializer.MaxJsonLength = int.MaxValue;
            StringBuilder json = new StringBuilder("\"vtnName\":\"" + vtnName + "\"");
            ODLVSEMETW.EventWriteStartODLLibrary(
                MethodBase.GetCurrentMethod().Name,
                json.ToString());
            if (string.IsNullOrEmpty(vtnName)) {
                throw new ArgumentException("No vtn name is specified.");
            }

            List<string> names = new List<string>();
            ODLVSEMETW.EventWriteCreateUri(MethodBase.GetCurrentMethod().Name,
                    "Creating URI for getting vBridges from ODL.");
            string requestUri = string.Format(CultureInfo.CurrentCulture,
                "{0}/vtn-webapi/vtns/{1}/vbridges.xml",
                this.Base_uri,
                vtnName);

            HttpWebRequest request = (HttpWebRequest)WebRequest.Create(requestUri);
            request.Credentials = this.Credential;

            var response = request.GetResponse(Constants.RETRY_COUNT, Constants.TIMEOUT_SECONDS);

            var respStream = response.GetResponseStream();

            XDocument doc = XDocument.Load(respStream);
            ODLVSEMETW.EventWriteGetVbridgesforVtn(MethodBase.GetCurrentMethod().Name,
                    "Extracting list of vBridges from ODL.");
            var vtns = doc.Elements("vbridges").Elements("vbridge");
            foreach (var v in vtns) {
                names.Add(v.Attribute("vbr_name").Value);
            }

            respStream.Close();
            response.Close();

            string output = "\"names\":" + JavaScriptSerializer.Serialize(names);
            ODLVSEMETW.EventWriteEndODLLibrary(MethodBase.GetCurrentMethod().Name, output);
            return names;
        }

        /// <summary>
        /// Update startup-configuration on VTNCoordinator.
        /// </summary>
        public void UpdateStartupConfiguration() {
            ODLVSEMETW.EventWriteStartODLLibrary(
                MethodBase.GetCurrentMethod().Name,
                string.Empty);

            string uri = string.Format(CultureInfo.CurrentCulture,
                @"{0}/vtn-webapi/configuration.xml",
                this.Base_uri);
            string xmlData = string.Format(CultureInfo.CurrentCulture,
                "<configuration operation=\"save\"/>");
            var request = (HttpWebRequest)WebRequest.Create(uri);
            request.Credentials = this.Credential;

            using (var response = request.XmlRequest(Constants.RETRY_COUNT,
               Constants.TIMEOUT_SECONDS,
               Constants.METHOD_PUT,
               xmlData)) {
                if (response.StatusCode != HttpStatusCode.NoContent) {
                    ODLVSEMETW.EventWriteFailedHttpRequestError(
                        MethodBase.GetCurrentMethod().Name,
                        string.Format(CultureInfo.CurrentCulture,
                        "Failed to Update startup-configuration on VTNCoordinator: {0}",
                        response.StatusDescription));
                    throw new InvalidOperationException(string.Format(CultureInfo.CurrentCulture,
                        "Failed to Update startup-configuration on VTNCoordinator: {0}",
                        response.StatusDescription));
                }
                ODLVSEMETW.EventWriteEndODLLibrary(MethodBase.GetCurrentMethod().Name, string.Empty);
            }
        }
    }
}
