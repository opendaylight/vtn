//     Copyright (c) 2015 NEC Corporation
//     All rights reserved.
//
//     This program and the accompanying materials are made available under the
//     terms of the Eclipse Public License v1.0 which accompanies this
//     distribution, and is available at http://www.eclipse.org/legal/epl-v10.html


namespace ODL.VSEMProvider.VSEMEvents
{
using System;
using System.Diagnostics.Tracing;

    [EventSource(Name = "ODLVTN-VSEM-PROVIDER")]
    public sealed class EventProviderVersionTwo : EventSource
    {

        internal unsafe bool TemplateVSEMStarted(
            string FuncName,
            string Input_Parameters
            )
        {

            WriteEvent(1, FuncName);
            WriteEvent(1, Input_Parameters);
            return true;
        }



        internal unsafe bool TemplateVSEMEnded(
            string FuncName,
            string Output
            )
        {

              WriteEvent(2, FuncName);
              WriteEvent(2, Output);
              return true;
        }



        internal unsafe bool TemplateConfigManagerError(
            string FuncName,
            string ErrorMessage
            )
        {
             WriteEvent(3, FuncName);
             WriteEvent(3, ErrorMessage);
             return true;
        }



        internal unsafe bool TemplateConfigManagerFileOpen(
            string FileName,
            uint OpenMode
            )
        {

              WriteEvent(4, FileName,(int)OpenMode);

            return true;

        }



        internal unsafe bool TemplateConfigManagerFileClose(
            string FileName,
            uint OperationType
            )
        {
              WriteEvent(5, FileName, (int)OperationType);

            return true;

        }



        internal unsafe bool TemplateArgument(
            string Type,
            string Method,
            string Message
            )
        {
              WriteEvent(6, Type);
              WriteEvent(6, Method);
              WriteEvent(6, Message);

            return true;

        }



        internal unsafe bool TemplateVSEM_Processing(
            string Method_Name,
            string Information
            )
        {
              WriteEvent(7, Method_Name);
              WriteEvent(7, Information);

            return true;

        }



        internal unsafe bool TemplateParameters_logging(
            string Cmdlet,
            string Message,
            string Input_parameters
            )
        {
              WriteEvent(8, Cmdlet);
              WriteEvent(8, Message);


            return true;

        }

        internal unsafe bool EventWriteConfigMgr(
            string Message
            )
        {
              WriteEvent(9, Message);

            return true;

        }
	public static EventProviderVersionTwo LOG = new EventProviderVersionTwo();

    }

    public static class ODLVSEMETW
    {
        //
        // Provider ODL-Vtnco-VSEM-Provider Event Count 104
        //

        // Task :  eventGUIDs
        //

        //
        // Event Descriptors
        //

        //
        // Event method for StartCmdlet
        //
        public static bool EventWriteStartCmdlet(string FuncName, string Input_Parameters)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEMStarted(FuncName, Input_Parameters);
        }

