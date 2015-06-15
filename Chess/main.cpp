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

struct AiMovementTree
{
    int value;
    col CurrentBoard;
    vector <AiMovementTree> FutureMoves;
    int CurrentPiece;
    int TakenPiece;
};

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
col board;
PieceInfo Pieces[32]; //Creates an array of structures, one per piece
int ScreenHeight; //the height of the display window, defined during runtime
int ScreenWidth; //the width of the display window, defined during runtime
int BoardHeight; //the Height of the Playing board, defined during runtime
int SquareSize; //The width and height of each individual square on the board, defined during runtime
const int Offset = 50; //The small buffer zone between the top, and left side of the window and the board
const int best = 1500;
int mousex; //Global variables to hold the mouse's x coordinate
int mousey; //Global variables to hold the mouse's y coordinate
bool WhiteTurn = true;
bool WhiteInCheck = false;
bool BlackInCheck = false;
int NumberWhiteMoves = 20; //set to 20 because thats the initial moves garentied
int NumberBlackMoves = 20; //set to 20 because thats the initial moves garentied
AiMovementTree PredictedMoves;

//Functions
void PrintBoard (TPiece Board[8][8]);
void SetPositions (); //Initially gives values in the structures on the pieces
void PieceSnapToSquare (int selectedPiece,int mousex,int mousey); //After the piece is dropped it aligns in onto the square below the mouse
int SelectPiece(int mousex,int mousey); //When the user clicks on a piece, it returns the piece's structure for manipulation
bool IsvalidMove(int selectedPiece,bool AiTurn); //Tests if the move the user wants to make is allowed
bool CheckForCheck (int KingColour); //Checks if the move cause either player to be in check
int PieceTake(int selectedpiece); //Moves the taken piece off the board;
void assignValues(TPiece WhatPiece,int Rectx,int Recty,string path,int i,bool isWhite,vector <PieceMove> moves);
vector <AiMovementTree> GenerateMoveSet(col Board, bool WhiteMoves,int *NumberOfMoves);
col UpdateBoard(col Board);
void updateScreen(col Board);
bool WhiteInCheckmate();
bool BlackInCheckmate();
int evaluate(col Board, bool AiTurn,int Taken);
bool stalemate(col Board);
bool AiMove();
void Render();

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
    SetPositions();

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

    PredictedMoves.CurrentBoard = board;

    PredictedMoves.value = 0;
    PredictedMoves.CurrentPiece = -1;
    PredictedMoves.TakenPiece = -1;

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
                        int TakenPiece = PieceTake(SelectedPiece);
                        bool TurnChange = true;
                        BlackInCheck = false;
                        WhiteInCheck = false;

                        if (CheckForCheck(30))
                        {
                            WhiteInCheck = true;
                            if (WhiteTurn)
                            {
                                TurnChange = false;
                                Pieces[SelectedPiece].Rect = Pieces[SelectedPiece].OldLocation;
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
                            BlackInCheck = true;
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

                        if (TurnChange)
                        {
                            Pieces[SelectedPiece].FirstMove = false;
                            WhiteTurn = !WhiteTurn;
                            col updatedBoard = UpdateBoard(board);
                            PredictedMoves.CurrentBoard = updatedBoard;
                            PredictedMoves.value = evaluate(PredictedMoves.CurrentBoard,false,TakenPiece);
                            PredictedMoves.CurrentPiece = SelectedPiece;
                            PredictedMoves.TakenPiece = TakenPiece;
                        }

                        if (WhiteInCheckmate())
                        {
                            cout << "Black Wins" << endl;
                            return 0;
                        }

                        if (BlackInCheckmate())
                        {
                            cout << "White Wins" << endl;
                            return 0;
                        }

                        if (stalemate(PredictedMoves.CurrentBoard))
                        {
                            cout << "StaleMate" << endl;
                            return 0;
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

        SDL_RenderClear(Renderer);
        SDL_RenderCopy(Renderer,BoardTexture,NULL,&BoardRect);
        CoordTexture = LoadText(Pieces[SelectedPiece].Rect.x,Pieces[SelectedPiece].Rect.y);
        SDL_RenderCopy(Renderer,CoordTexture,NULL,&TextRect2);
        CoordTexture = LoadText(mousex,mousey);
        SDL_RenderCopy(Renderer,CoordTexture,NULL,&TextRect);

        for (int i = 0; i < 32; i++)
            SDL_RenderCopy(Renderer,Pieces[i].Texture,NULL,&Pieces[i].Rect);

        SDL_RenderPresent(Renderer);

        if (!WhiteTurn)
            if (AiMove())
            {
                WhiteTurn = !WhiteTurn;
            }

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

void SetPositions()
{
    int xtrack = 0;
    vector <PieceMove> moves;

    for (int i = 0; i < 8; i++)
    {
        moves.clear();
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
        moves.clear();
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
        moves.clear();
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
        moves.clear();
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
        moves.clear();
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
        moves.clear();
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
        moves.clear();
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
        moves.clear();
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

    moves.clear();
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
    {
        row temp;
        for (int x = 0; x < 8; x++)
        {
            temp.push_back(blank);
        }
        board.push_back(temp);
    }

}

void Render()
{
    SDL_RenderClear(Renderer);

    for (int i = 0; i < 32; i++)
        SDL_RenderCopy(Renderer,Pieces[i].Texture,NULL,&Pieces[i].Rect);

    SDL_RenderPresent(Renderer);
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
            Pieces[beingTaken].Rect.x = 900;
            Pieces[beingTaken].Rect.y = 50;
            Pieces[selectedPiece].FirstMove = false;
            Pieces[beingTaken].IsTaken = true;
            Pieces[beingTaken].FirstMove = false;
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

col UpdateBoard(col Board)
{
    Board.clear();

    for (int x = 0; x < 8; x++)
    {
        row temp;
        for (int y = 0; y < 8; y++)
        {
            int i = FindMatch(x*SquareSize+Offset,y*SquareSize+Offset,-1);
            if (Pieces[i].IsTaken)continue;
            temp.push_back(Pieces[i].TypeOfPiece);
        }
        Board.push_back(temp);
    }
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
                if (Pieces[i].IsTaken)continue;
                bool skip = false;
                int num = MovedAlready.size();
                for (int j = 0; j < num; j++)
                    if (MovedAlready[j] == i) skip = true;
                if (!skip)
                {
                    if (Pieces[i].TypeOfPiece == Board[x][y])
                    {
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
bool KingMove(int selectedPiece, bool AiTurn)
{
    if (Pieces[selectedPiece].FirstMove)
    {
        //cout << "First King move" << endl;
        if (Pieces[selectedPiece].IsWhite)
        {
            //cout << "White Piece" << endl;
            if (Pieces[selectedPiece].Rect.x == Pieces[selectedPiece].OldLocation.x + 2*SquareSize && Pieces[selectedPiece].Rect.y == Pieces[selectedPiece].OldLocation.y)
            {
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

int badPawnsBlack(TPiece someBoard[8][8])
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
}

AiMovementTree FindBestMove (AiMovementTree node, bool aiTurn,int depth)
{
    int NumberOfMoves;
    vector <AiMovementTree> BoardMovements;

    if(depth <= 2) //if the max depth hasn't been reached, then add moves to tree
    {
        //calls the funtcion that create variable possibleMoves
        BoardMovements = GenerateMoveSet(node.CurrentBoard,aiTurn,&NumberOfMoves);
        for (int i = 0; i < NumberOfMoves; i++)
        {
            AiMovementTree CurrentBoard;
            CurrentBoard = BoardMovements[i];
            AiMovementTree newMove;
            newMove = CurrentBoard;
            node.FutureMoves.push_back(newMove);
        }
    }
    depth++; //increase the depth

    if(node.FutureMoves.size()!=0) //if the node has children
    {
        //cout << "The node has children" << endl;
        int num = node.FutureMoves.size();
        for(int x = 0; x < num; x++)
        {
            AiMovementTree Recursive;
            Recursive = node.FutureMoves[x];
            node.FutureMoves[x] = FindBestMove(Recursive,!aiTurn,depth); //recursively searching the tree
        }

    }
    else //if the node has no children, find a value for its board
    {
        node.value = evaluate(node.CurrentBoard,aiTurn,-1);
        return node; //return node, since no children
    }

    if(aiTurn) //if it's the ai's turn
    {
        node.value=best; //set best value to the worst possible
        //look through all the moves associated the the node
        for(int x = 0; x < (int)node.FutureMoves.size(); x++)
        {
            //keep the highest value
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
        node.value=-best; //set value to best
        for(int x = 0; x < (int)node.FutureMoves.size(); x++)
        {
            //keep the lowest possible value
            if(node.FutureMoves[x].value > node.value)
            {
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

    GenerateMoveSet(Board,false,&NumberBMoves);
    GenerateMoveSet(Board,true,&NumberWMoves);

    NumberBlackMoves = NumberBMoves;
    NumberWhiteMoves = NumberWMoves;

    evaluation += (NumberBMoves - NumberWMoves)/4;

    //subtract 5 points for the AI's bad pawns, add 5 points for the opponent's bad pawns
    //evaluation -= (badPawnsBlack(Board) - badPawnsWhite(Board))*5;
    return evaluation; //return evaluation of board
}

vector <AiMovementTree> GenerateMoveSet(col Board, bool WhiteMoves,int *NumberOfMoves)
{
    vector <AiMovementTree> PossibleMoves;
    col newboard;
    col OldBoard;

    OldBoard = Board;

    updateScreen(Board);

    for (int i = 0; i < 32; i++)
    {
        if (Pieces[i].IsTaken) continue;
        Pieces[i].OldLocation = Pieces[i].Rect;
    }

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

    for (int i = 0; i < 16; i++)
    {
        int MovingPiece = PieceNumbers[i];
        if (Pieces[MovingPiece].IsTaken) continue;

        for (int j = 0; j < 8; j++)
        {
            for (int k = 0; k < 8; k++)
            {
                Pieces[MovingPiece].Rect.x = k*SquareSize + Offset;
                Pieces[MovingPiece].Rect.y = j*SquareSize + Offset;

                if (Pieces[MovingPiece].OldLocation.x == Pieces[MovingPiece].Rect.x && Pieces[MovingPiece].OldLocation.y == Pieces[MovingPiece].Rect.y ) continue;

                if (IsvalidMove(MovingPiece,true))
                {
                    bool TurnChange = true;
                    int ToTake = PieceTake(MovingPiece);

                    if (CheckForCheck(30))
                    {
                        if (WhiteTurn)
                        {
                            TurnChange = false;
                            Pieces[MovingPiece].Rect = Pieces[MovingPiece].OldLocation;
                            if (ToTake >= 0)
                            {
                                Pieces[ToTake].IsTaken = false;
                                Pieces[ToTake].Rect = Pieces[ToTake].OldLocation;
                            }
                        }
                    }

                    if (CheckForCheck(31))
                    {
                        if (!WhiteTurn)
                        {
                            TurnChange = false;
                            Pieces[MovingPiece].Rect = Pieces[MovingPiece].OldLocation;
                            if (ToTake >= 0)
                            {
                                Pieces[ToTake].IsTaken = false;
                                Pieces[ToTake].Rect = Pieces[ToTake].OldLocation;
                            }
                        }
                    }

                    if (TurnChange && ToTake != -2)
                    {
                        if (ToTake >= 0)
                        {
                            Pieces[ToTake].IsTaken = false;
                            Pieces[ToTake].Rect = Pieces[ToTake].OldLocation;
                        }

                        col UpdatedBoard = UpdateBoard(Board);

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
                        AiMovementTree BoardHolder;
                        BoardHolder.CurrentBoard = newboard;
                        BoardHolder.CurrentPiece = MovingPiece;
                        BoardHolder.TakenPiece = ToTake;
                        PossibleMoves.push_back(BoardHolder);
                        TurnChange = false;
                        newboard.clear();
                    }
                }
                Pieces[MovingPiece].Rect = Pieces[MovingPiece].OldLocation;
            }
        }
    }

    for (int i = 0; i < 32; i++)
    {
        if (Pieces[i].IsTaken) continue;
        Pieces[i].Rect = Pieces[i].OldLocation;
    }


    *NumberOfMoves = PossibleMoves.size();
    updateScreen(Board);
    return PossibleMoves;
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
        if (KingMove(selectedPiece,AiTurn)) return true;
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
//                if (Pieces[Match].IsWhite) cout << "White ";
//                else cout << "Black ";
//                cout << "Rook Check up" << endl;
//                cout << "( " << Kingx << " , " << checky << " )" << endl;
                return true;
            }
            if ((Pieces[Match].TypeOfPiece == whitequeen || Pieces[Match].TypeOfPiece == blackqueen) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite)
            {
//                if (Pieces[Match].IsWhite) cout << "White ";
//                else cout << "Black ";
//                cout << "Queen Check up" << endl;
//                cout << "( " << Kingx << " , " << checky << " )" << endl;
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
//                if (Pieces[Match].IsWhite) cout << "White ";
//                else cout << "Black ";
//                cout << "Rook Check Down" << endl;
//                cout << "( " << Kingx << " , " << checky << " )" << endl;
                return true;
            }
            if ((Pieces[Match].TypeOfPiece == whitequeen || Pieces[Match].TypeOfPiece == blackqueen) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite)
            {
//                if (Pieces[Match].IsWhite) cout << "White ";
//                else cout << "Black ";
//                cout << "Queen Check Down" << endl;
//                cout << "( " << Kingx << " , " << checky << " )" << endl;
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
//                if (Pieces[Match].IsWhite) cout << "White ";
//                else cout << "Black ";
//                cout << "Rook Check Left" << endl;
//                cout << "( " << checkx << " , " << Kingy << " )" << endl;
                return true;
            }
            if ((Pieces[Match].TypeOfPiece == whitequeen || Pieces[Match].TypeOfPiece == blackqueen) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite)
            {
//                if (Pieces[Match].IsWhite) cout << "White ";
//                else cout << "Black ";
//                cout << "Queen Check Left" << endl;
//                cout << "( " << checkx << " , " << Kingy << " )" << endl;
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
//                if (Pieces[Match].IsWhite) cout << "White ";
//                else cout << "Black ";
//                cout << "Rook Check Right" << endl;
//                cout << "( " << checkx << " , " << Kingy << " )" << endl;
                return true;
            }
            if ((Pieces[Match].TypeOfPiece == whitequeen || Pieces[Match].TypeOfPiece == blackqueen) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite)
            {
//                if (Pieces[Match].IsWhite) cout << "White ";
//                else cout << "Black ";
//                cout << "Queen Check Right" << endl;
//                cout << "( " << checkx << " , " << Kingy << " )" << endl;
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
//                if (Pieces[Match].IsWhite) cout << "White ";
//                else cout << "Black ";
//                cout << "Pawn Check up Left" << endl;
                return true;
            }

            if ((Pieces[Match].TypeOfPiece == whitebishop || Pieces[Match].TypeOfPiece == blackbishop) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite)
            {
//                if (Pieces[Match].IsWhite) cout << "White ";
//                else cout << "Black ";
//                cout << "bishop Check up Left" << endl;
                return true;
            }
            if ((Pieces[Match].TypeOfPiece == whitequeen || Pieces[Match].TypeOfPiece == blackqueen) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite)
            {
//                if (Pieces[Match].IsWhite) cout << "White ";
//                else cout << "Black ";
//                cout << "queen Check up Left" << endl;
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
//                if (Pieces[Match].IsWhite) cout << "White ";
//                else cout << "Black ";
//                cout << "Pawn Check up Right" << endl;
                return true;
            }

            if ((Pieces[Match].TypeOfPiece == whitebishop || Pieces[Match].TypeOfPiece == blackbishop) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite)
            {
//                if (Pieces[Match].IsWhite) cout << "White ";
//                else cout << "Black ";
//                cout << "bishop Check up Right" << endl;
                return true;
            }
            if ((Pieces[Match].TypeOfPiece == whitequeen || Pieces[Match].TypeOfPiece == blackqueen) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite)
            {
//                if (Pieces[Match].IsWhite) cout << "White ";
//                else cout << "Black ";
//                cout << "queen Check up Right" << endl;
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
//                if (Pieces[Match].IsWhite) cout << "White ";
//                else cout << "Black ";
//                cout << "Pawn Check Down Left" << endl;
                return true;
            }

            if ((Pieces[Match].TypeOfPiece == whitebishop || Pieces[Match].TypeOfPiece == blackbishop) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite)
            {
//                if (Pieces[Match].IsWhite) cout << "White ";
//                else cout << "Black ";
//                cout << "bishop Check Down Left" << endl;
                return true;
            }
            if ((Pieces[Match].TypeOfPiece == whitequeen || Pieces[Match].TypeOfPiece == blackqueen) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite)
            {
//                if (Pieces[Match].IsWhite) cout << "White ";
//                else cout << "Black ";
//                cout << "queen Check Down Left" << endl;
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
//                if (Pieces[Match].IsWhite) cout << "White ";
//                else cout << "Black ";
//                cout << "Pawn Check Down Right" << endl;
                return true;
            }

            if ((Pieces[Match].TypeOfPiece == whitebishop || Pieces[Match].TypeOfPiece == blackbishop) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite)
            {
//                if (Pieces[Match].IsWhite) cout << "White ";
//                else cout << "Black ";
//                cout << "bishop Check Down Right" << endl;
                return true;
            }
            if ((Pieces[Match].TypeOfPiece == whitequeen || Pieces[Match].TypeOfPiece == blackqueen) && Pieces[Match].IsWhite != Pieces[KingColour].IsWhite)
            {
//                if (Pieces[Match].IsWhite) cout << "White ";
//                else cout << "Black ";
//                cout << "queen Check Down Right" << endl;
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
//            if (Pieces[AttackingPiece].IsWhite) cout << "White ";
//            else cout << "Black ";
//            cout << "Knight check ( " << knightXMove[i] << " , " << knightYMove[i] << " )" << endl;
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
    //if(posX3()) return true; //return true if a posistion has occured 3 time
    //if(stalemate50 == 50) return true; //return true if 50 move rule has been reached
    return false; //return false
}

bool AiMove()
{
    bool FirstMoveHolder[32];

    for (int i = 0; i < 32; i++) FirstMoveHolder[i] = Pieces[i].FirstMove;

    AiMovementTree BlackMove;
    BlackMove = FindBestMove(PredictedMoves,WhiteTurn,1);

    cout << BlackMove.CurrentBoard << endl;
    updateScreen(BlackMove.CurrentBoard);

    for (int i = 0; i < 32; i++)
    {
        if (i == BlackMove.CurrentPiece) continue;
        Pieces[i].FirstMove = FirstMoveHolder[i];
    }

    cout << BlackMove.TakenPiece << endl;
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
