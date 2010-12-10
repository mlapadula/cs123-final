#ifndef DRAWENGINE_H
#define DRAWENGINE_H

#include <QHash>
#include <QString>
#include <qgl.h>
#include "glm.h"
#include "common.h"
#include <CS123Algebra.h>

class QGLContext;
class QGLShaderProgram;
class QFile;
class QGLFramebufferObject;
class QKeyEvent;

struct Model {
    GLMmodel *model;
    GLuint idx;
};

struct Camera {
    float3 eye, center, up;
    float fovy, near, far;
};

class DrawEngine {
public:

    //ctor and dtor
    DrawEngine(const QGLContext *context, int w, int h);
    ~DrawEngine();

    //methods
    void draw_frame(float time, int w, int h);
    void resize_frame(int w, int h);
    void mouse_wheel_event(int dx);
    void mouse_drag_event(float2 p0, float2 p1);
    void key_press_event(QKeyEvent *event);
    //getters and setters
    float fps() { return fps_; }

    //member variables

protected:

    //methods
    void perspective_camera(int w, int h);
    void drawKleinBottle();
    void orthogonal_camera(int w, int h);
    void textured_quad(int w, int h, bool flip);
    void realloc_framebuffers(int w, int h);
    void render_blur(float width, float height);
    void load_models();
    void load_textures();
    void load_shaders();
    GLuint load_cube_map(QList<QFile *> files);
    void create_fbos(int w, int h);
    void create_blur_kernel(int radius,int w,int h,GLfloat* kernel,GLfloat* offsets);
    void render_scene(QGLFramebufferObject* fb, Vector3 eye, Vector3 pos, Vector3 up, int w, int h, float time, float theta, float phi);
    void render_to_immediate_buffer(Vector3 eye, Vector3 pos, Vector3 up, int w, int h, float time);
    GLuint generate_refract_cube_map();

    int refract_every_so_often;
    GLuint refract_cube_map;
    GLuint refract_framebuffer;

    //member variables
    QHash<QString, QGLShaderProgram *>          shader_programs_; ///hash map of all shader programs
    QHash<QString, QGLFramebufferObject *>      framebuffer_objects_; ///hash map of all framebuffer objects
    QHash<QString, Model>                       models_; ///hashmap of all models
    QHash<QString, GLuint>                      textures_; ///hashmap of all textures
    const QGLContext                            *context_; ///the current OpenGL context to render to
    float                                       previous_time_, fps_; ///the previous time and the fps counter
    Camera                                      camera_; ///a simple camera struct

    Vector3 refract_center;
    GLuint checker_texture;

    /*GLuint load_cube_map(QGLFramebufferObject* posx, QGLFramebufferObject* negx, QGLFramebufferObject* posy,
                                     QGLFramebufferObject* negy, QGLFramebufferObject* posz, QGLFramebufferObject* negz, int w, int h);*/
};

#endif // DRAWENGINE_H
