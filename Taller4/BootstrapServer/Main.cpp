#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <string>
#include <cstring>
#include <iostream>

using namespace std;
using namespace sf;

#define NUM_PLAYERS 4

struct PeerInfo {
	string IP;
	unsigned short port;
};

int main()
{
	vector <PeerInfo> peers;
		
	TcpListener listener;
	Socket::Status listStatus = listener.listen(5000);
	if (listStatus != Socket::Done)
		cout << "port error" << endl;
	else
		cout << "port ok" << endl;

	//Fins que s'hagin connectat tots els peers anem escoltant
	for (int i = 0; i < NUM_PLAYERS; i++) {
		TcpSocket newSock;
		
		Socket::Status status = listener.accept(newSock);
		if (status == Socket::Done) {
			
			//Fem un packet amb tota la info dels peers anteriors que tenim
			int currentPeers = peers.size();
			Packet aPacket;
			aPacket << currentPeers;
			for (int j = 0; j < peers.size(); j++) {
				aPacket << peers[j].IP << peers[j].port;
			}

			//enviem tota aquesta info al peer que s'ha conectat
			newSock.send(aPacket);

			//Afegim el nou peer al array de peers
			string peerIp = newSock.getRemoteAddress().toString();
			PeerInfo newPeer = { peerIp, newSock.getRemotePort() };
			peers.push_back(newPeer);

			//desconectem aquest peer, ja no el necessitem
			newSock.disconnect();

			cout << "Connectado nuevo peer" << endl;
		}
		else {
			cout << "Error de conexion" << endl;
		}
	}
	listener.close();
	
	return 0;
}