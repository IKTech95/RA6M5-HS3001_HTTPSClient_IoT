/***********************************************************************************************************************
 * File Name    : user_app_thread_entry.c
 * Description  : This file contains the User Application code for the AWS HTTPS Client
 ***********************************************************************************************************************/
/***********************************************************************************************************************
 * Copyright (c) 2022 - 2024 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 ***********************************************************************************************************************/

#include <user_app_thread.h>
#include "board_cfg.h"
#include "FreeRTOS_IP.h"
#include "FreeRTOS_IP_Private.h"
#include "FreeRTOS_Sockets.h"
#include "common_utils.h"
#include "littlefs_app.h"
#include "core_http_client.h"
#include "transport_mbedtls_pkcs11.h"
#include "user_app.h"
#include "hs300x_code.h"

#define CKR_ACTION_PROHIBITED  0x0000001BUL
#define CKR_DEVICE_MEMORY  0x00000031UL

/*******************************************************************************************************************//**
 * @addtogroup aws_https_client_ep
 * @{
 **********************************************************************************************************************/

/******************************************************************************
 Macro definitions
 ******************************************************************************/

/******************************************************************************
 Exported global functions (to be accessed by other files)
 ******************************************************************************/
extern TaskHandle_t user_app_thread;

/******************************************************************************
 Exported global variables
 ******************************************************************************/
ping_data_t ping_data = { RESET_VALUE, RESET_VALUE, RESET_VALUE };
IPV4Parameters_t xNd = {RESET_VALUE, RESET_VALUE, RESET_VALUE, {RESET_VALUE, RESET_VALUE}, RESET_VALUE, RESET_VALUE};
uint32_t dhcp_in_use = RESET_VALUE;

/* Variables to store H3001 readings */
struct hs3001_raw_data rawData;
struct sensor_data hs300x_data;
float temp  = RESET_VALUE;

/* Domain for the DNS Host lookup is used in this Example Project.
 * The project can be built with different *domain_name to validate the DNS client
 */
char *domain_name = HTTPS_HOST_ADDRESS;

/* IP address of the PC or any Device on the LAN/WAN where the Ping request is sent.
 * Note: Users needs to change this according to the LAN settings of your Test PC or device
 * when running this project.
 */
char remote_ip_address[] = HTTPS_TEST_PING_IP;

/* DHCP populates these IP address, Sub net mask and Gateway Address. So start with this is zeroed out values
 * The MAC address is Test MAC address.
 */
uint8_t ucMACAddress[6] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x54 };
uint8_t ucIPAddress[4] = { 192, 168, 31, 112 };
uint8_t ucNetMask[4] = { 255, 255, 255, 0 };
uint8_t ucGatewayAddress[4] = {  192, 168, 31, 1 };
uint8_t ucDNSServerAddress[4] = {  192, 168, 31, 1 };


/******************************************************************************
 Private global variables and functions
 ******************************************************************************/
/* Extracted ID from GET request to be updated in HTTPS_PUT_POST_API */
char id[SIZE_32] = { RESET_VALUE };
/* Flag bit for PUT request. if User calls directly without processed GET request */
bool is_get_called = false;
bool ID_alive=false;
struct NetworkContext
{
    TlsTransportParams_t * pParams;
};

/*Res and Recv buffers for header of HTTP request*/
uint8_t resUserBuffer[USER_BUFF]={RESET_VALUE};
uint8_t reqUserBuffer[USER_BUFF]={RESET_VALUE};

/*******************************************************************************************************************//**
 * @brief      This is the User Thread for the EP.
 * @param[in]  Thread specific parameters
 * @retval     None
 **********************************************************************************************************************/
