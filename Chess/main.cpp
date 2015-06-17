#include <iostream>
#include <vector>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <sstream>

using namespace std;

typedef enum {blank,whitepawn,whiterook,whiteknight,whitebishop,whiteking,whitequeen,blackpawn,blackrook,blackknight,blackbishop,blackking,blackqueen} TPiece; //defining the types of pieces
typedef vector <int> ivec;

//Creates types to hold a variable number of 2 dimensional vectors
typedef vector <TPiece> row;
typedef vector <row> col;
typedef vector <col> Boards;

//A structure to hold the specific moves of a piece
struct PieceMove
{
    int x;
    int y;
};

//A structure to hold
struct PieceInfo
{
    TPiece TypeOfPiece; //Determines the piece
    bool FirstMove; //if the piece has already moved, for speacial moves like pawn
    bool IsWhite; //Determine the colour of the piece
    SDL_Rect Rect; //Coordinates of the piece on the screen
    SDL_Rect OldLocation; //Hold the previous Co-ordinates
    SDL_Texture* Texture; //Picture of the piece
    bool IsTaken; //Tells the program when and what pieces have been removed from the board;
    vector <PieceMove> moves;
};

struct BoardArray
{
    TPiece board[8][8];
};

//Structure to hold information about each move
struct AiMovementTree
{
    int value; //evaluation of the board
    col CurrentBoard; //board state of the move
    vector <AiMovementTree> FutureMoves; //Vector of the next moves
    int CurrentPiece; //the piece that is moved on that move
    int TakenPiece; //the piece that has been taken on that move, -1 if nothing is taken
};

//for easy printing of vectors
ostream &operator << (ostream &stream, col &obj)
{
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            stream << obj[j][i] << " ";
        }
        stream << endl;
    }
    return stream;
}

//Declaring Global Variables
col board; //The Main board that is being used
PieceInfo Pieces[32]; //Creates an array of structures, one per piece
int ScreenHeight; //the height of the display window, defined during runtime
int ScreenWidth; //the width of the display window, defined during runtime
int BoardHeight; //the Height of the Playing board, defined during runtime
int SquareSize; //The width and height of each individual square on the board, defined during runtime
const int Offset = 50; //The small buffer zone between the top, and left side of the window and the board
const int best = 1500; //the best possible values for the computer
int mousex; //Global variables to hold the mouse's x coordinate
int mousey; //Global variables to hold the mouse's y coordinate
bool WhiteTurn = true; //true when it is whites turn, false when blacks turn
bool WhiteInCheck = false; //true when black has put white in check
bool BlackInCheck = false; //true when white has put black in check
int NumberWhiteMoves = 20; //set to 20 because thats the initial moves garentied
int NumberBlackMoves = 20; //set to 20 because thats the initial moves garentied
AiMovementTree PredictedMoves; //holds the inicial board position for predicting moves

//Functions
void PrintBoard (TPiece Board[8][8]); // prints out any given 2d array
void SetPositions (); //Initially gives values in the structures on the pieces
void PieceSnapToSquare (int selectedPiece,int mousex,int mousey); //After the piece is dropped it aligns in onto the square below the mouse
int SelectPiece(int mousex,int mousey); //When the user clicks on a piece, it returns the piece's structure for manipulation
bool IsvalidMove(int selectedPiece,bool AiTurn); //Tests if the move the user wants to make is allowed
bool CheckForCheck (int KingColour); //Checks if the move cause either player to be in check
int PieceTake(int selectedpiece); //Moves the taken piece off the board;
void assignValues(TPiece WhatPiece,int Rectx,int Recty,string path,int i,bool isWhite,vector <PieceMove> moves); // gets sent a whole bunch of values and gives the correct piece all its information
vector <AiMovementTree> GenerateMoveSet(col Board, bool WhiteMoves,int *NumberOfMoves); //creates board sets of all possible moves from any given board
col UpdateBoard(col Board); //updates the global board to look like the screen
void updateScreen(col Board); //updates the screen to look like any given board
bool WhiteInCheckmate(); //tests if white has lost
bool BlackInCheckmate(); //tests if black has lost
int evaluate(col Board, bool AiTurn,int Taken); //gives a values to the given board
bool stalemate(col Board); //tests if it is a tie
bool AiMove(); //Starts all the Computer proccesses
void Render(); //Makes everything show up on the screen

void StartSDL (); //Starts up and initalizes everything in SDL
void CloseSDL (); //Closes down and deletes everything in SDL
void ClosePromotion  ();
void LoadMedia(); //Loads all the pictures and fonts
SDL_Texture* LoadTexture( string path ); //Assigns all the pictures to their respective textures
SDL_Texture* LoadText (int X,int Y); //Loads and displays and given text onto the display window

//Piece and Board Textures
SDL_Renderer* Renderer = NULL;
SDL_Window* window = NULL;
SDL_Renderer* PromotionRenderer = NULL;
SDL_Window* PromotionWindow = NULL;
SDL_Texture* BoardTexture = NULL;
SDL_Texture* CoordTexture = NULL;
SDL_Color White = {0,0,0};
TTF_Font *gFont = NULL;
TTF_Font *Font = NULL;
bool PromotionQuit = true;

int main(int argc, char* args[])
{
    StartSDL(); //Starts up SDL libraries and initializes
    SDL_GetMouseState(&mousex,&mousey); //tells the computer where the mouse is
    LoadMedia(); //loads all the pictures into memory
    SetPositions(); //gives values to all of the pieces

    //rectangle for the board to render too
    SDL_Rect BoardRect;
    BoardRect.x = Offset;
    BoardRect.y = Offset;
    BoardRect.w = BoardHeight;
    BoardRect.h = BoardHeight;

//    SDL_Rect TextRect;
//    TextRect.x = 700;
//    TextRect.y = 50;
//    TextRect.w = 100;
//    TextRect.h = 32;
//
//    SDL_Rect TextRect2;
//    TextRect2.x = 700;
//    TextRect2.y = 90;
//    TextRect2.w = 100;
//    TextRect2.h = 32;

    SDL_Event e;
    SDL_Event g;

    bool quit;
    bool Mousedown = false;
    bool PromotionClick = false;

    int SelectedPiece = -1;

    UpdateBoard(board); //gives the global board a starting value

    //Sets the values of the board to be manipulated
    PredictedMoves.CurrentBoard = board;
    PredictedMoves.value = 0;
    PredictedMoves.CurrentPiece = -1;
    PredictedMoves.TakenPiece = -1;

    while (!quit)
    {
        while( SDL_PollEvent( &e ) != 0 )
        {
            SDL_GetMouseState(&mousex,&mousey); //Gets the coordinates of the mouse

            if( e.type == SDL_QUIT ) //if the user exits the program
                quit = true;
            if (e.type == SDL_MOUSEBUTTONDOWN) //when the user clicks on the screen
                Mousedown = true;
            if (e.type == SDL_MOUSEBUTTONUP) //when the user lets go of the mouse button
            {
                Mousedown = false;
                if (SelectedPiece >= 0)
                {
                    //runs a function to allign the piece to the grid
                    PieceSnapToSquare(SelectedPiece,mousex,mousey);
                    //Tests whether or not that move is allowed
                    if(!IsvalidMove(SelectedPiece,false))
                        Pieces[SelectedPiece].Rect = Pieces[SelectedPiece].OldLocation; //moves the piece back to its original position
                    else
                    {
                        int TakenPiece = PieceTake(SelectedPiece); //tests to see if the player is trying to take a piece
                        bool TurnChange = true;
                        BlackInCheck = false;
                        WhiteInCheck = false;

                        if (CheckForCheck(30)) //tests if the white king is in check
                        {
                            WhiteInCheck = true;
                            if (WhiteTurn)//if the player tries to make a move that puts him into check
                            {
                                TurnChange = false;
                                Pieces[SelectedPiece].Rect = Pieces[SelectedPiece].OldLocation; //resets that pieces location
                                if (TakenPiece >= 0)
                                {
                                    Pieces[TakenPiece].IsTaken = false; //resets values associated with the taken piece
                                    Pieces[TakenPiece].Rect = Pieces[TakenPiece].OldLocation; //resets the piece that has been taken's location
                                }
                            }
                            cout << "White is in check" << endl;
                        }


                        if (CheckForCheck(31)) //tests if the black king is in check
                        {
                            BlackInCheck = true;
                            if (!WhiteTurn)//if the player tries to make a move that puts him into check
                            {
                                TurnChange = false;
                                Pieces[SelectedPiece].Rect = Pieces[SelectedPiece].OldLocation;//resets that pieces location
                                if (TakenPiece >= 0)
                                {
                                    Pieces[TakenPiece].IsTaken = false;//resets values associated with the taken piece
                                    Pieces[TakenPiece].Rect = Pieces[TakenPiece].OldLocation; //resets the piece that has been taken's location
                                }
                            }
                            cout << "Black is in check" << endl;
                        }

                        if (TurnChange) //if everything has passed all of the checks
                        {
                            Pieces[SelectedPiece].FirstMove = false;
                            WhiteTurn = !WhiteTurn; //changes the turn
                            col updatedBoard = UpdateBoard(board); //updates the board to the current move
                            //sets the values of the move to be manipulated
                            PredictedMoves.CurrentBoard = updatedBoard;
                            PredictedMoves.value = evaluate(PredictedMoves.CurrentBoard,false,TakenPiece); //gives a value to the current board
                            PredictedMoves.CurrentPiece = SelectedPiece;
                            PredictedMoves.TakenPiece = TakenPiece;
                        }

                        if (WhiteInCheckmate()) //checks if white has lost
                        {
                            cout << "Black Wins" << endl;
                            return 0;
                        }

                        if (BlackInCheckmate()) //checks if black has lost
                        {
                            cout << "White Wins" << endl;
                            return 0;
                        }

                        if (stalemate(PredictedMoves.CurrentBoard)) //checks if it is a tie
                        {
                            cout << "StaleMate" << endl;
                            return 0;
                        }
                    }
                }
                SelectedPiece = -1; //resets the piece tracking
            }
        }

        if (Mousedown && SelectedPiece < 0 )
        {
            //Checking to see if the mouse is on the board
            if (mousex >= Offset && mousex < BoardHeight+Offset && mousey >= Offset && mousey < BoardHeight+Offset)
                SelectedPiece = SelectPiece(mousex,mousey); //choses the correct piece based on where the user clicks
        }

        //sets the location of the chosen piece to be centered around the mouse
        if (SelectedPiece >= 0)
        {
            Pieces[SelectedPiece].Rect.x = mousex-SquareSize/2;
            Pieces[SelectedPiece].Rect.y = mousey-SquareSize/2;
        }

        SDL_RenderClear(Renderer);//clears the screen
        SDL_RenderCopy(Renderer,BoardTexture,NULL,&BoardRect); //puts the board picture on screen
        //CoordTexture = LoadText(Pieces[SelectedPiece].Rect.x,Pieces[SelectedPiece].Rect.y);
        //SDL_RenderCopy(Renderer,CoordTexture,NULL,&TextRect2);
        //CoordTexture = LoadText(mousex,mousey);
        //SDL_RenderCopy(Renderer,CoordTexture,NULL,&TextRect);

        //Prints all of the pieces on screen
        for (int i = 0; i < 32; i++)
            SDL_RenderCopy(Renderer,Pieces[i].Texture,NULL,&Pieces[i].Rect);

        SDL_RenderPresent(Renderer);//finilizes the renderer

        //if its blacks turn to move, start the AI processes
        if (!WhiteTurn)
            if (AiMove())
            {
                WhiteTurn = !WhiteTurn;
            }

        /*while (!PromotionQuit)
        {
            while(SDL_PollEvent(&g) != 0)
            {
                if (g.type == SDL_MOUSEBUTTONDOWN)
                    PromotionClick = true;

                SDL_RenderClear(PromotionRenderer);
                SDL_RenderPresent(PromotionRenderer);
            }
            if (PromotionClick)
            {
                ClosePromotion();
                PromotionQuit = true;
            }
        }*/

    }

    CloseSDL(); //shut down all of the SDL libraries and clean up memory

    return 0;
}

