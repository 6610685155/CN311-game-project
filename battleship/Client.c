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

int canPlaceShip(
    char board[BOARD_SIZE][BOARD_SIZE],
    int row,
    int col,
    int size,
    char direction)
{
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


int fire(
    char board[BOARD_SIZE][BOARD_SIZE],
    int row,
    int col)
{
    if(board[row][col]=='S')
    {
        board[row][col]='X';
        return 1;
    }

    if(board[row][col]=='~')
    {
        board[row][col]='O';
        return 0;
    }

    return -1;
}

void playerTurn(char enemyBoard[BOARD_SIZE][BOARD_SIZE])
{
    int row,col;

    printf("\nYour Turn\n");

    scanf("%d %d",&row,&col);

    int result = fire(enemyBoard,row,col);

    if(result==1)
    {
        printf("HIT!\n");
    }
    else if(result==0)
    {
        printf("MISS!\n");
    }
    else
    {
        printf("Already Shot!\n");
    }
}


int main()
{
    char playerBoard[BOARD_SIZE][BOARD_SIZE];

    initBoard(playerBoard);

    setupShips(playerBoard);

    printBoard(playerBoard);

    return 0;
}