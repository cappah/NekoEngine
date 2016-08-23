//
//  iOS.cpp
//  GLESRenderer
//
//  Created by Alexandru Naiman on 10/08/16.
//  Copyright © 2016 Alexandru Naiman. All rights reserved.
//

#import "IGLView.h"
#import <GLKit/GLKit.h>
#include "GLESRenderer.h"

static IGLView *_view;

using namespace std;

bool GLESRenderer::Initialize(PlatformWindowType hWnd, unordered_map<string, string> *args, bool debug)
{
	_window = hWnd;
	
	UIViewController *viewController = [[UIViewController alloc] init];
	_view = [[IGLView alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
	[viewController setView:_view];
	[hWnd setRootViewController:viewController];
	
	[_view bindDrawable];
	
	return true;
}

void GLESRenderer::SetSwapInterval(int swapInterval)
{
	//
}

void GLESRenderer::ScreenResized()
{
	[_view updateDrawable];
}

void GLESRenderer::SwapBuffers()
{
	[_view swapBuffers];
}

void GLESRenderer::_DestroyContext()
{
	//
}

void GLESRenderer::BindDefaultFramebuffer()
{
	[_view bindDrawable];
	_boundFramebuffer = nullptr;
}

void GLESRenderer::MakeCurrent(int context)
{
/*	if (context == R_RENDER_CONTEXT)
		wglMakeCurrent(_dc, _ctx);
	else
		wglMakeCurrent(_dc, _loadCtx);*/
}

void GLESRenderer::MakeCurrent()
{
	//[EAGLContext setCurrentContext:_ctx];
}
