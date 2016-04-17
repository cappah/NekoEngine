#include <Engine.h>
#include <SceneManager.h>
#include <QtOpenGL>

#include "editorglwidget.h"

EditorGLWidget::EditorGLWidget(QWidget *parent) : QOpenGLWidget(parent)
{
    _timer = new QTimer(this);
    connect(_timer, SIGNAL(timeout()), this, SLOT(update()));

    _engineInitialized = false;
}

void EditorGLWidget::initializeGL()
{
    glClearColor(.5f, 0.f, .7f, 1.f);

    glewInit();

    Engine::Initialize("--ini=D:/Projects/farrah/Resources/Engine.ini --data=D:/Projects/farrah/Resources/Data --gfxdbg", true);
    SceneManager::LoadDefaultScene();

    _engineInitialized = true;
    _timer->start(1);
}

void EditorGLWidget::paintGL()
{
    if(_engineInitialized)
        Engine::Frame();

    context()->swapBuffers(context()->surface());
}

void EditorGLWidget::resizeGL(int width, int height)
{
    Engine::ScreenResized(width, height);
}
