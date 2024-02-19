#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <cstdlib>
#include <ctime>
#include<stdio.h>
#include <limits.h>
//在棋局進行到1/3前盡量別吃太多子
//避免對手下一步搶角
//搶下角則C位X位安全 延伸出的邊為穩定子
//散度理論 散度:周圍空格數量 優：吃對方散度為一的子 總吃敵方的散度要小
//01找到兩個板前後 翻轉的子(board中的對手 next_board中變成我方) 02計算每一顆的散度(空格用board的計) 03在比較處為高散度扣分

struct Point {
	int x, y;
	int value;
	Point() : Point(0, 0) {}
	Point(int x, int y) : x(x), y(y) {}////float??
	bool operator==(const Point& rhs) const {
		return x == rhs.x && y == rhs.y;
	}
	bool operator!=(const Point& rhs) const {
		return !operator==(rhs);
	}
	Point operator+(const Point& rhs) const {
		return Point(x + rhs.x, y + rhs.y);
	}
	Point operator-(const Point& rhs) const {
		return Point(x - rhs.x, y - rhs.y);
	}
	Point operator=(const Point& rhs) {
		this->x = rhs.x;
		this->y = rhs.y;
		return *this;
	}
};
int player;
class OthelloBoard {
public:
	enum SPOT_STATE {
		EMPTY = 0,
		BLACK = 1,
		WHITE = 2
	};
	static const int SIZE = 8;
	const std::array<Point, 8> directions{ {
		Point(-1, -1), Point(-1, 0), Point(-1, 1),
		Point(0, -1), /*{0, 0}, */Point(0, 1),
		Point(1, -1), Point(1, 0), Point(1, 1)
	} };
	std::array<std::array<int, SIZE>, SIZE> board;
	std::vector<Point> next_valid_spots;
	std::array<int, 3> disc_count;
	int cur_player;
	bool done;
	int winner;
	int pre_enemy_disc;


