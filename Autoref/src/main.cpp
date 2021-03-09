#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include "interaction.h"
#include "parammanager.h"
#include "pluginmanager.h"
#include "field.h"
void qmlRegister(){
    qmlRegisterType<Interaction>("Li", 1, 0, "Interaction");
    qmlRegisterType<Field>("Li", 1, 0, "Field");
}
int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);
    app.setOrganizationName("ZJUNlict");
    app.setOrganizationDomain("www.zjunlict.cn");
    QString theme;
    ZSS::LParamManager::instance()->loadParam(theme,"theme/name","Universal");
    qDebug() << "use theme : " << theme;
    QQuickStyle::setStyle(theme);

    qmlRegister();
    AutorefPM::instance()->init();

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/qml/li.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    auto res = app.exec();

    AutorefPM::instance()->exit();

    return res;
}
