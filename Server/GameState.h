#include <vector>
#include <mutex>

using namespace std;

mutex m;

int num_players = 0;
int num_spectators = 0;

bool start_game = false;
// attacks made by each player
vector<vector<pair<int, int>>> player_attempts{};