	OthelloBoard() {
		reset();
	}
	OthelloBoard(const OthelloBoard& b) {//copy
		for (int i = 0; i < SIZE; i++)
			for (int j = 0; j < SIZE; j++)
				this->board[i][j] = b.board[i][j];
		this->cur_player = b.cur_player;
		for (int i = 0; i < 3; i++)
			this->disc_count[i] = b.disc_count[i];
		this->done = b.done;
		this->winner = b.winner;
		this->next_valid_spots = b.next_valid_spots;
		this->pre_enemy_disc = b.pre_enemy_disc;
	}
	OthelloBoard& operator = (const OthelloBoard& b)
	{
		for (int i = 0; i < SIZE; i++)
			for (int j = 0; j < SIZE; j++)
				this->board[i][j] = b.board[i][j];
		this->cur_player = b.cur_player;
		for (int i = 0; i < 3; i++)
			this->disc_count[i] = b.disc_count[i];
		this->done = b.done;
		this->winner = b.winner;
		this->pre_enemy_disc = b.pre_enemy_disc;
		return *this;
	}
	void reset() {
		for (int i = 0; i < SIZE; i++) {
			for (int j = 0; j < SIZE; j++) {
				board[i][j] = EMPTY;
			}
		}
		board[3][4] = board[4][3] = BLACK;
		board[3][3] = board[4][4] = WHITE;
		cur_player = BLACK;
		disc_count[EMPTY] = 8 * 8 - 4;
		disc_count[BLACK] = 2;
		disc_count[WHITE] = 2;
		pre_enemy_disc = 0;
		next_valid_spots = get_valid_spots();
		done = false;
		winner = -1;
	}
	std::vector<Point> get_valid_spots() {
		std::vector<Point> valid_spots;
		for (int i = 0; i < SIZE; i++) {
			for (int j = 0; j < SIZE; j++) {
				Point p = Point(i, j);
				if (board[i][j] != EMPTY)
					continue;
				if (is_spot_valid(p))
					valid_spots.push_back(p);
			}
		}
		return valid_spots;
	}
	bool put_disc(Point p) {
		if (!is_spot_valid(p)) {
			winner = get_next_player(cur_player);
			done = true;
			return false;
		}
		set_disc(p, cur_player);
		disc_count[cur_player]++;
		disc_count[EMPTY]--;
		flip_discs(p);
		// Give control to the other player.
		cur_player = get_next_player(cur_player);
		next_valid_spots = get_valid_spots();
		// Check Win
		if (next_valid_spots.size() == 0) {
			cur_player = get_next_player(cur_player);
			next_valid_spots = get_valid_spots();
			if (next_valid_spots.size() == 0) {
				// Game ends
				done = true;
				int white_discs = disc_count[WHITE];
				int black_discs = disc_count[BLACK];
				if (white_discs == black_discs) winner = EMPTY;
				else if (black_discs > white_discs) winner = BLACK;
				else winner = WHITE;
			}
		}
		return true;
	}
	int count_value() {//!!OWO

		int ans = 0;

		int next_step_num = this->get_valid_spots().size();

		float ratio;
		if (this->disc_count[3 - player] == 0)
			ratio = (float)(this->disc_count[player]) / 1;
		else
			ratio = (float)(this->disc_count[player]) / (float)(this->disc_count[3 - player]);

        /*if((float)(this->disc_count[player]) + (float)(this->disc_count[3 - player]) < 21)//在棋局進行到1/3前盡量別吃太多子
            ans = (-1)*ratio + 10*next_step_num;
        else
            ans = 5*ratio + 8*next_step_num;*/

        this->cur_player = 3 - this->cur_player;
        for(auto p:this->get_valid_spots()){//避免對手下一步搶角
            if((p.x==0 && p.y==0) || (p.x==0 && p.y==7) || (p.x==7 && p.y==0) || (p.x==7 && p.y==7))
                ans -= 100000;
        }
         for(auto p:this->get_valid_spots()){//避免對手下一步搶角
            if((p.x==0 ) || (p.y==7) || ( p.y==0) || (p.x==7))
                ans -= 2000;
        }
        this->cur_player = 3 - this->cur_player;
		return ans;
	}
private:
	int get_next_player(int player) {
		return 3 - player;
	}
	bool is_spot_on_board(Point p) {
		return 0 <= p.x && p.x < SIZE && 0 <= p.y && p.y < SIZE;
	}
	int get_disc(Point p) {
		return board[p.x][p.y];
	}
	void set_disc(Point p, int disc) {
		board[p.x][p.y] = disc;
	}
	bool is_disc_at(Point p, int disc) {
		if (!is_spot_on_board(p))
			return false;
		if (get_disc(p) != disc)
			return false;
		return true;
	}
	bool is_spot_valid(Point center) {//確認此點是否可放子
		if (get_disc(center) != EMPTY)
			return false;
		for (Point dir : directions) {
			// Move along the direction while testing.
			Point p = center + dir;
			if (!is_disc_at(p, get_next_player(cur_player)))
				continue;
			p = p + dir;
			while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
				if (is_disc_at(p, cur_player))
					return true;
				p = p + dir;
			}
		}
		return false;
	}
	void flip_discs(Point center) {
		for (Point dir : directions) {
			// Move along the direction while testing.
			Point p = center + dir;
			if (!is_disc_at(p, get_next_player(cur_player)))
				continue;
			std::vector<Point> discs({ p });
			p = p + dir;
			while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
				if (is_disc_at(p, cur_player)) {
					for (Point s : discs) {
						set_disc(s, cur_player);
					}
					disc_count[cur_player] += discs.size();
					disc_count[get_next_player(cur_player)] -= discs.size();
					break;
				}
				discs.push_back(p);
				p = p + dir;
			}
		}
	}
};

const int SIZE = 8;
std::array<std::array<int, SIZE>, SIZE> board;
std::vector<Point> next_valid_spots;

int good_pos(int x, int y)
{
	int score[8][8] = {
        {120, -30,  16,  12,  12,  16, -30, 120},
        {-30, -30,   6,   4,   4,   6, -30, -30},
        { 16,   6,  10,   12,   12,  10,   6,  16},
        { 12,   4,   12,   0,   0,   12,   4,  12},
        { 12,   4,   12,   0,   0,   12,   4,  12},
        { 16,   6,  10,   12,   12,  10,   6,  16},
        {-30, -30,   6,   4,   4,   6, -30, -30},
        {120, -30,  16,  12,  12,  16, -30, 120}
    };
    if(board[0][0]==player) score[0][1] = score[1][0] = 9, score[1][1] = 2;//搶下角則C位X位安全 延伸出的邊為穩定子
    if(board[0][7]==player) score[0][6] = score[1][7] = 9, score[1][6] = 2;
    if(board[7][0]==player) score[6][0] = score[7][1] = 9, score[6][1] = 2;
    if(board[7][7]==player) score[7][6] = score[6][7] = 9, score[6][6] = 2;

    return score[x][y];
}

