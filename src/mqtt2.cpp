#include <iostream>

#include "mqtt2.h"


void user_callback::connection_lost(const std::string& cause) {
	std::cout << "\nConnection lost" << std::endl;
	if (!cause.empty())
		std::cout << "\tcause: " << cause << std::endl;
}

void user_callback::delivery_complete(mqtt::delivery_token_ptr tok) {
	//std::cout << "\n\t[Delivery complete for token: "
	//	<< (tok ? tok->get_message_id() : -1) << "]" << std::endl;
}

void MQTT::connect(const std::string& server, const std::string& client_id) {
	if (client)
		disconnect();

	//std::cout << "Initializing..." << std::endl;
	client = new mqtt::client(server, client_id);

	client->set_callback(cb);

	mqtt::connect_options connOpts;
	connOpts.set_keep_alive_interval(20);
	connOpts.set_clean_session(true);
	//std::cout << "...OK" << std::endl;

	//std::cout << "\nConnecting..." << std::endl;
	client->connect(connOpts);
	//std::cout << "...OK" << std::endl;
}

void MQTT::disconnect() {
	if (client) {
		//std::cout << "\nDisconnecting..." << std::endl;
		client->disconnect();
		//std::cout << "...OK" << std::endl;
		delete client;
	}
}

void MQTT::publish(const std::string& topic, const std::string& message, int QoS) {
	if (!client)
		return;

	auto pubmsg = mqtt::make_message(topic, message);
	pubmsg->set_qos(QoS);
	client->publish(pubmsg);
}

//
//int main() {
//	std::cout << "Initializing..." << std::endl;
//	mqtt::client client(SERVER_ADDRESS, CLIENT_ID);
//
//	user_callback cb;
//	client.set_callback(cb);
//
//	mqtt::connect_options connOpts;
//	connOpts.set_keep_alive_interval(20);
//	connOpts.set_clean_session(true);
//	std::cout << "...OK" << std::endl;
//
//	try {
//		std::cout << "\nConnecting..." << std::endl;
//		client.connect(connOpts);
//		std::cout << "...OK" << std::endl;
//
//		// First use a message pointer.
//
//		std::cout << "\nSending message..." << std::endl;
//		auto pubmsg = mqtt::make_message(TOPIC, "Hola");
//		pubmsg->set_qos(QOS);
//		client.publish(pubmsg);
//		std::cout << "...OK" << std::endl;
//
//		// Disconnect
//		std::cout << "\nDisconnecting..." << std::endl;
//		client.disconnect();
//		std::cout << "...OK" << std::endl;
//	}
//	catch (const mqtt::persistence_exception& exc) {
//		std::cerr << "Persistence Error: " << exc.what() << " ["
//			<< exc.get_reason_code() << "]" << std::endl;
//		return 1;
//	}
//	catch (const mqtt::exception& exc) {
//		std::cerr << exc.what() << std::endl;
//		return 1;
//	}
//
//	std::cout << "\nExiting" << std::endl;
//	return 0;
//}