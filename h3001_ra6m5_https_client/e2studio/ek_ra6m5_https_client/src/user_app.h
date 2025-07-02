/***********************************************************************************************************************
 * File Name    : user_app.h
 * Description  : Contains macros, data structures and functions used  in the Application
 ***********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2022 - 2024 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef USER_APP_H_
#define USER_APP_H_

#include "FreeRTOS_DHCP.h"


/******************************************************************************
 Macro definitions
 ******************************************************************************/

/**
 *  User Configurable Settings
 **/
#define NETWORK_SETTINGS_LENGTH_BYTES (4U)
#define MAC_ADDRESS_LENGTH_BYTES      (6U)

/** User has to update according to their LAN/WAN test setup for testing of ping IP address **/
#define HTTPS_TEST_PING_IP                       "8.8.8.8"

/** HOST Address **/
#define HTTPS_HOST_ADDRESS                       "io.adafruit.com"

/**
 *  @brief User has to be update according to their credentials viz., {user_name} and {feed_key} in the respective
 *  GET, PUT and POST URLs in the specified formats.
 *  For more information of obtaining user credentials can follow the steps provided in mark down file.
 **/

/** @brief To get the most recent value. Get API from GET url (https://ioadafruit.com/api/v2/{username}/feeds/{feed_key}/data?limit=1): /api/v2/{username}/feeds/{feed_key}/data?limit=1 **/
#define HTTPS_GET_API   "/api/v2/user1995/feeds/temperature/data?limit=1"

/** @brief HTTPS_PUT_POST_API can be used in PUT and POST methods.
 *  PUT method will update data point at requested <id>.
 *  PUT url: https://ioadafruit.com/api/v2/{username}/feeds/{feed_key}/data/{id} , where the <id> will append to the url in the process of
 *  PUT request, once it fetch from the GET request.
 *  API from PUT url: /api/v2/{username}/feeds/{feed_key}/data/{id}
 *  POST method will send new data to the server.
 *  POST url:https://ioadafruit.com/api/v2/{username}/feeds/{feed_key}/data/
 *  API from POST url: /api/v2/{username}/feeds/{feed_key}/data/
 **/
#define HTTPS_PUT_POST_API    "/api/v2/user1995/feeds/temperature/data/"

/** @brief User has to update their generated active key from the io.adafruit.com server. */
#define ACTIVE_KEY                             "aio_gMnp73O9HoPsBbUaArGYjivhBABq"



/* TLS port for HTTPS. */
#define HTTPS_PORT    ( ( uint16_t ) 443U )


/* Number of times to retry the HTTPS connection. A connection is only attempted again if
 * TLS_TRANSPORT_SUCCESS is not returned from TLS_FreeRTOS_Connect(). This indicates an error in the network
 * layer.
 */
#define HTTPS_CONNECTION_NUM_RETRY              ( ( uint32_t ) 5 )

#define SOCKET_SEND_RECV_TIME_OUT_MS            ( ( uint32_t ) 10000 )


/* Extracted ID from GET request to be updated in HTTPS_PUT_POST_API */

#define ID_LEN                                          ( (size_t) 26 )
#define ID_START_INDEX                                  (8)
#define INDEX_ZERO                                      (0)
#define URL_SIZE                                        (128)
#define USER_BUFF                                       (2048)

/**
 *  Client certificate to be updated by the user by following the process specified in the mark down file
 **/


 #define CLIENT_CERTIFICATE_PEM                                                  \
