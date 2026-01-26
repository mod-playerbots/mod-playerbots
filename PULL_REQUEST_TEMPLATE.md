# Pull Request

Describe what this change does and why it is needed...

---

## Design Philosophy

We prioritize **stability, performance, and predictability** over behavioral realism.  
Complex player-mimicking logic is intentionally limited due to its negative impact on scalability, maintainability, and
long-term robustness.

Excessive processing overhead can lead to server hiccups, increased CPU usage, and degraded performance for all
participants. Because every action and
decision tree is executed **per bot and per trigger**, even small increases in logic complexity can scale poorly and
negatively affect both players and
world (random) bots. Bots are not expected to behave perfectly, and perfect simulation of human decision-making is not a
project goal. Increased behavioral
realism often introduces disproportionate cost, reduced predictability, and significantly higher maintenance overhead.

Every additional branch of logic increases long-term responsibility. All decision paths must be tested, validated, and
maintained continuously as the system evolves.
If advanced or AI-intensive behavior is introduced, the **default configuration must remain the lightweight decision
model**. More complex behavior should only be
available as an **explicit opt-in option**, clearly documented as having a measurable performance cost.

Principles:

- **Stability before intelligence**  
  A stable system is always preferred over a smarter one.

- **Performance is a shared resource**  
  Any increase in bot cost affects all players and all bots.

- **Simple logic scales better than smart logic**  
  Predictable behavior under load is more valuable than perfect decisions.

- **Complexity must justify itself**  
  If a feature cannot clearly explain its cost, it should not exist.

- **Defaults must be cheap**  
  Expensive behavior must always be optional and clearly communicated.

- **Bots should look reasonable, not perfect**  
  The goal is believable behavior, not human simulation.

Before submitting, confirm that this change aligns with those principles.

---

## Feature Evaluation

Please answer the following:

- Describe the **minimum logic** required to achieve the intended behavior?
- Describe the **cheapest implementation** that produces an acceptable result?
- Describe the **runtime cost** when this logic executes across many bots?

---

## How to Test the Changes

- Step-by-step instructions to test the change
- Any required setup (e.g. multiple players, bots, specific configuration)
- Expected behavior and how to verify it

## Complexity & Impact

- Does this change add new decision branches?
    - [ ] No
    - [ ] Yes (**explain below**)

- Does this change increase per-bot or per-tick processing?
    - [ ] No
    - [ ] Yes (**describe and justify impact**)

- Could this logic scale poorly under load?
    - [ ] No
    - [ ] Yes (**explain why**)

---

## Defaults & Configuration

- Does this change modify default bot behavior?
    - [ ] No
    - [ ] Yes (**explain why**)

If this introduces more advanced or AI-heavy logic:

- [ ] Lightweight mode remains the default
- [ ] More complex behavior is optional and thereby configurable

---

## AI Assistance

- Was AI assistance (e.g. ChatGPT or similar tools) used while working on this change?
    - [ ] No
    - [ ] Yes (**explain below**)

If yes, please specify:

- AI tool or model used (e.g. ChatGPT, GPT-4, Claude, etc.)
- Purpose of usage (e.g. brainstorming, refactoring, documentation, code generation)
- Which parts of the change were influenced or generated
- Whether the result was manually reviewed and adapted

AI assistance is allowed, but all submitted code must be fully understood, reviewed, and owned by the contributor.
Any AI-influenced changes must be verified against existing CORE and PB logic. We expect contributors to be honest
about what they do and do not understand.

---

## Final Checklist

- [ ] Stability is not compromised
- [ ] Performance impact is understood, tested, and acceptable
- [ ] Added logic complexity is justified and explained
- [ ] Documentation updated if needed

---

## Notes for Reviewers

Anything that significantly improves realism at the cost of stability or performance should be carefully discussed
before merging.
