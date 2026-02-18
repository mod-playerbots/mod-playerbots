<!--
Thank you for contributing to mod-playerbots, please make sure that you...
1. Submit your PR to the test-staging branch, not master.  
2. Read the guidelines below before submitting.
3. Don't delete parts of this template.

DESIGN PHILOSOPHY: We prioritize STABILITY, PERFORMANCE, AND PREDICTABILITY over behavioral realism.

Every action and decision executes PER BOT AND PER TRIGGER. Small increases in logic complexity
scale poorly across thousands of bots and negatively affect all participants. We prioritize 
stability over intelligence: a stable system is always preferred over a smarter one. Complexity 
must justify its cost. Bots don't need to behave perfectly; believable behavior is the goal, 
not human simulation. Default behavior must be cheap; expensive behavior must be opt-in.

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
- Describe the **runtime cost** when this logic executes across many bots.


## How to Test the Changes
<!--
- Step-by-step instructions to test the change
- Any required setup (e.g. multiple players, bots, specific configuration)
- Expected behavior and how to verify it
-->


## Impact Assessment
<!-- Before and after measure of pmon (playerbot pmon tick) can help you here -->
- Does this change increase per-bot/per-tick processing or risk scaling poorly with thousands of bots?
    - [ ] No, not at all
    - [ ] Minimal impact (**explain below**)
    - [ ] Moderate impact (**explain below**)


- Does this change modify default bot behavior?
    - [ ] No
    - [ ] Yes (**explain why**)


- Does this change add new decision branches or increase maintenance complexity?
    - [ ] No
    - [ ] Yes (**explain below**)


## Messages to Translate
Does this change add bot messages to translate?
- [ ] No
- [ ] Yes (**list messages in the table**)
<!--
Bot messages have to be translatable, but you don't need to do the translations here. You only need to make sure
the message is in a translatable format, and list in the table the message_key and the default English message.
Serach for GetBotTextOrDefault in the codebase for examples.
-->
| Message key | Default message |
| ----------- | --------------- |
|             |                 |
|             |                 |


## AI Assistance
<!--
AI assistance is allowed, but all submitted code must be fully understood, reviewed, and owned by the contributor.
Any AI-influenced changes must be verified against existing CORE and PB logic. We expect contributors to be honest
about what they do and do not understand.
-->
Was AI assistance used while working on this change?
- [ ] No
- [ ] Yes (**explain below**)
<!--
If yes, please specify:

- AI tool or model used (e.g. ChatGPT, GPT-4, Claude, etc.)
- Purpose of usage (e.g. brainstorming, refactoring, documentation, code generation)
- Which parts of the change were influenced or generated
- Whether the result was manually reviewed and adapted
-->


## Final Checklist

- [ ] Stability is not compromised
- [ ] Performance impact is understood, tested, and acceptable
- [ ] Added logic complexity is justified and explained
- [ ] Documentation updated if needed (Conf comments, WiKi commands)

## Notes for Reviewers
<!-- Anything else that's helpful to review or test your pull request -->

