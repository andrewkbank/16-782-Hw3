# Symbolic planner

The executable supports three domain-independent search modes selected with the
`PLANNER_MODE` environment variable:

- `bfs`: breadth-first graph search (no heuristic)
- `astar-goal`: A* with the admissible normalized unsatisfied-goal count
- `astar-edl`: A* with the exact empty-delete-list cost `h+` (default)

PowerShell example:

```powershell
cmake -S . -B build
cmake --build build --config Release
$env:PLANNER_MODE = 'bfs'
.\build\Release\planner.exe TowersOfHanoi3.txt
```

For a single-config generator, the executable may instead be at
`.\build\planner.exe`.

Linux/autograder-style build:

```bash
mkdir -p build
cd build
g++ ../src/planner.cpp -std=c++11 -O2 -o planner
./planner Blocks.txt
```

The supplied Hanoi instances have known optimal plan lengths:

| Environment | Optimal length |
| --- | ---: |
| `TowersOfHanoi3.txt` | 7 |
| `TowersOfHanoi4.txt` | 15 |
| `TowersOfHanoi5.txt` | 31 |
| `TowersOfHanoi6.txt` | 63 |

Every run prints plan length, expanded states, generated and duplicate
successors, maximum frontier size, grounding time, search time, and total
wall-clock planning time.

## Planner optimizations

Grounded actions whose static preconditions are false in the initial state are
discarded before search. A predicate is considered static only when it never
appears in any action effect, so this pruning cannot remove an action that could
become applicable later.

The empty-delete-list mode computes `h+`: the exact shortest-plan cost in the
relaxed problem formed by removing every negative effect while retaining action
preconditions. It solves each relaxed problem with an inner A* search. An
event-driven `h_max` propagation supplies an admissible lower bound to that inner
search without changing the resulting exact relaxed-plan cost.

## Robot Courier domain

`RobotCourier.txt` is a second custom domain that is deliberately different
from a stacking problem. A robot moves along a bidirectional chain of four
locations and can carry one package at a time. Package `P1` starts at `B` and
must be delivered to `D`; package `P2` starts at `C` and must be delivered to
`A`. The `Destination` and `Connected` predicates are static, while `At`,
`HandEmpty`, `Carrying`, `Undelivered`, and `Delivered` change during planning.

The optimal plan contains 10 actions. It delivers `P1` first while traveling
from `A` toward `D`, then collects `P2` on the return trip to `A`.

`RobotCourierLarge.txt` is a stress-test instance with eight locations and four
packages. The locations form a bidirectional chain from `A` through `H`:

- `P1`: `B` to `H`
- `P2`: `G` to `A`
- `P3`: `C` to `F`
- `P4`: `E` to `B`

The action schemas are unchanged, so this instance directly tests how grounding
and search scale with more symbols, package-order choices, and longer routes.
Breadth-first search verifies that its optimal plan contains 30 actions.
