#include <iostream>
#include <vector>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <sstream>

using namespace std;

typedef enum {blank,whitepawn,whiterook,whiteknight,whitebishop,whiteking,whitequeen,blackpawn,blackrook,blackknight,blackbishop,blackking,blackqueen} TPiece; //defining the types of pieces

struct PieceMove
{
    int x;
    int y;
};

//creating PieceInfo structure
struct PieceInfo
{
    TPiece TypeOfPiece; //Determines the piece
    bool FirstMove; //if the piece has already moved, for speacial moves like pawn
    bool IsWhite; //Determine the colour of the piece
    SDL_Rect Rect; //Coordinates of the piece on the screen
    SDL_Rect OldLocation; //Hold the previous Co-ordinates
    SDL_Texture* Texture; //Picture of the piece
    bool IsTaken; //Tells the program when and what pieces have been removed from the board;
    int xMovement[8];
    int yMovement[8];
    vector <PieceMove> moves;
};

struct BoardArray
{
    TPiece board[8][8];
};

typedef vector <BoardArray> Boards;

//Declaring Global Variables
TPiece board[8][8];
Boards PossibleMoves; //an vector of the different board possitions for each move every turn
PieceInfo Pieces[32]; //Creates an array of structures, one per piece
int ScreenHeight; //the height of the display window, defined during runtime
int ScreenWidth; //the width of the display window, defined during runtime
int BoardHeight; //the Height of the Playing board, defined during runtime
int SquareSize; //The width and height of each individual square on the board, defined during runtime
const int Offset = 50; //The small buffer zone between the top, and left side of the window and the board
int mousex; //Global variables to hold the mouse's x coordinate
int mousey; //Global variables to hold the mouse's y coordinate
bool WhiteTurn = true;

//Functions
void PrintBoard (TPiece Board[8][8]);
void SetPositions (TPiece Board[8][8]); //Initially gives values in the structures on the pieces
void PieceSnapToSquare (int selectedPiece,int mousex,int mousey); //After the piece is dropped it aligns in onto the square below the mouse
int SelectPiece(int mousex,int mousey); //When the user clicks on a piece, it returns the piece's structure for manipulation
bool IsvalidMove(int selectedPiece,bool AiTurn); //Tests if the move the user wants to make is allowed
bool CheckForCheck (int KingColour); //Checks if the move cause either player to be in check
int PieceTake(int selectedpiece); //Moves the taken piece off the board;
void assignValues(TPiece WhatPiece,int Rectx,int Recty,string path,int i,bool isWhite,vector <PieceMove> moves);
void GenerateMoveSet();
void UpdateBoard(TPiece Board[8][8]);

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
    StartSDL();
    SDL_GetMouseState(&mousex,&mousey);
    LoadMedia();
    SetPositions(board);

    SDL_Rect BoardRect;
    BoardRect.x = Offset;
    BoardRect.y = Offset;
    BoardRect.w = BoardHeight;
    BoardRect.h = BoardHeight;

    SDL_Rect TextRect;
    TextRect.x = 700;
    TextRect.y = 50;
    TextRect.w = 100;
    TextRect.h = 32;

    SDL_Rect TextRect2;
    TextRect2.x = 700;
    TextRect2.y = 90;
    TextRect2.w = 100;
    TextRect2.h = 32;

    SDL_Event e;
    SDL_Event g;
    bool quit;
    bool Mousedown = false;
    bool PromotionClick = false;
    int SelectedPiece = -1;

    UpdateBoard(board);
    GenerateMoveSet();

    while (!quit)
    {
        while( SDL_PollEvent( &e ) != 0 )
        {
            SDL_GetMouseState(&mousex,&mousey);

            if( e.type == SDL_QUIT )
                quit = true;
            if (e.type == SDL_MOUSEBUTTONDOWN)
                Mousedown = true;
            if (e.type == SDL_MOUSEBUTTONUP)
            {
                Mousedown = false;
                if (SelectedPiece >= 0)
                {
                    PieceSnapToSquare(SelectedPiece,mousex,mousey);

                    if(!IsvalidMove(SelectedPiece,false))
                        Pieces[SelectedPiece].Rect = Pieces[SelectedPiece].OldLocation;
                    else
                    {
                        bool TurnChange = true;

                        int TakenPiece = PieceTake(SelectedPiece);

                        cout << "Taken Piece: " << Pieces[TakenPiece].TypeOfPiece << ":" << Pieces[TakenPiece].IsTaken << endl;

                        if (CheckForCheck(30))
                        {
                            if (WhiteTurn)
                            {
                                TurnChange = false;
                                Pieces[SelectedPiece].Rect = Pieces[SelectedPiece].OldLocation;
                                cout << TakenPiece << " Taken Piece" << endl;
                                if (TakenPiece >= 0)
                                {
                                    Pieces[TakenPiece].IsTaken = false;
                                    Pieces[TakenPiece].Rect = Pieces[TakenPiece].OldLocation;
                                }
                            }
                            cout << "White is in check" << endl;
                        }
                        if (CheckForCheck(31))
                        {
                            if (!WhiteTurn)
                            {
                                TurnChange = false;
                                Pieces[SelectedPiece].Rect = Pieces[SelectedPiece].OldLocation;
                                if (TakenPiece >= 0)
                                {
                                    Pieces[TakenPiece].IsTaken = false;
                                    Pieces[TakenPiece].Rect = Pieces[TakenPiece].OldLocation;
                                }
                            }
                            cout << "Black is in check" << endl;
                        }

                        cout << "Taken Piece: " << Pieces[TakenPiece].TypeOfPiece << ":" << Pieces[TakenPiece].IsTaken << endl;

                        if (TurnChange)
                        {
                            Pieces[SelectedPiece].FirstMove = false;
                            WhiteTurn = !WhiteTurn;
                            UpdateBoard(board);
                            GenerateMoveSet();
                        }
                    }
                }
                SelectedPiece = -1;
            }
        }

        if (Mousedown && SelectedPiece < 0 )
        {
            //Checking to see if the mouse is on the board
            if (mousex >= Offset && mousex < BoardHeight+Offset && mousey >= Offset && mousey < BoardHeight+Offset)
                SelectedPiece = SelectPiece(mousex,mousey);
        }

        if (SelectedPiece >= 0)
        {
            Pieces[SelectedPiece].Rect.x = mousex-SquareSize/2;
            Pieces[SelectedPiece].Rect.y = mousey-SquareSize/2;
        }

        /*if (!WhiteTurn)
            if (AiMove())
            {
                WhiteTurn = WhiteTurn;
            }*/


        SDL_RenderClear(Renderer);
        SDL_RenderCopy(Renderer,BoardTexture,NULL,&BoardRect);
        CoordTexture = LoadText(Pieces[SelectedPiece].Rect.x,Pieces[SelectedPiece].Rect.y);
        SDL_RenderCopy(Renderer,CoordTexture,NULL,&TextRect2);
        CoordTexture = LoadText(mousex,mousey);
        SDL_RenderCopy(Renderer,CoordTexture,NULL,&TextRect);

        for (int i = 0; i < 32; i++)
            SDL_RenderCopy(Renderer,Pieces[i].Texture,NULL,&Pieces[i].Rect);

        SDL_RenderPresent(Renderer);

        while (!PromotionQuit)
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
        }

    }

    PrintBoard(board);

    CloseSDL();

    return 0;
}