void assignValues (TPiece WhatPiece, int Rectx, int Recty, string path , int i,bool isWhite,vector <PieceMove> moves)
{
    //creates a temporary rectangle to print the piece on
    SDL_Rect Rect;
    Rect.x = Rectx;
    Rect.y = Recty;
    Rect.h = SquareSize; //defined by the screen size
    Rect.w = SquareSize;

    SDL_Texture* Texture;
    Texture = LoadTexture(path); //loads the picture from the path given

    //sets all of the values for the piece to its basic values at the start of a game
    Pieces[i].TypeOfPiece = WhatPiece;
    Pieces[i].Texture = Texture;
    Pieces[i].Rect = Rect;
    Pieces[i].OldLocation = Rect;
    Pieces[i].FirstMove = true;
    Pieces[i].IsWhite = isWhite;
    Pieces[i].moves = moves;
    Pieces[i].IsTaken = false;
}

void SetPositions()
{
    int xtrack = 0;
    vector <PieceMove> moves; //creates a vector to hold the specific piece's moves

    //Each loop runs through the numbers in the array that are assigned to that set of piecee
    //ie whitepawns 0-7,blackpawns 8-15 ect.

    for (int i = 0; i < 8; i++)
    {
        moves.clear();
        PieceMove Whitepawn; //creates a holder for cartesian coords of the moves
        int xmove[6] = {0,0,-1,1,-1,1};//the x movements that the piece is allowed to make
        int ymove[6] = {-1,-2,-1,-1,-1,-1};//the y movements that the piece is allowed to make

        //fills the vector with the move sets of the piece
        for (int j = 0; j < 6; j++)
        {
            Whitepawn.x = xmove[j];
            Whitepawn.y = ymove[j];
            moves.push_back(Whitepawn);
        }

        //sets all the values to the specific piece
        assignValues(whitepawn,xtrack*SquareSize+Offset,6*SquareSize+Offset,"Pictures/WhitePawn.gif",i,true,moves);
        xtrack++;
    }

    xtrack = 0;


    for (int i = 8; i < 16; i++)
    {
        moves.clear();
        PieceMove Blackpawn;//creates a holder for cartesian coords of the moves
        int xmove[6] = {0,0,-1,1,-1,1};//the x movements that the piece is allowed to make
        int ymove[6] = {1,2,1,1,1,1};//the y movements that the piece is allowed to make

        //fills the vector with the move sets of the piece
        for (int j = 0; j < 6; j++)
        {
            Blackpawn.x = xmove[j];
            Blackpawn.y = ymove[j];
            moves.push_back(Blackpawn);
        }

        //sets all the values to the specific piece
        assignValues(blackpawn,xtrack*SquareSize+Offset,SquareSize+Offset,"Pictures/BlackPawn.gif",i,false,moves);
        xtrack ++;
    }

    xtrack = 0;

    for (int i = 16; i < 18; i++)
    {
        moves.clear();
        PieceMove WhiteRook;//creates a holder for cartesian coords of the moves
        int xmove[6] = {0,1,0,-1};//the x movements that the piece is allowed to make
        int ymove[6] = {-1,0,1,0};//the y movements that the piece is allowed to make

        //fills the vector with the move sets of the piece
        for (int j = 0; j < 4; j++)
        {
            WhiteRook.x = xmove[j];
            WhiteRook.y = ymove[j];
            moves.push_back(WhiteRook);
        }

        //sets all the values to the specific piece
        assignValues(whiterook,xtrack*SquareSize+Offset,7*SquareSize+Offset,"Pictures/WhiteRook.gif",i,true,moves);
        xtrack += 7;
    }

    xtrack = 0;

    for (int i = 18; i < 20; i++)
    {
        moves.clear();
        PieceMove BlackRook;//creates a holder for cartesian coords of the moves
        int xmove[6] = {0,1,0,-1};//the x movements that the piece is allowed to make
        int ymove[6] = {-1,0,1,0};//the y movements that the piece is allowed to make

        //fills the vector with the move sets of the piece
        for (int j = 0; j < 4; j++)
        {
            BlackRook.x = xmove[j];
            BlackRook.y = ymove[j];
            moves.push_back(BlackRook);
        }

        //sets all the values to the specific piece
        assignValues(blackrook,xtrack*SquareSize+Offset,Offset,"Pictures/BlackRook.gif",i,false,moves);
        xtrack += 7;
    }

    xtrack = 1;

    for (int i = 20; i < 22; i++)
    {
        moves.clear();
        PieceMove WhiteKnight;//creates a holder for cartesian coords of the moves
        int xmove[8] = {-1,1,2,2,1,-1,-2,-2};//the x movements that the piece is allowed to make
        int ymove[8] = {-2,-2,-1,1,2,2,1,-1};//the y movements that the piece is allowed to make

        //fills the vector with the move sets of the piece
        for (int j = 0; j < 8; j++)
        {
            WhiteKnight.x = xmove[j];
            WhiteKnight.y = ymove[j];
            moves.push_back(WhiteKnight);
        }

        //sets all the values to the specific piece
        assignValues(whiteknight,xtrack*SquareSize+Offset,7*SquareSize+Offset,"Pictures/WhiteKnight.gif",i,true,moves);
        xtrack += 5;
    }

    xtrack = 1;

    for (int i = 22; i < 24; i++)
    {
        moves.clear();
        PieceMove BlackKnight;//creates a holder for cartesian coords of the moves
        int xmove[8] = {-1,1,2,2,1,-1,-2,-2};//the x movements that the piece is allowed to make
        int ymove[8] = {-2,-2,-1,1,2,2,1,-1};//the y movements that the piece is allowed to make

        //fills the vector with the move sets of the piece
        for (int j = 0; j < 8; j++)
        {
            BlackKnight.x = xmove[j];
            BlackKnight.y = ymove[j];
            moves.push_back(BlackKnight);
        }

        //sets all the values to the specific piece
        assignValues(blackknight,xtrack*SquareSize+Offset,Offset,"Pictures/BlackKnight.gif",i,false,moves);
        xtrack += 5;
    }

    xtrack = 2;

    for (int i = 24; i < 26; i++)
    {
        moves.clear();
        PieceMove WhiteBishop;//creates a holder for cartesian coords of the moves
        int xmove[4] = {-1,1,1,-1};//the x movements that the piece is allowed to make
        int ymove[4] = {-1,-1,1,1};//the y movements that the piece is allowed to make

        //fills the vector with the move sets of the piece
        for (int j = 0; j < 4; j++)
        {
            WhiteBishop.x = xmove[j];
            WhiteBishop.y = ymove[j];
            moves.push_back(WhiteBishop);
        }

        //sets all the values to the specific piece
        assignValues (whitebishop,xtrack*SquareSize+Offset,7*SquareSize+Offset,"Pictures/WhiteBishop.gif",i,true,moves);
        xtrack += 3;
    }

    xtrack = 2;

    for (int i = 26; i < 28; i++)
    {
        moves.clear();
        PieceMove BlackBishop;//creates a holder for cartesian coords of the moves
        int xmove[4] = {-1,1,1,-1};//the x movements that the piece is allowed to make
        int ymove[4] = {-1,-1,1,1};//the y movements that the piece is allowed to make

        //fills the vector with the move sets of the piece
        for (int j = 0; j < 4; j++)
        {
            BlackBishop.x = xmove[j];
            BlackBishop.y = ymove[j];
            moves.push_back(BlackBishop);
        }

        //sets all the values to the specific piece
        assignValues(blackbishop,xtrack*SquareSize+Offset,Offset,"Pictures/BlackBishop.gif",i,false,moves);
        xtrack += 3;
    }

    moves.clear();
    PieceMove Royalty;//creates a holder for cartesian coords of the moves
    int xmove[8] = {-1,0,1,1,1,0,-1,-1};//the x movements that the piece is allowed to make
    int ymove[8] = {-1,-1,-1,0,1,1,1,0};//the y movements that the piece is allowed to make

    //fills the vector with the move sets of the piece
    for (int j = 0; j < 8; j++)
    {
        Royalty.x = xmove[j];
        Royalty.y = ymove[j];
        moves.push_back(Royalty);
    }

    //sets all the values to the specific piece
    assignValues(blackking,4*SquareSize+Offset,Offset,"Pictures/BlackKing.gif",31,false,moves);
    //sets all the values to the specific piece
    assignValues(whiteking,4*SquareSize+Offset,7*SquareSize+Offset,"Pictures/WhiteKing.gif",30,true,moves);
    //sets all the values to the specific piece
    assignValues(blackqueen,3*SquareSize+Offset,Offset,"Pictures/BlackQueen.gif",29,false,moves);
    //sets all the values to the specific piece
    assignValues(whitequeen,3*SquareSize+Offset,7*SquareSize+Offset,"Pictures/WhiteQueen.gif",28,true,moves);
}

