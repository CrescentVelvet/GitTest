#include "zslayer.h"
#include <QtDebug>
namespace ZSS{
Layer::Layer(const std::string& name):ZSPlugin(name){
    _image_mutex.lock();
    _image = new QImage(QSize(200, 300),QImage::Format_ARGB32_Premultiplied);
    _painter.begin(_image);
    _painter.setPen(Qt::NoPen);
    _painter.setRenderHint(QPainter::Antialiasing, true);
    _painter.setRenderHint(QPainter::TextAntialiasing, true);
    _image_mutex.unlock();
    declare_receive("draw_signal");
}
Layer::~Layer(){
    _painter.end();
    delete _image;
}
void Layer::resize(int w,int h){
    if(w == 0 || h == 0) return;
    _image_mutex.lock();
    qDebug() << QString::fromStdString(this->name()) << " resize function : " << w << h;
    if(_painter.isActive())
        _painter.end();
    delete _image;
    _image = new QImage(QSize(w,h),QImage::Format_ARGB32_Premultiplied);
    _image->fill(Qt::transparent);
    _area = QRect(0,0,w,h);
    _painter.begin(_image);
    _painter.setRenderHint(QPainter::Antialiasing, true);
    _painter.setRenderHint(QPainter::TextAntialiasing, true);
    init();
    _image_mutex.unlock();
}
// template for run function
//void Layer::run(){
//    init();
//    while(true){
//        if(_image_mutex.try_lock()){
//            _image->fill(Qt::transparent);
//            repaint();
//            _image_mutex.unlock();
//        }
//        std::this_thread::sleep_for(std::chrono::microseconds(100));
//    }
//}
} // namespace ZSS
