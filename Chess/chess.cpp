//Note add en passent and stalemates and turns
#include <iostream> //preamble
#include <vector> //preamble

using namespace std; //premble

//declare a constant global variable for each peice (value of variable = peice value)
const int pawn = 10;
const int knight = 30;
const int bishop = 32;
const int rook = 50;
const int queen = 90;
const int king = 1035;

//create global variables (accessed by all functions)
int chessBoard[10][10];
bool WhiteInCheck = false;
bool BlackInCheck = false;
bool WhiteKingMove = false;
bool BlackKingMove = false;
bool WhiteKingRookMove = false;
bool WhiteQueenRookMove = false;
bool BlackKingRookMove = false;
bool BlackQueenRookMove = false;

//declaring functions
void startingPos();
void textDisplay();
void castlingConditions (int y, int x);
bool checkMove(int startingRank, int startingfile, int endingRank, int endingfile);
bool checkMovePawn(int startingRank, int startingfile, int endingRank, int endingfile, string colour);
bool checkMoveKnight(int startingRank, int startingfile, int endingRank, int endingfile, string colour);
bool checkMoveBishop(int startingRank, int startingfile, int endingRank, int endingfile, string colour);
bool checkMoveRook(int startingRank, int startingfile, int endingRank, int endingfile, string colour);
bool checkMoveQueen(int startingRank, int startingfile, int endingRank, int endingfile, string colour);
bool checkMoveKing(int startingRank, int startingfile, int endingRank, int endingfile, string colour);
bool checkForCastling(int startingRank, int startingfile, int endingRank, int endingfile, string colour);
bool checkSquare(int y, int x, string colour);
void CheckForCheck();
void CheckForCheckmate(int y, int x, string colour);

struct positions
{
    int board[8][8];
    int numTimes;
};

typedef vector <positions> pvec; //declare type

int main()
{
    int xi, yi, xf, yf;
    startingPos();
    for (;;)
    {
        textDisplay();
        cin >> yi >> xi >> yf >> xf;
        if (checkMove(yi,xi,yf,xf) == true)
        {
        }
    }
}

//function to reset board to starting position
void startingPos()
{
    //set all squares = 0 (no peice)
    for (int x = 0; x <= 10; x++)
    {
        for (int y = 1; y <= 8; y++) chessBoard[x][y] = 0;
    }
    //loop that sets each value for each peice in its starting position (black and white)
    for (int x = 1; x >= -1; x -= 2)
    {
        int rankValue = 1;
        //changes rank peices are placed on for black and white
        if (x < 0) rankValue = 8;
        //sets each peice to starting pos (pawns are not peices!!!!)
        chessBoard[rankValue][1] = rook * x;
        chessBoard[rankValue][2] = knight * x;
        chessBoard[rankValue][3] = bishop * x;
        chessBoard[rankValue][4] = queen * x;
        chessBoard[rankValue][5] = king * x;
        chessBoard[rankValue][6] = bishop * x;
        chessBoard[rankValue][7] = knight * x;
        chessBoard[rankValue][8] = rook * x;
        rankValue += x; //changes rank to 2nd or 7th for pawns
        //places pawns on all available spots on said rank
        for (int pawns = 1; pawns <= 8; pawns++)
            chessBoard[rankValue][pawns] = pawn * x;
    }
    WhiteKingMove = false;
    BlackKingMove = false;
    WhiteKingRookMove = false;
    WhiteQueenRookMove = false;
    BlackKingRookMove = false;
    BlackQueenRookMove = false;
}

//function to display the board in text
void textDisplay()
{
    for (int x = 8; x >= 1; x--)
    {
        for (int y = 1; y <= 8; y++)
        {
            if (chessBoard[x][y] == 0) cout << "00"; //outputting no piece
            else
            {
                int colour;
                if (chessBoard[x][y] > 0)
                {
                    cout << "W"; //outputting colour white
                    colour = 1;
                }
                else
                {
                    cout << "B"; //outputting colour black
                    colour = -1;
                }
                if (chessBoard[x][y] == colour*pawn) cout << "P"; //outputting pawn
                if (chessBoard[x][y] == colour*knight) cout << "N"; //outputting knight
                if (chessBoard[x][y] == colour*bishop) cout << "B"; //outputting bishop
                if (chessBoard[x][y] == colour*rook) cout << "R"; //outputting rook
                if (chessBoard[x][y] == colour*queen) cout << "Q"; //outputting queen
                if (chessBoard[x][y] == colour*king) cout << "K"; //outputting king
            }
            cout << " "; //putting a space between the pieces
        }
        cout << endl; //ending the line
    }
}

