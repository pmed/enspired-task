# enspired-task

Apartment And Chair Delivery Limited has a unique position on the housing market. The company not only builds apartments, but also equips them with chairs.
Now the business has grown continuously over the past few years and there are a few organizational problems that could be solved by automation.
We will focus on one of them here:

While a new residential building is erected, the chairs that are to be placed there need to be produced. In order to be able to plan this, the home buyers indicate the desired position of the armchairs in their home on a floor plan at the time of purchase. These plans are collected, and the number of different chairs to be produced are counted from them. The plans are also used to steer the workers carrying the chairs into the building when furnishing the apartments.

In the recent past, when manually counting the various types of chairs in the floor plans, many mistakes were made and caused great resentment among customers. That is why the owner of the company asked us to automate this process.

Unfortunately, the plans are in a very old format (the company's systems are still from the eighties), so modern planning software cannot be used here. An example of such an apartment plan is attached.

We now need a command line tool that reads in such a file and outputs the following information:
- Number of different chair types for the apartment
- Number of different chair types per room

The different types of chairs are as follows:
W: wooden chair
P: plastic chair
S: sofa chair
C: china chair

The output must look like this so that it can be read in with the old system:
```
total:
W: 3, P: 2, S: 0, C: 0
living room:
W: 3, P: 0, S: 0, C: 0
office:
W: 0, P: 2, S: 0, C: 0
```

The names of the rooms must be sorted alphabetically in the output.

Our sales team has promised Apartment And Chair Delivery Limited a solution within 5 days from now. I know that is very ambitious, but as you are our best developer, we all count on you.

# Python implementation notes

The task solution is following:

1. Load floor plan from a text file, assuming the file format in valid, and the plan has all required data.

2. Find all rooms by `(room name)` string pattern.

3. Find and count all known chair types for each room using non-recursive [flood fill algorithm][1] with 4 directions (up, down, left, right) for breadth-first search starting at the room name position that was found at step 2. 

[1]: https://en.wikipedia.org/wiki/Flood_fill

There is a `Plan` class in `chairs-planner.py` with the following methods:
  - `read(filename)`
  - `find_rooms()`
  - `find_chairs(room)`

### Plan.read(filename)

Loads a floor plan from a text file. The plan is ASCII pseudo-graphics like
```
+----------+-----------+
| (room 1) |   P       |
|   W    S | C (room2) |
+----------+-----------+
|(room3)  /
+--------+
```

There would be handling of invalid plan structure (inconsistent walls, unknown chair types) in real project.

### Plan.find_rooms()

A room name is a single line text inside parenthesis. We use regexp to find it. Room names are stored with stripped spaces around, and along with the name position on the plan.

This method modifies plan cells by erasing room names after finding them. This allows to avoid edge cases when rooms have names equal to the chair types, e.g. `(room P)` or `(W is not chair here)`

The found rooms are stored in `Plan` instance as a list of `Room` objects, sorted by the room name attribute. 

### Plan.find_chairs(room)

This method uses [Flood-fill algorithm][1] to find all chairs in the specified `room` limited by walls, using the room name as starting point.

This method modifies plan by setting special value `X` for already visited cells.

All known chairs types found during the visiting, will be counted by the chair type in the `room` object. 


## Running

All the code is placed in a single `chairs-planner.py` Python3 file. It accepts a file name as a command-line argument, or reads standard input when no file name was supplied:

```
# read from a file
$ python3 chairs-planner.py testdata/rooms.txt

# redirect file to stdin
$ python3 chairs-planner.py < testdata/rooms.txt
```

Python `unittest` module is used for testing:
```
$ python3 -m unittest chairs-planner.py
``` 

# C++ implementation notes

It's very similar to the Python one. All the code is also in the single `chairs-planner.cpp` file.


## Building

Only a C++17 compiler is needed. To compile `chairs-planner` program call `clang++` or `g++` like
```
$ c++ -std=c++17 chairs-planner.cpp -o chairs-planner
``` 

## Running

Program `chairs-planner` accepts a file name as a command-line argument, or reads standard input when no file name was supplied:

```
# read from a file
$ ./chairs-planner testdata/rooms.txt

# redirect file to stdin
$ ./chairs-planner < testdata/rooms.txt
```

There are several unit tests in the program embedded. These tests could be run with `--test` command-line option:
```
$ ./chairs-planner --test
``` 
