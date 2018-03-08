#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <string>
#include <cstring>
#include <iostream>

using namespace std;
using namespace sf;

struct PeerInfo {
	string IP;
	unsigned int port;
};

#define OTHER_PLAYERS 2

int main()
{
	//ESTABLIR CONNEXIÓ
	TcpSocket sock;
	IpAddress ip = IpAddress::getLocalAddress();
	sock.connect(ip, 5000);

	Packet packet;
	sock.receive(packet);
	unsigned short myPort = sock.getLocalPort();	
	sock.disconnect();
	
	SocketSelector selector;
	int currentPeers; string newIp; unsigned int newPort;
	packet >> currentPeers;
	
	cout << "COMENZANDO CONEXION CON LOS " << currentPeers << " PEERS ACTUALES..." << endl;
	for (int i = 0; i < currentPeers; i++) {
		packet >> newIp >> newPort;
		
		TcpSocket *newPeer = new TcpSocket;
		Socket::Status s = newPeer->connect(newIp, newPort); //teoricament el newPort esta b pero si el poso aqui no va
		if (s == Socket::Done) {
			selector.add(*newPeer);
			cout << "conectado al peer con ip : " << newIp << " en el puerto " << newPort << endl;
		}
		else {
			cout << "Error de tipo " << s <<" al conectar con peer con ip : " << newIp << " en el puerto " << newPort << endl;
		}

		delete newPeer;					
	}
	//mentre no estiguem conectats a tots els altres peers fem aquesta rutina
	if (currentPeers < OTHER_PLAYERS) {
		TcpListener listener;
		Socket::Status listStatus = listener.listen(myPort);		
		if (listStatus != Socket::Done)
			cout << "port error" << endl;
		else
			cout << "port " << myPort << " ok" << endl;
		
		for (int i = 0; i < OTHER_PLAYERS - currentPeers; i++) {
			cout << "ESPERANDO NUEVOS JUGADORES..." << endl;
			TcpSocket *newPeer = new TcpSocket;
			Socket::Status s = listener.accept(*newPeer);
			if (s == Socket::Done) {
				selector.add(*newPeer);
				cout << "conectado al peer con ip : " << newPeer->getRemoteAddress() << endl;
			}
			else
				cout << "error al aceptar nuevo peer" << endl;
			delete newPeer;
		}
		listener.close();
	}
			
	cout << "DONE!" << endl;
	system("pause");
	return 0;
}