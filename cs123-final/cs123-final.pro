#-------------------------------------------------
#
# Project created by QtCreator 2010-11-05T14:00:55
#
#-------------------------------------------------

QT       += core gui opengl

TARGET = cs123-final
TEMPLATE = app
CONFIG += console

SOURCES += main.cpp\
        mainwindow.cpp \
    glwidget.cpp \
    drawengine.cpp \
    targa.cpp \
    glm.cpp \
    CS123Vector.inl \
    CS123Matrix.inl \
    CS123Matrix.cpp

HEADERS  += mainwindow.h \
    glwidget.h \
    drawengine.h \
    targa.h \
    glm.h \
    common.h \
    CS123Vector.h \
    CS123Matrix.h \
    CS123Algebra.h \
    CS123Common.h

FORMS    += mainwindow.ui

RESOURCES +=
