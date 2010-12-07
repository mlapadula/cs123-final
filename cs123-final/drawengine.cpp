/**
  A simple OpenGL drawing engine.

  @author psastras
**/

#include "drawengine.h"
#include "glm.h"
#include <qgl.h>
#include <QKeyEvent>
#include <QGLContext>
#include <QHash>
#include <QGLShaderProgram>
#include <QQuaternion>
#include <QVector3D>
#include <QString>
#include <GL/glu.h>
#include <iostream>
#include <QFile>
#include <QGLFramebufferObject>
#define GL_GLEXT_PROTOTYPES
#include <GL/glext.h>
#include <CS123Algebra.h>

using std::cout;
using std::endl;

extern "C"{
    extern void APIENTRY glActiveTexture (GLenum);
    extern GLboolean APIENTRY glIsRenderbufferEXT (GLuint);
    extern void APIENTRY glBindRenderbufferEXT (GLenum, GLuint);
    extern void APIENTRY glDeleteRenderbuffersEXT (GLsizei, const GLuint *);
    extern void APIENTRY glGenRenderbuffersEXT (GLsizei, GLuint *);
    extern void APIENTRY glRenderbufferStorageEXT (GLenum, GLenum, GLsizei, GLsizei);
    extern void APIENTRY glGetRenderbufferParameterivEXT (GLenum, GLenum, GLint *);
    extern GLboolean APIENTRY glIsFramebufferEXT (GLuint);
    extern void APIENTRY glBindFramebufferEXT (GLenum, GLuint);
    extern void APIENTRY glDeleteFramebuffersEXT (GLsizei, const GLuint *);
    extern void APIENTRY glGenFramebuffersEXT (GLsizei, GLuint *);
    extern GLenum APIENTRY glCheckFramebufferStatusEXT (GLenum);
    extern void APIENTRY glFramebufferTexture1DEXT (GLenum, GLenum, GLenum, GLuint, GLint);
    extern void APIENTRY glFramebufferTexture2DEXT (GLenum, GLenum, GLenum, GLuint, GLint);
    extern void APIENTRY glFramebufferTexture3DEXT (GLenum, GLenum, GLenum, GLuint, GLint, GLint);
    extern void APIENTRY glFramebufferRenderbufferEXT (GLenum, GLenum, GLenum, GLuint);
}

/**
  @paragraph DrawEngine ctor.  Expects a Valid OpenGL context and the viewport's current
  width and height.  Initializes the draw engine.  Loads models,textures,shaders,
  and allocates framebuffers.  Also sets up OpenGL to begin drawing.

  @param context The current OpenGL context this drawing engine is associated with.
  Probably should be the context from the QGLWidget.

  @param w The viewport width used to allocate the correct framebuffer size.
  @param h The viewport heigh used to alloacte the correct framebuffer size.

**/
DrawEngine::DrawEngine(const QGLContext *context,int w,int h) : context_(context) {

    //initialize ogl settings
    glEnable(GL_TEXTURE_2D);
    glFrontFace(GL_CCW);
    glDisable(GL_DITHER);
    glDisable(GL_LIGHTING);
    glShadeModel(GL_FLAT);
    glClearColor(0.0f,0.0f,0.0f,0.0f);
    //init member variables
    previous_time_ = 0.0f;
    camera_.center.x = 0.f,camera_.center.y = 0.f,camera_.center.z = 0.f;
    camera_.eye.x = 0.f,camera_.eye.y = 0.0f,camera_.eye.z = -2.f;
    camera_.up.x = 0.f,camera_.up.y = 1.f,camera_.up.z = 0.f;
    camera_.near = 0.1f,camera_.far = 100.f;
    camera_.fovy = 60.f;

    //init resources - so i heard you like colored text?
    cout << "Using OpenGL Version " << glGetString(GL_VERSION) << endl << endl;
    //ideally we would now check to make sure all the OGL functions we use are supported
    //by the video card.  but that's a pain to do so we're not going to.
    cout << "Loading Resources..." << endl;
    load_models();
    load_shaders();
    load_textures();
    create_fbos(w,h);
    refract_center = Vector3(2,0,0);
    refract_cube_map = generate_refract_cube_map();
    refract_every_so_often = 0;
    cout << "Rendering..." << endl;
}

