#include <stdio.h>
#include <stdlib.h> 
#include <windows.h>
#include <conio.h>


#define ESC 27 // Escape key


// A linked list (for snake body) 
typedef struct Node
{
    COORD coord;
    struct Node *next;
} 
Node;


// An apple
typedef struct Apple
{
    COORD coord;
}
Apple;


// Console screen options
static SMALL_RECT windowSize_ = {0, 0, 40, 20};
static COORD bufferSize_ = {41, 21};



static int SnakeEatsApple(Node *snakeHead, Apple *apple)
{
    if ((snakeHead->coord.X == apple->coord.X) && (snakeHead->coord.Y == apple->coord.Y))
        return 1;
    else
    {
        return 0;
    }
}



static int SnakeBitesItself(Node *snakeBody, Node *snakeHead)
{
    Node *tail;


    tail = snakeBody;

    while (tail->next != NULL)
    {
        if ((snakeHead->coord.X == tail->coord.X) && (snakeHead->coord.Y == tail->coord.Y))
        {
            return 1;
        }
        tail = tail->next;
    }

    return 0;
}



static Apple *PlaceNewApple(HANDLE hStdout)
{
    Apple *apple;
    DWORD n_chars_written;

    apple = NULL;
    apple = (Apple*)calloc(1, sizeof(Apple));
    apple->coord.X = rand() % (bufferSize_.X - 1) + 1;
    apple->coord.Y = rand() % (bufferSize_.Y - 1) + 1;

    FillConsoleOutputCharacter(hStdout, 'A', 1, apple->coord, &n_chars_written);

    return apple;
}



static void DeleteApple(HANDLE hStdout, Apple *apple)
{
    DWORD n_chars_written;

    FillConsoleOutputCharacter(hStdout, ' ', 1, apple->coord, &n_chars_written);
    free(apple);
}



static void UpdateScore(HANDLE hStdout, COORD cursorPos, SHORT snakeLen)
{
    DWORD n_chars_written;

    SetConsoleCursorPosition(hStdout, cursorPos);
    FillConsoleOutputCharacter(hStdout, ' ', 1, cursorPos, &n_chars_written);
    printf("%d", snakeLen - 1);

    return;
}



static void Hidecursor(HANDLE hStdout)
{
   CONSOLE_CURSOR_INFO info;

   info.dwSize = 100;
   info.bVisible = FALSE;
   SetConsoleCursorInfo(hStdout, &info);
   return;
}



static COORD GetCursorPosition(HANDLE hStdout)
{
    CONSOLE_SCREEN_BUFFER_INFO csb_info;
    COORD write_pos;
    
    GetConsoleScreenBufferInfo(hStdout, &csb_info);
    write_pos.X = csb_info.dwCursorPosition.X;
    write_pos.Y = csb_info.dwCursorPosition.Y;

    return write_pos;
}



static void AppendNewSnakeHeadToBody(Node **headRef, Node **appendedNode) 
{ 
    Node *last;


    if (*headRef == NULL) 
    { 
       *headRef = *appendedNode; 
       return; 
    }

    last = *headRef;
 
    while (last->next != NULL) 
    {
        last = last->next; 
    }

    last->next = *appendedNode; 
    return;
} 



static void DeleteFirstNodeInBody(HANDLE hStdout, Node **headRef)
{
    Node *to_delete;
    DWORD n_chars_written;


    if(*headRef != NULL)
    {
        to_delete = *headRef;
        *headRef =  (*headRef)->next;

        FillConsoleOutputCharacter(hStdout, ' ', 1, to_delete->coord, &n_chars_written);
        free(to_delete);
    }

    return;
}



static void TeleportIfWallHit(COORD bufferSize, SHORT *x, SHORT *y)
{
    if (*x >= bufferSize.X)          *x = 1;
    else if (*x < 1)                 *x = bufferSize.X - 1;
    else if (*y >= bufferSize.Y)     *y = 1;
    else if (*y < 1)                 *y = bufferSize.Y - 1;

    return;
}



