#include <iostream>
#include <fstream> //To save score.
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <conio.h>
#include <string>
#include <iomanip>
#include <stdexcept>
//Whanted to use for graphic <SDL2/SDL.h> but did not work out, same with windows.h

/*
(One hell of a CODE)
Could be more optimized, I admit.
*/



using namespace std;
// Const values
int const SIZE = 4;
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

// To initialize
void initializeBoard(int board[SIZE][SIZE]) {
    // Set all elements of the board to 0
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            board[i][j] = 0;
        }
    }
    srand(static_cast<unsigned>(time(0)));
    board[rand() % SIZE][rand() % SIZE] = 2;
    board[rand() % SIZE][rand() % SIZE] = 2;

    //Add random tiles to the left
    board[0][0] = 4;
    }

// To display
void displayBoard(int board[SIZE][SIZE]) {
    cout<<"---------------------------"<<endl;
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
             if (board[i][j] == 0) {
                cout << "\x1B[47m\x1B[30m[ ]\x1B[0m\t"; // Empty cell
            } else {
                int value = log2(board[i][j]);
                int color = (value % 7) + 31;
                cout << "\x1B[47m\x1B[" << color << "m[" << board[i][j] << "]\x1B[0m\t";
            }
            // cout << board[i][j] << "\t";
        }
        cout<< endl;
    }
    cout<<"---------------------------"<<endl;
}

//Extra functions
//Checker
bool isBoardFull(const int board[SIZE][SIZE]) {
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            if (board[i][j] == 0) {
                return false; 
            }
        }
    }
    return true; 
}

//Spawn a 2 if after moving
void spawnNumber(int board[SIZE][SIZE]) {
    if(isBoardFull(board)){
        return;
    }
    int row, col;
    do {
        row = rand() % SIZE;
        col = rand() % SIZE;
    } while (board[row][col] != 0);
    board[row][col] = 2; 
}
//Score function
void updateScore(int mergedValue, int &score) {
    score += mergedValue;
}


//Save function
void saveScore(const string& playerName, int &score) {
    ofstream file("scores.txt", ios::app);

    if (file.is_open()) {
        time_t now = time(nullptr);
        char buffer[80];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %X", localtime(&now));
        file << "----------------------------------------\n";
        file << "Player: " << playerName << "\n";
        file << "Score: " << score << "\n";
        file << "Date: " << buffer << "\n";
        file << "----------------------------------------\n";

        file.close();
        cout << "Score saved successfully.\n";
    } else {
        cerr << "Error: Unable to open the file for saving scores.\n";
    }
}
//Read file
void displayLeaderboard() {
    ifstream file("scores.txt");
    if (file.is_open()) {
        cout << "Leaderboard:\n";
        int highestScore = 0;
        string highestScorer;

        while (!file.eof()) {
            string line;
            getline(file, line);
            if (line.find("Player:") != string::npos) {
                string playerName = line.substr(line.find(":") + 2);
                getline(file, line);
                
                try {
                    int score = stoi(line.substr(line.find(":") + 2));
                    if (score > highestScore) {
                        highestScore = score;
                        highestScorer = playerName;
                    }
                } catch (const invalid_argument& e) {
                    cerr << "Error converting score to integer: " << e.what() << endl;
                }
            }
        }
        if (highestScore > 0) {
            cout << "Highest Score: " << highestScore << " by " << highestScorer << "\n";
        } else {
            cout << "Not found.\n";
        }

        file.close();
    } else {
        cerr << "Error.\n";
    }
}


