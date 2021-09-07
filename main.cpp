/* This example demonstrates connection with AWS IoT through MQTT/HTTPS protocol. 
 *
 * AWS IoT: Thing Shadow MQTT Topics 
 * http://docs.aws.amazon.com/iot/latest/developerguide/thing-shadow-mqtt.html
 *
 * AWS IoT: Publish to a topic through HTTPS/POST method:
 * http://docs.aws.amazon.com/iot/latest/developerguide/protocols.html
 *
 * AWS IoT: Thing Shadow RESTful API:
 * http://docs.aws.amazon.com/iot/latest/developerguide/thing-shadow-rest-api.html
 */

#define AWS_IOT_MQTT_TEST       1
#define AWS_IOT_HTTPS_TEST      0

#include "mbed.h"
#include "MyTLSSocket.h"

#if AWS_IOT_MQTT_TEST
/* MQTT-specific header files */
#include "MQTTmbed.h"
#include "MQTTClient.h"
#endif  // End of AWS_IOT_MQTT_TEST


namespace {

/* List of trusted root CA certificates
 * currently only GlobalSign, the CA for os.mbed.com
 *
 * To add more than one root, just concatenate them.
 */
const char SSL_CA_CERT_PEM[] = "-----BEGIN CERTIFICATE-----\n"
    "MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n"
    "ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n"
    "b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n"
    "MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n"
    "b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n"
    "ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n"
    "9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n"
    "IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n"
    "VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n"
    "93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n"
    "jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n"
    "AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\n"
    "A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n"
    "U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n"
    "N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n"
    "o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n"
    "5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n"
    "rqXRfboQnoZsG4q5WTP468SQvvG5\n"
    "-----END CERTIFICATE-----\n";

/* User certificate which has been activated and attached with specific thing and policy */
const char SSL_USER_CERT_PEM[] = "-----BEGIN CERTIFICATE-----\n"
    "MIIDWTCCAkGgAwIBAgIUAzDIpEQWV/yKVo8suGhvjmFY0n4wDQYJKoZIhvcNAQEL\n"
    "BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g\n"
    "SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTE4MDQxNzA5NDMx\n"
    "M1oXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0\n"
    "ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKsLlECiw4ud5laejJmL\n"
    "bBhafKLdCRx6tkcjBYyEUoAC3Qs2ogqGngQgjU4QJoWpEBO/U1M+e1QtlZ2o/CiL\n"
    "MViHA3rYvP86N/TH8pFA3aPKaeEp+WIt5v4OXdfPkVNKTotiRuRCpzRzrY4xKp11\n"
    "ouKkVKf3FcNuKIMt/uEhje90KofBbFHQY3HFYe19qIg1m/IBV+npmNlAKElGNSB7\n"
    "xHHLzzUuue38s+ceJyzsWuPjFiVYoeyPHF8gDVWf28XJ4KUFs80Deycqe9efroud\n"
    "cQY/6aLDWDJXHvhenwoAIbHqUsYRoWoanrg5Cq3id5+pzVkadNV3+x9bGwROhpbQ\n"
    "M9ECAwEAAaNgMF4wHwYDVR0jBBgwFoAUVUXg3+Dd1qSnAT9LN413zSdNoE0wHQYD\n"
    "VR0OBBYEFIx86SOxw5k/50GtyRjUwlj+9d1gMAwGA1UdEwEB/wQCMAAwDgYDVR0P\n"
    "AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQCI1fqqjvLAFzL2E1nvWMrkaWN2\n"
    "EQK44uOcw53ZzgNNH7fJ85BW8T2l1yZx/Blgs10pEp7vmccnRoR7nYbUGO8++9nG\n"
    "S7bfZhiaE2syJqqvLwPGdqR6fvDdfEpmhgJ1CqeMCqun9XZvUTsgBn7Sqqz7P99h\n"
    "gGmDRKS/CtsPai0Df0ZPNuV/YuUkpHKJSDm+ZTnzevMS3KXkG1cc/sIuc4IwF+aj\n"
    "nbyzdC2fN0r+34srQ8/9aXezOTQ0NBWtoJCCkD+LL6PYJJkAgLA2jcbcbuRJUQ7n\n"
    "Zsp25kKX40fuyIcgPRsd/7sao3zTVYxwKy8r6/mbgrPiMeHvJZ8y3nwUpsPO\n"
    "-----END CERTIFICATE-----\n";

/* User private key paired with above */
const char SSL_USER_PRIV_KEY_PEM[] = "-----BEGIN RSA PRIVATE KEY-----\n"
    "MIIEowIBAAKCAQEAqwuUQKLDi53mVp6MmYtsGFp8ot0JHHq2RyMFjIRSgALdCzai\n"
    "CoaeBCCNThAmhakQE79TUz57VC2Vnaj8KIsxWIcDeti8/zo39MfykUDdo8pp4Sn5\n"
    "Yi3m/g5d18+RU0pOi2JG5EKnNHOtjjEqnXWi4qRUp/cVw24ogy3+4SGN73Qqh8Fs\n"
    "UdBjccVh7X2oiDWb8gFX6emY2UAoSUY1IHvEccvPNS657fyz5x4nLOxa4+MWJVih\n"
    "7I8cXyANVZ/bxcngpQWzzQN7Jyp715+ui51xBj/posNYMlce+F6fCgAhsepSxhGh\n"
    "ahqeuDkKreJ3n6nNWRp01Xf7H1sbBE6GltAz0QIDAQABAoIBAAzl7KILJA/NMmdp\n"
    "wVR6zQXxHODzJhK9ti0bGPoFqGr6zExiLEn66MOK6NzwHteJbirvDIuEdKxeW5/t\n"
    "9EXiaTAxzjNfULE2ZK3Svhnx+ES3qNBP5/xdVcPmtXDmuCC9w7qDCLGBzTYJWxcT\n"
    "4hDJpCTPG4sm+L8p+Wga+dNkQl3CFyHHINDZ0pKcP0kDDt6inKfiU7uU4lFYbCZy\n"
    "PceUgIOTQiNVoPQYtkHgZAtmD9rcwdq2/0GZEbzTkZuSE9S8+WlGxJP5xMGzeVsv\n"
    "zZ/scx0LM7fz5Zq0lsvAwSB1mcs04DaaNpU7Z0tXDIS249RTqdtpPkJzmevpAGhF\n"
    "VNe30/kCgYEA4rflfqyw/YHWKRxCGJRO+q0gPvlBIes30noz5Hxl0knb/J5Ng4Nx\n"
    "xMaIMZgCbwHbw5i01JOPvVKICROKb8wkli4Y2eVzxMPKk2CSpji16RQZ4eOl3YXL\n"
    "1Vnn07Ei+GpsGgDNF0HWf/Ur7es/KdAPCWbKJyoSR90+WN29gP2+Zp8CgYEAwSLv\n"
    "Kt/vdd6XKnR9xR3IajsW/X2GR/x/m2JffJPOP6VpDTKAbv86hVHDV0oBEDMDc7qy\n"
    "023ognyFCPb9Gzol2lq8egjMsisA2bgoB9HqldrSYlaZ0wPe0QJBf1gZ29jPyVJ0\n"
    "ciaBbNbSRhwTrwet7Bae9EbpJsyvBxVh00v0f48CgYEAvKQKviXudmCL01UB4fW0\n"
    "6XsXs44tlY1juyuW9exTxG9ULZOCJ4U9Kl+OfsVecQL42ny7KY1GMl7zdanerDsN\n"
    "zi+42cTDWNsYORxHqSrSoYbqKjwCjJmBCppt/IQM9umF3PUBsPJFCd7zmFj/C0lk\n"
    "2Yu/dGrbHxSFheeqgCOhQz0CgYBfZxdHUYji64o2cYay+QxH1Vp86yWKp6KNKeHL\n"
    "EuP9soKa/0hMDA1nT8UzeB3gV6Kr5xxwrkj9M+8vR3otmeKa4tlZWsFqfS2VXo9/\n"
    "lWTQk1/7LZYckzvceMXL1sQnQgkaBH366SRjlBYYhcP/YMa76Uypk+GVxePrltdU\n"
    "3Z8v5wKBgEXL38yc9LqTIWe1U40ZZKvp2A8c86jtstorEEFqXharE8kxcEpL8ZLL\n"
    "wjgPKdfNMIuApHSrhG7a7gU1rgJyDy1sOIwSvgTYrWfITPTVu5owvSZEblx4KYOm\n"
    "g8hke3Oego4v9cwctkQss3/HZ6rs3PR942oAetuxLy3KPF83IeFm\n"
    "-----END RSA PRIVATE KEY-----\n";

#if AWS_IOT_MQTT_TEST

#define AWS_IOT_MQTT_SERVER_NAME                "a1fljoeglhtf61-ats.iot.us-east-2.amazonaws.com"
#define AWS_IOT_MQTT_SERVER_PORT                8883

#define AWS_IOT_MQTT_THINGNAME                  "Nuvoton-Mbed-D001"

/* Uncomment and assign one unique MQTT client name; otherwise, one random will be assigned. */
//#define AWS_IOT_MQTT_CLIENTNAME                 "Nuvoton Client"

/* User self-test topic */
const char USER_MQTT_TOPIC[] = "Nuvoton/Mbed/D001";
const char *USER_MQTT_TOPIC_FILTERS[] = {
    "Nuvoton/Mbed/+"
};
const char USER_MQTT_TOPIC_PUBLISH_MESSAGE[] = "{ \"message\": \"Hello from Nuvoton Mbed device\" }";

/* Update thing shadow */
const char UPDATETHINGSHADOW_MQTT_TOPIC[] = "$aws/things/" AWS_IOT_MQTT_THINGNAME "/shadow/update";
const char *UPDATETHINGSHADOW_MQTT_TOPIC_FILTERS[] = {
    "$aws/things/" AWS_IOT_MQTT_THINGNAME "/shadow/update/accepted",
    "$aws/things/" AWS_IOT_MQTT_THINGNAME "/shadow/update/rejected"
};
const char UPDATETHINGSHADOW_MQTT_TOPIC_PUBLISH_MESSAGE[] = "{ \"state\": { \"reported\": { \"attribute1\": 3, \"attribute2\": \"1\" } } }";

/* Get thing shadow */
const char GETTHINGSHADOW_MQTT_TOPIC[] = "$aws/things/" AWS_IOT_MQTT_THINGNAME "/shadow/get";
const char *GETTHINGSHADOW_MQTT_TOPIC_FILTERS[] = {
    "$aws/things/" AWS_IOT_MQTT_THINGNAME "/shadow/get/accepted",
    "$aws/things/" AWS_IOT_MQTT_THINGNAME "/shadow/get/rejected"
};
const char GETTHINGSHADOW_MQTT_TOPIC_PUBLISH_MESSAGE[] = "";

/* Delete thing shadow */
const char DELETETHINGSHADOW_MQTT_TOPIC[] = "$aws/things/" AWS_IOT_MQTT_THINGNAME "/shadow/delete";
const char *DELETETHINGSHADOW_MQTT_TOPIC_FILTERS[] = {
    "$aws/things/" AWS_IOT_MQTT_THINGNAME "/shadow/delete/accepted",
    "$aws/things/" AWS_IOT_MQTT_THINGNAME "/shadow/delete/rejected"
};
const char DELETETHINGSHADOW_MQTT_TOPIC_PUBLISH_MESSAGE[] = "";

/* MQTT user buffer size */
const int MQTT_USER_BUFFER_SIZE = 600;

/* Configure MAX_MQTT_PACKET_SIZE to meet your application.
 * We may meet unknown MQTT error with MAX_MQTT_PACKET_SIZE too small, but 
 * MQTT lib doesn't tell enough error message. Try to enlarge it. */
const int MAX_MQTT_PACKET_SIZE = 1000;

/* Timeout for receiving message with subscribed topic */
const int MQTT_RECEIVE_MESSAGE_WITH_SUBSCRIBED_TOPIC_TIMEOUT_MS = 5000;

#if !defined(AWS_IOT_MQTT_CLIENTNAME)
#if TARGET_M23_NS
extern "C"
int mbedtls_hardware_poll( void *data, unsigned char *output, size_t len, size_t *olen );
#endif
#endif

#endif  // End of AWS_IOT_MQTT_TEST

#if AWS_IOT_HTTPS_TEST

#define AWS_IOT_HTTPS_SERVER_NAME               "a1fljoeglhtf61-ats.iot.us-east-2.amazonaws.com"
#define AWS_IOT_HTTPS_SERVER_PORT               8443

#define AWS_IOT_HTTPS_THINGNAME                 "Nuvoton-Mbed-D001"

/* Publish to user topic through HTTPS/POST 
 * HTTP POST https://"endpoint"/topics/"yourTopicHierarchy" */
const char USER_TOPIC_HTTPS_PATH[] = "/topics/Nuvoton/Mbed/D001?qos=1";
const char USER_TOPIC_HTTPS_REQUEST_METHOD[] = "POST";
const char USER_TOPIC_HTTPS_REQUEST_MESSAGE_BODY[] = "{ \"message\": \"Hello from Nuvoton Mbed device\" }";

/* Update thing shadow by publishing to UpdateThingShadow topic through HTTPS/POST
 * HTTP POST https://"endpoint"/topics/$aws/things/"thingName"/shadow/update */
const char UPDATETHINGSHADOW_TOPIC_HTTPS_PATH[] = "/topics/$aws/things/" AWS_IOT_HTTPS_THINGNAME "/shadow/update?qos=1";
const char UPDATETHINGSHADOW_TOPIC_HTTPS_REQUEST_METHOD[] = "POST";
const char UPDATETHINGSHADOW_TOPIC_HTTPS_REQUEST_MESSAGE_BODY[] = "{ \"state\": { \"reported\": { \"attribute1\": 3, \"attribute2\": \"1\" } } }";

/* Get thing shadow by publishing to GetThingShadow topic through HTTPS/POST
 * HTTP POST https://"endpoint"/topics/$aws/things/"thingName"/shadow/get */
const char GETTHINGSHADOW_TOPIC_HTTPS_PATH[] = "/topics/$aws/things/" AWS_IOT_HTTPS_THINGNAME "/shadow/get?qos=1";
const char GETTHINGSHADOW_TOPIC_HTTPS_REQUEST_METHOD[] = "POST";
const char GETTHINGSHADOW_TOPIC_HTTPS_REQUEST_MESSAGE_BODY[] = "";

/* Delete thing shadow by publishing to DeleteThingShadow topic through HTTPS/POST
 * HTTP POST https://"endpoint"/topics/$aws/things/"thingName"/shadow/delete */
const char DELETETHINGSHADOW_TOPIC_HTTPS_PATH[] = "/topics/$aws/things/" AWS_IOT_HTTPS_THINGNAME "/shadow/delete?qos=1";
const char DELETETHINGSHADOW_TOPIC_HTTPS_REQUEST_METHOD[] = "POST";
const char DELETETHINGSHADOW_TOPIC_HTTPS_REQUEST_MESSAGE_BODY[] = "";

/* Update thing shadow RESTfully through HTTPS/POST
 * HTTP POST https://endpoint/things/thingName/shadow */
const char UPDATETHINGSHADOW_THING_HTTPS_PATH[] = "/things/" AWS_IOT_HTTPS_THINGNAME "/shadow";
const char UPDATETHINGSHADOW_THING_HTTPS_REQUEST_METHOD[] = "POST";
const char UPDATETHINGSHADOW_THING_HTTPS_REQUEST_MESSAGE_BODY[] = "{ \"state\": { \"desired\": { \"attribute1\": 1, \"attribute2\": \"2\" }, \"reported\": { \"attribute1\": 2, \"attribute2\": \"1\" } } }";

/* Get thing shadow RESTfully through HTTPS/GET
 * HTTP GET https://"endpoint"/things/"thingName"/shadow */
const char GETTHINGSHADOW_THING_HTTPS_PATH[] = "/things/" AWS_IOT_HTTPS_THINGNAME "/shadow";
const char GETTHINGSHADOW_THING_HTTPS_REQUEST_METHOD[] = "GET";
const char GETTHINGSHADOW_THING_HTTPS_REQUEST_MESSAGE_BODY[] = "";

/* Delete thing shadow RESTfully through HTTPS/DELETE
 * HTTP DELETE https://endpoint/things/thingName/shadow */
const char DELETETHINGSHADOW_THING_HTTPS_PATH[] = "/things/" AWS_IOT_HTTPS_THINGNAME "/shadow";
const char DELETETHINGSHADOW_THING_HTTPS_REQUEST_METHOD[] = "DELETE";
const char DELETETHINGSHADOW_THING_HTTPS_REQUEST_MESSAGE_BODY[] = "";

/* HTTPS user buffer size */
const int HTTPS_USER_BUFFER_SIZE = 600;

const char *HTTPS_OK_STR = "200 OK";

#endif  // End of AWS_IOT_HTTPS_TEST

}

