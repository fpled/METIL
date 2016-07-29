#include "metil_qt_config.h"
#ifdef QT4_FOUND

#include <QtGui/QPixmap>
#include <QtGui/QImageWriter>
#include <QtGui/QPrinter>
#include <QtGui/QLinearGradient>
#include <QtGui/QPainter>
#include <QtGui/QMessageBox>
#include <QtSvg/QSvgGenerator>
#include <QtCore/QSettings>
#include <QtCore/QDebug>

#include "DisplayPainter.h"
#include <iostream>

QLinearGradient to_QLinearGradient( const DisplayPainter::SimpleGradient &sg, double x0, double y0, double x1, double y1 ) {
    QLinearGradient res( x0, y0, x1, y1 );
    for( int i=0; i < sg.data.size(); ++i )
        res.setColorAt( sg.data[i].p, QColor( 255 * sg.data[i].r, 255 * sg.data[i].g, 255 * sg.data[i].b, 255 * sg.data[i].a ) );
    return res;
}
void DisplayPainter::SimpleGradient::append( double p, double r, double g, double b, double a ) {
    Stop s; s.p = p; s.r = r; s.g = g; s.b = b; s.a = a;
    data.push_back( s );
}
QColor DisplayPainter::SimpleGradient::color_at( double p ) {
    p = std::min( 1.0, std::max( 0.0, p ) );
    for( int i=0; i < data.size(); ++i ) {
        if ( data[i+1].p >= p ) {
            double d = data[i+1].p - data[i].p; d += not d;
            return QColor( 
                255 * ( data[i].r + ( p - data[i].p ) / d * ( data[i+1].r - data[i].r ) ),
                255 * ( data[i].g + ( p - data[i].p ) / d * ( data[i+1].g - data[i].g ) ),
                255 * ( data[i].b + ( p - data[i].p ) / d * ( data[i+1].b - data[i].b ) ),
                255 * ( data[i].a + ( p - data[i].p ) / d * ( data[i+1].a - data[i].a ) )
            );
        }
    }
    return QColor( 0, 0, 0 );
}

QColor DisplayPainter::SimpleGradient::color_at( double p, double lum, double alpha ) {
    p   = std::min( 1.0, std::max( 0.0, p ) );
    for( int i=0; i < data.size(); ++i ) {
        if ( data[i+1].p >= p ) {
            double d = data[i+1].p - data[i].p; d += not d;
            return QColor( 
                255 * lum   * ( data[i].r + ( p - data[i].p ) / d * ( data[i+1].r - data[i].r ) ),
                255 * lum   * ( data[i].g + ( p - data[i].p ) / d * ( data[i+1].g - data[i].g ) ),
                255 * lum   * ( data[i].b + ( p - data[i].p ) / d * ( data[i+1].b - data[i].b ) ),
                255 * alpha * ( data[i].a + ( p - data[i].p ) / d * ( data[i+1].a - data[i].a ) )
            );
        }
    }
    return QColor( 0, 0, 0 );
}

DisplayPainter::DisplayPainter() {
    x0 = 0; y0 = 0;
    x1 = 1; y1 = 1;
    anti_aliasing          = 0;
    borders                = 0;
    shrink                 = 1;
    zoom_should_be_updated = 0;
    
    color_bar_gradient.append( 0.0, 0.0, 0.0, 1.0 );
    color_bar_gradient.append( 0.3, 0.0, 1.0, 0.0 );
    color_bar_gradient.append( 0.6, 1.0, 0.0, 0.0 );
    color_bar_gradient.append( 1.0, 1.0, 1.0, 1.0 );
}

static void load_color_set_settings( QSettings *settings, DisplayPainter::ColorSet &color_set, QString mode ) {
    QColor dfg(   0,   0,   0 );
    QColor dbg( 255, 255, 255 );
    QColor db0( 155, 155, 255 );
    QColor db1( 255, 255, 255 );
    if ( mode == "night" ) {
        dfg = QColor( 255, 255, 255 );
        dbg = QColor(   0,   0,   0 );
        db0 = QColor(   0,   0,   0 );
        db1 = QColor(   0,   0,  40 );
    } else if ( mode == "print" ) {
        db0 = db1;
    }
    settings->beginGroup( mode );
    color_set.fg_color   = settings->value( "fg_color"     , dfg ).value<QColor>();
    color_set.bg_color_0 = settings->value( "bg_color"     , dbg ).value<QColor>();
    color_set.bg_color_0 = settings->value( "bg_color_0"   , db0 ).value<QColor>();
    color_set.bg_color_1 = settings->value( "bg_color_1"   , db1 ).value<QColor>();
    settings->endGroup();
}

