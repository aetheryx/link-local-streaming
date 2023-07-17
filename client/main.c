#include "raylib.h"
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>

Image imageBuffer;
Texture displayTexture;

void ticker(void *vargp) {
  int socket_desc;
  struct sockaddr_in server_addr;

  socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_desc < 0){
        printf("Unable to create socket\n");
        return;
    }
      printf("Socket created successfully\n");
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(27015);
    server_addr.sin_addr.s_addr = inet_addr("169.254.216.233");
    
    if(connect(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        printf("Unable to connect\n");
        return;
    }
    printf("Connected with server successfully\n");

    while (true) {
      int count = recv(socket_desc, imageBuffer.data, 19906560, MSG_WAITALL);
      if(count < 0){
        printf("Error while receiving server's msg\n");
        return;
      }
    }

}

int main() {
	InitWindow(3456, 1440, "link local streaming poc");
	SetTargetFPS(144);
  ToggleFullscreen();

  imageBuffer.data = malloc(19906560);
  imageBuffer.height = 1440;
  imageBuffer.width = 3456;
  imageBuffer.mipmaps = 1;
  imageBuffer.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

	displayTexture = LoadTextureFromImage(imageBuffer);

  pthread_t threadID;
  pthread_create(&threadID, NULL, ticker, NULL);

	while (!WindowShouldClose()) {
		BeginDrawing();
		DrawTexture(displayTexture, 0, 0, WHITE);
    DrawFPS(100, 100);
		EndDrawing();

    UpdateTexture(displayTexture, imageBuffer.data);
  }

	UnloadImage(imageBuffer);
	UnloadTexture(displayTexture);
	CloseWindow();

	return 0;
}