//function to set castling conditions
void castlingConditions (int y, int x)
{
    if (y == 1 and x == 1) WhiteQueenRookMove =true; //testing and setting if white's queenside rook has moved
    if (y == 1 and x == 8) WhiteKingRookMove = true; //testing and setting if white's kingside rook has moved
    if (y == 8 and x == 1) BlackQueenRookMove =true; //testing and setting if black's queenside rook has moved
    if (y == 8 and x == 8) BlackKingRookMove = true;//testing and setting if black's kingside rook has moved
    if (y == 1 and x == 5) WhiteKingMove = true; //testing and setting if white's king has moved
    if (y == 8 and x == 5) BlackKingMove = true; //testing and setting if black's king has moved
}

//function to test if a peice can move
bool checkMove(int startingY, int startingX, int endingY, int endingX)
{
    int piece = chessBoard[startingY][startingX]; //finding what peice is being moved
    int color = 1;

    string colour; //declaring variable
    //if nothing is moved return false
    if (piece == 0) return false;
    //checking colour of piece
    if (piece > 0) colour = "White";
    else
    {
        colour = "Black"; //set val
        color = -1; //set val
        piece *= color; //set val
    }
    //calling appropriate function based on which peice is moving
    if (piece == pawn) //checking pawn moves
    {
        if (checkMovePawn(startingY, startingX, endingY, endingX, colour)==true)
        {
            chessBoard[startingY][startingX]=0; //setting pos
            chessBoard[endingY][endingX]=piece*color; //setting pos
            CheckForCheck(); //checking for check
            if (colour == "White")
            {
                if (WhiteInCheck == true)
                {
                    chessBoard[startingY][startingX]=piece*color; //reseting pos
                    chessBoard[endingY][endingX]=0; //reseting pos
                    return false; //returning false if white is in check
                }
            }
            else
            {
                if (BlackInCheck == true)
                {
                    chessBoard[startingY][startingX]=piece*color; //reseting pos
                    chessBoard[endingY][endingX]=0;//reseting pos
                    return false; //returning false if black is in check
                }
            }
            char promotion; //declare var
            if (colour == "White" and endingY == 8)
            {
                cout << "What peice will you promote to?(Q, R, B, N): "; //ask what peice they want
                cin >> promotion; //get what peice they want
                cout << promotion << " | " << endingY << " | " << endingX << endl;
                if (promotion == 'Q') chessBoard[endingY][endingX]=queen; //promote to a queen
                if (promotion == 'R') chessBoard[endingY][endingX]=rook; //promote to a rook
                if (promotion == 'B') chessBoard[endingY][endingX]=bishop; //promote to a bishop
                if (promotion == 'N') chessBoard[endingY][endingX]=knight; //promote to a kinght
            }
            if (colour == "Black" and endingY == 1)
            {
                cout << "What peice will you promote to?(Q, R, B, N: "; //ask what peice they want
                cin >> promotion; //get what peice they want
                if (promotion == 'Q') chessBoard[endingY][endingX]=-queen; //promote to a queen
                if (promotion == 'R') chessBoard[endingY][endingX]=-rook; //promote to a rook
                if (promotion == 'B') chessBoard[endingY][endingX]=-bishop; //promote to a bishop
                if (promotion == 'N') chessBoard[endingY][endingX]=-knight; //promote to a kinght
            }
            return true; //return true
        }
    }
    else if (piece == knight) //checking knight moves
    {
        if (checkMoveKnight(startingY, startingX, endingY, endingX, colour)==true)
        {
            chessBoard[startingY][startingX]=0; //setting pos
            chessBoard[endingY][endingX]=piece*color; //setting pos
            CheckForCheck(); //checking for check
            if (colour == "White")
            {
                if (WhiteInCheck == true)
                {
                    chessBoard[startingY][startingX]=piece*color; //reseting pos
                    chessBoard[endingY][endingX]=0; //reseting pos
                    return false; //returning false if white is in check
                }
            }
            else
            {
                if (BlackInCheck == true)
                {
                    chessBoard[startingY][startingX]=piece*color; //reseting pos
                    chessBoard[endingY][endingX]=0;//reseting pos
                    return false; //returning false if black is in check
                }
            }
            return true; //return true
        }
    }
    else if (piece == bishop) //checking bishop moves
    {
        if (checkMoveBishop(startingY, startingX, endingY, endingX, colour)==true)
        {
            chessBoard[startingY][startingX]=0; //setting pos
            chessBoard[endingY][endingX]=piece*color; //setting pos
            CheckForCheck(); //checking for check
            if (colour == "White")
            {
                if (WhiteInCheck == true)
                {
                    chessBoard[startingY][startingX]=piece*color; //reseting pos
                    chessBoard[endingY][endingX]=0; //reseting pos
                    return false; //returning false if white is in check
                }
            }
            else
            {
                if (BlackInCheck == true)
                {
                    chessBoard[startingY][startingX]=piece*color; //reseting pos
                    chessBoard[endingY][endingX]=0;//reseting pos
                    return false; //returning false if black is in check
                }
            }
            return true; //return true
        }
    }
    else if (piece == rook) //checking rook moves
    {
        if (checkMoveRook(startingY, startingX, endingY, endingX, colour)==true)
        {
            chessBoard[startingY][startingX]=0; //setting pos
            chessBoard[endingY][endingX]=piece*color; //setting pos
            CheckForCheck(); //checking for check
            if (colour == "White")
            {
                if (WhiteInCheck == true)
                {
                    chessBoard[startingY][startingX]=piece*color; //reseting pos
                    chessBoard[endingY][endingX]=0; //reseting pos
                    return false; //returning false if white is in check
                }
            }
            else
            {
                if (BlackInCheck == true)
                {
                    chessBoard[startingY][startingX]=piece*color; //reseting pos
                    chessBoard[endingY][endingX]=0;//reseting pos
                    return false; //returning false if black is in check
                }
            }
            return true; //return true
        }
    }
    else if (piece == queen) //checking queen moves
    {
        if (checkMoveQueen(startingY, startingX, endingY, endingX, colour)==true)
        {
            chessBoard[startingY][startingX]=0; //setting pos
            chessBoard[endingY][endingX]=piece*color; //setting pos
            CheckForCheck(); //checking for check
            if (colour == "White")
            {
                if (WhiteInCheck == true)
                {
                    chessBoard[startingY][startingX]=piece*color; //reseting pos
                    chessBoard[endingY][endingX]=0; //reseting pos
                    return false; //returning false if white is in check
                }
            }
            else
            {
                if (BlackInCheck == true)
                {
                    chessBoard[startingY][startingX]=piece*color; //reseting pos
                    chessBoard[endingY][endingX]=0;//reseting pos
                    return false; //returning false if black is in check
                }
            }
            return true; //return true
        }
    }
    else if (piece == king) //checking king moves
    {
        if (checkMoveKing(startingY, startingX, endingY, endingX, colour)==true)
        {
            chessBoard[startingY][startingX]=0; //setting pos
            chessBoard[endingY][endingX]=piece*color; //setting pos
            CheckForCheck(); //checking for check
            if (colour == "White")
            {
                if (WhiteInCheck == true)
                {
                    chessBoard[startingY][startingX]=piece*color; //reseting pos
                    chessBoard[endingY][endingX]=0; //reseting pos
                    return false; //returning false if white is in check
                }
            }
            else
            {
                if (BlackInCheck == true)
                {
                    chessBoard[startingY][startingX]=piece*color; //reseting pos
                    chessBoard[endingY][endingX]=0;//reseting pos
                    return false; //returning false if black is in check
                }
            }
            castlingConditions(startingY, startingX); //checking castling conditions
            return true; //return true
        }
    }
    return false; //return true
}

