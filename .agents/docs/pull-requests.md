# Commits and pull requests

## Branches

PRs target `test-staging`, never `master`. `master` only receives promotion PRs from
`test-staging`, opened by maintainers; `check_pr_source.yml` rejects anything else.

## Commit messages

`.git_commit_template.txt` is the format:

- Subject: `type(Scope/Subscope): short summary`, imperative, capitalised, no trailing period, max 72 characters. Types: `feat`, `fix`, `refactor`, `style`, `docs`, `test`, `chore`. Copy
  scope and subscope naming from recent commits touching the same files
  (`git log --oneline -- <paths>`).
- Blank line, then the why (what was wrong, what changes), wrapped at 72 columns.
- Links to issues, upstream commits, or references last.

## Upstream credit

mod-playerbots is GPLv2 and derived from the CMaNGOS playerbots (ike3). Code ported or adapted
from CMaNGOS, MaNGOS, or another GPLv2 project:

- keeps its upstream copyright notice; never replace it with the project header;
- gets a `Co-authored-by: Name <email>` trailer per original author on the commit (this module
  uses the trailer, not `--author` as the core does);
- carries an in-file "Ported/adapted from <project>" note on substantially ported files, or an
  inline `// Adapted from <project>` comment for small snippets;
- is declared in the PR's Code Provenance section with the source project, URL, and authors.

## PR body

Fill in `PULL_REQUEST_TEMPLATE.md` completely; do not delete sections. Strip only the
instructional `<!-- -->` comments.

- **Description**: the concrete, observable failure before and after. "Adds support for X" or
  "more realistic" is not a problem statement.
- **How to Test**: steps, required setup (bot count, config), expected behaviour.
- **Impact**: three questions (performance cost, default behaviour, new maintenance surface),
  each explained when not "None" or "No". The performance answer says what runs per bot, how
  often, and why it can't be done more cheaply; only trivial changes (typos, wrong IDs) skip it.
  The standard measurement is `playerbot pmon` before and after: set `BotActiveAlone = 100` and
  `botActiveAloneSmartScale = 0`, boot, run `playerbot pmon toggle` once the bots have logged
  in, then `playerbot pmon stack` five minutes later and compare the last "Total" line.
- **AI Use and Self-Review**: tick the AI question honestly and say what the AI wrote. Tick exactly
  one review box: either a self-review, naming the tool and model and pasting its output or a
  summary in the details block, or a request for a review, which adds the `Review requested`
  label. A maintainer-run review is posted unverified; reply on the PR where a finding is wrong.
- **Code Provenance**: per the upstream credit section above.
- **Checklist**: new bot lines translated, GPLv2 header on new files, docs and
  `playerbots.conf.dist` comments updated, no new compiler warnings.

## Scope

One PR, one feature. Global changes to shared code are split into their own PR and reviewed as if
any gated feature they support were off, because for most servers it will be. A change that needs
core edits gets a companion PR to `mod-playerbots/azerothcore-wotlk`, and the module PR links it.
