#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

using namespace std;
static const int TR[52] = {
    6,6,6,6,6,6,
    5,4,3,2,1,0,
    0,0,
    1,2,3,4,5,6,
    6,6,6,6,6,6,
    7,8,
    8,8,8,8,8,
    9,10,11,12,13,14,
    14,14,
    13,12,11,10,9,
    8,8,8,8,8,8
};

static const int TC[52] = {
    0,1,2,3,4,5,
    6,6,6,6,6,6,
    7,8,
    8,8,8,8,8,8,
    9,10,11,12,13,14,
    14,14,
    13,12,11,10,9,
    8,8,8,8,8,8,
    7,6,
    6,6,6,6,6,
    5,4,3,2,1,0
};

static const int ENTRY[2] = { 0, 20 };

static const int SAFE[8] = { 0, 8, 13, 20, 28, 33, 39, 46 };

static const int HP_R[2][5] = { {7,7,7,7,7}, {7,7,7,7,7} };
static const int HP_C[2][5] = { {1,2,3,4,5}, {13,12,11,10,9} };

static const int RY_R[4] = { 1,1,3,3 };
static const int RY_C[4] = { 1,4,1,4 };
static const int YY_R[4] = { 1,1,3,3 };
static const int YY_C[4] = { 10,13,10,13 };

char playerName[2][30];
int  tokenPos[2][4];
int  score[2];
int  currentPlayer;
int  diceValue;
int  gameOver;
int  extraTurn;

void gotoxy(int x, int y)
{
    COORD c;
    c.X = (SHORT)x;
    c.Y = (SHORT)y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
}

void setColor(int color)
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (WORD)color);
}

void resetColor()
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
}

void playSound(const char* filename)
{
    PlaySoundA(filename, NULL, SND_FILENAME | SND_ASYNC);
}

int getRelPos(int player, int absPos)
{
    return (absPos - ENTRY[player] + 52) % 52;
}

int getAbsPos(int player, int relPos)
{
    return (ENTRY[player] + relPos) % 52;
}

int isSafe(int absPos)
{
    for (int i = 0; i < 8; i++)
    {
        if (SAFE[i] == absPos)
        {
            return 1;
        }
    }
    return 0;
}

int canMove(int player, int t)
{
    int pos = tokenPos[player][t];

    if (pos == -1)
    {
        return (diceValue == 6) ? 1 : 0;
    }
    if (pos == 57)
    {
        return 0;
    }
    if (pos >= 0 && pos <= 51)
    {
        int newRel = getRelPos(player, pos) + diceValue;
        return (newRel <= 57) ? 1 : 0;
    }
    if (pos >= 52 && pos <= 56)
    {
        return (pos + diceValue <= 57) ? 1 : 0;
    }
    return 0;
}

int hasMovableToken(int player)
{
    for (int t = 0; t < 4; t++)
    {
        if (canMove(player, t))
        {
            return 1;
        }
    }
    return 0;
}

void finishToken(int player, int t)
{
    tokenPos[player][t] = 57;
    score[player]++;
    gotoxy(34, 6);
    setColor(11);
    cout << playerName[player] << " token DONE! (" << score[player] << "/4)   ";
    resetColor();
    playSound("finish.wav");
    Sleep(1000);
    gotoxy(34, 6);
    cout << "                                      ";
}

void moveToken(int player, int t)
{
    int pos = tokenPos[player][t];

    if (pos == -1)
    {
        tokenPos[player][t] = ENTRY[player];
        playSound("move.wav");
        return;
    }

    if (pos >= 0 && pos <= 51)
    {
        int rel = getRelPos(player, pos);
        int newRel = rel + diceValue;

        if (newRel < 52)
        {
            tokenPos[player][t] = getAbsPos(player, newRel);
            playSound("move.wav");
        }
        else
        {
            int homeStep = newRel - 52;
            if (homeStep >= 5)
            {
                finishToken(player, t);
            }
            else
            {
                tokenPos[player][t] = 52 + homeStep;
                playSound("move.wav");
            }
        }
        return;
    }

    if (pos >= 52 && pos <= 56)
    {
        int newPos = pos + diceValue;
        if (newPos >= 57)
        {
            finishToken(player, t);
        }
        else
        {
            tokenPos[player][t] = newPos;
            playSound("move.wav");
        }
    }
}