void user_app_thread_entry(void *pvParameters)
{
    fsp_err_t err = FSP_SUCCESS;
    BaseType_t status = pdFALSE;
    HTTPStatus_t httpsClientStatus = HTTPSuccess;
    NetworkContext_t xNetworkContext={RESET_VALUE};
    TransportInterface_t xTransportInterface={RESET_VALUE};
    unsigned char rByte[BUFFER_SIZE_DOWN] =  { RESET_VALUE };
    user_input_t user_input = RESET_VALUE;

    /* #HTTPClient_InitializeRequestHeaders. */
    HTTPRequestInfo_t xRequestInfo={RESET_VALUE};
    /* Represents a response returned from an HTTP server. */
    HTTPResponse_t xResponse={RESET_VALUE};
    /* Represents header data that will be sent in an HTTP request. */
    HTTPRequestHeaders_t xRequestHeaders={RESET_VALUE};


    FSP_PARAMETER_NOT_USED(pvParameters);

    /*Print Project info*/
    APP_PRINT(PROJECT_INFO);

    /*Initialize littlefs port*/
    err = hal_littlefs_init ();
    if (err != FSP_SUCCESS)
    {
        APP_PRINT("** Failed in hal_littlefs_init () function ** \r\n");
        __BKPT(0);
    }

    /*Initialize HS3001 sensor*/
    err = i2c_masterInit(0x44);
    if(err != FSP_SUCCESS)
    {
        APP_PRINT("** Failed in i2c_master_init () function to init H3001 **\r\n");
        hal_littlefs_deinit ();
        __BKPT(0);
    }

    /*Start Measurement Process-Wake up sensor by sending one byte 0x00*/
    err = start_measurement();
    if(err != FSP_SUCCESS)
    {
        APP_PRINT("** Failed in start_measurement() function to prepare HS3001 **\r\n");
        hal_littlefs_deinit ();
        __BKPT(0);
    }

    /* Initialize the crypto hardware acceleration. */
    /* Initialize mbedtls. */
    err = mbedtls_platform_setup (NULL);
    if (FSP_SUCCESS != err)
    {
        APP_PRINT("** Failed in mbedtls_platform_setup() function ** \r\n");
        hal_littlefs_deinit ();
        i2_masterDeinit();
        __BKPT(0);
    }
    else
    {
        APP_PRINT("\r\nmbedtls_platform setup successful\r\n");
    }

    /*Connect board to network*/
    status = getIP(ucIPAddress,ucNetMask,ucGatewayAddress,ucDNSServerAddress,ucMACAddress);
    if (status != pdTRUE)
    {
        APP_PRINT("Failed to connect to network \r\n");
        __BKPT(0);
    }

    /*Perform ping requests*/
    pingIP((char*)remote_ip_address);

    /*Perform device provisioning using specified TLS client credentials*/
    status = provision_alt_key ();
    if (status != pdPASS)
    {
        APP_PRINT("\r\nFailed in network_init() function\r\n");
        hal_littlefs_deinit ();
        mbedtls_platform_teardown (NULL);
        __BKPT(0);
    }

    /* Initialize HTTPS client with presigned URL */
    httpsClientStatus = connect_aws_https_client (&xNetworkContext);
    /* Handle_error */
    if (HTTPSuccess != httpsClientStatus)
    {
        APP_PRINT("\r\nFailed in server connection establishment");
        hal_littlefs_deinit ();
        mbedtls_platform_teardown (NULL);
        __BKPT(0);
    }
    else
    {
        xTransportInterface.pNetworkContext = &xNetworkContext;
        xTransportInterface.send = TLS_FreeRTOS_send;
        xTransportInterface.recv = TLS_FreeRTOS_recv;
    }

    APP_PRINT("\r\nClient successfully connected to adfruit.io server \r\n");

    /*Print Menu Options*/
    APP_PRINT(PRINT_MENU);

    while (true)
    {
        if (APP_CHECK_DATA)
        {
            APP_READ(rByte);
            user_input = (uint8_t) atoi ((char*)rByte);
            switch (user_input)
            {
                case POST:
                {
                    APP_PRINT("\r\nPreparing to get measurement from HS3001 \r\n");
                    //TO DO add the temperature reading
                    err = start_measurement();
                    if(err != FSP_SUCCESS)
                    {
                        __BKPT(0);
                    }
                    /*Wait to stabilize the sensor*/
                    R_BSP_SoftwareDelay(40,BSP_DELAY_UNITS_MILLISECONDS);

                    /*Read raw data*/
                    err = get_measurement(&rawData);
                    if(err != FSP_SUCCESS)
                    {
                        __BKPT(0);
                    }

                    /*Calculate humidity and temperature*/
                    calculateData (&hs300x_data,&rawData);
                    temp = convertTemperaturetoFloat();

                    APP_PRINT("\r\nProcessing POST Request\r\n");
                    /* Initialize all HTTP Client library API structs to 0. */
                    ( void ) memset( &xRequestInfo, 0, sizeof( xRequestInfo ) );
                    ( void ) memset( &xResponse, 0, sizeof( xResponse ) );
                    ( void ) memset( &xRequestHeaders, 0, sizeof( xRequestHeaders ) );
                    /* Initialize the request object. */
                    xRequestInfo.pPath = HTTPS_PUT_POST_API;
                    xRequestInfo.pathLen = strlen (HTTPS_PUT_POST_API);
                    xRequestInfo.pHost = HTTPS_HOST_ADDRESS;
                    xRequestInfo.hostLen = strlen (HTTPS_HOST_ADDRESS);
                    xRequestInfo.pMethod = HTTP_METHOD_POST;
                    xRequestInfo.methodLen = strlen(HTTP_METHOD_POST);

                    /* Set "Connection" HTTP header to "keep-alive" so that multiple requests
                     * can be sent over the same established TCP connection. */
                    xRequestInfo.reqFlags = HTTP_REQUEST_KEEP_ALIVE_FLAG;

                    /* Set the buffer used for storing request headers. */
                    xRequestHeaders.pBuffer = reqUserBuffer;
                    xRequestHeaders.bufferLen = sizeof(reqUserBuffer);
                    memset( xRequestHeaders.pBuffer, 0, xRequestHeaders.bufferLen );

                    httpsClientStatus = HTTPClient_InitializeRequestHeaders( &xRequestHeaders,
                                                                             &xRequestInfo );
                    /* Add header */
                    if( httpsClientStatus == HTTPSuccess )
                    {
                        httpsClientStatus = add_header(&xRequestHeaders);
                    }
                    else
                    {
                        APP_PRINT("Failed to initialize HTTP request headers: Error=%s. \r\n",
                                  HTTPClient_strerror( httpsClientStatus ) );
                    }
                    char upload_str[SIZE_64];
                    snprintf (upload_str, SIZE_64, "{\"datum\":{\"value\":\"%.02f\"}}", temp); //formating into string to send in JSON format
                    uint32_t length_upload;
                    length_upload = strlen ((const char*) upload_str);

                    xResponse.pBuffer = resUserBuffer;
                    xResponse.bufferLen = sizeof(resUserBuffer);
                    memset( xResponse.pBuffer, 0, xResponse.bufferLen );

                    if( httpsClientStatus == HTTPSuccess )
                    {
                        httpsClientStatus = HTTPClient_Send( &xTransportInterface,
                                                             &xRequestHeaders,
                                                             (const uint8_t *)upload_str,
                                                             length_upload,
                                                             &xResponse,
                                                             0 );
                    }


                    if (HTTPSuccess != httpsClientStatus)
                    {
                        APP_ERR_PRINT("** Failed in POST Request ** \r\n");
                    }
                    else
                    {
                        APP_PRINT("Received data using POST Request = %s\n", xResponse.pBody);
                    }
                    break;
                }

                case GET:
                {
                    APP_PRINT("\r\nProcessing Get Request\r\n");
                    /* Initialize all HTTP Client library API structs to 0. */
                    ( void ) memset( &xRequestInfo, 0, sizeof( xRequestInfo ) );
                    ( void ) memset( &xResponse, 0, sizeof( xResponse ) );
                    ( void ) memset( &xRequestHeaders, 0, sizeof( xRequestHeaders ) );
                    /* Initialize the request object. */
                    xRequestInfo.pPath = HTTPS_GET_API;
                    xRequestInfo.pathLen = strlen (HTTPS_GET_API);
                    xRequestInfo.pHost = HTTPS_HOST_ADDRESS;
                    xRequestInfo.hostLen = strlen (HTTPS_HOST_ADDRESS);
                    xRequestInfo.pMethod = HTTP_METHOD_GET;
                    xRequestInfo.methodLen = strlen(HTTP_METHOD_GET);

                    /* Set "Connection" HTTP header to "keep-alive" so that multiple requests
                     * can be sent over the same established TCP connection. */
                    xRequestInfo.reqFlags = HTTP_REQUEST_KEEP_ALIVE_FLAG;

                    /* Set the buffer used for storing request headers. */

                    xRequestHeaders.pBuffer = reqUserBuffer;
                    xRequestHeaders.bufferLen = sizeof(reqUserBuffer);
                    memset( xRequestHeaders.pBuffer, 0, xRequestHeaders.bufferLen );

                    httpsClientStatus = HTTPClient_InitializeRequestHeaders( &xRequestHeaders,
                                                                             &xRequestInfo );
                    /* Add header */
                    if( httpsClientStatus == HTTPSuccess )
                    {
                        httpsClientStatus = add_header(&xRequestHeaders);
                    }
                    else
                    {
                        APP_PRINT("Failed to initialize HTTP request headers: Error=%s. \r\n",
                                  HTTPClient_strerror( httpsClientStatus ) );
                    }

                    xResponse.pBuffer = resUserBuffer;
                    xResponse.bufferLen = sizeof(resUserBuffer);
                    memset( xResponse.pBuffer, 0, xResponse.bufferLen );

                    if( httpsClientStatus == HTTPSuccess )
                    {
                        httpsClientStatus = HTTPClient_Send( &xTransportInterface,
                                                             &xRequestHeaders,
                                                             NULL,
                                                             0,
                                                             &xResponse,
                                                             0 );
                    }

                    if(HTTPSuccess != httpsClientStatus)
                    {
                        APP_ERR_PRINT("** Failed in GET Request ** \r\n");
                    }
                    else
                    {
                        APP_PRINT("Received data using GET Request = %s\n", xResponse.pBody);
                        strncpy (&id[INDEX_ZERO], (char *)&xResponse.pBody[ID_START_INDEX], ID_LEN);
                        /* Fetch the feed id from the response body which to be update in HTTPS_PUT_POST_API */
                        /* SynchronousrResponse body starts with the string in the format like [{\"id\":\"0ENQG7RYQA40W17G2A2SFH8E9Q\",\"...\"}]".
                         *  So, to fetch the ID_KEY other portion of string is avoided*/
                        is_get_called = true;   //setting the flag to avoid GET call in the PUT request
                    }
                    break;
                }
                default:
                    APP_PRINT("Incorrect option. Choose either 1:POST request or 2: GET request \r\n");
                    break;
            }
            if (httpsClientStatus != HTTPSuccess)
            {
                mbedtls_platform_teardown (NULL);
                hal_littlefs_deinit ();
                APP_ERR_TRAP(httpsClientStatus);
            }
            /* Repeat the menu to display for user selection */
            APP_PRINT(PRINT_MENU);
        }
        vTaskDelay (100);
    }

}



