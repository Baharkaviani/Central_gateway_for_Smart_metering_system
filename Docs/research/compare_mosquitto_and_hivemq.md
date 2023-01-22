
links for mosquitto:
https://mosquitto.org/
https://mosquitto.org/download/

https://github.com/eclipse/mosquitto

https://test.mosquitto.org/


===============================================================================

Quick start for mosquitton: (github)
If you have installed a binary package the broker should have been started automatically. If not, it can be started with a basic configuration:

    mosquitto

Then use mosquitto_sub to subscribe to a topic:

    mosquitto_sub -t 'test/topic' -v


And to publish a message:

    mosquitto_pub -t 'test/topic' -m 'hello world'


===============================================================================

links for hivemq: 
https://www.hivemq.com
https://www.hivemq.com/hivemq/features/
https://www.hivemq.com/downloads/
https://www.hivemq.com/downloads/docker/

===============================================================================

links of comparison:

https://en.wikipedia.org/wiki/Comparison_of_MQTT_implementations
https://www.hivemq.com/blog/mqtt-broker-comparison-iot-application/
https://www.saashub.com/compare-mosquitto-vs-hivemq
https://tewarid.github.io/2019/03/21/comparison-of-mqtt-brokers.html