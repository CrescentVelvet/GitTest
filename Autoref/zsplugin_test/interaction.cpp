#include "interaction.h"
#include "plugins.h"
#include <QtDebug>
Interaction::Interaction(QObject *parent) : QObject(parent){
}
int Interaction::getControlCode(int id){
    int res = -1;
    switch(id){
    case 0:
        res = T::instance()->a.get_status();
        break;
    case 1:
        res = T::instance()->b.get_status();
        break;
    default:
        res = -1;
        break;
    }
    return res;
}
void Interaction::setControlCode(int id,int cmd){
    qDebug() << "setControlCode : " << id << cmd;
    int command[] = {-1,ZSPlugin::CONTROL_NEED_RUN,ZSPlugin::CONTROL_NEED_PAUSE,ZSPlugin::CONTROL_NEED_EXIT};
    bool need_exit = command[cmd] == ZSPlugin::CONTROL_NEED_EXIT;
    switch (id) {
    case 0:
        need_exit ? T::instance()->a.set_exit() : T::instance()->a.set_pause(command[cmd] == ZSPlugin::CONTROL_NEED_PAUSE);
        break;
    case 1:
        need_exit ? T::instance()->b.set_exit() : T::instance()->b.set_pause(command[cmd] == ZSPlugin::CONTROL_NEED_PAUSE);
        break;
    default:
        break;
    }
}
