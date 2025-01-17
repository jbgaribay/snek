// snek.c (2024)
// By: Jesus Barocio

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/time.h>
#include <termios.h>
#include <time.h>

#define PORT 0x271
#define WIDTH 80
#define HEIGHT 22
#define BUFF_SIZE 16
#define TRUE 1
#define FALSE 0
#define FORE 'w'
#define BACK 's'
#define LEFT 'a'
#define RITE 'd'
#define FOOD '&'
#define SNAKE 'O'
#define EMPTY ' '

// node struct for linked list
typedef struct node {

    int pair[2];
    struct node* next;
} Node;

// position struct for food
typedef struct {
    int x;
    int y;
} Position;

void renderGameState(Node* head, int foodPos[2], int score);
void clearScreen();
Node* createNode(int* pair);
Node* updateGameState(char direction, Node* head, int foodPos[2], char* lastDirection, int* gameOver, int* score);
void placeFood(Node* head, int foodPos[2]);
void freeList(Node* head);
int kbhit();
void setNonBlockingInputMode();
void restoreTerminalSettings();
int oppositeDirection(char dir1, char dir2);


void clearScreen() {
    printf("\033[H\033[J"); // clear screen
}


void renderGameState(Node* head, int foodPos[2], int score) {
    clearScreen();
    
    char grid[HEIGHT][WIDTH];
    
    // grid 
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {

            grid[i][j] = EMPTY;

        }
    }

    // borders
    for (int j = 0; j < WIDTH; j++) {

        grid[0][j] = '-';
        grid[HEIGHT - 1][j] = '-';

    }
    
    for (int i = 1; i < HEIGHT - 1; i++) {

        grid[i][0] = '|';
        grid[i][WIDTH - 1] = '|';

    }

    // snake in grid
    Node* current = head;
    while (current != NULL) {

        if (current->pair[0] >= 1 && current->pair[0] < HEIGHT - 1 &&
            current->pair[1] >= 1 && current->pair[1] < WIDTH - 1) {
            grid[current->pair[0]][current->pair[1]] = SNAKE;
        }
        current = current->next;
    }

    // food in grid
    grid[foodPos[0]][foodPos[1]] = FOOD;

    // will make a grid 
    for (int i = 0; i < HEIGHT; i++) {

        for (int j = 0; j < WIDTH; j++) {

            printf("%c", grid[i][j]);

        }
        printf("\n");
    }
    
    // score
    printf("Score: %d\n", score);
}

Node* createNode(int* pair) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {

        perror("Failed to allocate memory for new node");
        exit(EXIT_FAILURE);

    }
    newNode->pair[0] = pair[0];
    newNode->pair[1] = pair[1];
    newNode->next = NULL;
    return newNode;
}

Node* updateGameState(char direction, Node* head, int foodPos[2], char* lastDirection, int* gameOver, int* score) {
    int newHead[2];

    if ((direction == FORE && *lastDirection != BACK) || (direction == BACK && *lastDirection != FORE)) {
        
        newHead[0] = head->pair[0] + (direction == FORE ? -1 : 1);
        newHead[1] = head->pair[1];

    } 
    else if ((direction == LEFT && *lastDirection != RITE) || (direction == RITE && *lastDirection != LEFT)) {
        
        newHead[0] = head->pair[0];
        newHead[1] = head->pair[1] + (direction == LEFT ? -1 : 1);

    } 
    else {

        // move in the last direction if invalid key is pressed
        direction = *lastDirection;
        newHead[0] = head->pair[0] + (direction == FORE ? -1 : (direction == BACK ? 1 : 0));
        newHead[1] = head->pair[1] + (direction == LEFT ? -1 : (direction == RITE ? 1 : 0));

    }

    if (newHead[0] <= 0 || newHead[0] >= HEIGHT - 1 || newHead[1] <= 0 || newHead[1] >= WIDTH - 1) {
        
        *gameOver = TRUE;
        return head;

    }

    Node* newHeadNode = createNode(newHead);
    newHeadNode->next = head;

    Node* current = newHeadNode->next;

    while (current->next != NULL) {

        if (current->pair[0] == newHead[0] && current->pair[1] == newHead[1]) {

            *gameOver = TRUE;
            break;
        }

        current = current->next;
    }

    if (!*gameOver && newHead[0] == foodPos[0] && newHead[1] == foodPos[1]) {

        *score += 100; 
        placeFood(newHeadNode, foodPos);
        
    } 
    
    else {

        current = newHeadNode;
        while (current->next->next != NULL) {
            current = current->next;
        }
        free(current->next);
        current->next = NULL;

    }

    *lastDirection = direction;

    return newHeadNode;
}

void placeFood(Node* head, int foodPos[2]) {
    int x, y;
    Node* current = head;
    int collision = 1;

    while (collision) {
        
        collision = 0;
        x = rand() % (HEIGHT - 2) + 1;
        y = rand() % (WIDTH - 2) + 1;

        while (current != NULL) {
            
            if (current->pair[0] == x && 
                current->pair[1] == y) {

                collision = 1;
                break;

            }
            current = current->next;
        }

        if (!collision) {

            foodPos[0] = x;
            foodPos[1] = y;

        }
    }
}

void freeList(Node* head) {

    Node* current = head;
    
    while (current != NULL) {
        Node* next = current->next;
        free(current);
        current = next;
    }

}