//function to check if a pawn move is valid
bool checkMovePawn(int startingRank, int startingfile, int endingRank, int endingfile, string colour)
{
    int initialRank = 2; //declare var
    int x = 1; //declare var
    //sets some values if colour is black
    if (colour == "Black")
    {
        initialRank = 7; //set val
        x *= -1; //setval
    }
    if (startingfile != endingfile and startingfile+1 != endingfile and startingfile-1 != endingfile) return false; //return false
    if (startingfile == endingfile)
    {
        if (startingRank == initialRank)
        {
            if (startingRank+(2*x) == endingRank)
            {
                if (chessBoard[startingRank+(x)][startingfile]== 0 and chessBoard[startingRank+(2*x)][startingfile]== 0)
                {
                    chessBoard[endingRank][endingfile] = x;
                    return true; //return true
                }
            }
        }
        if (startingRank+(x) == endingRank) if (chessBoard[startingRank+(x)][startingfile]== 0) return true;//return true
    }
    if ((startingfile+1 == endingfile or startingfile-1 == endingfile)and startingRank + (1*x) == endingRank)
    {
        if (colour == "White") if (chessBoard[endingRank][endingfile] < 0) return true;//return true
            else if (chessBoard[endingRank][endingfile] > 0) return true; //return true
    }
    return false; //return false
}

