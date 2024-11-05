#include <iostream>

#include <open62541pp/open62541pp.hpp>
#include "mqtt2.h"


template <typename T, typename S>
void subscribeToPublishMQTT(opcua::Subscription<S>& sub, const opcua::NodeId& id, MQTT& mqtt, const std::string& topic, const std::function<std::string(T)>& to_string) {
    {
        S& client = sub.connection();
        T initialValue = client.getNode(id).readValueScalar<T>();
        mqtt.publish(topic, to_string(initialValue));
    }
    
    opcua::MonitoringParametersEx monitoringParameters{};
    monitoringParameters.samplingInterval = 100.0;

    auto mon = sub.subscribeDataChange(
        id,  // monitored node id
        opcua::AttributeId::Value,  // monitored attribute
        opcua::MonitoringMode::Reporting,
        monitoringParameters,
        [&, topic, to_string](uint32_t subId, uint32_t monId, const opcua::DataValue& dataValue) {
            const S& client = sub.connection();
            opcua::MonitoredItem item(client, subId, monId);
            const T value = dataValue.getValue().getScalar<T>();
            std::cout << std::format("{} = {}\n", topic, value);

            mqtt.publish(topic, to_string(value));
        }
    );
}

template <typename T>
std::string to_string(T value) {
	return std::format("{}", value);
}

template <>
std::string to_string<float>(float value) {
	return std::format("{:.1f}", value);
}

int main() {
    opcua::Client client;
    MQTT mqtt;

    //std::cout << "Server date (UTC): " << dt.format("%Y-%m-%d %H:%M:%S") << std::endl;

    client.onSessionActivated([&] {
        mqtt.connect("mqtt://localhost:1883", "OPC-MQTT");

        auto sub = client.createSubscription();

        // Modify and delete the subscription via the returned Subscription<T> object
        opcua::SubscriptionParameters subscriptionParameters{};
        subscriptionParameters.publishingInterval = 1000.0;
        sub.setSubscriptionParameters(subscriptionParameters);
        sub.setPublishingMode(true);
        // sub.deleteSubscription();



        // Create a monitored item within the subscription for data change notifications
		subscribeToPublishMQTT<float>(sub, { 1, 1001 }, mqtt, "temperature", to_string<float>);
		subscribeToPublishMQTT<float>(sub, { 1, 1002 }, mqtt, "humidity", to_string<float>);
		subscribeToPublishMQTT<int>(sub, { 1, 1003 }, mqtt, "button1", to_string<int>);
		subscribeToPublishMQTT<int>(sub, { 1, 1004 }, mqtt, "button2", to_string<int>);
        //std::cout << "Subscribing\n";
        //auto mon = sub.subscribeDataChange(
        //    { 1, 1003 },  // monitored node id
        //    opcua::AttributeId::Value,  // monitored attribute
        //    [&](uint32_t subId, uint32_t monId, const opcua::DataValue& dataValue) {
        //        opcua::MonitoredItem item(client, subId, monId);
        //        const auto value = dataValue.getValue().getScalar<bool>();
        //        std::cout << "Changed to " << value << '\n';

        //        //mqtt.publish(topic, std::to_string(value));
        //    }
        //);
        //// Modify and delete the monitored item via the returned MonitoredItem<T> object
        //opcua::MonitoringParametersEx monitoringParameters{};
        //monitoringParameters.samplingInterval = 100.0;
        //mon.setMonitoringParameters(monitoringParameters);
        //mon.setMonitoringMode(opcua::MonitoringMode::Reporting);
    });

    while (true) {
        try {
            client.connect("opc.tcp://localhost:4840");
            // Run the client's main loop to process callbacks and events.
            // This will block until client.stop() is called or an exception is thrown.
            client.run();
        }
        catch (const opcua::BadStatus& e) {
            // Workaround to enforce a new session
            // https://github.com/open62541pp/open62541pp/issues/51
            client.disconnect();
            std::cout << "Error: " << e.what() << "\nRetry to connect in 3 seconds\n";
            std::this_thread::sleep_for(std::chrono::seconds(3));
        }
    }
}