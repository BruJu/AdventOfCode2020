# Advent of Code 2020

My answers to https://adventofcode.com/2020 written in C++

Architecture is a bit wonky: the goal is to have an easy to compile / run process by using only one command line.

## Test / Compile / Run

**The bash way**
- `./run.sh` will run the last day
- `./run.sh 0` will run every day
- `./run.sh x` will run day x

**The standard way**
- You can also compile with `make`.
- And run with `./main ([0-9]+)?`

## Adding new days

- Add input files in `inputs/`
- `config.txt` : For each input, add a line `DAYID input_path part1_result part2_result`. Write ? instead of a number if unknown. Write `\_ for tests which result is unknwon and not required.
- `src/main.cpp` : add function declaration for `dayXX() ` and add in `dispatch()` the handler
- Create a `src/dayXX.cpp` file.

## License

WTFPL ¯\\\_( ツ )\_/¯

Generic Makefile distributed under the MIT License (see https://github.com/mbcrawfo/GenericMakefile for more information)