/*Function so that the board will get IP address and
 * connect to network */
BaseType_t getIP(const uint8_t IPAddress[NETWORK_SETTINGS_LENGTH_BYTES],
        const uint8_t NetMask[NETWORK_SETTINGS_LENGTH_BYTES],
        const uint8_t GatewayAddress[NETWORK_SETTINGS_LENGTH_BYTES],
        const uint8_t DNSServerAddress[NETWORK_SETTINGS_LENGTH_BYTES],
        const uint8_t MACAddress[MAC_ADDRESS_LENGTH_BYTES])
{
    BaseType_t status = pdFALSE;
    BaseType_t bt_status = pdFALSE;
    uint32_t ip_status = 0U;

    status = FreeRTOS_IPInit (IPAddress, NetMask, GatewayAddress, DNSServerAddress, MACAddress);
    if (pdFALSE == status)
    {
        APP_PRINT("FreeRTOS_IPInit failed \r\n");
        hal_littlefs_deinit ();
        mbedtls_platform_teardown (NULL);
        return status;
    }

    APP_PRINT("\r\nWaiting for network link up... \r\n");
    bt_status = xTaskNotifyWait(pdFALSE, pdFALSE, &ip_status, portMAX_DELAY);
    if (pdTRUE != bt_status)
    {
        APP_ERR_PRINT("xTaskNotifyWait Failed \r\n");
        hal_littlefs_deinit ();
        mbedtls_platform_teardown (NULL);
        return bt_status;
    }
    APP_PRINT("\r\nObtained successfully IP and connected to network... \r\n");
    print_ipconfig();
    return status;
}