/**
  @paragraph Dtor
**/
DrawEngine::~DrawEngine() {
    foreach(QGLShaderProgram *sp,shader_programs_)
        delete sp;
    foreach(QGLFramebufferObject *fbo,framebuffer_objects_)
        delete fbo;
    foreach(GLuint id,textures_)
        ((QGLContext *)(context_))->deleteTexture(id);
    foreach(Model m,models_)
        glmDelete(m.model);
}

/**
  @paragraph Loads models used by the program.  Caleed by the ctor once upon
  initialization.
**/
void DrawEngine::load_models() {
    cout << "Loading models..." << endl;
    models_["dragon"].model = glmReadOBJ("../cs123-final/models/xyzrgb_dragon.obj");
    glmUnitize(models_["dragon"].model);
    models_["dragon"].idx = glmList(models_["dragon"].model,GLM_SMOOTH);
    cout << "models/xyzrgb_dragon_old.obj" << endl;
    //Create grid
    models_["grid"].idx = glGenLists(1);
    glNewList(models_["grid"].idx,GL_COMPILE);
    float r = 1.f,dim = 10,delta = r * 2 / dim;
    for(int y = 0; y < dim; ++y) {
        glBegin(GL_QUAD_STRIP);
        for(int x = 0; x <= dim; ++x) {
            glVertex2f(x * delta - r,y * delta - r);
            glVertex2f(x * delta - r,(y + 1) * delta - r);
        }
        glEnd();
    }
    glEndList();
    cout << "grid compiled" << endl;
    models_["skybox"].idx = glGenLists(1);
    glNewList(models_["skybox"].idx,GL_COMPILE);
    //Be glad we wrote this for you...ugh.
    glBegin(GL_QUADS);
    float fExtent = 50.f;
    glTexCoord3f(1.0f,-1.0f,-1.0f); glVertex3f(fExtent,-fExtent,-fExtent);
    glTexCoord3f(-1.0f,-1.0f,-1.0f);glVertex3f(-fExtent,-fExtent,-fExtent);
    glTexCoord3f(-1.0f,1.0f,-1.0f);glVertex3f(-fExtent,fExtent,-fExtent);
    glTexCoord3f(1.0f,1.0f,-1.0f); glVertex3f(fExtent,fExtent,-fExtent);
    glTexCoord3f(1.0f,-1.0f,1.0f);glVertex3f(fExtent,-fExtent,fExtent);
    glTexCoord3f(1.0f,-1.0f,-1.0f); glVertex3f(fExtent,-fExtent,-fExtent);
    glTexCoord3f(1.0f,1.0f,-1.0f);  glVertex3f(fExtent,fExtent,-fExtent);
    glTexCoord3f(1.0f,1.0f,1.0f); glVertex3f(fExtent,fExtent,fExtent);
    glTexCoord3f(-1.0f,-1.0f,1.0f);  glVertex3f(-fExtent,-fExtent,fExtent);
    glTexCoord3f(1.0f,-1.0f,1.0f); glVertex3f(fExtent,-fExtent,fExtent);
    glTexCoord3f(1.0f,1.0f,1.0f);  glVertex3f(fExtent,fExtent,fExtent);
    glTexCoord3f(-1.0f,1.0f,1.0f); glVertex3f(-fExtent,fExtent,fExtent);
    glTexCoord3f(-1.0f,-1.0f,-1.0f); glVertex3f(-fExtent,-fExtent,-fExtent);
    glTexCoord3f(-1.0f,-1.0f,1.0f);glVertex3f(-fExtent,-fExtent,fExtent);
    glTexCoord3f(-1.0f,1.0f,1.0f); glVertex3f(-fExtent,fExtent,fExtent);
    glTexCoord3f(-1.0f,1.0f,-1.0f);glVertex3f(-fExtent,fExtent,-fExtent);
    glTexCoord3f(-1.0f,1.0f,-1.0f);glVertex3f(-fExtent,fExtent,-fExtent);
    glTexCoord3f(-1.0f,1.0f,1.0f);glVertex3f(-fExtent,fExtent,fExtent);
    glTexCoord3f(1.0f,1.0f,1.0f);glVertex3f(fExtent,fExtent,fExtent);
    glTexCoord3f(1.0f,1.0f,-1.0f);glVertex3f(fExtent,fExtent,-fExtent);
    glTexCoord3f(-1.0f,-1.0f,-1.0f);glVertex3f(-fExtent,-fExtent,-fExtent);
    glTexCoord3f(-1.0f,-1.0f,1.0f);glVertex3f(-fExtent,-fExtent,fExtent);
    glTexCoord3f(1.0f,-1.0f,1.0f); glVertex3f(fExtent,-fExtent,fExtent);
    glTexCoord3f(1.0f,-1.0f,-1.0f);glVertex3f(fExtent,-fExtent,-fExtent);
    glEnd();
    glEndList();
    cout << "skybox compiled" << endl;
}
/**
  @paragraph Loads shaders used by the program.  Caleed by the ctor once upon
  initialization.
**/
void DrawEngine::load_shaders() {
    cout << "Loading shaders..." << endl;
    shader_programs_["reflect"] = new QGLShaderProgram(context_);
    shader_programs_["reflect"]->addShaderFromSourceFile(QGLShader::Vertex,
                                                       "../cs123-final/shaders/reflect.vert");
    shader_programs_["reflect"]->addShaderFromSourceFile(QGLShader::Fragment,
                                                       "../cs123-final/shaders/reflect.frag");
    shader_programs_["reflect"]->link();
    cout << "shaders/reflect" << endl;
    shader_programs_["refract"] = new QGLShaderProgram(context_);
    shader_programs_["refract"]->addShaderFromSourceFile(QGLShader::Vertex,
                                                       "../cs123-final/shaders/refract.vert");
    shader_programs_["refract"]->addShaderFromSourceFile(QGLShader::Fragment,
                                                       "../cs123-final/shaders/refract.frag");
    shader_programs_["refract"]->link();
    cout << "shaders/refract" << endl;
    shader_programs_["brightpass"] = new QGLShaderProgram(context_);
    shader_programs_["brightpass"]->addShaderFromSourceFile(QGLShader::Fragment,
                                                       "../cs123-final/shaders/brightpass.frag");
    shader_programs_["brightpass"]->link();
    cout << "shaders/brightpass" << endl;

    shader_programs_["blur"] = new QGLShaderProgram(context_);
    shader_programs_["blur"]->addShaderFromSourceFile(QGLShader::Fragment,
                                                       "../cs123-final/shaders/blur.frag");
    shader_programs_["blur"]->link();
    cout << "shaders/blur" << endl;
}
/**
  @paragraph Loads textures used by the program.  Caleed by the ctor once upon
  initialization.
**/
void DrawEngine::load_textures() {
    cout << "Loading textures..." << endl;
    QList<QFile *> fileList;
    fileList.append(new QFile("../cs123-final/textures/astra/posx.jpg"));
    fileList.append(new QFile("../cs123-final/textures/astra/negx.jpg"));
    fileList.append(new QFile("../cs123-final/textures/astra/posy.jpg"));
    fileList.append(new QFile("../cs123-final/textures/astra/negy.jpg"));
    fileList.append(new QFile("../cs123-final/textures/astra/posz.jpg"));
    fileList.append(new QFile("../cs123-final/textures/astra/negz.jpg"));
    textures_["cube_map_1"] = load_cube_map(fileList);
}

