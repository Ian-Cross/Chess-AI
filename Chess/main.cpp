#include <iostream>
#include <vector>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <sstream>

using namespace std;

typedef enum {blank1,pawn11,rook11,knight,bishop,king11,queen1} TPiece; //defining the types of pieces

//creating PieceInfo structure
struct PieceInfo
{
    TPiece TypeOfPiece; //Determines the piece
    bool FirstMove; //if the piece has already moved, for speacial moves like pawn
    bool IsWhite; //Determine the colour of the piece
    SDL_Rect Rect; //Coordinates of the piece on the screen
    SDL_Rect OldLocation; //Hold the previous Co-ordinates
    SDL_Texture* Texture; //Picture of the piece
};

//Declaring Global Variables
TPiece board[8][8];
PieceInfo Pieces[32]; //Creates and area of structures, one per piece
int ScreenHeight; //the height of the display window, defined during runtime
int ScreenWidth; //the width of the display window, defined during runtime
int BoardHeight; //the Height of the Playing board, defined during runtime
int SquareSize; //The width and height of each individual square on the board, defined during runtime
const int Offset = 50; //The small buffer zone between the top, and left side of the window and the board
int mousex; //Global variables to hold the mouse's x coordinate
int mousey; //Global variables to hold the mouse's y coordinate

//Functions
void PrintBoard (TPiece Board[8][8]);
void SetPositions (TPiece Board[8][8]); //Initially gives values in the structures on the pieces
void PieceSnapToSquare (int selectedPiece,int mousex,int mousey); //After the piece is dropped it aligns in onto the square below the mouse
int SelectPiece(int mousex,int mousey); //When the user clicks on a piece, it returns the piece's structure for manipulation
bool IsvalidMove(int selectedPiece); //Tests if the move the user wants to make is allowed

void StartSDL (); //Starts up and initalizes everything in SDL
void CloseSDL (); //Closes down and deletes everything in SDL
void LoadMedia(); //Loads all the pictures and fonts
SDL_Texture* LoadTexture( string path ); //Assigns all the pictures to their respective textures
SDL_Texture* LoadText (int X,int Y); //Loads and displays and given text onto the display window

//Piece and Board Textures
SDL_Renderer* Renderer = NULL;
SDL_Window* window = NULL;
SDL_Texture* BoardTexture = NULL;
SDL_Texture* CoordTexture = NULL;
SDL_Color White = {0,0,0};
TTF_Font *gFont = NULL;
TTF_Font *Font = NULL;

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
    bool quit;
    bool Mousedown = false;
    int SelectedPiece = -1;

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
                    if(!IsvalidMove(SelectedPiece))
                    {
                        Pieces[SelectedPiece].Rect = Pieces[SelectedPiece].OldLocation;
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
    }

    PrintBoard(board);

    CloseSDL();

    return 0;
}

void assignValues (TPiece WhatPiece, int Rectx, int Recty, string path , int i,bool isWhite)
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
    Pieces[i].FirstMove = true;
    Pieces[i].IsWhite = isWhite;
}

void SetPositions (TPiece Board[8][8])
{
    int xtrack = 0;

    for (int i = 0; i < 8; i++)
    {
        assignValues(pawn11,xtrack*SquareSize+Offset,6*SquareSize+Offset,"Pictures/WhitePawn.gif",i,true);
        xtrack++;
    }

    xtrack = 0;

    for (int i = 8; i < 16; i++)
    {
        assignValues(pawn11,xtrack*SquareSize+Offset,SquareSize+Offset,"Pictures/BlackPawn.gif",i,false);
        xtrack ++;
    }

    xtrack = 0;

    for (int i = 16; i < 18; i++)
    {
        assignValues(rook11,xtrack*SquareSize+Offset,7*SquareSize+Offset,"Pictures/WhiteRook.gif",i,true);
        xtrack += 7;
    }

    xtrack = 0;

    for (int i = 18; i < 20; i++)
    {
        assignValues(rook11,xtrack*SquareSize+Offset,Offset,"Pictures/BlackRook.gif",i,false);
        xtrack += 7;
    }

    xtrack = 1;

    for (int i = 20; i < 22; i++)
    {
        assignValues(knight,xtrack*SquareSize+Offset,7*SquareSize+Offset,"Pictures/WhiteKnight.gif",i,true);
        xtrack += 5;
    }

    xtrack = 1;

    for (int i = 22; i < 24; i++)
    {
        assignValues(knight,xtrack*SquareSize+Offset,Offset,"Pictures/BlackKnight.gif",i,false);
        xtrack += 5;
    }

    xtrack = 2;

    for (int i = 24; i < 26; i++)
    {
        assignValues (bishop,xtrack*SquareSize+Offset,7*SquareSize+Offset,"Pictures/WhiteBishop.gif",i,true);
        xtrack += 3;
    }

    xtrack = 2;

    for (int i = 26; i < 28; i++)
    {
        assignValues(bishop,xtrack*SquareSize+Offset,Offset,"Pictures/BlackBishop.gif",i,false);
        xtrack += 3;
    }

    assignValues(king11,4*SquareSize+Offset,Offset,"Pictures/BlackKing.gif",31,false);
    assignValues(king11,4*SquareSize+Offset,7*SquareSize+Offset,"Pictures/WhiteKing.gif",30,true);
    assignValues(queen1,3*SquareSize+Offset,Offset,"Pictures/BlackQueen.gif",29,false);
    assignValues(queen1,3*SquareSize+Offset,7*SquareSize+Offset,"Pictures/WhiteQueen.gif",28,true);

    for (int y = 0; y < 8; y++)
        for (int x = 0; x < 8; x++)
            board[x][y] = blank1;
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
            cout << Pieces[i].TypeOfPiece << " has been selected " << i << endl;
            Pieces[i].OldLocation = Pieces[i].Rect;
            return i;
        }
    }
    return -1;
}

