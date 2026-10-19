// SPDX-License-Identifier: GPL-2.0-or-later
#include <QApplication>
#include <QElapsedTimer>
#include <QThread>
#include <QTemporaryDir>
#include <QQmlContext>
#include <QQmlExpression>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QAbstractItemModel>
#include <QJSValue>
#include <QQuickWindow>
#include <QEventLoop>
#include <QTimer>
#include <QLibrary>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <KConfigGroup>
#include <KConfigLoader>
#include <KLocalizedString>
#include <Plasma/Corona>
#include <Plasma/Containment>
#include <Plasma/Applet>
#include <PlasmaQuick/AppletQuickItem>
#include <cstdlib>
#include <memory>
#include <stdexcept>

class TestCorona : public Plasma::Corona {
public:
    QRect screenGeometry(int) const override { return QRect(0, 0, 1920, 1080); }
};

static void settle()
{
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < 350) {
        QCoreApplication::processEvents();
        QThread::msleep(5);
    }
}

static void require(bool condition, const char *message)
{
    if (!condition) {
        qCritical() << "FAIL:" << message;
        throw std::runtime_error(message);
    }
    qInfo() << "PASS:" << message;
}

static int runTests(int argc, char **argv)
{
    // Never read/write the real panel configuration or cache during a test.
    QTemporaryDir state;
    qputenv("XDG_CONFIG_HOME", (state.path() + QStringLiteral("/config")).toUtf8());
    qputenv("XDG_CACHE_HOME", (state.path() + QStringLiteral("/cache")).toUtf8());
    QApplication app(argc, argv);
    KLocalizedString::setApplicationDomain("plasma_applet_org.kde.plasma.taskmanager");
    QCoreApplication::addLibraryPath(QString::fromLocal8Bit(argv[1]));
    // The chooser has its own compatibility gate, separate from plugin loading.
    QQmlEngine chooserEngine;
    QQmlComponent chooserComponent(&chooserEngine);
    chooserComponent.setData("import org.kde.plasma.private.shell\nWidgetExplorer { application: \"plasmashell\" }", QUrl());
    const auto chooser = std::unique_ptr<QObject>(chooserComponent.create());
    if (!chooser) {
        qCritical() << chooserComponent.errors();
    }
    require(bool(chooser), "real widget chooser instantiates");
    settle();
    auto chooserModel = qobject_cast<QAbstractItemModel *>(chooser->property("widgetsModel").value<QObject *>());
    require(chooserModel, "widget chooser model is available");
    const auto roles = chooserModel->roleNames();
    const auto pluginRole = roles.key(QByteArrayLiteral("pluginName"), -1);
    const auto supportedRole = roles.key(QByteArrayLiteral("isSupported"), -1);
    require(pluginRole >= 0 && supportedRole >= 0, "widget chooser exposes compatibility roles");
    bool foundSupported = false;
    for (int row = 0; row < chooserModel->rowCount(); ++row) {
        const auto index = chooserModel->index(row, 0);
        if (chooserModel->data(index, pluginRole).toString() == QStringLiteral("org.matteo.icontaskspacers")) {
            foundSupported = chooserModel->data(index, supportedRole).toBool();
            break;
        }
    }
    require(foundSupported, "fork appears as supported in the real widget chooser");
    TestCorona corona;
    auto containment = corona.createContainment(QStringLiteral("null"));
    containment->setFormFactor(Plasma::Types::Horizontal);
    containment->setLocation(Plasma::Types::BottomEdge);
    auto applet = containment->createApplet(QStringLiteral("org.matteo.icontaskspacers"));
    require(applet && applet->pluginMetaData().isValid(), "fork plugin loads");
    auto config = applet->config().group(QStringLiteral("General"));
    config.writeEntry("launchers", QStringList{QStringLiteral("applications:org.kde.konsole.desktop"), QStringLiteral("applications:org.kde.kate.desktop"), QStringLiteral("applications:systemsettings.desktop")});
    config.writeEntry("showOnlyCurrentActivity", false);
    config.writeEntry("showOnlyCurrentDesktop", false);
    config.writeEntry("indicateAudioStreams", false);
    config.sync();
    applet->configScheme()->load();
    auto item = PlasmaQuick::AppletQuickItem::itemForApplet(applet);
    require(item, "QML root instantiates");
    QQuickWindow window;
    window.resize(600, 48);
    item->setParentItem(window.contentItem());
    item->setSize(QSizeF(600, 48));
    window.show();
    settle();
    auto evaluate = [&](const char *code) {
        QQmlExpression expression(qmlContext(item), item, QString::fromUtf8(code));
        auto result = expression.evaluate();
        if (expression.hasError()) {
            qCritical() << expression.error();
            throw std::runtime_error("QML expression failed");
        }
        if (result.metaType() == QMetaType::fromType<QJSValue>()) {
            return result.value<QJSValue>().toVariant();
        }
        return result;
    };
    require(evaluate("iconsOnly").toBool(), "fork uses icons-only mode");
    require(evaluate("tasksModel.count >= 3").toBool(), "pinned launchers become task delegates");
    require(evaluate("taskList.children[0].spacerExtent === 0").toBool(), "no initial spacer");
    evaluate("setSpacer(taskList.children[0].spacerKey, true)");
    settle();
    require(evaluate("taskList.children[0].spacerExtent === 24").toBool(), "add spacer using configured width");
    require(evaluate("Math.abs(taskList.children[1].x - (taskList.children[0].x + taskList.children[0].width) - 24) < 1").toBool(), "24 pixel gap outside icon hit target");
    evaluate("plasmoid.configuration.spacerWidth = 40");
    settle();
    require(evaluate("Math.abs(taskList.children[1].x - (taskList.children[0].x + taskList.children[0].width) - 40) < 1").toBool(), "width change relayouts existing spacers");
    if (argc > 3 && QByteArray(argv[2]) == QByteArrayLiteral("--screenshot")) {
        window.setColor(QColor(QStringLiteral("#25252e")));
        window.resize(260, 56);
        item->setSize(QSizeF(260, 56));
        settle();
        require(window.grabWindow().save(QString::fromLocal8Bit(argv[3])), "save actual widget screenshot");
        return 0;
    }
    require(evaluate("plasmoid.configuration.spacerAnchors.length === 1").toBool(), "anchor saved to widget configuration");
    evaluate("setSpacer(taskList.children[0].spacerKey, true)");
    require(evaluate("plasmoid.configuration.spacerAnchors.length === 1").toBool(), "adding same spacer twice is idempotent");
    evaluate("tasksModel.move(0, 2); tasksModel.syncLaunchers()");
    settle();
    require(evaluate("taskList.children[2].spacerExtent === 40 && taskList.children[0].spacerExtent === 0").toBool(), "spacer follows app when icons are reordered");
    evaluate("tasksModel.move(2, 0); tasksModel.syncLaunchers()");
    settle();
    applet->configScheme()->save();
    config.sync();
    KConfig onDisk(config.config()->name(), KConfig::SimpleConfig);
    const auto persistedGeneral = onDisk.group(QStringLiteral("Containments"))
        .group(QString::number(containment->id())).group(QStringLiteral("Applets"))
        .group(QString::number(applet->id())).group(QStringLiteral("Configuration"))
        .group(QStringLiteral("General"));
    require(!persistedGeneral.readEntry("spacerAnchors", QStringList()).isEmpty(), "spacer anchors written to config file");
    require(evaluate("TaskManagerApplet.SpacerTools.keyFor('file:///home/test/Camera%20Lucida.desktop', '') === TaskManagerApplet.SpacerTools.keyFor('applications:Camera Lucida.desktop', '')").toBool(), "launcher URL normalization handles encoded filenames");
    require(evaluate("TaskManagerApplet.SpacerTools.keyFor('preferred://browser', '') === 'preferred://browser'").toBool(), "default application anchors remain stable");
    require(evaluate("(() => { const data = TaskManagerApplet.TaskTools.dragMimeData({MimeType: '', MimeData: undefined}, 'file:///tmp/test.desktop'); return !Object.prototype.hasOwnProperty.call(data, '') && Object.values(data).every(v => typeof v === 'string') && data['text/uri-list'] === 'file:///tmp/test.desktop\\r\\n'; })()").toBool(), "launcher drag payload contains only valid MIME entries");
    require(evaluate("TaskManagerApplet.TaskTools.dragMimeData({MimeType: 'application/x-window', MimeData: 'window-payload'}, '')['application/x-window'] === 'window-payload'").toBool(), "window drag payload is preserved");
    auto menu = evaluate("createContextMenu(taskList.children[0], tasksModel.makeModelIndex(0), {})").value<QObject *>();
    require(menu, "task context menu instantiates");
    QObject *spacerAction = nullptr;
    for (auto child : menu->findChildren<QObject *>()) {
        if (child->property("text").toString() == QStringLiteral("Remove spacer")) {
            spacerAction = child;
            break;
        }
    }
    require(spacerAction && spacerAction->property("enabled").toBool(), "context menu offers Remove spacer on its app");
    require(QMetaObject::invokeMethod(spacerAction, "clicked", Qt::DirectConnection), "spacer menu action can be triggered");
    settle();
    require(evaluate("taskList.children[0].spacerExtent === 0").toBool(), "context menu action removes its app's spacer");
    require(spacerAction->property("text").toString() == QStringLiteral("Add spacer to the right"), "context menu offers Add spacer after removal");
    QMetaObject::invokeMethod(spacerAction, "clicked", Qt::DirectConnection);
    settle();
    require(evaluate("taskList.children[0].spacerExtent === 40").toBool(), "context menu action adds its app's spacer");
    delete menu;
    auto appearance = evaluate("Qt.createComponent('ConfigAppearance.qml').createObject(tasks)").value<QObject *>();
    require(appearance && appearance->property("cfg_spacerWidth").isValid(), "Appearance settings expose spacer width");
    delete appearance;
    evaluate("setSpacer(taskList.children[0].spacerKey, false)");
    settle();
    require(evaluate("Math.abs(taskList.children[1].x - (taskList.children[0].x + taskList.children[0].width)) < 1").toBool(), "remove spacer closes the gap");
    evaluate("setSpacer(taskList.children[1].spacerKey, true); plasmoid.configuration.reverseMode = true");
    settle();
    require(evaluate("Math.abs(taskList.children[0].x - (taskList.children[1].x + taskList.children[1].width) - 40) < 1").toBool(), "reversed horizontal layout keeps spacer physically to the right");
    evaluate("setSpacer(taskList.children[1].spacerKey, false); plasmoid.configuration.reverseMode = false");
    settle();
    evaluate("setSpacer(taskList.children[0].spacerKey, true)");
    containment->setFormFactor(Plasma::Types::Vertical);
    containment->setLocation(Plasma::Types::LeftEdge);
    window.resize(48, 600);
    item->setSize(QSizeF(48, 600));
    settle();
    require(evaluate("Math.abs(taskList.children[1].y - (taskList.children[0].y + taskList.children[0].height) - 40) < 1").toBool(), "vertical panel uses spacer height");

    if (argc > 2 && QByteArray(argv[2]) == QByteArrayLiteral("--drag")) {
        // XTEST drives real mouse events only on run_smoke.py's private display.
        QLibrary x11(QStringLiteral("X11")), xtst(QStringLiteral("Xtst"));
        auto openDisplay = reinterpret_cast<void *(*)(const char *)>(x11.resolve("XOpenDisplay"));
        auto closeDisplay = reinterpret_cast<int (*)(void *)>(x11.resolve("XCloseDisplay"));
        auto flush = reinterpret_cast<int (*)(void *)>(x11.resolve("XFlush"));
        auto motion = reinterpret_cast<int (*)(void *, int, int, int, unsigned long)>(xtst.resolve("XTestFakeMotionEvent"));
        auto button = reinterpret_cast<int (*)(void *, unsigned int, int, unsigned long)>(xtst.resolve("XTestFakeButtonEvent"));
        require(openDisplay && closeDisplay && flush && motion && button, "XTEST pointer driver loads");
        void *display = openDisplay(nullptr);
        require(display, "pointer driver connects to private display");
        QStringList launchers;
        for (int i = 0; i < 24; ++i) {
            const auto filename = state.path() + QStringLiteral("/drag-%1.desktop").arg(i);
            QFile file(filename);
            require(file.open(QIODevice::WriteOnly), "create test launcher");
            file.write("[Desktop Entry]\nType=Application\nName=Drag Test\nExec=/bin/true\nIcon=application-x-executable\n");
            file.close();
            launchers << QUrl::fromLocalFile(filename).toString();
        }
        containment->setFormFactor(Plasma::Types::Horizontal);
        containment->setLocation(Plasma::Types::BottomEdge);
        window.resize(1800, 48);
        item->setSize(QSizeF(1800, 48));
        auto launcherJson = QJsonDocument(QJsonArray::fromStringList(launchers)).toJson(QJsonDocument::Compact);
        const QByteArray launcherScript = QByteArrayLiteral("plasmoid.configuration.spacerAnchors = []; plasmoid.configuration.launchers = ") + launcherJson;
        evaluate(launcherScript.constData());
        settle();
        require(evaluate("tasksModel.count === 24").toBool(), "24 pinned apps ready for drag stress test");
        evaluate("for (let i = 0; i < 24; i += 3) setSpacer(taskList.children[i].spacerKey, true)");
        settle();
        auto drag = [&](int from, int to) {
            auto tasks = qobject_cast<QQuickItem *>(item->property("taskList").value<QObject *>());
            const auto source = tasks->childItems().at(from);
            const auto target = tasks->childItems().at(to);
            const auto start = source->mapToGlobal(QPointF(source->width() / 2, source->height() / 2)).toPoint();
            const auto end = target->mapToGlobal(QPointF(target->width() / 2, target->height() / 2)).toPoint();
            const auto key = source->property("spacerKey").toString();
            const auto hadSpacer = source->property("hasSpacer").toBool();
            qInfo() << "BEGIN pointer drag" << from << "to" << to;
            QEventLoop loop;
            QTimer heartbeat;
            heartbeat.setInterval(5);
            QElapsedTimer tick;
            tick.start();
            qint64 worstPause = 0;
            bool sawDrag = false;
            QObject::connect(&heartbeat, &QTimer::timeout, &loop, [&]() {
                worstPause = std::max(worstPause, tick.restart());
                sawDrag |= item->property("dragSource").value<QObject *>() != nullptr;
            });
            heartbeat.start();
            motion(display, -1, start.x(), start.y(), 0);
            flush(display);
            QTimer::singleShot(60, &loop, [&]() { button(display, 1, true, 0); flush(display); });
            for (int step = 1; step <= 30; ++step) {
                QTimer::singleShot(80 + step * 25, &loop, [&, step]() {
                    motion(display, -1, start.x() + (end.x() - start.x()) * step / 30,
                           start.y() + (end.y() - start.y()) * step / 30, 0);
                    flush(display);
                });
            }
            QTimer::singleShot(950, &loop, [&]() { button(display, 1, false, 0); flush(display); });
            QTimer::singleShot(1400, &loop, &QEventLoop::quit);
            loop.exec();
            qInfo() << "Drag" << from << "to" << to << "worst event-loop pause (ms):" << worstPause;
            require(sawDrag, "real pointer gesture starts task dragging");
            require(!item->property("dragSource").value<QObject *>(), "pointer release ends task dragging");
            int newIndex = -1;
            bool keptSpacer = false;
            for (auto current : tasks->childItems()) {
                if (current->property("spacerKey").toString() == key) {
                    newIndex = current->property("index").toInt();
                    keptSpacer = current->property("hasSpacer").toBool() == hadSpacer;
                    break;
                }
            }
            require(newIndex >= 0 && newIndex != from, "pointer drag reorders app and preserves its identity");
            require(keptSpacer, "pointer drag preserves the app's spacer setting");
            require(worstPause < 500, "drag avoids multi-second event-loop stalls");
        };
        drag(0, 8);
        drag(2, 10);
        drag(12, 4);
        evaluate("plasmoid.configuration.spacerAnchors = []");
        settle();
        drag(2, 10);
        closeDisplay(display);
    }
    if (argc > 2 && QByteArray(argv[2]) == QByteArrayLiteral("--fail-cleanup")) {
        require(false, "intentional failure to exercise Qt cleanup");
    }
    return 0;
}

int main(int argc, char **argv)
{
    try {
        return runTests(argc, argv);
    } catch (const std::exception &error) {
        // Unwind QApplication, QML engines and popup owners normally on failures.
        fprintf(stderr, "Test failed: %s\n", error.what());
        return 1;
    }
}
