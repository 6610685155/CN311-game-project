#include <stdio.h>
#include <stdlib.h>

#define BOARD_SIZE 7

char board[BOARD_SIZE][BOARD_SIZE];

void initBoard(char board[BOARD_SIZE][BOARD_SIZE])
{
    for(int i=0;i<BOARD_SIZE;i++)
    {
        for(int j=0;j<BOARD_SIZE;j++)
        {
            board[i][j]='~';
        }
    }
}

void printBoard(char board[BOARD_SIZE][BOARD_SIZE])
{
    printf("  ");

    for(int i=0;i<BOARD_SIZE;i++)
    {
        printf("%d ",i);
    }

    printf("\n");

    for(int i=0;i<BOARD_SIZE;i++)
    {
        printf("%d ",i);

        for(int j=0;j<BOARD_SIZE;j++)
        {
            printf("%c ",board[i][j]);
        }

        printf("\n");
    }
}

int canPlaceShip(char board[BOARD_SIZE][BOARD_SIZE],int row,int col,int size,char direction)
{
    if(row < 0 || row >= BOARD_SIZE)
        return 0;

    if(col < 0 || col >= BOARD_SIZE)
        return 0;

    if(direction=='H')
    {
        if(col+size > BOARD_SIZE)
            return 0;

        for(int i=0;i<size;i++)
        {
            if(board[row][col+i]=='S')
                return 0;
        }
    }
    else
    {
        if(row+size > BOARD_SIZE)
            return 0;

        for(int i=0;i<size;i++)
        {
            if(board[row+i][col]=='S')
                return 0;
        }
    }

    return 1;
}

void placeShip(
    char board[BOARD_SIZE][BOARD_SIZE],
    int row,
    int col,
    int size,
    char direction)
{
    if(direction=='H')
    {
        for(int i=0;i<size;i++)
        {
            board[row][col+i]='S';
        }
    }
    else
    {
        for(int i=0;i<size;i++)
        {
            board[row+i][col]='S';
        }
    }
}

void setupShips(char board[BOARD_SIZE][BOARD_SIZE])
{

    int ships[] = {4,3,2,2};

    for(int s=0;s<4;s++)
    {
        int row,col;
        char dir;



        while(1)
        {
            printBoard(board);

            printf("\nPlace ship size %d\n",ships[s]);

            printf("Row Col Direction(H/V): ");
            scanf("%d %d %c",&row,&col,&dir);

            if(canPlaceShip(
                board,
                row,
                col,
                ships[s],
                dir))
            {
                placeShip(
                    board,
                    row,
                    col,
                    ships[s],
                    dir);

                break;
            }

            printf("Invalid position!\n");
        }
    }
}


int shipsRemaining(char board[BOARD_SIZE][BOARD_SIZE])
{
    int count = 0;

    for(int i=0;i<BOARD_SIZE;i++)
    {
        for(int j=0;j<BOARD_SIZE;j++)
        {
            if(board[i][j] == 'S')
            {
                count++;
            }
        }
    }

    return count;
}


int fireAt(
    char enemyBoard[BOARD_SIZE][BOARD_SIZE],
    char shotBoard[BOARD_SIZE][BOARD_SIZE],
    int row,
    int col)
{
    if(row < 0 || row >= BOARD_SIZE)
        return -2;

    if(col < 0 || col >= BOARD_SIZE)
        return -2;

    if(shotBoard[row][col] != '~')
        return -1;

    if(enemyBoard[row][col] == 'S')
    {
        enemyBoard[row][col] = 'X';
        shotBoard[row][col] = 'X';

        return 1;
    }

    shotBoard[row][col] = 'O';

    return 0;
}

void playerTurn(char enemyBoard[BOARD_SIZE][BOARD_SIZE],char shotBoard[BOARD_SIZE][BOARD_SIZE])
{
    int row,col;

    printf("\nYour Turn\n");

    scanf("%d %d",&row,&col);

    int result =fireAt(enemyBoard, shotBoard, row, col);

    if(result == -2)
    {
        printf("Out of board!\n");
    }
    else if(result == 1)
    {
        printf("HIT!\n");
    }
    else if(result == 0)
    {
        printf("MISS!\n");
    }
    else
    {
        printf("Already Shot!\n");
    }
}

void printEnemyBoard(char board[BOARD_SIZE][BOARD_SIZE])
{
    printf("  ");

    for(int i=0;i<BOARD_SIZE;i++)
    {
        printf("%d ",i);
    }

    printf("\n");

    for(int i=0;i<BOARD_SIZE;i++)
    {
        printf("%d ",i);

        for(int j=0;j<BOARD_SIZE;j++)
        {
            char c = board[i][j];

            if(c == 'S')
            {
                printf("~ ");
            }
            else
            {
                printf("%c ",c);
            }
        }

        printf("\n");
    }
}


void printPlayerView(
    char ownBoard[BOARD_SIZE][BOARD_SIZE],
    char shotBoard[BOARD_SIZE][BOARD_SIZE])
{
    printf("\n=== YOUR BOARD ===\n");
    printBoard(ownBoard);

    printf("\n=== ENEMY BOARD ===\n");
    printBoard(shotBoard);
}


int main()
{
    char player1Board[BOARD_SIZE][BOARD_SIZE];
    char player2Board[BOARD_SIZE][BOARD_SIZE];

    char player1Shots[BOARD_SIZE][BOARD_SIZE];
    char player2Shots[BOARD_SIZE][BOARD_SIZE];

    initBoard(player1Board);
    initBoard(player2Board);

    initBoard(player1Shots);
    initBoard(player2Shots);

    printf("=== PLAYER 1 SETUP ===\n");
    setupShips(player1Board);

    system("cls");

    printf("=== PLAYER 2 SETUP ===\n");
    setupShips(player2Board);

    system("cls");

    int currentPlayer = 1;

    while(1)
    {
        system("cls");

        if(currentPlayer == 1)
        {
            printf("===== PLAYER 1 TURN =====\n");

            printf(
                "\nEnemy ship cells remaining: %d\n",
                shipsRemaining(player2Board));

            printPlayerView(
                player1Board,
                player1Shots);

            playerTurn(
                player2Board,
                player1Shots);

            if(shipsRemaining(player2Board) == 0)
            {
                printf("\nPLAYER 1 WINS!\n");

                printf("\nPress Enter to exit...");
                getchar();
                getchar();

                break;
            }

            printf(
                "\nPress Enter and pass to Player 2..."
            );

            getchar();
            getchar();

            currentPlayer = 2;
        }
        else
        {
            printf("===== PLAYER 2 TURN =====\n");

            printf(
                "\nEnemy ship cells remaining: %d\n",
                shipsRemaining(player1Board));

            printPlayerView(
                player2Board,
                player2Shots);

            playerTurn(
                player1Board,
                player2Shots);

            if(shipsRemaining(player1Board) == 0)
            {
                printf("\nPLAYER 2 WINS!\n");

                printf("\nPress Enter to exit...");
                getchar();
                getchar();

                break;
            }

            printf(
                "\nPress Enter and pass to Player 1..."
            );

            getchar();
            getchar();

            currentPlayer = 1;
        }
    }

    return 0;
}