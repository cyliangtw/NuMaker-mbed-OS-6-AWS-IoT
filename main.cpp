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
#if TARGET_M2354
#define SENSOR_BME680_TEST      1
#else
#define SENSOR_BME680_TEST      0
#endif
#if SENSOR_BME680_TEST
#define NVT_DEMO_SENSOR         1
#endif

#include "mbed.h"
#include "MyTLSSocket.h"

#if SENSOR_BME680_TEST
#include "mbed_bme680.h"
 
BME680 bme680(0x76 << 1);  // Slave address
#endif  // End of SENSOR_BME680_TEST

#if TARGET_M2354
#include "psa/protected_storage.h"
/* User-managed UID for PS/ITS storage */
psa_storage_uid_t uid_wifi_ssid = 0x5a5b0001;
psa_storage_uid_t uid_wifi_passwd = 0x5a5b0002;
uint8_t data_wifi_ssid[16];
uint8_t data_wifi_passwd[16];
I2C i2c(PB_12, PB_13);
#else
I2C i2c(I2C_SDA, I2C_SCL);  // Used inside the BME680 Mbed Lib.
#endif

#if AWS_IOT_MQTT_TEST
/* MQTT-specific header files */
#include "MQTTmbed.h"
#include "MQTTClient.h"
#endif  // End of AWS_IOT_MQTT_TEST

#ifdef TARGET_M2354
#include "lcd_api.h"
#include "lcdlib.h"
#else
#define lcd_init()
#define lcd_printf(X0,X1)
#define lcd_printNumber(X2,X3)
#define lcd_printNumberEx(X4,X5,X6)
#define lcd_setSymbol(X7,X8)
#define LCD_EnableBlink(X)
#define LCD_DisableBlink()
#define SYMBOL_TIME_DIG_COL1	0
#define ZONE_TIME_DIGIT		0
#define ZONE_MAIN_DIGIT		0
#endif

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
const char SSL_USER_CERT_PEM[] = "Input User Cert";


/* User private key paired with above */
const char SSL_USER_PRIV_KEY_PEM[] = "Input User Private Key";


#if AWS_IOT_MQTT_TEST

#define AWS_IOT_MQTT_SERVER_NAME                "Input User Broker Endpoint"
#define AWS_IOT_MQTT_SERVER_PORT                8883

#define AWS_IOT_MQTT_THINGNAME                  "Input User Thing Name"

/* Uncomment and assign one unique MQTT client name; otherwise, one random will be assigned. */
//#define AWS_IOT_MQTT_CLIENTNAME                 "Nuvoton Client"

#ifndef NVT_DEMO_SENSOR
/* User self-test topic */
const char USER_MQTT_TOPIC[] = "Nuvoton/Mbed/D001";
const char *USER_MQTT_TOPIC_FILTERS[] = {
    "Nuvoton/Mbed/+"
};
const char USER_MQTT_TOPIC_PUBLISH_MESSAGE[] = "{ \"message\": \"Hello from Nuvoton Mbed device\" }";
#endif

/* Update thing shadow */
const char UPDATETHINGSHADOW_MQTT_TOPIC[] = "$aws/things/" AWS_IOT_MQTT_THINGNAME "/shadow/update";
const char *UPDATETHINGSHADOW_MQTT_TOPIC_FILTERS[] = {
    "$aws/things/" AWS_IOT_MQTT_THINGNAME "/shadow/update/accepted",
    "$aws/things/" AWS_IOT_MQTT_THINGNAME "/shadow/update/rejected"
};

#ifndef NVT_DEMO_SENSOR
const char UPDATETHINGSHADOW_MQTT_TOPIC_PUBLISH_MESSAGE[] = "{ \"state\": { \"reported\": { \"attribute1\": 3, \"attribute2\": \"1\" } } }";
#else
//const char UPDATETHINGSHADOW_MQTT_TOPIC_PUBLISH_MESSAGE[] = "{ \"state\": { \"reported\": { \"temperature\": %2.2f } } }";
const char UPDATETHINGSHADOW_MQTT_TOPIC_PUBLISH_MESSAGE[] = "{ \"state\": { \"reported\": { \"clientName\":\"%s\", \"temperature\": %2.2f, \"humidity\": %2.2f, \"pressure\": %.2f } } }";
#endif

