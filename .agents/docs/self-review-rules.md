# Self-review rules

Project-specific rules for
[/self-review](https://github.com/eai-org/agent-toolkit/blob/main/skills/self-review/SKILL.md),
run on a PR before it is submitted or updated. PRs are human-authored; the reviewer checks, it
does not write. Generic improvements to the review process itself belong to the skill, not here.
The [code-review.md](code-review.md) rules apply on top, as do the core's
`../../.agents/docs/self-review-rules.md` rules on pinning the review to a commit.

## Regression risk

Automated test coverage is near zero and every bot runs the same code, so one change reaches
every bot on the server. Go deeper the farther it can reach:

- `src/Bot/`, `src/Ai/Base/`, and shared values: one change alters every class and every
  strategy — examine how the changed code is used elsewhere, not just the change itself.
- `src/Ai/Class/<Class>/`, `Dungeon/`, `Raid/`: impact is mostly contained to the class or
  encounter touched.
- SQL: watch for a `DELETE` or `UPDATE` whose `WHERE` catches rows it should not, and for a
  translation update that misses one of the nine locales.

## What the reviewer verifies itself

- Every new action, trigger, strategy, and value name resolves to a registered creator, and no
  registration is left orphaned (grep per `ai-engine.md`).
- New per-tick work is behind a gate or a cached value; a trigger that scans is a finding.
- No synchronous database query on a map-thread path.
- New `GetBotTextOrDefault` keys ship with their translation SQL.
- New config options are in `conf/playerbots.conf.dist` with a default, a comment, and the
  feature off by default when it is expensive or changes behaviour.
- The PR body's Feature Evaluation and Impact Assessment match the diff: per-tick work added, a
  default strategy set or default config value changed, or a new decision branch is not ticked
  "No".
- Ported code carries its upstream notice and the PR's Code Provenance names the source; a
  missing credit is a finding.

## In-game testing

The author tests in-game, which the reviewer cannot. Never guess what was tested — ask, and record
the answer in the report. Then name what else to test, especially side effects the author might
not expect: a change that adds per-tick work is tested with `playerbot pmon` before and after; a
fix on one class's rotation is checked against the other specs of that class; a change to a shared
action is checked in solo, group, and instance play. When the tested scenario matches the change's
main path, probe the branches it does not take.