"-----BEGIN CERTIFICATE-----\n" \
"MIID9zCCAt+gAwIBAgIUEJWi4aQTJ79x7GsCX4imnMyyoOYwDQYJKoZIhvcNAQEL\n" \
"BQAwgYoxCzAJBgNVBAYTAlZOMQswCQYDVQQIDAJTRzEPMA0GA1UEBwwGU2FpZ29u\n" \
"MQwwCgYDVQQKDANSVkMxDDAKBgNVBAsMA3NzMjEYMBYGA1UEAwwPaW8uYWRhZnJ1\n" \
"aXQuY29tMScwJQYJKoZIhvcNAQkBFhhjYW5oLnRyYW4ueGpAcmVuZXNhcy5jb20w\n" \
"HhcNMjMwMTA5MDk0NjQxWhcNMjQwMTA5MDk0NjQxWjCBijELMAkGA1UEBhMCVk4x\n" \
"CzAJBgNVBAgMAlNHMQ8wDQYDVQQHDAZTYWlnb24xDDAKBgNVBAoMA1JWQzEMMAoG\n" \
"A1UECwwDc3MyMRgwFgYDVQQDDA9pby5hZGFmcnVpdC5jb20xJzAlBgkqhkiG9w0B\n" \
"CQEWGGNhbmgudHJhbi54akByZW5lc2FzLmNvbTCCASIwDQYJKoZIhvcNAQEBBQAD\n" \
"ggEPADCCAQoCggEBANHjj+OUKVYGDYbd1N2dcHZy5aeMpKzRfED8+MlQsoXJBwP0\n" \
"6Xl8485Klo19qrFGD+NsiGylSdF5lg87iICxkCShTPyYEB5gN29wVIzgPv8HiKLa\n" \
"Zm66BF2yTcXRbFj36BplqsTq36hiqoJeTaTXnbjGn3Mf/fdb/UIoy0zgCZzXP9Yo\n" \
"2DlEacp0L+ORisH+J5I8cP4Q8eD04tHr3rSp5pR9SOAh8tIeWxm6VD1/rK8IFFN8\n" \
"F9Mv/S+p75hPEwrPlhV5XmR9BSFD2Yakc6fbiECnRtyUBgMpKinK+XC98EI3iWh+\n" \
"rNJONNSRyUMsKXbIpK+KQB80IE/+j3PEPyS7G0ECAwEAAaNTMFEwHQYDVR0OBBYE\n" \
"FAeoe9a6wgYcmHfXZU0HzuSmOLq7MB8GA1UdIwQYMBaAFAeoe9a6wgYcmHfXZU0H\n" \
"zuSmOLq7MA8GA1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQELBQADggEBAFxzzHeK\n" \
"jp4RXUKmXiVnR2XyXVV5Ev5c05OiKd7PTRynhzpbPAoCbiM902RD83ZBA5EzhvK1\n" \
"cs6/7E8O65PQJjmFKDpfLV7mG+iRzf5+0vn6U4oDMjSpp2mDVdQi8Si4DHiyOkXZ\n" \
"w/LlDQ0UwVQDJ5zMAAJw9Yrz6Tltkpi5xMK0fnrW8LMyNpJj40Bic7trWu7TTW+w\n" \
"ls0g/AR7JxlOvAIFdA/VixcalaonTjISD/rm+sPzNNPwC2gzxTyq+1P9uHVGIe1m\n" \
"I+GA6PmFG28G6TwL81knzYzQppZEwatE0ripTWZ9t4hdsD7tmnZh2y+sZ+BvjTdV\n" \
"JxlM3y7ATO95Tp4=\n" \
"-----END CERTIFICATE-----\n"


/**
 *  client key to be updated by the user
 **/
