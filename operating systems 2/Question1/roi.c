#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define Size 3

bool isFinished(char Board[Size][Size], bool *who_won);
bool isLegal(char BotMoves[9]);
bool make_move(char *ptr, int pos, bool user_turn);
bool draw(char Board[Size][Size]);
void printBoard(char Board[Size][Size]);

bool user_turn = true;

int main(int argc, char *argv[]) {
    if (argc != 2) { // Expect the program name and one argument containing 9 bot moves
        perror("Usage: of <9 bot moves>\n");
        perror(argv[1]);
        exit(EXIT_FAILURE);
    }

    if (strlen(argv[1]) != 9) { // Check if the provided moves string is exactly 9 characters long
        printf("Invalid input length. Expected 9 bot moves.\n");
        return -1;
    }

    char BotMoves[9];
    int bot_index = 0;
    bool who_won; // true User won, false Bot won
    char Board[Size][Size] = { { ' ', ' ', ' ' }, { ' ', ' ', ' ' }, { ' ', ' ', ' ' } };

    for (int i = 0; i < 9; i++) { // get the moves of the bot from argv
        BotMoves[i] = argv[1][i];
    }

    if (!isLegal(BotMoves)) { // if the bot has an invalid input
        printf("Invalid bot moves\n");
        return -1;
    }

    char *ptr = (char *)Board;
    while (!isFinished(Board, &who_won)) {
        if (user_turn) {
            printBoard(Board);
            char  x = -1;
            printf("Enter your move ");
            fflush(stdout);
            scanf(" %c", &x);
            int b = atoi(&x);
            if (b > 10 || b < 0 || ptr[b-1] != ' ' ) {
                printf("%d",b);
                printf("Invalid move. Try again.\n");
                continue;
            }
            printf("shit.\n");
            make_move(ptr, b, user_turn);
            user_turn = false;
        } else {
            while (ptr[BotMoves[bot_index] - '1'] == 'X' || ptr[BotMoves[bot_index] - '1'] == 'O') {
                bot_index++;
            }
            make_move(ptr, BotMoves[bot_index] - '1', user_turn);
            bot_index++;
            user_turn = true;
        }
    }
    printBoard(Board);
    if (draw(Board)) {
        printf("Draw\n");
    } else {
        if (who_won) {
            printf("User won\n");
        } else {
            printf("Bot won\n");
        }
    }
    return 0;
}

///////////////////////////////////////////////HELPERS///////////////////////////////////////////////
void printBoard(char Board[Size][Size]) {
    for (int i = 0; i < Size; i++) {
        for (int j = 0; j < Size; j++) {
            printf("%c", Board[i][j]);
            if (j < Size - 1) {
                printf("|");
            }
        }
        printf("\n");
        if (i < Size - 1) {
            printf("-----\n");
        }
    }
}

bool make_move(char *ptr, int pos, bool user_turn) {
    if (user_turn) {
        ptr[pos-1] = 'X';
    } else {
        ptr[pos] = 'O';
    }
    return true;
}

bool isLegal(char BotMoves[9]) { // I assume that the bot doesn't have a null somewhere in the moves
    bool valid[9] = { false };

    for (int i = 0; i < 9; i++) {
        int move = BotMoves[i] - '0';
        if (move > 0 && move <= 9 && !valid[move - 1]) {
            valid[move - 1] = true;
        } else {
            printf("Invalid move: %d\n", move);
            return false;
        }
    }
    return true;
}

bool draw(char Board[Size][Size]) {
    for (int i = 0; i < Size; i++) {
        for (int j = 0; j < Size; j++) {
            if (Board[i][j] != 'X' && Board[i][j] != 'O') {
                return false;
            }
        }
    }
    return true;
}

bool isFinished(char Board[Size][Size], bool *who_won) {
    for (int i = 0; i < Size; i++) {
        if (Board[i][0] != ' ' && Board[i][0] == Board[i][1] && Board[i][1] == Board[i][2]) {
            *who_won = (Board[i][0] == 'X');
            return true;
        }
        if (Board[0][i] != ' ' && Board[0][i] == Board[1][i] && Board[1][i] == Board[2][i]) {
            *who_won = (Board[0][i] == 'X');
            return true;
        }
    }
    if (Board[0][0] != ' ' && Board[0][0] == Board[1][1] && Board[1][1] == Board[2][2]) {
        *who_won = (Board[0][0] == 'X');
        return true;
    }
    if (Board[0][2] != ' ' && Board[0][2] == Board[1][1] && Board[1][1] == Board[2][0]) {
        *who_won = (Board[0][2] == 'X');
        return true;
    }

    return draw(Board); // if return true then true else it should be false 
}