void assignValues (TPiece WhatPiece, int Rectx, int Recty, string path , int i,bool isWhite,vector <PieceMove> moves)
{
    SDL_Rect Rect;
    Rect.x = Rectx;
    Rect.y = Recty;
    Rect.h = SquareSize;
    Rect.w = SquareSize;

    SDL_Texture* Texture;
    Texture = LoadTexture(path);

    Pieces[i].TypeOfPiece = WhatPiece;
    Pieces[i].Texture = Texture;
    Pieces[i].Rect = Rect;
    Pieces[i].OldLocation = Rect;
    Pieces[i].FirstMove = true;
    Pieces[i].IsWhite = isWhite;
    Pieces[i].moves = moves;
    Pieces[i].IsTaken = false;
}

void SetPositions (TPiece Board[8][8])
{
    int xtrack = 0;

    for (int i = 0; i < 8; i++)
    {
        vector <PieceMove> moves;
        PieceMove Whitepawn;
        int xmove[6] = {0,0,-1,1,-1,1};
        int ymove[6] = {-1,-2,-1,-1,-1,-1};

        for (int j = 0; j < 6; j++)
        {
            Whitepawn.x = xmove[j];
            Whitepawn.y = ymove[j];
            moves.push_back(Whitepawn);
        }

        assignValues(whitepawn,xtrack*SquareSize+Offset,6*SquareSize+Offset,"Pictures/WhitePawn.gif",i,true,moves);
        xtrack++;
    }

    xtrack = 0;

    for (int i = 8; i < 16; i++)
    {
        vector <PieceMove> moves;
        PieceMove Blackpawn;
        int xmove[6] = {0,0,-1,1,-1,1};
        int ymove[6] = {1,2,1,1,1,1};

        for (int j = 0; j < 6; j++)
        {
            Blackpawn.x = xmove[j];
            Blackpawn.y = ymove[j];
            moves.push_back(Blackpawn);
        }
        assignValues(blackpawn,xtrack*SquareSize+Offset,SquareSize+Offset,"Pictures/BlackPawn.gif",i,false,moves);
        xtrack ++;
    }

    xtrack = 0;

    for (int i = 16; i < 18; i++)
    {
        vector <PieceMove> moves;
        PieceMove WhiteRook;
        int xmove[6] = {0,1,0,-1};
        int ymove[6] = {-1,0,1,0};

        for (int j = 0; j < 4; j++)
        {
            WhiteRook.x = xmove[j];
            WhiteRook.y = ymove[j];
            moves.push_back(WhiteRook);
        }
        assignValues(whiterook,xtrack*SquareSize+Offset,7*SquareSize+Offset,"Pictures/WhiteRook.gif",i,true,moves);
        xtrack += 7;
    }

    xtrack = 0;

    for (int i = 18; i < 20; i++)
    {
        vector <PieceMove> moves;
        PieceMove BlackRook;
        int xmove[6] = {0,1,0,-1};
        int ymove[6] = {-1,0,1,0};

        for (int j = 0; j < 4; j++)
        {
            BlackRook.x = xmove[j];
            BlackRook.y = ymove[j];
            moves.push_back(BlackRook);
        }
        assignValues(blackrook,xtrack*SquareSize+Offset,Offset,"Pictures/BlackRook.gif",i,false,moves);
        xtrack += 7;
    }

    xtrack = 1;

    for (int i = 20; i < 22; i++)
    {
        vector <PieceMove> moves;
        PieceMove WhiteKnight;
        int xmove[8] = {-1,1,2,2,1,-1,-2,-2};
        int ymove[8] = {-2,-2,-1,1,2,2,1,-1};

        for (int j = 0; j < 8; j++)
        {
            WhiteKnight.x = xmove[j];
            WhiteKnight.y = ymove[j];
            moves.push_back(WhiteKnight);
        }
        assignValues(whiteknight,xtrack*SquareSize+Offset,7*SquareSize+Offset,"Pictures/WhiteKnight.gif",i,true,moves);
        xtrack += 5;
    }

    xtrack = 1;

    for (int i = 22; i < 24; i++)
    {
        vector <PieceMove> moves;
        PieceMove BlackKnight;
        int xmove[8] = {-1,1,2,2,1,-1,-2,-2};
        int ymove[8] = {-2,-2,-1,1,2,2,1,-1};

        for (int j = 0; j < 8; j++)
        {
            BlackKnight.x = xmove[j];
            BlackKnight.y = ymove[j];
            moves.push_back(BlackKnight);
        }
        assignValues(blackknight,xtrack*SquareSize+Offset,Offset,"Pictures/BlackKnight.gif",i,false,moves);
        xtrack += 5;
    }

    xtrack = 2;

    for (int i = 24; i < 26; i++)
    {
        vector <PieceMove> moves;
        PieceMove WhiteBishop;
        int xmove[4] = {-1,1,1,-1};
        int ymove[4] = {-1,-1,1,1};

        for (int j = 0; j < 4; j++)
        {
            WhiteBishop.x = xmove[j];
            WhiteBishop.y = ymove[j];
            moves.push_back(WhiteBishop);
        }
        assignValues (whitebishop,xtrack*SquareSize+Offset,7*SquareSize+Offset,"Pictures/WhiteBishop.gif",i,true,moves);
        xtrack += 3;
    }

    xtrack = 2;

    for (int i = 26; i < 28; i++)
    {
        vector <PieceMove> moves;
        PieceMove BlackBishop;
        int xmove[4] = {-1,1,1,-1};
        int ymove[4] = {-1,-1,1,1};

        for (int j = 0; j < 4; j++)
        {
            BlackBishop.x = xmove[j];
            BlackBishop.y = ymove[j];
            moves.push_back(BlackBishop);
        }
        assignValues(blackbishop,xtrack*SquareSize+Offset,Offset,"Pictures/BlackBishop.gif",i,false,moves);
        xtrack += 3;
    }

    vector <PieceMove> moves;
    PieceMove Royalty;
    int xmove[8] = {-1,0,1,1,1,0,-1,-1};
    int ymove[8] = {-1,-1,-1,0,1,1,1,0};

    for (int j = 0; j < 8; j++)
    {
        Royalty.x = xmove[j];
        Royalty.y = ymove[j];
        moves.push_back(Royalty);
    }
    assignValues(blackking,4*SquareSize+Offset,Offset,"Pictures/BlackKing.gif",31,false,moves);
    assignValues(whiteking,4*SquareSize+Offset,7*SquareSize+Offset,"Pictures/WhiteKing.gif",30,true,moves);
    assignValues(blackqueen,3*SquareSize+Offset,Offset,"Pictures/BlackQueen.gif",29,false,moves);
    assignValues(whitequeen,3*SquareSize+Offset,7*SquareSize+Offset,"Pictures/WhiteQueen.gif",28,true,moves);

    for (int y = 0; y < 8; y++)
        for (int x = 0; x < 8; x++)
            board[x][y] = blank;
}

