#include "main.h"
SGL_CONSOLE_FRAME

SOCKET server;

using std::cout;
using std::endl;

NEW_THREAD_FUNC(mainHandler) {
	char *str = (char *)param;
	SOCKET socket;
	memcpy(&socket, (char *)param + strlen((char *)param) + 1, sizeof(socket));

	struct JSON *json = readJson(str);
	string inst = getContent(json, "inst")->data.json_string;

	cout << inst << endl;

	freeJson(json);
	free(param);
	return 0;
}
NEW_THREAD_FUNC(singleMsg) {
	SOCKET tmp = *(SOCKET *)param;
	char buffer[256] = { 0 };
	char *buf;

	while (socketReceive(tmp, buffer, 256) != SG_CONNECTION_FAILED) {
		buf = buffer;
		while (buf[0]) {
			char *param = (char *)malloc(256);
			memcpy(param, buf, strlen(buf) + 1);
			memcpy(param + strlen(buf) + 1, &tmp, sizeof(tmp));
			createThread(mainHandler, param);
			buf += strlen(buf) + 1;
		}
		memset(buffer, 0, 256);
	}
	closeSocket(tmp);
	free(param);
	return 0;
}
NEW_THREAD_FUNC(socketResponse) {
	bool nodelay = true;
	while (1) {
		SOCKET connection = acceptOne(server);
		setsockopt(connection, SOL_SOCKET, TCP_NODELAY, (const char *)&nodelay, sizeof(bool));

		SOCKET *param = (SOCKET *)malloc(sizeof(SOCKET));
		*param = connection;
		createThread(singleMsg, param);
	}
	return 0;
}

void cmdProc(string cmd) {

}

int main() {
	server = createServer(4497);
	createThread(socketResponse, NULL);
	cout << "LS Server is on." << endl;

	while (1) {
		string buf;
		std::getline(std::cin, buf);
		cmdProc(buf);
	}
}