void checkCapture(int player, int t)
{
    int pos = tokenPos[player][t];

    if (pos < 0 || pos > 51)
    {
        return;
    }
    if (isSafe(pos))
    {
        return;
    }

    int opp = 1 - player;
    for (int ot = 0; ot < 4; ot++)
    {
        if (tokenPos[opp][ot] == pos)
        {
            tokenPos[opp][ot] = -1;
            extraTurn = 1;

            gotoxy(34, 7);
            setColor(12);
            cout << playerName[player] << " captured " << playerName[opp] << " Bonus turn!  ";
            resetColor();
            playSound("capture.wav");
            Sleep(1000);
            gotoxy(34, 7);
            cout << "                                             ";
        }
    }
}

int checkWin(int player)
{
    return (score[player] == 4) ? 1 : 0;
}

void showWinner(int player)
{
    gotoxy(0, 17);
    setColor(11);
    cout << "\n\n";
    cout << "  *************************************\n";
    cout << "  *  CONGRATULATIONS!                *\n";
    cout << "  *  " << playerName[player] << " WINS THE GAME!           *\n";
    cout << "  *************************************\n";
    resetColor();
    playSound("win.wav");
    Sleep(3000);
}

void drawBoard()
{
    char dispCh[15][15];
    int  dispCol[15][15];

    for (int r = 0; r < 15; r++)
    {
        for (int c = 0; c < 15; c++)
        {
            dispCh[r][c] = ' ';
            dispCol[r][c] = 8;
        }
    }

    for (int r = 0; r <= 5; r++)
    {
        for (int c = 0; c <= 5; c++)
        {
            dispCh[r][c] = '#';
            dispCol[r][c] = 4;
        }

        for (int c = 9; c <= 14; c++)
        {
            dispCh[r][c] = '#';
            dispCol[r][c] = 6;
        }
    }
    for (int r = 9; r <= 14; r++)
    {
        for (int c = 0; c <= 5; c++)
        {
            dispCh[r][c] = '#';
            dispCol[r][c] = 2;
        }
        for (int c = 9; c <= 14; c++)
        {
            dispCh[r][c] = '#';
            dispCol[r][c] = 1;
        }
    }

    for (int i = 0; i < 52; i++)
    {
        dispCh[TR[i]][TC[i]] = '.';
        dispCol[TR[i]][TC[i]] = 7;
    }

    for (int i = 0; i < 8; i++)
    {
        int idx = SAFE[i];
        dispCh[TR[idx]][TC[idx]] = '*';
        dispCol[TR[idx]][TC[idx]] = 10;
    }

    for (int p = 0; p < 2; p++)
    {
        for (int s = 0; s < 5; s++)
        {
            dispCh[HP_R[p][s]][HP_C[p][s]] = '-';
            dispCol[HP_R[p][s]][HP_C[p][s]] = (p == 0) ? 12 : 14;
        }
    }

    for (int r = 1; r <= 13; r++)
    {
        if (dispCh[r][7] == ' ')
        {
            dispCh[r][7] = '.';
            dispCol[r][7] = 8;
        }
    }

    dispCh[7][7] = 'F';
    dispCol[7][7] = 11;

    for (int p = 0; p < 2; p++)
    {
        char ch = (p == 0) ? 'R' : 'Y';
        int  col = (p == 0) ? 12 : 14;

        for (int t = 0; t < 4; t++)
        {
            int pos = tokenPos[p][t];

            if (pos >= 0 && pos <= 51)
            {
                dispCh[TR[pos]][TC[pos]] = ch;
                dispCol[TR[pos]][TC[pos]] = col;
            }
            else if (pos >= 52 && pos <= 56)
            {
                int s = pos - 52;
                dispCh[HP_R[p][s]][HP_C[p][s]] = ch;
                dispCol[HP_R[p][s]][HP_C[p][s]] = col;
            }
        }
    }

    gotoxy(0, 0);
    for (int r = 0; r < 15; r++)
    {
        for (int c = 0; c < 15; c++)
        {
            setColor(dispCol[r][c]);
            cout << dispCh[r][c] << ' ';
        }
        resetColor();
        cout << '\n';
    }

    for (int t = 0; t < 4; t++)
    {
        if (tokenPos[0][t] == -1)
        {
            gotoxy(RY_C[t] * 2, RY_R[t]);
            setColor(12);
            cout << 'R';
        }
        if (tokenPos[1][t] == -1)
        {
            gotoxy(YY_C[t] * 2, YY_R[t]);
            setColor(14);
            cout << 'Y';
        }
    }

    resetColor();
}

