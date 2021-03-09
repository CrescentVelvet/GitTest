#ifndef INTERACTION_H
#define INTERACTION_H

#include <QObject>
#include <QString>
class Interaction : public QObject
{
    Q_OBJECT
public:
    explicit Interaction(QObject *parent = nullptr);
    Q_INVOKABLE int getControlCode(int);

    //! set control
    //! 1 : run
    //! 2 : pause
    //! 3 : exit
    Q_INVOKABLE void setControlCode(int,int);

signals:

public slots:
};

#endif // INTERACTION_H