#if AWS_IOT_MQTT_TEST

/**
 * /brief   AWS_IoT_MQTT_Test implements the logic with AWS IoT User/Thing Shadow topics through MQTT.
 */
class AWS_IoT_MQTT_Test {

public:
    /**
     * @brief   AWS_IoT_MQTT_Test Constructor
     *
     * @param[in] domain    Domain name of the MQTT server
     * @param[in] port      Port number of the MQTT server
     * @param[in] net_iface Network interface
     */
    AWS_IoT_MQTT_Test(const char * domain, const uint16_t port, NetworkInterface *net_iface) :
        _domain(domain), _port(port), _net_iface(net_iface) {
        _tlssocket = new MyTLSSocket;
        _mqtt_client = new MQTT::Client<MyTLSSocket, Countdown, MAX_MQTT_PACKET_SIZE>(*_tlssocket);
    }

    /**
     * @brief AWS_IoT_MQTT_Test Destructor
     */
    ~AWS_IoT_MQTT_Test() {
        delete _mqtt_client;
        _mqtt_client = NULL;

        _tlssocket->close();
        delete _tlssocket;
        _tlssocket = NULL;
    }
    /**
     * @brief   Start AWS IoT test through MQTT
     */
    void start_test() {

        int tls_rc;
        int mqtt_rc;

        do {
            /* Set host name of the remote host, used for certificate checking */
            _tlssocket->set_hostname(_domain);

            /* Set the certification of Root CA */
            tls_rc = _tlssocket->set_root_ca_cert(SSL_CA_CERT_PEM);
            if (tls_rc != NSAPI_ERROR_OK) {
                printf("TLSSocket::set_root_ca_cert(...) returned %d\n", tls_rc);
                break;
            }

            /* Set client certificate and client private key */
            tls_rc = _tlssocket->set_client_cert_key(SSL_USER_CERT_PEM, SSL_USER_PRIV_KEY_PEM);
            if (tls_rc != NSAPI_ERROR_OK) {
                printf("TLSSocket::set_client_cert_key(...) returned %d\n", tls_rc);
                break;
            }

            /* Blocking mode */
            _tlssocket->set_blocking(true);

            /* Open a network socket on the network stack of the given network interface */
            printf("Opening network socket on network stack\n");
            tls_rc = _tlssocket->open(_net_iface);
            if (tls_rc != NSAPI_ERROR_OK) {
                printf("Opens network socket on network stack failed: %d\n", tls_rc);
                break;
            }
            printf("Opens network socket on network stack OK\n");

            /* DNS resolution */
            printf("DNS resolution for %s...\n", _domain);
            SocketAddress sockaddr;
            tls_rc = _net_iface->gethostbyname(_domain, &sockaddr);
            if (tls_rc != NSAPI_ERROR_OK) {
                printf("DNS resolution for %s failed with %d\n", _domain, tls_rc);
                break;
            }
            sockaddr.set_port(_port);
            printf("DNS resolution for %s: %s:%d\n", _domain, sockaddr.get_ip_address(), sockaddr.get_port());

            /* Connect to the server */
            /* Initialize TLS-related stuff */
            printf("Connecting with %s:%d\n", _domain, _port);
            tls_rc = _tlssocket->connect(sockaddr);
            if (tls_rc != NSAPI_ERROR_OK) {
                printf("Connects with %s:%d failed: %d\n", _domain, _port, tls_rc);
                break;
            }
            printf("Connects with %s:%d OK\n", _domain, _port);

            /* See the link below for AWS IoT support for MQTT:
             * http://docs.aws.amazon.com/iot/latest/developerguide/protocols.html */

            /* MQTT connect */
            /* The message broker does not support persistent sessions (connections made with 
             * the cleanSession flag set to false. */
            MQTTPacket_connectData conn_data = MQTTPacket_connectData_initializer;
            /* AWS IoT message broker implementation is based on MQTT version 3.1.1
             * 3 = 3.1
             * 4 = 3.1.1 */
            conn_data.MQTTVersion = 4;
            /* Version number of this structure. Must be 0 */
            conn_data.struct_version = 0;

            /* The message broker uses the client ID to identify each client. The client ID is passed
             * in from the client to the message broker as part of the MQTT payload. Two clients with
             * the same client ID are not allowed to be connected concurrently to the message broker.
             * When a client connects to the message broker using a client ID that another client is using,
             * a CONNACK message will be sent to both clients and the currently connected client will be
             * disconnected. */
#if defined(AWS_IOT_MQTT_CLIENTNAME)
            conn_data.clientID.cstring = AWS_IOT_MQTT_CLIENTNAME;
#else
            char client_id_data[32];
#if TARGET_M23_NS
            /* FMC/UID lies in SPE and is inaccessible to NSPE. Use random to generate pseudo-unique instead. */
            uint32_t rand_words[3];
            size_t olen;
            mbedtls_hardware_poll(NULL, (unsigned char *) rand_words, sizeof(rand_words), &olen);
            snprintf(client_id_data, sizeof(client_id_data), "%08X-%08X-%08X",
                     rand_words[0], rand_words[1], rand_words[2]);
#else
            /* Use FMC/UID to generate unique client ID */
            SYS_UnlockReg();
            FMC_Open();
            snprintf(client_id_data, sizeof(client_id_data), "%08X-%08X-%08X",
                     FMC_ReadUID(0), FMC_ReadUID(1), FMC_ReadUID(2));
            FMC_Close();
            SYS_LockReg();
#endif
            conn_data.clientID.cstring = client_id_data;
#endif
            printf("Resolved MQTT client ID: %s\n", conn_data.clientID.cstring);

            /* The message broker does not support persistent sessions (connections made with 
             * the cleanSession flag set to false. The AWS IoT message broker assumes all sessions 
             * are clean sessions and messages are not stored across sessions. If an MQTT client 
             * attempts to connect to the AWS IoT message broker with the cleanSession set to false, 
             * the client will be disconnected. */
            conn_data.cleansession = 1;
            //conn_data.username.cstring = "USERNAME";
            //conn_data.password.cstring = "PASSWORD";

            MQTT::connackData connack_data;

            /* _tlssocket must connect to the network endpoint before calling this. */
            printf("MQTT connecting");
            if ((mqtt_rc = _mqtt_client->connect(conn_data, connack_data)) != 0) {
                printf("\rMQTT connects failed: %d\n", mqtt_rc);
                break;
            }
            printf("\rMQTT connects OK\n\n");

            /* Subscribe/publish user topic */
            printf("Subscribing/publishing user topic\n");
            if (! sub_pub_topic(USER_MQTT_TOPIC, USER_MQTT_TOPIC_FILTERS, sizeof (USER_MQTT_TOPIC_FILTERS) / sizeof (USER_MQTT_TOPIC_FILTERS[0]), USER_MQTT_TOPIC_PUBLISH_MESSAGE)) {
                break;
            }
            printf("Subscribes/publishes user topic OK\n\n");

            /* Subscribe/publish UpdateThingShadow topic */
            printf("Subscribing/publishing UpdateThingShadow topic\n");
            if (! sub_pub_topic(UPDATETHINGSHADOW_MQTT_TOPIC, UPDATETHINGSHADOW_MQTT_TOPIC_FILTERS, sizeof (UPDATETHINGSHADOW_MQTT_TOPIC_FILTERS) / sizeof (UPDATETHINGSHADOW_MQTT_TOPIC_FILTERS[0]), UPDATETHINGSHADOW_MQTT_TOPIC_PUBLISH_MESSAGE)) {
                break;
            }
            printf("Subscribes/publishes UpdateThingShadow topic OK\n\n");

            /* Subscribe/publish GetThingShadow topic */
            printf("Subscribing/publishing GetThingShadow topic\n");
            if (! sub_pub_topic(GETTHINGSHADOW_MQTT_TOPIC, GETTHINGSHADOW_MQTT_TOPIC_FILTERS, sizeof (GETTHINGSHADOW_MQTT_TOPIC_FILTERS) / sizeof (GETTHINGSHADOW_MQTT_TOPIC_FILTERS[0]), GETTHINGSHADOW_MQTT_TOPIC_PUBLISH_MESSAGE)) {
                break;
            }
            printf("Subscribes/publishes GetThingShadow topic OK\n\n");

            /* Subscribe/publish DeleteThingShadow topic */
            printf("Subscribing/publishing DeleteThingShadow topic\n");
            if (! sub_pub_topic(DELETETHINGSHADOW_MQTT_TOPIC, DELETETHINGSHADOW_MQTT_TOPIC_FILTERS, sizeof (DELETETHINGSHADOW_MQTT_TOPIC_FILTERS) / sizeof (DELETETHINGSHADOW_MQTT_TOPIC_FILTERS[0]), DELETETHINGSHADOW_MQTT_TOPIC_PUBLISH_MESSAGE)) {
                break;
            }
            printf("Subscribes/publishes DeleteThingShadow topic OK\n\n");

        } while (0);

        printf("MQTT disconnecting");
        if ((mqtt_rc = _mqtt_client->disconnect()) != 0) {
            printf("\rMQTT disconnects failed %d\n\n", mqtt_rc);
        }
        printf("\rMQTT disconnects OK\n\n");

        _tlssocket->close();
    }

protected:

