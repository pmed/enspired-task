#include <iostream>
#include <fstream>
#include <sstream>

#include <algorithm>
#include <array>
#include <vector>
#include <map>
#include <queue>

#include <regex>
#include <string>
#include <stdexcept>

#include "test.hpp"

constexpr auto ChairTypes = std::array{ 'W', 'P', 'S', 'C' };
constexpr auto WallTypes = std::array{ '+', '-', '|', '\\', '/', '\n' };
constexpr auto Visited = 'X';

int chair_type(char c) {
    for (int i = 0; i < ChairTypes.size(); ++i) {
        if (c == ChairTypes[i]) {
            return i;
        }
    }
    return -1;
}

bool is_wall(char c) {
    return std::find(WallTypes.begin(), WallTypes.end(), c) != WallTypes.end(); 
}

std::string trim(std::string str) {
    const auto beg = std::find_if(str.begin(), str.end(), [](char c){ return !isspace(c); });
    const auto end = std::find_if(str.rbegin(), std::string::reverse_iterator{beg}, [](char c){ return !isspace(c); }).base();
    return std::string{beg, end};
}

struct Pos {
    ssize_t x = 0;
    ssize_t y = 0;
    std::string str() const {
        return "(" + std::to_string(x) + ", " + std::to_string(y) + ")";
    }

    // for tests
    bool operator==(const Pos& other) const {
        return this->x == other.x && this->y == other.y;
    }
    friend std::ostream& operator<<(std::ostream& os, const Pos& pos) {
        return os << "(" << pos.x << ", " << pos.y << ")";
    }
};

using ChairCount = std::array<size_t, std::size(ChairTypes)>;

struct Room {
    Pos pos;
    ChairCount chairs{};

    std::string chairs_str(const std::string& name) const {
        std::string str = name;
        if (!name.empty()) str += ":\n";
        const char* delim = "";
        for (size_t i = 0; i < chairs.size(); ++i) {
            str += delim;
            str += ChairTypes[i];
            str += ": ";
            str += std::to_string(chairs[i]);
            delim = ", ";
        }
        return str;
    }

    // for tests
    bool operator==(const Room& other) const {
        return this->pos == other.pos && this->chairs == other.chairs;
    }
    friend std::ostream& operator<<(std::ostream& os, const Room& room) {
        return os << "pos: " << room.pos << ", chairs: " << room.chairs_str("");
    }
};
using Rooms = std::map<std::string, Room>;

std::ostream& operator<<(std::ostream& os, const Rooms& rooms) {
    for (const auto& [name, room] : rooms) {
        os << name << ": " << room << '\n';
    }
    return os;
}

class Plan {
private:
    std::vector<std::string> plan;
    Rooms rooms;
public:
    void clear() {
        plan.clear();
        rooms.clear();
    }

    void read(std::istream& input) {
        for (std::string line; std::getline(input, line); line.clear()) {
            plan.push_back(std::move(line));
        }
    }

    Rooms& find_rooms() {
        const std::regex pattern("\\(([^)]*)\\)");
        ssize_t y = 0;
        for (auto& line : plan) {
            for (auto it = std::sregex_iterator{line.begin(), line.end(), pattern}, end = std::sregex_iterator{}; it != end; ++it) {
                const auto& match = *it;
                const auto name = trim(match.str(1));
                const auto pos = Pos{match.position(), y};
                if (name.empty()) {
                    throw std::runtime_error("Empty room name at " + pos.str());
                }
                if (const auto it = rooms.find(name); it != rooms.end()) {
                    throw std::runtime_error("Duplicate room name " + name + ", initially defined at " + it->second.pos.str());

                }
                rooms[name].pos = pos;
                std::fill_n(line.begin() + match.position(), match.length(), ' '); // erase room name in the plan
            } 
            ++y;
        }
        return rooms;
    }

    void find_chairs(Room& room) {
        // Use non-recursive flood fill algorithm with 4 directions
        // (see https://en.wikipedia.org/wiki/Flood_fill)
        // Visited cells will be marked as X on the plan
        const auto directions = { Pos{0, 1}, Pos{0, -1}, Pos{1, 0}, Pos{-1, 0} };
        std::queue<Pos> q;
        q.push(room.pos);
        while (!q.empty()) {
            auto pos = q.front(); q.pop();
            auto& cell = plan[pos.y][pos.x];
            if (cell == Visited) {
                continue;
            } else if (const int type = chair_type(cell); type >= 0) {
                room.chairs[type] += 1;
            }
            cell = Visited;
            for (const auto& [dx, dy] : directions) {
                const Pos new_pos{pos.x + dx, pos.y + dy};
                if (0 <= new_pos.y && new_pos.y < plan.size() && 0 <= new_pos.x && new_pos.x < plan[new_pos.y].size()) {
                    const auto cell = plan[new_pos.y][new_pos.x];
                    if (cell != Visited && !is_wall(cell)) {
                        q.push(new_pos);
                    }
                }
            }
        }
    }
};

bool test_trim() {
    auto test = [](std::string str, std::string expected) {
            return TestCase{str, [=]{ return trim(str) == expected; } };
    };
    const auto cases = {
        test("", ""),
        test("    ", ""),
        test(" aa bb c", "aa bb c"),
        test("aaa    bbb cc    ", "aaa    bbb cc"),
        test("   a   bb    ccc     ", "a   bb    ccc"),
    };
    return run(cases, "\n  ");
}