/*Function to print the IP configurations*/
void print_ipconfig(void)
{
    if (dhcp_in_use)
    {
        ucNetMask[3] = (uint8_t) ((xNd.ulNetMask & 0xFF000000) >> 24);
        ucNetMask[2] = (uint8_t) ((xNd.ulNetMask & 0x00FF0000) >> 16);
        ucNetMask[1] = (uint8_t) ((xNd.ulNetMask & 0x0000FF00) >> 8);
        ucNetMask[0] = (uint8_t) (xNd.ulNetMask & 0x000000FF);

        ucGatewayAddress[3] = (uint8_t) ((xNd.ulGatewayAddress & 0xFF000000) >> 24);
        ucGatewayAddress[2] = (uint8_t) ((xNd.ulGatewayAddress & 0x00FF0000) >> 16);
        ucGatewayAddress[1] = (uint8_t) ((xNd.ulGatewayAddress & 0x0000FF00) >> 8);
        ucGatewayAddress[0] = (uint8_t) (xNd.ulGatewayAddress & 0x000000FF);

        ucDNSServerAddress[3] = (uint8_t) ((xNd.ulDNSServerAddresses[0] & 0xFF000000) >> 24);
        ucDNSServerAddress[2] = (uint8_t) ((xNd.ulDNSServerAddresses[0] & 0x00FF0000) >> 16);
        ucDNSServerAddress[1] = (uint8_t) ((xNd.ulDNSServerAddresses[0] & 0x0000FF00) >> 8);
        ucDNSServerAddress[0] = (uint8_t) (xNd.ulDNSServerAddresses[0] & 0x000000FF);

        ucIPAddress[3] = (uint8_t) ((xNd.ulIPAddress & 0xFF000000) >> 24);
        ucIPAddress[2] = (uint8_t) ((xNd.ulIPAddress & 0x00FF0000) >> 16);
        ucIPAddress[1] = (uint8_t) ((xNd.ulIPAddress & 0x0000FF00) >> 8);
        ucIPAddress[0] = (uint8_t) (xNd.ulIPAddress & 0x000000FF);
    }
    APP_PRINT("\r\nEthernet adapter for Renesas "KIT_NAME":\r\n");
    APP_PRINT("\tDescription                       : Renesas "KIT_NAME" Ethernet\r\n");
    APP_PRINT("\tPhysical Address                  : %02x-%02x-%02x-%02x-%02x-%02x\r\n",
            ucMACAddress[0], ucMACAddress[1], ucMACAddress[2], ucMACAddress[3], ucMACAddress[4], ucMACAddress[5]);
    APP_PRINT("\tDHCP Enabled                      : %s\r\n", dhcp_in_use ? "Yes" : "No");
    APP_PRINT("\tIPv4 Address                      : %d.%d.%d.%d\r\n", ucIPAddress[0], ucIPAddress[1], ucIPAddress[2],
            ucIPAddress[3]);
    APP_PRINT("\tSubnet Mask                       : %d.%d.%d.%d\r\n", ucNetMask[0], ucNetMask[1], ucNetMask[2],
            ucNetMask[3]);
    APP_PRINT("\tDefault Gateway                   : %d.%d.%d.%d\r\n", ucGatewayAddress[0], ucGatewayAddress[1],
            ucGatewayAddress[2], ucGatewayAddress[3]);
    APP_PRINT("\tDNS Servers                       : %d.%d.%d.%d\r\n", ucDNSServerAddress[0], ucDNSServerAddress[1],
            ucDNSServerAddress[2], ucDNSServerAddress[3]);
}


