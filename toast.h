#ifndef TOAST_H
#define TOAST_H

#include <QObject>
#include <QRect>
class ToastDlg;

class Toast: QObject
{
public:
    enum Level
    {
      INFO, WARN
    };
public:
    Toast();
    static Toast& instance();
    void show(Level level, const QString& text);

private:
    void timerEvent(QTimerEvent *event) override;
    ToastDlg* mDlg;
    int mTimerId{0};
    QRect mGeometry;
};

#endif // TOAST_H