GLuint DrawEngine::generate_refract_cube_map() {
    GLuint id;
    glGenTextures(1,&id);
    glBindTexture(GL_TEXTURE_CUBE_MAP,id);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //NULL means reserve texture memory, but texels are undefined
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+0, 0, GL_RGBA8, 2048, 2048, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+1, 0, GL_RGBA8, 2048, 2048, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+2, 0, GL_RGBA8, 2048, 2048, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+3, 0, GL_RGBA8, 2048, 2048, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+4, 0, GL_RGBA8, 2048, 2048, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+5, 0, GL_RGBA8, 2048, 2048, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

    glBindTexture(GL_TEXTURE_CUBE_MAP,0);
    return id;
}

/**
  @paragraph Creates the intial framebuffers for drawing.  Called by the ctor once
  upon initialization.

  @todo Finish filling this in.

  @param w:    the viewport width
  @param h:    the viewport height
**/
void DrawEngine::create_fbos(int w,int h) {

    //Allocate the main framebuffer object for rendering the scene to
    //This needs a depth attachment.
    framebuffer_objects_["fbo_0"] = new QGLFramebufferObject(w,h,QGLFramebufferObject::Depth,
                                                             GL_TEXTURE_2D,GL_RGB16F_ARB);
    framebuffer_objects_["fbo_0"]->format().setSamples(16);
    //Allocate the secondary framebuffer obejcts for rendering textures to (post process effects)
    //These do not require depth attachments.
    framebuffer_objects_["fbo_1"] = new QGLFramebufferObject(w,h,QGLFramebufferObject::NoAttachment,
                                                             GL_TEXTURE_2D,GL_RGB16F_ARB);
    //You need to create another framebuffer here.  Look up two lines to see how to do this... =.=
    framebuffer_objects_["fbo_2"] = new QGLFramebufferObject(w,h,QGLFramebufferObject::NoAttachment,
                                                             GL_TEXTURE_2D,GL_RGB16F_ARB);

    glGenFramebuffersEXT(1, &refract_framebuffer);
}
/**
  @paragraph Reallocates all the framebuffers.  Called when the viewport is
  resized.

  @param w:    the viewport width
  @param h:    the viewport height
**/
void DrawEngine::realloc_framebuffers(int w,int h) {
    foreach(QGLFramebufferObject *fbo,framebuffer_objects_)  {
        const QString &key = framebuffer_objects_.key(fbo);
        QGLFramebufferObjectFormat format = fbo->format();
        delete fbo;
        framebuffer_objects_[key] = new QGLFramebufferObject(w,h,format);
    }
}