void read_board(std::ifstream& fin) {
	fin >> player;
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			fin >> board[i][j];
		}
	}
}

void read_valid_spots(std::ifstream& fin) {
	int n_valid_spots;
	fin >> n_valid_spots;
	int x, y;
	for (int i = 0; i < n_valid_spots; i++) {
		fin >> x >> y;
		next_valid_spots.push_back({ x, y });
	}
}

int minimax(int depth, OthelloBoard& board, int alpha, int beta)
{
	if (depth >= 6 || board.done == true) {
		return board.count_value();

	}


	if (depth % 2 == 1) {//
		int max = -100;
		std::vector<Point> child = board.get_valid_spots();
		std::vector<Point>::iterator it;
		for (it = child.begin(); it != child.end(); it++) {
			OthelloBoard next_board = board;
			next_board.put_disc(*it);
			int m = minimax(depth + 1, next_board, alpha, beta) + good_pos((*it).x, (*it).y);//!!OWO
			//散度理論 散度:周圍空格數量 優：吃對方散度為一的子 總吃敵方的散度要小
             //01找到兩個板前後 翻轉的子(board中的對手 next_board中變成我方) 02計算每一顆的散度(空格用board的計) 03在比較處為高散度扣分
             int sandu = 0;
             for(int i=0;i<SIZE;i++){
                for(int j=0;j<SIZE;j++){
                    if(board.board[i][j]==(3 - board.cur_player)&&next_board.board[i][j]==(board.cur_player)){
                        if(board.board[i-1][j-1]==0) sandu++;
                        if(board.board[i-1][j]==0) sandu++;
                        if(board.board[i-1][j+1]==0) sandu++;
                        if(board.board[i][j-1]==0) sandu++;
                        if(board.board[i][j+1]==0) sandu++;
                        if(board.board[i+1][j-1]==0) sandu++;
                        if(board.board[i+1][j]==0) sandu++;
                        if(board.board[i+1][j+1]==0) sandu++;
                    }

                }
             }
             m -= sandu;

			if (m > max) {
				max = m;
				alpha = max;
			}
			if (alpha > beta) break;
		}
		return max;
	}
	else if (depth % 2 == 0) { //
		int min = 100;
		std::vector<Point> child = board.get_valid_spots();
		std::vector<Point>::iterator it;
		for (it = child.begin(); it != child.end(); it++) {
			OthelloBoard next_board = board;
			next_board.put_disc(*it);
			int m = minimax(depth + 1, next_board, alpha, beta) + good_pos((*it).x, (*it).y);//!!OWO
			if (m < min) {
				min = m;
				beta = m;
			}
			if (alpha > beta) break;
		}
		return min;
	}
	return 0;
}

void write_valid_spot(std::ofstream& fout) {
	srand(time(NULL));

	OthelloBoard Board;
	Board.cur_player = player;
	for (int i = 0; i < 8; i++)
		for (int j = 0; j < 8; j++)
			Board.board[i][j] = board[i][j];
	Point best_point;
	int bp = -9999999;
	std::vector<Point>::iterator it;
	int alpha = INT_MAX, beta = INT_MIN;

	for (it = next_valid_spots.begin(); it != next_valid_spots.end(); it++) {//對每一個可走的點
		OthelloBoard next_board = Board;
		next_board.put_disc(*it);  //放disc
		int m = 0;
		if (next_board.done == 1)
            m = next_board.count_value() + good_pos((*it).x, (*it).y) ;//!!OWO
		else m = minimax(2, next_board, alpha, beta) + good_pos((*it).x, (*it).y);//!!OWO
		if (m > bp) {
			bp = m;
			best_point = *it;
			alpha = m;
		}
	}
	//printf("bp %d (%d, %d)\n", bp, best_point.x, best_point.y);
	int my_flag = 1;
for(auto qoq:next_valid_spots){
  if(qoq == best_point) my_flag = 0;
  if(my_flag==0) break;
}
if(my_flag){
  int n_valid_spots = next_valid_spots.size();
  int index = (rand() % n_valid_spots);
  Point p = next_valid_spots[index];
  fout << p.x << " " << p.y << std::endl;
}
else
  fout<<best_point.x<<" "<<best_point.y<<std::endl;
	fout.flush();
}

int main(int, char** argv) {
	std::ifstream fin(argv[1]);
	std::ofstream fout(argv[2]);
	read_board(fin);
	read_valid_spots(fin);
	write_valid_spot(fout);
	//system("pause");
	fin.close();
	fout.close();
	return 0;
}
