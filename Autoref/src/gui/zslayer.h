#ifndef __LAYER_H__
#define __LAYER_H__
#include <QImage>
#include <QRect>
#include <QPainter>
#include <mutex>
#include <atomic>
#include <memory>
#include "zsplugin.hpp"
#include <QtDebug>
namespace ZSS{
struct ControlVariable{
    ControlVariable(const std::string& name):sw(true),name(name){}
    std::atomic<bool> sw;
    std::string name;
};
class Layer : public ZSPlugin{
public:
    Layer(const std::string& name);
    ~Layer();
    void resize(int,int);
    virtual void init() = 0;
    virtual void run() = 0;
    QImage* get() const{
//        qDebug() << QString::fromStdString(this->name()) << _area;
        return _image; }
    void lock(){_image_mutex.lock();}
    void unlock(){_image_mutex.unlock();}
protected:
    QImage* _image;
    QPainter _painter;
    QRect _area;
    std::mutex _image_mutex;
    std::vector<std::unique_ptr<ControlVariable>> _controls;
};
}
#endif // __LAYER_H__
