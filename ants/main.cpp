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
#include <random>

/* Define structures here */
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

struct Position {
    int row;
    int col;
    bool operator == (const Position &p2) const
    {
        return this->row == p2.row and this->col == p2.col;
    }
};

struct Cell {
    int row;
    int col;
    std::string symbol;
    
    //peramone sugar level
    int psl;
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

std::random_device rd;     // only used once to initialise (seed) engine
int randomNumber(int min, int max){
    std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)
    std::uniform_int_distribution<int> uni(min,max); // guaranteed unbiased
    return uni(rng);
}

class GameController {
    
    /* Define Constants here */
    const int MAX_ROWS = 20;
    const int MAX_COLS = MAX_ROWS;
    const int MILI_BEFORE_ITERATION = 1000;
    const int ROW_DRAW_COEFF = 2;
    const int COL_DRAW_COEFF = 4;
    const double NEST_DECIMAL_LOC_MIN = 0.4;
    const double NEST_DECIMAL_LOC_MAX = 0.6;
    const int MAX_SUGARS = 4;
    
    
private:
    Cell* c0;
private:
    std::vector<Cell*> toDraw;
private:
    std::vector<Position> randomSugarPositions;
    
private:
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

private:
    /*
     
     +---+---+---+
     | F |   |   |
     +---+---+---+
     |   |   |   |
     +---+---+---+
     |   |   |   |
     +---+---+---+
     
     */
    void drawMap(){
        //clear anything in terminal before drawing
        std::cout << "\033[2J\033[1;1H";

        for(int row = 0; row <= MAX_ROWS*ROW_DRAW_COEFF; row++){
            int rowModulo = row % ROW_DRAW_COEFF;
            for(int col = 0; col <= MAX_COLS*COL_DRAW_COEFF; col++){
                std::string character;
                int colModulo = col % COL_DRAW_COEFF;
                
                //TODO: Can be optimized, but not important since this is run only once... before the start of the loop.
                if(colModulo == 0){
                    if(rowModulo == 0){
                        character = "+";
                    } else{
                        character = "|";
                    }
                }else{
                    if(rowModulo == 0){
                        character = "-";
                    }else{
                        character = " ";
                    }
                }
                
                this->draw(row, col, character);
            };
        }
    }
    
private:
    void moveCursor(std::ostream& os, int col, int row)
    {
        os << "\033[" << col << ";" << row << "H";
    }

private:
    void draw(int row, int col, std::string character){
        //adding +1 because terminal position starts at 1,1 and not 0,0.
        moveCursor(std::cout, row+1, col+1);
        std::cout << character << std::endl;
    }
private:
    void drawCells(){
        for(auto &cell : this->toDraw){
            this->draw(cell->row*ROW_DRAW_COEFF+(ROW_DRAW_COEFF/2), cell->col*COL_DRAW_COEFF+(COL_DRAW_COEFF/2), cell->symbol);
        }
    }
    
private:
    void update(){
        this->toDraw.clear();
        this->recursiveUpdate(this->c0, !this->c0->handled);
        //this->log("Update called");
    }
    