void setNonBlockingInputMode() {

    struct termios oldattr, newattr;
    tcgetattr(STDIN_FILENO, &oldattr);
    newattr = oldattr;
    newattr.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newattr);

}

void restoreTerminalSettings() {

    struct termios oldattr;
    tcgetattr(STDIN_FILENO, &oldattr);
    oldattr.c_lflag |= (ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);

}

int oppositeDirection(char dir1, char dir2) {
    
    return (dir1 == FORE && dir2 == BACK) ||
           (dir1 == BACK && dir2 == FORE) ||
           (dir1 == LEFT && dir2 == RITE) ||
           (dir1 == RITE && dir2 == LEFT);
}

int kbhit() {
   
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if(ch != EOF) {

        ungetc(ch, stdin);
        return 1;

    }

    return 0;
}

int main(int argc, char const *argv[]) {
    
    printf("%s, expects (1) arg, %d provided\n", argv[0], argc - 1);
    
    if (argc != 2) {

        printf("Usage: %s [option]\n", argv[0]);
        return 1;

    }

    if (argv[1][1] == 's') {

        printf("Starting server...\n");

        int serverSock, clientSock;
        struct sockaddr_in6 serverAddr, clientAddr;
        socklen_t clientAddrLen;

        serverSock = socket(AF_INET6, SOCK_STREAM, 0);
        if (serverSock == -1) {

            perror("Socket creation failed");
            return 1;

        }

        serverAddr.sin6_family = AF_INET6;
        serverAddr.sin6_port = htons(PORT);
        serverAddr.sin6_addr = in6addr_any;

        if (bind(serverSock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
           
            perror("Bind failed");
            return 1;

        }

        if (listen(serverSock, 1) == -1) {
            
            perror("Listen failed");
            return 1;

        }

        clientAddrLen = sizeof(clientAddr);
        clientSock = accept(serverSock, (struct sockaddr *)&clientAddr, &clientAddrLen);

        if (clientSock == -1) {

            perror("Accept failed");
            return 1;

        }

        while (1) {

            // Initialize seed
            srand(time(NULL));

            Node* gameState = createNode((int[]){HEIGHT / 2, WIDTH / 2});
            int foodPos[2] = {0};
            char lastDirection = FORE; // Initial direction
            int gameOver = FALSE;
            int score = 0;

            // Randomly place food
            placeFood(gameState, foodPos);

            while (!gameOver) {
                struct timeval timeout;
                timeout.tv_sec = 1;
                timeout.tv_usec = 0;

                fd_set readfds;
                FD_ZERO(&readfds);
                FD_SET(clientSock, &readfds);

                int activity = select(clientSock + 1, &readfds, NULL, NULL, &timeout);
                
                if (activity == -1) {

                    perror("Select error");
                    break;

                } 
                else if (activity == 0) {
                    
                    // continue moving in the last direction, also a part of google snake, so players cant just freeze mid game
                    gameState = updateGameState(lastDirection, gameState, foodPos, &lastDirection, &gameOver, &score);
                    renderGameState(gameState, foodPos, score);
                    continue;

                }

                char command;
                if (recv(clientSock, &command, sizeof(command), 0) == -1) {
                    
                    perror("Receive failed");
                    break;

                }

                // check for invalid moves, in the google version of snake, the snake can't move in the opposite direction. (e.g. can't move up if it's moving down)
                if (!oppositeDirection(lastDirection, command)) {
                    
                    gameState = updateGameState(command, gameState, foodPos, &lastDirection, &gameOver, &score);
                    renderGameState(gameState, foodPos, score);

                }
            }

            printf("Game over!\n");
            printf("Final Score: %d\n", score);
            freeList(gameState);

            printf("Press 'r' to restart or any other key to exit: ");
            char restart;
            scanf(" %c", &restart);
            
            if (restart != 'r') {
                break; 
            }
        }

        close(clientSock);
        close(serverSock);
    }

    else if (argv[1][1] == 'c') {
        printf("Starting client...\n");

        int clientSock;
        struct sockaddr_in6 serverAddr;

        clientSock = socket(AF_INET6, SOCK_STREAM, 0);
        if (clientSock == -1) {

            perror("Socket creation failed");
            return 1;

        }

        serverAddr.sin6_family = AF_INET6;
        serverAddr.sin6_port = htons(PORT);
        inet_pton(AF_INET6, "::1", &serverAddr.sin6_addr);

        if (connect(clientSock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
            
            perror("Connection failed");
            return 1;

        }

        Node* gameState = createNode((int[]){HEIGHT / 2, WIDTH / 2});
        int foodPos[2] = {0};

        setNonBlockingInputMode();

        printf("Press 'q' to quit.\n");

        while (1) {

            if (kbhit()) {
                char command;
                scanf(" %c", &command);

                if (command == 'q') {
                   
                    printf("Quitting...\n");
                    break;

                }

                if (send(clientSock, &command, sizeof(command), 0) == -1) {
                    
                    perror("Send failed");
                    break;

                }
            }
            usleep(100000);
        }

        restoreTerminalSettings();
        close(clientSock);
    }

    else if (argv[1][1] == 'h') {

        printf("HELP: Usage:  -s for server, -c for client\n");

    }

    else {

        printf("Invalid option. Use -h for help.\n");
        return 1;

    }

    return 0;
}
