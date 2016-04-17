#ifndef EDITORGLWIDGET_H
#define EDITORGLWIDGET_H

#include <QTimer>
#include <QOpenGLWidget>

class EditorGLWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    explicit EditorGLWidget(QWidget *parent = 0);

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);

signals:

public slots:

private:
    bool _engineInitialized;
    QTimer *_timer;
};

#endif // EDITORGLWIDGET_H