/*******************************************************************************************************************//**
 * @brief      This is the User Hook for the DHCP Response. xApplicationDHCPHook() is called by DHCP Client Code when DHCP
 *             handshake messages are exchanged from the Server.
 * @param[in]  Different Phases of DHCP Phases and the Offered IP Address
 * @retval     Returns DHCP Answers.
 **********************************************************************************************************************/
eDHCPCallbackAnswer_t xApplicationDHCPHook(eDHCPCallbackPhase_t eDHCPPhase, uint32_t lulIPAddress)
{
    eDHCPCallbackAnswer_t eReturn = eDHCPContinue;
    /*
     * This hook is called in a couple of places during the DHCP process, as identified by the eDHCPPhase parameter.
     */
    switch (eDHCPPhase)
    {
        case eDHCPPhasePreDiscover:
            /*
             *  A DHCP discovery is about to be sent out.  eDHCPContinue is returned to allow the discovery to go out.
             *  If eDHCPUseDefaults had been returned instead then the DHCP process would be stopped and the statically
             *  configured IP address would be used.
             *  If eDHCPStopNoChanges had been returned instead then the DHCP process would be stopped and whatever the
             *  current network configuration was would continue to be used.
             */
            break;

        case eDHCPPhasePreRequest:
            /* An offer has been received from the DHCP server, and the offered IP address is passed in the lulIPAddress
             * parameter.
             */

            /*
             * The sub-domains don't match, so continue with the DHCP process so the offered IP address is used.
             */
            /* Update the Structure, the DHCP state Machine is not updating this */
            xNd.ulIPAddress = lulIPAddress;
            dhcp_in_use = 1;
            updateDhcpResponseToUsr ();
            break;

        default:
            /*
             * Cannot be reached, but set eReturn to prevent compiler warnings where compilers are disposed to generating one.
             */
            break;
    }

    return eReturn;
}

