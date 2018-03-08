#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <string>
#include <cstring>
#include <iostream>
#include <list>

using namespace std;
using namespace sf;

struct PeerInfo {
	string IP;
	unsigned int port;
};

#define OTHER_PLAYERS 2
void SendAllPlayers(string msg);
void ReceiveFromAll(std::vector<std::string>* aMensajes);
list <TcpSocket*> peers;

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
	
	
	int currentPeers; string newIp; unsigned int newPort;
	packet >> currentPeers;
	
	cout << "COMENZANDO CONEXION CON LOS " << currentPeers << " PEERS ACTUALES..." << endl;
	for (int i = 0; i < currentPeers; i++) {
		packet >> newIp >> newPort;
		
		TcpSocket *newPeer = new TcpSocket;
		Socket::Status s = newPeer->connect(newIp, newPort); //teoricament el newPort esta b pero si el poso aqui no va
		if (s == Socket::Done) {
			peers.push_back(newPeer);
			cout << "conectado al peer con ip : " << newIp << " en su puerto : " << newPort << endl;
		}
		else {
			cout << "Error al conectar con peer con ip : " << newIp << " en su puerto " << newPort << endl;
			delete newPeer;
		}						
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
				peers.push_back(newPeer);
				cout << "conectado al peer con ip : " << newPeer->getRemoteAddress() << endl;
			}
			else {
				cout << "error al aceptar nuevo peer" << endl;
				delete newPeer;
			}
			
		}
		listener.close();
	}
			
	cout << "TODOS LOS PEERS SE HAN CONECTADO CORRECTAMENTE!" << endl;

	//Posem a non-blocking tots els peers
	for (list<TcpSocket*>::iterator it = peers.begin(); it != peers.end(); ++it)
	{
		TcpSocket* clientRef = *it;
		clientRef->setBlocking(false);
		
	}

	//AQUI JA EL CHAT PROPIAMENT
	std::vector<std::string> aMensajes;

	sf::Vector2i screenDimensions(800, 600);

	sf::RenderWindow window;
	window.create(sf::VideoMode(screenDimensions.x, screenDimensions.y), "Chat");

	sf::Font font;
	if (!font.loadFromFile("courbd.ttf"))
	{
		std::cout << "Can't load the font file" << std::endl;
	}

	sf::String mensaje = " >";

	sf::Text chattingText(mensaje, font, 14);
	chattingText.setFillColor(sf::Color(0, 160, 0));
	chattingText.setStyle(sf::Text::Bold);


	sf::Text text(mensaje, font, 14);
	text.setFillColor(sf::Color(0, 160, 0));
	text.setStyle(sf::Text::Bold);
	text.setPosition(0, 560);

	sf::RectangleShape separator(sf::Vector2f(800, 5));
	separator.setFillColor(sf::Color(200, 200, 200, 255));
	separator.setPosition(0, 550);

	while (window.isOpen())
	{
		sf::Event evento;
		while (window.pollEvent(evento))
		{
			switch (evento.type)
			{
			case sf::Event::Closed:
				window.close();
				break;
			case sf::Event::KeyPressed:
				if (evento.key.code == sf::Keyboard::Escape)
					window.close();
				else if (evento.key.code == sf::Keyboard::Return)
				{
					aMensajes.push_back(mensaje);
					if (aMensajes.size() > 25)
					{
						aMensajes.erase(aMensajes.begin(), aMensajes.begin() + 1);
					}
					SendAllPlayers(mensaje);
					mensaje = ">";					
				}
				break;
			case sf::Event::TextEntered:
				if (evento.text.unicode >= 32 && evento.text.unicode <= 126)
					mensaje += (char)evento.text.unicode;
				else if (evento.text.unicode == 8 && mensaje.getSize() > 0)
					mensaje.erase(mensaje.getSize() - 1, mensaje.getSize());
				break;
			}
		}
		window.draw(separator);

		ReceiveFromAll(&aMensajes);

		for (size_t i = 0; i < aMensajes.size(); i++)
		{
			std::string chatting = aMensajes[i];
			chattingText.setPosition(sf::Vector2f(0, 20 * i));
			chattingText.setString(chatting);
			window.draw(chattingText);
		}
		std::string mensaje_ = mensaje + "_";
		text.setString(mensaje_);
		window.draw(text);


		window.display();
		window.clear();
	}
	system("pause");
	return 0;	
}

void SendAllPlayers(string msg) {
	for (list<TcpSocket*>::iterator it = peers.begin(); it != peers.end(); ++it)
	{
		TcpSocket& clientRef = **it;
		
		sf::Socket::Status status;
		string toSend = msg;
		size_t bytesSend;

		do
		{
			status = clientRef.send(toSend.c_str(), toSend.length() + 1, bytesSend);
			if (status == sf::Socket::Partial) {
				toSend = toSend.substr(bytesSend + 1, toSend.length() - bytesSend);
			}
		} while (status == sf::Socket::Partial);		
	}
}

void ReceiveFromAll(std::vector<std::string>* aMensajes) {
	
	for (list<TcpSocket*>::iterator it = peers.begin(); it != peers.end(); ++it)
	{
		TcpSocket& clientRef = **it;
		size_t received;
		sf::Socket::Status status;
		string tmp;
		char buffer[500];

		status = clientRef.receive(buffer, sizeof(buffer), received);

		switch (status)
		{
		case sf::Socket::Done:
			buffer[received] = '\0';
			tmp = buffer;
			aMensajes->push_back(tmp);
			break;

		case sf::Socket::Partial:
			break;
		case sf::Socket::Disconnected:
			tmp = "Disconnected";
			aMensajes->push_back(tmp);
			exit(0);
			break;
		case sf::Socket::Error:
			tmp = "Error to receive";
			aMensajes->push_back(tmp);
			break;
		default:
			break;
		}
	}

}