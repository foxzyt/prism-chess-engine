#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <algorithm>
#include <ctime>
#include <cmath>

using namespace std;

#define U64 unsigned long long

enum { P = 1, N, B, R, Q, K, p, n, b, r, q, k };

const int INF = 50000;
const int MATE = 49000;

int board[128];
int side;
int enpassant = -1;
int castle = 15;
int ply = 0;
U64 piece_keys[13][128];
U64 side_key;
U64 castle_keys[16];
U64 current_hash;
vector<U64> history;
long nodes;

const int squares[64] = {
    0,  1,  2,  3,  4,  5,  6,  7,
    16, 17, 18, 19, 20, 21, 22, 23,
    32, 33, 34, 35, 36, 37, 38, 39,
    48, 49, 50, 51, 52, 53, 54, 55,
    64, 65, 66, 67, 68, 69, 70, 71,
    80, 81, 82, 83, 84, 85, 86, 87,
    96, 97, 98, 99, 100, 101, 102, 103,
    112, 113, 114, 115, 116, 117, 118, 119
};

const int material[13] = { 0, 100, 320, 330, 500, 900, 20000, -100, -320, -330, -500, -900, -20000 };

const int pst_p[64] = {
    0,  0,  0,  0,  0,  0,  0,  0,
    50, 50, 50, 50, 50, 50, 50, 50,
    10, 10, 20, 30, 30, 20, 10, 10,
    5,  5, 10, 25, 25, 10,  5,  5,
    0,  0,  0, 20, 20,  0,  0,  0,
    5, -5,-10,  0,  0,-10, -5,  5,
    5, 10, 10,-20,-20, 10, 10,  5,
    0,  0,  0,  0,  0,  0,  0,  0
};
const int pst_n[64] = {
    -50,-40,-30,-30,-30,-30,-40,-50,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50
};
const int pst_b[64] = {
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0, 10, 10, 10, 10,  0,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -20,-10,-10,-10,-10,-10,-10,-20
};
const int pst_r[64] = {
    0,  0,  0,  0,  0,  0,  0,  0,
    5, 10, 10, 10, 10, 10, 10,  5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    0,  0,  0,  5,  5,  0,  0,  0
};
const int pst_k[64] = {
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -20,-30,-30,-40,-40,-30,-30,-20,
    -10,-20,-20,-20,-20,-20,-20,-10,
    20, 20,  0,  0,  0,  0, 20, 20,
    20, 30, 10,  0,  0, 10, 30, 20
};

int* pst_table[13] = { nullptr, (int*)pst_p, (int*)pst_n, (int*)pst_b, (int*)pst_r, nullptr, (int*)pst_k, (int*)pst_p, (int*)pst_n, (int*)pst_b, (int*)pst_r, nullptr, (int*)pst_k };

const int mvv_lva[13][13] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,105,104,103,102,101,100,105,104,103,102,101,100},
    {0,205,204,203,202,201,200,205,204,203,202,201,200},
    {0,305,304,303,302,301,300,305,304,303,302,301,300},
    {0,405,404,403,402,401,400,405,404,403,402,401,400},
    {0,505,504,503,502,501,500,505,504,503,502,501,500},
    {0,605,604,603,602,601,600,605,604,603,602,601,600},
    {0,105,104,103,102,101,100,105,104,103,102,101,100},
    {0,205,204,203,202,201,200,205,204,203,202,201,200},
    {0,305,304,303,302,301,300,305,304,303,302,301,300},
    {0,405,404,403,402,401,400,405,404,403,402,401,400},
    {0,505,504,503,502,501,500,505,504,503,502,501,500},
    {0,605,604,603,602,601,600,605,604,603,602,601,600}
};

U64 rand64() {
    return (U64)rand() | ((U64)rand() << 15) | ((U64)rand() << 30) | ((U64)rand() << 45) | ((U64)rand() << 60);
}

void init_zobrist() {
    for(int i=0; i<13; i++) for(int j=0; j<128; j++) piece_keys[i][j] = rand64();
    side_key = rand64();
    for(int i=0; i<16; i++) castle_keys[i] = rand64();
}

struct Move {
    int from, to, promoted, captured, flags, score;
};

void reset_board() {
    for (int i = 0; i < 128; ++i) board[i] = 0;
    int init[64] = {
        r, n, b, q, k, b, n, r,
        p, p, p, p, p, p, p, p,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        P, P, P, P, P, P, P, P,
        R, N, B, Q, K, B, N, R
    };
    for (int i = 0; i < 64; ++i) board[squares[i]] = init[i];
    side = 0; enpassant = -1; castle = 15; ply = 0;
    current_hash = 0;
    history.clear();
    
    for(int i=0; i<128; i++) if(board[i]) current_hash ^= piece_keys[board[i]][i];
    current_hash ^= castle_keys[castle];
    history.push_back(current_hash);
}

