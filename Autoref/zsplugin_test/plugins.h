#ifndef PLUGINS_H
#define PLUGINS_H

#include "zsplugin.hpp"
#include "singleton.hpp"
class A:public ZSPlugin{
public:
    A():ZSPlugin("A"){
        declare_publish("msg");
    }
    void run() override{
        int count = 0;
        while(response_to_control() != CONTROL_NEED_EXIT){
            auto&& str = std::to_string(count++);
            std::cout << "A before publish" << std::endl;
            publish("msg",str.c_str(),str.size());
            std::cout << "A after publish" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(800));
        }
    }
};

class B:public ZSPlugin{
public:
    B():ZSPlugin("B"){
        declare_receive("msg");
    }
    void run() override{
        std::string str;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        ZSData data;
        while(response_to_control() != CONTROL_NEED_EXIT){
            std::cout << "B before receive" << std::endl;
            receive("msg",data);
            std::cout << "B after receive" << std::endl;
            std::cout << std::string((const char *)data.data(),data.size()) << std::endl;
        }
    }
};

class Test{
public:
    Test(){
        a.link(&b,"msg");
        a.start_detach();
        b.start_detach();
    }
    A a;
    B b;
};
typedef Singleton<Test> T;
#endif // PLUGINS_H