#define CLIENT_KEY_PEM                                                          \
"-----BEGIN PRIVATE KEY-----\n" \
"MIIEvAIBADANBgkqhkiG9w0BAQEFAASCBKYwggSiAgEAAoIBAQDJhO0HTaRmIpzn\n"\
"F/OApyIUOEZNVIAE6P+iMEX4G0vQ4m/ON9wPX+dg4PPK00X8cCKWVr0lzjucQYGa\n"\
"lbvceg2wz62jat4FuhADowAnvTZpWsqOW8RRYFklL6IVYOqGtMcfEpstExCeFwzq\n"\
"FRkI4FHlqPcjmv4kOZkJlEoh79Dtlqywf5T+7UD18CVq69iNGEuDm+WRj8i7UOkF\n"\
"QjcBH9QWs0POlqOuXZnZjgP/HhzXIFs95D/8jlq3Zys5hX0+prmGddu7IPGI/4ZU\n"\
"WI15O0ghviIM8GevCyhRQNbLD9OLhE0nHLPdqPQBFcRIfb6wfK5ffZ3OOsv+nmOP\n"\
"TEIsbncRAgMBAAECggEAAJv9yX1dx2Sdu9cnMyw43qhy50308HmHOFrUA1WLIPpm\n"\
"TdB+bQTPDXcbVZwrmdakTCG+rawWIw6m+9bYZr4ZvNShQ51m8XZ9Zj1qb979w0t1\n"\
"JwFhB8FLqQtKqVeN2KgYSArwMYPdaNT3WDKFzQXnSX39d/vtKSKMHhV230EQjSoG\n"\
"QnRHc2wt+eMmOpNKBPsmWaGw88g/1FV03E9wodqJJraJ/4TGmWDFbcexOk2DQ5bE\n"\
"HcxYmTzVh3AcFSrFVVnJRlqvLA5qLUOGdIJMJJvufY6RvsZ0Ltu3eAAAS/DidBFh\n"\
"j1NAgyhGUjBiDC1ED4mI0qUBlI7Q7WxERkBZKnUTiQKBgQD25M6S4nQsdB/n2jXd\n"\
"KkWDMNBl608GB/z2fEpxoPyML4HdBOmhivqCMpd9/eb84uBZR4CsNAOH0aJasPeI\n"\
"5KK9HPQn5e8qOlLGP5veQBXAlmnRNnknBwYQP2i+2IuhG82UQT754R1qt5Fa3CuM\n"\
"2VOzBJ4UjIXCsLnefHVwv7HRpQKBgQDQ87BFBsYTWBjdP4RWwfxW7Hh/QMxZvRP3\n"\
"JHY6/ASzc1PLC9oSFmu9KDS8svapZ2Z/Z0IfaNFQt0xubS84BCs5l9m6oIwfIyMa\n"\
"CP/hFHKyMJufaQxMho/ChzmszJqCl4tHz4B+P5C7SQnP1PxpNqIqRqPSKboB4+0X\n"\
"rKAFkJh7/QKBgDlH6plviq6JnqFnIhoW5ZvDZoZO6r2mes0hFxB33kAUAZgqvJ2r\n"\
"JeWxIS6tCrtfU9fC3BqX0r3fKEDMUBHlodAoeIkNMsjkUTIHrf0Jd0KAYzqT3dXt\n"\
"Lj30yDOHABY34iFkZd0Jmo6Y2lNvQ3dhMKcXCioQVDRQVj9FNuYaw4G1AoGADzli\n"\
"I3Vd4zTtAtnI46VYpGCwWt02xmcPWFdwwdVw5j5F4apoZh76FJskVIdZj3g4MJou\n"\
"aClz7sScLgOIiDVNjPGybHWcH/yoZxI2yoh53t/JxvCZc83uFYdSvXNe4pr2C/Jj\n"\
"c09gVTT6IQlspqHTaO8iqoJdyht4P9xVAHLLmWUCgYA8rdE9w7RJFbiMrpMVNj7z\n"\
"2GydrrmFDRXKw0vtwVvCfykqB68aTJFIpKjdAzMGWsSUwLZIQHTznlDK4umKTuLn\n"\
"jRi1vf4o3SfOA8uWwTMDQViut3N+Ax8jd83b087b9Go+u6tqacj0p0SBNy6i5pHC\n"\
"bUW+rZt03feOxLgDPt/PBg==\n"   \
"-----END PRIVATE KEY-----\n"


/**
 *  @brief Trusted ROOT certificate can be update by following the process specified in the mark down file
 **/
#define HTTPS_TRUSTED_ROOT_CA                               \
"-----BEGIN CERTIFICATE-----\n" \
"MIIEjTCCA3WgAwIBAgIQDQd4KhM/xvmlcpbhMf/ReTANBgkqhkiG9w0BAQsFADBh\n" \
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
"d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH\n" \
"MjAeFw0xNzExMDIxMjIzMzdaFw0yNzExMDIxMjIzMzdaMGAxCzAJBgNVBAYTAlVT\n" \
"MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n" \
"b20xHzAdBgNVBAMTFkdlb1RydXN0IFRMUyBSU0EgQ0EgRzEwggEiMA0GCSqGSIb3\n" \
"DQEBAQUAA4IBDwAwggEKAoIBAQC+F+jsvikKy/65LWEx/TMkCDIuWegh1Ngwvm4Q\n" \
"yISgP7oU5d79eoySG3vOhC3w/3jEMuipoH1fBtp7m0tTpsYbAhch4XA7rfuD6whU\n" \
"gajeErLVxoiWMPkC/DnUvbgi74BJmdBiuGHQSd7LwsuXpTEGG9fYXcbTVN5SATYq\n" \
"DfbexbYxTMwVJWoVb6lrBEgM3gBBqiiAiy800xu1Nq07JdCIQkBsNpFtZbIZhsDS\n" \
"fzlGWP4wEmBQ3O67c+ZXkFr2DcrXBEtHam80Gp2SNhou2U5U7UesDL/xgLK6/0d7\n" \
"6TnEVMSUVJkZ8VeZr+IUIlvoLrtjLbqugb0T3OYXW+CQU0kBAgMBAAGjggFAMIIB\n" \
"PDAdBgNVHQ4EFgQUlE/UXYvkpOKmgP792PkA76O+AlcwHwYDVR0jBBgwFoAUTiJU\n" \
"IBiV5uNu5g/6+rkS7QYXjzkwDgYDVR0PAQH/BAQDAgGGMB0GA1UdJQQWMBQGCCsG\n" \
"AQUFBwMBBggrBgEFBQcDAjASBgNVHRMBAf8ECDAGAQH/AgEAMDQGCCsGAQUFBwEB\n" \
"BCgwJjAkBggrBgEFBQcwAYYYaHR0cDovL29jc3AuZGlnaWNlcnQuY29tMEIGA1Ud\n" \
"HwQ7MDkwN6A1oDOGMWh0dHA6Ly9jcmwzLmRpZ2ljZXJ0LmNvbS9EaWdpQ2VydEds\n" \
"b2JhbFJvb3RHMi5jcmwwPQYDVR0gBDYwNDAyBgRVHSAAMCowKAYIKwYBBQUHAgEW\n" \
"HGh0dHBzOi8vd3d3LmRpZ2ljZXJ0LmNvbS9DUFMwDQYJKoZIhvcNAQELBQADggEB\n" \
"AIIcBDqC6cWpyGUSXAjjAcYwsK4iiGF7KweG97i1RJz1kwZhRoo6orU1JtBYnjzB\n" \
"c4+/sXmnHJk3mlPyL1xuIAt9sMeC7+vreRIF5wFBC0MCN5sbHwhNN1JzKbifNeP5\n" \
"ozpZdQFmkCo+neBiKR6HqIA+LMTMCMMuv2khGGuPHmtDze4GmEGZtYLyF8EQpa5Y\n" \
"jPuV6k2Cr/N3XxFpT3hRpt/3usU/Zb9wfKPtWpoznZ4/44c1p9rzFcZYrWkj3A+7\n" \
"TNBJE0GmP2fhXhP1D/XVfIW/h0yCJGEiV9Glm/uGOa3DXHlmbAcxSyCRraG+ZBkA\n" \
"7h4SeM6Y8l/7MBRpPCz6l8Y=\n" \
"-----END CERTIFICATE-----\n"



