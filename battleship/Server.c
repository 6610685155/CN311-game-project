/* ============================================================
 *  Server.c  -  Battleship (PLAYER 1)
 *  Platform : Windows  (Winsock2)
 *
 *  Compile (MinGW / Dev-C++):
 *      gcc Server.c -o Server.exe -lws2_32
 *  Compile (MSVC):
 *      cl Server.c ws2_32.lib
 *
 *  Run:
 *      Server.exe
 *      -> waits for Player 2 (Client.exe) to connect on port 5000
 *
 *  How it works:
 *      - Server is Player 1, Client is Player 2.
 *      - Each program keeps ONLY its own board; nobody can see
 *        the opponent's ships.
 *      - On a turn the attacker sends a Shot {row,col}; the
 *        defender applies it to its own board and replies with
 *        a ShotResult {hit?, ships left}. The server fires first.
 * ============================================================ */

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")   /* MSVC auto-link; MinGW still needs -lws2_32 */

#define BOARD_SIZE 7
#define PORT       5000

/* ---------- messages exchanged over the socket ---------- */
typedef struct {
    int row;
    int col;
} Shot;

typedef struct {
    int result;          /* 1 = HIT , 0 = MISS                */
    int shipsRemaining;  /* defender's ship cells still alive */
} ShotResult;

/* ===================== BOARD HELPERS ===================== */

void initBoard(char board[BOARD_SIZE][BOARD_SIZE])
{
    for (int i = 0; i < BOARD_SIZE; i++)
        for (int j = 0; j < BOARD_SIZE; j++)
            board[i][j] = '~';
}

void printBoard(char board[BOARD_SIZE][BOARD_SIZE])
{
    printf("  ");
    for (int i = 0; i < BOARD_SIZE; i++)
        printf("%d ", i);
    printf("\n");

    for (int i = 0; i < BOARD_SIZE; i++)
    {
        printf("%d ", i);
        for (int j = 0; j < BOARD_SIZE; j++)
            printf("%c ", board[i][j]);
        printf("\n");
    }
}

int canPlaceShip(char board[BOARD_SIZE][BOARD_SIZE],
                 int row, int col, int size, char direction)
{
    if (row < 0 || row >= BOARD_SIZE) return 0;
    if (col < 0 || col >= BOARD_SIZE) return 0;

    if (direction == 'H')
    {
        if (col + size > BOARD_SIZE) return 0;
        for (int i = 0; i < size; i++)
            if (board[row][col + i] == 'S') return 0;
    }
    else
    {
        if (row + size > BOARD_SIZE) return 0;
        for (int i = 0; i < size; i++)
            if (board[row + i][col] == 'S') return 0;
    }
    return 1;
}

void placeShip(char board[BOARD_SIZE][BOARD_SIZE],
               int row, int col, int size, char direction)
{
    if (direction == 'H')
        for (int i = 0; i < size; i++)
            board[row][col + i] = 'S';
    else
        for (int i = 0; i < size; i++)
            board[row + i][col] = 'S';
}

void setupShips(char board[BOARD_SIZE][BOARD_SIZE])
{
    int ships[] = {4, 3, 2, 2};

    for (int s = 0; s < 4; s++)
    {
        int row, col;
        char dir;

        while (1)
        {
            printBoard(board);
            printf("\nPlace ship size %d\n", ships[s]);
            printf("Row Col Direction(H/V): ");
            scanf("%d %d %c", &row, &col, &dir);

            if (canPlaceShip(board, row, col, ships[s], dir))
            {
                placeShip(board, row, col, ships[s], dir);
                break;
            }
            printf("Invalid position!\n");
        }
    }
}

int shipsRemaining(char board[BOARD_SIZE][BOARD_SIZE])
{
    int count = 0;
    for (int i = 0; i < BOARD_SIZE; i++)
        for (int j = 0; j < BOARD_SIZE; j++)
            if (board[i][j] == 'S')
                count++;
    return count;
}

void printPlayerView(char ownBoard[BOARD_SIZE][BOARD_SIZE],
                     char shotBoard[BOARD_SIZE][BOARD_SIZE])
{
    printf("\n=== YOUR BOARD ===\n");
    printBoard(ownBoard);
    printf("\n=== ENEMY BOARD (your shots) ===\n");
    printBoard(shotBoard);
}

/* ==================== NETWORK HELPERS ====================
 * TCP is a byte stream: one send() may be split or merged
 * across several recv() calls. These helpers loop until the
 * whole fixed-size message has actually gone through.        */

int sendAll(SOCKET s, const void *buf, int len)
{
    const char *p = (const char *)buf;
    int sent = 0;
    while (sent < len)
    {
        int n = send(s, p + sent, len - sent, 0);
        if (n == SOCKET_ERROR || n == 0) return -1;
        sent += n;
    }
    return 0;
}

int recvAll(SOCKET s, void *buf, int len)
{
    char *p = (char *)buf;
    int got = 0;
    while (got < len)
    {
        int n = recv(s, p + got, len - got, 0);
        if (n == SOCKET_ERROR || n == 0) return -1;
        got += n;
    }
    return 0;
}

/* ==================== GAMEPLAY HELPERS ==================== */

/* Ask THIS player for a target until it is on the board and
 * has not been fired at before (checked against our shotBoard). */