/*******************************************************************************************************************//**
 * @brief      Update the DHCP info to the User data structure.
 * @param[in]  None
 * @retval     None
 **********************************************************************************************************************/
void updateDhcpResponseToUsr(void)
{
    if (dhcp_in_use)
    {
        xNd.ulNetMask = FreeRTOS_GetNetmask();
        xNd.ulGatewayAddress = FreeRTOS_GetGatewayAddress();
        xNd.ulDNSServerAddresses[0] = FreeRTOS_GetDNSServerAddress();
    }
}

/********************************************************************************************************************//**
 * @brief      DHCP Hook function to populate the user defined Host name for the Kit.
 * @param[in]  None
 * @retval     Hostname
 **********************************************************************************************************************/
#if( ipconfigDHCP_REGISTER_HOSTNAME == true )
const char* pcApplicationHostnameHook(void)
{
    return KIT_NAME;
}
#endif

/*Function to perform ping requests*/
void pingIP(const char *ip_address)
{
    uint32_t usrPingCount = 0U;
    BaseType_t status =pdFALSE;
    while (usrPingCount < USR_PING_COUNT)
    {
        status = vSendPing (ip_address);
        if (pdFAIL != status)
        {
            ping_data.sent++;
        }
        else
        {
            ping_data.lost++;
        }
        usrPingCount++;
        /* Add some delay between pings */
        vTaskDelay (PING_DELAY);
    }
    print_pingResult();
}



BaseType_t vSendPing(const char *pcIPAddress)
{
    uint32_t ulIPAddress = RESET_VALUE;
    /*
     * The pcIPAddress parameter holds the destination IP address as a string in
     * decimal dot notation (for example, “192.168.0.200”). Convert the string into
     * the required 32-bit format.
     */
    ulIPAddress = FreeRTOS_inet_addr (pcIPAddress);

    /*
     * Send a ping request containing 8 data bytes.  Wait (in the Blocked state) a
     * maximum of 100ms for a network buffer into which the generated ping request
     * can be written and sent.
     */
    return (FreeRTOS_SendPingRequest (ulIPAddress, 8, 100 / portTICK_PERIOD_MS));
}

/*Print the ping results*/
void print_pingResult(void)
{
    APP_PRINT("\r\nPing Statistics for %s :\r\n", (char* )remote_ip_address);
    APP_PRINT("\r\nPackets: Sent  = %02d, Received = %02d, Lost = %02d \r\n", ping_data.sent, ping_data.received, ping_data.lost);
}