bool test_is_wall() {
    auto test = [](std::string str, bool expected = true) {
            return TestCase{str, [=]{ return std::all_of(str.begin(), str.end(), is_wall) == expected; } };
    };
    const auto cases = {
        test("+----+"),
        test("|/\\"),
        test(" \t   cc", false),
        test(" Q asdf P R S W", false),
    };
    return run(cases, "\n  ");
}

bool test_char_type() {
    auto test = [](char chair, int type) {
            return TestCase{std::string{"chair "} + chair, [=]{ return chair_type(chair) == type; } };
    };
    const auto cases = {
        test('A', -1),
        test('-', -1),
        test('|', -1),
        test(' ', -1),
        test('W', 0),
        test('P', 1),
        test('S', 2),
        test('C', 3),
    };
    return run(cases, "\n  ");
}

bool test_room() {
    const auto cases = {
        TestCase{"ctor", []{
            Room room;
            return room.pos.x == 0
                && room.pos.y == 0
                && std::all_of(room.chairs.begin(), room.chairs.end(), [](size_t count) { return count == 0; });
        } },
    };
    return run(cases, "\n  ");
}

bool test_plan() {
    const auto test = [](std::string name, std::string data, Rooms expected, const bool fail = false) {
        return TestCase{name, [=] {
            try {
                Plan plan;
                std::istringstream input(data);
                plan.read(input);
                Rooms found;
                for (auto& [name, room] : plan.find_rooms()) {
                    plan.find_chairs(room);
                    found[name] = room;
                }
                if (fail) {
                    throw std::runtime_error("Exception expected");
                }
                if (found != expected) {
                    std::cerr << "found:" << found << "\n != expected:\n" << expected << "\n";
                    return false;
                }
                return true;
            } catch (const std::exception& ex) {
                if (fail) {
                    return true;
                }
                throw;
            }
        }};
    };

    const auto rooms = R"(
+-----------+------------------------------------+
|           |                                    |
| (closet)  |                                    |
|         P |                            S       |
|         P |         (sleeping room)            |
|         P |                                    |
|           |                                    |
+-----------+    W                               |
|           |                                    |
|        W  |                                    |
|           |                                    |
|           +--------------+---------------------+
|                          |                     |
|                          |                W W  |
|                          |    (office)         |
|                          |                     |
+--------------+           |                     |
|              |           |                     |
| (toilet)     |           |             P       |
|   C          |           |                     |
|              |           |                     |
+--------------+           +---------------------+
|              |           |                     |
|              |           |                     |
|              |           |                     |
| (bathroom)   |           |      (kitchen)      |
|              |           |                     |
|              |           |      W   W          |
|              |           |      W   W          |
|       P      +           |                     |
|             /            +---------------------+
|            /                                   |
|           /                                    |
|          /                          W    W   W |
+---------+                                      |
|                                                |
| S                                   W    W   W |
|                (living room)                   |
| S                                              |
|                                                |
|                                                |
|                                                |
|                                                |
+--------------------------+---------------------+
                           |                     |
                           |                  P  |
                           |  (balcony)          |
                           |                 P   |
                           |                     |
                           +---------------------+
)"; 

    const auto cases = {
        TestCase{"ctor", []{
            Plan plan;
            return plan.find_rooms().empty();
        } },
        test("empty", "", Rooms{}),
        test("rooms.txt", rooms, Rooms{
            // { name, Room{ pos, chairs: W P S C } }
            { "balcony",       Room{ Pos{30, 47}, ChairCount{ 0, 2, 0, 0 } } },
            { "bathroom",      Room{ Pos{ 2, 26}, ChairCount{ 0, 1, 0, 0 } } },
            { "closet",        Room{ Pos{ 2,  3}, ChairCount{ 0, 3, 0, 0 } } },
            { "kitchen",       Room{ Pos{34, 26}, ChairCount{ 4, 0, 0, 0 } } },
            { "living room",   Room{ Pos{17, 38}, ChairCount{ 7, 0, 2, 0 } } },
            { "office",        Room{ Pos{32, 15}, ChairCount{ 2, 1, 0, 0 } } },
            { "sleeping room", Room{ Pos{22,  5}, ChairCount{ 1, 0, 1, 0 } } },
            { "toilet",        Room{ Pos{ 2, 19}, ChairCount{ 0, 0, 0, 1 } } },
        }),
    };
    return run(cases, "\n  ");
}
int main(int argc, char* argv[]) try {
    const std::string filename = (argc > 1 ? argv[1] : "");
    if (filename == "--test") {
        // simple tests runner
        const auto tests = {
            TestCase{"trim", test_trim},
            TestCase{"is_wall", test_is_wall},
            TestCase{"chair_type", test_char_type},
            TestCase{"room", test_room},
            TestCase{"plan", test_plan},
        };
        return run(tests) ? 0 : 1;
    }

    // read plan
    std::ifstream file(filename);
    Plan plan;
    plan.read(filename.empty() ? std::cin : file);

    // find and count chairs
    std::map<std::string, Room> chairs_per_room;
    Room& total = chairs_per_room[""]; // pseudo room for total count

    for (auto& [name, room] : plan.find_rooms()) {
        plan.find_chairs(room);
        chairs_per_room[name] = room;
        for (size_t i = 0; i < room.chairs.size(); ++i) {
            total.chairs[i] += room.chairs[i];
        }
    }

    // print results
    for (const auto& [name, room] : chairs_per_room) {
        std::cout << room.chairs_str(name.empty() ? "total" : name) << std::endl;
    }
    return 0;
} catch (const std::exception& ex) {
    std::cerr << ex.what() << std::endl;
    return -1;
}