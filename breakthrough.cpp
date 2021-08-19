/**
 * Breakthrough
 *
 * This
 */

#include <chrono>
#include <iomanip>
#include <iostream>
#include <vector>
#include <locale>
#include <math.h>
#include <climits>
#include <sylvan_obj.hpp>

#include <assert.h>
#include <utility>
#include <tuple>

using namespace std;
using namespace std::chrono;
using namespace sylvan;

enum Square {
    EE = 0, // Empty square
    WW = 1, // White Pawn
    BB = 2, // Black Pawn
};

// n columns, m rows

/* 5x5 */
// #define M 5
// #define N 5
// Square BOARD[M][N] = {
//     {WW,WW,WW,WW,WW,},
//     {WW,WW,WW,WW,WW,},
//     {EE,EE,EE,EE,EE,},
//     {BB,BB,BB,BB,BB,},
//     {BB,BB,BB,BB,BB,},
// };

/* 5x4 */
#define M 5
#define N 4
Square BOARD[M][N] = {
    {WW,WW,WW,WW,},
    {WW,WW,WW,WW,},
    {EE,EE,EE,EE,},
    {BB,BB,BB,BB,},
    {BB,BB,BB,BB,},
};

/* 6x4 */
// #define M 6
// #define N 4
// Square BOARD[M][N] = {
//     {WW,WW,WW,WW,},
//     {WW,WW,WW,WW,},
//     {EE,EE,EE,EE,},
//     {EE,EE,EE,EE,},
//     {BB,BB,BB,BB,},
//     {BB,BB,BB,BB,},
// };

/* 5x6 */
// #define M 5
// #define N 6
// Square BOARD[M][N] = {
//     {WW,WW,WW,WW,WW,WW,},
//     {WW,WW,WW,WW,WW,WW,},
//     {EE,EE,EE,EE,EE,EE,},
//     {BB,BB,BB,BB,BB,BB,},
//     {BB,BB,BB,BB,BB,BB,},
// };

/* 7x3 */
// #define M 7
// #define N 3
// Square BOARD[M][N] = {
//     {WW,WW,WW,},
//     {WW,WW,WW,},
//     {EE,EE,EE,},
//     {EE,EE,EE,},
//     {EE,EE,EE,},
//     {BB,BB,BB,},
//     {BB,BB,BB,},
// };



/**
 * Declare callback function for printing boards from BDD package
 */
VOID_TASK_DECL_4(printboard, void*, BDDVAR*, uint8_t*, int);
#define printboard TASK(printboard)


struct PartialRelation {
    Bdd MoveRelation;
    BddSet RelationVars; // unprimed, primed
};

class Breakthrough {

public:
    // BDD variables:
    BddSet SourceVars;   	// unprimed
    BddSet TargetVars;   	// primed
    BddSet RelationVars; 	// unprimed, primed
    BDDVAR player = M*N*4;	// Bdd(player) -- white turn, !Bdd(player) -- black turn

    // Mapping from MxN square coordinates to resp. (unprimed) BDD variable:
    BDDVAR Coor2Var[M][N];
    pair<int, int> Var2Coor[M*N*4];

    // Vector of connected squares, used for CreateMoveRelation
    vector<tuple<BDDVAR, BDDVAR>> connected;

    // BDDs representing sets of boards:
    Bdd InitialBoard;
    Bdd WinningBoardsWhite;
    Bdd WinningBoardsBlack;
    Bdd WhiteHitAll;
    Bdd BlackHitAll;

    // BDD representing the move relation:
    Bdd MoveRelation;
    // BDDs representing partial (local) move relations (over a 'quad' of squares)
    vector<PartialRelation>  Relations;