int FindMatch(int x, int y, int SelectedPiece)
{
    for (int i = 0; i < 32; i++)
    {
        if (i == SelectedPiece) continue;

        if (Pieces[i].Rect.y == y && Pieces[i].Rect.x == x) return i;
    }
    return -1;
}

bool PieceTake(int selectedPiece)
{
    int beingTaken = FindMatch(Pieces[selectedPiece].Rect.x, Pieces[selectedPiece].Rect.y, selectedPiece);

    if (beingTaken >= 0)
        if (Pieces[selectedPiece].IsWhite != Pieces[beingTaken].IsWhite)
        {
            Pieces[beingTaken].Rect.x = 8*SquareSize + Offset;
            Pieces[beingTaken].Rect.y = Offset;
            Pieces[selectedPiece].FirstMove = false;
            return true;
        }
    return false;
}

void PrintBoard (TPiece Board[8][8])
{
    int xtrack = 0;
    int ytrack = 0;

    for (int i = 0; i < 32; i++)
    {
        xtrack = (Pieces[i].Rect.x-50)/SquareSize;
        ytrack = (Pieces[i].Rect.y-50)/SquareSize;

        Board[xtrack][ytrack] = Pieces[i].TypeOfPiece;
    }

    for (int y = 0; y < 8; y++)
    {
        for (int x = 0; x < 8; x++)
            cout << Board[x][y] << " ";
        cout << endl;
    }
}

//*****************************************************
//********************Pawn Movement********************
//*****************************************************

int PawnSomethingInWay(int selectedPiece)
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

bool PawnMove(int selectedPiece)
{
    int SomethingInfrontOfPawn = PawnSomethingInWay(selectedPiece);

    if (Pieces[selectedPiece].FirstMove)
    {
        if (Pieces[selectedPiece].IsWhite) // White pawn first move
        {
            if (Pieces[selectedPiece].Rect.y == Pieces[selectedPiece].OldLocation.y - 2*SquareSize && Pieces[selectedPiece].Rect.x == Pieces[selectedPiece].OldLocation.x && SomethingInfrontOfPawn == 0)
            {
                Pieces[selectedPiece].FirstMove = false;
                return true;
            }
            if (Pieces[selectedPiece].Rect.y == Pieces[selectedPiece].OldLocation.y - SquareSize && Pieces[selectedPiece].Rect.x == Pieces[selectedPiece].OldLocation.x && SomethingInfrontOfPawn != 1)
            {
                Pieces[selectedPiece].FirstMove = false;
                return true;
            }
        }
        else //Black pawn first move
        {
            if (Pieces[selectedPiece].Rect.y == Pieces[selectedPiece].OldLocation.y + 2*SquareSize && Pieces[selectedPiece].Rect.x == Pieces[selectedPiece].OldLocation.x && SomethingInfrontOfPawn == 0)
            {
                Pieces[selectedPiece].FirstMove = false;
                return true;
            }
            if (Pieces[selectedPiece].Rect.y == Pieces[selectedPiece].OldLocation.y + SquareSize && Pieces[selectedPiece].Rect.x == Pieces[selectedPiece].OldLocation.x && SomethingInfrontOfPawn != 1)
            {
                Pieces[selectedPiece].FirstMove = false;
                return true;
            }
        }
    }
    else //Regular One square moving after the first move
    {
        if (Pieces[selectedPiece].IsWhite)
        {
            if (Pieces[selectedPiece].Rect.y == Pieces[selectedPiece].OldLocation.y - SquareSize && Pieces[selectedPiece].Rect.x == Pieces[selectedPiece].OldLocation.x && SomethingInfrontOfPawn != 1)
                return true;
        }
        else
        {
            if (Pieces[selectedPiece].Rect.y == Pieces[selectedPiece].OldLocation.y + SquareSize && Pieces[selectedPiece].Rect.x == Pieces[selectedPiece].OldLocation.x && SomethingInfrontOfPawn != 1)
                return true;
        }
    }

    if (Pieces[selectedPiece].IsWhite) //White Pawns Taking Pieces
    {
        if ((Pieces[selectedPiece].Rect.y == Pieces[selectedPiece].OldLocation.y - SquareSize && Pieces[selectedPiece].Rect.x == Pieces[selectedPiece].OldLocation.x-SquareSize)||(Pieces[selectedPiece].Rect.y == Pieces[selectedPiece].OldLocation.y - SquareSize && Pieces[selectedPiece].Rect.x == Pieces[selectedPiece].OldLocation.x+SquareSize))
        {
            if (PieceTake(selectedPiece)) return true;
        }
    }
    else //Black Pawns Taking Pieces
    {
        if ((Pieces[selectedPiece].Rect.y == Pieces[selectedPiece].OldLocation.y + SquareSize && Pieces[selectedPiece].Rect.x == Pieces[selectedPiece].OldLocation.x+SquareSize)||(Pieces[selectedPiece].Rect.y == Pieces[selectedPiece].OldLocation.y + SquareSize && Pieces[selectedPiece].Rect.x == Pieces[selectedPiece].OldLocation.x-SquareSize))
        {
            if (PieceTake(selectedPiece)) return true;
        }
    }

    return false;
}

