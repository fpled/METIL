#ifndef DISPLAY_PAINTER_CONTEXT_H
#define DISPLAY_PAINTER_CONTEXT_H

#include <limits>

///
struct DisplayPainterContext {
    ///
    struct Point {
        Point( double x, double y, double z = 0 ) : x(x), y(y), z(z) {}
        Point( double x, double y, double z, double w ) : x(x/w), y(y/w), z(z/w) {}
        Point operator+( const Point &p ) const { return Point( x + p.x, y + p.y, z + p.z ); }
        Point operator/( double w ) const { return Point( x / w, y / w, z / w ); }
        double x, y, z;
    };
    struct Matrix {
        Matrix();
        Matrix operator*( const Matrix &m );
        double &operator[]( int i )       { return data[ i ]; }
        double  operator[]( int i ) const { return data[ i ]; }
        double &operator()( int r, int c )       { return data[ 4 * r + c ]; }
        double  operator()( int r, int c ) const { return data[ 4 * r + c ]; }
        Point   operator()( const Point &p ) const;
        Matrix inverse() const;
        
        double data[ 4 * 4 ];
    };
    ///
    struct BackgroundImg {
        struct Pix {
            Pix() : c(0), l(0), a(0), z(std::numeric_limits<double>::max()) {}
            Pix( double c, double l = 1, double a = 1, double z = 0 ) : c(c), l(l), a(a), z(z) {}
            void add( double c_, double l_ = 1, double a_ = 1, double z_ = 0 ) {
                if ( z < z_ )
                    return;
                z = z_;
                double na = std::min( a + a_, 1.0 );
                na += na==0;
                c = std::min( a * c + a_ * c_, 1.0 ) / na;
                l = std::min( a * l + a_ * l_, 1.0 ) / na;
                a = na;
            }
            double c; // field value
            double l; // luminosity
            double a; // alpha chanel
            double z; // depth
        };
        
        BackgroundImg( int w, int h ) : w( w ), h( h ) { data = new Pix[ w * h ]; }
        ~BackgroundImg() { delete [] data; }
        Pix &operator()( int x, int y ) { return data[ y * w + x ]; }
        
        int w, h;
        Pix *data;
    };
    
    ///
    DisplayPainterContext( int w, int h, double x0, double y0, double x1, double y1 ) : img( w, h ), x0(x0), y0(y0), x1(x1), y1(y1), w(w), h(h) {
        xm = 0.5 * ( x0 + x1 );
        ym = 0.5 * ( y0 + y1 );
        w_div_2 = w / 2.0;
        h_div_2 = h / 2.0;
        //
        double scale_x = w / ( x1 - x0 );
        double scale_y = h / ( y1 - y0 );
        scale_r = std::min( scale_x, scale_y );
        inv_scale_r = 1.0 / scale_r;
    }
    
    double img_x( double world_x ) { return w_div_2 + ( world_x - xm ) * scale_r; }
    double img_y( double world_y ) { return h_div_2 + ( ym - world_y ) * scale_r; }
    
    double world_x( double img_x ) { return xm + ( img_x - w_div_2 ) * inv_scale_r; }
    double world_y( double img_y ) { return ym + ( h_div_2 - img_y ) * inv_scale_r; }
    
    ///
    BackgroundImg img;
    /// "minimum" box -> what we want to see
    double x0, y0, x1, y1, xm, ym, w, h;
    double w_div_2, h_div_2, scale_r, inv_scale_r;
};

#endif // DISPLAY_PAINTER_CONTEXT_H
