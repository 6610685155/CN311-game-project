/* ============================================================
 *  Client.c  -  Battleship (PLAYER 2)
 *  Platform : Windows  (Winsock2)
 *
 *  Compile (MinGW / Dev-C++):
 *      gcc Client.c -o Client.exe -lws2_32
 *  Compile (MSVC):
 *      cl Client.c ws2_32.lib
 *
 *  Run:
 *      Client.exe                 -> connects to 127.0.0.1 (same PC test)
 *      Client.exe 192.168.1.20    -> connects to the server's IP address
 *
 *  How it works:
 *      - Server is Player 1, Client is Player 2.
 *      - Each program keeps ONLY its own board; nobody can see
 *        the opponent's ships.
 *      - The server fires first, so the client DEFENDS first
 *        each round and then takes its own shot.
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

int main(int argc, char *argv[])
{
    WSADATA wsa;
    SOCKET  sock;
    struct sockaddr_in serverAddr;
    const char *serverIp = "127.0.0.1";   /* default: same computer */

    char myBoard[BOARD_SIZE][BOARD_SIZE];   /* my ships          */
    char myShots[BOARD_SIZE][BOARD_SIZE];   /* my shots at enemy */

    if (argc >= 2)
        serverIp = argv[1];                 /* Client.exe <server-ip> */

    initBoard(myBoard);
    initBoard(myShots);

    /* ----- 1. start Winsock ----- */
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("WSAStartup failed.\n");
        return 1;
    }

    /* ----- 2. create the socket ----- */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    {
        printf("socket() failed.\n");
        WSACleanup();
        return 1;
    }

    serverAddr.sin_family      = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(serverIp);
    serverAddr.sin_port        = htons(PORT);

    printf("===== BATTLESHIP CLIENT (PLAYER 2) =====\n");
    printf("Connecting to server %s:%d ...\n", serverIp, PORT);

    /* ----- 3. connect to the server ----- */
    if (connect(sock, (struct sockaddr *)&serverAddr,
                sizeof(serverAddr)) == SOCKET_ERROR)
    {
        printf("connect() failed. Is Server.exe running and the IP correct?\n");
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    printf("Connected to server!\n");

    /* ----- 4. place my ships ----- */
    printf("\n=== PLAYER 2 SETUP ===\n");
    setupShips(myBoard);
    system("cls");

    /* ----- 5. game loop: the server fires first, so we defend first ----- */
    while (1)
    {
        Shot shot;
        ShotResult res;
        int hit;

        /* ========== OPPONENT ATTACK TURN ========== */
        printf("========== PLAYER 2 (CLIENT) ==========\n");
        printf("Waiting for Player 1's move...\n");

        if (recvAll(sock, &shot, sizeof(shot)) != 0)
        { printf("Connection lost.\n"); break; }

        hit = applyIncomingShot(myBoard, shot.row, shot.col);
        res.result         = hit;
        res.shipsRemaining = shipsRemaining(myBoard);

        if (sendAll(sock, &res, sizeof(res)) != 0)
        { printf("Connection lost.\n"); break; }

        printf("Player 1 fired at (%d,%d) -> %s\n",
               shot.row, shot.col, hit ? "HIT" : "MISS");
        printf("Your ship cells remaining: %d\n", res.shipsRemaining);

        if (res.shipsRemaining == 0)
        {
            printf("\n***  PLAYER 1 (SERVER) WINS!  ***\n");
            break;
        }

        /* ========== MY ATTACK TURN ========== */
        printf("\n========== PLAYER 2 (CLIENT) TURN ==========\n");
        printPlayerView(myBoard, myShots);

        getValidShot(myShots, &shot.row, &shot.col);

        if (sendAll(sock, &shot, sizeof(shot)) != 0)
        { printf("Connection lost.\n"); break; }

        if (recvAll(sock, &res, sizeof(res)) != 0)
        { printf("Connection lost.\n"); break; }

        myShots[shot.row][shot.col] = res.result ? 'X' : 'O';
        printf("\nYou fired at (%d,%d) -> %s\n",
               shot.row, shot.col, res.result ? "HIT!" : "MISS!");
        printf("Enemy ship cells remaining: %d\n", res.shipsRemaining);

        if (res.shipsRemaining == 0)
        {
            printf("\n***  PLAYER 2 (CLIENT) WINS!  ***\n");
            break;
        }

        waitEnter("\nPress Enter for the next round...");
        system("cls");
    }

    /* ----- 6. clean up ----- */
    waitEnter("\nGame over. Press Enter to exit...");
    closesocket(sock);
    WSACleanup();
    return 0;
}
