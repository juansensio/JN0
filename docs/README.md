# Janus Noise development docs

Janus Noise is a shared platform for small competitive games. The first MVP validates the technical stack with a deliberately minimal card game.

Read these references before implementing a feature:

- [Vision and scope](vision.md): product direction, MVP boundaries, and later ambitions.
- [Architecture](architecture.md): responsibilities, dependencies, core interfaces, and repository layout.
- [MVP game rules](game-rules.md): specified behavior, test cases, and decisions needed before M0 can close.
- [MVP roadmap](roadmap.md): dependencies, work, exit gates, metrics, and progress tracking.

## Source authority

- [Janus Noise — MVP Roadmap](../Janus%20Noise%20%E2%80%94%20MVP%20Roadmap.docx)
- [Janus Noise — Visión y arquitectura técnica](../Janus%20Noise%20%E2%80%94%20Visio%CC%81n%20y%20arquitectura%20te%CC%81cnica.docx)

These files summarize the two original Word documents in the repository root. The MVP Roadmap explicitly identifies itself as the operational source of truth for MVP development. Use it for milestone order and MVP scope; use the vision document for long-term direction. Preserve the originals.

The roadmap uses `client/`, while the vision proposes `client-godot/`. These summaries use `client/` to follow the MVP roadmap. The vision discusses casual P2P and publishing; the MVP defers P2P and public distribution fees. Its dependency gates also permit AI work after M2 and platform builds after M3, despite the more linear sequence in the vision.

The repository currently contains source documents and these development references, with no application implementation or build/test commands. The source roadmap starts at M0; creating summaries alone does not close it. Update the roadmap with evidence as implementation progresses. Record new decisions in the relevant reference and reconcile changes with the source roadmap or architecture document rather than silently overriding them.
