#include "pluginmanager.h"
#include "visionmodule.h"
#include "udpreceiver.h"
PluginManager::PluginManager():vision_udp_receiver(nullptr){
    vision_udp_receiver = new UDPReceiver("ssl_vision","224.5.23.2",10020);
}
PluginManager::~PluginManager(){
    delete vision_udp_receiver;
}
void PluginManager::init(){
    auto* vm = VisionModule::instance();
    vision_udp_receiver->link(vm,"ssl_vision");
    vm->parseCameraMode(0x01);
    vm->start_detach();
    vision_udp_receiver->start_detach();
}
void PluginManager::exit(){
    vision_udp_receiver->set_exit();
    VisionModule::instance()->set_exit();
}
