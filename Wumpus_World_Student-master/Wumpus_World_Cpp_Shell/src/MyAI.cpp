// ======================================================================
// FILE:        MyAI.cpp
//
// AUTHOR:      Abdullah Younis
//
// DESCRIPTION: This file contains your agent class, which you will
//              implement. You are responsible for implementing the
//              'getAction' function and any helper methods you feel you
//              need.
//
// NOTES:       - If you are having trouble understanding how the shell
//                works, look at the other parts of the code, as well as
//                the documentation.
//
//              - You are only allowed to make changes to this portion of
//                the code. Any changes to other portions of the code will
//                be lost when the tournament runs your code.
// ======================================================================

#include "MyAI.hpp"
#include <algorithm>
#include <iostream>


using namespace std;
MyAI::MyAI() : Agent()
{
    //meta info
    movesInAdvance = 6;
    totalMoves = 0;
    currentScore = 0;
    wumpusKilled = false;
    goldGrabbed = false;

    //position info
    orientation = EAST;
    currentxValue = 1;
    currentyValue = 1;

    //wall and boundary info
    topWall = -1;
    rightWall = -1;

    //backtracking info
    backTrackingOn = false;
    turningAround = false;
    turningAroundComplete = false;
    trail.push(pair<int,int>(1,1));
}

void MyAI::handlePositionChange()
{
    if (orientation == NORTH)
        currentyValue += 1;
    else if (orientation == SOUTH)
        currentyValue -= 1;
    else if (orientation == EAST)
        currentxValue += 1;
    else if (orientation == WEST)
        currentxValue -= 1;
}

void MyAI::handleBump()
{
    if (orientation == EAST)
    {
        currentxValue -= 1;
        rightWall = currentxValue;
    }
    else if (orientation == NORTH)
    {
        currentyValue -= 1;
        topWall = currentyValue;
    }
}

template <typename T1>
void removeFromVector(std::vector<T1> & toRemoveFrom, T1 toRemove)
{
    for (typename std::vector<T1>::iterator beg = toRemoveFrom.begin(); beg != toRemoveFrom.end(); ++beg)
        if (*beg == toRemove)
        {
            toRemoveFrom.erase(beg); 
            return;
        }
}

vector<MyAI::direction> MyAI::determineWalls(int currentX, int currentY)
{
    vector<MyAI::direction> availableDirections;
    availableDirections.push_back(WEST);
    availableDirections.push_back(EAST);
    availableDirections.push_back(NORTH);
    availableDirections.push_back(SOUTH);

    if (currentX == 1)
        removeFromVector(availableDirections, WEST);

    if (currentY == 1)
        removeFromVector(availableDirections, SOUTH);

    if (currentX == rightWall)
        removeFromVector(availableDirections, EAST);

    if (currentY == topWall)
        removeFromVector(availableDirections, NORTH);
    
    if (!backTrackingOn && trail.size() > 0)
    {
        pair<int,int> previousSquare = trail.top();
        removeFromVector(availableDirections,determineBackTrackDirection(previousSquare.first,previousSquare.second));
    }
    
    return availableDirections;
}

bool MyAI::tileExist(const int &x, const int &y)
{
    map<int, std::map<int, tile>>::iterator iter = worldMap.find(x);
    if (iter == worldMap.end()) return false;
    map<int, tile>::iterator iter2 = worldMap[x].find(y);
    return iter2 != worldMap[x].end() ? true : false;
}

void MyAI::addNewTile(bool glitter,bool stench,bool breeze)
{
    if (!tileExist(currentxValue, currentyValue))
    {
        tile currentTile;
        currentTile.glitter = glitter;
        currentTile.stench = stench;
        currentTile.breeze = breeze;
        worldMap[currentxValue][currentyValue] = currentTile;
    }
}


MyAI::direction MyAI::resolveNewOrientation(const MyAI::direction & orientation,const Agent::Action & action)
{
    MyAI::direction newOrientation;
    if (action == TURN_RIGHT)
        newOrientation = (MyAI::direction)((orientation + 1) % 4);
    else if (action == TURN_LEFT && (orientation - 1) >= 0)
        newOrientation = (MyAI::direction)((orientation - 1) % 4);
    else if (action == TURN_LEFT && (orientation - 1) < 0)
        newOrientation = (MyAI::direction)((4 + (orientation - 1)) % 4);
    return newOrientation;
}

Agent::Action MyAI::adjustDirection(const MyAI::direction & orientation,const MyAI::direction & chosenDirection)
{
    int numRotation1 = 0;
    for (int i = 1; i < 4; ++i)
    {
        ++numRotation1;
        if (((orientation + i) % 4) == chosenDirection)
            break;
    }
    int numRotation2 = 0;
    for (int i = 1; i < 4; ++i)
    {
        ++numRotation2;
        if ((orientation - i) >= 0 &&((orientation - i) % 4) == chosenDirection)
            break;
        else if ((orientation - i) < 0 &&( (4 + (orientation - i)) % 4) == chosenDirection)
            break;
    }
    Agent::Action toTurn;
    if (numRotation1 > numRotation2)
        toTurn = TURN_LEFT;
    else   
        toTurn = TURN_RIGHT;
    return toTurn;
}