//function to check if a knight is valid
bool checkMoveKnight(int yi, int xi, int yf, int xf, string colour)
{
    if (colour == "White" and chessBoard[yf][xf] > 0) return false; //return false
    if (colour == "Black" and chessBoard[yf][xf] < 0) return false; //return false
    if (yi + 2 == yf and xi + 1 == xf) return true; //return true
    if (yi + 2 == yf and xi - 1 == xf) return true; //return true
    if (yi - 2 == yf and xi + 1 == xf) return true; //return true
    if (yi - 2 == yf and xi - 1 == xf) return true; //return true
    if (xi + 2 == xf and yi + 1 == yf) return true; //return true
    if (xi + 2 == xf and yi - 1 == yf) return true; //return true
    if (xi - 2 == xf and yi + 1 == yf) return true; //return true
    if (xi - 2 == xf and yi - 1 == yf) return true; //return true
    return false; //return false
}

//function to check if a bishop move is valid
bool checkMoveBishop(int yi, int xi, int yf, int xf, string colour)
{
    int xcheck; //declare var
    bool check = false; //declare var
    if (colour == "White" and chessBoard[yf][xf] > 0) return false; //return false
    if (colour == "Black" and chessBoard[yf][xf] < 0) return false; //return false
    xcheck = xi;//set val
    for (int checky = yi+1; checky <= yf; checky++)
    {
        if (checky == yf and xcheck == xf) return true; //return true
        if (chessBoard[checky][xcheck] != 0)break; //exit loop
        xcheck++; //add one
    }
    xcheck = xi; //set val
    for (int checky = yi+1; checky <= yf; checky++)
    {
        if (checky == yf and xcheck == xf) return true; //return true
        if (chessBoard[checky][xcheck] != 0)  break; //exit loop
        xcheck--; //subtract one
    }
    xcheck = xi; //set val
    for (int checky = yi-1; checky >= yf; checky--)
    {
        if (checky == yf and xcheck == xf) return true; //return true
        if (chessBoard[checky][xcheck] != 0) break; //exit loop
        xcheck++; //add one
    }
    xcheck = xi; //set val
    for (int checky = yi-1; checky >= yf; checky--)
    {
        if (checky == yf and xcheck == xf) return true; //return true
        if (chessBoard[checky][xcheck] != 0) break; //exit loop
        xcheck--; //subtract one
    }
    return false; //return false
}

//function to check if a rook move is valid
bool checkMoveRook(int yi, int xi, int yf, int xf, string colour)
{
    if (colour == "White" and chessBoard[yf][xf] > 0) return false; //return false
    if (colour == "Black" and chessBoard[yf][xf] < 0) return false; //return false
    if (yi != yf and xi != xf) return false; //return false
    if (yi != yf)
    {
        for (int y = yi+1; y <= yf; y++)
        {
            if (y == yf) return true; //return true
            if (chessBoard[y][xi]!= 0) return false; //return false
        }
        for (int y = yi-1; y >= yf; y--)
        {
            if (y == yf) return true; //return true
            if (chessBoard[y][xi]!= 0) return false; //return false
        }
    }
    else if (xi != xf)
    {
        for (int x = xi+1; x <= xf; x++)
        {
            if (x == xf) return true; //return true
            if (chessBoard[yi][x]!= 0) return false; //return false
        }
        for (int x = xi-1; x >= xf; x--)
        {
            if (x == xf) return true; //return true
            if (chessBoard[yi][x]!= 0) return false; //return false
        }
    }
    return false; //return false
}