void PieceSnapToSquare (int selectedPiece,int mousex,int mousey)
{
    for (int i = 0; i <= 7; i++)
    {
        if (mousex >= i*SquareSize+Offset && mousex < (i+1)*SquareSize+Offset)
        {
            Pieces[selectedPiece].Rect.x = i*SquareSize+50;
        }
        if (mousey >= i*SquareSize+Offset && mousey < (i+1)*SquareSize+Offset)
        {
            Pieces[selectedPiece].Rect.y = i*SquareSize+50;
        }
    }
}

int SelectPiece(int mousex,int mousey)
{
    for (int i = 0; i < 32; i++)
    {
        int lowx = Pieces[i].Rect.x;
        int lowy = Pieces[i].Rect.y;
        int Highx = lowx+SquareSize;
        int Highy = lowy+SquareSize;

        if (mousex >= lowx && mousex < Highx && mousey >= lowy && mousey < Highy)
        {
            if (WhiteTurn == Pieces[i].IsWhite)
            {
                cout << Pieces[i].TypeOfPiece << " has been selected " << i << endl;
                Pieces[i].OldLocation = Pieces[i].Rect;
                return i;
            }
        }
    }
    return -1;
}

int FindMatch(int x, int y, int SelectedPiece)
{
    for (int i = 0; i < 32; i++)
    {
        if (i == SelectedPiece) continue;

        if (Pieces[i].Rect.y == y && Pieces[i].Rect.x == x)
        {
            //cout << Pieces[i].TypeOfPiece << " is in the way " << i << endl;
            return i;
        }
    }
    return -1;
}

int PieceTake(int selectedPiece)
{
    int beingTaken = FindMatch(Pieces[selectedPiece].Rect.x, Pieces[selectedPiece].Rect.y, selectedPiece);

    if (beingTaken >= 0)
    {
        Pieces[beingTaken].OldLocation = Pieces[beingTaken].Rect;
        if (Pieces[selectedPiece].IsWhite != Pieces[beingTaken].IsWhite)
        {
            Pieces[beingTaken].Rect.x = 8*SquareSize + Offset;
            Pieces[beingTaken].Rect.y = Offset;
            Pieces[selectedPiece].FirstMove = false;
            Pieces[beingTaken].IsTaken = true;
            return beingTaken;
        }
        else
        {
            Pieces[selectedPiece].Rect.x = Pieces[selectedPiece].OldLocation.x;
            Pieces[selectedPiece].Rect.y = Pieces[selectedPiece].OldLocation.y;
            return -2;
        }
    }

    return -1;
}

void UpdateBoard(TPiece Board[8][8])
{
    int xtrack = 0;
    int ytrack = 0;

    for (int y = 0; y < 8; y++)
        for (int x = 0; x < 8; x++)
            board[x][y] = blank;

    xtrack = 0;
    ytrack = 0;

    for (int i = 0; i < 32; i++)
    {
        if (Pieces[i].IsTaken) continue;
        xtrack = (Pieces[i].Rect.x-50)/SquareSize;
        ytrack = (Pieces[i].Rect.y-50)/SquareSize;
        board[xtrack][ytrack] = Pieces[i].TypeOfPiece;
    }
}

void PrintBoard (TPiece Board[8][8])
{
    cout << endl;
    for (int y = 0; y < 8; y++)
    {
        for (int x = 0; x < 8; x++)
            cout << Board[x][y] << " ";
        cout << endl;
    }
    cout << endl;
}

/*Start of Movements */

//********************Pawn Movement********************
int IsPawnBlocked(int selectedPiece)
{
    if (Pieces[selectedPiece].IsWhite)
    {
        for (int i = 0; i <= 2; i++)
        {
            int Block = FindMatch(Pieces[selectedPiece].OldLocation.x, Pieces[selectedPiece].OldLocation.y-i*SquareSize, selectedPiece);
            if (Block >= 0) return i;
        }
    }
    else if (!Pieces[selectedPiece].IsWhite)
        for (int i = 0; i <= 2; i++)
        {
            int Block = FindMatch(Pieces[selectedPiece].OldLocation.x, Pieces[selectedPiece].OldLocation.y+i*SquareSize, selectedPiece);
            if (Block >= 0) return i;
        }
    return 0;
}