void Render()
{
    SDL_RenderClear(Renderer); //clears the screen

    //shows all of the pieces on the screen
    for (int i = 0; i < 32; i++)
        SDL_RenderCopy(Renderer,Pieces[i].Texture,NULL,&Pieces[i].Rect);

    //finilizes the screen
    SDL_RenderPresent(Renderer);
}

void PieceSnapToSquare (int selectedPiece,int mousex,int mousey)
{
    //finds the square that the mouse is over top of and sets the location of the piece to the square
    for (int i = 0; i < 8; i++)
    {
        if (mousex >= i*SquareSize+Offset && mousex < (i+1)*SquareSize+Offset)
            Pieces[selectedPiece].Rect.x = i*SquareSize+50;

        if (mousey >= i*SquareSize+Offset && mousey < (i+1)*SquareSize+Offset)
            Pieces[selectedPiece].Rect.y = i*SquareSize+50;
    }
}

int SelectPiece(int mousex,int mousey)
{
    for (int i = 0; i < 32; i++)
    {
        //creates the corners of a rectanlge around the current piece
        int lowx = Pieces[i].Rect.x;
        int lowy = Pieces[i].Rect.y;
        int Highx = lowx+SquareSize;
        int Highy = lowy+SquareSize;

        //if the mouse is within that rectangle then that piece is chosen
        if (mousex >= lowx && mousex < Highx && mousey >= lowy && mousey < Highy)
        {
            //if the colour of the piece matches whose turn it is
            if (WhiteTurn == Pieces[i].IsWhite)
            {
                //that piece is then sent back to main
                cout << Pieces[i].TypeOfPiece << " has been selected " << i << endl;
                Pieces[i].OldLocation = Pieces[i].Rect;
                return i;
            }
        }
    }
    //a specified fail return
    return -1;
}

int FindMatch(int x, int y, int SelectedPiece)
{
    //cycles through all the pieces on the board
    for (int i = 0; i < 32; i++)
    {
        //skips itself
        if (i == SelectedPiece) continue;
        //matches the given coordinates to any piece with the same corrdinates
        if (Pieces[i].Rect.y == y && Pieces[i].Rect.x == x) return i;
    }
    //specified fail flag
    return -1;
}

int PieceTake(int selectedPiece)
{
    //find the piece that is being taken
    int beingTaken = FindMatch(Pieces[selectedPiece].Rect.x, Pieces[selectedPiece].Rect.y, selectedPiece);

    //if there is actually a piece being taken
    if (beingTaken >= 0)
    {
        //sets the old location of the piece
        Pieces[beingTaken].OldLocation = Pieces[beingTaken].Rect;
        //makes sure the pice is a different colour
        if (Pieces[selectedPiece].IsWhite != Pieces[beingTaken].IsWhite)
        {
            //moves the piece off the screen
            Pieces[beingTaken].Rect.x = 900;
            Pieces[beingTaken].Rect.y = 50;
            //set all of its flags to show that its gone
            Pieces[selectedPiece].FirstMove = false;
            Pieces[beingTaken].IsTaken = true;
            Pieces[beingTaken].FirstMove = false;
            return beingTaken;
        }
        else
        {
            //moves both the pieces back to the old location
            Pieces[selectedPiece].Rect.x = Pieces[selectedPiece].OldLocation.x;
            Pieces[selectedPiece].Rect.y = Pieces[selectedPiece].OldLocation.y;
            //specified fail flag
            return -2;
        }
    }
    //specified fail flag
    return -1;
}

col UpdateBoard(col Board)
{
    Board.clear();

    for (int x = 0; x < 8; x++)
    {
        row temp;
        for (int y = 0; y < 8; y++)
        {
            //finds if a piece is on the current square
            int i = FindMatch(x*SquareSize+Offset,y*SquareSize+Offset,-1);
            //skips the piece if it has been taken
            if (Pieces[i].IsTaken)continue;
            //if there is nothing that matches the square set it to blank
            if (i == -1)
                temp.push_back(blank);
            else //when there is a matching piece, set the board value acordingly
                temp.push_back(Pieces[i].TypeOfPiece);
            temp.push_back(Pieces[i].TypeOfPiece);
            temp.push_back(Pieces[i].TypeOfPiece);
        }
        //pushes the row onto the 2d array
        Board.push_back(temp);
    }
    // gives the whole array back
    return Board;
}

void updateScreen(col Board)
{
    ivec MovedAlready;

    for (int x = 0; x < 8; x ++)
    {
        for (int y = 0; y < 8; y++)
        {
            for (int i = 0; i < 32; i++)
            {
                //skips the piece if it has been taken
                if (Pieces[i].IsTaken)continue;

                //success flag
                bool skip = false;
                int num = MovedAlready.size();

                //if the piece number has been flagged, skip it
                for (int j = 0; j < num; j++)
                    if (MovedAlready[j] == i) skip = true;

                if (!skip)
                {
                    //ensures the piece and the number on the board match
                    if (Pieces[i].TypeOfPiece == Board[x][y])
                    {
                        //sets the location of the piece to the same location on the board
                        Pieces[i].Rect.x = x*SquareSize+Offset;
                        Pieces[i].Rect.y = y*SquareSize+Offset;
                        MovedAlready.push_back(i);
                        break;
                    }
                }
            }
        }
    }
}

/*Start of Movements */

//********************Pawn Movement********************
int IsPawnBlocked(int selectedPiece)
{
    if (Pieces[selectedPiece].IsWhite)
    {
        //tests if there is a piece infront of the pawn
        for (int i = 0; i <= 2; i++)
        {
            //finds a match for a piece infront of a pawn
            int Block = FindMatch(Pieces[selectedPiece].OldLocation.x, Pieces[selectedPiece].OldLocation.y-i*SquareSize, selectedPiece);
            if (Block >= 0) return i; //returns the distance bewteen the piece and the pawn
        }
    }
    else if (!Pieces[selectedPiece].IsWhite)
    {
        //tests if there is a piece infront of the pawn
        for (int i = 0; i <= 2; i++)
        {
            //finds a match for a piece infront of a pawn
            int Block = FindMatch(Pieces[selectedPiece].OldLocation.x, Pieces[selectedPiece].OldLocation.y+i*SquareSize, selectedPiece);
            if (Block >= 0) return i; //returns the distance between the piece and the pawn
        }
    }

    return 0;
}

/*void Promotion (int selectedPiece)
{
    if (Pieces[selectedPiece].Rect.y == Offset && Pieces[selectedPiece].IsWhite && PromotionQuit)
    {
        PromotionWindow = SDL_CreateWindow( "Promotion", 8+ScreenWidth/2, 31, ScreenWidth/4, ScreenHeight/4, SDL_WINDOW_SHOWN );
        PromotionRenderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED );
        PromotionQuit = false;
    }
    else if (Pieces[selectedPiece].Rect.y == Offset+7*SquareSize && !Pieces[selectedPiece].IsWhite && PromotionQuit)
    {
        PromotionWindow = SDL_CreateWindow( "Promotion", 8+ScreenWidth/2, 31, ScreenWidth/4, ScreenHeight/4, SDL_WINDOW_SHOWN );
        PromotionRenderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED );
        PromotionQuit = false;
    }
}*/

bool PawnMove(int selectedPiece)
{
    //finds if there is a piece blocking the pawn
    int SomethingInfrontOfPawn = IsPawnBlocked(selectedPiece);

    //if the pawn can move 2 squares
    if (Pieces[selectedPiece].FirstMove)
    {
        if (Pieces[selectedPiece].IsWhite) // White pawn first move
        {
            //tests if the pawn has made a valid 1 or 2 square move
            if (Pieces[selectedPiece].Rect.y == Pieces[selectedPiece].OldLocation.y - 2*SquareSize && Pieces[selectedPiece].Rect.x == Pieces[selectedPiece].OldLocation.x && SomethingInfrontOfPawn == 0)
                return true;
            if (Pieces[selectedPiece].Rect.y == Pieces[selectedPiece].OldLocation.y - SquareSize && Pieces[selectedPiece].Rect.x == Pieces[selectedPiece].OldLocation.x && SomethingInfrontOfPawn != 1)
                return true;
        }
        else //Black pawn first move
        {
            //tests if the pawn has made a valid 1 or 2 square move
            if (Pieces[selectedPiece].Rect.y == Pieces[selectedPiece].OldLocation.y + 2*SquareSize && Pieces[selectedPiece].Rect.x == Pieces[selectedPiece].OldLocation.x && SomethingInfrontOfPawn == 0)
                return true;
            if (Pieces[selectedPiece].Rect.y == Pieces[selectedPiece].OldLocation.y + SquareSize && Pieces[selectedPiece].Rect.x == Pieces[selectedPiece].OldLocation.x && SomethingInfrontOfPawn != 1)
                return true;
        }
    }
    else //Regular One square moving after the first move
    {
        if (Pieces[selectedPiece].IsWhite)
        {
            //if the piece made a valid 1 square movement
            if (Pieces[selectedPiece].Rect.y == Pieces[selectedPiece].OldLocation.y - SquareSize && Pieces[selectedPiece].Rect.x == Pieces[selectedPiece].OldLocation.x && SomethingInfrontOfPawn != 1)
            {
                //Promotion(selectedPiece);
                return true;
            }
        }
        else
        {
            //if the piece made a valid 1 square movement
            if (Pieces[selectedPiece].Rect.y == Pieces[selectedPiece].OldLocation.y + SquareSize && Pieces[selectedPiece].Rect.x == Pieces[selectedPiece].OldLocation.x && SomethingInfrontOfPawn != 1)
            {
                //Promotion(selectedPiece);
                return true;
            }
        }
    }

    if (Pieces[selectedPiece].IsWhite) //White Pawns Taking Pieces
    {
        //finds a match for the piece being taken
        int AttackingPiece = FindMatch(Pieces[selectedPiece].Rect.x,Pieces[selectedPiece].Rect.y,selectedPiece);
        //if the pieces are different colours and there is a match for the piece
        if (Pieces[AttackingPiece].IsWhite != Pieces[selectedPiece].IsWhite && AttackingPiece >= 0)
            //if the pawn makes a valid attack
            if ((Pieces[selectedPiece].Rect.y == Pieces[selectedPiece].OldLocation.y - SquareSize && Pieces[selectedPiece].Rect.x == Pieces[selectedPiece].OldLocation.x-SquareSize)||(Pieces[selectedPiece].Rect.y == Pieces[selectedPiece].OldLocation.y - SquareSize && Pieces[selectedPiece].Rect.x == Pieces[selectedPiece].OldLocation.x+SquareSize))
            {
                //Promotion(selectedPiece);
                return true;
            }
    }
    else //Black Pawns Taking Pieces
    {
        //finds a match for the piece being taken
        int AttackingPiece = FindMatch(Pieces[selectedPiece].Rect.x,Pieces[selectedPiece].Rect.y,selectedPiece);
        //if the pieces are different colours and there is a match for the piece
        if (Pieces[AttackingPiece].IsWhite != Pieces[selectedPiece].IsWhite)
            //if the pawn makes a valid attack
            if ((Pieces[selectedPiece].Rect.y == Pieces[selectedPiece].OldLocation.y + SquareSize && Pieces[selectedPiece].Rect.x == Pieces[selectedPiece].OldLocation.x+SquareSize)||(Pieces[selectedPiece].Rect.y == Pieces[selectedPiece].OldLocation.y + SquareSize && Pieces[selectedPiece].Rect.x == Pieces[selectedPiece].OldLocation.x-SquareSize))
            {
                //Promotion(selectedPiece);
                return true;
            }
    }
    return false;
}

