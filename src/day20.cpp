#include "libs.hpp"

#include <algorithm>
#include <vector>
#include <variant>
#include <memory>
#include <set>
#include <map>
#include <regex>
#include <ostream>
#include <cmath>

// https://adventofcode.com/2020/day/20

namespace part_a {
    using Raw = std::vector<std::vector<char>>;

    struct Transformations {
        bool is_flipped_left_right = false;
        bool is_flipped_bottom_up = false;
        int is_rotate_right = 0;

        friend std::ostream & operator<<(std::ostream & stream, const Transformations & trans) {
            stream << "FlippedRL=" << trans.is_flipped_left_right
                << " ; FlippedTB=" << trans.is_flipped_bottom_up
                << " ; rotate= " << trans.is_rotate_right;
            return stream;
        }
    };

    struct Path;

    struct Tile {
        int id;
        std::vector<std::vector<char>> image;

        explicit Tile(int id) : id(id) {}

        [[nodiscard]] const std::vector<char> & operator[](size_t x) const { return image[x]; }

        friend std::ostream & operator<<(std::ostream & stream, const Tile & tile) {
            stream << "== Tile " << tile.id << "\n";

            for (const auto & line : tile.image) {
                for (const auto pixel : line) {
                    stream << pixel;
                }

                stream << '\n';
            }

            stream << '\n';
            return stream;
        }

        [[nodiscard]] std::vector<Path> adjacent(const Tile & other_tile) const;
    };

    struct Path {
        int my_rotation;
        Transformations his_transformations;

        friend std::ostream & operator<<(std::ostream & stream, const Path & path) {
            stream << "left= " << path.my_rotation << " ; " << path.his_transformations;
            return stream;
        }
    };

    using Paths = std::vector<Path>;
    using AllPaths = std::map<int, std::map<int, Paths>>;

    struct AltereredTile {
        Tile representation;
        Transformations transformations;

        AltereredTile(Tile tile) : representation(std::move(tile)) {}

        // ok
        void flip_left_right() {
            transformations.is_flipped_left_right = !transformations.is_flipped_left_right;

            for (auto & line : representation.image) {
                std::vector<char> new_line { line.rbegin(), line.rend() };
                line = std::move(new_line);
            }
        }

        // ok
        void flip_bottom_up() {
            transformations.is_flipped_bottom_up = !transformations.is_flipped_bottom_up;

            std::vector<std::vector<char>> image { representation.image.rbegin(), representation.image.rend() };
            representation.image = std::move(image);
        }

        // ok
        void rotate_right() {
            transformations.is_rotate_right = (transformations.is_rotate_right + 1) % 4;

            std::vector<std::vector<char>> image;

            for (size_t y = 0 ; y != representation.image.size() ; ++y) {
                image.push_back({});
                for (size_t x = 0 ; x != representation.image[y].size() ; ++x) {
                    image.back().push_back(representation[representation.image.size() - x - 1][y]);
                }
            }

            representation.image = std::move(image);
        }

        friend std::ostream & operator<<(std::ostream & stream, const AltereredTile & tile) {
            stream << tile.representation << tile.transformations << "\n\n";
            return stream;
        }

        [[nodiscard]] bool adjacent_left_to_right(const AltereredTile & other_tile) const;
        [[nodiscard]] bool adjacent_top_to_down(const AltereredTile & other_tile) const;
    };

    struct TileStateExplorer {
        int i = 0;

        operator bool() {
            return i != 2 * 2 * 4;
        }

        void operator()(AltereredTile & tile) {
            tile.rotate_right();

            if (i % 4 == 0) {
                tile.flip_left_right();
            }
            if (i % 8 == 0) {
                tile.flip_bottom_up();
            }

            ++i;
        }
    };


    static bool connect(const std::vector<std::vector<char>> & lhs, const std::vector<std::vector<char>> & rhs) {
        for (size_t i = 0 ; i != lhs.size() ; ++i) {
            if (lhs[i].back() != rhs[i].front()) {
                return false;
            }
        }

        return true;
    }


    bool AltereredTile::adjacent_left_to_right(const AltereredTile & other_tile) const {
        return connect(representation.image, other_tile.representation.image);
    }

    bool AltereredTile::adjacent_top_to_down(const AltereredTile & other_tile) const {
        return representation.image.back() == other_tile.representation.image.front();
    }