//check to see if a queen move is valid
bool checkMoveQueen(int yi, int xi, int yf, int xf, string colour)
{
    if (checkMoveBishop(yi, xi, yf, xf, colour) == true or checkMoveRook(yi, xi, yf, xf, colour) == true) return true; //return true
    return false; //return false
}

//check to see if a king move is valid
bool checkMoveKing(int yi, int xi, int yf, int xf, string colour)
{
    if (colour == "White" and chessBoard[yf][xf] > 0) return false; //return false
    if (colour == "Black" and chessBoard[yf][xf] < 0) return false; //return false
    if (yi == yf or yi + 1 == yf or yi - 1 == yf) if (xi == xf or xi + 1 == xf or xi - 1 == xf) return true; //return true
    if (checkForCastling(yi,xi,yf,xf,colour) == true) return true; //return true
    return false; //return false
}

//check to see if castling is valid
bool checkForCastling(int yi, int xi, int yf, int xf, string colour)
{
    if (yi == 1 and xi == 5 and colour == "White")
    {
        if (yf == 1 and xf == 7 and WhiteKingMove == false and WhiteKingRookMove == false)
        {
            if (checkSquare(1,5,colour)==true and checkSquare(1,6,colour)==true and checkSquare(1,7,colour)==true)
            {
                if (chessBoard[1][6]==0 and chessBoard[1][7] == 0)
                {
                    chessBoard[1][8]=0; //set val
                    chessBoard[1][5]=0; //set val
                    chessBoard[1][6]=rook; //set val
                    chessBoard[1][7]=king; //set val
                    return true;//return true
                }
            }

        }
        if (yf == 1 and xf == 3 and WhiteKingMove == false and WhiteQueenRookMove == false)
        {
            if (checkSquare(1,5,colour)==true and checkSquare(1,4,colour)==true and checkSquare(1,3,colour)==true)
            {
                if (chessBoard[1][4]==0 and chessBoard[1][3] == 0)
                {
                    chessBoard[1][1]=0; //set val
                    chessBoard[1][5]=0; //set val
                    chessBoard[1][4]=rook; //set val
                    chessBoard[1][3]=king; //set val
                    return true; //return true
                }
            }

        }
    }
    if (yi == 8 and xi == 5 and colour == "Black")
    {
        if (yf == 8 and xf == 7 and BlackKingMove == false and BlackKingRookMove == false)
        {
            if (checkSquare(8,5,colour)==true and checkSquare(8,6,colour)==true and checkSquare(8,7,colour)==true)
            {
                if (chessBoard[8][6]==0 and chessBoard[8][7] == 0)
                {
                    chessBoard[8][8]=0; //set val
                    chessBoard[8][5]=0; //set val
                    chessBoard[8][6]=-rook; //set val
                    chessBoard[8][7]=-king; //set val
                    return true; //return true
                }
            }
        }
        if (yf == 8 and xf == 3 and BlackKingMove == false and BlackQueenRookMove == false)
        {
            if (checkSquare(8,5,colour)==true and checkSquare(8,4,colour)==true and checkSquare(8,3,colour)==true)
            {
                if (chessBoard[8][4]==0 and chessBoard[8][3] == 0)
                {
                    chessBoard[8][1]=0; //set val
                    chessBoard[8][5]=0; //set val
                    chessBoard[8][4]=-rook; //set val
                    chessBoard[8][3]=-king; //set val
                    return true; //return true
                }
            }
        }
    }
    return false; //return false
}

