//     Copyright (c) 2015 NEC Corporation
//     All rights reserved.
//
//     This program and the accompanying materials are made available under the
//     terms of the Eclipse Public License v1.0 which accompanies this
//     distribution, and is available at http://www.eclipse.org/legal/epl-v10.html


using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Management.Automation;
using System.Runtime.InteropServices;
using System.Runtime.Serialization;
using System.Security;
using System.Security.Cryptography;
using System.Text;
using ODL.VSEMProvider.Libraries.Common;

namespace ODL.VSEMProvider.Libraries.Entity {
    /// <summary>
    /// ODLInformation class.
    /// This class has the function to manage the ODLInformation.config file.
    /// </summary>
    [Serializable, DataContract]
    public class OdlInformation : ConfigManagerBase {
        /// <summary>
        /// Config file name.
        /// </summary>
        private const string CONFIG_FILE_NAME = "ODLInformation.config";

        /// <summary>
        /// Entity(Information of ODL).
        /// </summary>
        [DataMember]
        public List<string> Info {
            get;
            set;
        }

        /// <summary>
        /// AES key for encryption and decryption.
        /// </summary>
        private byte[] AESKey = null;

        /// <summary>
        /// AES IV for encryption and decryption.
        /// </summary>
        private byte[] AESIV = null;

        /// <summary>
        /// Constructor to initialize CONFIG_FILE_NAME, AESKey and AESIV.
        /// </summary>
        /// <param name="parentFolder">Name of ODL host name as parent folder.</param>
        public OdlInformation(string parentFolder) {
            this.SetPath(string.Format(CultureInfo.CurrentCulture,
                @"{0}\{1}",
                parentFolder,
                OdlInformation.CONFIG_FILE_NAME));

            string secretKey = "!@#$%^&*()_+s34234SF#EEWEecret123key";
            string ivkey = "Secure786IV";

            System.Text.UTF8Encoding UTF8 = new System.Text.UTF8Encoding();
            MD5CryptoServiceProvider HashProvider = new MD5CryptoServiceProvider();

            this.AESKey = HashProvider.ComputeHash(UTF8.GetBytes(secretKey));
            this.AESIV = HashProvider.ComputeHash(UTF8.GetBytes(ivkey));
        }

        /// <summary>
        /// Method to get credentials of ODL from Info.
        /// </summary>
        /// <returns>ODL credentials.</returns>
        public PSCredential GetCredentials() {
            if (this.Info.Count != 3) {
                throw new ArgumentException("The parameter 'Info' is invalid.");
            }
            string username = this.Info[0];
            string decryptedString = string.Empty;

            using (AesCryptoServiceProvider aes = new AesCryptoServiceProvider()) {
                byte[] encryptAry = Convert.FromBase64String(this.Info[1]);

                // Decrypt the bytes to a string.
                decryptedString = this.DecryptStringFromBytes_Aes(encryptAry);
            }

            char[] chars = decryptedString.ToCharArray();
            SecureString password = new SecureString();
            foreach (char ch in chars) {
                password.AppendChar(ch);
            }

            PSCredential cred = new PSCredential(username, password);
            return cred;
        }

        /// <summary>
        /// Method to get connection string of ODL from Info.
        /// </summary>
        /// <returns>Connection string.</returns>
        public string GetConnectionString() {
            if (this.Info.Count != 3) {
                throw new ArgumentException("The parameter 'Info' is invalid.");
            }
            return this.Info[2];
        }

        /// <summary>
        /// Method to set credentials and connectionstring of ODL in Info.
        /// </summary>
        /// <param name="credentials">Credentials of ODL.</param>
        /// <param name="connectionString">Connection string of ODL.</param>
        public void SetCredentials(PSCredential credentials, string connectionString) {
            this.Info = new List<string>();
            this.Info.Add(credentials.UserName);
            string encryptStr = string.Empty;
            using (AesCryptoServiceProvider aes = new AesCryptoServiceProvider()) {
                // Encrypt the string to an array of bytes.
                byte[] encrypted = this.EncryptStringToBytes_Aes(this.SecureStringToString(credentials.Password));
                encryptStr = Convert.ToBase64String(encrypted);
            }
            this.Info.Add(encryptStr);
            this.Info.Add(connectionString);
        }

        /// <summary>
        /// Store the config info to field member(Entity).
        /// </summary>
        /// <param name="config">ConfigManager object.</param>
        protected override void SetEntity(ConfigManagerBase config) {
            if (config == null) {
                this.Info = null;
            } else {
                this.Info = ((OdlInformation)config).Info;
            }
        }