    std::vector<Path> Tile::adjacent(const Tile & other_tile) const {
        std::vector<Path> paths;

        AltereredTile me { *this };

        std::set<Raw> my_raws;

        for (size_t i = 0 ; i != 4 ; ++i, me.rotate_right()) {
            if (my_raws.contains(me.representation.image)) {
                continue;
            }

            my_raws.insert(me.representation.image);

            std::set<Raw> other_raws;
            AltereredTile other { other_tile };

            for (TileStateExplorer trans ; trans ; trans(other)) {
                if (!other_raws.contains(other.representation.image)) {
                    other_raws.insert(other.representation.image);

                    if (connect(me.representation.image, other.representation.image)) {
                        paths.push_back(Path { me.transformations.is_rotate_right, other.transformations });
                    }
                }
            }
        }

        return paths;
    }


    std::vector<Tile> to_tiles(const std::vector<std::string> & lines) {
        std::vector<Tile> retval;

        for (const auto & line : lines) {
            if (line == "") continue;

            if (line.rfind("Tile ", 0) == 0) {
                std::string id { line.begin() + 5, line.end() - 1 };
                retval.emplace_back(std::stoi(id));
            } else {
                retval.back().image.emplace_back();

                for (const auto c : line) {
                    retval.back().image.back().emplace_back(c);
                }
            }
        }

        return retval;
    }


    struct TempGrid {
        struct TTile {
            const Tile * tile;
            bool used = false;
        };

        std::vector<std::vector<std::optional<AltereredTile>>> m_grid;
        std::vector<TTile> tiles;
        const AllPaths & all_paths;
        size_t m_size;

        explicit TempGrid(const std::vector<Tile> & p_tiles, const AllPaths & all_paths) : all_paths(all_paths) {
            for (const auto & tile : p_tiles) {
                tiles.push_back(TTile { &tile, false });
            }

            const size_t size = static_cast<size_t>(std::sqrt(tiles.size()));

            for (size_t i = 0 ; i != size; ++i) {
                m_grid.emplace_back();
                for (size_t j = 0 ; j != size ; ++j) {
                    m_grid.back().emplace_back(std::nullopt);
                }
            }
            m_size = size;
        }

        bool resolve() {
            return try_place_at(0, 0);
        }

        [[nodiscard]] size_t min_required_path(size_t x, size_t y) {
            size_t req = 2;

            if (x != 0 && x != m_size) ++req;
            if (y != 0 && y != m_size) ++req;

            return req;
        }

        bool compatible(size_t x, size_t y) const {
            if (x != 0 && m_grid[y][x - 1] && !m_grid[y][x - 1]->adjacent_left_to_right(*m_grid[y][x])) {
                return false;
            }

            if (y != 0 && m_grid[y - 1][x] && !m_grid[y - 1][x]->adjacent_top_to_down(*m_grid[y][x])) {
                return false;
            }

            if (x != m_size - 1 && m_grid[y][x + 1] && !m_grid[y][x]->adjacent_left_to_right(*m_grid[y][x + 1])) {
                return false;
            }

            if (y != m_size - 1 && m_grid[y + 1][x] && !m_grid[y][x]->adjacent_top_to_down(*m_grid[y + 1][x])) {
                return false;
            }

            return true;
        }

        bool try_place_at(size_t x, size_t y) {
            //if (y == m_size) return true;
            //const size_t next_x = x != m_size - 1 ? x + 1 : 0;
            //const size_t next_y = x != m_size - 1 ?     y : y + 1;

            if (x == m_size * 2 + 1) return true;
            const size_t next_x = x != 0 ? x - 1 : y + 1;
            const size_t next_y = x != 0 ? y + 1 : 0;
            if (x >= m_size || y >= m_size) {
                return try_place_at(next_x, next_y);
            }

            for (auto & [tile, used] : tiles) {
                if (used) continue;

                used = true;

                //const size_t number_of_paths = all_paths.find(tile.id)->second.size();
                //if (min_required_path(x, y) > number_of_paths) continue;

                //if (x != 0 && m_grid[y][x - 1] && no_hope(tile.id, m_grid[y][x - 1]->representation.id)) continue;
                //if (y != 0 && m_grid[y - 1][x] && no_hope(tile.id, m_grid[y - 1][x]->representation.id)) continue;

                m_grid[y][x] = AltereredTile { *tile };

                for (TileStateExplorer trans ; trans ; trans(*m_grid[y][x])) {
                    if (!compatible(x, y)) {
                        continue;
                    }

                    if (try_place_at(next_x, next_y)) {
                        return true;
                    }
                }

                m_grid[y][x] = std::nullopt;
                used = false;
            }

            return false;
        }
    };

    struct Grid {
        std::vector<std::vector<int>> m_grid;
        std::vector<std::vector<Raw>> m_raws;

        explicit Grid(const TempGrid & temp_grid) {
            for (const auto & line : temp_grid.m_grid) {
                m_grid.emplace_back();
                m_raws.emplace_back();
                for (const auto & value : line) {
                    m_grid.back().emplace_back(value->representation.id);
                    m_raws.back().emplace_back(value->representation.image);
                }
            }
        }

