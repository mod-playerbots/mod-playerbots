<!--
Thank you for contributing to mod-playerbots, please make sure that you...
1. Submit your PR to the test-staging branch, not master.
2. Read the guidelines below before submitting.
3. Don't delete parts of this template.

DESIGN PHILOSOPHY: We prioritize STABILITY, PERFORMANCE, AND PREDICTABILITY over behavioral realism.

Every action and decision executes PER BOT AND PER TRIGGER. Small increases in logic complexity scale
poorly across thousands of bots and negatively affect all. We prioritize a stable system over a smarter
one. Bots don't need to behave perfectly; believable behavior is the goal, not human simulation.

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
Every line of code incurs processing costs that may cause unexpected results at scale. Small fixes can tick "None" and skip the explanation.
It's important to know whether the feature incurs minimal costs vs very large ones.
Use pmon to determine the performance hit by taking measurements before and after implementing the changes.
For comparable numbers, set BotActiveAlone = 100 and botActiveAloneSmartScale = 0, run `playerbot pmon toggle`
once the bots finish logging in, then `playerbot pmon stack` 5 minutes later, and post the last "Total" line
from each run.
In some cases direct code profiling of specific functions may be needed.
If there is a high impact then there needs to be some justification for the change.
-->
- [ ] None
- [ ] Minimal (explain below)
- [ ] Moderate or high (explain below)

**2. Does this change what bots do by default?**
<!-- Default means without the player turning on a new strategy or config option. -->
- [ ] No
- [ ] Yes (explain why the new default is better)

## AI Use and Self-Review
<!--
AI help is allowed, but you must understand and stand behind every line you submit, and be able to
explain it when a maintainer asks. AI-assisted work follows the AzerothCore guidelines linked below.
-->

**Did you use AI while writing this change?**
- [ ] No
- [ ] Yes (say what you used it for and which parts it wrote)

- [ ] I have read and understood the [AC guidelines for AI Agentic Engineering](https://www.azerothcore.org/wiki/agentic-engineering) (required if you ticked Yes)

**Review before submitting**
<!--
A self-review is used as a first pass to catch obvious problems before a maintainer spends time on them.
Run /self-review with a capable model, as described in step 5 of the guidelines above, and paste the
resulting SELF-REVIEW.md verbatim as a comment on this PR.
-->
- [ ] I ran a self-review. Tool and model: ______

## Code Provenance
<!--
mod-playerbots is GPLv2 and derives from the CMaNGOS playerbots (ike3). Code ported or adapted from
another GPLv2 project must keep its upstream attribution (GPLv2 §1 and §2(a)).

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

## Notes for Reviewers
<!-- Anything else that helps someone review or test this. -->