    /**
     * @brief   Subscribe/publish specific topic
     */
    bool sub_pub_topic(const char *topic, const char **topic_filters, size_t topic_filters_size, const char *publish_message_body) {

        bool ret = false;
        int mqtt_rc;

        do {
            const char **topic_filter;
            const char **topic_filter_end = topic_filters + topic_filters_size;

            for (topic_filter = topic_filters; topic_filter != topic_filter_end; topic_filter ++) {
                /* AWS IoT does not support publishing and subscribing with QoS 2.
                 * The AWS IoT message broker does not send a PUBACK or SUBACK when QoS 2 is requested. */
                printf("MQTT subscribing to %s", *topic_filter);
                if ((mqtt_rc = _mqtt_client->subscribe(*topic_filter, MQTT::QOS1, message_arrived)) != 0) {
                    printf("\rMQTT subscribes to %s failed: %d\n", *topic_filter, mqtt_rc);
                    continue;
                }
                printf("\rMQTT subscribes to %s OK\n", *topic_filter);
            }

            /* Clear count of received message with subscribed topic */
            clear_message_arrive_count();

            MQTT::Message message;

            int _bpos;

            _bpos = snprintf(_buffer, sizeof (_buffer) - 1, publish_message_body);
            if (_bpos < 0 || ((size_t) _bpos) > (sizeof (_buffer) - 1)) {
                printf("snprintf failed: %d\n", _bpos);
                break;
            }
            _buffer[_bpos] = 0;
            /* AWS IoT does not support publishing and subscribing with QoS 2.
             * The AWS IoT message broker does not send a PUBACK or SUBACK when QoS 2 is requested. */
            message.qos = MQTT::QOS1;
            message.retained = false;
            message.dup = false;
            message.payload = _buffer;
            message.payloadlen = strlen(_buffer);
            /* Print publish message */
            printf("Message to publish:\n");
            printf("%s\n", _buffer);
            printf("MQTT publishing message to %s", topic);
            if ((mqtt_rc = _mqtt_client->publish(topic, message)) != 0) {
                printf("\rMQTT publishes message to %s failed: %d\n", topic, mqtt_rc);
                break;
            }
            printf("\rMQTT publishes message to %s OK\n", topic);

            /* Receive message with subscribed topic */
            printf("MQTT receives message with subscribed %s...\n", topic);
            Timer timer;
            timer.start();
            while (! _message_arrive_count) {
                if (timer.read_ms() >= MQTT_RECEIVE_MESSAGE_WITH_SUBSCRIBED_TOPIC_TIMEOUT_MS) {
                    printf("MQTT receives message with subscribed %s TIMEOUT\n", topic);
                    break;
                }

                _mqtt_client->yield(100);
            }
            if (_message_arrive_count) {
                printf("MQTT receives message with subscribed %s OK\n", topic);
            }
            printf("\n");

            /* Unsubscribe 
             * We meet second unsubscribe failed. This is caused by MQTT lib bug. */
            for (topic_filter = topic_filters; topic_filter != topic_filter_end; topic_filter ++) {
                printf("MQTT unsubscribing from %s", *topic_filter);
                if ((mqtt_rc = _mqtt_client->unsubscribe(*topic_filter)) != 0) {
                    printf("\rMQTT unsubscribes from %s failed: %d\n", *topic_filter, mqtt_rc);
                    continue;
                }
                printf("\rMQTT unsubscribes from %s OK\n", *topic_filter);
            }

            ret = true;

        } while (0);

        return ret;
    }

protected:
    MyTLSSocket *                                                           _tlssocket;
    MQTT::Client<MyTLSSocket, Countdown, MAX_MQTT_PACKET_SIZE> *            _mqtt_client;