//*******************Knight Movement*******************

bool IsKnightBlocked(int selectedPiece)
{
    //finds a match to the square the piece is moving too
    int MovingToSquare = FindMatch(Pieces[selectedPiece].Rect.x, Pieces[selectedPiece].Rect.y, selectedPiece);

    if (MovingToSquare >= 0)
        //if there if a piece there and it is the same colour as the moving piece then cancel the move
        if (Pieces[selectedPiece].IsWhite == Pieces[MovingToSquare].IsWhite)
            return true;

    return false;
}

bool KnightMove(int selectedPiece)
{
    //tests if there is something blocking the knight
    bool SomethingInKnightMove = IsKnightBlocked(selectedPiece);

    if (SomethingInKnightMove)
    {
        cout << "Something is in the way" << endl;
        return false; //cancel movement
    }

    //Tests to see if the piece has been moved in a valid patturn
    if (Pieces[selectedPiece].OldLocation.y + 2*SquareSize == Pieces[selectedPiece].Rect.y && Pieces[selectedPiece].OldLocation.x + SquareSize == Pieces[selectedPiece].Rect.x) return true;
    if (Pieces[selectedPiece].OldLocation.y + 2*SquareSize == Pieces[selectedPiece].Rect.y && Pieces[selectedPiece].OldLocation.x - SquareSize == Pieces[selectedPiece].Rect.x) return true;
    if (Pieces[selectedPiece].OldLocation.y - 2*SquareSize == Pieces[selectedPiece].Rect.y && Pieces[selectedPiece].OldLocation.x + SquareSize == Pieces[selectedPiece].Rect.x) return true;
    if (Pieces[selectedPiece].OldLocation.y - 2*SquareSize == Pieces[selectedPiece].Rect.y && Pieces[selectedPiece].OldLocation.x - SquareSize == Pieces[selectedPiece].Rect.x) return true;
    if (Pieces[selectedPiece].OldLocation.x + 2*SquareSize == Pieces[selectedPiece].Rect.x && Pieces[selectedPiece].OldLocation.y + SquareSize == Pieces[selectedPiece].Rect.y) return true;
    if (Pieces[selectedPiece].OldLocation.x + 2*SquareSize == Pieces[selectedPiece].Rect.x && Pieces[selectedPiece].OldLocation.y - SquareSize == Pieces[selectedPiece].Rect.y) return true;
    if (Pieces[selectedPiece].OldLocation.x - 2*SquareSize == Pieces[selectedPiece].Rect.x && Pieces[selectedPiece].OldLocation.y + SquareSize == Pieces[selectedPiece].Rect.y) return true;
    if (Pieces[selectedPiece].OldLocation.x - 2*SquareSize == Pieces[selectedPiece].Rect.x && Pieces[selectedPiece].OldLocation.y - SquareSize == Pieces[selectedPiece].Rect.y) return true;
    return false;
}

//*******************Bishop Movement*******************

bool IsBishopBlocked(int selectedPiece)
{
    int xcheck = Pieces[selectedPiece].OldLocation.x;

    //if the bishop is moving down and to the right
    if (Pieces[selectedPiece].OldLocation.y < Pieces[selectedPiece].Rect.y && Pieces[selectedPiece].OldLocation.x < Pieces[selectedPiece].Rect.x)
    {
        //checks for anything diagonally blocking the bishop
        for (int checky = Pieces[selectedPiece].OldLocation.y; checky <= Pieces[selectedPiece].Rect.y; checky += SquareSize)
        {
            if (checky == Pieces[selectedPiece].Rect.y && xcheck == Pieces[selectedPiece].Rect.x) return false;
            //finds a match to the testing location, cancels the move if there is something blocking it
            if (FindMatch(xcheck,checky,selectedPiece)>= 0) return true;
            xcheck+= SquareSize;
        }
    }
    //if the bishop is moving down and to the left
    else if (Pieces[selectedPiece].OldLocation.y < Pieces[selectedPiece].Rect.y && Pieces[selectedPiece].OldLocation.x > Pieces[selectedPiece].Rect.x)
    {
        //checks for anything diagonally blocking the bishop
        for (int checky = Pieces[selectedPiece].OldLocation.y; checky <= Pieces[selectedPiece].Rect.y; checky += SquareSize)
        {
            if (checky == Pieces[selectedPiece].Rect.y && xcheck == Pieces[selectedPiece].Rect.x) return false;
            //finds a match to the testing location, cancels the move if there is something blocking it
            if (FindMatch(xcheck,checky,selectedPiece)>= 0) return true;
            xcheck-= SquareSize;
        }
    }
    //if the bishop is moving up and to the right
    else if (Pieces[selectedPiece].OldLocation.y > Pieces[selectedPiece].Rect.y && Pieces[selectedPiece].OldLocation.x < Pieces[selectedPiece].Rect.x)
    {
        //checks for anything diagonally blocking the bishop
        for (int checky = Pieces[selectedPiece].OldLocation.y; checky >= Pieces[selectedPiece].Rect.y; checky -= SquareSize)
        {
            if (checky == Pieces[selectedPiece].Rect.y && xcheck == Pieces[selectedPiece].Rect.x) return false;
            //finds a match to the testing location, cancels the move if there is something blocking it
            if (FindMatch(xcheck,checky,selectedPiece)>= 0) return true;
            xcheck+= SquareSize;
        }
    }
    //if the bishop is moving up and to the left
    else if (Pieces[selectedPiece].OldLocation.y > Pieces[selectedPiece].Rect.y && Pieces[selectedPiece].OldLocation.x > Pieces[selectedPiece].Rect.x)
    {
        //checks for anything diagonally blocking the bishop
        for (int checky = Pieces[selectedPiece].OldLocation.y; checky >= Pieces[selectedPiece].Rect.y; checky -= SquareSize)
        {
            if (checky == Pieces[selectedPiece].Rect.y && xcheck == Pieces[selectedPiece].Rect.x) return false;
            //finds a match to the testing location, cancels the move if there is something blocking it
            if (FindMatch(xcheck,checky,selectedPiece) >= 0) return true;
            xcheck-= SquareSize;
        }
    }
    return true;
}

bool BishopMove(int selectedPiece)
{
    //returns with a good move if nothing is blocking the bishop
    if (!IsBishopBlocked(selectedPiece))return true;
    return false;
}

//********************Rook Movement********************

bool IsRookBlocked(int selectedPiece)
{
    //if the rook has been moved down
    if (Pieces[selectedPiece].OldLocation.y < Pieces[selectedPiece].Rect.y && Pieces[selectedPiece].OldLocation.x == Pieces[selectedPiece].Rect.x)
    {
        //tests all the squares along the movement path for something blocking it
        for (int checky = Pieces[selectedPiece].OldLocation.y; checky <= Pieces[selectedPiece].Rect.y; checky += SquareSize)
        {
            if (checky == Pieces[selectedPiece].Rect.y) return false; //when the desired square has been reached finish the move
            if (FindMatch(Pieces[selectedPiece].Rect.x,checky,selectedPiece)>= 0) return true; //finds a match for the squares along the movement path
        }
    }
    //if the rook has been moved up
    else if (Pieces[selectedPiece].OldLocation.y > Pieces[selectedPiece].Rect.y && Pieces[selectedPiece].OldLocation.x == Pieces[selectedPiece].Rect.x)
    {
        //tests all the squares along the movement path for something blocking it
        for (int checky = Pieces[selectedPiece].OldLocation.y; checky >= Pieces[selectedPiece].Rect.y; checky -= SquareSize)
        {
            if (checky == Pieces[selectedPiece].Rect.y) return false; //when the desired square has been reached finish the move
            if (FindMatch(Pieces[selectedPiece].Rect.x,checky,selectedPiece)>= 0) return true; //finds a match for the squares along the movement path
        }
    }
    //if the rook has been moved right
    else if (Pieces[selectedPiece].OldLocation.y == Pieces[selectedPiece].Rect.y && Pieces[selectedPiece].OldLocation.x < Pieces[selectedPiece].Rect.x)
    {
        //tests all the squares along the movement path for something blocking it
        for (int checkx = Pieces[selectedPiece].OldLocation.x; checkx <= Pieces[selectedPiece].Rect.x; checkx += SquareSize)
        {
            if (checkx == Pieces[selectedPiece].Rect.x) return false; //when the desired square has been reached finish the move
            if (FindMatch(checkx,Pieces[selectedPiece].Rect.y,selectedPiece)>= 0) return true; //finds a match for the squares along the movement path
        }
    }
    //if the rook has been moved right
    else if (Pieces[selectedPiece].OldLocation.y == Pieces[selectedPiece].Rect.y && Pieces[selectedPiece].OldLocation.x > Pieces[selectedPiece].Rect.x)
    {
        //tests all the squares along the movement path for something blocking it
        for (int checkx = Pieces[selectedPiece].OldLocation.x; checkx >= Pieces[selectedPiece].Rect.x; checkx -= SquareSize)
        {
            if (checkx == Pieces[selectedPiece].Rect.x) return false; //when the desired square has been reached finish the move
            if (FindMatch(checkx,Pieces[selectedPiece].Rect.y,selectedPiece)>= 0) return true; //finds a match for the squares along the movement path
        }
    }
    return true;
}

bool RookMove(int selectedPiece)
{
    //whens there is nothing blocking the rook, finish the move
    if (!IsRookBlocked(selectedPiece))return true;
    return false;
}

