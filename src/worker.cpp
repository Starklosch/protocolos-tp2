
class ServerWorker {
	std::mutex queueMutex, workerMutex, callersMutex;
	std::condition_variable callers, worker;
	std::queue<std::function<void()>> writeQueue;

	void wait() {
		//std::cout << "Waiting for new tasks\n";
		std::unique_lock lk(workerMutex);
		worker.wait(lk);
	}

	void write_one() {
		std::lock_guard lk(queueMutex);
		//std::cout << "Signaling task end\n";
		writeQueue.front()();
		writeQueue.pop();
	}

	void add(const std::function<void()>& f) {
		std::lock_guard lk(queueMutex);
		writeQueue.push(f);
		//std::cout << "Signaling new task\n";
		worker.notify_one();
	}

	void add_and_wait(const std::function<void()>& f) {
		add(f);
		//std::cout << "Waiting task to end\n"; Algo anda mal con esto
		std::unique_lock lk(callersMutex);
		callers.wait(lk);
	}

public:
	ServerWorker(const opcua::Server& server) {
		std::thread([&]() {
			std::this_thread::sleep_for(1s);
			while (server.isRunning())
			{
				if (empty())
					wait();

				write_one();
				callers.notify_one();
			}
			}).detach();
	}

	template <typename T>
	void writeValueScalar(opcua::Node<opcua::Server>& node, T value) {
		add([value, &node]() {
			node.writeValueScalar(value);
			//std::cout << node.readBrowseName().getName() << " = " << value << "\n";
			});
	}

	bool empty() {
		std::lock_guard lk(queueMutex);
		return writeQueue.empty();
	}
};