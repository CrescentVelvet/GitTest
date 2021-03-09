#include <QQuickStyle>
#include <QGuiApplication>
#include <QProcess>
#include <QtDebug>
#include "interaction.h"
#include "parammanager.h"
namespace{
    auto pm = ZSS::LParamManager::instance();
}
Interaction::Interaction(QObject *parent) : QObject(parent){
}
void Interaction::setTheme(QString theme){
    qDebug() << "change theme : " << theme;
    pm->changeParam("theme/name",QVariant(theme));
}
QString Interaction::getTheme(){
    return QQuickStyle::name();
}
void Interaction::restartApp(){
    qApp->quit();
    QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
}
QStringList Interaction::availableTheme(){
    return QQuickStyle::availableStyles();
}
void Interaction::setFieldWidth(int width){
    pm->changeParam("auto/fieldWidth",width);
}
void Interaction::setFieldHeight(int height){
    pm->changeParam("auto/fieldHeight",height);
}
int Interaction::getFieldWidth(){
    int width;
    pm->loadParam(width,"auto/fieldWidth",500);
    return width;
}
int Interaction::getFieldHeight(){
    int height;
    pm->loadParam(height,"auto/fieldHeight",350);
    return height;
}