void Promotion (int selectedPiece)
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
}

bool PawnMove(int selectedPiece)
{
    int SomethingInfrontOfPawn = IsPawnBlocked(selectedPiece);

    if (Pieces[selectedPiece].FirstMove)
    {
        if (Pieces[selectedPiece].IsWhite) // White pawn first move
        {
            if (Pieces[selectedPiece].Rect.y == Pieces[selectedPiece].OldLocation.y - 2*SquareSize && Pieces[selectedPiece].Rect.x == Pieces[selectedPiece].OldLocation.x && SomethingInfrontOfPawn == 0)
                return true;
            if (Pieces[selectedPiece].Rect.y == Pieces[selectedPiece].OldLocation.y - SquareSize && Pieces[selectedPiece].Rect.x == Pieces[selectedPiece].OldLocation.x && SomethingInfrontOfPawn != 1)
                return true;
        }
        else //Black pawn first move
        {
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
            if (Pieces[selectedPiece].Rect.y == Pieces[selectedPiece].OldLocation.y - SquareSize && Pieces[selectedPiece].Rect.x == Pieces[selectedPiece].OldLocation.x && SomethingInfrontOfPawn != 1)
            {
                Promotion(selectedPiece);
                return true;
            }

        }
        else
        {
            if (Pieces[selectedPiece].Rect.y == Pieces[selectedPiece].OldLocation.y + SquareSize && Pieces[selectedPiece].Rect.x == Pieces[selectedPiece].OldLocation.x && SomethingInfrontOfPawn != 1)
            {
                Promotion(selectedPiece);
                return true;
            }
        }
    }

    if (Pieces[selectedPiece].IsWhite) //White Pawns Taking Pieces
    {
        int AttackingPiece = FindMatch(Pieces[selectedPiece].Rect.x,Pieces[selectedPiece].Rect.y,selectedPiece);
        if (Pieces[AttackingPiece].IsWhite != Pieces[selectedPiece].IsWhite && AttackingPiece >= 0)
            if ((Pieces[selectedPiece].Rect.y == Pieces[selectedPiece].OldLocation.y - SquareSize && Pieces[selectedPiece].Rect.x == Pieces[selectedPiece].OldLocation.x-SquareSize)||(Pieces[selectedPiece].Rect.y == Pieces[selectedPiece].OldLocation.y - SquareSize && Pieces[selectedPiece].Rect.x == Pieces[selectedPiece].OldLocation.x+SquareSize))
            {
                Promotion(selectedPiece);
                return true;
            }
    }
    else //Black Pawns Taking Pieces
    {
        int AttackingPiece = FindMatch(Pieces[selectedPiece].Rect.x,Pieces[selectedPiece].Rect.y,selectedPiece);
        if (Pieces[AttackingPiece].IsWhite != Pieces[selectedPiece].IsWhite)
            if ((Pieces[selectedPiece].Rect.y == Pieces[selectedPiece].OldLocation.y + SquareSize && Pieces[selectedPiece].Rect.x == Pieces[selectedPiece].OldLocation.x+SquareSize)||(Pieces[selectedPiece].Rect.y == Pieces[selectedPiece].OldLocation.y + SquareSize && Pieces[selectedPiece].Rect.x == Pieces[selectedPiece].OldLocation.x-SquareSize))
            {
                Promotion(selectedPiece);
                return true;
            }
    }

    return false;
}

//*******************Knight Movement*******************

bool IsKnightBlocked(int selectedPiece)
{
    int MovingToSquare = FindMatch(Pieces[selectedPiece].Rect.x, Pieces[selectedPiece].Rect.y, selectedPiece);

    if (MovingToSquare >= 0)
        if (Pieces[selectedPiece].IsWhite == Pieces[MovingToSquare].IsWhite)
            return true;

    return false;
}

bool KnightMove(int selectedPiece)
{
    bool SomethingInKnightMove = IsKnightBlocked(selectedPiece);

    if (SomethingInKnightMove)
    {
        //cout << "Something is in the way" << endl;
        return false;
    }

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

    if (Pieces[selectedPiece].OldLocation.y < Pieces[selectedPiece].Rect.y && Pieces[selectedPiece].OldLocation.x < Pieces[selectedPiece].Rect.x)
    {
        for (int checky = Pieces[selectedPiece].OldLocation.y; checky <= Pieces[selectedPiece].Rect.y; checky += SquareSize)
        {
            if (checky == Pieces[selectedPiece].Rect.y && xcheck == Pieces[selectedPiece].Rect.x) return false;
            if (FindMatch(xcheck,checky,selectedPiece)>= 0) return true;
            xcheck+= SquareSize;
        }
    }
    else if (Pieces[selectedPiece].OldLocation.y < Pieces[selectedPiece].Rect.y && Pieces[selectedPiece].OldLocation.x > Pieces[selectedPiece].Rect.x)
    {
        for (int checky = Pieces[selectedPiece].OldLocation.y; checky <= Pieces[selectedPiece].Rect.y; checky += SquareSize)
        {
            if (checky == Pieces[selectedPiece].Rect.y && xcheck == Pieces[selectedPiece].Rect.x) return false;
            if (FindMatch(xcheck,checky,selectedPiece)>= 0) return true;
            xcheck-= SquareSize;
        }
    }
    else if (Pieces[selectedPiece].OldLocation.y > Pieces[selectedPiece].Rect.y && Pieces[selectedPiece].OldLocation.x < Pieces[selectedPiece].Rect.x)
    {
        for (int checky = Pieces[selectedPiece].OldLocation.y; checky >= Pieces[selectedPiece].Rect.y; checky -= SquareSize)
        {
            if (checky == Pieces[selectedPiece].Rect.y && xcheck == Pieces[selectedPiece].Rect.x) return false;
            if (FindMatch(xcheck,checky,selectedPiece)>= 0) return true;
            xcheck+= SquareSize;
        }
    }
    else if (Pieces[selectedPiece].OldLocation.y > Pieces[selectedPiece].Rect.y && Pieces[selectedPiece].OldLocation.x > Pieces[selectedPiece].Rect.x)
    {
        for (int checky = Pieces[selectedPiece].OldLocation.y; checky >= Pieces[selectedPiece].Rect.y; checky -= SquareSize)
        {
            if (checky == Pieces[selectedPiece].Rect.y && xcheck == Pieces[selectedPiece].Rect.x) return false;
            if (FindMatch(xcheck,checky,selectedPiece) >= 0) return true;
            xcheck-= SquareSize;
        }
    }
    return true;
}

