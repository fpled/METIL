#ifndef DISPLAY_WINDOW_CREATOR_H
#define DISPLAY_WINDOW_CREATOR_H

#include "metil_qt_config.h"
#ifdef QT4_FOUND

#include <QtCore/QObject>
#include <QtCore/QWaitCondition>
#include <QtCore/QMutex>
#include <QtCore/QMap>

#include "DisplayPainter.h"

/** */
class DisplayWindowCreator : public QObject {
    Q_OBJECT
public:
    struct Display {
        Display() : window( NULL ) {}
        class DisplayWindow *window;
        DisplayPainter painter;
    };
    enum {
        ID_SHRINK       = 0,
        ID_ANTIALIASING = 1,
        ID_BORDERS      = 2,
        ID_FIT          = 3
    };
    
    DisplayWindowCreator();
    void call_add_paint_function( int num_display_window, void *make_tex_function, void *paint_function, void *bounding_box_function, void *data );
    void call_save_as           ( int num_display_window, const char *s, int si, int w, int h );
    void call_update_disp_widget( int num_display_window );
    void call_rm_paint_functions( int num_display_window );
    void call_set_val           ( int num_display_window, int id_val, int val );
    void call_set_min_max       ( int num_display_window, double mi, double ma );
    
    void wait_for_display_windows();
    class QSettings *get_settings();
signals:
    void sig_add_paint_function( int, void *, void *, void *, void * );
    void sig_save_as           ( int, QString, int, int      );
    void sig_update_disp_widget( int                         );
    void sig_rm_paint_functions( int                         );
    void sig_set_val           ( int, int, int               );
    void sig_set_min_max       ( int, double, double         );
public slots: // must be called in main_thread
    void add_paint_function( int num_display_window, void *make_tex_function, void *paint_function, void *bounding_box_function, void *data );
    void save_as           ( int num_display_window, QString f, int w, int h );
    void update_disp_widget( int num_display_window );
    void rm_paint_functions( int num_display_window );
    void set_val           ( int num_display_window, int id_val, int val );
    void set_min_max       ( int num_display_window, double mi, double ma );
    
    void lastWindowClosed();
private:
    void make_num_display_window_if_necessary( int num_display_window );
    class DisplayWindow *new_disp_win_if_nec( Display &d );
    
    void load_setting();
    void save_setting();
    QMutex mutex; // for the wait condition
    QWaitCondition last_window_closed;
    bool at_least_one_window_was_created;
    QMap<int,Display> displays;
    class QSettings *_settings;
};

void __wait_for_display_windows__( struct Thread *th );

#else // QT4_FOUND

class DisplayWindowCreator {
public:
    void call_add_paint_function( int num_display_window, void *make_tex_function, void *paint_function, void *bounding_box_function, void *data ) {}
    void call_save_as           ( int num_display_window, const char *s, int si, int w, int h ) {}
    void call_update_disp_widget( int num_display_window ) {}
    void call_rm_paint_functions( int num_display_window ) {}
    void call_set_val           ( int num_display_window, int id_val, int val ) {}
    void call_set_min_max       ( int num_display_window, double mi, double ma ) {}
    
    void wait_for_display_windows() {}
};

inline void __wait_for_display_windows__( struct Thread *th ) { }

#endif // QT4_FOUND

#endif // DISPLAY_WINDOW_CREATOR_H