//********************King Movement********************
bool KingMove(int selectedPiece, bool AiTurn)
{
    //if the king is eligable for castling
    if (Pieces[selectedPiece].FirstMove)
    {
        if (Pieces[selectedPiece].IsWhite)
        {
            //if a king side castle is attempted
            if (Pieces[selectedPiece].Rect.x == Pieces[selectedPiece].OldLocation.x + 2*SquareSize && Pieces[selectedPiece].Rect.y == Pieces[selectedPiece].OldLocation.y)
            {
                //if the rook is eligable for castling
                if (Pieces[17].FirstMove)
                {
                    //finds matches for pieces in the way along the movement paths of both pieces
                    for (int i = 0; i <= 2; i++)
                    {
                        Pieces[selectedPiece].Rect.x = Pieces[selectedPiece].OldLocation.x + i*SquareSize;
                        //ensures the king does not move through, into or out of check
                        if (CheckForCheck(selectedPiece)) return false;
                        //tests to see if there is something in the way, if there is it cancels the move
                        if (FindMatch(Pieces[selectedPiece].OldLocation.x+i*SquareSize,Pieces[selectedPiece].OldLocation.y,selectedPiece) != -1)
                        {
                            //cout << "Something is in the way" << endl;
                            return false;
                        }
                    }
                    //sets the location of both pieces
                    Pieces[selectedPiece].Rect.x = Pieces[selectedPiece].OldLocation.x + 2*SquareSize;
                    Pieces[17].Rect.x = Pieces[17].Rect.x - 2*SquareSize;
                    return true;
                }
            }
            //if a queen side castle is attempted
            if (Pieces[selectedPiece].Rect.x == Pieces[selectedPiece].OldLocation.x - 2*SquareSize && Pieces[selectedPiece].Rect.y == Pieces[selectedPiece].OldLocation.y)
            {
                //if the rook is eligable for castling
                if (Pieces[16].FirstMove)
                {
                    //finds matches for pieces in the way along the movement paths of both pieces
                    for (int i = 0; i <= 2; i++)
                    {
                        Pieces[selectedPiece].Rect.x = Pieces[selectedPiece].OldLocation.x - i*SquareSize;
                        //ensures the king does not move through, into or out of check
                        if (CheckForCheck(selectedPiece)) return false;
                        //tests to see if there is something in the way, if there is it cancels the move
                        if (FindMatch(Pieces[selectedPiece].OldLocation.x-i*SquareSize,Pieces[selectedPiece].OldLocation.y,selectedPiece) != -1)
                        {
                            //cout << "Something is in the way" << endl;
                            return false;
                        }
                    }
                    //sets the location of both pieces
                    Pieces[selectedPiece].Rect.x = Pieces[selectedPiece].OldLocation.x - 2*SquareSize;
                    Pieces[16].Rect.x = Pieces[16].Rect.x + 3*SquareSize;
                    return true;
                }
            }
        }
        else
        {
            //if a king side castle is attempted
            if (Pieces[selectedPiece].Rect.x == Pieces[selectedPiece].OldLocation.x + 2*SquareSize && Pieces[selectedPiece].Rect.y == Pieces[selectedPiece].OldLocation.y)
            {
                //if the rook is eligable for castling
                if (Pieces[19].FirstMove)
                {
                    //finds matches for pieces in the way along the movement paths of both pieces
                    for (int i = 0; i <= 2; i++)
                    {
                        Pieces[selectedPiece].Rect.x = Pieces[selectedPiece].OldLocation.x + i*SquareSize;
                        //ensures the king does not move through, into or out of check
                        if (CheckForCheck(selectedPiece)) return false;
                        //tests to see if there is something in the way, if there is it cancels the move
                        if (FindMatch(Pieces[selectedPiece].OldLocation.x+i*SquareSize,Pieces[selectedPiece].OldLocation.y,selectedPiece) != -1)
                        {
                            //cout << "Something is in the way" << endl;
                            return false;
                        }
                    }
                    //sets the location of both pieces
                    Pieces[selectedPiece].Rect.x = Pieces[selectedPiece].OldLocation.x + 2*SquareSize;
                    Pieces[19].Rect.x = Pieces[19].Rect.x - 2*SquareSize;
                    return true;
                }
            }
            //if a queen side castle is attempted
            if (Pieces[selectedPiece].Rect.x == Pieces[selectedPiece].OldLocation.x - 2*SquareSize && Pieces[selectedPiece].Rect.y == Pieces[selectedPiece].OldLocation.y)
            {
                //if the rook is eligable for castling
                if (Pieces[18].FirstMove)
                {
                    //finds matches for pieces in the way along the movement paths of both pieces
                    for (int i = 0; i <= 2; i++)
                    {
                        Pieces[selectedPiece].Rect.x = Pieces[selectedPiece].OldLocation.x - i*SquareSize;
                        //ensures the king does not move through, into or out of check
                        if (CheckForCheck(selectedPiece)) return false;
                        //tests to see if there is something in the way, if there is it cancels the move
                        if (FindMatch(Pieces[selectedPiece].OldLocation.x-i*SquareSize,Pieces[selectedPiece].OldLocation.y,selectedPiece) != -1)
                        {
                            //cout << "Something is in the way" << endl;
                            return false;
                        }
                    }
                    //sets the location of both pieces
                    Pieces[selectedPiece].Rect.x = Pieces[selectedPiece].OldLocation.x - 2*SquareSize;
                    Pieces[18].Rect.x = Pieces[18].Rect.x + 3*SquareSize;
                    return true;
                }
            }
        }
    }

    //checks to see if the king has moved in any other way
    if (Pieces[selectedPiece].Rect.x == Pieces[selectedPiece].OldLocation.x && Pieces[selectedPiece].Rect.y == Pieces[selectedPiece].OldLocation.y+SquareSize) return true;
    if (Pieces[selectedPiece].Rect.x == Pieces[selectedPiece].OldLocation.x && Pieces[selectedPiece].Rect.y == Pieces[selectedPiece].OldLocation.y-SquareSize) return true;
    if (Pieces[selectedPiece].Rect.x == Pieces[selectedPiece].OldLocation.x+SquareSize && Pieces[selectedPiece].Rect.y == Pieces[selectedPiece].OldLocation.y) return true;
    if (Pieces[selectedPiece].Rect.x == Pieces[selectedPiece].OldLocation.x-SquareSize && Pieces[selectedPiece].Rect.y == Pieces[selectedPiece].OldLocation.y) return true;
    if (Pieces[selectedPiece].Rect.x == Pieces[selectedPiece].OldLocation.x+SquareSize && Pieces[selectedPiece].Rect.y == Pieces[selectedPiece].OldLocation.y+SquareSize) return true;
    if (Pieces[selectedPiece].Rect.x == Pieces[selectedPiece].OldLocation.x-SquareSize && Pieces[selectedPiece].Rect.y == Pieces[selectedPiece].OldLocation.y+SquareSize) return true;
    if (Pieces[selectedPiece].Rect.x == Pieces[selectedPiece].OldLocation.x+SquareSize && Pieces[selectedPiece].Rect.y == Pieces[selectedPiece].OldLocation.y-SquareSize) return true;
    if (Pieces[selectedPiece].Rect.x == Pieces[selectedPiece].OldLocation.x-SquareSize && Pieces[selectedPiece].Rect.y == Pieces[selectedPiece].OldLocation.y-SquareSize) return true;

    return false;
}

//********************Queen Movement*******************

bool IsQueenBlocked(int selectedPiece)
{
    int xcheck = Pieces[selectedPiece].OldLocation.x;

    //Diagonally moving down right
    if (Pieces[selectedPiece].OldLocation.y < Pieces[selectedPiece].Rect.y && Pieces[selectedPiece].OldLocation.x < Pieces[selectedPiece].Rect.x)
    {
        //looks for any pieces in the path of the queen
        for (int checky = Pieces[selectedPiece].OldLocation.y; checky <= Pieces[selectedPiece].Rect.y; checky += SquareSize)
        {
            //if the destination square has been reached complete the move
            if (checky == Pieces[selectedPiece].Rect.y && xcheck == Pieces[selectedPiece].Rect.x)return false;
            //if a pieces is found in the way cancel the move
            if (FindMatch(xcheck,checky,selectedPiece)>= 0) return true;
            xcheck+= SquareSize;
        }
    }
    //Diagonally moving down left
    else if (Pieces[selectedPiece].OldLocation.y < Pieces[selectedPiece].Rect.y && Pieces[selectedPiece].OldLocation.x > Pieces[selectedPiece].Rect.x)
    {
        //looks for any pieces in the path of the queen
        for (int checky = Pieces[selectedPiece].OldLocation.y; checky <= Pieces[selectedPiece].Rect.y; checky += SquareSize)
        {
            //if the destination square has been reached complete the move
            if (checky == Pieces[selectedPiece].Rect.y && xcheck == Pieces[selectedPiece].Rect.x)return false;
            //if a pieces is found in the way cancel the move
            if (FindMatch(xcheck,checky,selectedPiece)>= 0) return true;
            xcheck-= SquareSize;
        }
    }
    //Diagonally moving up right
    else if (Pieces[selectedPiece].OldLocation.y > Pieces[selectedPiece].Rect.y && Pieces[selectedPiece].OldLocation.x < Pieces[selectedPiece].Rect.x)
    {
        //looks for any pieces in the path of the queen
        for (int checky = Pieces[selectedPiece].OldLocation.y; checky >= Pieces[selectedPiece].Rect.y; checky -= SquareSize)
        {
            //if the destination square has been reached complete the move
            if (checky == Pieces[selectedPiece].Rect.y && xcheck == Pieces[selectedPiece].Rect.x)return false;
            //if a pieces is found in the way cancel the move
            if (FindMatch(xcheck,checky,selectedPiece)>= 0) return true;
            xcheck+= SquareSize;
        }
    }
    //Diagonally moving up left
    else if (Pieces[selectedPiece].OldLocation.y > Pieces[selectedPiece].Rect.y && Pieces[selectedPiece].OldLocation.x > Pieces[selectedPiece].Rect.x)
    {
        //looks for any pieces in the path of the queen
        for (int checky = Pieces[selectedPiece].OldLocation.y; checky >= Pieces[selectedPiece].Rect.y; checky -= SquareSize)
        {
            //if the destination square has been reached complete the move
            if (checky == Pieces[selectedPiece].Rect.y && xcheck == Pieces[selectedPiece].Rect.x)return false;
            //if a pieces is found in the way cancel the move
            if (FindMatch(xcheck,checky,selectedPiece) >= 0) return true;

            xcheck-= SquareSize;
        }
    }
    //moving down
    else if (Pieces[selectedPiece].OldLocation.y < Pieces[selectedPiece].Rect.y && Pieces[selectedPiece].OldLocation.x == Pieces[selectedPiece].Rect.x)
    {
        //looks for any pieces in the path of the queen
        for (int checky = Pieces[selectedPiece].OldLocation.y; checky <= Pieces[selectedPiece].Rect.y; checky += SquareSize)
        {
            //if the destination square has been reached complete the move
            if (checky == Pieces[selectedPiece].Rect.y)return false;
            //if a pieces is found in the way cancel the move
            if (FindMatch(Pieces[selectedPiece].Rect.x,checky,selectedPiece)>= 0) return true;
        }
    }
    //moving up
    else if (Pieces[selectedPiece].OldLocation.y > Pieces[selectedPiece].Rect.y && Pieces[selectedPiece].OldLocation.x == Pieces[selectedPiece].Rect.x)
    {
        //looks for any pieces in the path of the queen
        for (int checky = Pieces[selectedPiece].OldLocation.y; checky >= Pieces[selectedPiece].Rect.y; checky -= SquareSize)
        {
            //if the destination square has been reached complete the move
            if (checky == Pieces[selectedPiece].Rect.y)return false;
            //if a pieces is found in the way cancel the move
            if (FindMatch(Pieces[selectedPiece].Rect.x,checky,selectedPiece)>= 0) return true;
        }
    }
    //moveing right
    else if (Pieces[selectedPiece].OldLocation.y == Pieces[selectedPiece].Rect.y && Pieces[selectedPiece].OldLocation.x < Pieces[selectedPiece].Rect.x)
    {
        //looks for any pieces in the path of the queen
        for (int checkx = Pieces[selectedPiece].OldLocation.x; checkx <= Pieces[selectedPiece].Rect.x; checkx += SquareSize)
        {
            //if the destination square has been reached complete the move
            if (checkx == Pieces[selectedPiece].Rect.x)return false;
            //if a pieces is found in the way cancel the move
            if (FindMatch(checkx,Pieces[selectedPiece].Rect.y,selectedPiece)>= 0) return true;
        }
    }
    //Moving left
    else if (Pieces[selectedPiece].OldLocation.y == Pieces[selectedPiece].Rect.y && Pieces[selectedPiece].OldLocation.x > Pieces[selectedPiece].Rect.x)
    {
        //looks for any pieces in the path of the queen
        for (int checkx = Pieces[selectedPiece].OldLocation.x; checkx >= Pieces[selectedPiece].Rect.x; checkx -= SquareSize)
        {
            //if the destination square has been reached complete the move
            if (checkx == Pieces[selectedPiece].Rect.x)return false;
            //if a pieces is found in the way cancel the move
            if (FindMatch(checkx,Pieces[selectedPiece].Rect.y,selectedPiece)>= 0) return true;
        }
    }
    return true;
}