bool BishopMove(int selectedPiece)
{
    if (!IsBishopBlocked(selectedPiece))return true;
    return false;
}

//********************Rook Movement********************

bool IsRookBlocked(int selectedPiece)
{
    if (Pieces[selectedPiece].OldLocation.y < Pieces[selectedPiece].Rect.y && Pieces[selectedPiece].OldLocation.x == Pieces[selectedPiece].Rect.x)
    {
        for (int checky = Pieces[selectedPiece].OldLocation.y; checky <= Pieces[selectedPiece].Rect.y; checky += SquareSize)
        {
            if (checky == Pieces[selectedPiece].Rect.y) return false;
            if (FindMatch(Pieces[selectedPiece].Rect.x,checky,selectedPiece)>= 0) return true;
        }
    }
    else if (Pieces[selectedPiece].OldLocation.y > Pieces[selectedPiece].Rect.y && Pieces[selectedPiece].OldLocation.x == Pieces[selectedPiece].Rect.x)
    {
        for (int checky = Pieces[selectedPiece].OldLocation.y; checky >= Pieces[selectedPiece].Rect.y; checky -= SquareSize)
        {
            if (checky == Pieces[selectedPiece].Rect.y) return false;
            if (FindMatch(Pieces[selectedPiece].Rect.x,checky,selectedPiece)>= 0) return true;
        }
    }
    else if (Pieces[selectedPiece].OldLocation.y == Pieces[selectedPiece].Rect.y && Pieces[selectedPiece].OldLocation.x < Pieces[selectedPiece].Rect.x)
    {
        for (int checkx = Pieces[selectedPiece].OldLocation.x; checkx <= Pieces[selectedPiece].Rect.x; checkx += SquareSize)
        {
            if (checkx == Pieces[selectedPiece].Rect.x) return false;
            if (FindMatch(checkx,Pieces[selectedPiece].Rect.y,selectedPiece)>= 0) return true;
        }
    }
    else if (Pieces[selectedPiece].OldLocation.y == Pieces[selectedPiece].Rect.y && Pieces[selectedPiece].OldLocation.x > Pieces[selectedPiece].Rect.x)
    {
        for (int checkx = Pieces[selectedPiece].OldLocation.x; checkx >= Pieces[selectedPiece].Rect.x; checkx -= SquareSize)
        {
            if (checkx == Pieces[selectedPiece].Rect.x) return false;
            if (FindMatch(checkx,Pieces[selectedPiece].Rect.y,selectedPiece)>= 0) return true;
        }
    }
    return true;
}

bool RookMove(int selectedPiece)
{
    if (!IsRookBlocked(selectedPiece))return true;
    return false;
}

