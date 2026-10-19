# Contributing

This is an independent fork of KDE Plasma's task manager. Please report issues
to this project's issue tracker, not KDE's tracker, unless the same behavior
also reproduces in the stock widget.

Include your distribution, architecture, Plasma and Qt versions, widget
version, and whether the session uses Wayland or X11. Give reproducible steps
and describe your grouping, sorting, and spacer settings. Avoid posting personal
launcher URLs or complete panel configuration files when a small example works.

Build in a fresh directory and run the suite before submitting changes:

```sh
cmake -S . -B build-dev -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_TESTING=ON
cmake --build build-dev -j 4
ctest --test-dir build-dev --output-on-failure
```

The tests use a private Xvfb display and temporary configuration. They cover
the widget chooser, actions, saved spacer positions, geometry, pointer drags,
and failure cleanup. This does not replace testing in a real Wayland panel.

Keep the plugin ID `org.matteo.icontaskspacers` stable so existing settings keep
working. Preserve upstream copyright and SPDX notices, and license new code
under GPL-2.0-or-later. See UPSTREAM.md for the base version and source layout.

When porting to a new Plasma release, compare the corresponding upstream task
manager and private QML dependencies. Rebuilding fixes binary mismatches but
does not automatically adapt changed APIs.
