#include <iostream>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "Request.hpp"

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;  
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t clientAddressLength;
    char buffer[1024];

    // Création du socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Erreur lors de la création du socket." << std::endl;
        return 1;
    }

    // Configuration de l'adresse du serveur
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(8080); // htons : converts bytes from host byte order to network byte order

    // Attachement du socket à l'adresse du serveur
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        std::cerr << "Erreur lors de l'attachement du socket à l'adresse." << std::endl;
        close(serverSocket);
        return 1;
    }

    // Ecoute des connexions entrantes
    if (listen(serverSocket, 10) == -1) {
        std::cerr << "Erreur lors de l'écoute des connexions entrantes." << std::endl;
        close(serverSocket);
        return 1;
    }

    std::cout << "Le serveur est en attente de connexions sur le port 8080..." << std::endl;

    while (true) {
        // Accepter une nouvelle connexion
        clientAddressLength = sizeof(clientAddress);
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
        if (clientSocket == -1) {
            std::cerr << "Erreur lors de l'acceptation de la connexion." << std::endl;
            close(serverSocket);
            return 1;
        }

        // Lire la requête du client
        ssize_t bytesRead = read(clientSocket, buffer, sizeof(buffer));
        if (bytesRead == -1) {
            std::cerr << "Erreur lors de la lecture de la requête du client." << std::endl;
            close(clientSocket);
            close(serverSocket);
            return 1;
        }

        // Vérifier si la méthode est GET (mais il faudra aussi gerer POST et DELETE)
        std::string request(buffer, bytesRead);
        std::string response;
        std::cout << "----------- NEW REQUEST ---------------- "<< std::endl;
        std::cout  << request << std::endl;
        std::cout << "---------------------------------------- "<< std::endl;
        Request *req = new Request(clientSocket, request);
        req->prepareResponse();

        ssize_t bytesSent = write(clientSocket, req->getResponse().c_str(), req->getResponse().length());

        std:: cout << std::endl << "Response: " << std::endl << req->getResponse() << std::endl;

        if (bytesSent == -1) {
            std::cerr << "Erreur lors de l'envoi de la réponse au client." << std::endl;
            close(clientSocket);
            close(serverSocket);
            return 1;
        }
        close(clientSocket);
    }
    // Fermer le socket du serveur
    close(serverSocket);

    return 0;
}