//********************King Movement********************
bool KingMove(int selectedPiece)
{
    if (Pieces[selectedPiece].FirstMove)
    {
        //cout << "First King move" << endl;
        if (Pieces[selectedPiece].IsWhite)
        {
            //cout << "White Piece" << endl;
            if (Pieces[selectedPiece].Rect.x == Pieces[selectedPiece].OldLocation.x + 2*SquareSize && Pieces[selectedPiece].Rect.y == Pieces[selectedPiece].OldLocation.y)
            {
                cout << "CASTLING" << endl;
                //cout << "White Castle right Try" << endl;

                if (Pieces[17].FirstMove)
                {
                    //cout << "right White rook First move" << endl;

                    for (int i = 0; i <= 2; i++)
                    {
                        Pieces[selectedPiece].Rect.x = Pieces[selectedPiece].OldLocation.x + i*SquareSize;
                        if (CheckForCheck(selectedPiece)) return false;
                        if (FindMatch(Pieces[selectedPiece].OldLocation.x+i*SquareSize,Pieces[selectedPiece].OldLocation.y,selectedPiece) != -1)
                        {
                            //cout << "Something is in the way" << endl;
                            return false;
                        }
                    }

                    Pieces[selectedPiece].Rect.x = Pieces[selectedPiece].OldLocation.x + 2*SquareSize;
                    Pieces[17].Rect.x = Pieces[17].Rect.x - 2*SquareSize;
                    return true;
                }
            }
            if (Pieces[selectedPiece].Rect.x == Pieces[selectedPiece].OldLocation.x - 2*SquareSize && Pieces[selectedPiece].Rect.y == Pieces[selectedPiece].OldLocation.y)
            {
                cout << "CASTLING" << endl;
                //cout << "White Castle left Try" << endl;
                if (Pieces[16].FirstMove)
                {
                    //cout << "left White rook First move" << endl;
                    for (int i = 0; i <= 2; i++)
                    {
                        Pieces[selectedPiece].Rect.x = Pieces[selectedPiece].OldLocation.x - i*SquareSize;
                        if (CheckForCheck(selectedPiece)) return false;
                        if (FindMatch(Pieces[selectedPiece].OldLocation.x-i*SquareSize,Pieces[selectedPiece].OldLocation.y,selectedPiece) != -1)
                        {
                            //cout << "Something is in the way" << endl;
                            return false;
                        }
                    }

                    Pieces[selectedPiece].Rect.x = Pieces[selectedPiece].OldLocation.x - 2*SquareSize;
                    Pieces[16].Rect.x = Pieces[16].Rect.x + 3*SquareSize;
                    return true;
                }
            }
        }
        else
        {
            //cout << "Black Piece" << endl;
            if (Pieces[selectedPiece].Rect.x == Pieces[selectedPiece].OldLocation.x + 2*SquareSize && Pieces[selectedPiece].Rect.y == Pieces[selectedPiece].OldLocation.y)
            {
                cout << "CASTLING" << endl;
                //cout << "Black Castle right Try" << endl;
                if (Pieces[19].FirstMove)
                {
                    // cout << "Left Black rook First move" << endl;

                    for (int i = 0; i <= 2; i++)
                    {
                        Pieces[selectedPiece].Rect.x = Pieces[selectedPiece].OldLocation.x + i*SquareSize;
                        if (CheckForCheck(selectedPiece)) return false;
                        if (FindMatch(Pieces[selectedPiece].OldLocation.x+i*SquareSize,Pieces[selectedPiece].OldLocation.y,selectedPiece) != -1)
                        {
                            //cout << "Something is in the way" << endl;
                            return false;
                        }
                    }

                    Pieces[selectedPiece].Rect.x = Pieces[selectedPiece].OldLocation.x + 2*SquareSize;
                    Pieces[19].Rect.x = Pieces[19].Rect.x - 2*SquareSize;
                    return true;
                }
            }
            if (Pieces[selectedPiece].Rect.x == Pieces[selectedPiece].OldLocation.x - 2*SquareSize && Pieces[selectedPiece].Rect.y == Pieces[selectedPiece].OldLocation.y)
            {
                cout << "CASTLING" << endl;
                //cout << "White Castle left Try" << endl;
                if (Pieces[18].FirstMove)
                {
                    //cout << "left White rook First move" << endl;
                    for (int i = 0; i <= 2; i++)
                    {
                        Pieces[selectedPiece].Rect.x = Pieces[selectedPiece].OldLocation.x - i*SquareSize;
                        if (CheckForCheck(selectedPiece)) return false;
                        if (FindMatch(Pieces[selectedPiece].OldLocation.x-i*SquareSize,Pieces[selectedPiece].OldLocation.y,selectedPiece) != -1)
                        {
                            //cout << "Something is in the way" << endl;
                            return false;
                        }
                    }

                    Pieces[selectedPiece].Rect.x = Pieces[selectedPiece].OldLocation.x - 2*SquareSize;
                    Pieces[18].Rect.x = Pieces[18].Rect.x + 3*SquareSize;
                    return true;
                }
            }
        }
    }


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
        for (int checky = Pieces[selectedPiece].OldLocation.y; checky <= Pieces[selectedPiece].Rect.y; checky += SquareSize)
        {
            if (checky == Pieces[selectedPiece].Rect.y && xcheck == Pieces[selectedPiece].Rect.x)return false;
            if (FindMatch(xcheck,checky,selectedPiece)>= 0) return true;
            xcheck+= SquareSize;
        }
    }
    else if (Pieces[selectedPiece].OldLocation.y < Pieces[selectedPiece].Rect.y && Pieces[selectedPiece].OldLocation.x > Pieces[selectedPiece].Rect.x)
    {
        for (int checky = Pieces[selectedPiece].OldLocation.y; checky <= Pieces[selectedPiece].Rect.y; checky += SquareSize)
        {
            if (checky == Pieces[selectedPiece].Rect.y && xcheck == Pieces[selectedPiece].Rect.x)return false;
            if (FindMatch(xcheck,checky,selectedPiece)>= 0) return true;
            xcheck-= SquareSize;
        }
    }
    else if (Pieces[selectedPiece].OldLocation.y > Pieces[selectedPiece].Rect.y && Pieces[selectedPiece].OldLocation.x < Pieces[selectedPiece].Rect.x)
    {
        for (int checky = Pieces[selectedPiece].OldLocation.y; checky >= Pieces[selectedPiece].Rect.y; checky -= SquareSize)
        {
            if (checky == Pieces[selectedPiece].Rect.y && xcheck == Pieces[selectedPiece].Rect.x)return false;
            if (FindMatch(xcheck,checky,selectedPiece)>= 0) return true;
            xcheck+= SquareSize;
        }
    }
    else if (Pieces[selectedPiece].OldLocation.y > Pieces[selectedPiece].Rect.y && Pieces[selectedPiece].OldLocation.x > Pieces[selectedPiece].Rect.x)
    {
        for (int checky = Pieces[selectedPiece].OldLocation.y; checky >= Pieces[selectedPiece].Rect.y; checky -= SquareSize)
        {
            if (checky == Pieces[selectedPiece].Rect.y && xcheck == Pieces[selectedPiece].Rect.x)return false;
            if (FindMatch(xcheck,checky,selectedPiece) >= 0) return true;

            xcheck-= SquareSize;
        }
    }
    else if (Pieces[selectedPiece].OldLocation.y < Pieces[selectedPiece].Rect.y && Pieces[selectedPiece].OldLocation.x == Pieces[selectedPiece].Rect.x)
    {
        for (int checky = Pieces[selectedPiece].OldLocation.y; checky <= Pieces[selectedPiece].Rect.y; checky += SquareSize)
        {
            if (checky == Pieces[selectedPiece].Rect.y)return false;
            if (FindMatch(Pieces[selectedPiece].Rect.x,checky,selectedPiece)>= 0) return true;
        }
    }
    else if (Pieces[selectedPiece].OldLocation.y > Pieces[selectedPiece].Rect.y && Pieces[selectedPiece].OldLocation.x == Pieces[selectedPiece].Rect.x)
    {
        for (int checky = Pieces[selectedPiece].OldLocation.y; checky >= Pieces[selectedPiece].Rect.y; checky -= SquareSize)
        {
            if (checky == Pieces[selectedPiece].Rect.y)return false;
            if (FindMatch(Pieces[selectedPiece].Rect.x,checky,selectedPiece)>= 0) return true;
        }
    }
    else if (Pieces[selectedPiece].OldLocation.y == Pieces[selectedPiece].Rect.y && Pieces[selectedPiece].OldLocation.x < Pieces[selectedPiece].Rect.x)
    {
        for (int checkx = Pieces[selectedPiece].OldLocation.x; checkx <= Pieces[selectedPiece].Rect.x; checkx += SquareSize)
        {
            if (checkx == Pieces[selectedPiece].Rect.x)return false;
            if (FindMatch(checkx,Pieces[selectedPiece].Rect.y,selectedPiece)>= 0) return true;
        }
    }
    else if (Pieces[selectedPiece].OldLocation.y == Pieces[selectedPiece].Rect.y && Pieces[selectedPiece].OldLocation.x > Pieces[selectedPiece].Rect.x)
    {
        for (int checkx = Pieces[selectedPiece].OldLocation.x; checkx >= Pieces[selectedPiece].Rect.x; checkx -= SquareSize)
        {
            if (checkx == Pieces[selectedPiece].Rect.x)return false;
            if (FindMatch(checkx,Pieces[selectedPiece].Rect.y,selectedPiece)>= 0) return true;
        }
    }
    return true;
}