bool is_repetition() {
    int c = 0;
    for(int i=0; i < (int)history.size() - 1; ++i) {
        if(history[i] == current_hash) c++;
    }
    return c >= 1;
}

int evaluate() {
    int score = 0;
    for (int i = 0; i < 64; ++i) {
        int sq = squares[i];
        int pc = board[sq];
        if (pc) {
            int val = material[pc];
            if (pst_table[pc]) {
                if (pc <= 6) val += pst_table[pc][i];
                else val -= pst_table[pc][63 - i];
            }
            score += val;
        }
    }
    return (side == 0) ? score : -score;
}

void generate_moves(vector<Move>& moves) {
    for (int i = 0; i < 64; ++i) {
        int sq = squares[i];
        int pc = board[sq];
        if (!pc) continue;
        if ((side == 0 && pc > 6) || (side == 1 && pc <= 6)) continue;

        if (pc == P || pc == p) {
            int dir = (side == 0) ? -16 : 16;
            int to = sq + dir;
            if (!(to & 0x88) && !board[to]) {
                if ((side == 0 && to < 8) || (side == 1 && to > 119)) {
                    int pros[] = {(side==0?Q:q), (side==0?R:r), (side==0?B:b), (side==0?N:n)};
                    for (int pr : pros) moves.push_back({sq, to, pr, 0, 0, 0});
                } else {
                    moves.push_back({sq, to, 0, 0, 0, 0});
                    int to2 = to + dir;
                    if (((side == 0 && sq >= 96 && sq <= 103) || (side == 1 && sq >= 16 && sq <= 23)) && !board[to2]) {
                        moves.push_back({sq, to2, 0, 0, 0, 0});
                    }
                }
            }
            int cap_dirs[] = {(side==0?-17:17), (side==0?-15:15)};
            for (int d : cap_dirs) {
                to = sq + d;
                if (!(to & 0x88)) {
                    if (board[to] && ((side==0 && board[to]>6) || (side==1 && board[to]<=6))) {
                        if ((side == 0 && to < 8) || (side == 1 && to > 119)) {
                            int pros[] = {(side==0?Q:q), (side==0?R:r), (side==0?B:b), (side==0?N:n)};
                            for (int pr : pros) moves.push_back({sq, to, pr, board[to], 0, 0});
                        } else {
                            moves.push_back({sq, to, 0, board[to], 0, 0});
                        }
                    } else if (to == enpassant) {
                        moves.push_back({sq, to, 0, (side==0?p:P), 1, 0});
                    }
                }
            }
        } else {
            int t = (pc > 6) ? pc - 6 : pc;
            int dirs[8], cnt = 0;
            if (t==N) { int d[]={-33,-31,-18,-14,14,18,31,33}; memcpy(dirs,d,sizeof(d)); cnt=8; }
            else if (t==B) { int d[]={-17,-15,15,17}; memcpy(dirs,d,sizeof(d)); cnt=4; }
            else if (t==R) { int d[]={-16,-1,1,16}; memcpy(dirs,d,sizeof(d)); cnt=4; }
            else { int d[]={-17,-16,-15,-1,1,15,16,17}; memcpy(dirs,d,sizeof(d)); cnt=8; }

            for (int k=0; k<cnt; ++k) {
                int to = sq + dirs[k];
                while (!(to & 0x88)) {
                    int cap = board[to];
                    if (cap) {
                        if ((side==0 && cap>6) || (side==1 && cap<=6)) moves.push_back({sq, to, 0, cap, 0, 0});
                        break;
                    }
                    moves.push_back({sq, to, 0, 0, 0, 0});
                    if (t==N || t==K) break;
                    to += dirs[k];
                }
            }
        }
    }
}

void score_moves(vector<Move>& moves) {
    for (auto& m : moves) {
        if (m.captured) m.score = 10000 + mvv_lva[m.captured][board[m.from]];
        else m.score = 0;
    }
}

void pick_move(vector<Move>& moves, int start_index) {
    int best_idx = -1;
    int best_score = -1;
    for (size_t i = start_index; i < moves.size(); ++i) {
        if (moves[i].score > best_score) {
            best_score = moves[i].score;
            best_idx = i;
        }
    }
    if (best_idx != -1) swap(moves[start_index], moves[best_idx]);
}