static void GameOver(HANDLE hStdout, COORD bufferSize, int snakeLen)
{
    COORD print = {bufferSize.X/2 - 4, bufferSize.Y/3};

    system("cls");

    SetConsoleCursorPosition(hStdout, print);
    printf("GAME OVER\n\n");

    print = GetCursorPosition(hStdout);
    print.X = bufferSize.X/2 - 4;

    SetConsoleCursorPosition(hStdout, print);
    printf("Score: %d", snakeLen - 1);

    Sleep(5000);
}



static Node* UpdateSnakeHeadPosition(SHORT *x, SHORT *y)
{
    Node *snake_head_updated;


    snake_head_updated = (Node*)calloc(1, sizeof(Node));

    TeleportIfWallHit(bufferSize_, x, y);

    snake_head_updated->coord.X = *x;
    snake_head_updated->coord.Y = *y;

    return snake_head_updated;
}



static void PrintSnakeHeadPosition(HANDLE hStdout, Node* snakeHead)
{
    DWORD n_chars_written;

    FillConsoleOutputCharacter(hStdout, 'S', 1, snakeHead->coord, &n_chars_written);
}


static void WaitForFirstInput()
{
    int wait_for_first_input = 1;

    while (wait_for_first_input)
    {
        if (_kbhit())
        {
            wait_for_first_input = 0;
        }
    }

    return;
}


static void SetConsoleWindowSettings(HANDLE hStdout)
{
    SetConsoleWindowInfo(hStdout, TRUE, &windowSize_);
    SetConsoleTitleA("Snake");
    SetConsoleScreenBufferSize(hStdout, bufferSize_);

    Hidecursor(hStdout);

    printf("Score: ");
}



int main(void)
{
    HANDLE hStdout;
    COORD cursor_pos_score;
    Node *snake_head, *snake_head_updated, *list_head;
    Apple *apple;
    SHORT x, y, x_change, y_change, snake_len;
    int wait_for_first_input = 1;
    int game_over = 0;


    srand ( time(NULL) );
    x_change = y_change = 0;
    snake_len = 1;
    snake_head = snake_head_updated = list_head = NULL;
    apple = NULL;

    hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

    SetConsoleWindowSettings(hStdout);
    cursor_pos_score = GetCursorPosition(hStdout);
    UpdateScore(hStdout, cursor_pos_score, 1);


    x = rand() % (bufferSize_.X - 1) + 1;
    y = rand() % (bufferSize_.Y - 1) + 1;
    snake_head = UpdateSnakeHeadPosition(&x, &y);
    list_head = snake_head;
    PrintSnakeHeadPosition(hStdout, snake_head);


    apple = PlaceNewApple(hStdout);

    WaitForFirstInput();

    while (!game_over) 
    {

        if (_kbhit())
        {
            switch (_getch())
            {
            case 'a':
                x_change = -1;
                y_change = 0;
                break;
            case 'd':
                x_change = 1;
                y_change = 0;
                break;
            case 'w':
                x_change = 0;
                y_change = -1;
                break;
            case 's':
                x_change = 0;
                y_change = 1;
                break;
            case ESC:
                game_over = 1;
                break;
            default:
                break;
            }
        }

        x += x_change;
        y += y_change;

        snake_head_updated = UpdateSnakeHeadPosition(&x, &y);

        AppendNewSnakeHeadToBody(&list_head, &snake_head_updated);

        if (snake_len > 1)
        {
            if (SnakeBitesItself(list_head, snake_head_updated))
            {    
                game_over = 1;
                break;
            } 
        }

        if (SnakeEatsApple(snake_head_updated, apple))
        {
            DeleteApple(hStdout, apple);

            apple = PlaceNewApple(hStdout);

            snake_len += 1;
            UpdateScore(hStdout, cursor_pos_score, snake_len);
        }
        else
        {
            DeleteFirstNodeInBody(hStdout, &list_head);
        }

        
        PrintSnakeHeadPosition(hStdout, snake_head_updated);

        Sleep(100);
    }

    GameOver(hStdout, bufferSize_, snake_len);

    return 0;
}