        //
        // Event method for StartLibrary
        //
        public static bool EventWriteStartLibrary(string FuncName, string Input_Parameters)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEMStarted( FuncName, Input_Parameters);
        }

        //
        // Event method for StartODLLibrary
        //
        public static bool EventWriteStartODLLibrary(string FuncName, string Input_Parameters)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEMStarted( FuncName, Input_Parameters);
        }

        //
        // Event method for EndCmdlet
        //
        public static bool EventWriteEndCmdlet(string FuncName, string Output)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEMEnded( FuncName, Output);
        }

        //
        // Event method for EndLibrary
        //
        public static bool EventWriteEndLibrary(string FuncName, string Output)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEMEnded( FuncName, Output);
        }

        //
        // Event method for EndODLLibrary
        //
        public static bool EventWriteEndODLLibrary(string FuncName, string Output)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEMEnded( FuncName, Output);
        }

        //
        // Event method for ConfigManagerDiagError
        //
        public static bool EventWriteConfigManagerDiagError(string FuncName, string ErrorMessage)
        {

            return EventProviderVersionTwo.LOG.TemplateConfigManagerError( FuncName, ErrorMessage);
        }

        //
        // Event method for ConfigManagerFileIOError
        //
        public static bool EventWriteConfigManagerFileIOError(string FuncName, string ErrorMessage)
        {

            return EventProviderVersionTwo.LOG.TemplateConfigManagerError( FuncName, ErrorMessage);
        }

        //
        // Event method for ConfigManagerFileOpen
        //
        public static bool EventWriteConfigManagerFileOpen(string FileName, uint OpenMode)
        {

            return EventProviderVersionTwo.LOG.TemplateConfigManagerFileOpen( FileName, OpenMode);
        }

        //
        // Event method for ConfigManagerFileClose
        //
        public static bool EventWriteConfigManagerFileClose(string FileName, uint OperationType)
        {

            return EventProviderVersionTwo.LOG.TemplateConfigManagerFileClose( FileName, OperationType);
        }

        //
        // Event method for ConfigManagerFileDelete
        //
        public static bool EventWriteConfigManagerFileDelete(string FileName)
        {

            return EventProviderVersionTwo.LOG.EventWriteConfigMgr( FileName);

        }

        //
        // Event method for ArgumentError
        //
        public static bool EventWriteArgumentError(string Type, string Method, string Message)
        {

            return EventProviderVersionTwo.LOG.TemplateArgument( Type, Method, Message);
        }

        //
        // Event method for CheckMatchType
        //
        public static bool EventWriteCheckMatchType(string FuncName, string Input_Parameters)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEMStarted( FuncName, Input_Parameters);
        }

        //
        // Event method for CloseConnectionInformation
        //
        public static bool EventWriteCloseConnectionInformation(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for ConnectionStringError
        //
        public static bool EventWriteConnectionStringError(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for ConnectionStringFormatError
        //
        public static bool EventWriteConnectionStringFormatError(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for ControllerFormatError
        //
        public static bool EventWriteControllerFormatError(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for CountControllerIPError
        //
        public static bool EventWriteCountControllerIPError(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for CreateSwitchExtensionInfo
        //
        public static bool EventWriteCreateSwitchExtensionInfo(string FuncName, string Input_Parameters)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEMStarted( FuncName, Input_Parameters);
        }

        //
        // Event method for CreateSwitchExtensionSwitchFeature
        //
        public static bool EventWriteCreateSwitchExtensionSwitchFeature(string FuncName, string Input_Parameters)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEMStarted( FuncName, Input_Parameters);
        }

        //
        // Event method for CreateUniqueVbridgeName
        //
        public static bool EventWriteCreateUniqueVbridgeName(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for CreateUniqueVTNName
        //
        public static bool EventWriteCreateUniqueVTNName(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for CreateUplinkPortProfile
        //
        public static bool EventWriteCreateUplinkPortProfile(string FuncName, string Input_Parameters)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEMStarted( FuncName, Input_Parameters);
        }

        //
        // Event method for CreateUri
        //
        public static bool EventWriteCreateUri(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for CreateVirtualPortProfile
        //
        public static bool EventWriteCreateVirtualPortProfile(string FuncName, string Input_Parameters)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEMStarted( FuncName, Input_Parameters);
        }

        //
        // Event method for CredentialError
        //
        public static bool EventWriteCredentialError(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for DoCmdlet
        //
        public static bool EventWriteDoCmdlet(string Cmdlet, string Message, string Input_parameters)
        {

            return EventProviderVersionTwo.LOG.TemplateParameters_logging( Cmdlet, Message, Input_parameters);
        }

        //
        // Event method for ExtractFabricNetworkDefinition
        //
        public static bool EventWriteExtractFabricNetworkDefinition(string FuncName, string Input_Parameters)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEMStarted( FuncName, Input_Parameters);
        }

        //
        // Event method for ExtractFabricNetworkDefinitionlist
        //
        public static bool EventWriteExtractFabricNetworkDefinitionlist(string FuncName, string Input_Parameters)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEMStarted( FuncName, Input_Parameters);
        }

        //
        // Event method for ExtractSwitchExtensionInfo
        //
        public static bool EventWriteExtractSwitchExtensionInfo(string FuncName, string Input_Parameters)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEMStarted( FuncName, Input_Parameters);
        }

        //
        // Event method for ExtractSwitchExtensionInfolist
        //
        public static bool EventWriteExtractSwitchExtensionInfolist(string FuncName, string Input_Parameters)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEMStarted( FuncName, Input_Parameters);
        }

        //
        // Event method for ExtractsystemInfo
        //
        public static bool EventWriteExtractsystemInfo(string FuncName, string Input_Parameters)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEMStarted( FuncName, Input_Parameters);
        }

        //
        // Event method for ExtractUplinkPortProfile
        //
        public static bool EventWriteExtractUplinkPortProfile(string FuncName, string Input_Parameters)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEMStarted( FuncName, Input_Parameters);
        }

        //
        // Event method for ExtractUplinkPortProfilelist
        //
        public static bool EventWriteExtractUplinkPortProfilelist(string FuncName, string Input_Parameters)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEMStarted( FuncName, Input_Parameters);
        }

        //
        // Event method for ExtractVirtualPortProfile
        //
        public static bool EventWriteExtractVirtualPortProfile(string FuncName, string Input_Parameters)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEMStarted( FuncName, Input_Parameters);
        }

        //
        // Event method for ExtractVirtualPortProfilelist
        //
        public static bool EventWriteExtractVirtualPortProfilelist(string FuncName, string Input_Parameters)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEMStarted( FuncName, Input_Parameters);
        }

        //
        // Event method for ExtractVMNetworkInfo
        //
        public static bool EventWriteExtractVMNetworkInfo(string FuncName, string Input_Parameters)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEMStarted( FuncName, Input_Parameters);
        }

        //
        // Event method for ExtractVMNetworkInfolist
        //
        public static bool EventWriteExtractVMNetworkInfolist(string FuncName, string Input_Parameters)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEMStarted( FuncName, Input_Parameters);
        }

        //
        // Event method for FailedHttpRequestError
        //
        public static bool EventWriteFailedHttpRequestError(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for FoundNoUnusedVlanError
        //
        public static bool EventWriteFoundNoUnusedVlanError(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for FoundNoVlanError
        //
        public static bool EventWriteFoundNoVlanError(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for FoundNoVlanODLError
        //
        public static bool EventWriteFoundNoVlanODLError(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for GetFabricNetworkDefinitionError
        //
        public static bool EventWriteGetFabricNetworkDefinitionError(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for GetVbridgesforVtn
        //
        public static bool EventWriteGetVbridgesforVtn(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for GetVSEMVMNetworkError
        //
        public static bool EventWriteGetVSEMVMNetworkError(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for GetVtn
        //
        public static bool EventWriteGetVtn(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for HttpRequest
        //
        public static bool EventWriteHttpRequest(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for HttpRequestTimeout
        //
        public static bool EventWriteHttpRequestTimeout(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for HttpResponse
        //
        public static bool EventWriteHttpResponse(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for HttpResponseError
        //
        public static bool EventWriteHttpResponseError(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for ProcessCmdletError
        //
        public static bool EventWriteProcessCmdletError(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for ProcessCmdletWarning
        //
        public static bool EventWriteProcessCmdletWarning(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for ProcessFailedVMNetworkError
        //
        public static bool EventWriteProcessFailedVMNetworkError(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for ProcessFailedVMSubNetworkError
        //
        public static bool EventWriteProcessFailedVMSubNetworkError(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for ProcessLibraryError
        //
        public static bool EventWriteProcessLibraryError(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for ProcesOdlLibraryError
        //
        public static bool EventWriteProcesOdlLibraryError(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for RecreateUniqueVTNName
        //
        public static bool EventWriteRecreateUniqueVTNName(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for RegisterVlan
        //
        public static bool EventWriteRegisterVlan(string FuncName, string Input_Parameters)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEMStarted( FuncName, Input_Parameters);
        }

        //
        // Event method for RemoveIpAddressPool
        //
        public static bool EventWriteRemoveIpAddressPool(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for RemoveVMNetwork
        //
        public static bool EventWriteRemoveVMNetwork(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for RetryCountHttpRequest
        //
        public static bool EventWriteRetryCountHttpRequest(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for RetryHttpResponse
        //
        public static bool EventWriteRetryHttpResponse(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for ReturnLibrary
        //
        public static bool EventWriteReturnLibrary(string FuncName, string Input_Parameters)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEMStarted( FuncName, Input_Parameters);
        }

        //
        // Event method for ReturnODLLibrary
        //
        public static bool EventWriteReturnODLLibrary(string FuncName, string Input_Parameters)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEMStarted( FuncName, Input_Parameters);
        }

        //
        // Event method for SelectVlanIdExactMatch
        //
        public static bool EventWriteSelectVlanIdExactMatch(string FuncName, string Input_Parameters)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEMStarted( FuncName, Input_Parameters);
        }

        //
        // Event method for SelectVlanIdWildcardMatch
        //
        public static bool EventWriteSelectVlanIdWildcardMatch(string FuncName, string Input_Parameters)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEMStarted( FuncName, Input_Parameters);
        }

        //
        // Event method for SuccessVmNetwork
        //
        public static bool EventWriteSuccessVmNetwork(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for SupportsVMNetworkProvisioningError
        //
        public static bool EventWriteSupportsVMNetworkProvisioningError(string FuncName, string Input_Parameters)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEMStarted( FuncName, Input_Parameters);
        }

        //
        // Event method for UpdateFabricNetworkDefinition
        //
        public static bool EventWriteUpdateFabricNetworkDefinition(string FuncName, string Input_Parameters)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEMStarted( FuncName, Input_Parameters);
        }

        //
        // Event method for ValidateAddressRangeEndError
        //
        public static bool EventWriteValidateAddressRangeEndError(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for ValidateAddressRangeStartError
        //
        public static bool EventWriteValidateAddressRangeStartError(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for ValidateConnectionObjectError
        //
        public static bool EventWriteValidateConnectionObjectError(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for ValidateConnectionParameterError
        //
        public static bool EventWriteValidateConnectionParameterError(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for ValidateCreateVmNetworkDefinitionForvBr
        //
        public static bool EventWriteValidateCreateVmNetworkDefinitionForvBr(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for ValidateIPAddressPoolError
        //
        public static bool EventWriteValidateIPAddressPoolError(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for ValidateIPAddressSubnetError
        //
        public static bool EventWriteValidateIPAddressSubnetError(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for ValidateNameError
        //
        public static bool EventWriteValidateNameError(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for ValidateNameLengthError
        //
        public static bool EventWriteValidateNameLengthError(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for ValidateNetworkGatewayError
        //
        public static bool EventWriteValidateNetworkGatewayError(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for ValidateRecreateVtnNameError
        //
        public static bool EventWriteValidateRecreateVtnNameError(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for ValidateRecreateVtnNameLength
        //
        public static bool EventWriteValidateRecreateVtnNameLength(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for ValidatevBridgeNameError
        //
        public static bool EventWriteValidatevBridgeNameError(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for ValidatevBridgeNameLengthError
        //
        public static bool EventWriteValidatevBridgeNameLengthError(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for ValidatevmNetDefError
        //
        public static bool EventWriteValidatevmNetDefError(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for ValidateVSEMRepositoryError
        //
        public static bool EventWriteValidateVSEMRepositoryError(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for ValidateConnectionString
        //
        public static bool EventWriteValidateConnectionString(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for ValidateControllerString
        //
        public static bool EventWriteValidateControllerString(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for VerifyConnection
        //
        public static bool EventWriteVerifyConnection(string FuncName, string Input_Parameters)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEMStarted( FuncName, Input_Parameters);
        }

        //
        // Event method for VerifyWebapiVersion
        //
        public static bool EventWriteVerifyWebapiVersion(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for WaitHttpRequest
        //
        public static bool EventWriteWaitHttpRequest(string Method_Name, string Information)
        {


            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for CreateVMsubNetwork
        //
        public static bool EventWriteCreateVMsubNetwork(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for CreateVTNName
        //
        public static bool EventWriteCreateVTNName(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for CreateVtnVMNetwork
        //
        public static bool EventWriteCreateVtnVMNetwork(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for CreateODLData
        //
        public static bool EventWriteCreateODLData(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for ValidateVMNetDefinitionError
        //
        public static bool EventWriteValidateVMNetDefinitionError(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for FailedCmdlet
        //
        public static bool EventWriteFailedCmdlet(string FuncName, string Input_Parameters)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEMStarted( FuncName, Input_Parameters);
        }

        //
        // Event method for ValidateCmdletParameter
        //
        public static bool EventWriteValidateCmdletParameter(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for FailedVTNRemoval
        //
        public static bool EventWriteFailedVTNRemoval(string Method_Name, string Information)
        {

            return  EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for RemoveIPPoolWarning
        //
        public static bool EventWriteRemoveIPPoolWarning(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for GetHNVLogicalNetworkNotFound
        //
        public static bool EventWriteGetHNVLogicalNetworkNotFound(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for GetHNVUplinkPortProfileNotFound
        //
        public static bool EventWriteGetHNVUplinkPortProfileNotFound(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for NotHNVLogicalNetwork
        //
        public static bool EventWriteNotHNVLogicalNetwork(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for ValidateLogicalNetworkError
        //
        public static bool EventWriteValidateLogicalNetworkError(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }

        //
        // Event method for NotHNVVMNetwork
        //
        public static bool EventWriteNotHNVVMNetwork(string Method_Name, string Information)
        {

            return EventProviderVersionTwo.LOG.TemplateVSEM_Processing( Method_Name, Information);
        }
}


}
