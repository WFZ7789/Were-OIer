# Development Contract

- Preserve deterministic behavior: all gameplay randomness comes from the game seed.
- Keep hidden information private in the event log and public APIs.
- Express player and AI decisions as validated actions; do not mutate state from UI code.
- Every bug fix adds a regression test, especially hangs, invalid input and elimination chains.
- Do not migrate legacy content until it has a gameplay purpose and fits the new action/event model.
- A stage is complete only when Debug and Release tests pass and the terminal vertical slice remains playable.