/**
  @paragraph Should render one frame at the given elapsed time in the program.
  Assumes that the GL context is valid when this method is called.

  @todo Finish filling this in

  @param time: the current program time in milliseconds
  @param w:    the viewport width
  @param h:    the viewport height

**/
void DrawEngine::draw_frame(float time,int w,int h) {
    fps_ = 1000.f / (time - previous_time_),previous_time_ = time;


    // render the cube map every other frame to save on computing power
    if (refract_every_so_often % 2 == 0) {
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, refract_framebuffer);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_CUBE_MAP_POSITIVE_X, refract_cube_map, 0);
        render_to_immediate_buffer(Vector3(refract_center.x,refract_center.y,refract_center.z),
                         Vector3(refract_center.x+1, refract_center.y, refract_center.z),
                         Vector3(camera_.up.x, -camera_.up.y, camera_.up.z), w, h);

        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, refract_cube_map, 0);
        render_to_immediate_buffer(Vector3(refract_center.x,refract_center.y,refract_center.z),
                         Vector3(refract_center.x-1, refract_center.y, refract_center.z),
                         Vector3(camera_.up.x, -camera_.up.y, camera_.up.z), w, h);

        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, refract_cube_map, 0);
        render_to_immediate_buffer(Vector3(refract_center.x,refract_center.y,refract_center.z),
                         Vector3(refract_center.x, refract_center.y+1, refract_center.z),
                         Vector3(camera_.up.x, camera_.up.y, camera_.up.z), w, h);

        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, refract_cube_map, 0);
        render_to_immediate_buffer(Vector3(refract_center.x,refract_center.y,refract_center.z),
                         Vector3(refract_center.x, refract_center.y-1, refract_center.z),
                         Vector3(camera_.up.x, camera_.up.y, camera_.up.z), w, h);

        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, refract_cube_map, 0);
        render_to_immediate_buffer(Vector3(refract_center.x,refract_center.y,refract_center.z),
                         Vector3(refract_center.x, refract_center.y, refract_center.z+1),
                         Vector3(camera_.up.x, -camera_.up.y, camera_.up.z), w, h);

        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, refract_cube_map, 0);
        render_to_immediate_buffer(Vector3(refract_center.x,refract_center.y,refract_center.z),
                         Vector3(refract_center.x, refract_center.y, refract_center.z-1),
                         Vector3(camera_.up.x, -camera_.up.y, camera_.up.z), w, h);
    }
    refract_every_so_often++;

    // and render the actual scene
    render_scene(framebuffer_objects_["fbo_0"], Vector3(camera_.center.x, camera_.center.y, camera_.center.z), Vector3(camera_.eye.x, camera_.eye.y, camera_.eye.z), Vector3(camera_.up.x, camera_.up.y, camera_.up.z), w, h);


    //copy the rendered scene into framebuffer 1
    framebuffer_objects_["fbo_0"]->blitFramebuffer(framebuffer_objects_["fbo_1"],
                                                   QRect(0,0,w,h),framebuffer_objects_["fbo_0"],
                                                   QRect(0,0,w,h),GL_COLOR_BUFFER_BIT,GL_NEAREST);

    orthogonal_camera(w,h);
    glBindTexture(GL_TEXTURE_2D, framebuffer_objects_["fbo_1"]->texture());
    textured_quad(w, h, true);
    glBindTexture(GL_TEXTURE_2D, 0);


    framebuffer_objects_["fbo_2"]->bind();  // bind framebuffer two
    shader_programs_["brightpass"]->bind(); // bind brightpass shader
    glBindTexture(GL_TEXTURE_2D, framebuffer_objects_["fbo_1"]->texture()); // bind framebuffer one's texture
    textured_quad(w, h, true);  // draw quad
    shader_programs_["brightpass"]->release();  // unbind shader
    glBindTexture(GL_TEXTURE_2D, 0);    // unbind texture
    framebuffer_objects_["fbo_2"]->release();   // unbind framebuffer


    //Uncomment this section in step 2 of the lab...
    float scales[] = {4.f,8.f,16.f,32.f};
    for(int i = 0; i < 4; ++i) {
        render_blur(w /scales[i],h /scales[i]);
        glBindTexture(GL_TEXTURE_2D,framebuffer_objects_["fbo_1"]->texture());
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE,GL_ONE);
        glTranslatef(0.f,(scales[i] - 1)* -h,0.f);
        textured_quad(w * scales[i],h * scales[i],false);
        glDisable(GL_BLEND);
        glBindTexture(GL_TEXTURE_2D,0);
     }
}

