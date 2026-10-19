# Changelog

## 1.0.3 — 2026-10-19

First public-release preparation. Widget behavior is unchanged from 1.0.2.

- Add an openSUSE RPM recipe, public installation guide, contribution guide,
  issue template, release checklist, and announcement draft.
- Make source-only, reproducible archives the default release artifact.
- Label optional binary bundles with their actual build-time Plasma, Qt,
  operating system, architecture, and private Qt symbol requirements.
- Add explicit build-directory selection to the standalone installer.
- Point fork metadata to its own proposed project and issue tracker, retain
  KDE authorship, and identify the fork maintainer separately.

## 1.0.2 — 2026-10-18

- Eliminate repeated desktop-file resolution in spacer layout bindings during
  dragging; use direct app identities and cached preferred-app matches.
- Supply valid drag MIME data for launcher-only tasks.
- Add real mouse-drag regression tests with 24 pinned apps and mixed spacers.
- Fix test failure cleanup so assertions do not cause Qt teardown crashes.

## 1.0.1 — 2026-10-18

- Restore upstream's Plasma API metadata value, 6.0, so KDE's widget chooser
  accepts the plugin. Add a check using the real widget chooser.

## 1.0.0 — 2026-10-18

- Fork Plasma 6.7.5's Icons-Only Task Manager under a separate plugin ID.
- Add and remove an app's spacer from its context menu.
- Configure a shared spacer width of 1–256 logical pixels.
- Save spacer positions by app identity, including vertical and reversed layouts.