#ifndef NVT_DEMO_SENSOR
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
#endif

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

#define AWS_IOT_HTTPS_SERVER_NAME               "Input User Broker Endpoint"
#define AWS_IOT_HTTPS_SERVER_PORT               8443

#define AWS_IOT_HTTPS_THINGNAME                 "Input User Thing Name"

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
        char cClientName[32];

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
                     (unsigned int)rand_words[0], (unsigned int)rand_words[1],(unsigned int) rand_words[2]);
#else
            /* Use FMC/UID to generate unique client ID */
            SYS_UnlockReg();
            FMC_Open();
            snprintf(client_id_data, sizeof(client_id_data), "%08X-%08X-%08X",
                     (unsigned int)FMC_ReadUID(0), (unsigned int)FMC_ReadUID(1), (unsigned int)FMC_ReadUID(2));
            FMC_Close();
            SYS_LockReg();
#endif
            conn_data.clientID.cstring = client_id_data;
#endif
            strncpy(cClientName, conn_data.clientID.cstring, sizeof(cClientName));
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
            /* MQTT connects OK set default LCD display. */
            char text [8];

            LCD_DisableBlink();
            lcd_printf(ZONE_MAIN_DIGIT,"   OK");
            thread_sleep_for(800);
            lcd_printf(ZONE_MAIN_DIGIT,"");
            lcd_setSymbol(SYMBOL_NVT, 1);
            lcd_printNumber(ZONE_VER_DIGIT, 101);
            lcd_setSymbol(SYMBOL_VERSION, 1);
            lcd_setSymbol(SYMBOL_VER_DIG_P1, 1); 
            /* Show default pressure, hPa */
            sprintf(text, "%4dhPa", 0000);
            lcd_printf(ZONE_MAIN_DIGIT, text);
            /* Show default humidity, %rH */
            lcd_printNumberEx(ZONE_PPM_DIGIT, 00, 2);
            lcd_setSymbol(SYMBOL_PERCENTAGE, 1);
            /* Show default temperature, degC */    
            lcd_printNumberEx(ZONE_TEMP_DIGIT, 00, 2);
            lcd_setSymbol(SYMBOL_TEMP_C, 1);
            lcd_setSymbol(SYMBOL_WIFI, 1);

