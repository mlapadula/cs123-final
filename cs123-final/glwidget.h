#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QGLWidget>
#include "common.h"
class DrawEngine;
class QTimer;
class QTime;

class QFile;
class ParticleEmitter;

class GLWidget : public QGLWidget {
    Q_OBJECT
public:
    GLWidget(QWidget *parent = 0);
    ~GLWidget();
    static GLuint loadTexture(const QFile &file);
protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);

    void render_text();
    void wheelEvent(QWheelEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    QTimer *timer_;
    QTime *time_;
    DrawEngine *draw_engine_;
    float prev_fps_;
    float2 mouse_pos_prev_;


    ParticleEmitter *m_emitter;
};

#endif // GLWIDGET_H