void getValidShot(char shotBoard[BOARD_SIZE][BOARD_SIZE], int *row, int *col)
{
    while (1)
    {
        int c;
        printf("\nEnter row col to hit: ");

        if (scanf("%d %d", row, col) != 2)
        {
            while ((c = getchar()) != '\n' && c != EOF);  /* drop bad line */
            printf("Invalid input! Type two numbers: row col\n");
            continue;
        }
        if (*row < 0 || *row >= BOARD_SIZE ||
            *col < 0 || *col >= BOARD_SIZE)
        {
            printf("Out of board!\n");
            continue;
        }
        if (shotBoard[*row][*col] != '~')
        {
            printf("Already shot there!\n");
            continue;
        }
        return;
    }
}

/* Apply the opponent's shot to MY board.
 * Returns 1 if it hit one of my ships, 0 otherwise. */
int applyIncomingShot(char myBoard[BOARD_SIZE][BOARD_SIZE], int row, int col)
{
    if (myBoard[row][col] == 'S')
    {
        myBoard[row][col] = 'X';   /* my ship got hit       */
        return 1;
    }
    if (myBoard[row][col] == '~')
        myBoard[row][col] = 'o';   /* opponent missed water */
    return 0;
}

/* Pause so the player can read the screen before it clears. */
void waitEnter(const char *msg)
{
    int c;
    printf("%s", msg);
    while ((c = getchar()) != '\n' && c != EOF);  /* finish current line */
    getchar();                                    /* wait for Enter key  */
}

/* ========================== MAIN ========================== */

int main()
{
    WSADATA wsa;
    SOCKET  listenSock, connSock;
    struct sockaddr_in serverAddr, clientAddr;
    int clientLen = sizeof(clientAddr);

    char myBoard[BOARD_SIZE][BOARD_SIZE];   /* my ships          */
    char myShots[BOARD_SIZE][BOARD_SIZE];   /* my shots at enemy */

    initBoard(myBoard);
    initBoard(myShots);

    /* ----- 1. start Winsock ----- */
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("WSAStartup failed.\n");
        return 1;
    }

    /* ----- 2. create the listening socket ----- */
    listenSock = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSock == INVALID_SOCKET)
    {
        printf("socket() failed.\n");
        WSACleanup();
        return 1;
    }

    serverAddr.sin_family      = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;     /* listen on every NIC */
    serverAddr.sin_port        = htons(PORT);

    /* ----- 3. bind + listen ----- */
    if (bind(listenSock, (struct sockaddr *)&serverAddr,
             sizeof(serverAddr)) == SOCKET_ERROR)
    {
        printf("bind() failed.\n");
        closesocket(listenSock);
        WSACleanup();
        return 1;
    }
    if (listen(listenSock, 1) == SOCKET_ERROR)
    {
        printf("listen() failed.\n");
        closesocket(listenSock);
        WSACleanup();
        return 1;
    }

    printf("===== BATTLESHIP SERVER (PLAYER 1) =====\n");
    printf("Waiting for Player 2 to connect on port %d ...\n", PORT);

    /* ----- 4. accept exactly one client ----- */
    connSock = accept(listenSock, (struct sockaddr *)&clientAddr, &clientLen);
    if (connSock == INVALID_SOCKET)
    {
        printf("accept() failed.\n");
        closesocket(listenSock);
        WSACleanup();
        return 1;
    }
    printf("Player 2 connected!\n");
    closesocket(listenSock);          /* no more clients needed */

    /* ----- 5. place my ships ----- */
    printf("\n=== PLAYER 1 SETUP ===\n");
    setupShips(myBoard);
    system("cls");

    /* ----- 6. game loop: the server fires first every round ----- */
    while (1)
    {
        Shot shot;
        ShotResult res;
        int hit;

        /* ========== MY ATTACK TURN ========== */
        printf("========== PLAYER 1 (SERVER) TURN ==========\n");
        printPlayerView(myBoard, myShots);

        getValidShot(myShots, &shot.row, &shot.col);

        if (sendAll(connSock, &shot, sizeof(shot)) != 0)
        { printf("Connection lost.\n"); break; }

        if (recvAll(connSock, &res, sizeof(res)) != 0)
        { printf("Connection lost.\n"); break; }

        myShots[shot.row][shot.col] = res.result ? 'X' : 'O';
        printf("\nYou fired at (%d,%d) -> %s\n",
               shot.row, shot.col, res.result ? "HIT!" : "MISS!");
        printf("Enemy ship cells remaining: %d\n", res.shipsRemaining);

        if (res.shipsRemaining == 0)
        {
            printf("\n***  PLAYER 1 (SERVER) WINS!  ***\n");
            break;
        }

        printf("\nWaiting for Player 2's move...\n");

        /* ========== OPPONENT ATTACK TURN ========== */
        if (recvAll(connSock, &shot, sizeof(shot)) != 0)
        { printf("Connection lost.\n"); break; }

        hit = applyIncomingShot(myBoard, shot.row, shot.col);
        res.result         = hit;
        res.shipsRemaining = shipsRemaining(myBoard);

        if (sendAll(connSock, &res, sizeof(res)) != 0)
        { printf("Connection lost.\n"); break; }

        printf("Player 2 fired at (%d,%d) -> %s\n",
               shot.row, shot.col, hit ? "HIT" : "MISS");
        printf("Your ship cells remaining: %d\n", res.shipsRemaining);

        if (res.shipsRemaining == 0)
        {
            printf("\n***  PLAYER 2 (CLIENT) WINS!  ***\n");
            break;
        }

        waitEnter("\nPress Enter for your next turn...");
        system("cls");
    }

    /* ----- 7. clean up ----- */
    waitEnter("\nGame over. Press Enter to exit...");
    closesocket(connSock);
    WSACleanup();
    return 0;
}
