# Upstream source and provenance

This independent fork is based on KDE's plasma-desktop **v6.7.5** task manager:
https://invent.kde.org/plasma/plasma-desktop/-/tree/v6.7.5/applets/taskmanager

The complete buildable fork is in `applet/`. The two configuration-generation
inputs in `config/` come from upstream's
`kcms/recentFiles/kactivitymanagerd_plugins_settings.kcfg` and `.kcfgc`.
No checkout of upstream is required to build this source release. Local working
trees may retain an ignored `upstream/` comparison snapshot; it is not needed
in the public repository or release archives.

Existing source-level copyrights and SPDX notices are retained. The applet and
new fork code are GPL-2.0-or-later. Inherited LGPL-2.0-or-later files retain
their individual licenses. The full texts are included in `LICENSES/`.

The fork uses the separate ID `org.matteo.icontaskspacers`. Keep this ID stable
for users upgrading from the initial local builds. KDE's installed translation
domain is reused for inherited strings; the new spacer strings use KDE's
translation functions but do not yet have a separate translation catalog.

The changes add persistent app-identity anchors, real layout margins, context
menu actions, a global spacer width, in-memory identity matching during drags,
and valid MIME payloads for launcher-only tasks. The underlying KDE task model
and C++ backend remain based on the upstream implementation.

Plasma 6.7 embeds this applet's QML in its native plugin. Reusing the stock
plugin with `X-Plasma-RootPath` also selects its stock resource interface and
configuration schema, so the fork has its own compiled plugin.

The API metadata remains at upstream's `6.0`. Plasma 6.7.5's widget chooser
rejects API minor versions above 6 even though the runtime is 6.7. CMake
separately enforces the source's minimum Plasma and Qt build requirements.
Reference:
https://invent.kde.org/plasma/plasma-workspace/-/blob/v6.7.5/components/shellprivate/plasmaappletitemmodel.cpp