bool make_move(Move m) {
    int from = m.from, to = m.to;
    int pc = board[from];
    int captured = m.captured;
    
    if (m.flags == 1) {
        if (side == 0) board[to + 16] = 0;
        else board[to - 16] = 0;
    }
    
    board[to] = m.promoted ? m.promoted : pc;
    board[from] = 0;
    
    current_hash ^= piece_keys[pc][from];
    current_hash ^= piece_keys[board[to]][to];
    if(captured) current_hash ^= piece_keys[captured][to];
    
    side ^= 1; 
    current_hash ^= side_key;
    
    history.push_back(current_hash);
    ply++;
    return true;
}

void unmake_move(Move m) {
    ply--;
    history.pop_back();
    
    side ^= 1;
    current_hash ^= side_key;
    
    int pc = m.promoted ? (side==0?P:p) : board[m.to];
    
    current_hash ^= piece_keys[board[m.to]][m.to]; 
    
    board[m.from] = pc;
    board[m.to] = m.captured;
    
    current_hash ^= piece_keys[pc][m.from];
    if(m.captured) current_hash ^= piece_keys[m.captured][m.to];

    if (m.flags == 1) {
        board[m.to] = 0;
        if (side == 0) board[m.to + 16] = p; else board[m.to - 16] = P;
    }
}

bool is_attacked(int sq, int by_side) {
    if (by_side == 0) { 
        if (!((sq+17) & 0x88) && board[sq+17] == P) return true;
        if (!((sq+15) & 0x88) && board[sq+15] == P) return true;
    } else {
        if (!((sq-17) & 0x88) && board[sq-17] == p) return true;
        if (!((sq-15) & 0x88) && board[sq-15] == p) return true;
    }
    
    int n_dirs[] = {-33,-31,-18,-14,14,18,31,33};
    for(int d : n_dirs) {
        int t = sq + d;
        if(!(t & 0x88) && board[t] == (by_side==0?N:n)) return true;
    }
    
    int k_dirs[] = {-17,-16,-15,-1,1,15,16,17};
    for(int d : k_dirs) {
        int t = sq + d;
        if(!(t & 0x88) && board[t] == (by_side==0?K:k)) return true;
    }

    int b_dirs[] = {-17,-15,15,17};
    for(int d : b_dirs) {
        int t = sq + d;
        while(!(t & 0x88)) {
            if(board[t]) {
                if(board[t] == (by_side==0?B:b) || board[t] == (by_side==0?Q:q)) return true;
                break;
            }
            t += d;
        }
    }
    
    int r_dirs[] = {-16,-1,1,16};
    for(int d : r_dirs) {
        int t = sq + d;
        while(!(t & 0x88)) {
            if(board[t]) {
                if(board[t] == (by_side==0?R:r) || board[t] == (by_side==0?Q:q)) return true;
                break;
            }
            t += d;
        }
    }
    return false;
}

bool in_check() {
    int kpos = -1;
    for(int i=0; i<128; i++) if(board[i] == (side==0?K:k)) { kpos=i; break; }
    return is_attacked(kpos, side^1);
}

int quiescence(int alpha, int beta) {
    nodes++;
    int eval = evaluate();
    if (eval >= beta) return beta;
    if (eval > alpha) alpha = eval;

    vector<Move> moves;
    generate_moves(moves);
    score_moves(moves);

    for (size_t i=0; i<moves.size(); ++i) {
        pick_move(moves, i);
        Move m = moves[i];
        if (m.captured == 0 && m.flags == 0) continue;

        make_move(m);
        int kpos = -1;
        for(int x=0; x<128; x++) if(board[x] == (side==0?k:K)) { kpos=x; break; }
        if (kpos == -1 || is_attacked(kpos, side)) { unmake_move(m); continue; }

        int score = -quiescence(-beta, -alpha);
        unmake_move(m);
        
        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }
    return alpha;
}

int negamax(int depth, int alpha, int beta) {
    if (is_repetition()) return 0;
    
    nodes++;
    if (depth == 0) return quiescence(alpha, beta);

    vector<Move> moves;
    generate_moves(moves);
    score_moves(moves);
    
    int legal_moves = 0;
    int best_val = -INF;

    for (size_t i=0; i<moves.size(); ++i) {
        pick_move(moves, i);
        Move m = moves[i];
        
        make_move(m);
        
        int kpos = -1;
        for(int x=0; x<128; x++) if(board[x] == (side==0?k:K)) { kpos=x; break; }
        if (kpos == -1 || is_attacked(kpos, side)) { unmake_move(m); continue; }

        legal_moves++;
        int score = -negamax(depth - 1, -beta, -alpha);
        unmake_move(m);

        if (score >= beta) return beta;
        if (score > best_val) {
            best_val = score;
            if (score > alpha) alpha = score;
        }
    }
    if (legal_moves == 0) {
        if(in_check()) return -MATE + ply;
        else return 0;
    }
    return best_val;
}

Move best_move_root;