//*****************************************************
//*******************Knight Movement*******************
//*****************************************************

bool IsKnightBlocked(int selectedPiece)
{
    int MovingToSquare = FindMatch(Pieces[selectedPiece].Rect.x, Pieces[selectedPiece].Rect.y, selectedPiece);

    if (Pieces[selectedPiece].IsWhite == Pieces[MovingToSquare].IsWhite) return false;

    return true;
}

bool KnightMove(int selectedPiece)
{
    bool GoodMove = false;

    bool SomethingInKnightMove = IsKnightBlocked(selectedPiece);

    if (Pieces[selectedPiece].FirstMove) Pieces[selectedPiece].FirstMove = false;

    if (!SomethingInKnightMove) return false;

    if (Pieces[selectedPiece].OldLocation.y + 2*SquareSize == Pieces[selectedPiece].Rect.y && Pieces[selectedPiece].OldLocation.x + SquareSize == Pieces[selectedPiece].Rect.x) GoodMove = true;
    if (Pieces[selectedPiece].OldLocation.y + 2*SquareSize == Pieces[selectedPiece].Rect.y && Pieces[selectedPiece].OldLocation.x - SquareSize == Pieces[selectedPiece].Rect.x) GoodMove = true;
    if (Pieces[selectedPiece].OldLocation.y - 2*SquareSize == Pieces[selectedPiece].Rect.y && Pieces[selectedPiece].OldLocation.x + SquareSize == Pieces[selectedPiece].Rect.x) GoodMove = true;
    if (Pieces[selectedPiece].OldLocation.y - 2*SquareSize == Pieces[selectedPiece].Rect.y && Pieces[selectedPiece].OldLocation.x - SquareSize == Pieces[selectedPiece].Rect.x) GoodMove = true;
    if (Pieces[selectedPiece].OldLocation.x + 2*SquareSize == Pieces[selectedPiece].Rect.x && Pieces[selectedPiece].OldLocation.y + SquareSize == Pieces[selectedPiece].Rect.y) GoodMove = true;
    if (Pieces[selectedPiece].OldLocation.x + 2*SquareSize == Pieces[selectedPiece].Rect.x && Pieces[selectedPiece].OldLocation.y - SquareSize == Pieces[selectedPiece].Rect.y) GoodMove = true;
    if (Pieces[selectedPiece].OldLocation.x - 2*SquareSize == Pieces[selectedPiece].Rect.x && Pieces[selectedPiece].OldLocation.y + SquareSize == Pieces[selectedPiece].Rect.y) GoodMove = true;
    if (Pieces[selectedPiece].OldLocation.x - 2*SquareSize == Pieces[selectedPiece].Rect.x && Pieces[selectedPiece].OldLocation.y - SquareSize == Pieces[selectedPiece].Rect.y) GoodMove = true;

    if (GoodMove)
    {
        PieceTake(selectedPiece);
        return true;
    }

    return false;
}

//*****************************************************
//*******************Bishop Movement*******************
//*****************************************************

