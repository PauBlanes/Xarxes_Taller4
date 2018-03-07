#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <string>
#include <cstring>
#include <iostream>

using namespace std;
using namespace sf;

struct PeerInfo {
	string IP;
	unsigned short port;
};

int main()
{
	//ESTABLIR CONNEXIÓ
	TcpSocket sock;
	IpAddress ip = IpAddress::getLocalAddress();
	sock.connect(ip, 5000);

	Packet packet;
	sock.receive(packet);
	
	SocketSelector selector;
	int currentPeers; string newIp; unsigned short newPort;
	if (packet >> currentPeers){ //si ho hem rebut ok
		
		for (int i = 0; i < currentPeers; i++) {
			
			if (packet >> newIp && packet >> newPort) { //si ho hem rebut ok
				
				TcpSocket *newPeer = new TcpSocket;
				Socket::Status s = newPeer->connect(newIp, 5000); //teoricament el newPort esta b pero si el poso aqui no va
				if (s == Socket::Done) {
					selector.add(*newPeer);
					cout << "conectado al peer con ip : " << newIp << endl;
				}
				else {
					cout << "Error al conectar con peer con ip : " << newIp << endl;
				}
				delete newPeer;
			}
			else
				cout << "failed to extract peer data" << endl;			
		}
	}
	else
		cout << "failed to extract numPeers of packet" << endl;
	
		
	system("pause");
	
	return 0;
}