/**
  @paragraph Should run a gaussian blur on the texture stored in
  fbo 2 and put the result in fbo 1.  The blur should have a radius of 2.

  @todo Finish filling this in.

  @param time: the current program time in milliseconds
  @param w:    the viewport width
  @param h:    the viewport height

**/
void DrawEngine::render_blur(float w,float h) {
    int radius = 2,dim = radius * 2 + 1;
    GLfloat kernel[dim * dim],offsets[dim * dim * 2];
    create_blur_kernel(radius,w,h,&kernel[0],&offsets[0]);
    // you may want to add code here
    // run a blur on framebuffer two and store results in framebuffer one
    //glBindTexture(GL_TEXTURE_2D, framebuffer_objects_["fbo_2"]->texture());
    //textured_quad(w, h, true);
    //glBindTexture(GL_TEXTURE_2D, 0);
    framebuffer_objects_["fbo_1"]->bind();  // bind framebuffer one
    shader_programs_["blur"]->bind(); // bind blur shader
    shader_programs_["blur"]->setUniformValueArray("offsets", offsets, dim*dim*2, 2);
    shader_programs_["blur"]->setUniformValueArray("kernel", kernel, dim*dim, 1);

    /*glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glScalef(1.0, -1.0, 1.0);
    glTranslatef(0, 1.0, 0);
    glMatrixMode(GL_MODELVIEW);*/

    glBindTexture(GL_TEXTURE_2D, framebuffer_objects_["fbo_2"]->texture()); // bind framebuffer two's texture


    textured_quad(w, h, true);  // draw quad
    shader_programs_["blur"]->release();  // unbind shader
    glBindTexture(GL_TEXTURE_2D, 0);    // unbind texture
    framebuffer_objects_["fbo_1"]->release();   // unbind framebuffer
}

void DrawEngine::render_to_immediate_buffer(Vector3 eye, Vector3 pos, Vector3 up, int w, int h) {
    float ratio = w / static_cast<float>(h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(camera_.fovy,ratio,camera_.near,camera_.far);
    glViewport(0,0,2048,2048);
    gluLookAt(eye.x, eye.y, eye.z + .000000001,
              pos.x, pos.y, pos.z,
              up.x, up.y, up.z);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    GLUquadric* quad = gluNewQuadric();

    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE_CUBE_MAP);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textures_["cube_map_1"]);
    glCallList(models_["skybox"].idx);
    glEnable(GL_CULL_FACE);
    glActiveTexture(GL_TEXTURE0);

    // sphere to draw (without shader)
    glPushMatrix();
    gluSphere(quad, 1, 20, 20);
    glPopMatrix();


    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glBindTexture(GL_TEXTURE_CUBE_MAP,0);
    glDisable(GL_TEXTURE_CUBE_MAP);

    gluDeleteQuadric(quad);
}

