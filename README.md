# Icons-Only Task Manager with Spacers

Group pinned applications with real spaces inside KDE Plasma's task manager.
Right-click an app to add or remove its spacer, and choose one width for all
spacers in the widget's Appearance settings.

![The task manager with a spacer between pinned applications](docs/images/taskmanager.png)

This is an independent fork of KDE's Icons-Only Task Manager, based on Plasma
6.7.5. It keeps KDE's existing window grouping, previews, audio indicators and
launcher behavior. Spacer positions are saved per widget and follow their apps
when icons move. No empty .desktop launchers are needed.

## Compatibility

| Environment | Status |
| --- | --- |
| openSUSE Slowroll, x86_64, Plasma 6.7.5, Qt 6.11.2 | Tested |
| Other distributions or Plasma/Qt versions | Not yet verified |
| Minimum build requirements | C++20, CMake 3.22, Qt 6.10, Plasma 6.7, KF6 development libraries |

This is a **compiled native plugin**, not a package installable through Get New
Widgets. Binary builds depend on the destination system's libraries, including
private Qt APIs. Rebuild after incompatible Plasma/Qt changes; source changes
may also be needed when private KDE interfaces change.

## Build and install

On openSUSE, the RPM specification lists the required development and test
packages. Install the packaging tool and then the declared dependencies:

```sh
sudo zypper install rpm-build
sudo bash scripts/install-build-deps.sh
```

Build in a fresh directory, run the checks, and install:

```sh
cmake -S . -B build-release \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DCMAKE_INSTALL_PREFIX=/usr \
  -DBUILD_TESTING=ON
cmake --build build-release -j 4
ctest --test-dir build-release --output-on-failure
sudo cmake --install build-release --strip
```

The installation adds `org.matteo.icontaskspacers.so` to Qt's Plasma plugin
directory. It does not replace the stock widget. If this widget was already
running, log out and back in when convenient to load the new library.

Alternatively, the standalone installer accepts an explicit build directory:

```sh
sudo sh install.sh --build-dir build-release
```

Maintainers can build an RPM using [the packaging guide](docs/DISTRIBUTION.md).
An OBS repository is not provided by the source tree; follow only repository
instructions actually published by the maintainer.

## Use

1. Open **Show Alternatives…** on your task manager and choose **Icons-Only
   Task Manager with Spacers**, or add it from the panel's widget chooser.
2. Right-click an app and choose **Add spacer to the right**.
3. Open **Configure → Appearance → Spacer width (pixels)** to change all gaps.
4. Right-click the same app and choose **Remove spacer** to close its gap.

The default is 24 logical pixels, with a range of 1–256. Vertical panels use
**Add spacer below**, and the setting controls height. With window grouping
disabled, one spacer appears after the last task for that application. An
unpinned app's spacer disappears when the app closes and returns with the app.

KDE's Show Alternatives mechanism copies the current widget configuration,
including pinned launcher order. If you used transparent launchers, add a real
spacer to the preceding app and then unpin the old transparent launcher.

Multi-row layouts keep KDE's column alignment: a spacer in one row can widen
that column in other rows. New spacer action labels are currently English;
inherited task-manager labels use the installed KDE translations.

## Updating and removing

After moving the source directory, or updating development libraries, use a new
build directory. A CMake cache retains absolute paths. Keep the plugin ID stable
when rebuilding so the existing launcher and spacer configuration is retained.

Switch away from this widget before removing a manual installation:

```sh
sudo rm -- "$(qtpaths6 --query QT_INSTALL_PLUGINS)/plasma/applets/org.matteo.icontaskspacers.so"
```

If you installed an RPM, remove it through your package manager instead.

## Development and releases

Tests use a private Xvfb display and temporary settings. They cover KDE's widget
chooser, actual menu actions, geometry, saved anchors, reordering, real mouse
drags with 24 pinned apps, valid MIME payloads, and test failure cleanup. They do
not replace testing in a real Wayland session.

```sh
python3 scripts/package.py
```

The default release is a reproducible source archive with a SHA-256 sidecar.
Optional binary bundles must be requested explicitly and identify the Plasma,
Qt, system and architecture used for compilation. See
[distribution and release instructions](docs/DISTRIBUTION.md), the
[changelog](CHANGELOG.md), and [contribution guide](CONTRIBUTING.md).

## Attribution and license

The upstream task manager was written by KDE contributors. Existing copyright
and SPDX notices are preserved. The fork is maintained by TheGreatOilPainter.
See [UPSTREAM.md](UPSTREAM.md), [LICENSE](LICENSE) and [LICENSES](LICENSES/).

Report fork-specific bugs at
https://github.com/TheGreatOilPainter/icons-only-spacers/issues, including your
distribution, Plasma/Qt versions, widget version, and reproduction steps.
