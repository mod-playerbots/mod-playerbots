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



## Feature Evaluation
<!--
If your PR is very minimal (comment typo, wrong ID reference, etc), and it is very obvious it will not have
any impact on performance, you may skip these question. If necessary, a maintainer may ask you for them later.
-->

<!-- Please answer the following: -->
- Describe the **minimum logic** required to achieve the intended behavior.
- Describe the **processing cost** when this logic executes across many bots.



## How to Test the Changes
<!--
- Step-by-step instructions to test the change.
- Any required setup (e.g. multiple players, number of bots, specific configuration).
- Expected behavior and how to verify it.
-->



## Impact Assessment
<!-- As a generic test, before and after measure of pmon (playerbot pmon tick) can help you here. -->
- Does this change increase per-bot/per-tick processing or risk scaling poorly with thousands of bots?
    - - [ ] No, not at all
    - - [ ] Minimal impact (**explain below**)
    - - [ ] Moderate impact (**explain below**)



- Does this change modify default bot behavior?
    - - [ ] No
    - - [ ] Yes (**explain why**)



- Does this change add new decision branches or increase maintenance complexity?
    - - [ ] No
    - - [ ] Yes (**explain below**)



## AI Assistance
<!--
AI assistance is allowed, but all submitted code must be fully understood, reviewed, and owned by the contributor.
We expect contributors to be honest about what they do and do not understand.
-->
Was AI assistance used while working on this change?
- - [ ] No
- - [ ] Yes (**explain below**)
<!--
If yes, please specify:
- Purpose of usage (e.g. brainstorming, refactoring, documentation, code generation).
- Which parts of the change were influenced or generated, and whether it was thoroughly reviewed.
-->



<!--
TRANSLATIONS:
Anything new that the bots say in chat must be in a translatable format. This is done using GetBotTextOrDefault,
which you can search for in the codebase to find examples. Your code needs to have English as the default fallback,
while the full translations need to be in an SQL update file. The languages in the file are the nine language
options supported by AzerothCore: English, Korean, French, German, Chinese, Taiwanese, Spanish, Spanish Mexico, and
Russian. See data/sql/playerbots/updates/2025_12_27_ai_playerbot_fishing_text.sql as an example of a translation SQL
update, whose content are called within the codebase at src/strategy/actions/FishingAction.cpp
-->

## Final Checklist

- - [ ] Stability is not compromised.
- - [ ] Performance impact is understood, tested, and acceptable.
- - [ ] Added logic complexity is justified and explained.
- - [ ] Any new bot dialogue lines are translated.
- - [ ] Documentation updated if needed (Conf comments, WiKi commands).

## Notes for Reviewers
<!-- Anything else that's helpful to review or test your pull request. -->


