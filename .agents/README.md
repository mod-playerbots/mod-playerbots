# .agents

Source of truth for this module's agent skills and docs, laid out like the AzerothCore core's
`.agents/` so one convention covers both trees:

- `skills/<name>/SKILL.md`
- `docs/` — task-scoped agent docs; taxonomy and placement policy in `docs/README.md`
- `plans/` — per-task planning docs, gitignored

Conventions live in `AGENTS.md` and `docs/`. Agent-specific files point there instead of restating
them, so there is one copy to keep current.

## Relationship to the core's `.agents/`

The module is always checked out at `modules/mod-playerbots/` inside an AzerothCore fork, so the
core's docs are reachable at `../../.agents/docs/`. The module tree is self-contained anyway:
`docs/cpp-guidelines.md` is a verbatim copy of the core's, and the other docs hold what is specific
to the module. Where module and core docs differ, the module docs win for paths under the module.

## Hooking up your agent

- **Claude Code** — reads `CLAUDE.md`, which imports `AGENTS.md`. Skills are exposed by a relative
  symlink: `.claude/skills/<name> -> ../../.agents/skills/<name>`.
- **GitHub Copilot** — reads `AGENTS.md` natively.
- **Any other agent** — point it at `AGENTS.md` through its own entry file, or tell it to read
  `AGENTS.md` first.

On Windows, symlinks need `git config core.symlinks true` plus Developer Mode or an elevated shell;
without them git checks the links out as plain text files. A one-line `SKILL.md` that says
"read `.agents/skills/<name>/SKILL.md`" works everywhere.
