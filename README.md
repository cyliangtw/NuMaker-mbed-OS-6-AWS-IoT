# Example for Connection with AWS IoT thru MQTT/HTTPS on Mbed OS

This is an example to demonstrate connection with [AWS IoT](https://aws.amazon.com/iot)
on Nuvoton Mbed-enabled boards.

## Supported platforms
On Mbed OS, connection with AWS IoT requires Mbed TLS. It requires more than 64 KB RAM.
Currently, the following Nuvoton Mbed-enalbed boards can afford such memory footprint:
- [NuMaker-PFM-NUC472](https://developer.mbed.org/platforms/Nuvoton-NUC472/)
- [NuMaker-PFM-M487](https://developer.mbed.org/platforms/NUMAKER-PFM-M487/)
- [NuMaker-IoT-M487](https://os.mbed.com/platforms/NUMAKER-IOT-M487/)
- [NuMaker-M2354](https://os.mbed.com/platforms/NUMAKER-M2354/)
- [NuMaker-IoT-M2354 (Board page not ready yet)](https://os.mbed.com/platforms/NUMAKER-M2354/)

## Supported prebuilt image
For easy verification of AWS IoT connection on NuMaker platforms, we provided prebuilt bin file with AWS certificate & key and you could drag & drop this bin file into your NuMaker board on hand. For details, you could open the user guide in the prebuilt folder.
Currently, you could get prebuilt bin file from the following Nuvoton boards:
- [NuMaker-IOT-M2354](https://github.com/OpenNuvoton/M2354BSP/tree/master/SampleCode/NuMakerIoT/mbedOS6_AWS_IoT/)

## Access and manage AWS IoT Service
To run the example, you need to register one [AWS account](https://aws.amazon.com/)
to access and manage AWS IoT Service for your device to connect with.
This [link](https://docs.aws.amazon.com/iot/latest/developerguide/what-is-aws-iot.html) gives detailed
information about it.

1. Sign in to [AWS Management Console](https://aws.amazon.com/console/).
1. Enter AWS IoT Service.
1. In AWS IoT Service, create a thing.
The Console may prompt you to also create a certificate and a policy. Skip for creating them later.
1. In AWS IoT Service, create a policy. A workable example would be below.
Note that you need to replace **REGION** and **ACCOUNT** to match your case.

    <pre>
    {
        "Version": "2012-10-17",
        "Statement": [
            {
                "Effect": "Allow",
                "Action": "iot:Connect",
                "Resource": "arn:aws:iot:<b>REGION</b>:<b>ACCOUNT</b>:client/*"
            },
            {
                "Effect": "Allow",
                "Action": "iot:Subscribe",
                "Resource": ["arn:aws:iot:<b>REGION</b>:<b>ACCOUNT</b>:topicfilter/*"]
            },
            {
                "Effect": "Allow",
                "Action": ["iot:Publish", "iot:Receive"],
                "Resource": "arn:aws:iot:<b>REGION</b>:<b>ACCOUNT</b>:topic/*"
            },
            {
                "Effect": "Allow",
                "Action": ["iot:UpdateThingShadow", "iot:GetThingShadow", "iot:DeleteThingShadow"],
                "Resource": "arn:aws:iot:<b>REGION</b>:<b>ACCOUNT</b>:thing/*"
            }
        ]
    }
    </pre>

1. In AWS IoT Service, create a certificate. You would get 4 security credential files from it.
   Download them for later use.
   - AWS IoT's CA certificate
   - User certificate
   - User private key
   - User public key
   
   After creating the certificate, do:
   1. Activate the certificate
   1. Attach the thing created above to the certificate
   1. Attach the policy created above to the certificate

## Configure your device with AWS IoT
Before connecting your device with AWS IoT, you need to configure security credential and
protocol dependent parameters into your device. These configurations are all centralized in `main.cpp`.

### Configure certificate into your device
From above, you've got 4 security credential files: CA certificate and user certificate/private key/public key.
Configure CA certificate, user certificate, and user private key into your device.
User public key has been included in user certificate and is not used here.
1. Replace CA certificate with downloaded from the Console.
    ```
    const char SSL_CA_CERT_PEM[] = "-----BEGIN CERTIFICATE-----\n"
        "Replace Me"
    ```

1. Replace user certificate with downloaded from the Console.
    ```
    const char SSL_USER_CERT_PEM[] = "-----BEGIN CERTIFICATE-----\n"
        "Replace Me"
    ```

1. Replace user private key with downloaded from the Console.
    ```
    const char SSL_USER_PRIV_KEY_PEM[] = "-----BEGIN RSA PRIVATE KEY-----\n"
        "Replace Me"
    ```

**NOTE:** The credential hard-coded in source code may get deactivated or deleted.
          Use your own credential for connection with AWS IoT.

### Connect through MQTT
To connect your device with AWS IoT through MQTT, you need to configure the following parameters.

1. Enable connection through MQTT.
    ```
    #define AWS_IOT_MQTT_TEST       1
    ```

1. Replace server name (endpoint). **Endpoint** has the following format and you just 
   need to modify **IDENTIFIER** and **REGION** to match your case.
    <pre>
    #define AWS_IOT_MQTT_SERVER_NAME                "<b>IDENTIFIER</b>.iot.<b>REGION</b>.amazonaws.com"
    </pre>
   
1. Server port number is fixed. Don't change it.
    ```
    #define AWS_IOT_MQTT_SERVER_PORT                8883
    ```
    
1. Replace **THINGNAME** to match your case. The **THINGNAME** is just the name of the thing you've created above.
    <pre>
    #define AWS_IOT_MQTT_THINGNAME                  "<b>THINGNAME</b>"
    </pre>
    
1. Replace **CLIENTNAME** to match your case. If you adopt the example policy above,
   you can modify it arbitrarily because the policy permits any client name bound to your account.
    <pre>
    #define AWS_IOT_MQTT_CLIENTNAME                 "<b>CLIENTNAME</b>"
    </pre>

AWS IoT MQTT protocol supports topic subscribe/publish. The example demonstrates:
- Subscribe/publish with user topic
- Subscribe/publish with reserved topic (starting with $) to:
    - Update thing shadow
    - Get thing shadow
    - Delete thing shadow

### Connect through HTTPS
To connect your device with AWS IoT through HTTPS, you need to configure the following parameters.

1. Enable connection through HTTPS.
    ```
    #define AWS_IOT_HTTPS_TEST      1
    ```

1. Replace server name (endpoint). **Endpoint** has the following format and you just 
   need to modify **IDENTIFIER** and **REGION** to match your case.
    <pre>
    #define AWS_IOT_HTTPS_SERVER_NAME               "<b>IDENTIFIER</b>.iot.<b>REGION</b>.amazonaws.com"
    </pre>
   
1. Server port number is fixed. Don't change it.
    ```
    #define AWS_IOT_HTTPS_SERVER_PORT               8443
    ```
    
1. Replace **THINGNAME** to match your case. The **THINGNAME** is just the name of the thing you've created above.
    <pre>
    #define AWS_IOT_HTTPS_THINGNAME                 "<b>THINGNAME</b>"
    </pre>

AWS IoT HTTPS protocol supports topic publish-only and RESTful API. The example demonstrates:
- Publish to user topic
- Publish to reserved topic (starting with $) to:
    - Update thing shadow
    - Get thing shadow
    - Delete thing shadow
- RESTful API to:
    - Update thing shadow RESTfully through HTTPS/POST method
    - Get thing shadow RESTfully through HTTPS/GET method
    - Delete thing shadow RESTfully through HTTPS/DELETE method

## Monitor the application
If you configure your terminal program with **115200/8-N-1**, you would see output similar to:

**NOTE:** Make sure that the network is functional before running the application.

<pre>
Starting AWS IoT test
Using Mbed OS 6.14.0
Connected to the network successfully. IP address: 192.168.8.105
Opening network socket on network stack
Opens network socket on network stack OK
DNS resolution for a1fljoeglhtf61-ats.iot.us-east-2.amazonaws.com...
DNS resolution for a1fljoeglhtf61-ats.iot.us-east-2.amazonaws.com: 3.129.252.104:8883
</pre>

If you get here successfully, it means configurations with security credential are correct.
<pre>
Connecting with a1fljoeglhtf61-ats.iot.us-east-2.amazonaws.com:8883
Connects with a1fljoeglhtf61-ats.iot.us-east-2.amazonaws.com:8883 OK
Resolved MQTT client ID: 002E0051-013B87F3-00000021
MQTT connects OK
</pre>

MQTT handshake goes:
<pre>
MQTT connects OK

Subscribing/publishing user topic
MQTT subscribes to Nuvoton/Mbed/+ OK
Message to publish:
{ "message": "Hello from Nuvoton Mbed device" }
MQTT publishes message to Nuvoton/Mbed/D001 OK
MQTT receives message with subscribed Nuvoton/Mbed/D001...
Message arrived: qos 1, retained 0, dup 0, packetid 1
Payload:
{ "message": "Hello from Nuvoton Mbed device" }
MQTT receives message with subscribed Nuvoton/Mbed/D001 OK

MQTT unsubscribes from Nuvoton/Mbed/+ OK
Subscribes/publishes user topic OK

Subscribing/publishing UpdateThingShadow topic
MQTT subscribes to $aws/things/Nuvoton-Mbed-D001/shadow/update/accepted OK
MQTT subscribes to $aws/things/Nuvoton-Mbed-D001/shadow/update/rejected OK
Message to publish:
{ "state": { "reported": { "attribute1": 3, "attribute2": "1" } } }
MQTT publishes message to $aws/things/Nuvoton-Mbed-D001/shadow/update OK
MQTT receives message with subscribed $aws/things/Nuvoton-Mbed-D001/shadow/update...
Message arrived: qos 1, retained 0, dup 0, packetid 1
Payload:
{"state":{"reported":{"attribute1":3,"attribute2":"1"}},"metadata":{"reported":{"attribute1":{"timestamp":1630637720},"attribute2":{"timestamp":1630637720}}},"version":229,"timestamp":1630637720}
MQTT receives message with subscribed $aws/things/Nuvoton-Mbed-D001/shadow/update OK

MQTT unsubscribes from $aws/things/Nuvoton-Mbed-D001/shadow/update/accepted OK
MQTT unsubscribes from $aws/things/Nuvoton-Mbed-D001/shadow/update/rejected OK
Subscribes/publishes UpdateThingShadow topic OK

Subscribing/publishing GetThingShadow topic
MQTT subscribes to $aws/things/Nuvoton-Mbed-D001/shadow/get/accepted OK
MQTT subscribes to $aws/things/Nuvoton-Mbed-D001/shadow/get/rejected OK
Message to publish:

MQTT publishes message to $aws/things/Nuvoton-Mbed-D001/shadow/get OK
MQTT receives message with subscribed $aws/things/Nuvoton-Mbed-D001/shadow/get...
Message arrived: qos 1, retained 0, dup 0, packetid 1
Payload:
{"state":{"reported":{"attribute1":3,"attribute2":"1"}},"metadata":{"reported":{"attribute1":{"timestamp":1630637720},"attribute2":{"timestamp":1630637720}}},"version":229,"timestamp":1630637722}
MQTT receives message with subscribed $aws/things/Nuvoton-Mbed-D001/shadow/get OK

MQTT unsubscribes from $aws/things/Nuvoton-Mbed-D001/shadow/get/accepted OK
MQTT unsubscribes from $aws/things/Nuvoton-Mbed-D001/shadow/get/rejected OK
Subscribes/publishes GetThingShadow topic OK

Subscribing/publishing DeleteThingShadow topic
MQTT subscribes to $aws/things/Nuvoton-Mbed-D001/shadow/delete/accepted OK
MQTT subscribes to $aws/things/Nuvoton-Mbed-D001/shadow/delete/rejected OK
Message to publish:

MQTT publishes message to $aws/things/Nuvoton-Mbed-D001/shadow/delete OK
MQTT receives message with subscribed $aws/things/Nuvoton-Mbed-D001/shadow/delete...
Message arrived: qos 1, retained 0, dup 0, packetid 1
Payload:
{"version":229,"timestamp":1630637724}
MQTT receives message with subscribed $aws/things/Nuvoton-Mbed-D001/shadow/delete OK

MQTT unsubscribes from $aws/things/Nuvoton-Mbed-D001/shadow/delete/accepted OK
MQTT unsubscribes from $aws/things/Nuvoton-Mbed-D001/shadow/delete/rejected OK
Subscribes/publishes DeleteThingShadow topic OK

MQTT disconnects OK
</pre>

## Trouble-shooting
-   Reduce memory footprint according to RFC 6066 TLS extension.
    We reduce memory footprint by:
    1. Enabling RFC 6066 max_fragment_length extension by configuing `my-tlssocket.tls-max-frag-len` to 4.

        `my-tlssocket/mbed_lib.json`:
        ```json
        {
            "name": "my-tlssocket",
            "config": {
                "tls-max-frag-len": {
                    "help": "Maximum fragment length value for the payload in one packet, doesn't include TLS header and encryption overhead. Is needed for constrained devices having low MTU sizes, Value 0 = disabled, 1 = MBEDTLS_SSL_MAX_FRAG_LEN_512, 2= MBEDTLS_SSL_MAX_FRAG_LEN_1024, 3 = MBEDTLS_SSL_MAX_FRAG_LEN_2048, 4 = MBEDTLS_SSL_MAX_FRAG_LEN_4096",
                    "value": 0
                },
            }
        }
        ```

        `mbed_app.json`:
        ```json
        "SOME_TARGET": {
            "my-tlssocket.tls-max-frag-len"         : 4,
        },
        ```

    1. Consistent with above, allocating these buffers with `MBEDTLS_SSL_IN_CONTENT_LEN`/`MBEDTLS_SSL_OUT_CONTENT_LEN` being larger than 4KiB/4KiB.

        `mbedtls_user_config.h`:
        ```C++
        /* Maximum length (in bytes) of incoming plaintext fragments */
        #define MBEDTLS_SSL_IN_CONTENT_LEN      8192 

        /* Maximum length (in bytes) of outgoing plaintext fragments */
        #define MBEDTLS_SSL_OUT_CONTENT_LEN     8192 
        ```

    **NOTE:**: With `my-tlssocket.tls-max-frag-len` being 4, `MBEDTLS_SSL_IN_CONTENT_LEN`/`MBEDTLS_SSL_OUT_CONTENT_LEN` must be larger than 4KiB/4KiB.
    We enlarge them to 8KiB/8KiB because TLS handshake also uses these buffers and may require larger.

    But this approach is risky because:
    1. AWS IoT doesn't support RFC 6066 TLS extension yet.
    1. TLS handshake may need larger I/O buffers than configured.

    If you doubt your trouble is caused by this configuration, disable it by:
    1.  Removing the line `my-tlssocket.tls-max-frag-len` in `mbed_app.json`.
    1.  Commenting out `MBEDTLS_SSL_IN_CONTENT_LEN`/`MBEDTLS_SSL_OUT_CONTENT_LEN` in `mbedtls_user_config.h`.
        This will change back to 16KiB/16KiB.