void drawSidebar()
{
    const int X = 34;

    gotoxy(X, 0);
    setColor(15);
    cout << "=== LUDO  2-Player ===       ";

    gotoxy(X, 1);
    setColor(currentPlayer == 0 ? 12 : 14);
    cout << "Turn: " << playerName[currentPlayer] << "                  ";

    gotoxy(X, 2);
    resetColor();
    cout << "Score   Red=" << score[0] << "   Yellow=" << score[1] << "        ";

    gotoxy(X, 3);

    for (int t = 0; t < 4; t++)
    {
        gotoxy(X, 4 + t);
        setColor(12);
        cout << "  R" << (t + 1) << ": ";
        resetColor();
        int pos = tokenPos[0][t];
        if (pos == -1)       cout << "Yard       ";
        else if (pos == 57)  cout << "DONE!      ";
        else if (pos >= 52)  cout << "Home[" << (pos - 52) << "]   ";
        else                 cout << "Sq " << pos << "        ";
    }

    for (int t = 0; t < 4; t++)
    {
        gotoxy(X, 8 + t);
        setColor(14);
        cout << "  Y" << (t + 1) << ": ";
        resetColor();
        int pos = tokenPos[1][t];
        if (pos == -1)       cout << "Yard       ";
        else if (pos == 57)  cout << "DONE!      ";
        else if (pos >= 52)  cout << "Home[" << (pos - 52) << "]   ";
        else                 cout << "Sq " << pos << "        ";
    }
}

int rollDice()
{
    int val = 1;
    for (int i = 0; i < 10; i++)
    {
        val = rand() % 6 + 1;
        gotoxy(34, 13);
        setColor(14);
        cout << "Rolling...  [" << val << "]    ";
        resetColor();
        Sleep(55);
    }
    playSound("roll.wav");
    return val;
}

int selectToken(int player)
{
    gotoxy(34, 14);
    setColor(9);
    cout << "Movable: ";
    for (int t = 0; t < 4; t++)
    {
        if (canMove(player, t))
        {
            cout << (t + 1) << " ";
        }
    }
    cout << "          ";
    resetColor();

    while (1)
    {
        gotoxy(34, 15);
        setColor(15);
        cout << "Pick token (1-4): ";
        resetColor();

        cin.clear();
        int choice;
        if (!(cin >> choice))
        {
            cin.clear();
            cin.ignore(1000, '\n');
            continue;
        }
        choice--;

        if (choice < 0 || choice > 3 || !canMove(player, choice))
        {
            gotoxy(34, 16);
            setColor(12);
            cout << "Invalid. Try again.           ";
            resetColor();
            continue;
        }

        gotoxy(34, 14);
        cout << "                              ";
        gotoxy(34, 15);
        cout << "                              ";
        gotoxy(34, 16);
        cout << "                              ";
        return choice;
    }
}

void saveGame()
{
    ofstream f("ludo_save.dat", ios::binary);
    if (!f)
    {
        return;
    }
    f.write((char*)playerName, sizeof(playerName));
    f.write((char*)tokenPos, sizeof(tokenPos));
    f.write((char*)score, sizeof(score));
    f.write((char*)&currentPlayer, sizeof(int));
    f.write((char*)&extraTurn, sizeof(int));
    f.close();

    gotoxy(34, 17);
    setColor(10);
    cout << "Game saved!                   ";
    resetColor();
    Sleep(800);
    gotoxy(34, 17);
    cout << "                              ";
}

int loadGame()
{
    ifstream f("ludo_save.dat", ios::binary);
    if (!f)
    {
        return 0;
    }
    f.read((char*)playerName, sizeof(playerName));
    f.read((char*)tokenPos, sizeof(tokenPos));
    f.read((char*)score, sizeof(score));
    f.read((char*)&currentPlayer, sizeof(int));
    f.read((char*)&extraTurn, sizeof(int));
    f.close();
    return 1;
}