        [[nodiscard]] test::Value corners() const noexcept {
            return test::Value(m_grid.front().front())
                * test::Value(m_grid.front().back())
                * test::Value(m_grid.back().front())
                * test::Value(m_grid.back().back());
        }


        static std::optional<Grid> make(const std::vector<Tile> & tiles, const AllPaths & all_paths) {
            TempGrid temp_grid { tiles, all_paths };

            if (temp_grid.resolve()) {
                return Grid(temp_grid);
            } else {
                return std::nullopt;
            }
        }
    };

    std::optional<Grid> solve(const std::vector<std::string> & lines) {
        std::vector<Tile> tiles = to_tiles(lines);

        std::map<int, std::map<int, Paths>> all_paths;

        for (const auto & tile : tiles) {
            for (const auto & tile_ : tiles) {
                if (&tile != &tile_) {
                    const auto paths = tile.adjacent(tile_);
                    if (!paths.empty()) {
                        all_paths[tile.id][tile_.id] = paths;
                    }
                }
            }
        }

        // Heuristic : We should start with pieces that are connected to few others
        std::sort(tiles.begin(), tiles.end(), 
            [&](const Tile & lhs, const Tile & rhs) {
                size_t lhs_score = all_paths[lhs.id].size();
                size_t rhs_score = all_paths[rhs.id].size();

                return lhs_score < rhs_score;
            }
        );

        return Grid::make(tiles, all_paths);
    }
}

namespace part_b {
    constexpr const char * monster_t = "                  # ";
    constexpr const char * monster_m = "#    ##    ##    ###";
    constexpr const char * monster_b = " #  #  #  #  #  #   ";
    constexpr size_t monster_width = 20;
    constexpr size_t monster_height = 3;

    using part_a::Tile;
    using part_a::Raw;

    Tile rebuild_tile(const part_a::Grid & grid) {
        Tile tile { 0 };

        const size_t tile_size = grid.m_raws[0][0].size();

        for (const auto & line_of_raws : grid.m_raws) {
            const size_t tile_y = tile.image.size();
            for (size_t i = 0 ; i != tile_size - 2 ; ++i) {
                tile.image.push_back({});
            }

            for (const auto & raw : line_of_raws) {

                for (size_t y = 1 ; y != raw.size() - 1; ++y) {
                    for (size_t x = 1 ; x != raw[y].size() - 1; ++x) {
                        tile.image[tile_y + y - 1].emplace_back(raw[y][x]);
                    }
                }
            }
        }

        return tile;
    }

    bool is_here(Raw & raw, size_t x, size_t y, bool replace = false) {
        const auto point = [&](char & reality, char monster, bool replace) -> bool {
            if (monster == ' ') {
                return true;
            }

            if (reality == '#' || reality == 'R') {
                if (replace) {
                    reality = 'R';
                }

                return true;
            } else {
                return false;
            }
        };

        for (size_t i = 0 ; i != monster_width ; ++i) {
            if (!point(raw[y    ][x + i], monster_t[i], replace)) return false;
            if (!point(raw[y + 1][x + i], monster_m[i], replace)) return false;
            if (!point(raw[y + 2][x + i], monster_b[i], replace)) return false;
        }

        return true;
    }

    bool search_monster(Raw & raw, size_t x, size_t y) {
        if (is_here(raw, x, y)) {
            is_here(raw, x, y, true);
            return true;
        } else {
            return false;
        }
    }

   std::pair<size_t, Raw> search_monster(Raw raw) {
        for (size_t x = 0 ; x + monster_width  != raw.size() ; ++x) {
        for (size_t y = 0 ; y + monster_height != raw.size() ; ++y) {
            search_monster(raw, x, y);
        }}

        size_t rought = 0;
        for (const auto & line : raw) {
            for (const char c : line) {
                if (c == '#') {
                    rought += 1;
                }
            }
        }

        return std::pair<size_t, Raw>(rought, raw);
    }

    size_t solve(const part_a::Grid & grid) {
        Tile big_picture = rebuild_tile(grid);

        part_a::AltereredTile other { big_picture };

        size_t found = 9999999;
        for (part_a::TileStateExplorer trans ; trans ; trans(other)) {
            auto [found_monsters, raw] = search_monster(other.representation.image);
            found = std::min(found, found_monsters);
        }

        return found;
    }
}

Output day20(const std::vector<std::string> & lines, const DayExtraInfo &) {
    std::optional<part_a::Grid> grid = part_a::solve(lines);
    if (!grid) return Output(0, 0);

    const size_t roughness = part_b::solve(*grid);
    return Output(grid->corners(), roughness);
}