bool QueenMove(int selectedPiece)
{
    if (!IsQueenBlocked(selectedPiece))return true;
    return false;
}

/*End of Movements*/

void GenerateMoveSet()
{
    BoardArray newboard;
    TPiece OldBoard[8][8];

    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            newboard.board[i][j] = board[i][j];
            OldBoard[i][j] = board[i][j];
        }
    }

    cout << "Origional Board" << endl;
    PrintBoard(OldBoard);
    PossibleMoves.push_back(newboard);

    int PieceNumbers[16] = {};

    if (WhiteTurn)
    {
        int Temp[16] = {0,1,2,3,4,5,6,7,16,17,20,21,24,25,28,30};
        for (int i = 0; i < 16; i++)
        {
            PieceNumbers[i] = Temp[i];
        }
    }

    else
    {
        int Temp[16] = {8,9,10,11,12,13,14,15,18,19,22,23,26,27,29,31};
        for (int i = 0; i < 16; i++)
        {
            PieceNumbers[i] = Temp[i];
        }
    }


    for (int i = 0; i < 16; i++)
    {
        int currentPiece = PieceNumbers[i];
        //cout << "Current Piece: " << Pieces[currentPiece].TypeOfPiece << endl;

        if (Pieces[currentPiece].IsTaken == true)
        {
            cout << "Taken Piece: " << Pieces[currentPiece].TypeOfPiece << ":" << Pieces[currentPiece].IsTaken << endl;
            continue;
        }

        Pieces[currentPiece].OldLocation = Pieces[currentPiece].Rect;

        for (int j = 0; j < 8; j++)
        {
            for (int k = 0; k < 8; k++)
            {
                Pieces[currentPiece].Rect.x = j*SquareSize + Offset;
                Pieces[currentPiece].Rect.y = k*SquareSize + Offset;

                SDL_RenderClear(Renderer);
                for (int i = 0; i < 32; i++)
                    SDL_RenderCopy(Renderer,Pieces[i].Texture,NULL,&Pieces[i].Rect);
                SDL_RenderPresent(Renderer);

                SDL_Delay(10);

                if (Pieces[currentPiece].OldLocation.x == Pieces[currentPiece].Rect.x && Pieces[currentPiece].OldLocation.y == Pieces[currentPiece].Rect.y ) continue;

                if (IsvalidMove(currentPiece,true))
                {
                    bool TurnChange = true;
                    int TakenPiece = PieceTake(currentPiece);

                    if (CheckForCheck(30))
                    {
                        if (WhiteTurn)
                        {
                            TurnChange = false;
                            Pieces[currentPiece].Rect = Pieces[currentPiece].OldLocation;
                            if (TakenPiece >= 0)
                            {
                                Pieces[TakenPiece].IsTaken = false;
                                Pieces[TakenPiece].Rect = Pieces[TakenPiece].OldLocation;
                            }
                        }
                    }

                    if (CheckForCheck(31))
                    {
                        if (!WhiteTurn)
                        {
                            TurnChange = false;
                            Pieces[currentPiece].Rect = Pieces[currentPiece].OldLocation;
                            if (TakenPiece >= 0)
                            {
                                Pieces[TakenPiece].IsTaken = false;
                                Pieces[TakenPiece].Rect = Pieces[TakenPiece].OldLocation;
                            }
                        }
                    }

                    if (TurnChange && TakenPiece != -2)
                    {
                        if (TakenPiece >= 0)
                        {
                            Pieces[TakenPiece].IsTaken = false;
                            Pieces[TakenPiece].Rect = Pieces[TakenPiece].OldLocation;
                        }


                        UpdateBoard(board);
                        BoardArray Board;

                        for (int l = 0; l < 8; l++)
                        {
                            for (int m = 0; m < 8; m++)
                            {
                                Board.board[l][m] = board[l][m];
                                board[l][m] = OldBoard[l][m];
                            }
                        }
                        //PrintBoard(Board.board);
                        PossibleMoves.push_back(Board);
                        cout << "Moves size: " << PossibleMoves.size() << endl;
                    }
                }
                Pieces[currentPiece].Rect = Pieces[currentPiece].OldLocation;
            }
        }
    }
//    int x = PossibleMoves.size();
//    for (int i = 0; i < x; i++)
//    {
//        BoardArray Print;
//        Print = PossibleMoves[i];
//        PrintBoard(Print.board);
//    }
    cout << PossibleMoves.size() << endl;
    PossibleMoves.clear();
}

bool IsvalidMove(int selectedPiece,bool AiTurn)
{
    if (!AiTurn)
    {
        if (mousex >= BoardHeight+50 || mousex < 50 || mousey >= BoardHeight+50 || mousey < 50) return false;
    }

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
        if (KingMove(selectedPiece)) return true;
    }

    return false;
}