/*******************************************************************************************************************//**
 * @brief      User Hook for the Ping Reply. vApplicationPingReplyHook() is called by the TCP/IP
 *             stack when the stack receives a ping reply.
 * @param[in]  Ping reply status and Identifier
 * @retval     None
 **********************************************************************************************************************/
void vApplicationPingReplyHook(ePingReplyStatus_t eStatus, uint16_t usIdentifier)
{
    (void) usIdentifier;

    switch (eStatus)
    {
        /* A valid ping reply has been received */
        case eSuccess:
            ping_data.received++;
            break;
            /* A reply was received but it was not valid. */
        case eInvalidData:
        default:
            ping_data.lost++;
            break;
    }
}

/*******************************************************************************************************************//**
 * @brief      Network event callback. Indicates the Network event. Added here to avoid the build errors
 * @param[in]  None
 * @retval     Hostname
 **********************************************************************************************************************/
#if ( ipconfigUSE_NETWORK_EVENT_HOOK == 1 )
void vApplicationIPNetworkEventHook (eIPCallbackEvent_t eNetworkEvent)
{
    if((eNetworkUp == eNetworkEvent) && (dhcp_in_use))
    {
        uint32_t lulIPAddress, lulNetMask, lulGatewayAddress, lulDNSServerAddress;
        int8_t lcBuffer[BUFF_SIZE];

        /* Signal application the network is UP */
        xTaskNotifyFromISR(user_app_thread, eNetworkUp, eSetBits, NULL);

        /* The network is up and configured.  Print out the configuration
        obtained from the DHCP server. */
        FreeRTOS_GetAddressConfiguration(&lulIPAddress,
                &lulNetMask,
                &lulGatewayAddress,
                &lulDNSServerAddress);

        /* Convert the IP address to a string then print it out. */
        FreeRTOS_inet_ntoa(lulIPAddress, (char *)lcBuffer);

        /* Convert the net mask to a string then print it out. */
        FreeRTOS_inet_ntoa(lulNetMask, (char *)lcBuffer);

        /* Convert the IP address of the gateway to a string then print it out. */
        FreeRTOS_inet_ntoa(lulGatewayAddress, (char *)lcBuffer);

        /* Convert the IP address of the DNS server to a string then print it out. */
        FreeRTOS_inet_ntoa(lulDNSServerAddress, (char *)lcBuffer);
    }
}
#endif

/* provision_alt_key function provides the device with client certificate and client key*/
BaseType_t provision_alt_key(void)
{
    BaseType_t status = pdPASS;
    ProvisioningParams_t params = {RESET_VALUE};
    CK_RV xResult = CKR_OK;
    /* Provision the device. */
    params.pucClientPrivateKey       = (uint8_t *) CLIENT_KEY_PEM;
    params.pucClientCertificate      = (uint8_t *) CLIENT_CERTIFICATE_PEM;
    params.ulClientPrivateKeyLength  = 1 + strlen((const char *) params.pucClientPrivateKey);
    params.ulClientCertificateLength = 1 + strlen((const char *) params.pucClientCertificate);
    params.pucJITPCertificate        = NULL;
    params.ulJITPCertificateLength   = RESET_VALUE;


    xResult = vAlternateKeyProvisioning(&params);
    if (CKR_OK != xResult)
    {
        APP_ERR_PRINT("\r\nFailed in vAlternateKeyProvisioning() function ");
        return (BaseType_t) xResult;
    }

    APP_PRINT("\r\nSuccessfully provisioned the device with client certificate and client key ");
    return status;
}

