#include <mqtt/client.h>

class user_callback : public virtual mqtt::callback {
	void connection_lost(const std::string& cause) override;
	void delivery_complete(mqtt::delivery_token_ptr tok) override;
};

class MQTT {
	mqtt::client* client = nullptr;
	user_callback cb;

public:
	MQTT() = default;
	MQTT(MQTT& other) = delete;

	inline ~MQTT() {
		disconnect();
	}

	void connect(const std::string& server, const std::string& client_id);

	void disconnect();

	void publish(const std::string& topic, const std::string& message, int QoS = 1);
};