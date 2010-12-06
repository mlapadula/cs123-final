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

using std::cout;
using std::endl;

extern "C"{
    extern void APIENTRY glActiveTexture (GLenum);
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
    //Render the scene to a framebuffer
    framebuffer_objects_["fbo_0"]->bind();
    perspective_camera(w,h);
    render_scene(time,w,h);
    framebuffer_objects_["fbo_0"]->release();
    //copy the rendered scene into framebuffer 1
    framebuffer_objects_["fbo_0"]->blitFramebuffer(framebuffer_objects_["fbo_1"],
                                                   QRect(0,0,w,h),framebuffer_objects_["fbo_0"],
                                                   QRect(0,0,w,h),GL_COLOR_BUFFER_BIT,GL_NEAREST);

    //you may want to add code here
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

/**
  @paragraph Renders the actual scene.  May be called multiple times by
  DrawEngine::draw_frame(float time,int w,int h) if necessary.

  @param w: the viewport width
  @param h: the viewport height

**/
void DrawEngine::render_scene(float time,int w,int h) {
    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE_CUBE_MAP);
    glBindTexture(GL_TEXTURE_CUBE_MAP,textures_["cube_map_1"]);
    glCallList(models_["skybox"].idx);
    glEnable(GL_CULL_FACE);
    glActiveTexture(GL_TEXTURE0);
    shader_programs_["refract"]->bind();
    shader_programs_["refract"]->setUniformValue("CubeMap",GL_TEXTURE0);
    glPushMatrix();
    glTranslatef(-1.25f,0.f,0.f);
    glCallList(models_["dragon"].idx);
    glPopMatrix();
    shader_programs_["refract"]->release();
    shader_programs_["reflect"]->bind();
    shader_programs_["reflect"]->setUniformValue("CubeMap",GL_TEXTURE0);
    glPushMatrix();
    glTranslatef(1.25f,0.f,0.f);
    glCallList(models_["dragon"].idx);
    glPopMatrix();
    shader_programs_["reflect"]->release();
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glBindTexture(GL_TEXTURE_CUBE_MAP,0);
    glDisable(GL_TEXTURE_CUBE_MAP);




    // BEZIER:
    // Control points (substitute these values with your own if you like)
    double Ax = -2.0; double Ay = -1.0; double Az = 1.0;
    double Bx = -1.0; double By = 3.0; double Bz = 1.0;
    double Cx = 1.0; double Cy = -3.0; double Cz = -1.0;
    double Dx = 2.0; double Dy = 1.0; double Dz = -1.0;

    // Points on the curve
    double X;
    double Y;
    double Z;

    // Variable
    double a = 1.0;
    double b = 1.0 - a;

    // Tell OGL to start drawing a line strip
    glBegin(GL_LINE_STRIP);

    /* We will not actually draw a curve, but we will divide the curve into small
    points and draw a line between each point. If the points are close enough, it
    will appear as a curved line. 20 points are plenty, and since the variable goes
    from 1.0 to 0.0 we must change it by 1/20 = 0.05 each time */

    for(int i = 0; i <= 20; i++)
    {
      // Get a point on the curve
      X = Ax*a*a*a + Bx*3*a*a*b + Cx*3*a*b*b + Dx*b*b*b;
      Y = Ay*a*a*a + By*3*a*a*b + Cy*3*a*b*b + Dy*b*b*b;
      Z = Az*a*a*a + Bz*3*a*a*b + Cz*3*a*b*b + Dz*b*b*b;

      // Draw the line from point to point (assuming OGL is set up properly)
      glVertex3d(X, Y, Z);

      // Change the variable
      a -= 0.05;
      b = 1.0 - a;
    }

    // Tell OGL to stop drawing the line strip
    glEnd();

    /* Normally you will want to save the coordinates to an array for later use. And
    you will probably not need to calculate the curve each frame. This code
    demonstrates an easily understandable way to do it, not necessarily the most
    useful way. */



}

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