//bool function to check a square
bool checkSquare(int y, int x, string color)
{
    int colour; //declare var
    int checkx; //declare var
    if (color == "White") colour = -1; //set val
    if (color == "Black") colour = 1; //set val
    if (chessBoard[y+(1*(-colour))][x+1] == colour*pawn or chessBoard[y+(1*(-colour))][x-1] == colour*pawn) return false; //return false
    checkx = x; //set val
    for (int i = y; i <= 8; i++)
    {
        if (chessBoard[i][checkx] == colour*bishop or chessBoard[i][checkx] == colour*queen) return false; //return false
        if (chessBoard[i][checkx] != 0) break; //exit loop
        checkx++; //set val
    }
    checkx = x; //set val
    for (int i = y; i <= 8; i++)
    {
        if (chessBoard[i][checkx] == colour*bishop or chessBoard[i][checkx] == colour*queen) return false; //return false
        if (chessBoard[i][checkx] != 0) break; //exit loop
        checkx--; //set val
    }
    checkx = x; //set val
    for (int i = y; i >= 1; i--)
    {
        if (chessBoard[i][checkx] == colour*bishop or chessBoard[i][checkx] == colour*queen) return false; //return false
        if (chessBoard[i][checkx] != 0) break; //exit loop
        checkx++; //set val
    }
    checkx = x; //set val
    for (int i = y; i >= 1; i--)
    {
        if (chessBoard[i][checkx] == colour*bishop or chessBoard[i][checkx] == colour*queen) return false; //return false
        if (chessBoard[i][checkx] != 0) break; //exit loop
        checkx--; //set val
    }
    if (chessBoard[y + 2][x + 1] == colour*knight) return false; //return false
    if (chessBoard[y + 2][x - 1] == colour*knight) return false; //return false
    if (y != 1)
    {
        if (chessBoard[y - 2][x + 1] == colour*knight) return false; //return false
        if (chessBoard[y - 2][x - 1] == colour*knight) return false; //return false
    }
    if (chessBoard[y + 1][x + 2] == colour*knight) return false; //return false
    if (chessBoard[y - 1][x + 2] == colour*knight) return false; //return false
    if (chessBoard[y + 1][x - 2] == colour*knight) return false; //return false
    if (chessBoard[y - 1][x - 2] == colour*knight) return false; //return false
    for (int i = y; i <= 8; i++)
    {
        if (chessBoard[i][x] == colour*rook or chessBoard[i][x] == colour*queen)return false; //return false
        if (chessBoard[i][x] != 0) break; //exit loop
    }
    for (int i = y; i >= 0; i--)
    {
        if (chessBoard[i][x] == colour*rook or chessBoard[i][x] == colour*queen) return false; //return false
        if (chessBoard[i][x] != 0) break; //exit loop
    }
    for (int i = y; i <= 8; i++)
    {
        if (chessBoard[y][i] == colour*rook or chessBoard[y][i] == colour*queen) return false; //return false
        if (chessBoard[y][i] != 0) break; //exit loop
    }
    for (int i = y; i >= 0; i--)
    {
        if (chessBoard[y][i] == colour*rook or chessBoard[y][i] == colour*queen) return false; //return false
        if (chessBoard[y][i] != 0) break; //exit loop
    }
    if (chessBoard[x][y+1] == colour*king) return false; //return false
    if (chessBoard[x+1][y+1] == colour*king) return false; //return false
    if (chessBoard[x+1][y] == colour*king) return false; //return false
    if (chessBoard[x+1][y-1] == colour*king) return false; //return false
    if (chessBoard[x][y-1] == colour*king) return false; //return false
    if (chessBoard[x-1][y-1] == colour*king) return false; //return false
    if (chessBoard[x-1][y] == colour*king) return false; //return false
    if (chessBoard[x-1][y+1] == colour*king) return false; //return false
    return true; //return true
}

//void to check for check
void CheckForCheck()
{
    string colour; //declare var
    for (int y = 1; y <= 8; y++)
    {
        for (int x = 1; x <= 8; x++)
        {
            if (chessBoard[y][x] == king or chessBoard[y][x] == -king)
            {
                if (checkSquare(y,x,colour) == false)
                {
                    if (chessBoard[y][x]>0) WhiteInCheck = true; //set val
                    if (chessBoard[y][x]<0) BlackInCheck = true; //set val
                }
            }
        }
    }
}

void CheckForCheckmate(int y, int x, string colour)
{

}