bool CheckForCheck(int KingColour)
{
    int Kingx = Pieces[KingColour].Rect.x;
    int Kingy = Pieces[KingColour].Rect.y;

    //Check Up
    for (int checky = Kingy; checky >= Offset; checky -= SquareSize)
    {
        int Match = FindMatch(Kingx,checky,KingColour);
        if (Match >= 0)
        {
            if ((Pieces[Match].TypeOfPiece == whiterook || Pieces[Match].TypeOfPiece == blackrook) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite)
            {
                cout << "Rook Check up" << endl;
                cout << "( " << Kingx << " , " << checky << " )" << endl;
                return true;
            }
            if ((Pieces[Match].TypeOfPiece == whitequeen || Pieces[Match].TypeOfPiece == blackqueen) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite)
            {
                cout << "Queen Check up" << endl;
                cout << "( " << Kingx << " , " << checky << " )" << endl;
                return true;
            }
            break;
        }
    }

    //Check Down
    for (int checky = Kingy; checky <= Offset+8*SquareSize; checky += SquareSize)
    {
        int Match = FindMatch(Kingx,checky,KingColour);
        if (Match >= 0)
        {
            if ((Pieces[Match].TypeOfPiece == whiterook || Pieces[Match].TypeOfPiece == blackrook) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite)
            {
                cout << "Rook Check Down" << endl;
                cout << "( " << Kingx << " , " << checky << " )" << endl;
                return true;
            }
            if ((Pieces[Match].TypeOfPiece == whitequeen || Pieces[Match].TypeOfPiece == blackqueen) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite)
            {
                cout << "Queen Check Down" << endl;
                cout << "( " << Kingx << " , " << checky << " )" << endl;
                return true;
            }
            break;
        }
    }

    //Check Left
    for (int checkx = Kingx; checkx >= Offset; checkx -= SquareSize)
    {
        int Match = FindMatch(checkx,Kingy,KingColour);
        if (Match >= 0)
        {
            if ((Pieces[Match].TypeOfPiece == whiterook || Pieces[Match].TypeOfPiece == blackrook) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite)
            {
                cout << "Rook Check Left" << endl;
                cout << "( " << checkx << " , " << Kingy << " )" << endl;
                return true;
            }
            if ((Pieces[Match].TypeOfPiece == whitequeen || Pieces[Match].TypeOfPiece == blackqueen) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite)
            {
                cout << "Queen Check Left" << endl;
                cout << "( " << checkx << " , " << Kingy << " )" << endl;
                return true;
            }
            break;
        }
    }

    //Check Right
    for (int checkx = Kingx; checkx <= Offset+8*SquareSize; checkx += SquareSize)
    {
        int Match = FindMatch(checkx,Kingy,KingColour);
        if (Match >= 0)
        {
            if ((Pieces[Match].TypeOfPiece == whiterook || Pieces[Match].TypeOfPiece == blackrook) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite)
            {
                cout << "Rook Check Right" << endl;
                cout << "( " << checkx << " , " << Kingy << " )" << endl;
                return true;
            }
            if ((Pieces[Match].TypeOfPiece == whitequeen || Pieces[Match].TypeOfPiece == blackqueen) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite)
            {
                cout << "Queen Check Right" << endl;
                cout << "( " << checkx << " , " << Kingy << " )" << endl;
                return true;
            }
            break;
        }
    }

    int checkx = Kingx;

    //Check Up Left
    for (int checky = Kingy; checky >= Offset; checky -= SquareSize)
    {
        int Match = FindMatch(checkx,checky,KingColour);
        if (Match >= 0)
        {
            if ((Pieces[Match].TypeOfPiece == whitepawn || Pieces[Match].TypeOfPiece == blackpawn) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite && checkx == Kingx-SquareSize && checky == Kingy-SquareSize && Pieces[Match].IsWhite == false)
            {
                cout << "Pawn Check up Left" << endl;
                return true;
            }

            if ((Pieces[Match].TypeOfPiece == whitebishop || Pieces[Match].TypeOfPiece == blackbishop) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite)
            {
                cout << "bishop Check up Left" << endl;
                return true;
            }
            if ((Pieces[Match].TypeOfPiece == whitequeen || Pieces[Match].TypeOfPiece == blackqueen) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite)
            {
                cout << "queen Check up Left" << endl;
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
        int Match = FindMatch(checkx,checky,KingColour);
        if (Match >= 0)
        {
            if ((Pieces[Match].TypeOfPiece == whitepawn || Pieces[Match].TypeOfPiece == blackpawn) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite && checkx == Kingx+SquareSize && checky == Kingy-SquareSize && Pieces[Match].IsWhite == false)
            {
                cout << "Pawn Check up Right" << endl;
                return true;
            }

            if ((Pieces[Match].TypeOfPiece == whitebishop || Pieces[Match].TypeOfPiece == blackbishop) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite)
            {
                cout << "bishop Check up Right" << endl;
                return true;
            }
            if ((Pieces[Match].TypeOfPiece == whitequeen || Pieces[Match].TypeOfPiece == blackqueen) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite)
            {
                cout << "queen Check up Right" << endl;
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
        int Match = FindMatch(checkx,checky,KingColour);
        if (Match >= 0)
        {
            if ((Pieces[Match].TypeOfPiece == whitepawn || Pieces[Match].TypeOfPiece == blackpawn) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite && checkx == Kingx-SquareSize && checky == Kingy+SquareSize && Pieces[Match].IsWhite == true)
            {
                cout << "Pawn Check Down Left" << endl;
                return true;
            }

            if ((Pieces[Match].TypeOfPiece == whitebishop || Pieces[Match].TypeOfPiece == blackbishop) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite)
            {
                cout << "bishop Check Down Left" << endl;
                return true;
            }
            if ((Pieces[Match].TypeOfPiece == whitequeen || Pieces[Match].TypeOfPiece == blackqueen) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite)
            {
                cout << "queen Check Down Left" << endl;
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
        int Match = FindMatch(checkx,checky,KingColour);
        if (Match >= 0)
        {
            if ((Pieces[Match].TypeOfPiece == whitepawn || Pieces[Match].TypeOfPiece == blackpawn) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite && checkx == Kingx+SquareSize && checky == Kingy+SquareSize && Pieces[Match].IsWhite == true)
            {
                cout << "Pawn Check Down Right" << endl;
                return true;
            }

            if ((Pieces[Match].TypeOfPiece == whitebishop || Pieces[Match].TypeOfPiece == blackbishop) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite)
            {
                cout << "bishop Check Down Right" << endl;
                return true;
            }
            if ((Pieces[Match].TypeOfPiece == whitequeen || Pieces[Match].TypeOfPiece == blackqueen) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite)
            {
                cout << "queen Check Down Right" << endl;
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
            cout << "Knight check ( " << knightXMove[i] << " , " << knightYMove[i] << " )" << endl;
            return true;
        }
    }
    return false;
}

void StartSDL()
{
    SDL_Init( SDL_INIT_EVERYTHING );

    SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" );

    SDL_DisplayMode current;

    SDL_GetCurrentDisplayMode(0,&current);

    ScreenHeight = current.h-79;
    ScreenWidth = current.w-16;
    BoardHeight = ScreenHeight-100;
    BoardHeight -= BoardHeight%8;
    SquareSize = BoardHeight/8;

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
