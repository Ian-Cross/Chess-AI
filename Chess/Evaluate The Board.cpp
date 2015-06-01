#include <iostream>

using namespace std;

const int best = 1500;

//this function evaluates a board posistion and assigns a value based on the the following criteria:
//material, mobility(# of squates you can move to), bad pawns, and set values for stalemate and checkmate
int evaluate(TPiece someBoard[8][8], bool aiTurn)
{
    int evaluation = 0; //value to return, starts at zero
    //double loops to look through the entire board
    for(int x = 0; x < 8; x++)
    {
        for(int y = 0; y < 8; y++)
        {
            if(someBoard[x][y]==blackpawn) evaluation +=10; //add 10 points for AI's pawn
            else if(somBoard[x][y]==whitepawn) evaluation -=10; //subtract 10 points for opponent's pawn
            else if(someBoard[x][y]==blackknight) evaluation +=30; //add 30 points for AI's knight
            else if(someBoard[x][y]==whitekinght) evaluation -=30; //subtract 30 points for opponent's knight
            else if(someBoard[x][y]==blackbishop) evaluation +=32; //add 32 points for AI's bishop
            else if(someBoard[x][y]==whitebishop) evaluation -=32; //subtract 32 points for opponent's bishop
            else if(someBoard[x][y]==blackrook) evaluation +=50; //add 50 points for AI's rook
            else if(someBoard[x][y]==whiterook) evaluation -=50; //subtract 50 points for opponent's rook
            else if(someBoard[x][y]==blackqueen) evaluation +=90; //add 90 points for AI's queen
            else if(someBoard[x][y]==whitequeen) evaluation -=90; //subtract 90 points for opponent's queen
        }
    }
    //add 2 points for every square the AI can move to, subtract 2 for the square the opponent can move to
    evaluation += (numMovesBlack(someBoard) - numMovesWhite(someBoard))*2;
    //subtract 5 points for the AI's bad pawns, add 5 points for the opponent's bad pawns
    evaluation -= (badPawnsBlack(someBoard) - badPawnsWhite(someBoard))*5;
    //if stalemate, return 0
    if(stalemate(someBoard,aiTurn))
        return 0;
    //AI won by checkmate, return best possible value
    if(whiteincheckmate(someBoard))
        return best;
    //AI lost by checkmate, return worst possible value
    if(blackincheckmate(someBoard))
        return -best;
    return evaluation; //return evaluation of board
}

//this function checks if there is stalemate at a given board posistion
bool stalemate(TPiece someBoard[8][8], bool aiTurn)
{
    //if white's move, not in check and no legal moves, return true
    if(aiTurn == false and numMovesWhite(someBoard) == 0 and CheckForCheck(30)==false)
        return true;
    //if black's move, not in check and no legal moves, return true
    if(aiTurn == true and numMovesBlack(someBoard) == 0 and CheckForCheck(31)==false)
        return true;
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
            if(someBoard[x][y]==blackpawn) numPawns++; //pawn
            else if(someBoard[x][y]==whitepawn) numPawns++; //pawn
            else if(someBoard[x][y]==blackknight) numKnightsB++; //black knight
            else if(someBoard[x][y]==whitekinght) numKnightsW++; //white knight
            else if(someBoard[x][y]==blackbishop) numBishopsB++; //black bishop
            else if(someBoard[x][y]==whitebishop) numBishopsW++; //white bishop
            else if(someBoard[x][y]==blackrook) numRooks++; //rook
            else if(someBoard[x][y]==whiterook) numRooks++; //rook
            else if(someBoard[x][y]==blackqueen) numQueens++; //queen
            else if(someBoard[x][y]==whitequeen) numQueens++; //queen
        }
    }
    //checking for stalemate by lack of material
    if(numPawns==0 and numRooks==0 and numQueens==0) //if pawns, rooks or queens, stalemate
    {
        //if 2 knights or less and no bishops, stalemate
        if(numknightsB<=2 and numKnightsW<=2 and numBishopsB==0 and numBishopsW==0) return true;
        //if less than 2 bishops and no knights, stalemate (note: we are not allowing promoting to a bishop)
        if(numBishopsB<2 and numBishopsW<2 and numKnightsW==0 and numKnightsB==0) return true;
    }
    if(posX3()) return true; //return true if a posistion has occured 3 time
    if(stalemate50==50) return true; //return true if 50 move rule has been reached
    return false; //return false
}

//function to see if white is checkmated
bool whiteincheckmate(someBoard)
{
    //if white's turn, white in check and no legal moves, return true
    if(WhiteTurn == true and numMovesWhite(someBoard) == 0 and CheckForCheck(30) == true)
        return true;
    return false; //return false if checkmate hasn't occured
}

//function to see if black is checkmated
bool blackincheckmate(someBoard)
{
    //if blacks's turn, black in check and no legal moves, return true
    if(WhiteTurn == false and numMovesBlack(someBoard) == 0 and CheckForCheck(31) == true)
        return true;
    return false;  //return false if checkmate hasn't occured
}
