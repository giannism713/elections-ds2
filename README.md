# Elections Management System (CS240 Project Phase 2)

## Overview

This project is an Elections Management System developed for the CS240 (Data Structures) course, Phase 2, Winter Semester 2024-2025. It simulates the organization and management of parliamentary elections, including the creation of districts, stations, parties, candidates, voter registration, voting, and the formation of parliament.

## Features

- **District and Station Management:**
	- Create electoral districts and assign seats.
	- Create voting stations and associate them with districts.
- **Voter and Candidate Registration:**
	- Register voters to specific stations.
	- Register candidates to parties and districts.
- **Voting Process:**
	- Record votes for candidates and parties.
	- Count votes per district.
- **Parliament Formation:**
	- Form parliament based on election results.
	- Print information about districts, stations, parties, and the parliament.
- **Bonus Features:**
	- Unregister voters and free allocated memory (for bonus tasks).

## Usage

The program reads commands from an input file, where each line represents an action (e.g., create district, register voter, cast vote, etc.). The main logic is implemented in `main.c`, which parses the input and calls the appropriate event functions declared in `voting.h` and implemented in `voting.c`.

## Files

- `main.c`: Main program logic and input parsing.
- `voting.h`: Structure and function declarations for all election events.
- `voting.c`: Implementations of the event functions (to be completed by the user).
- `Makefile`: Build instructions.

## Compilation

Use the provided `Makefile` to compile the project:

```bash
make
```

## Running

Run the program with an input file:

```bash
./main <input_file>
```
# elections-ds2