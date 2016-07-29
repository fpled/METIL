#ifndef DISPLAY_WIDGET_H
#define DISPLAY_WIDGET_H

#include "metil_qt_config.h"
#ifdef QT4_FOUND

#include <QtGui/QWidget>
#include <QtCore/QVector>
  
/** */
class DisplayWidget : public QWidget {
public:
    DisplayWidget( QWidget *parent, struct DisplayPainter *display_painter );
    
    struct DisplayPainter *display_painter;
    
public slots:
    void add_vanishing_text( QString txt );
protected:
    struct TextData {
        QString txt;
        double alpha;
    };
    
    void paintEvent       ( QPaintEvent  *event );
    void wheelEvent       ( QWheelEvent  *event );
    void mousePressEvent  ( QMouseEvent  *event );
    void mouseMoveEvent   ( QMouseEvent  *event );
    void mouseReleaseEvent( QMouseEvent  *event );
    void resizeEvent      ( QResizeEvent *event );
    void timerEvent       ( QTimerEvent  *event );
    
    int x_during_last_mouse_event;
    int y_during_last_mouse_event;
    QVector<TextData> text_data;
    int timer_id;
};

#endif // QT4_FOUND

#endif // DISPLAY_WIDGET_H

