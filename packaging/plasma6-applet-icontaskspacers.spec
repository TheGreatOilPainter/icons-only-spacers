# SPDX-License-Identifier: GPL-2.0-or-later
%define srcname icons-only-spacers
%bcond_without tests

Name:           plasma6-applet-icontaskspacers
Version:        1.0.3
Release:        0
Summary:        Icons-only Plasma task manager with configurable spacers
License:        GPL-2.0-or-later AND LGPL-2.0-or-later
URL:            https://github.com/TheGreatOilPainter/icons-only-spacers
Source0:        %{url}/releases/download/v%{version}/%{srcname}-%{version}.tar.gz
BuildRequires:  cmake >= 3.22
BuildRequires:  gcc-c++
BuildRequires:  kf6-extra-cmake-modules >= 6.0
BuildRequires:  ninja
BuildRequires:  cmake(KF6Config)
BuildRequires:  cmake(KF6CoreAddons)
BuildRequires:  cmake(KF6I18n)
BuildRequires:  cmake(KF6KIO)
BuildRequires:  cmake(KF6Notifications)
BuildRequires:  cmake(KF6Service)
BuildRequires:  cmake(KF6WindowSystem)
BuildRequires:  cmake(KSysGuard)
BuildRequires:  cmake(LibNotificationManager)
BuildRequires:  cmake(Plasma) >= 6.7
BuildRequires:  cmake(PlasmaActivities)
BuildRequires:  cmake(PlasmaActivitiesStats)
BuildRequires:  cmake(Qt6Core) >= 6.10
BuildRequires:  cmake(Qt6DBus)
BuildRequires:  cmake(Qt6Gui)
BuildRequires:  cmake(Qt6Qml)
BuildRequires:  cmake(Qt6Quick)
BuildRequires:  cmake(Qt6Widgets)
%if %{with tests}
BuildRequires:  python3 >= 3.9
BuildRequires:  cmake(PlasmaQuick)
BuildRequires:  kf6-kcmutils-imports
BuildRequires:  kf6-kdeclarative-imports
BuildRequires:  kf6-kirigami-imports
BuildRequires:  kf6-ksvg-imports
BuildRequires:  kpipewire6-imports
BuildRequires:  libXtst6
BuildRequires:  plasma6-desktop >= 6.7
BuildRequires:  qt6-qt5compat-imports
BuildRequires:  xorg-x11-server-Xvfb
%endif
Requires:       kf6-kcmutils-imports
Requires:       kf6-kdeclarative-imports
Requires:       kf6-kirigami-imports
Requires:       kf6-ksvg-imports
Requires:       kpipewire6-imports
Requires:       plasma6-desktop >= 6.7
Requires:       plasma6-workspace >= 6.7
Requires:       qt6-qt5compat-imports
Recommends:     plasma6-pa

%description
A fork of KDE's Icons-Only Task Manager with an action to insert a spacer
beside an application and a setting for the width of all spacers. Positions
are saved per widget and follow applications when launchers are reordered.

This is a native Plasma plugin. Build it against the destination system's
Plasma and Qt libraries; private APIs can change between updates.

%prep
%autosetup -n %{srcname}-%{version}

%build
%cmake_kf6 -DBUILD_TESTING:BOOL=%{with tests}
%kf6_build

%install
%kf6_install

%check
%if %{with tests}
ctest --test-dir build --output-on-failure
%endif

%files
%license LICENSE LICENSES/*
%doc README.md UPSTREAM.md CHANGELOG.md CONTRIBUTING.md docs
%{_kf6_plugindir}/plasma/applets/org.matteo.icontaskspacers.so

%changelog
