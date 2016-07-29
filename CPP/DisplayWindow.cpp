#include "metil_qt_config.h"
#ifdef QT4_FOUND

#include <QtGui/QApplication>
#include <QtGui/QKeyEvent>
#include <QtGui/QMessageBox>
#include <QtGui/QPrintDialog>
#include <QtGui/QPrinter>
#include <QtGui/QPainter>
#include <QtCore/QTimer>
#include <QtCore/QDebug>
#include <QtCore/QSettings>
#include <QtCore/QFile>

#include "DisplayWindow.h"
#include "DisplayWidget.h"
#include "DisplayPainter.h"

// --------------------------------------------------------------------------------------------------------------------------

DisplayWindow::DisplayWindow( DisplayPainter *display_painter, QSettings *settings ) {
    disp_widget = new DisplayWidget( this, display_painter );
    setCentralWidget( disp_widget );
    load_settings( settings );
}

void DisplayWindow::load_settings( QSettings *settings ) {
    settings->beginGroup("DisplayWindow");
     resize( settings->value( "size", QSize( 400, 400 ) ).toSize() );
     img_prefix = settings->value( "img_prefix", "img_" ).toString();
     img_suffix = settings->value( "img_suffix", ".pdf" ).toString();
    settings->endGroup();
}

void DisplayWindow::save_settings( QSettings *settings ) {
    settings->beginGroup("DisplayWindow");
     settings->setValue( "size", size() );
     settings->setValue( "img_prefix", img_prefix );
     settings->setValue( "img_suffix", img_suffix );
    settings->endGroup();
}
    
void DisplayWindow::keyPressEvent( QKeyEvent *event ) {
    if ( event->key() == Qt::Key_Q or event->key() == Qt::Key_Escape ) {
        close();
    } else if ( event->key() == Qt::Key_A ) {
        disp_widget->display_painter->anti_aliasing = not disp_widget->display_painter->anti_aliasing;
        disp_widget->add_vanishing_text( "Anti-aliasing " + QString( disp_widget->display_painter->anti_aliasing ? "on" : "off" ) );
        // QTimer::singleShot( 0, disp_widget, SLOT(update()) );
    } else if ( event->key() == Qt::Key_B ) {
        disp_widget->display_painter->borders = not disp_widget->display_painter->borders;
        disp_widget->add_vanishing_text( "Borders " + QString( disp_widget->display_painter->borders ? "on" : "off" ) );
        // QTimer::singleShot( 0, disp_widget, SLOT(update()) );
    } else if ( event->key() == Qt::Key_F ) {
        disp_widget->display_painter->zoom_should_be_updated = true;
        disp_widget->add_vanishing_text( "Fit" );
        // QTimer::singleShot( 0, disp_widget, SLOT(update()) );
    } else if ( event->key() == Qt::Key_S ) {
        if ( event->modifiers() & Qt::ControlModifier ) { // save prefs
            disp_widget->add_vanishing_text( "Save settings" );
            QSettings settings( "LMT", "DisplayWindowCreator" );
            save_settings( &settings );
            disp_widget->display_painter->save_settings( &settings );
        } else { // save img
            QString filename;
            for(unsigned i=0;;++i) {
                filename = img_prefix + QString::number( i ) + img_suffix;
                if ( not QFile::exists( filename ) )
                    break;
            }
            disp_widget->add_vanishing_text( "Save Image '" + filename + "'" );
            
            //
            QString old_color_mode = disp_widget->display_painter->cur_color_mode;
            if ( img_suffix == ".pdf" )
                disp_widget->display_painter->change_color_mode( "print" );
            disp_widget->display_painter->save_as( filename, disp_widget->width(), disp_widget->height() );
            if ( img_suffix == ".pdf" )
                disp_widget->display_painter->change_color_mode( old_color_mode );
        }
    } else if ( event->key() == Qt::Key_P ) {
        QPrinter printer;
        QPrintDialog printDialog( &printer, NULL );
        if ( printDialog.exec() == QDialog::Accepted ) {
            QPainter painter( &printer );
            QRectF pr( printer.pageRect() );
            disp_widget->display_painter->paint( painter, pr.width(), pr.height() );
        }
    } else if ( event->key() == Qt::Key_H ) {
        QMessageBox::information( NULL, "Help", tr(
            "<strong>Bindings</strong> :" \
            "<table><tbody>" \
            "<tr><td> <em>Q or Esc     </em> </td><td></td> quit </tr>" \
            "<tr><td> <em>A            </em> </td><td></td> toggle anti-aliasing </tr>" \
            "<tr><td> <em>B            </em> </td><td></td> toggle surface borders </tr>" \
            "<tr><td> <em>F            </em> </td><td></td> fit in window </tr>" \
            "<tr><td> <em>H            </em> </td><td></td> help </tr>" \
            "<tr><td> <em>P            </em> </td><td></td> print </tr>" \
            "<tr><td> <em>S            </em> </td><td></td> save current image </tr>" \
            "<tr><td> <em>ctrl-S       </em> </td><td></td> save current image </tr>" \
            "<tr><td> <em>wheel        </em> </td><td></td> zoom </tr>" \
            "<tr><td> <em>shift-wheel  </em> </td><td></td> surface shrink ratio </tr>" \
            "<tr><td> <em>middle button</em> </td><td></td> pan </tr>" \
            "</tbody>" \
            "</table>" \
        ) );
    }
}

#endif