    const char *_domain;                    /**< Domain name of the MQTT server */
    const uint16_t _port;                   /**< Port number of the MQTT server */
    char _buffer[MQTT_USER_BUFFER_SIZE];    /**< User buffer */
    NetworkInterface *_net_iface;

private:
    static volatile uint16_t   _message_arrive_count;

    static void message_arrived(MQTT::MessageData& md) {
        MQTT::Message &message = md.message;
        printf("Message arrived: qos %d, retained %d, dup %d, packetid %d\r\n", message.qos, message.retained, message.dup, message.id);
        printf("Payload:\n");
        printf("%.*s\n", message.payloadlen, (char*)message.payload);
        ++ _message_arrive_count;
    }

    static void clear_message_arrive_count() {
        _message_arrive_count = 0;
    }
};

volatile uint16_t   AWS_IoT_MQTT_Test::_message_arrive_count = 0;

#endif  // End of AWS_IOT_MQTT_TEST


#if AWS_IOT_HTTPS_TEST

/**
 * /brief   AWS_IoT_HTTPS_Test implements the logic with AWS IoT User/Thing Shadow topics (publish-only)
 *          and Thing Shadow RESTful API through HTTPS.
 */
class AWS_IoT_HTTPS_Test {

public:
    /**
     * @brief   AWS_IoT_HTTPS_Test Constructor
     *
     * @param[in] domain    Domain name of the HTTPS server
     * @param[in] port      Port number of the HTTPS server
     * @param[in] net_iface Network interface
     */
    AWS_IoT_HTTPS_Test(const char * domain, const uint16_t port, NetworkInterface *net_iface) :
        _domain(domain), _port(port), _net_iface(net_iface) {
        _tlssocket = new MyTLSSocket;
    }
    /**
     * @brief AWS_IoT_HTTPS_Test Destructor
     */
    ~AWS_IoT_HTTPS_Test() {
        _tlssocket->close();
        delete _tlssocket;
        _tlssocket = NULL;
    }
    /**
     * @brief Start AWS IoT test through HTTPS
     *
     * @param[in] path  The path of the file to fetch from the HTTPS server
     */
    void start_test() {

        int tls_rc;

        do {
            /* Set host name of the remote host, used for certificate checking */
            _tlssocket->set_hostname(_domain);

            /* Set the certification of Root CA */
            tls_rc = _tlssocket->set_root_ca_cert(SSL_CA_CERT_PEM);
            if (tls_rc != NSAPI_ERROR_OK) {
                printf("TLSSocket::set_root_ca_cert(...) returned %d\n", tls_rc);
                break;
            }

            /* Set client certificate and client private key */
            tls_rc = _tlssocket->set_client_cert_key(SSL_USER_CERT_PEM, SSL_USER_PRIV_KEY_PEM);
            if (tls_rc != NSAPI_ERROR_OK) {
                printf("TLSSocket::set_client_cert_key(...) returned %d\n", tls_rc);
                break;
            }

            /* Open a network socket on the network stack of the given network interface */
            printf("Opening network socket on network stack\n");
            tls_rc = _tlssocket->open(_net_iface);
            if (tls_rc != NSAPI_ERROR_OK) {
                printf("Opens network socket on network stack failed: %d\n", tls_rc);
                break;
            }
            printf("Opens network socket on network stack OK\n");

            /* DNS resolution */
            printf("DNS resolution for %s...\n", _domain);
            SocketAddress sockaddr;
            tls_rc = _net_iface->gethostbyname(_domain, &sockaddr);
            if (tls_rc != NSAPI_ERROR_OK) {
                printf("DNS resolution for %s failed with %d\n", _domain, tls_rc);
                break;
            }
            sockaddr.set_port(_port);
            printf("DNS resolution for %s: %s:%d\n", _domain, sockaddr.get_ip_address(), sockaddr.get_port());

            /* Connect to the server */
            /* Initialize TLS-related stuff */
            printf("Connecting with %s:%d\n", _domain, _port);
            tls_rc = _tlssocket->connect(sockaddr);
            if (tls_rc != NSAPI_ERROR_OK) {
                printf("Connects with %s:%d failed: %d\n", _domain, _port, tls_rc);
                break;
            }
            printf("Connects with %s:%d OK\n", _domain, _port);

            /* Non-blocking mode
             *
             * Don't change to non-blocking mode before connect; otherwise, we may meet NSAPI_ERROR_IN_PROGRESS.
             */
            _tlssocket->set_blocking(false);

            /* Publish to user topic through HTTPS/POST */
            printf("Publishing to user topic through HTTPS/POST\n");
            if (! run_req_resp(USER_TOPIC_HTTPS_PATH, USER_TOPIC_HTTPS_REQUEST_METHOD, USER_TOPIC_HTTPS_REQUEST_MESSAGE_BODY)) {
                break;
            }
            printf("Publishes to user topic through HTTPS/POST OK\n\n");

            /* Update thing shadow by publishing to UpdateThingShadow topic through HTTPS/POST */
            printf("Updating thing shadow by publishing to Update Thing Shadow topic through HTTPS/POST\n");
            if (! run_req_resp(UPDATETHINGSHADOW_TOPIC_HTTPS_PATH, UPDATETHINGSHADOW_TOPIC_HTTPS_REQUEST_METHOD, UPDATETHINGSHADOW_TOPIC_HTTPS_REQUEST_MESSAGE_BODY)) {
                break;
            }
            printf("Update thing shadow by publishing to Update Thing Shadow topic through HTTPS/POST OK\n\n");

            /* Get thing shadow by publishing to GetThingShadow topic through HTTPS/POST */
            printf("Getting thing shadow by publishing to GetThingShadow topic through HTTPS/POST\n");
            if (! run_req_resp(GETTHINGSHADOW_TOPIC_HTTPS_PATH, GETTHINGSHADOW_TOPIC_HTTPS_REQUEST_METHOD, GETTHINGSHADOW_TOPIC_HTTPS_REQUEST_MESSAGE_BODY)) {
                break;
            }
            printf("Get thing shadow by publishing to GetThingShadow topic through HTTPS/POST OK\n\n");

            /* Delete thing shadow by publishing to DeleteThingShadow topic through HTTPS/POST */
            printf("Deleting thing shadow by publishing to DeleteThingShadow topic through HTTPS/POST\n");
            if (! run_req_resp(DELETETHINGSHADOW_TOPIC_HTTPS_PATH, DELETETHINGSHADOW_TOPIC_HTTPS_REQUEST_METHOD, DELETETHINGSHADOW_TOPIC_HTTPS_REQUEST_MESSAGE_BODY)) {
                break;
            }
            printf("Delete thing shadow by publishing to DeleteThingShadow topic through HTTPS/POST OK\n\n");

            /* Update thing shadow RESTfully through HTTPS/POST */
            printf("Updating thing shadow RESTfully through HTTPS/POST\n");
            if (! run_req_resp(UPDATETHINGSHADOW_THING_HTTPS_PATH, UPDATETHINGSHADOW_THING_HTTPS_REQUEST_METHOD, UPDATETHINGSHADOW_THING_HTTPS_REQUEST_MESSAGE_BODY)) {
                break;
            }
            printf("Update thing shadow RESTfully through HTTPS/POST OK\n\n");

            /* Get thing shadow RESTfully through HTTPS/GET */
            printf("Getting thing shadow RESTfully through HTTPS/GET\n");
            if (! run_req_resp(GETTHINGSHADOW_THING_HTTPS_PATH, GETTHINGSHADOW_THING_HTTPS_REQUEST_METHOD, GETTHINGSHADOW_THING_HTTPS_REQUEST_MESSAGE_BODY)) {
                break;
            }
            printf("Get thing shadow RESTfully through HTTPS/GET OK\n\n");

            /* Delete thing shadow RESTfully through HTTPS/DELETE */
            printf("Deleting thing shadow RESTfully through HTTPS/DELETE\n");
            if (! run_req_resp(DELETETHINGSHADOW_THING_HTTPS_PATH, DELETETHINGSHADOW_THING_HTTPS_REQUEST_METHOD, DELETETHINGSHADOW_THING_HTTPS_REQUEST_MESSAGE_BODY)) {
                break;
            }
            printf("Delete thing shadow RESTfully through HTTPS/DELETE OK\n\n");
            
        } while (0);
        
        /* Close socket */
        _tlssocket->close();
    }

protected:

    /**
     * @brief   Run request/response through HTTPS
     */
    bool run_req_resp(const char *https_path, const char *https_request_method, const char *https_request_message_body) {

        bool ret = false;

        do {
            int tls_rc;
            bool _got200 = false;

            int _bpos;

            /* Fill the request buffer */
            _bpos = snprintf(_buffer, sizeof(_buffer) - 1,
                            "%s %s HTTP/1.1\r\n" "Host: %s\r\n" "Content-Length: %d\r\n" "\r\n" "%s",
                            https_request_method, https_path, AWS_IOT_HTTPS_SERVER_NAME, strlen(https_request_message_body), https_request_message_body);
            if (_bpos < 0 || ((size_t) _bpos) > (sizeof (_buffer) - 1)) {
                printf("snprintf failed: %d\n", _bpos);
                break;
            }
            _buffer[_bpos] = 0;
            /* Print request message */
            printf("HTTPS: Request message:\n");
            printf("%s\n", _buffer);

            int offset = 0;
            do {
                tls_rc = _tlssocket->send((const unsigned char *) _buffer + offset, _bpos - offset);
                if (tls_rc > 0) {
                    offset += tls_rc;
                }
            } while (offset < _bpos && 
                    (tls_rc > 0 || tls_rc == NSAPI_ERROR_WOULD_BLOCK));
            if (tls_rc < 0 &&
                tls_rc != NSAPI_ERROR_WOULD_BLOCK) {
                print_mbedtls_error("_tlssocket->send", tls_rc);
                break;
            }

            /* Read data out of the socket */
            offset = 0;
            size_t content_length = 0;
            size_t offset_end = 0;
            char *line_beg = _buffer;
            char *line_end = NULL;
            do {
                tls_rc = _tlssocket->recv((unsigned char *) _buffer + offset, sizeof(_buffer) - offset - 1);
                if (tls_rc > 0) {
                    offset += tls_rc;
                }
                
                /* Make it null-terminated */
                _buffer[offset] = 0;

                /* Scan response message
                 *             
                 * 1. A status line which includes the status code and reason message (e.g., HTTP/1.1 200 OK)
                 * 2. Response header fields (e.g., Content-Type: text/html)
                 * 3. An empty line (\r\n)
                 * 4. An optional message body
                 */
                if (! offset_end) {
                    line_end = strstr(line_beg, "\r\n");
                    if (line_end) {
                        /* Scan status line */
                        if (! _got200) {
                            _got200 = strstr(line_beg, HTTPS_OK_STR) != NULL;
                        }
            
                        /* Scan response header fields for Content-Length 
                         * 
                         * NOTE: Assume chunked transfer (Transfer-Encoding: chunked) is not used
                         * NOTE: Assume response field name are in lower case
                         */
                        if (content_length == 0) {
                            sscanf(line_beg, "content-length:%d", &content_length);
                        }
                    
                        /* An empty line indicates end of response header fields */
                        if (line_beg == line_end) {
                            offset_end = line_end - _buffer + 2 + content_length;
                        }
                    
                        /* Go to next line */
                        line_beg = line_end + 2;
                        line_end = NULL;
                    }
                }
            } while ((offset_end == 0 || offset < offset_end) &&
                    (tls_rc > 0 || tls_rc == NSAPI_ERROR_WOULD_BLOCK));
            if (tls_rc < 0 && 
                tls_rc != NSAPI_ERROR_WOULD_BLOCK) {
                print_mbedtls_error("_tlssocket->read", tls_rc);
                break;
            }
            _bpos = offset;

            _buffer[_bpos] = 0;

            /* Print status messages */
            printf("HTTPS: Received %d chars from server\n", _bpos);
            printf("HTTPS: Received 200 OK status ... %s\n", _got200 ? "[OK]" : "[FAIL]");
            printf("HTTPS: Received message:\n");
            printf("%s\n", _buffer);

            ret = true;

        } while (0);

        return ret;
    }

protected:
    MyTLSSocket *     _tlssocket;