/*void DrawEngine::render_to_buffer(QGLFramebufferObject* fb, Vector3 eye, Vector3 pos, Vector3 up, int w, int h) {

    fb->bind();
    float ratio = w / static_cast<float>(h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(camera_.fovy,ratio,camera_.near,camera_.far);
    gluLookAt(eye.x, eye.y, eye.z + .000000001,
              pos.x, pos.y, pos.z,
              up.x, up.y, up.z);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    GLUquadric* quad = gluNewQuadric();

    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE_CUBE_MAP);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textures_["cube_map_1"]);
    glCallList(models_["skybox"].idx);
    glEnable(GL_CULL_FACE);
    glActiveTexture(GL_TEXTURE0);

    // sphere to draw (without shader)
    glPushMatrix();
    gluSphere(quad, 1, 20, 20);
    glPopMatrix();

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glBindTexture(GL_TEXTURE_CUBE_MAP,0);
    glDisable(GL_TEXTURE_CUBE_MAP);

    gluDeleteQuadric(quad);

    fb->release();
}*/

void DrawEngine::render_scene(QGLFramebufferObject* fb, Vector3 look, Vector3 pos, Vector3 up, int w, int h) {

    fb->bind();
    float ratio = w / static_cast<float>(h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(camera_.fovy,ratio,camera_.near,camera_.far);
    glViewport(0,0,w,h);
    gluLookAt(camera_.eye.x, camera_.eye.y, camera_.eye.z,
              camera_.center.x, camera_.center.y, camera_.center.z,
              camera_.up.x, camera_.up.y, camera_.up.z);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    GLUquadric* quad = gluNewQuadric();

    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE_CUBE_MAP);


    textures_["cube_map_2"] = refract_cube_map;
    glBindTexture(GL_TEXTURE_CUBE_MAP, textures_["cube_map_1"]);
    glCallList(models_["skybox"].idx);

    glBindTexture(GL_TEXTURE_CUBE_MAP, refract_cube_map);
    glEnable(GL_CULL_FACE);
    glActiveTexture(GL_TEXTURE0);

    // refracted sphere...
    shader_programs_["refract"]->bind();
    shader_programs_["refract"]->setUniformValue("CubeMap",GL_TEXTURE0);
    glPushMatrix();
    //glTranslatef(-1.25f,0.f,0.f);
    //glCallList(models_["dragon"].idx);
    glTranslatef(refract_center.x, refract_center.y, refract_center.z);
    gluSphere(quad, 1, 20, 20);
    glTranslatef(-refract_center.x, -refract_center.y, -refract_center.z);
    //drawKleinBottle();
    glPopMatrix();
    shader_programs_["refract"]->release();

    glBindTexture(GL_TEXTURE_CUBE_MAP, textures_["cube_map_1"]);

    // sphere to draw (without shader)
    glPushMatrix();
    gluSphere(quad, 1, 20, 20);
    glPopMatrix();

    // reflect sphere...
    /*shader_programs_["reflect"]->bind();
    shader_programs_["reflect"]->setUniformValue("CubeMap",GL_TEXTURE0);
    glPushMatrix();
    glTranslatef(1.25f,0.f,0.f);

    //glCallList(models_["dragon"].idx);
    glPopMatrix();
    shader_programs_["reflect"]->release();*/


    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glBindTexture(GL_TEXTURE_CUBE_MAP,0);
    glDisable(GL_TEXTURE_CUBE_MAP);

    gluDeleteQuadric(quad);

    fb->release();
}

/**
  @paragraph Renders the actual scene.  May be called multiple times by
  DrawEngine::draw_frame(float time,int w,int h) if necessary.

  @param w: the viewport width
  @param h: the viewport height

**/

/**
  @paragraph Draws a textured quad. The texture most be bound and unbound
  before and after calling this method - this method assumes that the texture
  has been bound before hand.

  @param w: the width of the quad to draw
  @param h: the height of the quad to draw
  @param flip: flip the texture vertically

**/
void DrawEngine::textured_quad(int w,int h,bool flip) {
    glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f,flip ? 1.0f : 0.0f);
    glVertex2f(0.0f,0.0f);
    glTexCoord2f(1.0f,flip ? 1.0f : 0.0f);
    glVertex2f(w,0.0f);
    glTexCoord2f(1.0f,flip ? 0.0f : 1.0f);
    glVertex2f(w,h);
    glTexCoord2f(0.0f,flip ? 0.0f : 1.0f);
    glVertex2f(0.0f,h);
    glEnd();
}