bool QueenMove(int selectedPiece)
{
    //if nothing is blocking the queen then finish the move
    if (!IsQueenBlocked(selectedPiece))return true;
    return false;
}

/*End of Movements*/

/*int badPawnsBlack(TPiece someBoard[8][8])
{
    int badPawns = 0;
    int colsWpawns[8] = {0,0,0,0,0,0,0,0};
    for(int x = 0; x < 8; x++)
    {
        for(int y = 0; y < 8; y++)
        {
            if(someBoard[x][y]==blackpawn)
                colsWpawns[x]++;
        }
    }
    for(int x = 0; x < 8; x++)
    {
        if(x==0)
        {
            if(colsWpawns[x+1]==0)
                badPawns+=colsWpawns[x];
            else if(colsWpawns[x]>1)
                badPawns+=colsWpawns[x]-1;
        }
        else if(x==7)
        {
            if(colsWpawns[x-1]==0)
                badPawns+=colsWpawns[x];
            else if(colsWpawns[x]>1)
                badPawns+=colsWpawns[x]-1;
        }
        else
        {
            if(colsWpawns[x-1]==0 and colsWpawns[x+1]==0)
                badPawns+=colsWpawns[x];
            else if(colsWpawns[x]>1)
                badPawns+=colsWpawns[x]-1;
        }
    }
    return badPawns;
}

int badPawnsWhite(TPiece someBoard[8][8])
{
    int badPawns = 0;
    int colsWpawns[8] = {0,0,0,0,0,0,0,0};
    for(int x = 0; x < 8; x++)
    {
        for(int y = 0; y < 8; y++)
        {
            if(someBoard[x][y]==whitepawn)
                colsWpawns[x]++;
        }
    }
    for(int x = 0; x < 8; x++)
    {
        if(x==0)
        {
            if(colsWpawns[x+1]==0)
                badPawns+=colsWpawns[x];
            else if(colsWpawns[x]>1)
                badPawns+=colsWpawns[x]-1;
        }
        else if(x==7)
        {
            if(colsWpawns[x-1]==0)
                badPawns+=colsWpawns[x];
            else if(colsWpawns[x]>1)
                badPawns+=colsWpawns[x]-1;
        }
        else
        {
            if(colsWpawns[x-1]==0 and colsWpawns[x+1]==0)
                badPawns+=colsWpawns[x];
            else if(colsWpawns[x]>1)
                badPawns+=colsWpawns[x]-1;
        }
    }
    return badPawns;
}*/

AiMovementTree FindBestMove (AiMovementTree node, bool aiTurn,int depth)
{
    int NumberOfMoves;
    vector <AiMovementTree> BoardMovements;

    if(depth <= 2) //if the max depth hasn't been reached, then add moves to tree
    {
        //calls the funtcion that create variable possibleMoves
        BoardMovements = GenerateMoveSet(node.CurrentBoard,aiTurn,&NumberOfMoves);
        //cycles through the generated moves
        for (int i = 0; i < NumberOfMoves; i++)
        {
            //collects the information about each generated move
            AiMovementTree CurrentBoard;
            CurrentBoard = BoardMovements[i];
            AiMovementTree newMove;
            newMove = CurrentBoard;
            //adds it into the origional nodes poissible moves
            node.FutureMoves.push_back(newMove);
        }
    }
    depth++; //increase the depth

    if(node.FutureMoves.size()!=0) //if the node has children
    {
        int num = node.FutureMoves.size();
        //cycle through each of the possible moves for that node
        for(int x = 0; x < num; x++)
        {
            //finds the best possible move for next move
            AiMovementTree Recursive;
            Recursive = node.FutureMoves[x];
            node.FutureMoves[x] = FindBestMove(Recursive,!aiTurn,depth); //recursively searching the tree
        }

    }
    else //if the node has no children, find a value for its board
    {
        //finds the values of the given board position
        node.value = evaluate(node.CurrentBoard,aiTurn,-1);
        return node; //return node, since no children
    }

    if(aiTurn) //if it's the ai's turn
    {
        node.value=best; //set the value to beat as the worst
        //look through all the moves associated the the node
        for(int x = 0; x < (int)node.FutureMoves.size(); x++)
        {
            //keep the highest value, because the AI wants to make the best move
            if(node.FutureMoves[x].value < node.value)
            {
                node.value = node.FutureMoves[x].value;
            }
        }
        //remove all children from the node
        for(int x = 0; x < (int)node.FutureMoves.size(); x++)
            node.FutureMoves.pop_back();
    }
    else //if it is not the ai's turn
    {
        node.value=-best; //set the value to beat to the best
        for(int x = 0; x < (int)node.FutureMoves.size(); x++)
        {
            //keep the lowest possible value, because the AI wants the player to make the worst move
            if(node.FutureMoves[x].value > node.value)
            {
                //keeps all the information of the new board
                node.value = node.FutureMoves[x].value;
                node.CurrentBoard = node.FutureMoves[x].CurrentBoard;
                node.TakenPiece = node.FutureMoves[x].TakenPiece;
                node.CurrentPiece = node.FutureMoves[x].CurrentPiece;
            }
        }
        //remove all the children from the node
        for(int x = 0; x < (int)node.FutureMoves.size(); x++)
            node.FutureMoves.pop_back();
    }
    return node;
}

//this function evaluates a board posistion and assigns a value based on the the following criteria:
//material, mobility(# of squares you can move to), bad pawns, and set values for stalemate and checkmate
int evaluate(col Board, bool AiTurn,int Taken)
{
    int evaluation = 0; //value to return, starts at zero

    //if stalemate, return 0
    if(stalemate(Board))
        return 0;

    //AI won by checkmate, return best possible value
    if(WhiteInCheckmate())
        return best;
    //AI lost by checkmate, return worst possible value
    if(BlackInCheckmate())
        return -best;

    //double loops to look through the entire board
    for(int x = 0; x < 8; x++)
    {
        for(int y = 0; y < 8; y++)
        {
            if(Board[x][y]==blackpawn) evaluation +=10; //add 10 points for AI's pawn
            else if(Board[x][y]==whitepawn) evaluation -=10; //subtract 10 points for opponent's pawn
            else if(Board[x][y]==blackknight) evaluation +=30; //add 30 points for AI's knight
            else if(Board[x][y]==whiteknight) evaluation -=30; //subtract 30 points for opponent's knight
            else if(Board[x][y]==blackbishop) evaluation +=32; //add 32 points for AI's bishop
            else if(Board[x][y]==whitebishop) evaluation -=32; //subtract 32 points for opponent's bishop
            else if(Board[x][y]==blackrook) evaluation +=50; //add 50 points for AI's rook
            else if(Board[x][y]==whiterook) evaluation -=50; //subtract 50 points for opponent's rook
            else if(Board[x][y]==blackqueen) evaluation +=90; //add 90 points for AI's queen
            else if(Board[x][y]==whitequeen) evaluation -=90; //subtract 90 points for opponent's queen
        }
    }

    //add 2 points for every square the AI can move to, subtract 2 for the square the opponent can move to
    int NumberBMoves;
    int NumberWMoves;

    GenerateMoveSet(Board,false,&NumberBMoves);//finds the number of moves black has
    GenerateMoveSet(Board,true,&NumberWMoves); //finds the numebr of moves white has

    //assign the values to their global partners
    NumberBlackMoves = NumberBMoves;
    NumberWhiteMoves = NumberWMoves;

    //gives the mobility a value
    evaluation += (NumberBMoves - NumberWMoves)/4;

    return evaluation; //return evaluation of board
}

