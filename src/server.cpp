#include <iostream>
#include <unordered_map>
#include <thread>
#include <queue>
#include <functional>
#include <open62541pp/open62541pp.hpp>

using namespace std::chrono_literals;

float getTemperature() {
	static int i = 0;
	if (i > 30)
		i = 20;
	return i++;
}

float getHumidity() {
	static int i = 0;
	if (i > 60)
		i = 50;
	return i++;
}

int getButton1() {
	static int i = 0;
	if (i > 3)
		i = 0;
	return i++;
}

int getButton2() {
	static bool b = false;
	b = !b;
	return b;
}



class ServerWorker {
	using Node = opcua::Node<opcua::Server>;

	std::mutex workerMutex;
	std::condition_variable worker;
	std::unordered_map<Node*, opcua::Variant> variables;
	std::atomic_bool modified = false;

	void wait() {
		//std::cout << "Waiting for new tasks\n";
		std::unique_lock lk(workerMutex);
		worker.wait(lk);
	}

	void add(Node* node, opcua::Variant v) {
		variables[node] = v;
		modified = true;
		//std::cout << "Signaling new task\n";
		worker.notify_one();
	}

public:
	ServerWorker(const opcua::Server& server) {
		std::thread([&]() {
			std::this_thread::sleep_for(1s);
			while (server.isRunning())
			{
				if (!modified)
					wait();

				for (auto& [node, value] : variables) {
					node->writeValue(value);
				}
			}
			}).detach();
	}

	template <typename T>
	void writeValueScalar(opcua::Node<opcua::Server>& node, T value) {
		add(&node, opcua::Variant::fromScalar<opcua::VariantPolicy::ReferenceIfPossible>(const_cast<T&>(value)));
	}
};

int main() {
    opcua::Server server;

    // Add a variable node to the Objects node
    opcua::Node parentNode(server, opcua::ObjectId::ObjectsFolder);

    opcua::Node temperatureNode = parentNode.addVariable({ 1, 1001 }, "temperature");
    opcua::Node humidityNode = parentNode.addVariable({ 1, 1002 }, "humidity");
    opcua::Node button1Node = parentNode.addVariable({ 1, 1003 }, "button1");
    opcua::Node button2Node = parentNode.addVariable({ 1, 1004 }, "button2");

	temperatureNode.writeValueScalar(getTemperature());
	humidityNode.writeValueScalar(getHumidity());

	std::thread dhtThread([&]() {
		std::this_thread::sleep_for(2s);
		while (server.isRunning())
		{
			temperatureNode.writeValueScalar(getTemperature());
			humidityNode.writeValueScalar(getHumidity());
			std::this_thread::sleep_for(2s);
		}
	});

	std::thread buttonThread([&]() {
		std::this_thread::sleep_for(2s);
		while (server.isRunning())
		{
			button1Node.writeValueScalar(getButton1());
			button2Node.writeValueScalar(getButton2());
			std::this_thread::sleep_for(500ms);
		}
	});

    server.run();
	dhtThread.join();
	buttonThread.join();
}