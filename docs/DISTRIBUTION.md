# Distribution and maintenance

The proposed project location is
https://github.com/TheGreatOilPainter/icons-only-spacers. Local preparation does
not create that repository, a GitHub release, or an OBS repository.

## Source releases

```sh
python3 scripts/package.py
```

This creates `dist/icons-only-spacers-VERSION.tar.gz` and a SHA-256 sidecar.
The archive contains the complete buildable fork source and licenses. It
excludes local build trees, caches, existing binary bundles, and redundant
upstream working copies. `SOURCE_DATE_EPOCH` controls normalized timestamps;
its default is zero, making repeated source archives byte-identical.

After building, an explicit binary bundle can also be created:

```sh
python3 scripts/package.py --binary --build-dir build-release
```

Its filename includes the build-time Plasma version, Qt version, system and
architecture. Its `prebuilt/build-info.json` lists private Qt requirements.
It is intended only for matching library environments, not as a universal
Plasma download. The source is included in the same archive.

## Build an openSUSE RPM locally

Install `rpm-build` and the BuildRequires listed in
`packaging/plasma6-applet-icontaskspacers.spec`. Use:

```sh
rpmspec --query --buildrequires packaging/plasma6-applet-icontaskspacers.spec
python3 scripts/package.py
mkdir -p build-rpm/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}
cp dist/icons-only-spacers-1.0.3.tar.gz build-rpm/SOURCES/
rpmbuild --define "_topdir $PWD/build-rpm" -ba packaging/plasma6-applet-icontaskspacers.spec
```

The recipe builds from source, runs the regression suite, and installs one
uniquely named plugin plus documentation and licenses. RPM's normal dependency
generator records native library and versioned-symbol dependencies, including
Qt private API requirements. Explicit requirements cover QML modules embedded
in the plugin. `--without tests` is available for packaging diagnostics; release
validation should keep tests enabled.

Local RPMs are unsigned. Do not describe them as signed repository packages.
For distribution through OBS, configure the appropriate project and signing
before announcing a repository installation command.

## Open Build Service

Create an OBS package containing:

- `plasma6-applet-icontaskspacers.spec`
- `plasma6-applet-icontaskspacers.changes`
- `icons-only-spacers-1.0.3.tar.gz`

Use a build target whose Plasma and Qt versions match the intended users.
Check Slowroll's actual target/repository availability when configuring OBS;
do not assume a Tumbleweed binary matches a Slowroll snapshot. Enable builds,
review build logs and test results, and verify the resulting RPM dependencies.
OBS can rebuild when its declared build dependencies change, but private API
source changes still require maintenance and testing.

References:

- https://openbuildservice.org/help/manuals/obs-user-guide/cha-obs-package-formats
- https://openbuildservice.org/help/manuals/obs-user-guide/cha-obs-local-building
- https://develop.kde.org/docs/plasma/widget/c-api/

## Publish checklist

1. Review the repository name, fork authorship, licenses and support matrix.
2. Verify screenshots and release notes match the tagged source.
3. Build and test the source archive, and inspect the RPM payload/dependencies.
4. Create the GitHub repository and publish a reviewed commit/tag `v1.0.3`.
5. Attach the source archive and its checksum to the release. Attach binaries
   only with their precise build identity and tested-system description.
6. Set up and validate OBS separately before advertising package repositories.
7. Publish the announcement draft after all its links point to live resources.

The native plugin is not a file installable through KDE's Get New Widgets
mechanism. KDE recommends distribution packaging for compiled widgets.
