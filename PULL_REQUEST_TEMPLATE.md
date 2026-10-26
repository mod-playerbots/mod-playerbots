<!--
Thank you for contributing to mod-playerbots, please make sure that you...
1. Submit your PR to the test-staging branch, not master.
2. Read the guidelines below before submitting.
3. Don't delete parts of this template.

DESIGN PHILOSOPHY: We prioritize STABILITY, PERFORMANCE, AND PREDICTABILITY over behavioral realism.

Every action and decision executes PER BOT AND PER TRIGGER. Small increases in logic complexity scale
poorly across thousands of bots and negatively affect all. We prioritize a stable system over a smarter
one. Bots don't need to behave perfectly; believable behavior is the goal, not human simulation.
Default behavior must be cheap in processing; expensive behavior must be opt-in.

Before submitting, make sure your changes aligns with these principles.
-->

## Pull Request Description
<!-- Describe what this change does and why it is needed -->



## How to Test
<!--
- Steps to see the old behavior and the new one.
- Setup needed (number of bots, party or raid, config values).
- What you should see when it works.
-->



## Impact Assessment

**1. Performance cost at scale**
<!--
Tiny fixes (typo, wrong ID) can tick "None" and skip the explanation.
Otherwise, say what runs per bot and how often, and why it can't be done more cheaply.

pmon before/after numbers help. Set BotActiveAlone = 100 and botActiveAloneSmartScale = 0,
run `playerbot pmon toggle` once the bots finish logging in, then `playerbot pmon stack` 5 minutes later.
Post the last "Total" line from each run.
-->
- [ ] None
- [ ] Minimal (explain below)
- [ ] Moderate or more (explain below)



**2. Does this change what bots do by default?**
<!-- Default means without the player turning on a new strategy or config option. -->
- [ ] No
- [ ] Yes (explain why the new default is better)



**3. Does this add new pieces that need maintaining?**
<!--
For example a new strategy, trigger/action pair, config option, state flag or multiplier.
Every one of these is something future changes have to keep working, so tell us why it's worth it.
-->
- [ ] No
- [ ] Yes (list them and explain below)



## AI Use and Self-Review
<!-- AI help is allowed, but you must understand and stand behind every line you submit. -->

**Did you use AI while writing this change?**
- [ ] No
- [ ] Yes (say what you used it for and which parts it wrote)

**Review before submitting** (tick one)
<!--
A self-review catches the obvious problems before a maintainer spends time on them. Use a capable model (Claude, Codex or similar).

Paste the review, or a summary of what it found and what you changed, in the details block below.

If you can't run one, tick the second box to request a review.
-->
- [ ] I ran a self-review. Tool and model: ______
- [ ] I could not run a review and am requesting one

## Code Provenance
<!--
mod-playerbots is GPLv2 and derives from the CMaNGOS playerbots (ike3). Code ported or adapted from
another GPLv2 project must keep its upstream attribution (GPLv2 §1 and §2(a)). Never replace an
upstream copyright notice with the project header.

If yes, give:
- Source project and URL (file or commit if you know it).
- Original author(s), each with a `Co-authored-by: Name <email>` trailer on your commit.
- An in-file "Ported/adapted from <project>" note on substantially ported files, or an inline
  "// Adapted from <project>" comment on small snippets.
-->
Was any code copied or adapted from another project (CMaNGOS playerbots, MaNGOS, another module)?
- [ ] No, all code is original
- [ ] Yes (name the project and original authors below)



## Checklist
<!--
Translations: anything new that bots say in chat must go through GetBotTextOrDefault with English
as the fallback, and the translations for all nine AzerothCore locales go in an SQL update. Example:
data/sql/playerbots/updates/2025_12_27_ai_playerbot_fishing_text.sql, used in
src/Ai/Base/Actions/FishingAction.cpp.
-->
- [ ] New bot chat lines are translated (see comment above), or there are none.
- [ ] New source files use the GPLv2 header.
- [ ] Docs updated where needed (code comments, conf comments, wiki commands).
- [ ] No new compiler warnings.

## Notes for Reviewers
<!-- Anything else that helps someone review or test this. -->
