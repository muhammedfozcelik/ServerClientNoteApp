#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#define PORT 8081
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define MAX_NOTES 100

#pragma comment(lib, "ws2_32.lib")

char notes[MAX_NOTES][BUFFER_SIZE];
int note_count = 0;

void handle_client_request(int client_socket, char *buffer) {
    if (strncmp(buffer, "ADD ", 4) == 0) {
        if (note_count < MAX_NOTES) {
            strcpy(notes[note_count], buffer + 4);
            note_count++;
            send(client_socket, "Note added.\n", 12, 0);
        } else {
            send(client_socket, "Note limit reached.\n", 20, 0);
        }
    } else if (strncmp(buffer, "LIST", 4) == 0) {
    char response[BUFFER_SIZE] = "Notes:\n";
    for (int i = 0; i < note_count; i++) {
        char line[BUFFER_SIZE];
        // \n karakteri ekleniyor, her notu alt alta listeleyeceğiz
        sprintf(line, "%d: %s\n", i + 1, notes[i]); 
        strcat(response, line);
    }
    send(client_socket, response, strlen(response), 0);
} else if (strncmp(buffer, "EDIT ", 5) == 0) {
        int id;
        char new_note[BUFFER_SIZE];
        sscanf(buffer + 5, "%d %[^\n]", &id, new_note);
        if (id > 0 && id <= note_count) {
            strcpy(notes[id - 1], new_note);
            send(client_socket, "Note updated.\n", 14, 0);
        } else {
            send(client_socket, "Invalid note ID.\n", 17, 0);
        }
    } else {
        send(client_socket, "Invalid command.\n", 17, 0);
    }
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int client_sockets[MAX_CLIENTS] = {0};
    char buffer[BUFFER_SIZE];

    // Winsock başlatma
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed.\n");
        return 1;
    }

    // Socket oluşturma
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Socket creation failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bağlantı işlemi
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == SOCKET_ERROR) {
        printf("Bind failed with error: %d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    // Dinleme
    if (listen(server_fd, 3) == SOCKET_ERROR) {
        printf("Listen failed with error: %d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    printf("Server is running on port %d...\n", PORT);

    while (1) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        int max_sd = server_fd;

        // Bağlı istemcileri kontrol et
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] > 0) {
                FD_SET(client_sockets[i], &readfds);
            }
            if (client_sockets[i] > max_sd) {
                max_sd = client_sockets[i];
            }
        }

        // select() ile soketleri bekle
        if (select(max_sd + 1, &readfds, NULL, NULL, NULL) < 0) {
            printf("Select failed with error: %d\n", WSAGetLastError());
            break;
        }

        // Yeni bağlantılar için kabul et
        if (FD_ISSET(server_fd, &readfds)) {
            if ((client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) == INVALID_SOCKET) {
                printf("Accept failed with error: %d\n", WSAGetLastError());
                break;
            }
            // Boş bir soket bul ve onu ayarla
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (client_sockets[i] == 0) {
                    client_sockets[i] = client_fd;
                    break;
                }
            }
            printf("New client connected\n");
        }

        // İstemcilerden gelen veriyi işle
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (FD_ISSET(client_sockets[i], &readfds)) {
                int valread = recv(client_sockets[i], buffer, BUFFER_SIZE, 0);
                if (valread == 0) {
                    printf("Client disconnected\n");
                    closesocket(client_sockets[i]);
                    client_sockets[i] = 0;
                } else {
                    buffer[valread] = '\0';
                    handle_client_request(client_sockets[i], buffer);
                }
            }
        }
    }

    // Çıkış işlemleri
    closesocket(server_fd);
    WSACleanup();
    return 0;
}