vector <AiMovementTree> GenerateMoveSet(col Board, bool WhiteMoves,int *NumberOfMoves)
{
    //creates a vector to hold all the moves
    vector <AiMovementTree> PossibleMoves;

    //creates two tracking boards
    col newboard;
    col OldBoard;

    //sets the old board to the current board
    OldBoard = Board;

    //sets the render triangles of all the pieces to the current board
    updateScreen(Board);

    //sets all of the old locations of the pieces
    for (int i = 0; i < 32; i++)
    {
        if (Pieces[i].IsTaken) continue; //skips the piece if it has been taken
        Pieces[i].OldLocation = Pieces[i].Rect;
    }

    //assigns the proper numbers in the array that corispond to all of the pieces of a single colour
    int PieceNumbers[16] = {};
    if (WhiteMoves)
    {
        int Temp[16] = {0,1,2,3,4,5,6,7,16,17,20,21,24,25,28,30};
        for (int i = 0; i < 16; i++)
            PieceNumbers[i] = Temp[i];
    }
    else
    {
        int Temp[16] = {8,9,10,11,12,13,14,15,18,19,22,23,26,27,29,31};
        for (int i = 0; i < 16; i++)
            PieceNumbers[i] = Temp[i];
    }

    //cycles through all the pieces of one colour
    for (int i = 0; i < 16; i++)
    {
        //sets the current piece
        int MovingPiece = PieceNumbers[i];

        if (Pieces[MovingPiece].IsTaken) continue; //skip the piece if it has been taken

        //move through each square in the board
        for (int j = 0; j < 8; j++)
        {
            for (int k = 0; k < 8; k++)
            {
                //moves the piece to the current square
                Pieces[MovingPiece].Rect.x = k*SquareSize + Offset;
                Pieces[MovingPiece].Rect.y = j*SquareSize + Offset;

                //skips the old location of the piece
                if (Pieces[MovingPiece].OldLocation.x == Pieces[MovingPiece].Rect.x && Pieces[MovingPiece].OldLocation.y == Pieces[MovingPiece].Rect.y ) continue;

                //test whether the move is valid
                if (IsvalidMove(MovingPiece,true))
                {
                    bool TurnChange = true;
                    //tests if a pieces has been taken
                    int ToTake = PieceTake(MovingPiece);

                    if (CheckForCheck(30)) //checks if black has put white in check
                    {
                        if (WhiteTurn)//if the player tries to make a move that puts him into check
                        {
                            TurnChange = false;
                            Pieces[MovingPiece].Rect = Pieces[MovingPiece].OldLocation; //resets the piece's location
                            if (ToTake >= 0)
                            {
                                Pieces[ToTake].IsTaken = false; //resets the flags of the piece
                                Pieces[ToTake].Rect = Pieces[ToTake].OldLocation; //resets the taken piece's location
                            }
                        }
                    }

                    if (CheckForCheck(31))//checks if black has put white in check
                    {
                        if (!WhiteTurn)//if the player tries to make a move that puts him into check
                        {
                            TurnChange = false;
                            Pieces[MovingPiece].Rect = Pieces[MovingPiece].OldLocation;//resets the piece's location
                            if (ToTake >= 0)
                            {
                                Pieces[ToTake].IsTaken = false;//resets the flags of the piece
                                Pieces[ToTake].Rect = Pieces[ToTake].OldLocation;//resets the taken piece's location
                            }
                        }
                    }

                    if (TurnChange && ToTake != -2) //If the move has passed all of the check
                    {
                        if (ToTake >= 0) //if a piece has been taken
                        {
                            //resets the flags back to the origioanl board to be manipulated again
                            Pieces[ToTake].IsTaken = false;
                            Pieces[ToTake].Rect = Pieces[ToTake].OldLocation;
                        }

                        //updates the board to the new move
                        col UpdatedBoard = UpdateBoard(Board);

                        //fill in the tracking vectors with the current move
                        for (int l = 0; l < 8; l++)
                        {
                            row temp;
                            for (int m = 0; m < 8; m++)
                            {
                                temp.push_back(UpdatedBoard[l][m]);
                                UpdatedBoard[l][m] = OldBoard[l][m];
                            }
                            newboard.push_back(temp);
                        }

                        //sets all the values of the current move to a holder
                        AiMovementTree BoardHolder;
                        BoardHolder.CurrentBoard = newboard;
                        BoardHolder.CurrentPiece = MovingPiece;
                        BoardHolder.TakenPiece = ToTake;
                        //stores the holder into the main tree
                        PossibleMoves.push_back(BoardHolder);
                        TurnChange = false;
                        newboard.clear();
                    }
                }
                //returns the piece back to its origional place
                Pieces[MovingPiece].Rect = Pieces[MovingPiece].OldLocation;
            }
        }
    }

    //returns all the pieces back tot heir origional places
    for (int i = 0; i < 32; i++)
    {
        if (Pieces[i].IsTaken) continue;
        Pieces[i].Rect = Pieces[i].OldLocation;
    }


    *NumberOfMoves = PossibleMoves.size(); //finds the number of moves that player has
    updateScreen(Board); //makes the screen look like the origional board
    return PossibleMoves;
}

bool IsvalidMove(int selectedPiece,bool AiTurn)
{
    //ensures the mouse is on the board
    if (!AiTurn)
        if (mousex >= BoardHeight+50 || mousex < 50 || mousey >= BoardHeight+50 || mousey < 50) return false;

    //depending on the piece that moves, it tests for that specific move set of the piece
    //when true, the move is allowed, when false the move is cancelled and everything is reset
    if (Pieces[selectedPiece].TypeOfPiece == whitepawn || Pieces[selectedPiece].TypeOfPiece == blackpawn)
    {
        if (PawnMove(selectedPiece)) return true;
    }
    else if (Pieces[selectedPiece].TypeOfPiece == whiteknight || Pieces[selectedPiece].TypeOfPiece == blackknight)
    {
        if (KnightMove(selectedPiece)) return true;
    }
    else if (Pieces[selectedPiece].TypeOfPiece == whitebishop || Pieces[selectedPiece].TypeOfPiece == blackbishop)
    {
        if (BishopMove(selectedPiece)) return true;
    }
    else if (Pieces[selectedPiece].TypeOfPiece == whiterook || Pieces[selectedPiece].TypeOfPiece == blackrook)
    {
        if (RookMove(selectedPiece)) return true;
    }
    else if (Pieces[selectedPiece].TypeOfPiece == whitequeen || Pieces[selectedPiece].TypeOfPiece == blackqueen)
    {
        if (QueenMove(selectedPiece)) return true;
    }
    else if (Pieces[selectedPiece].TypeOfPiece == whiteking || Pieces[selectedPiece].TypeOfPiece == blackking)
    {
        if (KingMove(selectedPiece,AiTurn)) return true;
    }
    return false;
}

bool CheckForCheck(int KingColour)
{
    int Kingx = Pieces[KingColour].Rect.x;
    int Kingy = Pieces[KingColour].Rect.y;
    int Match = -1;

    //Check Up
    for (int checky = Kingy; checky >= Offset; checky -= SquareSize)
    {
        //checks all the squares above the king for any piece
        Match = FindMatch(Kingx,checky,KingColour);
        //tests if any of the matches are pieces that can actually put the king in check
        if (Match >= 0)
        {
            if ((Pieces[Match].TypeOfPiece == whiterook || Pieces[Match].TypeOfPiece == blackrook) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite)
            {
                return true;
            }
            if ((Pieces[Match].TypeOfPiece == whitequeen || Pieces[Match].TypeOfPiece == blackqueen) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite)
            {
                return true;
            }
            break;
        }
    }

    //Check Down
    for (int checky = Kingy; checky <= Offset+8*SquareSize; checky += SquareSize)
    {
        //checks all the squares below the king for any piece
        Match = FindMatch(Kingx,checky,KingColour);
        //tests if any of the matches are pieces that can actually put the king in check
        if (Match >= 0)
        {
            if ((Pieces[Match].TypeOfPiece == whiterook || Pieces[Match].TypeOfPiece == blackrook) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite)
            {
                return true;
            }
            if ((Pieces[Match].TypeOfPiece == whitequeen || Pieces[Match].TypeOfPiece == blackqueen) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite)
            {
                return true;
            }
            break;
        }
    }

    //Check Left
    for (int checkx = Kingx; checkx >= Offset; checkx -= SquareSize)
    {
        //checks all the squares to the left of the king for any piece
        Match = FindMatch(checkx,Kingy,KingColour);
        //tests if any of the matches are pieces that can actually put the king in check
        if (Match >= 0)
        {
            if ((Pieces[Match].TypeOfPiece == whiterook || Pieces[Match].TypeOfPiece == blackrook) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite)
            {
                return true;
            }
            if ((Pieces[Match].TypeOfPiece == whitequeen || Pieces[Match].TypeOfPiece == blackqueen) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite)
            {
                return true;
            }
            break;
        }
    }

    //Check Right
    for (int checkx = Kingx; checkx <= Offset+8*SquareSize; checkx += SquareSize)
    {
        //checks all the squares to the right of the king for any piece
        Match = FindMatch(checkx,Kingy,KingColour);
        //tests if any of the matches are pieces that can actually put the king in check
        if (Match >= 0)
        {
            cout << Pieces[Match].TypeOfPiece << endl;
            cout << whiterook << endl;
            cout << blackrook << endl;
            if ((Pieces[Match].TypeOfPiece == whiterook || Pieces[Match].TypeOfPiece == blackrook) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite);
            {
                return true;
            }
            if ((Pieces[Match].TypeOfPiece == whitequeen || Pieces[Match].TypeOfPiece == blackqueen) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite)
            {
                return true;
            }
            break;
        }
    }

    int checkx = Kingx;

    //Check Up Left
    for (int checky = Kingy; checky >= Offset; checky -= SquareSize)
    {
        //checks all the squares up and to the left of the king for any piece
        Match = FindMatch(checkx,checky,KingColour);
        //tests if any of the matches are pieces that can actually put the king in check
        if (Match >= 0)
        {
            if ((Pieces[Match].TypeOfPiece == whitepawn || Pieces[Match].TypeOfPiece == blackpawn) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite && checkx == Kingx-SquareSize && checky == Kingy-SquareSize && Pieces[Match].IsWhite == false)
            {
                return true;
            }
            if ((Pieces[Match].TypeOfPiece == whitebishop || Pieces[Match].TypeOfPiece == blackbishop) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite)
            {
                return true;
            }
            if ((Pieces[Match].TypeOfPiece == whitequeen || Pieces[Match].TypeOfPiece == blackqueen) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite)
            {
                return true;
            }
            break;
        }
        checkx-= SquareSize;
    }

    checkx = Kingx;

    //Check Up Right
    for (int checky = Kingy; checky >= Offset; checky -= SquareSize)
    {
        //checks all the squares up and to the right of the king for any piece
        Match = FindMatch(checkx,checky,KingColour);
        //tests if any of the matches are pieces that can actually put the king in check
        if (Match >= 0)
        {
            if ((Pieces[Match].TypeOfPiece == whitepawn || Pieces[Match].TypeOfPiece == blackpawn) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite && checkx == Kingx+SquareSize && checky == Kingy-SquareSize && Pieces[Match].IsWhite == false)
            {
                return true;
            }
            if ((Pieces[Match].TypeOfPiece == whitebishop || Pieces[Match].TypeOfPiece == blackbishop) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite)
            {
                return true;
            }
            if ((Pieces[Match].TypeOfPiece == whitequeen || Pieces[Match].TypeOfPiece == blackqueen) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite)
            {
                return true;
            }
            break;
        }
        checkx+= SquareSize;
    }

    checkx = Kingx;

    //Check Down Left
    for (int checky = Kingy; checky <= Offset+8*SquareSize; checky += SquareSize)
    {
        //checks all the squares down and to the left of the king for any piece
        Match = FindMatch(checkx,checky,KingColour);
        //tests if any of the matches are pieces that can actually put the king in check
        if (Match >= 0)
        {
            if ((Pieces[Match].TypeOfPiece == whitepawn || Pieces[Match].TypeOfPiece == blackpawn) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite && checkx == Kingx-SquareSize && checky == Kingy+SquareSize && Pieces[Match].IsWhite == true)
            {
                return true;
            }
            if ((Pieces[Match].TypeOfPiece == whitebishop || Pieces[Match].TypeOfPiece == blackbishop) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite)
            {
                return true;
            }
            if ((Pieces[Match].TypeOfPiece == whitequeen || Pieces[Match].TypeOfPiece == blackqueen) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite)
            {
                return true;
            }
            break;
        }
        checkx-= SquareSize;
    }

    checkx = Kingx;

    //Check Down Right
    for (int checky = Kingy; checky <= Offset+8*SquareSize; checky += SquareSize)
    {
        //checks all the squares down and to the right of the king for any piece
        Match = FindMatch(checkx,checky,KingColour);
        //tests if any of the matches are pieces that can actually put the king in check
        if (Match >= 0)
        {
            if ((Pieces[Match].TypeOfPiece == whitepawn || Pieces[Match].TypeOfPiece == blackpawn) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite && checkx == Kingx+SquareSize && checky == Kingy+SquareSize && Pieces[Match].IsWhite == true)
            {
                return true;
            }
            if ((Pieces[Match].TypeOfPiece == whitebishop || Pieces[Match].TypeOfPiece == blackbishop) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite)
            {
                return true;
            }

            if ((Pieces[Match].TypeOfPiece == whitequeen || Pieces[Match].TypeOfPiece == blackqueen) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite)
            {
                return true;
            }

            break;
        }
        checkx+= SquareSize;
    }


    //Check Knight
    int knightXMove[8] = {-1,1,2,2,1,-1,-2,-2};
    int knightYMove[8] = {-2,-2,-1,1,2,2,1,-1};

    for (int i = 0; i < 8; i++)
    {
        int AttackingPiece = FindMatch(Kingx+knightXMove[i]*SquareSize,Kingy+knightYMove[i]*SquareSize,KingColour);
        if ((Pieces[AttackingPiece].TypeOfPiece == whiteknight || Pieces[AttackingPiece].TypeOfPiece == blackknight) && Pieces[AttackingPiece].IsWhite != Pieces[KingColour].IsWhite)
        {
            return true;
        }
    }
    return false;
}

