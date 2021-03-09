#ifndef __PLUGIN_MANAGER_H__
#define __PLUGIN_MANAGER_H__
#include "singleton.hpp"
#include "zsplugin.hpp"
class PluginManager{
public:
    PluginManager();
    ~PluginManager();
    void init();
    void exit();
private:
    ZSPlugin* vision_udp_receiver;
};
typedef Singleton <PluginManager> AutorefPM;
#endif // __PLUGIN_MANAGER_H__
