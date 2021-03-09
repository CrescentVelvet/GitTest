#ifndef INTERACTION_H
#define INTERACTION_H

#include <QObject>
#include <QString>
class Interaction : public QObject
{
    Q_OBJECT
public:
    explicit Interaction(QObject *parent = nullptr);
    // set
    Q_INVOKABLE void setTheme(QString theme);
    Q_INVOKABLE QString getTheme();
    Q_INVOKABLE void restartApp();
    // available
    Q_INVOKABLE QStringList availableTheme();
    // field
    Q_INVOKABLE int getFieldWidth();
    Q_INVOKABLE int getFieldHeight();
    Q_INVOKABLE void setFieldWidth(int);
    Q_INVOKABLE void setFieldHeight(int);
signals:

public slots:
};

#endif // INTERACTION_H