    /* Number existing squares and init the variable sets */
    Breakthrough() {
    	BDDVAR var = 0;
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < N; j++) {
                BDDVAR source = var++;   // unprimed (source square in relation)
                BDDVAR target = var++; 	 // primed (target square in relation)
                BDDVAR source2 = var++;	 // unprimed (source square2 in relation)
                BDDVAR target2 = var++;  // primed (target square2 in relation)

                Coor2Var[i][j] = source; // only remember source (target is +1, source2 is +2, target2 is +3)
                Var2Coor[source] = {i,j};

                SourceVars.add(source);
                SourceVars.add(source2);
                TargetVars.add(target);
                TargetVars.add(target2);
            }
        }
        // Add the player bit, source and target (+1)
        SourceVars.add(player);
        TargetVars.add(player+1);

        // All relational variables:
        RelationVars.add(SourceVars);
        RelationVars.add(TargetVars);

        for (int i = 0; i < M; i++) {
        	for (int j = 0; j < N; j++) {
        		// Add across left
        		if (j > 0 && i < M-1) {
        			connected.push_back(make_tuple(Coor2Var[i][j], Coor2Var[i+1][j-1]));
        		}
        		// Add across
        		if (i < M-1) {
        			connected.push_back(make_tuple(Coor2Var[i][j], Coor2Var[i+1][j]));
        		}
        		// Add across right
        		if (j < N-1 && i < M-1) {
        			connected.push_back(make_tuple(Coor2Var[i][j], Coor2Var[i+1][j+1]));
        		}
        	}
        }
    }

    void CreateInitialBoard() {
        InitialBoard = Bdd::bddOne();  // true BDD

        // iterate over all existing squares
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < N; j++) {
                BDDVAR square = Coor2Var[i][j];
                BDDVAR square2 = square + 2;
                if (BOARD[i][j] == EE) { // empty
                    InitialBoard &= (!Bdd(square)) & (!Bdd(square2));
                } else if (BOARD[i][j] == WW) {
                    InitialBoard &= (!Bdd(square)) & Bdd(square2);
                } else if (BOARD[i][j] == BB) {
                	InitialBoard &= Bdd(square) & (!Bdd(square2));
                }
            }
        }
        InitialBoard = InitialBoard & Bdd(player); // White starts
    }

    void CreateWinningBoardsWhite() {
    	// Winning White boards have a white piece on the back rank (i = M-1)
        WinningBoardsWhite = Bdd::bddZero(); // false BDD (empty set)
        Bdd BlackOnOwnBackRank = Bdd::bddZero();
        Bdd NoBlackStones = Bdd::bddOne();

        // If there are no longer any black stones, you've won
        for (int i = 0; i < M; i++) {
        	for (int j = 0; j < N; j++) {
        		BDDVAR square = Coor2Var[i][j];
        		BDDVAR square2 = square + 2;
        		NoBlackStones &= !(Bdd(square) & (!Bdd(square2)));
        	}
        }
        WhiteHitAll = NoBlackStones;
        WhiteHitAll &= (!Bdd(player));

        // Winning board if a white stone is on the second-last rank
        for (int j = 0; j < N; j++) {
			BDDVAR square = Coor2Var[M-2][j];
			BDDVAR square2 = square + 2;
			WinningBoardsWhite |= ((!Bdd(square)) & Bdd(square2));
		}

        /* Niet nodig, want alle onhaalbare winningboards worden nog verwijdert 
         * Toch wel want het verkleint WinningBoardsWhite erg :) 
         */
        // Catch all (1,1) entries, they aren't reachable
		for (int i = 0; i < M; i++) {
			for (int j = 0; j < N; j++) {
				BDDVAR square = Coor2Var[i][j];
				BDDVAR square2 = square + 2;
				WinningBoardsWhite &= !(Bdd(square) & Bdd(square2));
			}
		}

        // No black stone is allowed on your own back rank
        for (int j = 0; j < N; j++) {
        	BDDVAR square = Coor2Var[0][j];
        	BDDVAR square2 = square + 2;
        	BlackOnOwnBackRank |= (Bdd(square) & (!Bdd(square2)));
        }
        WinningBoardsWhite &= !BlackOnOwnBackRank;

        // If White can win, it's White's turn
        WinningBoardsWhite &= Bdd(player);
    }

    void CreateWinningBoardsBlack() {
		// Winning Black boards have a black piece on the back rank (i = 0)
		WinningBoardsBlack = Bdd::bddZero(); // false BDD (empty set)
		Bdd WhiteOnOwnBackRank = Bdd::bddZero();
		Bdd NoWhiteStones = Bdd::bddOne();

		// If there are no longer any white stones, you've won
		for (int i = 0; i < M; i++) {
			for (int j = 0; j < N; j++) {
				BDDVAR square = Coor2Var[i][j];
				BDDVAR square2 = square + 2;
				NoWhiteStones &= !((!Bdd(square)) & Bdd(square2)); //
			}
		}
		BlackHitAll = NoWhiteStones;
        BlackHitAll &= Bdd(player);

		// Winning board if a black stone is on the back rank
		for (int j = 0; j < N; j++) {
			BDDVAR square = Coor2Var[1][j];
			BDDVAR square2 = square + 2;
			WinningBoardsBlack |= (Bdd(square) & (!Bdd(square2)));
		}

		// If Black won, it's now White's turn
		WinningBoardsBlack &= (!Bdd(player));
	}

	/*
	Bdd(V[0]  ) is the source  square
	Bdd(V[0]+1) is the target  square
	Bdd(V[0]+2) is the source2 square
	Bdd(V[0]+3) is the target2 square

	   source2/target2
	   | 0  |  1
	-------------
	0  | EE |  WW
	1  | BB |  --
	^
	|
	---- source1/target1
	*/
    // todo:    when going left/right across it just checks that it cant take the place of your stone.
    //          maybe it's better to check that it should be opponent stone or empty, now 
    //          it can be a "dead" cell.
    void CreateMoveRelation() {
    	for (tuple<BDDVAR, BDDVAR> p : connected) {
    		// Add variables to partial relation:
			PartialRelation Partial;

			BDDVAR var = get<0>(p);
			Partial.RelationVars.add(var);		// source
			Partial.RelationVars.add(var+1);	// target
			Partial.RelationVars.add(var+2);	// source2
			Partial.RelationVars.add(var+3);	// target2

			BDDVAR var2 = get<1>(p);
			Partial.RelationVars.add(var2);		// source
			Partial.RelationVars.add(var2+1);	// target
			Partial.RelationVars.add(var2+2);	// source2
			Partial.RelationVars.add(var2+3);	// target

			Partial.RelationVars.add(player);	// unprimed
			Partial.RelationVars.add(player+1);	// primed

			Partial.MoveRelation = Bdd::bddZero();

			int j1 = Var2Coor[get<0>(p)].second;
			int j2 = Var2Coor[get<1>(p)].second;

			// add player
			Bdd White = Bdd(player) & (!Bdd(player+1));
			Bdd Black = (!Bdd(player)) & Bdd(player+1);



			if (j1 == j2) {
				// straigt across -- get<1>(p) has to be empty
				// white
				Partial.MoveRelation |= (!Bdd(get<0>(p))) & Bdd(get<0>(p)+2)		// old 0 - white
									  & (!Bdd(get<1>(p))) & (!Bdd(get<1>(p)+2))		// old 1 - empty
									  & (!Bdd(get<0>(p)+1)) & (!Bdd(get<0>(p)+3))	// new 0 - empty
									  & (!Bdd(get<1>(p)+1)) & Bdd(get<1>(p)+3) & White;		// new 1 - white
				// black
				Partial.MoveRelation |= Bdd(get<1>(p)) & (!Bdd(get<1>(p)+2))		// old 0 - black
									  & (!Bdd(get<0>(p))) & (!Bdd(get<0>(p)+2))		// old 1 - empty
									  & (!Bdd(get<1>(p)+1)) & (!Bdd(get<1>(p)+3))	// new 0 - empty
									  & Bdd(get<0>(p)+1) & (!Bdd(get<0>(p)+3)) & Black;		// new 1 - black
			} else {
				// right or left across -- get<1>(p) can be empty or opponent
				// white
				Partial.MoveRelation |= (!Bdd(get<0>(p))) & Bdd(get<0>(p)+2)		// old 0 - white
									  & (!((!Bdd(get<1>(p))) & Bdd(get<1>(p)+2)))	// old 1 - not white
									  & (!Bdd(get<0>(p)+1)) & (!Bdd(get<0>(p)+3))	// new 0 - empty
									  & (!Bdd(get<1>(p)+1)) & Bdd(get<1>(p)+3) & White;		// new 1 - white
				// black
				Partial.MoveRelation |= Bdd(get<1>(p)) & (!Bdd(get<1>(p)+2))		// old 0 - black
									  & (!(Bdd(get<0>(p)) & (!Bdd(get<0>(p)+2))))	// old 1 - not black
									  & (!Bdd(get<1>(p)+1)) & (!Bdd(get<1>(p)+3))	// new 0 - empty
									  & Bdd(get<0>(p)+1) & (!Bdd(get<0>(p)+3)) & Black;		// new 1 - black
			}

			Relations.push_back(Partial);
    	}

        // Create total move relation from partial relations
        size_t total_nodes = 0;
        MoveRelation = Bdd::bddZero(); // false BDD (empty set)
        for (PartialRelation &Partial : Relations) {
            Bdd Rel = Partial.MoveRelation; // copy

            // extend relation full variable domain setting x <==> x' for new variables:
            for (BDDVAR var : SourceVars) {
                if (Partial.RelationVars.contains(var)) // already in partial
                    continue;
                Rel &= Bdd(var).Xnor(Bdd(var + 1));
            }

            MoveRelation |= Rel;
            total_nodes += Partial.MoveRelation.NodeCount();
        }
        cout << endl << "Partial relations have a total of " << total_nodes <<" BDD nodes" << endl;
    }


    // Same as FixPointBFS() but keeps already winning boards from being further worked out by
    //      removing these boards from Next.
    Bdd StoppedFixPointBFS(Bdd &Start) {
        StartTimer();

        // int level = 0;
        Bdd Next = Start;       // BFS Queue
        Bdd Visited = Start;

        size_t nodes = 0;
        size_t peak = 0;

        while (Next != Bdd::bddZero()) {
            Next = Next - WinningBoardsWhite;
            Next = Next - WinningBoardsBlack;
            Next = Next - WhiteHitAll;
            Next = Next - BlackHitAll;
            Next = Next.RelNext(MoveRelation, RelationVars);
            Next = Next - Visited;
            Visited = Visited | Next;

            nodes = Visited.NodeCount();
            if (peak < nodes) {
                peak = nodes;
            }
            cout << nodes << ", ";    
            

            // PrintLevel (Visited, level++);
            // PrintBoards(Next, "Next");
        }
        cout << "\nPeak: " << peak << endl;
        cout << "End nodes: " << Visited.NodeCount() << endl;
        return Visited;
    }

    /* Prints all boards one move further from board(s) Start */
    Bdd OneMove(Bdd &Start) {
        // PrintBoards(Start, "Start");
        Bdd Next = Start.RelNext(MoveRelation, RelationVars);
        // PrintBoards(Next, "Next");
        return Next;
    }

    void RetrogradeRelWhite(Bdd Reachable) {
        Bdd Deadlock = Bdd::bddZero();
        Deadlock = Deadlock.RelPrevForall(MoveRelation);
        Bdd temp = Bdd::bddZero();
        Bdd Prev = Bdd::bddZero();

        // Bdd WhiteWinBlackMove = WinningBoardsWhite;
        // Bdd WhiteWinWhiteMove = Bdd::bddZero();



        // new
        WinningBoardsWhite |= Reachable & WhiteHitAll.RelPrev(MoveRelation, RelationVars);
        Bdd WhiteWinWhiteMove = WinningBoardsWhite;
        Bdd WhiteWinBlackMove = WhiteHitAll;

        int num = 1;
        while (temp != WinningBoardsWhite) {
            if (WhiteWinBlackMove == Bdd::bddZero() && num == 0) {
                break;
            }
            num = 0;

            temp = WinningBoardsWhite;

            Prev = WhiteWinBlackMove.RelPrev(MoveRelation, RelationVars);
            Prev &= Reachable;

            WinningBoardsWhite |= Prev;
            WhiteWinWhiteMove |= Prev;
            
            Prev = WhiteWinWhiteMove.RelPrevForall(MoveRelation);
            Prev -= Deadlock;
            Prev &= Reachable;

            WinningBoardsWhite |= Prev;
            WhiteWinBlackMove = Prev;
        }
        WinningBoardsWhite |= WhiteHitAll;
    }

    void RetrogradeRelBlack(Bdd Reachable) {
        Bdd Deadlock = Bdd::bddZero();
        Deadlock = Deadlock.RelPrevForall(MoveRelation);
        Bdd temp = Bdd::bddZero();
        Bdd Prev = Bdd::bddZero();

        // Bdd BlackWinWhiteMove = WinningBoardsBlack;
        // Bdd BlackWinBlackMove = Bdd::bddZero();


        // new
        WinningBoardsBlack |= Reachable & BlackHitAll.RelPrev(MoveRelation, RelationVars);
        Bdd BlackWinBlackMove = WinningBoardsBlack;
        Bdd BlackWinWhiteMove = BlackHitAll;

        int num = 1;
        while (temp != WinningBoardsBlack) {
            if (BlackWinWhiteMove == Bdd::bddZero() && num == 0) {
                break;
            }
            num = 0;

            temp = WinningBoardsBlack;
            
            Prev = BlackWinWhiteMove.RelPrev(MoveRelation, RelationVars);
            Prev &= Reachable;

            WinningBoardsBlack |= Prev;
            BlackWinBlackMove |= Prev;

            Prev = BlackWinBlackMove.RelPrevForall(MoveRelation);
            Prev -= Deadlock;
            Prev &= Reachable;

            WinningBoardsBlack |= Prev;
            BlackWinWhiteMove = Prev;
        }
        WinningBoardsBlack |= BlackHitAll;
    }

    /*  Check whether from the given board position Board white or black is winning 
        returns 1 if white is winning, returns 2 if black is winning.
        returns -1 if the board is winning for none or both (mistake) */
    int Winning(Bdd Board) {
        Bdd temp1 = Board & WinningBoardsWhite;
        Bdd temp2 = Board & WinningBoardsBlack;

        if (temp1 != Bdd::bddZero()) {
            if (temp2 != Bdd::bddZero()) {
                cout << "both shouldn't be winning..." << endl;
                return -1;
            }
            cout << "White wins from this position." << endl;
            return 1;
        }
        else if (temp2 != Bdd::bddZero()) {
            cout << "Black wins from this position." << endl;
            return 2;
        }
        cout << "No winner from this position." << endl;
        return -1;
    }

    /* Checks for overlapping boards between WinningBoardsBlack and WinningBoardsWhite*/
    void Overlap() {

        Bdd overlap = WinningBoardsWhite & WinningBoardsBlack;
        if (overlap == Bdd::bddZero()) {
            cout << "No overlap" << endl;
        } else {
            cout << "There is overlap, " << overlap.SatCount(SourceVars) << " boards overlapping" << endl;
            // PrintBoards(overlap, "overlap");
        }
        return;
    }

    void PrintBoards(Bdd &B, string name) {
        LACE_ME;
        cout << endl <<  endl << "Printing boards: " << name << endl;
        sylvan_enum(B.GetBDD(), SourceVars.GetBDD(), printboard, this);
    }

    system_clock::duration start;
    void StartTimer() {
        start = system_clock::now().time_since_epoch();
    }

    void PrintTimer() {
        system_clock::duration now = system_clock::now().time_since_epoch();
        double ms = duration_cast< milliseconds >(now - start).count();
        cout << "["<< fixed  << setprecision(2) << setfill(' ') << setw(8) << ms / 1000 <<"]  ";
    }

    size_t MaxNodes = 0;