/*Connects to the server with all required connection configuration settings*/
HTTPStatus_t connect_aws_https_client(NetworkContext_t *NetworkContext)
{
    HTTPStatus_t httpsClientStatus = HTTPSuccess;
    TlsTransportStatus_t TCP_connect_status = TLS_TRANSPORT_SUCCESS;
    /* The current attempt in the number of connection tries. */
    uint32_t connAttempt = RESET_VALUE;
    NetworkCredentials_t connConfig = { RESET_VALUE };
    TlsTransportParams_t xPlaintextTransportParams={ RESET_VALUE };
    assert( NetworkContext != NULL );

    ( void ) memset( &connConfig, 0U, sizeof( NetworkCredentials_t ) );
    ( void ) memset( NetworkContext, 0U, sizeof( NetworkContext_t ) );
    NetworkContext->pParams=&xPlaintextTransportParams;

    /* Set the connection configurations. */
    connConfig.disableSni = pdFALSE;
    connConfig.pRootCa = (const unsigned char *) HTTPS_TRUSTED_ROOT_CA;
    connConfig.rootCaSize = sizeof(HTTPS_TRUSTED_ROOT_CA);
    connConfig.pUserName = NULL;
    connConfig.userNameSize = 0;
    connConfig.pPassword = NULL;
    connConfig.passwordSize = 0;
    connConfig.pClientCertLabel = pkcs11configLABEL_DEVICE_CERTIFICATE_FOR_TLS;
    connConfig.pPrivateKeyLabel = pkcs11configLABEL_DEVICE_PRIVATE_KEY_FOR_TLS;
    connConfig.pAlpnProtos=NULL;

    /* Connect to server. */
    for (connAttempt = 1; connAttempt <= HTTPS_CONNECTION_NUM_RETRY; connAttempt++)
    {
        TCP_connect_status = TLS_FreeRTOS_Connect (NetworkContext,HTTPS_HOST_ADDRESS,HTTPS_PORT,&connConfig,SOCKET_SEND_RECV_TIME_OUT_MS,SOCKET_SEND_RECV_TIME_OUT_MS);

        if ((TCP_connect_status != TLS_TRANSPORT_SUCCESS) && (connAttempt < HTTPS_CONNECTION_NUM_RETRY))
        {
            APP_ERR_PRINT("Failed to connect the server, retrying after 3000 ms.\r\n");
            vTaskDelay(3000);
            continue;
        }
        else
        {
            break;
        }
    }
    if ( TLS_TRANSPORT_SUCCESS != TCP_connect_status )
    {
        APP_PRINT("Unable to connect the server. Error code: %d.\r\n", TCP_connect_status);
        httpsClientStatus = HTTPNetworkError;
        return httpsClientStatus;
    }

    APP_PRINT("\r\nConnected to the server\r\n");
    return httpsClientStatus;
}

/* @brief      This function adds the header for https request in JSON format.
 *             User has to update their Active Key generated from io.adafruit.com site in the ACTIVE_KEY macro.
 *
 * @param[in]  pRequestHeaders              request handle should be updated according to the GET, PUT, POST methods.
 * @retval     HTTPSuccess                  Upon successful client Initialization.
 * @retval     Any other Error Code         Upon unsuccessful client Initialization.
 **********************************************************************************************************************/
HTTPStatus_t add_header (HTTPRequestHeaders_t * pRequestHeaders)
{
    HTTPStatus_t Status = HTTPSuccess;
    configASSERT(pRequestHeaders != NULL);
    Status = HTTPClient_AddHeader (pRequestHeaders, "Content-Type", strlen ("Content-Type"),
                                   "application/json", strlen ("application/json"));
    if (Status == HTTPSuccess)
    {
        Status = HTTPClient_AddHeader (pRequestHeaders, "X-AIO-Key", strlen ("X-AIO-Key"), ACTIVE_KEY, strlen (ACTIVE_KEY));
        if (Status != HTTPSuccess)
        {
            APP_ERR_PRINT("An error occurred at adding Active Key in HTTPClient_AddHeader() with error code: Error=%s. \r\n",
                          HTTPClient_strerror( Status ) );
        }
    }
    else
    {
        APP_ERR_PRINT("An error occurred at adding json format in HTTPClient_AddHeader() with error code: Error=%s. \r\n",
                      HTTPClient_strerror( Status ));
    }
    return Status;
}

float convertTemperaturetoFloat(void)
{
    float temperature = 0.0;
    int16_t tempArray[2] = {hs300x_data.temperature_data.integer_part ,hs300x_data.temperature_data.decimal_part};
    temperature = tempArray[0] + tempArray[1] / 100.0f;
    return temperature;
}