void DrawEngine::drawKleinBottle(){
    float m_param1 = 50;
    float m_param2 = 50;
    float diffi = (360 / (float) m_param1)*(M_PI / 180.0);
    float diffj = (360 / (float) m_param2)*(M_PI / 180.0);
    glBegin(GL_QUADS);
    for(int i=0; i<m_param1; i++){
        for(int j=0; j<m_param2; j++){

            float u = i*diffi;
            float v = j*diffj;
            float cosu = cos(u);
            float cosv = cos(v);
            float sinu = sin(u);
            float sinv = sin(v);
            float r = 4*(1-cosu/2.0);

            float x1, y1;
            if(u <= M_PI){
                x1 = 6*cosu*(1+sinu) + r*cosu*cosv;
                y1 = 16*sinu + r*sinu*cosv;
            }
            else{
                x1 = 6*cosu*(1+sinu) + r*cos(v + M_PI);
                y1 = 16*sinu;
            }
            float z1 = r*sinv;
            glVertex3f(x1, y1, z1);

            u = (i+1)*diffi;
            v = j*diffj;
            cosu = cos(u);
            cosv = cos(v);
            sinu = sin(u);
            sinv = sin(v);
            r = 4*(1-cosu/2.0);
            float x2, y2;
            if(u <= M_PI){
                x2 = 6*cosu*(1+sinu) + r*cosu*cosv;
                y2 = 16*sinu + r*sinu*cosv;
            }
            else{
                x2 = 6*cosu*(1+sinu) + r*cos(v + M_PI);
                y2 = 16*sinu;
            }
            float z2 = r*sinv;

            glVertex3f(x2, y2, z2);


            u = (i+1)*diffi;
            v = (j+1)*diffj;
            cosu = cos(u);
            cosv = cos(v);
            sinu = sin(u);
            sinv = sin(v);
            r = 4*(1-cosu/2.0);
            float x3, y3;
            if(u <= M_PI){
                x3 = 6*cosu*(1+sinu) + r*cosu*cosv;
                y3 = 16*sinu + r*sinu*cosv;
            }
            else{
                x3 = 6*cosu*(1+sinu) + r*cos(v + M_PI);
                y3 = 16*sinu;
            }
            float z3 = r*sinv;


            glVertex3f(x3, y3, z3);

            u = i*diffi;
            v = (j+1)*diffj;
            cosu = cos(u);
            cosv = cos(v);
            sinu = sin(u);
            sinv = sin(v);
            r = 4*(1-cosu/2.0);
            float x4, y4;
            if(u <= M_PI){
                x4 = 6*cosu*(1+sinu) + r*cosu*cosv;
                y4 = 16*sinu + r*sinu*cosv;
            }
            else{
                x4 = 6*cosu*(1+sinu) + r*cos(v + M_PI);
                y4 = 16*sinu;
            }
            float z4 = r*sinv;
            glVertex3f(x4, y4, z4);
            glVertex3f(x4, y4, z4);
            glVertex3f(x3, y3, z3);
            glVertex3f(x2, y2, z2);
            glVertex3f(x1, y1, z1);


        }

    }
    glEnd();
}

