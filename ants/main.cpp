//
//  main.cpp
//  ants
//
//  Created by Kevin Faulhaber on 24/03/2018.
//  Copyright © 2018 Julia. All rights reserved.
//

#include <iostream>
#include <vector>
#include <fstream>
#include <chrono>
#include <thread>

const int MAX_ROWS = 20;
const int MAX_COLS = MAX_ROWS;

const int NEST_START_ROW = MAX_ROWS * 0.4;
const int NEST_END_ROW = MAX_ROWS * 0.6;
const int NEST_START_COL = NEST_START_ROW;
const int NEST_END_COL = NEST_END_ROW;




struct Ant {
    bool hasSugar;
    
    bool chercheSucre(){
        return !hasSugar;
    }
    
    bool rentreNid(){
        return hasSugar;
    }
};

struct Nest {
    
};

struct Sugar {
    
};

struct Cell {
    int row;
    int col;
    
    //peramone sugar level
    double psl;
    //peramopne nest level
    double pnl;
    
    bool handled;
    std::vector<Cell*> *neighbors;
    
    //Only one of these three may be active at once.
    Ant* ant;
    Nest* nest;
    Sugar* sugar;
    
    bool contienSucre(){
        //TODO: Figure out how much surgar a Struct Sugar contains
        return sugar != NULL;
    }
    
    bool contientNid(){
        return nest != NULL;
    }
    
    bool contientFourmi(){
        return ant != NULL;
    }
    
    bool vide(){
        return !contienSucre() and !contientNid() and !contientFourmi();
    }
    
    bool surUnePiste(){
        return psl > 0;
    }
    
    bool plusProcheNid(Cell* c){
        return pnl >= c->pnl;
    }
    
    bool plusLoinNid(Cell* c){
        return pnl < c->pnl;
    }
};

void update(){
    
}

void moveCursor(std::ostream& os, int col, int row)
{
    //os << "\033[" << col << ";" << row << "H";
}

void draw(){
    moveCursor(std::cout, 10, 10);
    
    std::cout << "hey";
}



int milisecondsBeforeIteration = 1000;
void gameLoop(){
    std::cout << "starting game";
    while(true){
        std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
        update();
        draw();
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        auto mili = milisecondsBeforeIteration-std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        if(mili > 0) std::this_thread::sleep_for(std::chrono::milliseconds(mili));
    }
    return;
}



Cell* c0;

Cell* newCell(int row, int col, bool state = false){
    Cell* temp = new Cell();
    temp->row = row;
    temp->col = col;
    temp->handled = state;
    //TODO: this can be precalculated, since we already know the location of the nest.
    temp->pnl = 0;
    temp->psl = 0;
    //new returns a pointer
    temp->neighbors = new std::vector<Cell *>();
    return temp;
}

/* Recursive Method that checks if current is a neighbor of newCell, and links them together. */
void linkToNeighbors(Cell* newCell, Cell* current, bool state){
    
    //a new cell can only have a max of 8 neighbors
    if(newCell->neighbors->size() == 8) return;
    //Nothing to do if the current cell has already been handled
    if(current->handled == state) return;
    current->handled = state;
    //Nothing to do if both cells are the same
    if(newCell->row == current->row and newCell->col == current->col) return;
    //Nothing to do if they are not neighbors
    //Impossible for both abs return values to be 0, since we already checked that they are not the same cells.
    if(abs(newCell->row - current->row) <= 1 and abs(newCell->col - current->col) <= 1){
        //Arriving here we can assume that they are neighbors.
        newCell->neighbors->push_back(current);
        current->neighbors->push_back(newCell);
    }
    //Loop through all neighbors
    for(auto &ref : *current->neighbors){
        linkToNeighbors(newCell, ref, state);
    }
    
}
void setup(){
    c0 = newCell(0, 0);
    
    for(int row = 0; row < MAX_ROWS; row++){
        for(int col = 0; col < MAX_COLS; col++){
            //This condition refers to c0 that we have already handled.
            if(row == 0 and col == 0) continue;
            Cell* temp = newCell(row, col, c0->handled);
            linkToNeighbors(temp, c0, !c0->handled);
        }
    }
}

/* function to test and see if our setup() and linkToNeighbors funciton worked correctly */
bool setup_test(Cell* current = c0, bool state = c0->handled, std::vector<Cell*> *done = new std::vector<Cell*>()){
    
    //Four possible types: Corner = 3 neighbors, Middle = 8 neighbors, Side = 5 neighbors
    
    bool result = true;
    done->push_back(current);
    
    int expectedSize = -1;
    int rowModulo = current->row % (MAX_ROWS-1);
    int colModulo = current->col % (MAX_COLS-1);
    //Here we can assume it's a corner
    if(rowModulo == 0 and colModulo == 0){
        result = current->neighbors->size() == 3;
        expectedSize = 3;
    } else
        if(rowModulo == 0 or colModulo == 0){
            //we can assume it's a side
            result = current->neighbors->size() == 5;
            expectedSize =5;
        } else {
            result = current->neighbors->size() == 8;
            expectedSize =8;
        }
    
    if(!result){
        std::cout << "row: " << current->row << ", col: " << current->col << " Wrong number of neighbors, expected: " << expectedSize << ", but got " << current->neighbors->size() << "\n";
    }
    
    //Check to see if the state corresponds
    result = state == current->handled;
    if(!result){
        std::cout << "wrong handled bool";
        return result;
    }
    
    for(auto &ref : *current->neighbors){
        std::vector<Cell*>::iterator it = std::find(done->begin(), done->end(), current);
        //cell has already been handled
        if(it != done->end()) continue;
        result = setup_test(ref, state, done);
        if(!result) return result;
    }
    
    return result;
}

int main(int argc, const char * argv[]) {
    std::cout << "in";
    gameLoop();
    //setup();
    //std::cout << (setup_test() ? "setup_test completed with no errors\n": "setup_test completed with errors\n");
    return 0;
}