        /// <summary>
        /// Mthod for encryption.
        /// </summary>
        /// <param name="password">String to encrypt.</param>
        /// <returns>Encrypted cypher text.</returns>
        private byte[] EncryptStringToBytes_Aes(string password) {
            // Check arguments.
            if (password == null || password.Length <= 0) {
                throw new ArgumentException("The parameter 'password' is null or invalid.");
            }
            if (this.AESKey == null || this.AESKey.Length <= 0) {
                throw new ArgumentException("The 'AESKey' is null or invalid.");
            }
            if (this.AESIV == null || this.AESIV.Length <= 0) {
                throw new ArgumentException("The 'AESIV' is null or invalid.");
            }

            byte[] encrypted;

            // Create an AesCryptoServiceProvider object
            // with the specified key and IV.
            using (AesCryptoServiceProvider aesAlg = new AesCryptoServiceProvider()) {
                aesAlg.Key = this.AESKey;
                aesAlg.IV = this.AESIV;

                // Create a decrytor to perform the stream transform.
                ICryptoTransform encryptor = aesAlg.CreateEncryptor(aesAlg.Key, aesAlg.IV);

                // Create the streams used for encryption.
                MemoryStream msencrypt = null;
                try {
                    msencrypt = new MemoryStream();
                    CryptoStream csencrypt = null;
                    try {
                        csencrypt = new CryptoStream(msencrypt, encryptor, CryptoStreamMode.Write);
                        StreamWriter swencrypt = null;
                        try {
                            swencrypt = new StreamWriter(csencrypt);
                            csencrypt = null;
                            swencrypt.Write(password);
                        } finally {
                            if (swencrypt != null) {
                                swencrypt.Dispose();
                            }
                        }

                        encrypted = msencrypt.ToArray();
                        msencrypt = null;
                    } finally {
                        if (csencrypt != null) {
                            csencrypt.Dispose();
                        }
                    }
                } finally {
                    if (msencrypt != null) {
                        msencrypt.Dispose();
                    }
                }
            }

            return encrypted;
        }

        /// <summary>
        /// Method for decryption.
        /// </summary>
        /// <param name="cipherText">Array of bytes to decrypt.</param>
        /// <returns>Decrypted string.</returns>
        private string DecryptStringFromBytes_Aes(byte[] cipherText) {
            if (cipherText == null || cipherText.Length <= 0) {
                throw new ArgumentException("The parameter 'cipherText' is null or invalid.");
            }
            if (this.AESKey == null || this.AESKey.Length <= 0) {
                throw new ArgumentException("The 'AESKey' is null or invalid.");
            }
            if (this.AESIV == null || this.AESIV.Length <= 0) {
                throw new ArgumentException("The 'AESIV' is null or invalid.");
            }

            // Declare the string used to hold
            // the decrypted text.
            string plaintext = null;

            // Create an AesCryptoServiceProvider object
            // with the specified key and IV.
            using (AesCryptoServiceProvider aesAlg = new AesCryptoServiceProvider()) {
                aesAlg.Key = this.AESKey;
                aesAlg.IV = this.AESIV;

                // Create a decrytor to perform the stream transform.
                ICryptoTransform decryptor = aesAlg.CreateDecryptor(aesAlg.Key, aesAlg.IV);

                // Create the streams used for decryption.
                MemoryStream msdecrypt = null;
                try {
                    msdecrypt = new MemoryStream(cipherText);
                    CryptoStream csdecrypt = null;
                    try {
                        csdecrypt = new CryptoStream(msdecrypt, decryptor, CryptoStreamMode.Read);
                        msdecrypt = null;
                        using (StreamReader srdecrypt = new StreamReader(csdecrypt)) {
                            csdecrypt = null;

                            // Read the decrypted bytes from the decrypting stream
                            // and place them in a string.
                            plaintext = srdecrypt.ReadToEnd();
                        }
                    } finally {
                        if (csdecrypt != null) {
                            csdecrypt.Dispose();
                        }
                    }
                } finally {
                    if (msdecrypt != null) {
                        msdecrypt.Dispose();
                    }
                }
            }
            return plaintext;
        }

        /// <summary>
        /// Method to convert SecureString to String.
        /// </summary>
        /// <param name="value">SecureString t be converted.</param>
        /// <returns>Converted string.</returns>
        private string SecureStringToString(SecureString value) {
            IntPtr valuePtr = IntPtr.Zero;
            try {
                valuePtr = Marshal.SecureStringToGlobalAllocUnicode(value);
                return Marshal.PtrToStringUni(valuePtr);
            } finally {
                Marshal.ZeroFreeGlobalAllocUnicode(valuePtr);
            }
        }
    }
}