void showMenu()
{
    system("cls");
    setColor(11);
    cout << "\n";
    cout << "  ======================================\n";
    cout << "           L U D O  2 Players        \n";
    cout << "  ======================================\n\n";
    resetColor();
    cout << "  1. New Game\n";

    {
        ifstream chk("ludo_save.dat", ios::binary);
        if (chk)
        {
            cout << "  2. Resume Saved Game\n";
            chk.close();
        }
    }

    cout << "  3. Quit\n\n";
    cout << "  4. Credits\n\n";
    cout << "  Choice: ";
}

void showCredits()
{
    system("cls");

    setColor(11);

    cout << "\n\n";
    cout << "  ======================================\n";
    cout << "               C R E D I T S            \n";
    cout << "  ======================================\n\n";

    resetColor();
    cout << "Developed by:\n";
    cout << "     Muhammad Abdullah 25F-0517\n";
    cout << "     Muhammad Haseeb 25F-0552\n\n";

    cout << "  Press ENTER to return to menu...";

    cin.ignore();
    cin.get();
}


void getPlayerNames()
{
    system("cls");
    cout << "\n  Player 1 name (Red):    ";
    cin >> playerName[0];
    cout << "  Player 2 name (Yellow): ";
    cin >> playerName[1];
}

int main()
{
    srand((unsigned int)time(0));

    CONSOLE_CURSOR_INFO ci;
    ci.dwSize = 1;
    ci.bVisible = FALSE;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &ci);

    showMenu();
    int choice;
    cin >> choice;

    if (choice == 3)
    {
        return 0;
    }

    if (choice == 4)
    {
        showCredits();
        main();
        return 0;
    }

    if (choice == 2 && !loadGame())
    {
        cout << "\n  No save file found. Starting new game.\n";
        Sleep(1500);
        choice = 1;
    }

    if (choice == 1)
    {
        getPlayerNames();
        for (int p = 0; p < 2; p++)
        {
            for (int t = 0; t < 4; t++)
            {
                tokenPos[p][t] = -1;
            }
            score[p] = 0;
        }
        currentPlayer = 0;
        extraTurn = 0;
    }

    gameOver = 0;
    system("cls");

    while (!gameOver)
    {
        drawBoard();
        drawSidebar();

        gotoxy(34, 12);
        setColor(15);
        cout << "ENTER = Roll   S = Save   Q = Quit  ";
        resetColor();

        cin.ignore(1000, '\n');
        string inp;
        getline(cin, inp);

        if (!inp.empty())
        {
            char cmd = (char)tolower((unsigned char)inp[0]);
            if (cmd == 'q')
            {
                break;
            }
            if (cmd == 's')
            {
                saveGame();
                continue;
            }
        }

        diceValue = rollDice();

        gotoxy(34, 13);
        setColor(14);
        cout << "You rolled: " << diceValue << "                    ";
        resetColor();

        if (diceValue == 6)
        {
            extraTurn = 1;
            gotoxy(34, 18);
            setColor(10);
            cout << "Rolled 6! Extra turn!             ";
            resetColor();
        }
        else
        {
            extraTurn = 0;
            gotoxy(34, 18);
            cout << "                                  ";
        }

        if (!hasMovableToken(currentPlayer))
        {
            gotoxy(34, 19);
            setColor(12);
            cout << "No valid moves. Skipping turn.    ";
            resetColor();
            Sleep(1400);
            gotoxy(34, 19);
            cout << "                                  ";
        }
        else
        {
            int token = selectToken(currentPlayer);
            moveToken(currentPlayer, token);
            checkCapture(currentPlayer, token);

            if (checkWin(currentPlayer))
            {
                drawBoard();
                showWinner(currentPlayer);
                gameOver = 1;
                remove("ludo_save.dat");
                break;
            }
        }

        if (!extraTurn)
        {
            currentPlayer = 1 - currentPlayer;
        }
        extraTurn = 0;
    }
    gotoxy(0, 22);
    resetColor();
    cout << "\n  Press ENTER to exit...";
    cin.ignore(1000, '\n');
    cin.get();
    system("pause");
    return 0;
}
