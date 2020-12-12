#include "libs.hpp"

// https://adventofcode.com/2020/day/12

// Position

struct Position {
    int x;
    int y;

    void move(char direction, int value) {
        switch (direction) {
            case 'N': y += value; break;
            case 'S': y -= value; break;
            case 'E': x += value; break;
            case 'W': x -= value; break;
        }
    }

    void rotateLeft() {
        std::swap(x, y);
        x = -x;
    }

    void rotateRight() {
        std::swap(x, y);
        y = -y;
    }

    void advance(Position other, int value) {
        x += other.x * value;
        y += other.y * value;
    }

    [[nodiscard]] static int abs(int x) noexcept { return x < 0 ? -x : x; }
    [[nodiscard]] int manhattan() const noexcept { return abs(x) + abs(y); }
};


// Moving

struct Command {
    char command;
    int value;

    static Command from_string(const std::string & s) {
        return {
            .command = s[0],
            .value = std::stoi(s.substr(1))
        };
    }
};

template <typename Ferry>
int the_ferry_hype_has_no_brakes(const std::vector<Command> & commands) {
    const Ferry ferry = std::for_each(commands.begin(), commands.end(), Ferry());
    return ferry.manhattan();
}


// Movement strategies

class Individual {
    Position self = Position{ 0, 0 };
    static constexpr const char * const Directions = "NESW";
    int direction_id = 1;
public:
    [[nodiscard]] int manhattan() const noexcept { return self.manhattan(); }

    void operator()(Command command) {
        if (command.command == 'L') {
            direction_id = (direction_id - command.value / 90) % 4;
        } else if (command.command == 'R') {
            direction_id = (direction_id + command.value / 90) % 4;
        } else if (command.command == 'F') {
            self.move(Directions[direction_id], command.value);
        } else {
            self.move(command.command, command.value);
        }
    }
};

class Waypoint {
    Position self     = Position{ 0, 0 };
    Position waypoint = Position{ 10, 1 };

public:
    [[nodiscard]] int manhattan() const noexcept { return self.manhattan(); }

    void operator()(Command command) {
        if (command.command == 'L') {
            waypoint.rotateLeft();
        } else if (command.command == 'R') {
            waypoint.rotateRight();
        } else if (command.command == 'F') {
            self.advance(waypoint, command.value);
        } else {
            waypoint.move(command.command, command.value);
        }
    }
};


// Main

Output day12(const std::vector<std::string> & lines) {
    const std::vector<Command> commands = lines_transform::map<Command>(lines, Command::from_string);

    return Output {
        the_ferry_hype_has_no_brakes<Individual>(commands),
        the_ferry_hype_has_no_brakes<Waypoint>(commands)
    };
}
