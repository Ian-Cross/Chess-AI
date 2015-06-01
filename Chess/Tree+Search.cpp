#include <iostream>
#include <vector>

using namespace std;

const int best = 1500;

struct AiMovementTree
{
    int value;
    TPiece board[8][8];
    vector <AiMovementTree> FutureMoves;
};


//recursive function that return the best possible move
AiMovementTree bestMove(AiMovementTree *node, int depth, bool aiTurn)
{
    findPossibleMoves(board); //calls the funtcion that create variable possibleMoves
    if(depth<=7) //if the max depth hasn't been reached, then add moves to tree
    {
        //add nodes to the tree
        for(int x = 0; x < possibleMoves.size(); x++)
        {
            AiMovementTree *newNode = new AiMovementTree;
            newNode->board=possibleMoves[x];
            node->FutureMoves.push_back(newNode);
        }
    }
    depth++; //increase the depth
    //if the node still has children, call this function for each of them
    if(node->FutureMoves.size()!=0)
    {
        for(int x = 0; x < node->FutureMoves.size(); x++)
            node->FutureMoves[x]=bestMove(node->FutureMoves[x],depth,!aiTurn); //recursively searching the tree
    }
    else //if the node has no children, find a value for its board
    {
        node->value=evaluate(node->board);
        return node; //return node, since no children
    }
    if(aiTurn) //if it's the ai's turn
    {
        node->value=-best; //set best value to the worst possible
        //look through all the moves associated the the node
        for(int x = 0; x < node->FutureMoves.size(); x++)
        {
            //keep the highest value
            if(node->FutureMoves[x]->value > node->value)
                node->value = node->FutureMoves[x]->value;
            AiMovementTree *temp; //temp node
            temp = node->FutureMoves[x];
            delete temp; //delete move
        }
        //remove all children from the node
        for(int x = 0; x < node->FutureMoves.size(); x++)
            node->FutureMoves.pop_back();
    }
    else //if it is not the ai's turn
    {
        node->value=best; //set value to best
        for(int x = 0; x < node->FutureMoves.size(); x++)
        {
            //keep the lowest possible value
            if(node->FutureMoves[x]->value < node->value)
                node->value = node->FutureMoves[x]->value;
            AiMovementTree *temp; //temp node
            temp = node->FutureMoves[x];
            delete temp;//delete move
        }
        //remove all the children from the node
        for(int x = 0; x < node->FutureMoves.size(); x++)
            node->FutureMoves.pop_back();
    }
    return node; //return node
}
