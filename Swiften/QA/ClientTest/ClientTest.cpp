#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include "Swiften/Client/Client.h"
#include "Swiften/Network/Timer.h"
#include "Swiften/EventLoop/MainEventLoop.h"
#include "Swiften/EventLoop/SimpleEventLoop.h"
#include "Swiften/Queries/Requests/GetRosterRequest.h"
#include "Swiften/Client/ClientXMLTracer.h"
#include "Swiften/Network/BoostIOServiceThread.h"
#include "Swiften/Network/MainBoostIOServiceThread.h"

using namespace Swift;

SimpleEventLoop eventLoop;

Client* client = 0;
bool reconnected = false;
bool rosterReceived = false;

void handleRosterReceived(boost::shared_ptr<Payload>) {
	if (reconnected) {
		rosterReceived = true;
		client->disconnect();
		eventLoop.stop();
	}
	else {
		reconnected = true;
		client->disconnect();
		client->connect();
	}
}

void handleConnected() {
	boost::shared_ptr<GetRosterRequest> rosterRequest(new GetRosterRequest(client));
	rosterRequest->onResponse.connect(boost::bind(&handleRosterReceived, _1));
	rosterRequest->send();
}

int main(int, char**) {
	char* jid = getenv("SWIFT_CLIENTTEST_JID");
	if (!jid) {
		std::cerr << "Please set the SWIFT_CLIENTTEST_JID environment variable" << std::endl;
		return -1;
	}
	char* pass = getenv("SWIFT_CLIENTTEST_PASS");
	if (!pass) {
		std::cerr << "Please set the SWIFT_CLIENTTEST_PASS environment variable" << std::endl;
		return -1;
	}

	client = new Swift::Client(JID(jid), String(pass));
	ClientXMLTracer* tracer = new ClientXMLTracer(client);
	client->onConnected.connect(&handleConnected);
	client->connect();

	{
		boost::shared_ptr<Timer> timer(new Timer(30000, &MainBoostIOServiceThread::getInstance().getIOService()));
		timer->onTick.connect(boost::bind(&SimpleEventLoop::stop, &eventLoop));
		timer->start();

		eventLoop.run();
	}

	delete tracer;
	delete client;
	return !rosterReceived;
}