/**
 *  End of User Configurable Settings
 **/

/* Set up macro for debugging */
#define DEBUG_HTTPS (0)

/* ENABLE, DIABLE MACROs */
#define ENABLE      (1)
#define DISABLE     (0)

/* Ethernet setup macros */
#define USR_PING_COUNT              (50)
#define PING_DELAY                  (10)
#define TASK_DELAY                  (100)
#define BUFF_SIZE                   (16)

#define PROJECT_INFO                 "\r\nThis project demonstrates an https client and establishes a secure connection"\
		                        "\r\non adafruit.io server. A temperature sensor is connected and the temperature"\
		                        "\r\nreadings are sent and shown to server dashboard. User can select through the"\
		                        "\r\nmenu in RTT viewer to perform either POST or GET  requests.\r\n"

#define PRINT_MENU              "\r\nSelect from the below menu options "\
                                "\r\n 1. POST Request"\
                                "\r\n 2. GET Request\r\n"



typedef struct st_ping_data
{
    uint32_t sent;     // Ping Request
    uint32_t received; // Ping Response
    uint32_t lost;     // Ping failure
} ping_data_t;

typedef enum Userinput
{
    POST = 1,
    GET = 2
}user_input_t;

#if( ipconfigDHCP_REGISTER_HOSTNAME == 1 )
    /* DHCP has an option for clients to register their hostname.  It doesn't
    have much use, except that a device can be found in a router along with its
    name. If this option is used the callback below must be provided by the
    application writer to return a const string, denoting the device's name. */
    const char *pcApplicationHostnameHook( void );
#endif /* ipconfigDHCP_REGISTER_HOSTNAME */

/*
 * function declarations
 */
void print_ipconfig(void);
BaseType_t getIP(const uint8_t IPAddress[NETWORK_SETTINGS_LENGTH_BYTES],
                 const uint8_t NetMask[NETWORK_SETTINGS_LENGTH_BYTES],
                 const uint8_t GatewayAddress[NETWORK_SETTINGS_LENGTH_BYTES],
                 const uint8_t DNSServerAddress[NETWORK_SETTINGS_LENGTH_BYTES],
                 const uint8_t MACAddress[MAC_ADDRESS_LENGTH_BYTES]);
void pingIP(const char *ip_address);
void print_pingResult(void);
void dnsQuerryFunc(char *domain_name);
void updateDhcpResponseToUsr(void);
BaseType_t vSendPing(const char *pcIPAddress);
BaseType_t provision_alt_key(void);
//eDHCPCallbackAnswer_t  xApplicationDHCPHook(eDHCPCallbackPhase_t eDHCPPhase, uint32_t lulIPAddress);
HTTPStatus_t connect_aws_https_client(NetworkContext_t *NetworkContext);
HTTPStatus_t add_header (HTTPRequestHeaders_t * pRequestHeaders);
float convertTemperaturetoFloat(void);
#endif /* USER_APP_H_ */
