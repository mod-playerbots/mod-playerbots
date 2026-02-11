<!--
Thank you for contributing to mod-playerbots, please make sure that you...
1. Submit your PR to the test-staging branch, not master.  
2. Read the guidelines below before submitting.
3. Don't delete parts of this template.
-->

## Pull Request Description
<!-- Describe what this change does and why it is needed -->


<!--
DESIGN PHILOSOPHY:

We prioritize STABILITY, PERFORMANCE, AND PREDICTABILITY over behavioral realism.  
Complex player-mimicking logic is intentionally limited due to its negative impact on scalability, maintainability, and
long-term robustness.

Excessive processing overhead can lead to server hiccups, increased CPU usage, and degraded performance for all
participants. Because every action and
decision tree is executed PER BOT AND PER TRIGGER, even small increases in logic complexity can scale poorly and
negatively affect both players and
world (random) bots. Bots are not expected to behave perfectly, and perfect simulation of human decision-making is not a
project goal. Increased behavioral
realism often introduces disproportionate cost, reduced predictability, and significantly higher maintenance overhead.

Every additional branch of logic increases long-term responsibility. All decision paths must be tested, validated, and
maintained continuously as the system evolves.
If advanced or AI-intensive behavior is introduced, the DEFAULT CONFIGURATION MUST REMAIN THE LIGHTWEIGHT DECISION MODEL. 
More complex behavior should only be available as an EXPLICIT OPT-IN OPTION, clearly documented as having a measurable
performance cost.

Principles:

- STABILITY BEFORE INTELLIGENCE  
  A stable system is always preferred over a smarter one.

- PERFORMANCE IS A SHARED RESOURCE  
  Any increase in bot cost affects all players and all bots.

- SIMPLE LOGIC SCALES BETTER THAN SMART LOGIC  
  Predictable behavior under load is more valuable than perfect decisions.

- COMPLEXITY MUST JUSTIFY ITSELF  
  If a feature cannot clearly explain its cost, it should not exist.

- DEFAULTS MUST BE CHEAP  
  Expensive behavior must always be optional and clearly communicated.

- BOTS SHOULD LOOK REASONABLE, NOT PERFECT  
  The goal is believable behavior, not human simulation.

Before submitting, confirm that this change aligns with those principles.
-->


## Feature Evaluation
<!--
If your PR is very minimal (comment typo, wrong ID reference, etc), and it is very obvious it will not have any impact on
performance, you may skip these question. If necessary, a maintainer may ask you for them later.
-->

<!-- Please answer the following: -->

- Describe the **minimum logic** required to achieve the intended behavior?
- Describe the **runtime cost** when this logic executes across many bots?


## How to Test the Changes
<!--
- Step-by-step instructions to test the change
- Any required setup (e.g. multiple players, bots, specific configuration)
- Expected behavior and how to verify it
-->


## Impact Assessment

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


## AI Assistance
<!--
AI assistance is allowed, but all submitted code must be fully understood, reviewed, and owned by the contributor.
Any AI-influenced changes must be verified against existing CORE and PB logic. We expect contributors to be honest
about what they do and do not understand.
-->
Was AI assistance (e.g. ChatGPT or similar tools) used while working on this change?
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
- [ ] Documentation updated if needed

## Notes for Reviewers
<!-- Anything else that's helpful to review or test your pull request -->

