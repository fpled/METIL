#ifndef DISPLAY_WINDOW_H
#define DISPLAY_WINDOW_H

#include "metil_qt_config.h"
#ifdef QT4_FOUND

#include <QtGui/QMainWindow>

/** */
class DisplayWindow : public QMainWindow {
    Q_OBJECT
public:
    DisplayWindow( struct DisplayPainter *display_painter, class QSettings *settings );
    void load_settings( class QSettings *settings );
    void save_settings( class QSettings *settings );
    
    class DisplayWidget *disp_widget;
protected:
    void keyPressEvent( QKeyEvent *event );
    QString img_prefix, img_suffix;
};

#endif // QT4_FOUND

#endif // DISPLAY_WINDOW_H
