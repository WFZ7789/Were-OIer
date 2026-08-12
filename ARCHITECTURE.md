# Were-OIer v0.3 Architecture

## Product contract

Were-OIer is a deterministic, single-player social-deduction game. Its core promise is that information and speech cause observable changes in beliefs and votes. Random dialogue that does not affect decisions is presentation, not simulation.

## Boundaries

- `were_oier_core` owns rules, hidden information, belief updates, AI decisions and the event log.
- The terminal executable only translates player input into typed actions and renders visible events.
- A seed plus the same action sequence must reproduce the same game.
- The engine never reads from standard input, sleeps, clears the screen or calls platform APIs.
- Public outcomes may update beliefs; private role information must only reach its entitled viewer.

## First vertical slice

The v0.3 slice supports Villager, Werewolf, Seer, Witch and Hunter. It proves the night/discussion/vote loop, explainable AI votes, deterministic replay and safe terminal input. Magician, White Wolf King, achievements, shop and gift codes remain legacy content until they can use these contracts without bypassing them.