//The movement function
//Move left
void moveLeft(int board[SIZE][SIZE],int &score) {
    for (int i = 0; i < SIZE; ++i) {
        // Shift tiles to the left
        for (int j = 0; j < SIZE - 1; ++j) {
            if (board[i][j] == 0) {
                for (int k = j + 1; k < SIZE; ++k) {
                    if (board[i][k] != 0) {
                        board[i][j] = board[i][k];
                        board[i][k] = 0;
                        break;
                    }
                }
            }
        }
        // Merge tiles
        for (int j = 0; j < SIZE - 1; ++j) {
            if (board[i][j] == board[i][j + 1] && board[i][j] != 0) {
                updateScore(board[i][j], score);
                board[i][j] *= 2;
                board[i][j + 1] = 0;
            }
        }
        // Shift tiles to the left after merging
        for (int j = 0; j < SIZE - 1; ++j) {
            if (board[i][j] == 0) {
                for (int k = j + 1; k < SIZE; ++k) {
                    if (board[i][k] != 0) {
                        board[i][j] = board[i][k];
                        board[i][k] = 0;
                        break;
                    }
                }
            }
        }
    }
}
//Move right
void moveRight(int board[SIZE][SIZE],int &score) {
    for (int i = 0; i < SIZE; ++i) {
        // Shift tiles to the right
        for (int j = SIZE - 1; j > 0; --j) {
            if (board[i][j] == 0) {
                for (int k = j - 1; k >= 0; --k) {
                    if (board[i][k] != 0) {
                        board[i][j] = board[i][k];
                        board[i][k] = 0;
                        break;
                    }
                }
            }
        }
        // Merge tiles
        for (int j = SIZE - 1; j > 0; --j) {
            if (board[i][j] == board[i][j - 1] && board[i][j] != 0) {
                updateScore(board[i][j], score);
                board[i][j] *= 2;
                board[i][j - 1] = 0;
            }
        }
        // Shift tiles to the right after merging
        for (int j = SIZE - 1; j > 0; --j) {
            if (board[i][j] == 0) {
                for (int k = j - 1; k >= 0; --k) {
                    if (board[i][k] != 0) {
                        board[i][j] = board[i][k];
                        board[i][k] = 0;
                        break;
                    }
                }
            }
        }
    }
}
//Move up if you did not understand by the function naming.
void moveUp(int board[SIZE][SIZE],int &score) {
    for (int j = 0; j < SIZE; ++j) {
        // Shift tiles up
        for (int i = 0; i < SIZE - 1; ++i) {
            if (board[i][j] == 0) {
                for (int k = i + 1; k < SIZE; ++k) {
                    if (board[k][j] != 0) {
                        board[i][j] = board[k][j];
                        board[k][j] = 0;
                        break;
                    }
                }
            }
        }
        // Merge tiles
        for (int i = 0; i < SIZE - 1; ++i) {
            if (board[i][j] == board[i + 1][j] && board[i][j] != 0) {
                updateScore(board[i][j], score);
                board[i][j] *= 2;
                board[i + 1][j] = 0;
            }
        }
        // Shift tiles up after merging
        for (int i = 0; i < SIZE - 1; ++i) {
            if (board[i][j] == 0) {
                for (int k = i + 1; k < SIZE; ++k) {
                    if (board[k][j] != 0) {
                        board[i][j] = board[k][j];
                        board[k][j] = 0;
                        break;
                    }
                }
            }
        }
    }
}
//Moving down.
void moveDown(int board[SIZE][SIZE],int &score) {
    for (int j = 0; j < SIZE; ++j) {
        // Shift tiles down
        for (int i = SIZE - 1; i > 0; --i) {
            if (board[i][j] == 0) {
                for (int k = i - 1; k >= 0; --k) {
                    if (board[k][j] != 0) {
                        board[i][j] = board[k][j];
                        board[k][j] = 0;
                        break;
                    }
                }
            }
        }
        // Merge tiles
        for (int i = SIZE - 1; i > 0; --i) {
            if (board[i][j] == board[i - 1][j] && board[i][j] != 0) {
                updateScore(board[i][j], score);
                board[i][j] *= 2;
                board[i - 1][j] = 0;
            }
        }
        // Shift tiles down after merging
        for (int i = SIZE - 1; i > 0; --i) {
            if (board[i][j] == 0) {
                for (int k = i - 1; k >= 0; --k) {
                    if (board[k][j] != 0) {
                        board[i][j] = board[k][j];
                        board[k][j] = 0;
                        break;
                    }
                }
            }
        }
    }
}
//----
// Additinal checking for game over
bool areBoardsEqual(const int board1[SIZE][SIZE], const int board2[SIZE][SIZE]) {
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            if (board1[i][j] != board2[i][j]) {
                return false;
            }
        }
    }
    return true;
}

//Game over?
bool isGameOver(const int board[SIZE][SIZE]) {
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            if (board[i][j] == 0) {
                return false;
            }
            if (j < SIZE - 1 && (board[i][j] == board[i][j + 1] || board[i][j + 1] == 0)) {
                return false;
            }
            if (i < SIZE - 1 && (board[i][j] == board[i + 1][j] || board[i + 1][j] == 0)) {
                return false;
            }
        }
    }
    cout << "No more moves left. Game over." << endl;
    return true;
}



//Key listiner
char keyPress() {
    char key = _getch();
    if (key == 0 || key == 0xE0) { 
        key = _getch();
        switch (key) {
            case 72: return 'w'; 
            case 80: return 's'; 
            case 75: return 'a'; 
            case 77: return 'd'; 
        }
    }
    return key;
}

//The main body
int main() {
    bool flag = false;
    string choice;
    while(!flag) {
        system("cls"); 
        cout<<"Type 1 to start a game \n";
        cout<<"Type 2 to view the leaderboard \n";   
        cout<<"Type quit to end the game \n";
        cin>>choice;
        cin.ignore();
        if(choice=="1") {
            system("cls");
            cout<<"Chosen Nr1 \n";
            string playerName;
            cout<<"Enter player name ? : ";
            getline(cin, playerName);
            cout<<"Game started! \n";
            cout<<"Press AWSD to move. Q to quit"<<endl;
           int score = 0;
           int board[SIZE][SIZE];
        initializeBoard(board);
        displayBoard(board);
            char key;
                do {
                     if (isGameOver(board)) { 
                    system("pause");
                    break;
                }
        //cin>>key;
                    key = keyPress();
                    switch (key)  {
                        case 'a':
                           moveLeft(board,score);
                            break;
                        case 'd':
                            moveRight(board,score);
                            break;
                        case 'w':
                            moveUp(board,score);
                            break;
                        case 's':
                            moveDown(board,score);
                            break;
                            default:
                                break;
                             }
                                spawnNumber(board);
                             // Adding a 2
                            system("cls"); 
                            cout<<"Game in progress..."<<endl;
                            cout<<"Press AWSD to move. Q to quit loser"<<endl;
                            displayBoard(board);
                            cout<<"Score : "<< score << endl;
    
                            cout<<"\n";

                 }
                while (key !='q');

               if (isGameOver(board)) { 
                cout << "Game Over! No more moves left.\n";
                cout << "Press any key to exit\n";
            }

                saveScore(playerName,score);
                cout<<"Thanks for playing! | "<<playerName<<" | Your end score is : "<<score<<endl;
                cout<<"Press enter to continue... \n";
                 cin.get();
    }
    else if(choice=="2") {
          system("cls");
         cout<<"Chosen Nr2 \n";
       cout<<"Here is the highest scorer \n";
       displayLeaderboard();
       system("timeout /nobreak /t 5 >nul");
       cout<<"Press enter to continue... \n";
       cin.get();
    }
    else if(choice=="quit") {
        flag = true;
    }
    }
    return 0;
};