#ifndef NVT_DEMO_SENSOR
            /* Subscribe/publish user topic */
            printf("Subscribing/publishing user topic\n");
            if (! sub_pub_topic(USER_MQTT_TOPIC, USER_MQTT_TOPIC_FILTERS, sizeof (USER_MQTT_TOPIC_FILTERS) / sizeof (USER_MQTT_TOPIC_FILTERS[0]), USER_MQTT_TOPIC_PUBLISH_MESSAGE)) {
                break;
            }
            printf("Subscribes/publishes user topic OK\n\n");
            lcd_printf(ZONE_MAIN_DIGIT, "USER OK");
            
            /* Subscribe/publish UpdateThingShadow topic */
            printf("Subscribing/publishing UpdateThingShadow topic\n");
            if (! sub_pub_topic(UPDATETHINGSHADOW_MQTT_TOPIC, UPDATETHINGSHADOW_MQTT_TOPIC_FILTERS, sizeof (UPDATETHINGSHADOW_MQTT_TOPIC_FILTERS) / sizeof (UPDATETHINGSHADOW_MQTT_TOPIC_FILTERS[0]), UPDATETHINGSHADOW_MQTT_TOPIC_PUBLISH_MESSAGE)) {
                break;
            }
            printf("Subscribes/publishes UpdateThingShadow topic OK\n\n");
            lcd_printf(ZONE_MAIN_DIGIT, "UPDATE OK");
            
            /* Subscribe/publish GetThingShadow topic */
            printf("Subscribing/publishing GetThingShadow topic\n");
            if (! sub_pub_topic(GETTHINGSHADOW_MQTT_TOPIC, GETTHINGSHADOW_MQTT_TOPIC_FILTERS, sizeof (GETTHINGSHADOW_MQTT_TOPIC_FILTERS) / sizeof (GETTHINGSHADOW_MQTT_TOPIC_FILTERS[0]), GETTHINGSHADOW_MQTT_TOPIC_PUBLISH_MESSAGE)) {
                break;
            }
            printf("Subscribes/publishes GetThingShadow topic OK\n\n");
            lcd_printf(ZONE_MAIN_DIGIT, "GET OK");
            
            /* Subscribe/publish DeleteThingShadow topic */
            printf("Subscribing/publishing DeleteThingShadow topic\n");
            if (! sub_pub_topic(DELETETHINGSHADOW_MQTT_TOPIC, DELETETHINGSHADOW_MQTT_TOPIC_FILTERS, sizeof (DELETETHINGSHADOW_MQTT_TOPIC_FILTERS) / sizeof (DELETETHINGSHADOW_MQTT_TOPIC_FILTERS[0]), DELETETHINGSHADOW_MQTT_TOPIC_PUBLISH_MESSAGE)) {
                break;
            }
            printf("Subscribes/publishes DeleteThingShadow topic OK\n\n");
            lcd_printf(ZONE_MAIN_DIGIT, "DEL OK");
#endif
        } while (0);

#ifdef NVT_DEMO_SENSOR
        char cDataBuffer[ 256 ];
        char cLcdStr [8];
        uint32_t pressure, humidity, temperature;
        do {
            /* Subscribe/publish UpdateThingShadow topic */
            printf("Subscribing/publishing UpdateThingShadow topic\n");
            if (bme680.performReading()) {
//                ( void ) snprintf(cDataBuffer, sizeof(cDataBuffer) - 1, UPDATETHINGSHADOW_MQTT_TOPIC_PUBLISH_MESSAGE, bme680.getHumidity() );
                ( void ) snprintf(cDataBuffer, sizeof(cDataBuffer) - 1, UPDATETHINGSHADOW_MQTT_TOPIC_PUBLISH_MESSAGE, cClientName, bme680.getTemperature(), bme680.getHumidity(), bme680.getPressure() );
                if (! sub_pub_topic(UPDATETHINGSHADOW_MQTT_TOPIC, UPDATETHINGSHADOW_MQTT_TOPIC_FILTERS, sizeof (UPDATETHINGSHADOW_MQTT_TOPIC_FILTERS) / sizeof (UPDATETHINGSHADOW_MQTT_TOPIC_FILTERS[0]), (const char*)cDataBuffer)) {
                    break;
                }
                printf("Subscribes/publishes UpdateThingShadow topic OK\n\n");
                temperature = bme680.getTemperature();
                pressure = bme680.getPressure()/100;
                humidity = bme680.getHumidity();
                sprintf(cLcdStr, "%4dhPa", (int)pressure);
                lcd_printf(ZONE_MAIN_DIGIT, cLcdStr);
                lcd_printNumberEx(ZONE_TEMP_DIGIT,temperature,2);
                lcd_printNumberEx(ZONE_PPM_DIGIT, humidity, 2);
            } else {
                printf("Read Sensor failed \n\n");
                lcd_printf(ZONE_MAIN_DIGIT, "SENSOR FAIL");
            }
            thread_sleep_for(500);
            /*  RTC display  */
            time_t rtctt;
            char buffer[32];
            uint32_t u32TimeData, u32TimeHour, u32TimeMinute;
            rtctt = rtc_read();
            strftime(buffer, 32, "%H:%M\n", localtime(&rtctt));
            printf("Time as a custom formatted string = %s", buffer);

            strftime(buffer, 32, "%H\n", localtime(&rtctt));
            u32TimeHour = atoi(buffer);
            strftime(buffer, 32, "%M\n", localtime(&rtctt));
            u32TimeMinute = atoi(buffer);
            u32TimeData = ( u32TimeHour *100) + u32TimeMinute ;
            lcd_printNumber(ZONE_TIME_DIGIT, u32TimeData);
        } while (1);