private:
    void PrintLevel (const Bdd& Visited, double level) {
        PrintTimer();
        size_t NodeCount = Visited.NodeCount();
        if (MaxNodes < NodeCount)
            MaxNodes = NodeCount;
        cout << "Search Level " << setprecision (0) << setw (3) << setfill (' ') << level <<
                ": Visited has " << setprecision (0) << fixed << setfill (' ') << setw (12)
                << Visited.SatCount(SourceVars) << " boards and "
                << NodeCount << " BDD nodes" << endl;
    }
};


/**
 * Implementation of callback for printing boards from BDD package
 */
VOID_TASK_IMPL_4(printboard, void*, ctx, BDDVAR*, VA, uint8_t*, values, int, count) {
	Breakthrough *Game = (Breakthrough *) ctx;
	BDDVAR player = M*N*2; // Todo: niet zo
    cout << "To move: ";
    if (values[player]) {
    	cout << "white" << endl;
    } else {
    	cout << "black" << endl;
    }
    cout << "--------" << endl;
    for (int i = M-1; i+1 > 0; i--) {
        for (int j = 0; j < N; j++) {
			BDDVAR var = Game->Coor2Var[i][j];
			for (int k = 0; k < count; k++) {
				if (var == VA[k]) {
					if (values[k] && (!values[k+1])) {
						cout << "B, ";
					} else if ((!values[k]) && (!values[k+1])) {
						cout << "_, ";
					} else if ((!values[k]) && values[k+1]) {
						cout << "W, ";
					} else {
						cout << "!, ";
					}
					break;
				}
			}
        }
        cout << endl;
    }
    cout << "--------" << endl;
}

