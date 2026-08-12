# Were-OIer v0.3

Were-OIer is being rebuilt as a deterministic, cross-platform social-deduction game. This branch is the first playable vertical slice of the new engine; the 2024 single-file version remains available as legacy history.

## What is playable

- Ten seats with Villager, Werewolf, Seer, Witch and Hunter.
- A complete night → discussion → vote loop.
- Structured accusations, defenses, role claims and questions.
- Per-agent suspicion and credibility that directly determine AI votes.
- Public vote reasons and private Seer/Witch information.
- Seeded deterministic games and safe line-based terminal input.

The first slice deliberately does not migrate Magician, White Wolf King, achievements, the role shop or gift codes. They return only after the deduction loop is stable and each feature has a gameplay purpose.

## Build and run

Requirements: CMake 3.20+, Ninja and a C++20 compiler.

```sh
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
./build/debug/were_oier 20260812
```

The optional number is the game seed. Identical seeds and action sequences reproduce the same match.

## Project map

- `include/were_oier/`: typed game contracts.
- `src/game.cpp`: rules, hidden information, beliefs, AI and event log.
- `src/main.cpp`: terminal adapter only.
- `tests/`: deterministic and legacy-regression coverage.
- `ARCHITECTURE.md`: invariants for the rebuild.
- `ROADMAP.md`: staged content migration and deduction depth.