void DisplayPainter::load_settings( QSettings *settings ) {
    settings->beginGroup( "DisplayPainter" );
    anti_aliasing  = settings->value( "anti_aliasing" , false ).toBool  ();
    borders        = settings->value( "borders"       , false ).toBool  ();
    shrink         = settings->value( "shrink"        , 1     ).toDouble();
    // names of color_sets
    QStringList color_modes = settings->value( "color_modes", "night day print" ).toString().split(" ",QString::SkipEmptyParts);
    if ( not color_modes.size() )
        color_modes = QString( "night day print" ).split(" ");
    //
    cur_color_mode = settings->value( "cur_color_mode", color_modes[0] ).toString();
    for(int i=0;i<color_modes.size();++i)
        load_color_set_settings( settings, color_sets[ color_modes[i] ], color_modes[i] );
    settings->endGroup();
    
    //
    cur_color_set = color_sets[ cur_color_mode ];
}

static void save_color_set_settings( QSettings *settings, const DisplayPainter::ColorSet &color_set, QString mode ) {
    settings->beginGroup( mode );
    settings->setValue( "fg_color"   , color_set.fg_color   );
    settings->setValue( "bg_color"   , color_set.bg_color   );
    settings->setValue( "bg_color_0" , color_set.bg_color_0 );
    settings->setValue( "bg_color_1" , color_set.bg_color_1 );
    settings->endGroup();
}

void DisplayPainter::save_settings( QSettings *settings ) {
    settings->beginGroup( "DisplayPainter" );
    settings->setValue( "anti_aliasing" , anti_aliasing  );
    settings->setValue( "borders"       , borders        );
    settings->setValue( "shrink"        , shrink         );
    settings->setValue( "cur_color_mode", cur_color_mode );
    QString color_modes;
    for( QMap<QString,ColorSet>::const_iterator iter = color_sets.begin(); iter != color_sets.end(); ) {
        save_color_set_settings( settings, iter.value(), iter.key() );
        color_modes += iter.key();
        if ( ++iter != color_sets.end() )
            color_modes += " ";
    }
    settings->setValue( "color_modes"   , color_modes    );
    settings->endGroup();
}

void DisplayPainter::add_paint_function( void *make_tex_function, void *paint_function, void *bounding_box_function, void *data ) {
    DispFun df;
    df.make_tex_function     = reinterpret_cast<MakeTexFunction     *>( make_tex_function     );
    df.paint_function        = reinterpret_cast<PaintFunction       *>( paint_function        );
    df.bounding_box_function = reinterpret_cast<BoundingBoxFunction *>( bounding_box_function );
    df.data                  = data;

    paint_functions.push_back( df );
    zoom_should_be_updated = true;
}

void DisplayPainter::rm_paint_functions() {
    paint_functions.clear();
}

void DisplayPainter::change_color_mode( QString cm ) {
    cur_color_mode = cm;
    cur_color_set  = color_sets[ cm ];
}

void DisplayPainter::save_as( const QString &filename, int w, int h ) {
    if ( filename.endsWith( ".pdf" ) ) {
        QPrinter printer( QPrinter::HighResolution );
        // printer.setPageSize( QPrinter::Custom );
        printer.setPaperSize     ( QSizeF( w, h ), QPrinter::DevicePixel );
        printer.setPageMargins   ( 0, 0, 0, 0, QPrinter::DevicePixel );
        printer.setOutputFileName( filename );
        //
        QPainter painter( &printer );
        paint( painter, printer.pageRect().width(), printer.pageRect().height() );
    } else if ( filename.endsWith( ".svg" ) ) {
        QSvgGenerator printer;
        printer.setSize( QSize( w, h ) );
        printer.setFileName( filename );
        //
        QPainter painter( &printer );
        paint( painter, w, h );
    } else {
        QList<QByteArray> lst_ext_pix = QImageWriter::supportedImageFormats();
        bool pix_ok = false;
        for(int i=0;i<lst_ext_pix.size();++i)
            pix_ok |= filename.endsWith( lst_ext_pix[i] );
        if ( pix_ok ) {
            QPixmap printer( w, h );
            //
            QPainter painter( &printer );
            paint( painter, w, h );
            printer.save( filename );
        } else
            QMessageBox::warning( NULL, "save as problem", filename + " does not have an allowed extension. Only png, jpg, svg and pdf are allowed." );
    }
}