int main() {
    auto start = steady_clock::now();

    /* Init BDD package */
    lace_init(0, 0);
    lace_startup(0, NULL, NULL);

    LACE_ME;
    sylvan_set_sizes(1ULL<<22, 1ULL<<27, 1ULL<<20, 1ULL<<26);
    // to do:
    // sylvan_set_sizes(1ULL<<22, 1ULL<<30, 1ULL<<20, 1ULL<<26);
    sylvan_init_package();
    sylvan_init_bdd();

    /* Create Breakthrough game */
    Breakthrough Game;

    Game.CreateInitialBoard();
    Game.PrintBoards(Game.InitialBoard, "initial board");

    Game.CreateMoveRelation();
    cout << "Done: CreateMoveRelation()" << endl;

    Game.CreateWinningBoardsWhite();
    cout << "Done: CreateWinningBoardsWhite()" << endl;
    Game.CreateWinningBoardsBlack();
    cout << "Done: CreateWinningBoardsBlack()" << endl;


    cout << "Total relation has " << scientific << setprecision(2)
        << Game.MoveRelation.SatCount(Game.RelationVars) << " board tuples and "
        << Game.MoveRelation.NodeCount() << " BDD nodes" << endl << endl;

    Bdd FixPoint;
    cout << "Start: StoppedFixPointBFS()" << endl;
    FixPoint = Game.StoppedFixPointBFS(Game.InitialBoard);
    cout << "Done: StoppedFixPointBFS()" << endl << endl;

    // reachable boards
    cout << "Reachable boards count: " << setprecision(0) << fixed << setfill (' ') << setw (12)
        << FixPoint.SatCount(Game.SourceVars) << " (" << FixPoint.NodeCount() << " nodes)" << endl;
    cout << "Maximum queue size: " << Game.MaxNodes << " BDD nodes" << endl;
    
    // limit WinningBoards to possible boards    
    Game.WinningBoardsWhite &= FixPoint;
    Game.WinningBoardsBlack &= FixPoint;
    cout << "Winning boards White: " << Game.WinningBoardsWhite.SatCount(Game.SourceVars) << endl;
    cout << "Winning boards Black: " << Game.WinningBoardsBlack.SatCount(Game.SourceVars) << endl;

    Game.WhiteHitAll &= FixPoint;
    Game.BlackHitAll &= FixPoint;

    // expand winning boards in previous boards
    cout << "Start: RetrogradeRelWhite()" << endl;
    Game.RetrogradeRelWhite(FixPoint);
    cout << "End: RetrogradeRelWhite()" << endl;
    cout << "Start: RetrogradeRelBlack()" << endl;
    Game.RetrogradeRelBlack(FixPoint);
    cout << "End: RetrogradeRelBlack()" << endl;

    cout << "Winning boards White final: " << Game.WinningBoardsWhite.SatCount(Game.SourceVars) << endl;
    cout << "Winning boards Black final: " << Game.WinningBoardsBlack.SatCount(Game.SourceVars) << endl << endl;

    cout << "From starting position: " << endl;
    Game.Winning(Game.InitialBoard);


    /* Some checks */
    Game.Overlap();

    if (Game.WinningBoardsWhite.SatCount(Game.SourceVars) + Game.WinningBoardsBlack.SatCount(Game.SourceVars) == FixPoint.SatCount(Game.SourceVars)) {
        cout << "It adds up" << endl;
    } else {
        cout << "it doesn't add up" << endl;
    }

    if (FixPoint - Game.WinningBoardsBlack - Game.WinningBoardsWhite == Bdd::bddZero()) {
        cout << "full coverage of reachable board states." << endl;
    } else {
        cout << "not fully covered." << endl;
    }


    auto end = steady_clock::now();
    duration<double> elapsed_seconds = end - start;

    cout << endl << "Elapsed time: " << elapsed_seconds.count() << "s" << endl;

    return 0;
}














    /* Gekloot */
    // Bdd one = Bdd::bddOne(); // the True terminal
    // Bdd zero = Bdd::bddZero(); // the False terminal

    // BDDVAR een = 1;
    // BDDVAR twee = 2;
    // BDDVAR drie = 3;

    // Bdd Een = Bdd(een);
    // Bdd Twee = Bdd(twee);
    // Bdd Drie = Bdd(drie);

    // zero |= Een;
    // if (zero == Een) {
    //     cout << "1" << endl; //!
    // }
    // if (zero.Else() == Een) {
    //     cout << "2" << endl;
    // }

    // Bdd test = Een.Xnor(Twee);

    // if (test.Then() == Twee) { // klopt
    //     cout << "b" << endl;
    // }
    // if (test.Else() == !Twee) { // klopt
    //     cout << "a" << endl;
    // }
    // if (test.Then().Then() == one) { // klopt
    //     cout << "mm" << endl;
    // }
    // if (test.Else().Else() == one) { // klopt
    //     cout << "mmm" << endl;
    // }
    // if (test.Else().Then() == zero) { // klopt
    //     cout << "mmmm" << endl;
    // }
    // if (test.TopVar() == een) { // klopt
    //     cout << "mmmmm" << endl;
    // }