void MyAI::createSequenceOfAction(const MyAI::direction & chosenDirection)
{
    MyAI::direction mockOrientation = orientation;
    while (mockOrientation != chosenDirection)
    {
       Agent::Action action = adjustDirection(mockOrientation,chosenDirection);
       sequenceOfActions.push(action);  
       mockOrientation = resolveNewOrientation(mockOrientation, action);
    }
    sequenceOfActions.push(FORWARD);
}

Agent::Action MyAI::Turn_Left(){
    orientation = resolveNewOrientation(orientation,TURN_LEFT);
    return TURN_LEFT;
}

Agent::Action MyAI::Turn_Right(){
    orientation = resolveNewOrientation(orientation,TURN_RIGHT);
    return TURN_RIGHT;
}

Agent::Action MyAI::Forward(){
    if (!backTrackingOn)
        trail.push(pair<int,int>(currentxValue,currentyValue));
    handlePositionChange();
    return FORWARD;
}

Agent::Action MyAI::resolveAction(Agent::Action action)
{
    if (action == TURN_LEFT)
        return Turn_Left();
    else if (action == TURN_RIGHT)
        return Turn_Right();
    else
        return Forward();    
}

MyAI::direction MyAI::determineBackTrackDirection(const int & destinationxValue,const int & destinationyValue)
{
    if(currentxValue == destinationxValue)
    {
       if(currentyValue > destinationyValue)
          return SOUTH;
       else
          return NORTH;
    }
    else //case where currentyValue == destinationyValue
    {
       if(currentxValue > destinationxValue)
          return WEST;
       else
          return EAST;
    }
}

void MyAI::backtrackAction()
{
   pair<int,int> destinationTile = trail.top();
   trail.pop();
   MyAI::direction chosenBackTrackDir = determineBackTrackDirection(destinationTile.first,destinationTile.second);
//    std::cout << "Chosen BkTrack Dir " << chosenBackTrackDir << std::endl; 
   createSequenceOfAction(chosenBackTrackDir);  
}

//main method
Agent::Action MyAI::getAction(bool stench, bool breeze, bool glitter, bool bump, bool scream)
{
    //adding tile
    addNewTile(glitter,stench,breeze);
    tile currentTile = worldMap[currentxValue][currentyValue];
    
    // std::cout << "Position " << currentxValue << " , " << currentyValue << std::endl;
    // std::cout << "Orientation " << orientation << std::endl;
    // std::cout << "BacktrackingOn " << backTrackingOn << std::endl;
    // std::cout << "Trail Size " << trail.size() << std::endl;
    // std::cout << "Sequence of Action Size " << sequenceOfActions.size() << std::endl;

    //updates wall location information
    if (bump)
        handleBump();

   
    //***************************************************Action Logic***************************************************
    //continues to resolve desired direction if needed
    if (sequenceOfActions.size() != 0)
    {
        Agent::Action action = sequenceOfActions.front();
        sequenceOfActions.pop();
       return resolveAction(action);
    }  

    //always check tile for gold and grab if possible
    if (currentTile.glitter && !goldGrabbed)
    {
       currentScore += 1000;
       goldGrabbed = true;
       backTrackingOn = true;
       return GRAB;
    }

    //handle backtracking decision
    if (backTrackingOn && (currentxValue != 1 || currentyValue != 1))
    {
        backtrackAction();
        Agent::Action action = sequenceOfActions.front();
        sequenceOfActions.pop();
        return resolveAction(action);
    }
    else if (backTrackingOn && currentxValue == 1 && currentyValue == 1)
        return CLIMB;

    //BASE CASE 1: Leaving: you leave when you have the gold or you sense danger
    if (currentTile.breeze == true || currentTile.stench == true) 
    {
        if (trail.size() == 1 && currentxValue == 1 && currentyValue == 1)
            return CLIMB;
        else
        {
           backTrackingOn = true;
           backtrackAction();
           Agent::Action action = sequenceOfActions.front();
           sequenceOfActions.pop();
           return resolveAction(action);
        }
    }
    //BASE CASE 2: No danger, move forward in a random direction
    else 
    {
        vector<MyAI::direction> availableDirections = determineWalls(currentxValue, currentyValue);
        int randomIndex = rand() % availableDirections.size();
        MyAI::direction chosenDirection = availableDirections[randomIndex];
        createSequenceOfAction(chosenDirection);
        // std::cout << "Chosen Direction " << chosenDirection << std::endl;
        Agent::Action action = sequenceOfActions.front();
        sequenceOfActions.pop();
       return resolveAction(action);
    }   
}