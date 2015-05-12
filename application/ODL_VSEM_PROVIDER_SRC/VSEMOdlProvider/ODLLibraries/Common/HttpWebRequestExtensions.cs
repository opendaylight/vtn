//     Copyright (c) 2015 NEC Corporation
//     All rights reserved.
//     This program and the accompanying materials  are   made
//     available under the terms of the Eclipse Public License
//     v1.0  which  accompanies  this  distribution,  and   is
//     available at  http://www.eclipse.org/legal/epl-v10.html


using System;
using System.Diagnostics;
using System.Globalization;
using System.Net;
using System.Reflection;
using ODL.VSEMProvider.VSEMEvents;

namespace ODL.VSEMProvider.CTRLibraries.Common {
    /// <summary>
    /// Web request extension class.
    /// </summary>
    public static class HttpWebRequestExtensions {
        /// <summary>
        /// Returns a response from Internet source. If received a timeout response,
        /// retry for the specified amount of times.
        /// </summary>
        /// <param name="request">HTTP request.</param>
        /// <param name="retryCount">Number of times to retry.</param>
        /// <param name="timeoutSeconds">Interval of times to retry.</param>
        /// <returns>Respose of the web request.</returns>
        public static WebResponse GetResponse(this HttpWebRequest request,
            int retryCount,
            int timeoutSeconds) {
            ODLVSEMETW.EventWriteHttpRequest(MethodBase.GetCurrentMethod().Name,
                "Making Http Request.");

            int retry = 0;

            // Set the request timeout
            ODLVSEMETW.EventWriteHttpRequestTimeout(MethodBase.GetCurrentMethod().Name,
                "Setting Request Timeout and ReadWrite Timeout.");

            ODLVSEMETW.EventWriteHttpResponse(MethodBase.GetCurrentMethod().Name,
                string.Format(CultureInfo.CurrentCulture,
                "Getting web response for URI = '{0}'.",
                request.RequestUri.ToString()));
            Trace.WriteLine(string.Format(CultureInfo.CurrentCulture,
                "Getting web response for URI = '{0}'.",
                request.RequestUri.ToString()));

            do {
                try {
                    request.Timeout = timeoutSeconds * 1000;
                    request.ReadWriteTimeout = timeoutSeconds * 1000;
                    var response = request.GetResponse();

                    ODLVSEMETW.EventWriteRetryHttpResponse(MethodBase.GetCurrentMethod().Name,
                    string.Format(CultureInfo.CurrentCulture,
                    "Web response returned for URI = '{0}'.",
                    request.RequestUri.ToString()));
                    Trace.WriteLine(string.Format(CultureInfo.CurrentCulture,
                        "Web response returned for '{0}'.",
                        request.RequestUri.ToString()));

                    return response;
                } catch (WebException ex) {
                    HttpWebResponse response = ex.Response as HttpWebResponse;
                    if (ex.Status == WebExceptionStatus.Timeout
                        || ex.Status == WebExceptionStatus.NameResolutionFailure
                        || ex.Status == WebExceptionStatus.ConnectFailure
                        || ex.Status == WebExceptionStatus.Pending
                        || ex.Status == WebExceptionStatus.ReceiveFailure
                        || ex.Status == WebExceptionStatus.PipelineFailure
                        || ex.Status == WebExceptionStatus.SendFailure
                        || ex.Status == WebExceptionStatus.SecureChannelFailure
                        || (response != null
                        && response.StatusCode == HttpStatusCode.InternalServerError)) {
                        retry++;
                        if (retry > retryCount) {
                            if (response != null
                                && response.StatusCode == HttpStatusCode.InternalServerError) {
                                throw new WebException(internalServerErrorMessage, ex.Status);
                            } else {
                                throw;
                            }
                        }

                        ODLVSEMETW.EventWriteRetryCountHttpRequest(
                            MethodBase.GetCurrentMethod().Name,
                            string.Format(CultureInfo.CurrentCulture,
                            "Web request '{1}' failed, retry count = {0}.",
                            retry,
                            request.RequestUri.ToString()));
                        Trace.WriteLine(string.Format(CultureInfo.CurrentCulture,
                            "Web request '{1}' failed, retry count = {0}.",
                            retry,
                            request.RequestUri.ToString()));

                        // Wait 100 miliseconds before retrying
                        ODLVSEMETW.EventWriteWaitHttpRequest(
                            MethodBase.GetCurrentMethod().Name,
                           string.Format(CultureInfo.CurrentCulture,
                           "Waiting 100 miliseconds before retrying. retry count={0}, Request uri = {1}",
                            retry,
                            request.RequestUri.ToString()));
                        int sleepingtime = request.Timeout;
                        if (ex.Status == WebExceptionStatus.Timeout) {
                            sleepingtime = 100;
                        }
                        var uri = request.RequestUri;
                        var cred = request.Credentials;

                        request = (HttpWebRequest)WebRequest.Create(uri);
                        request.Credentials = cred;

                        System.Threading.Thread.Sleep(sleepingtime);
                    } else {
                        if (response != null) {
                            ODLVSEMETW.EventWriteHttpResponseError(
                                MethodBase.GetCurrentMethod().Name,
                                string.Format(CultureInfo.CurrentCulture,
                               "Web request '{0}' failed. Status Code={1}, StatusDescription={2}",
                                request.RequestUri.ToString(),
                                response.StatusCode.ToString(),
                                response.StatusDescription));
                            Trace.WriteLine(string.Format(CultureInfo.CurrentCulture,
                                "Web request '{0}' failed. Status Code={1}, StatusDescription={2}",
                                request.RequestUri.ToString(),
                                response.StatusCode.ToString(),
                                response.StatusDescription));
                        }
                        if (response != null
                                && response.StatusCode == HttpStatusCode.InternalServerError) {
                            throw new WebException(internalServerErrorMessage, ex.Status);
                        } else {
                            throw;
                        }
                    }
                }
            } while (retry <= retryCount);

            return null;
        }