void search_position(int depth) {
    int alpha = -INF;
    int beta = INF;
    nodes = 0;
    
    vector<Move> moves;
    generate_moves(moves);
    score_moves(moves);
    
    int best_val = -INF;
    
    for (size_t i=0; i<moves.size(); ++i) {
        pick_move(moves, i);
        Move m = moves[i];
        
        make_move(m);
        
        int kpos = -1;
        for(int x=0; x<128; x++) if(board[x] == (side==0?k:K)) { kpos=x; break; }
        if(kpos == -1 || is_attacked(kpos, side)) { unmake_move(m); continue; }

        int val = -negamax(depth - 1, -beta, -alpha);
        unmake_move(m);
        
        if (val > best_val) {
            best_val = val;
            best_move_root = m;
        }
        if (val > alpha) alpha = val;
    }
}

string to_san(Move m) {
    int pc = board[m.from];
    int pt = (pc > 6) ? pc - 6 : pc;
    
    if (pt == K && abs(m.from - m.to) == 2) {
        if (m.to > m.from) return "O-O";
        else return "O-O-O";
    }

    string s = "";
    
    if (pt != P) {
        if (pt == N) s += 'N';
        else if (pt == B) s += 'B';
        else if (pt == R) s += 'R';
        else if (pt == Q) s += 'Q';
        else if (pt == K) s += 'K';

        vector<Move> others;
        generate_moves(others);
        bool file_amb = false, rank_amb = false;
        bool conflict = false;

        for (auto& om : others) {
            if (om.from == m.from) continue;
            if (om.to != m.to) continue;
            if (board[om.from] != pc) continue;

            make_move(om);
            int kpos = -1;
            for(int x=0; x<128; x++) if(board[x] == (side==0?k:K)) { kpos=x; break; }
            bool legal = (kpos != -1 && !is_attacked(kpos, side));
            unmake_move(om);
            
            if (legal) {
                conflict = true;
                if ((om.from & 7) == (m.from & 7)) rank_amb = true;
                else file_amb = true;
            }
        }

        if (conflict) {
            if (file_amb) s += (char)('a' + (m.from & 7));
            if (rank_amb) s += (char)('8' - (m.from >> 4));
        }
    }

    if (m.captured || m.flags == 1) {
        if (pt == P) s += (char)('a' + (m.from & 7));
        s += 'x';
    }

    s += (char)('a' + (m.to & 7));
    s += (char)('8' - (m.to >> 4));

    if (m.promoted) {
        s += "=";
        int prm = (m.promoted > 6) ? m.promoted - 6 : m.promoted;
        if (prm == Q) s += 'Q';
        else if (prm == R) s += 'R';
        else if (prm == B) s += 'B';
        else if (prm == N) s += 'N';
    }

    make_move(m);
    if (in_check()) {
        vector<Move> legals;
        generate_moves(legals);
        bool can_move = false;
        for (auto& lm : legals) {
            make_move(lm);
            int kpos = -1;
            for(int x=0; x<128; x++) if(board[x] == (side==0?k:K)) { kpos=x; break; }
            if (kpos != -1 && !is_attacked(kpos, side)) can_move = true;
            unmake_move(lm);
            if(can_move) break;
        }
        if (!can_move) s += "#";
        else s += "+";
    }
    unmake_move(m);

    return s;
}

int main() {
    srand(time(0));
    init_zobrist();
    reset_board();

    cout << "[Event \"Prism Self-Play\"]" << endl;
    cout << "[Site \"Localhost\"]" << endl;
    cout << "[Date \"2025.04.07\"]" << endl;
    cout << "[Round \"1\"]" << endl;
    cout << "[White \"Prism 1\"]" << endl;
    cout << "[Black \"Prism 2\"]" << endl;
    cout << "[Result \"*\"]" << endl << endl;
    
    int full_moves = 1;
    bool game_over = false;
    int fifty_rule = 0;

    while (!game_over && full_moves < 250) {
        search_position(5);
        
        if (nodes == 0) { 
            if(in_check()) cout << (side==0?"0-1":"1-0") << " {Mate}" << endl;
            else cout << "1/2-1/2 {Stalemate}" << endl; 
            break; 
        }

        Move m = best_move_root;
        string san = to_san(m);
        
        if (m.captured || m.promoted || (board[m.from]==P) || (board[m.from]==p)) fifty_rule = 0;
        else fifty_rule++;

        if (side == 0) {
            cout << full_moves << ". " << san << " ";
        } else {
            cout << san << " ";
            full_moves++;
        }
        cout.flush();
        
        make_move(m);
        
        if(fifty_rule >= 100) { cout << "1/2-1/2 {50 moves}" << endl; break; }
        if(is_repetition()) { cout << "1/2-1/2 {Repetition}" << endl; break; }
    }
    
    return 0;
}