/**
  @paragraph Called to switch to the perspective OpenGL camera.
  Used to render the scene regularly with the current camera parameters.

  @param w: the viewport width
  @param h: the viewport height

**/
void DrawEngine::perspective_camera(int w,int h) {
    float ratio = w / static_cast<float>(h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(camera_.fovy,ratio,camera_.near,camera_.far);
    gluLookAt(camera_.eye.x,camera_.eye.y,camera_.eye.z,
              camera_.center.x,camera_.center.y,camera_.center.z,
              camera_.up.x,camera_.up.y,camera_.up.z);

    //gluLookAt(0,0,-1,refract_center.x,refract_center.y,refract_center.z,camera_.up.x,camera_.up.y,camera_.up.z);
    // center = object's center
    // eye = all 6 directions
    // up = same up

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

/**
  @paragraph Called to switch to an orthogonal OpenGL camera.
  Useful for rending a textured quad across the whole screen.

  @param w: the viewport width
  @param h: the viewport height

**/
void DrawEngine::orthogonal_camera(int w,int h) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0,static_cast<float>(w),static_cast<float>(h),0.f,-1.f,1.f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

/**
  @paragraph Called when the viewport has been resized. Needs to
  resize the camera perspective and reallocate the framebuffer
  sizes.

  @param w: the viewport width
  @param h: the viewport height

**/
void DrawEngine::resize_frame(int w,int h) {
    glViewport(0,0,w,h);
    realloc_framebuffers(w,h);
}

/**
  @paragraph Called by GLWidget when the mouse is dragged.  Rotates the camera
  based on mouse movement.

  @param p0: the old mouse position
  @param p1: the new mouse position
**/
void DrawEngine::mouse_drag_event(float2 p0,float2 p1) {
    int dx = p1.x - p0.x,dy = p1.y - p0.y;
    QQuaternion qq = QQuaternion::fromAxisAndAngle(0, 1, 0, -dx / 5.0);
    QVector3D qv3 = qq.rotatedVector(QVector3D(camera_.eye.x, camera_.eye.y,
                                               camera_.eye.z));
    qq = QQuaternion::fromAxisAndAngle(qq.rotatedVector(QVector3D(1, 0, 0)), dy / 5.0);
    qv3 = qq.rotatedVector(qv3);
    camera_.eye.x = qv3.x(), camera_.eye.y = qv3.y(), camera_.eye.z = qv3.z();
}

/**
  @paragraph Called by GLWidget when the mouse wheel is turned. Zooms the camera in
  and out.

  @param dx: The delta value of the mouse wheel movement.
**/
void DrawEngine::mouse_wheel_event(int dx) {
    if((camera_.center - camera_.eye).getMagnitude() > .5 || dx < 0)
        camera_.eye += (camera_.center - camera_.eye).getNormalized() * dx * .005;
}

/**
  @paragraph Loads the cube map into video memory.

  @param files: a list of files containing the cube map images (should be length
  six) in order.
  @return The assigned OpenGL id to the cube map.
**/
GLuint DrawEngine::load_cube_map(QList<QFile *> files) {
    GLuint id;
    glGenTextures(1,&id);
    glBindTexture(GL_TEXTURE_CUBE_MAP,id);
    for(unsigned i = 0; i < 6; ++i) {
        QImage image,texture;
        image.load(files[i]->fileName());
        image = image.mirrored(false,true);
        texture = QGLWidget::convertToGLFormat(image);
        texture = texture.scaledToWidth(1024,Qt::SmoothTransformation);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,3,3,texture.width(),texture.height(),0,GL_RGBA,GL_UNSIGNED_BYTE,texture.bits());
        gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_X +i, 3, texture.width(), texture.height(), GL_RGBA, GL_UNSIGNED_BYTE, texture.bits());
        cout << files[i]->fileName().toStdString() << endl;
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_MIN_FILTER,GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_MAG_FILTER,GL_NEAREST_MIPMAP_NEAREST);
    glBindTexture(GL_TEXTURE_CUBE_MAP,0);
    return id;
}

/**
  @paragraph Creates a gaussian blur kernel with the specified radius.  The kernel values
  and offsets are stored.

  @param radius: The radius of the kernel to create.
  @param w: The width of the image.
  @param h: The height of the image.
  @param kernel: The array to write the kernel values to.
  @param offsets: The array to write the offset values to.
**/
void DrawEngine::create_blur_kernel(int radius,int w,int h,GLfloat* kernel,GLfloat* offsets) {
    int size = radius * 2 + 1;
    float sigma = radius / 3.0f,twoSigmaSigma = 2.0f * sigma * sigma,
        rootSigma = sqrt(twoSigmaSigma * M_PI),total = 0.0f;
    float xOff = 1.0f / w,yOff = 1.0f / h;
    int offsetIndex = 0;
    for(int y = -radius,idx = 0; y <= radius; ++y) {
        for(int x = -radius; x <= radius; ++x,++idx) {
            float d = x * x + y * y;
            kernel[idx] = exp(-d / twoSigmaSigma) / rootSigma;
            total += kernel[idx];
            offsets[offsetIndex++] = x * xOff;
            offsets[offsetIndex++] = y * yOff;
        }
    }
    for(int i = 0; i < size * size; ++i) kernel[i] /= total;
}

/**
  @paragraph Called when a key has been pressed in the GLWidget.

  @param event: The key press event associated with the current key press.
  **/
void DrawEngine::key_press_event(QKeyEvent *event) {
    switch(event->key()) {

    }
}