    void recursiveUpdate(Cell* current, bool state){
        //Nothing to do if the current cell has already been handled
        if(current->handled == state) return;
        current->handled = state;
        
        if(current->psl > 0) current->psl -= 5;
        
        //Not needed since S is predefined. if(current->contienSucre()) current->symbol = "S";
        if(current->contientFourmi()){
            if(current->ant->chercheSucre()){
                /*int randomStart = randomNumber(0, current->neighbors->size()-1);
                while(current->neighbors->at(randomStart)->){
                    
                }*/
                current->symbol = "f";
            }else{
                current->symbol = "F";
            }
            //We know ants move all the time so add it to the draw.
            this->toDraw.push_back(current);
        }

        //Nothing to do if it's a nest, since it's only drawn once.
        
    }
private:
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
private:
    bool isInitNest(int row, int col){
        return
            row > MAX_ROWS*NEST_DECIMAL_LOC_MIN and
            row < MAX_ROWS*NEST_DECIMAL_LOC_MAX-1 and
            col > MAX_COLS*NEST_DECIMAL_LOC_MIN and
            col < MAX_COLS*NEST_DECIMAL_LOC_MAX-1;
    }
private:
    bool isInitAnt(int row, int col){
        return !this->isInitNest(row, col) and this->isCenterCell(row,col);
        
    }
private:
    bool isCenterCell(int row, int col){
        return
        row > MAX_ROWS*NEST_DECIMAL_LOC_MIN-1 and
        row < MAX_ROWS*NEST_DECIMAL_LOC_MAX and
        col > MAX_COLS*NEST_DECIMAL_LOC_MIN-1 and
        col < MAX_COLS*NEST_DECIMAL_LOC_MAX;
    }
    
private:
    bool isSugar(Cell* cell){
        Position p;
        p.row = cell->row;
        p.col = cell->col;
        return std::find(this->randomSugarPositions.begin(), this->randomSugarPositions.end(), p) != this->randomSugarPositions.end();
    }

private:
    void setPnl(Cell* cell, double maxDistanceFromCenter){
        cell->pnl = (1 - sqrt(
         pow((cell->row-(MAX_ROWS/2)), 2)+pow((cell->row-(MAX_COLS/2)), 2)
         ))/maxDistanceFromCenter;
    }
    
public:
    void setup(){
        double maxDistanceFromCenter = sqrt(
            (pow(MAX_ROWS,2) + pow(MAX_COLS,2))
        )/2;
        this->c0 = this->newCell(0, 0);
        this->generateRandomSugarPositions();
        for(int row = 0; row < MAX_ROWS; row++){
            for(int col = 0; col < MAX_COLS; col++){
                //This condition refers to c0 that we have already handled.
                if(row == 0 and col == 0) continue;
                Cell* temp = this->newCell(row, col, c0->handled);
                
                this->setPnl(temp, maxDistanceFromCenter);
                
                //Check to see what kind of cell it is.
                if(this->isInitNest(row, col)){
                    //this->log("Added to nest");
                    //Guaranteed a nest
                    temp->nest = new Nest();
                    temp->symbol = "N";
                    this->toDraw.push_back(temp);
                }
                else if(this->isInitAnt(row, col)){
                    //guaranteed an ant
                    temp->ant = new Ant();
                    temp->symbol = "f";
                    this->toDraw.push_back(temp);
                }
                else if(this->isSugar(temp)){
                    temp->sugar = new Sugar();
                    temp->symbol = "S";
                    this->toDraw.push_back(temp);
                }
                
                this->linkToNeighbors(temp, c0, !c0->handled);
            }
        }
    }
private:
    void log(std::string message){
        this->draw(MAX_ROWS*ROW_DRAW_COEFF+1, 0, message);
    }
public:
    void start(){
        this->drawMap();
        this->drawCells();
        
        //Simulationn starts here.
        this->loop();
    }
    
private:
    void loop(){
        while(true){
            //Recording draw time here
            std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
            
            this->update();
            this->drawCells();
            
            //Calculate how long it took to update and draw
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            auto mili = MILI_BEFORE_ITERATION-std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            //Make the thread sleep the difference so we iterate every MILI_BEFORE_ITERATION milisconds.
            if(mili > 0) std::this_thread::sleep_for(std::chrono::milliseconds(mili));
        }
    }
private:
    void generateRandomSugarPositions(){
        //taken from https://stackoverflow.com/a/19728404/3589092
        std::random_device rd;     // only used once to initialise (seed) engine
        std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)
        std::uniform_int_distribution<int> uniRow(0,MAX_ROWS-1); // guaranteed unbiased
        std::uniform_int_distribution<int> uniCol(0,MAX_COLS-1); // guaranteed unbiased

        while(this->randomSugarPositions.size() < MAX_SUGARS){
            int row = uniRow(rng);
            int col = uniCol(rng);
            
            Position p;
            p.row = row;
            p.col = col;
            
            //Nothing to do if center cell... or already a sugar cell
            if(this->isCenterCell(row, col) and std::find(this->randomSugarPositions.begin(), this->randomSugarPositions.end(), p) != this->randomSugarPositions.end()){
                continue;
            }
            this->randomSugarPositions.push_back(p);
        }
    }
};



/* function to test and see if our setup() and linkToNeighbors funciton worked correctly */
/*bool setup_test(Cell* current = c0, bool state = c0->handled, std::vector<Cell*> *done = new std::vector<Cell*>()){
    
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
}*/

int main(int argc, const char * argv[]) {
    GameController* gc = new GameController();
    gc->setup();
    gc->start();
    //std::cout << (setup_test() ? "setup_test completed with no errors\n": "setup_test completed with errors\n");
    return 0;
}