    const char *_domain;                    /**< Domain name of the HTTPS server */
    const uint16_t _port;                   /**< Port number of the HTTPS server */
    char _buffer[HTTPS_USER_BUFFER_SIZE];   /**< User buffer */
    NetworkInterface *_net_iface;
};

#endif  // End of AWS_IOT_HTTPS_TEST

int main() {

    /* The default 9600 bps is too slow to print full TLS debug info and could
     * cause the other party to time out. */

    printf("\nStarting AWS IoT test\n");

#if defined(MBED_MAJOR_VERSION)
    printf("Using Mbed OS %d.%d.%d\n", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);
#else
    printf("Using Mbed OS from master.\n");
#endif

    NetworkInterface *net = NetworkInterface::get_default_instance();
    if (NULL == net) {
        printf("Connecting to the network failed. See serial output.\n");
        return 1;
    }
    nsapi_error_t status = net->connect();
    if (status != NSAPI_ERROR_OK) {
        printf("Connecting to the network failed %d!\n", status);
        return -1;
    }
    SocketAddress sockaddr;
    status = net->get_ip_address(&sockaddr);
    if (status != NSAPI_ERROR_OK) {
        printf("Network interface get_ip_address(...) failed with %d", status);
        return -1;
    }
    printf("Connected to the network successfully. IP address: %s\n", sockaddr.get_ip_address());

#if AWS_IOT_MQTT_TEST
    AWS_IoT_MQTT_Test *mqtt_test = new AWS_IoT_MQTT_Test(AWS_IOT_MQTT_SERVER_NAME, AWS_IOT_MQTT_SERVER_PORT, net);
    mqtt_test->start_test();
    delete mqtt_test;
#endif  // End of AWS_IOT_MQTT_TEST

#if AWS_IOT_HTTPS_TEST
    AWS_IoT_HTTPS_Test *https_test = new AWS_IoT_HTTPS_Test(AWS_IOT_HTTPS_SERVER_NAME, AWS_IOT_HTTPS_SERVER_PORT, net);
    https_test->start_test();
    delete https_test;
#endif  // End of AWS_IOT_HTTPS_TEST

    /* Some cellular modems e.g.: QUECTEL EC2X need graceful exit; otherwise, they will break in next reboot. */
    status = net->disconnect();
    if (status != NSAPI_ERROR_OK) {
        printf("\n\nDisconnect from network interface failed %d\n", status);
    }
}
