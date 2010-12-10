//commenting is overrated

#include "glwidget.h"
#include "drawengine.h"

#include <qgl.h>
#include <GL/glu.h>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QTimer>
#include <QTime>
#include <QFileDialog>
#include <QGLShader>
#include <QGLShaderProgram>
#include <QFile>

#include "particleemitter.h"

GLWidget::GLWidget(QWidget *parent) :
    QGLWidget(QGLFormat(QGL::DoubleBuffer), parent) {
    this->setFocusPolicy(Qt::StrongFocus);
    this->setMouseTracking(true);
    this->setAutoBufferSwap(false);
    prev_fps_ = 0.f;
}

GLWidget::~GLWidget() {
    delete timer_, delete time_;
}

void GLWidget::initializeGL() {
    draw_engine_ = new DrawEngine(this->context(), this->width(), this->height());
    time_ = new QTime();
    timer_ = new QTimer(this);
    connect(timer_, SIGNAL(timeout()), this, SLOT(repaint()));
    timer_->start(30.f);


    m_emitter = new ParticleEmitter(loadTexture(QFile(":/textures/Particle2.bmp")));
}

void GLWidget::paintGL() {
    draw_engine_->draw_frame(time_->elapsed(),
                             this->width(), this->height());
    render_text();

    /*m_emitter->updateParticles();       //Move the particles
    m_emitter->drawParticles();         //Draw the particles*/

    glFlush();
    swapBuffers();
}

void GLWidget::mouseMoveEvent(QMouseEvent *event) {
    float2 pos = {event->x(), event->y()};
    if(event->buttons() & Qt::LeftButton || event->buttons() & Qt::RightButton)
        draw_engine_->mouse_drag_event(mouse_pos_prev_, pos);
    mouse_pos_prev_ = pos;
}

void GLWidget::mousePressEvent(QMouseEvent *event) {
    mouse_pos_prev_.x = event->x(), mouse_pos_prev_.y = event->y();
}

void GLWidget::wheelEvent(QWheelEvent *event) {
    draw_engine_->mouse_wheel_event(event->delta());
}

void GLWidget::resizeGL(int width, int height) {
    draw_engine_->resize_frame(width, height);
}

void GLWidget::keyPressEvent(QKeyEvent *event) {
    switch(event->key()) {
        case Qt::Key_S:
        QImage qi = grabFrameBuffer(false);
        QString filter;
        QString fileName = QFileDialog::getSaveFileName(this,
             tr("Save Image"), "", tr("PNG Image (*.png)"), &filter);
        qi.save(QFileInfo(fileName).absoluteDir().absolutePath() + "/" + QFileInfo(fileName).baseName() + ".png", "PNG", 100);
        break;
    }
    draw_engine_->key_press_event(event);
}

void GLWidget::render_text() {
    glColor3f(1.f, 1.f, 1.f);
    QFont f("Deja Vu Sans Mono", 8, 4, false);
    float fps =  draw_engine_->fps();
    if (fps >= 0 && fps < 1000) {
       prev_fps_ *= 0.95;
       prev_fps_ += draw_engine_->fps() * 0.05;

    } this->renderText(10.0, 20.0, "FPS: " + QString::number((int)(prev_fps_)), f);
    this->renderText(10.0, 35.0, "S: Save screenshot", f);
}

/**
  This method should load the specified image file as a texture in video memory
  and return its texture id.

  @TODO: Finish filling this in!
  **/
GLuint GLWidget::loadTexture(const QFile &file) {
    QImage image, texture;
    if(!file.exists()) return -1;
    image.load(file.fileName());
    texture = QGLWidget::convertToGLFormat(image);
    GLuint textureID;
    //Put your code here

    glEnable(GL_TEXTURE_2D);

    glGenTextures(1, &textureID);

    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, 3, texture.width(), texture.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.bits());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return textureID; /* return something meaningful */
}