#endif

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

            _bpos = snprintf(_buffer, sizeof (_buffer) - 1, "%s", publish_message_body);
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
                if ((timer.elapsed_time()).count()/1000 >= MQTT_RECEIVE_MESSAGE_WITH_SUBSCRIBED_TOPIC_TIMEOUT_MS) {   
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

#if SENSOR_BME680_TEST
static void sensor_test() {
    int count = 10;
   
    if (!bme680.begin()) {
        printf("BME680 Begin failed \r\n");
        return;
    }
    do {
        if (++count >= 10)
        {
            count = 0;
            printf("\r\nTemperature  Humidity  Pressure    VOC\r\n"
                   "    degC        %%        hPa      KOhms\r\n"
                   "------------------------------------------\r\n");
        }
 
        if (bme680.performReading())
        {
            printf("   %.2f      ", bme680.getTemperature());
            printf("%.2f    ", bme680.getHumidity());
            printf("%.2f    ", bme680.getPressure() / 100.0);
            printf("%0.2f\r\n", bme680.getGasResistance() / 1000.0);
        }
        thread_sleep_for(1000);
    } while(0);
}
#else
#define sensor_test()
#endif  // End of SENSOR_BME680_TEST

extern "C" {
    MBED_WEAK int fetch_host_command(void);
}

Ticker flipper;
static volatile uint32_t g_u32RTCINT = 0;

/* Add tikcer for symbol ":" blinking 1/sec  */
void flip()
{
    g_u32RTCINT++;
    if(g_u32RTCINT == 1)
        lcd_setSymbol(SYMBOL_TIME_DIG_COL1, 1);
    else{
        g_u32RTCINT = 0;    
        lcd_setSymbol(SYMBOL_TIME_DIG_COL1, 0);}
}

int main() {
    /* The default 9600 bps is too slow to print full TLS debug info and could
     * cause the other party to time out. */

    printf("\nStarting AWS IoT test\n");

    printf("Enable LCD program ...\r\n");
    lcd_init();

    lcd_printf(ZONE_MAIN_DIGIT, "START");
    thread_sleep_for(2000);
    lcd_printf(ZONE_MAIN_DIGIT, "");

    sensor_test();
    
#if defined(MBED_MAJOR_VERSION)
    printf("Using Mbed OS %d.%d.%d\n", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);
#else
    printf("Using Mbed OS from master.\n");
#endif
    
    lcd_printf(ZONE_MAIN_DIGIT, "CONNECT");
    LCD_EnableBlink(500);
    
    /* lcd showing RTC */
    //set_time(1256729737);
    time_t rtctt;
    char buffer[32];
    uint32_t u32TimeData, u32TimeHour, u32TimeMinute;
    rtc_init();
    rtctt = rtc_read();

    printf("this as seconds since january 1, 1970 =%d\n", (int)rtctt);

    strftime(buffer, 32, "%H:%M\n", localtime(&rtctt));
    printf("Time as a custom formatted string = %s", buffer);

    strftime(buffer, 32, "%H\n", localtime(&rtctt));
    u32TimeHour = atoi(buffer);
    strftime(buffer, 32, "%M\n", localtime(&rtctt));
    u32TimeMinute = atoi(buffer);
    u32TimeData = ( u32TimeHour *100) + u32TimeMinute ;
    lcd_setSymbol(SYMBOL_TIME_DIG_COL1, 1);
    lcd_printNumber(ZONE_TIME_DIGIT, u32TimeData);
    flipper.attach(&flip, 500ms);   

    NetworkInterface *net = NetworkInterface::get_default_instance();

    if (NULL == net) {
        printf("Connecting to the network failed. See serial output.\n");
        return 1;
    }

#define WIFI 2
#if (MBED_CONF_TARGET_NETWORK_DEFAULT_INTERFACE_TYPE != WIFI )
    nsapi_error_t status = net->connect();
#else // For wifi interface
    char my_ssid[16];
    char my_passwd[16];
    int choice;
    int jj = 0;
    nsapi_error_t status;
    printf("Input WiFi SSID/PASSWD(Y/N):\r\n");
    for( jj=0; jj < 50; jj++)
    {
        choice = fetch_host_command();
        if(choice != -1) {
            if( (choice == 'Y') || (choice == 'y') || (choice == 'N') || (choice == 'n') ) {
                break;
            }
        }
        wait_us(50*1000);
    }
    if( (choice == 'Y') || (choice == 'y') ) {
        printf("WiFi SSID:");
        scanf("%s", (char *)&my_ssid);
        printf("\nSSID=%s\r\n", my_ssid);
        printf("WiFi PASSWD:");
        scanf("%s",(char *)&my_passwd);
#ifdef TARGET_M2354
        /* Write data into PSA storage */
        /* Create a new, or modify an existing, uid/value pair */
        if (PSA_SUCCESS != psa_ps_set(uid_wifi_ssid, sizeof(my_ssid), my_ssid, PSA_STORAGE_FLAG_NONE)) {
            printf("Store WiFi SSID into PSA storage failed \r\n");
        }
        if (PSA_SUCCESS != psa_ps_set(uid_wifi_passwd, sizeof(my_passwd), my_passwd, PSA_STORAGE_FLAG_NONE)) {
            printf("Store WiFi PASWWD into PSA storage failed \r\n");
        }
#endif
    } else {
#ifdef TARGET_M2354
        /* Get information of data from PSA storage */
        psa_status_t retStatus;
        //struct psa_storage_info_t data1_info; 
        size_t retLen;
        retStatus = psa_ps_get(uid_wifi_ssid, 0, sizeof(data_wifi_ssid), &data_wifi_ssid, &retLen);
        if (PSA_SUCCESS != retStatus) {
            strcpy(my_ssid, MBED_CONF_NSAPI_DEFAULT_WIFI_SSID);
        } else {
            strcpy(my_ssid, (const char*)data_wifi_ssid);
        }
        retStatus = psa_ps_get(uid_wifi_passwd, 0, sizeof(data_wifi_passwd), &data_wifi_passwd, &retLen);
        if (PSA_SUCCESS != retStatus) {
            strcpy(my_ssid, MBED_CONF_NSAPI_DEFAULT_WIFI_PASSWORD);
        } else {
            strcpy(my_passwd, (const char*)data_wifi_passwd);
        }
#else
        strcpy(my_ssid, MBED_CONF_NSAPI_DEFAULT_WIFI_SSID);
        strcpy(my_passwd, MBED_CONF_NSAPI_DEFAULT_WIFI_PASSWORD);
#endif
        printf("\nDefault SSID=%s\r\n", my_ssid);
    }
    status = (net->wifiInterface())->connect(my_ssid, my_passwd, NSAPI_SECURITY_WPA2,0);    
#endif

    if (status != NSAPI_ERROR_OK) {
        printf("Connecting to the network failed %d!\n", status);
        LCD_DisableBlink();
        thread_sleep_for(800);
        lcd_printf(ZONE_MAIN_DIGIT, "  FAIL");
        return -1;
    }
    SocketAddress sockaddr;
    status = net->get_ip_address(&sockaddr);
    if (status != NSAPI_ERROR_OK) {
        printf("Network interface get_ip_address(...) failed with %d", status);
        return -1;
    }
    printf("Connected to the network successfully. IP address: %s\n", sockaddr.get_ip_address());
//    lcd_printf(sockaddr.get_ip_address());
//    ThisThread::sleep_for(500);
//    lcd_printf(sockaddr.get_ip_address() + 8);
//    lcd_printNumber(200);

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
    lcd_printf(ZONE_MAIN_DIGIT, "   End");
}
