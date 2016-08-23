/* NekoEngine
 *
 * EngineWidget.cpp
 * Author: Alexandru Naiman
 *
 * NekoEngine Widget
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2016, Alexandru Naiman
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <QWindow>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QHBoxLayout>

#include <QDebug>

#include <Engine/Engine.h>
#include <Engine/Input.h>
#include <Engine/SceneManager.h>

#include "EngineWidget.h"

EngineWidget::EngineWidget(QWidget *parent) : QWidget(parent)
{
	_timer = new QTimer(this);
	connect(_timer, SIGNAL(timeout()), this, SLOT(update()));

	setMouseTracking(true);
	setFocusPolicy(Qt::StrongFocus);

	setAttribute(Qt::WA_NativeWindow, true);
	setAttribute(Qt::WA_PaintOnScreen, true);
	setAttribute(Qt::WA_NoSystemBackground, true);
	setAutoFillBackground(false);

	Engine::Initialize("--ini=E:/Projects/NekoEngine/Resources/Engine.ini --data=E:/Projects/NekoEngine/Resources/Data --gfxdbg", true);

	QWindow *nativeWindow = QWindow::fromWinId((WId)Platform::GetActiveWindow());
	QWidget *nativeWidget = QWidget::createWindowContainer(nativeWindow);

	QHBoxLayout *l = new QHBoxLayout(this);
	l->setContentsMargins(0, 0, 0, 0);
	l->addWidget(nativeWidget);

	_engineInitialized = true;
	_timer->start(1);
}

void EngineWidget::update()
{
	if(!SceneManager::IsSceneLoaded())
		SceneManager::LoadDefaultScene();

	Engine::Frame();
}

void EngineWidget::resizeEvent(QResizeEvent *event)
{
	QSize size = event->size();
	Engine::ScreenResized(size.rwidth(), size.rheight());
}

void EngineWidget::keyPressEvent(QKeyEvent *event)
{
	Input::Key(event->nativeVirtualKey(), true);
}

void EngineWidget::keyReleaseEvent(QKeyEvent *event)
{
	Input::Key(event->nativeVirtualKey(), false);
	qDebug() << "key release";
}

void EngineWidget::mouseMoveEvent(QMouseEvent *event)
{
	qDebug() << "mouse";
}

void EngineWidget::mousePressEvent(QMouseEvent *event)
{
	qDebug() << "mouse press";
}

void EngineWidget::mouseReleaseEvent(QMouseEvent *event)
{
	qDebug() << "mouse release";
}

EngineWidget::~EngineWidget()
{
	Engine::CleanUp();
}