//function to see if white is checkmated
bool WhiteInCheckmate()
{
    //if white's turn, white in check and no legal moves, return true
    if(WhiteTurn && NumberWhiteMoves == 0 && WhiteInCheck)
        return true;
    return false; //return false if checkmate hasn't occured
}

//function to see if black is checkmated
bool BlackInCheckmate()
{
    //if blacks's turn, black in check and no legal moves, return true
    if(!WhiteTurn && NumberBlackMoves == 0 && BlackInCheck)
        return true;
    return false;  //return false if checkmate hasn't occured
}

//this function checks if there is stalemate at a given board posistion
bool stalemate(col Board)
{
    //if white's move, not in check and no legal moves, return true
    if(WhiteTurn && NumberWhiteMoves == 0 && !WhiteInCheck)
    {
        cout << "White can't move" << endl;
        return true;
    }

    //if black's move, not in check and no legal moves, return true
    if(!WhiteTurn && NumberBlackMoves == 0 && !BlackInCheck)
    {
        cout << "Black can't move" << endl;
        return true;
    }

    //variables for counting the number of pieces for stalemate by lack of material
    int numPawns = 0; //pawns
    int numKnightsW = 0; //white knights
    int numKnightsB = 0; //black knights
    int numBishopsW = 0; //white bishops
    int numBishopsB = 0; //black bishops
    int numRooks = 0; //rooks
    int numQueens = 0; //queens

    //pair of loops to look through board and cout pieces to variavles above
    for(int x = 0; x < 8; x++)
    {
        for(int y = 0; y < 8; y++)
        {
            if(Board[x][y]==blackpawn) numPawns++; //pawn
            else if(Board[x][y]==whitepawn) numPawns++; //pawn
            else if(Board[x][y]==blackknight) numKnightsB++; //black knight
            else if(Board[x][y]==whiteknight) numKnightsW++; //white knight
            else if(Board[x][y]==blackbishop) numBishopsB++; //black bishop
            else if(Board[x][y]==whitebishop) numBishopsW++; //white bishop
            else if(Board[x][y]==blackrook) numRooks++; //rook
            else if(Board[x][y]==whiterook) numRooks++; //rook
            else if(Board[x][y]==blackqueen) numQueens++; //queen
            else if(Board[x][y]==whitequeen) numQueens++; //queen
        }
    }
    //checking for stalemate by lack of material
    if(numPawns==0 and numRooks==0 and numQueens==0) //if pawns, rooks or queens, stalemate
    {
        //if 2 knights or less and no bishops, stalemate
        if(numKnightsB<=2 and numKnightsW<=2 and numBishopsB==0 and numBishopsW==0)
        {
            cout << "Lack of material 1" << endl;
            return true;
        }
        //if less than 2 bishops and no knights, stalemate (note: we are not allowing promoting to a bishop)
        if(numBishopsB<2 and numBishopsW<2 and numKnightsW==0 and numKnightsB==0)
        {
            cout << "Lack of material 2" << endl;
            return true;
        }
    }
    return false; //return false
}

bool AiMove()
{
    //stores all of the flags for special first moves
    bool FirstMoveHolder[32];
    for (int i = 0; i < 32; i++) FirstMoveHolder[i] = Pieces[i].FirstMove;

    //finds and stores the best move
    AiMovementTree BlackMove;
    BlackMove = FindBestMove(PredictedMoves,WhiteTurn,1);

    //outputs the moves to the consol
    cout << BlackMove.CurrentBoard << endl;
    updateScreen(BlackMove.CurrentBoard);

    //resets all of the first move flags
    for (int i = 0; i < 32; i++)
    {
        if (i == BlackMove.CurrentPiece) continue;
        Pieces[i].FirstMove = FirstMoveHolder[i];
    }

    //if a piece is taken in the best move, get rid of the piece and set the flags accordingly
    if (BlackMove.TakenPiece >= 0)
    {
        Pieces[BlackMove.TakenPiece].Rect.x = 8*SquareSize + Offset;
        Pieces[BlackMove.TakenPiece].Rect.y = Offset;
        Pieces[BlackMove.CurrentPiece].FirstMove = false;
        Pieces[BlackMove.TakenPiece].IsTaken = true;
    }
    return true;
}

void StartSDL()
{
    //initializes SDL
    SDL_Init( SDL_INIT_EVERYTHING );
    SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" );
    SDL_DisplayMode current;
    SDL_GetCurrentDisplayMode(0,&current);

    //finds the optimal window,renderer,square size and offset
    ScreenHeight = current.h-79;
    ScreenWidth = current.w-16;
    BoardHeight = ScreenHeight-100;
    BoardHeight -= BoardHeight%8;
    SquareSize = BoardHeight/8;

    //creates the window, renderer, image loader and font loader
    window = SDL_CreateWindow( "Chess", 8+ScreenWidth/2, 31, ScreenWidth, ScreenHeight, SDL_WINDOW_SHOWN );
    Renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED );
    SDL_SetRenderDrawColor( Renderer, 0xFF, 0xFF, 0xFF, 0xFF );
    IMG_Init( IMG_INIT_PNG );
    TTF_Init();
    Font = TTF_OpenFont("Numbers.ttf", 24);
    SDL_UpdateWindowSurface( window );
}

void CloseSDL()
{
    //destroys the window, renderer and the image loader
    SDL_DestroyRenderer( Renderer );
    SDL_DestroyWindow( window );
    window = NULL;
    Renderer = NULL;
    IMG_Quit();
    SDL_Quit();
}

void ClosePromotion()
{
    SDL_DestroyRenderer( PromotionRenderer );
    SDL_DestroyWindow( PromotionWindow );
    PromotionWindow = NULL;
    PromotionRenderer = NULL;
}

void LoadMedia()
{
    //loads the board picture and the text
    BoardTexture = LoadTexture("Pictures/Board.gif");
    CoordTexture = LoadText(mousex,mousey);
}

SDL_Texture* LoadTexture( string path )
{
    SDL_Texture* newTexture = NULL;
    SDL_Surface* loadedSurface = IMG_Load(path.c_str());

    if (loadedSurface == NULL)
        cout << "Image Didnt Load" << endl;

    newTexture = SDL_CreateTextureFromSurface(Renderer,loadedSurface);
    if (newTexture == NULL)
        cout << "Image Didnt Load" << endl;

    SDL_FreeSurface(loadedSurface);

    return newTexture;
}

SDL_Texture* LoadText(int X, int Y)
{
    SDL_Texture* newTexture = NULL;

    string x,y,temp;

    stringstream pp;
    pp << X;
    pp >> x;

    pp.clear();

    pp << Y;
    pp >> y;

    temp = "( " + x + " , " + y + " )";

    SDL_Surface* loadedSurface = TTF_RenderText_Solid(Font,temp.c_str(),White);

    newTexture = SDL_CreateTextureFromSurface(Renderer, loadedSurface);

    SDL_FreeSurface(loadedSurface);

    return newTexture;
}