double DisplayPainter::get_scale_r( double w, double h ) const {
    double scale_x = w / ( x1 - x0 );
    double scale_y = h / ( y1 - y0 );
    return std::min( scale_x, scale_y );
}

void DisplayPainter::zoom( double fact, double x, double y, double w, double h ) {
    double xd = ( x - 0.5 * w ) / get_scale_r( w, h ) + xm();
    double yd = ( 0.5 * h - y ) / get_scale_r( w, h ) + ym();
    double ox0 = x0, oy0 = y0, ox1 = x1, oy1 = y1;
    x0 = xd + fact * ( ox0 - xd ); y0 = yd + fact * ( oy0 - yd );
    x1 = xd + fact * ( ox1 - xd ); y1 = yd + fact * ( oy1 - yd );
}

void DisplayPainter::pan( double dx, double dy, double w, double h ) {
    dx /= get_scale_r( w, h );
    dy /= get_scale_r( w, h );
    x0 += dx; y0 += dy;
    x1 += dx; y1 += dy;
}

void DisplayPainter::set_min_max( double mi_, double ma_ ) {
    mi = mi_; ma = ma_;
}


void DisplayPainter::init_bb() {
    x0 = std::numeric_limits<double>::max();
    y0 = std::numeric_limits<double>::max();
    x1 = std::numeric_limits<double>::min();
    y1 = std::numeric_limits<double>::min();
}

void DisplayPainter::add_to_bb( double x, double y ) {
    x0 = std::min( x0, x );
    y0 = std::min( y0, y );
    x1 = std::max( x1, x );
    y1 = std::max( y1, y );
}

void DisplayPainter::paint( QPainter &painter, int w, int h ) {
    // hints
    painter.setRenderHint( QPainter::Antialiasing, anti_aliasing );
    if ( not w ) w = x1 - x0;
    if ( not h ) h = x1 - x0;
    
    // recompute x0, ... ?
    if ( zoom_should_be_updated ) {
        init_bb();
        for(int i=0;i<paint_functions.size();++i)
            paint_functions[i].bounding_box_function( this, paint_functions[i].data );
        double dx = x1 - x0, dy = y1 - y0, xm = 0.5 * ( x0 + x1 ), ym = 0.5 * ( y0 + y1 );
        x0 = xm - dx * 0.6; y0 = ym - dy * 0.6;
        x1 = xm + dx * 0.6; y1 = ym + dy * 0.6;
        zoom_should_be_updated = false;
    }
    
    // background
    QLinearGradient bg_gradient( QPoint( 0, 0 ), QPoint( 0, h ) );
    bg_gradient.setColorAt( 0.0, cur_color_set.bg_color_0 );
    bg_gradient.setColorAt( 1.0, cur_color_set.bg_color_1 );
    painter.setBrush( bg_gradient );
    painter.setPen  ( Qt::NoPen   );
    painter.drawRect( 0, 0, w, h );
    
    // transf
    DisplayPainterContext dc( w, h, x0, y0, x1, y1 );
    
    // data -> texture
    for(int i=0;i<paint_functions.size();++i)
        paint_functions[i].make_tex_function( dc, this, paint_functions[i].data );
    QImage qimg( w, h, QImage::Format_ARGB32 );
    for(int j=0;j<h;++j) {
        for(int i=0;i<w;++i) {
            QColor col = color_bar_gradient.color_at( dc.img(i,j).c, dc.img(i,j).l, dc.img(i,j).a );
            qimg.setPixel( i, j, col.rgba() );
        }
    }
    painter.drawImage( QRect(0,0,w,h), qimg );
    
    // data -> vectorial painter data
    painter.setBrush( Qt::NoBrush );
    for(int i=0;i<paint_functions.size();++i)
        paint_functions[i].paint_function( painter, dc, this, paint_functions[i].data );
}



#endif // QT4_FOUND