        /// <summary>
        /// Message for internal server error.
        /// </summary>
        private static string internalServerErrorMessage =
            string.Format(CultureInfo.CurrentCulture,
            "{0}\n{1}\n1. {2}\n2. {3}\n3. {4}\n4. {5}",
            "'Internal Server Error' occurred while communicating with ODL.",
            "Possible reasons could be:",
            "The server failed to fulfill the request.",
            "The server is in the config mode.",
            "VM subnet is being deleted and the specified parent VTN is not found on ODL.",
            "VM subnet is being created with a VLAN ID which is already allocated to other vBridge on ODL.");

        /// <summary>
        /// Post the specified XML data.
        /// </summary>
        /// <param name="request">Web request.</param>
        /// <param name="retryCount">Number of times to retry.</param>
        /// <param name="timeoutSeconds">Timeouts in seconds.</param>
        /// <param name="method">HTTP method.</param>
        /// <param name="xmlData">XML data to send.</param>
        /// <returns>Web response.</returns>
        public static HttpWebResponse XmlRequest(this HttpWebRequest request,
            int retryCount,
            int timeoutSeconds,
            string method,
            string xmlData) {
            int retry = 0;

            do {
                try {
                    // Set the request timeout
                    request.Timeout = timeoutSeconds * 1000;
                    request.ReadWriteTimeout = timeoutSeconds * 1000;

                    // Don't use persistent connection as connection
                    // may be close in the middle of requests.
                    request.KeepAlive = false;
                    if (string.IsNullOrEmpty(method)
                        || (string.Compare(method, Constants.METHOD_DELETE, StringComparison.Ordinal) != 0
                        && string.Compare(method, Constants.METHOD_PUT, StringComparison.Ordinal) != 0
                        && string.Compare(method, Constants.METHOD_POST, StringComparison.Ordinal) != 0)) {
                        throw new ArgumentException("Request method is invalid.");
                    }

                    // Set request method
                    request.Method = method;

                    // Set the XML data
                    if (!string.IsNullOrEmpty(xmlData)) {
                        byte[] buffer = System.Text.Encoding.UTF8.GetBytes(xmlData);
                        request.ContentType = "text/xml";
                        request.ContentLength = buffer.Length;
                        var postData = request.GetRequestStream();
                        postData.Write(buffer, 0, buffer.Length);
                        postData.Close();
                    }
                    ODLVSEMETW.EventWriteRetryHttpResponse(MethodBase.GetCurrentMethod().Name,
                        string.Format(CultureInfo.CurrentCulture,
                        "Web response returned for URI = '{0}'",
                        request.RequestUri.ToString()));
                    var response = request.GetResponse();

                    return response as HttpWebResponse;
                } catch (WebException ex) {
                    HttpWebResponse response = ex.Response as HttpWebResponse;
                    if (ex.Status == WebExceptionStatus.Timeout
                        || ex.Status == WebExceptionStatus.SecureChannelFailure
                        || ex.Status == WebExceptionStatus.NameResolutionFailure
                        || ex.Status == WebExceptionStatus.ConnectFailure
                        || ex.Status == WebExceptionStatus.Pending
                        || ex.Status == WebExceptionStatus.ReceiveFailure
                        || ex.Status == WebExceptionStatus.PipelineFailure
                        || ex.Status == WebExceptionStatus.SendFailure
                        || (response != null
                        && response.StatusCode == HttpStatusCode.InternalServerError)) {
                        retry++;

                        if (retry > retryCount) {
                            if (response != null
                                && response.StatusCode == HttpStatusCode.InternalServerError) {
                                throw new WebException(internalServerErrorMessage, ex.Status);
                            } else {
                                throw;
                            }
                        }

                        // Wait 100 miliseconds before retrying
                        int sleepingtime = request.Timeout;
                        if (ex.Status == WebExceptionStatus.Timeout) {
                            sleepingtime = 100;
                        }
                        System.Threading.Thread.Sleep(sleepingtime);
                    } else {
                        if (response != null
                            && response.StatusCode == HttpStatusCode.InternalServerError) {
                            throw new WebException(internalServerErrorMessage, ex.Status);
                        } else {
                            throw;
                        }
                    }

                    // Recreate the request on retries
                    var uri = request.RequestUri;
                    var cred = request.Credentials;

                    request = (HttpWebRequest)WebRequest.Create(uri);
                    request.Credentials = cred;
                }
            } while (retry <= retryCount);

            return null;
        }
    }
}
