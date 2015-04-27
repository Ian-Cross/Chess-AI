#include <iostream>
#include <vector>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <sstream>

using namespace std;

//creating piece structure
struct Piece
{
    int BoardLocX;
    int BoardLocY;
    int Value;
    bool FirstMove;
    SDL_Rect Rect;
    SDL_Texture* Texture;
};

int board[8][8];
Piece Pieces[32];
int ScreenHeight = 0;
int ScreenWidth = 0;
int BoardHeight = 0;
int BoardWidth = 0;
int SquareSize = 0;
int mousex;
int mousey;

//Functions
void PrintBoard (Piece Pieces[32],int Board[8][8]);
void SetPositions (Piece Pieces[32],int Board[8][8]);
void assignValues (int xtrack, int ytrack, int Value, int Rectx, int Recty, string path, int i);
int SelectPiece(int mousex,int mousey);
void LoadMedia();
SDL_Texture* LoadTexture( string path );
SDL_Texture* LoadText (int X,int Y);
void StartSDL ();
void CloseSDL ();

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
    SetPositions(Pieces,board);

    SDL_Rect BoardRect;
    BoardRect.x = 50;
    BoardRect.y = 50;
    BoardRect.w = BoardHeight;
    BoardRect.h = BoardHeight;

    SDL_Rect TextRect;
    TextRect.x = 700;
    TextRect.y = 50;
    TextRect.w = 32;
    TextRect.h = 32;

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
                SelectedPiece = -1;
            }

        }

        if (Mousedown && SelectedPiece < 0 )
        {
            if (mousex >= 50 && mousex < BoardHeight && mousey >= 50 && mousey < BoardHeight)
                SelectedPiece = SelectPiece(mousex,mousey);
        }

        if (SelectedPiece > 0)
        {
            Pieces[SelectedPiece].Rect.x = mousex-SquareSize/2;
            Pieces[SelectedPiece].Rect.y = mousey-SquareSize/2;
        }

        SDL_RenderClear(Renderer);
        SDL_RenderCopy(Renderer,BoardTexture,NULL,&BoardRect);
        SDL_RenderCopy(Renderer,CoordTexture,NULL,&TextRect);

        for (int i = 0; i < 32; i++)
            SDL_RenderCopy(Renderer,Pieces[i].Texture,NULL,&Pieces[i].Rect);

        SDL_RenderPresent(Renderer);
    }

    PrintBoard(Pieces,board);

    CloseSDL();

    return 0;
}

void SetPositions (Piece Pieces[32],int Board[8][8])
{
    int xtrack = 0;

    for (int i = 0; i < 8; i++)
    {
        assignValues(xtrack,6,10,xtrack*SquareSize+50,6*SquareSize+50,"Pictures/WhitePawn.gif",i);
        xtrack++;
    }

    xtrack = 0;

    for (int i = 8; i < 16; i++)
    {
        assignValues(xtrack,1,-10,xtrack*SquareSize+50,SquareSize+50,"Pictures/BlackPawn.gif",i);
        xtrack ++;
    }

    xtrack = 0;

    for (int i = 16; i < 18; i++)
    {
        assignValues(xtrack,7,50,xtrack*SquareSize+50,7*SquareSize+50,"Pictures/WhiteRook.gif",i);
        xtrack += 7;
    }

    xtrack = 0;

    for (int i = 18; i < 20; i++)
    {
        assignValues(xtrack,0,-50,xtrack*SquareSize+50,50,"Pictures/BlackRook.gif",i);
        xtrack += 7;
    }

    xtrack = 1;

    for (int i = 20; i < 22; i++)
    {
        assignValues(xtrack,7,30,xtrack*SquareSize+50,7*SquareSize+50,"Pictures/WhiteKnight.gif",i);
        xtrack += 5;
    }

    xtrack = 1;

    for (int i = 22; i < 24; i++)
    {
        assignValues(xtrack,0,-30,xtrack*SquareSize+50,50,"Pictures/BlackKnight.gif",i);
        xtrack += 5;
    }

    xtrack = 2;

    for (int i = 24; i < 26; i++)
    {
        assignValues(xtrack,7,32,xtrack*SquareSize+50,7*SquareSize+50,"Pictures/WhiteBishop.gif",i);
        xtrack += 3;
    }

    xtrack = 2;

    for (int i = 26; i < 28; i++)
    {
        assignValues(xtrack,0,-32,xtrack*SquareSize+50,50,"Pictures/BlackBishop.gif",i);
        xtrack += 3;
    }

    assignValues(4,0,-1111,4*SquareSize+50,50,"Pictures/BlackKing.gif",31);
    assignValues(4,7,1111,4*SquareSize+50,7*SquareSize+50,"Pictures/WhiteKing.gif",30);
    assignValues(3,0,-90,3*SquareSize+50,50,"Pictures/BlackQueen.gif",29);
    assignValues(3,7,90,3*SquareSize+50,7*SquareSize+50,"Pictures/WhiteQueen.gif",28);

    for (int y = 0; y < 8; y++)
        for (int x = 0; x < 8; x++)
            board[x][y] = 0;
}

void assignValues (int xtrack, int ytrack, int Value, int Rectx, int Recty, string path , int i)
{
    SDL_Rect Rect;
    Rect.x = Rectx;
    Rect.y = Recty;
    Rect.h = SquareSize;
    Rect.w = SquareSize;

    SDL_Texture* Texture;

    Texture = LoadTexture(path);

    Pieces[i].BoardLocX = xtrack;
    Pieces[i].BoardLocY = ytrack;
    Pieces[i].Value = Value;
    Pieces[i].Texture = Texture;
    Pieces[i].Rect = Rect;
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
            return i;
    }
    return -1;
}

void PrintBoard (Piece Pieces[32],int Board[8][8])
{
    int xtrack = 0;
    int ytrack = 0;

    for (int i = 0; i < 32; i++)
    {
        xtrack = Pieces[i].BoardLocX;
        ytrack = Pieces[i].BoardLocY;
        Board[xtrack][ytrack] = Pieces[i].Rect.x;
    }

    for (int y = 0; y < 8; y++)
    {
        for (int x = 0; x < 8; x++)
            cout << Board[x][y] << " ";
        cout << endl;
    }
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

    string x,y;

    stringstream pp;
    pp << X;
    pp >> x;

    SDL_Surface* loadedSurface = TTF_RenderText_Solid(Font,x.c_str(),White);

    newTexture = SDL_CreateTextureFromSurface(Renderer, loadedSurface);

    SDL_FreeSurface(loadedSurface);

    return newTexture;
}