bool IsBishopBlocked(int selectedPiece)
{
    int xcheck = Pieces[selectedPiece].OldLocation.x;

    if (Pieces[selectedPiece].OldLocation.y < Pieces[selectedPiece].Rect.y && Pieces[selectedPiece].OldLocation.x < Pieces[selectedPiece].Rect.x)
    {
        cout << "1" << endl;
        for (int checky = Pieces[selectedPiece].OldLocation.y; checky <= Pieces[selectedPiece].Rect.y; checky += SquareSize)
        {

            if (FindMatch(xcheck,checky,selectedPiece)>= 0) return true;
            if (checky == Pieces[selectedPiece].Rect.y && xcheck == Pieces[selectedPiece].Rect.x) return false;
            xcheck+= SquareSize;
        }
    }
    else if (Pieces[selectedPiece].OldLocation.y < Pieces[selectedPiece].Rect.y && Pieces[selectedPiece].OldLocation.x > Pieces[selectedPiece].Rect.x)
    {
        cout << "2" << endl;
        for (int checky = Pieces[selectedPiece].OldLocation.y; checky <= Pieces[selectedPiece].Rect.y; checky += SquareSize)
        {
            if (FindMatch(xcheck,checky,selectedPiece)>= 0) return true;
            if (checky == Pieces[selectedPiece].Rect.y && xcheck == Pieces[selectedPiece].Rect.x) return false;
            xcheck-= SquareSize;
        }
    }
    else if (Pieces[selectedPiece].OldLocation.y > Pieces[selectedPiece].Rect.y && Pieces[selectedPiece].OldLocation.x < Pieces[selectedPiece].Rect.x)
    {
        cout << "3" << endl;
        for (int checky = Pieces[selectedPiece].OldLocation.y; checky >= Pieces[selectedPiece].Rect.y; checky -= SquareSize)
        {
            cout << xcheck << " , " << checky << endl;
            cout << Pieces[selectedPiece].Rect.x << " , " << Pieces[selectedPiece].Rect.y << endl << endl;
            if (FindMatch(xcheck,checky,selectedPiece)>= 0) return true;
            if (checky == Pieces[selectedPiece].Rect.y && xcheck == Pieces[selectedPiece].Rect.x) return false;
            xcheck+= SquareSize;
        }
    }
    else if (Pieces[selectedPiece].OldLocation.y > Pieces[selectedPiece].Rect.y && Pieces[selectedPiece].OldLocation.x > Pieces[selectedPiece].Rect.x)
    {
        cout << "4" << endl;
        for (int checky = Pieces[selectedPiece].OldLocation.y; checky >= Pieces[selectedPiece].Rect.y; checky -= SquareSize)
        {
            if (FindMatch(xcheck,checky,selectedPiece)>= 0) return true;
            if (checky == Pieces[selectedPiece].Rect.y && xcheck == Pieces[selectedPiece].Rect.x)
            {
                Pie
                return false;
            }
            xcheck-= SquareSize;
        }
    }
    return true;
}

bool BishopMove(int selectedPiece)
{
    if (!IsBishopBlocked(selectedPiece))
    {
        if (Pieces[selectedPiece].FirstMove) Pieces[selectedPiece].FirstMove = false;

        return true;
    }
    return false;
}

//*****************************************************
//********************Rook Movement********************
//*****************************************************



//*****************************************************
//********************King Movement********************
//*****************************************************



//*****************************************************
//********************Queen Movement*******************
//*****************************************************

bool IsvalidMove(int selectedPiece)
{
    if (mousex >= BoardHeight+50 || mousex < 50 || mousey >= BoardHeight+50 || mousey < 50) return false;

    if (Pieces[selectedPiece].TypeOfPiece == pawn11)
    {
        if (PawnMove(selectedPiece)) return true;
    }
    else if (Pieces[selectedPiece].TypeOfPiece == knight)
    {
        if (KnightMove(selectedPiece)) return true;
    }
    else if (Pieces[selectedPiece].TypeOfPiece == bishop)
    {
        if (BishopMove(selectedPiece)) return true;
    }
    else
    {
        return true;
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

    window = SDL_CreateWindow( "Chess", 8, 31, ScreenWidth, ScreenHeight, SDL_WINDOW_SHOWN );

    Renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED );

    SDL_SetRenderDrawColor( Renderer, 0xFF, 0xFF, 0xFF, 0xFF );

    IMG_Init( IMG_INIT_PNG ) & (IMG_INIT_PNG );

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

void LoadMedia()
{
    BoardTexture = LoadTexture("Pictures/Board.gif");
    CoordTexture = LoadText(mousex,mousey);
    SDL_Texture* TempTexture = NULL;
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
