#include <iostream>

//#include <open62541pp/open62541pp.hpp>
#include <mqtt/client.h>

const std::string SERVER_ADDRESS{ "mqtt://localhost:1883" };
const std::string CLIENT_ID{ "sync_publish_cpp" };
const std::string TOPIC{ "hello" };

const int QOS = 1;

class user_callback : public virtual mqtt::callback
{
	void connection_lost(const std::string& cause) override {
		std::cout << "\nConnection lost" << std::endl;
		if (!cause.empty())
			std::cout << "\tcause: " << cause << std::endl;
	}

	void delivery_complete(mqtt::delivery_token_ptr tok) override {
		std::cout << "\n\t[Delivery complete for token: "
			<< (tok ? tok->get_message_id() : -1) << "]" << std::endl;
	}

public:
};

int main() {
	std::cout << "Initializing..." << std::endl;
	mqtt::client client(SERVER_ADDRESS, CLIENT_ID);

	user_callback cb;
	client.set_callback(cb);

	mqtt::connect_options connOpts;
	connOpts.set_keep_alive_interval(20);
	connOpts.set_clean_session(true);
	std::cout << "...OK" << std::endl;

	try {
		std::cout << "\nConnecting..." << std::endl;
		client.connect(connOpts);
		std::cout << "...OK" << std::endl;

		// First use a message pointer.

		std::cout << "\nSending message..." << std::endl;
		auto pubmsg = mqtt::make_message(TOPIC, "Hola");
		pubmsg->set_qos(QOS);
		client.publish(pubmsg);
		std::cout << "...OK" << std::endl;

		// Disconnect
		std::cout << "\nDisconnecting..." << std::endl;
		client.disconnect();
		std::cout << "...OK" << std::endl;
	}
	catch (const mqtt::persistence_exception& exc) {
		std::cerr << "Persistence Error: " << exc.what() << " ["
			<< exc.get_reason_code() << "]" << std::endl;
		return 1;
	}
	catch (const mqtt::exception& exc) {
		std::cerr << exc.what() << std::endl;
		return 1;
	}

	std::cout << "\nExiting" << std::endl;
	return 0;
}