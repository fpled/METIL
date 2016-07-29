#include "metil_qt_config.h"
#ifdef QT4_FOUND

#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtGui/QWheelEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QResizeEvent>
#include <QtCore/QTimer>
#include <QtCore/QDebug>
#include <cmath>

#include "DisplayWidget.h"
#include "DisplayPainter.h"

DisplayWidget::DisplayWidget( QWidget *parent, DisplayPainter *display_painter ) : QWidget( parent ), display_painter( display_painter ) {
    setAutoFillBackground( true );
    timer_id = 0;
}

void DisplayWidget::add_vanishing_text( QString txt ) {
    TextData td;
    td.txt      = txt;
    td.alpha    = 255;
    text_data.push_back( td );
    //
    if ( not timer_id )
        timer_id = startTimer(50);
    QTimer::singleShot( 0, this, SLOT(update()) );
}

void DisplayWidget::paintEvent( QPaintEvent *event ) {
    QPainter painter( this );
    painter.save();
    display_painter->paint( painter, width(), height() );
    // additional text
    painter.restore();
    QColor fg( display_painter->cur_color_set.fg_color );
    QColor bg( display_painter->cur_color_set.bg_color );
    double ox = 5, oy = 5;
    for(int i=0;i<text_data.size();++i) {
        QRectF text_rcmd( ox, oy, width() - 10, height() ), text_rect;
        if ( text_data[i].alpha >= 40 ) {
            fg.setAlpha( text_data[i].alpha );
            painter.setPen( fg );
            painter.drawText( text_rcmd, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, text_data[i].txt, &text_rect );
        } else
            text_rect = painter.boundingRect( text_rcmd, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, text_data[i].txt );
        oy += text_rect.height();
    }
}

void DisplayWidget::wheelEvent( QWheelEvent *event ) {
    if ( event->modifiers() & Qt::ShiftModifier )
        display_painter->shrink = std::max( 0.1, std::min( display_painter->shrink + event->delta() / 3600.0, 1.0 ) );
    else
        display_painter->zoom( std::pow( 2.0, - event->delta() / 720.0 ), event->x(), event->y(), width(), height() );
    update();
}

void DisplayWidget::mousePressEvent( QMouseEvent *event ) {
    x_during_last_mouse_event = event->x();
    y_during_last_mouse_event = event->y();
}

void DisplayWidget::mouseMoveEvent( QMouseEvent *event ) {
    if ( event->buttons() & Qt::MidButton ) {
        display_painter->pan( x_during_last_mouse_event - event->x(), event->y() - y_during_last_mouse_event, width(), height() );
        update();
    }
    x_during_last_mouse_event = event->x();
    y_during_last_mouse_event = event->y();
}

void DisplayWidget::mouseReleaseEvent( QMouseEvent *event ) {
}

void DisplayWidget::resizeEvent( QResizeEvent *event ) {
}

void DisplayWidget::timerEvent( QTimerEvent *event ) {
    bool has_alpha_sup = false;
    for(int i=0;i<text_data.size();++i) {
        text_data[i].alpha -= 20;
        has_alpha_sup |= ( text_data[i].alpha >= 40 );
    }
    if ( not has_alpha_sup ) {
        text_data.clear();
        killTimer( timer_id );
        timer_id = 0;
    }
    QTimer::singleShot( 0, this, SLOT(update()) );
}

#endif // QT4_FOUND


