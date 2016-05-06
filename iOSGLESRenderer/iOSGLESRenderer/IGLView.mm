/* Neko Engine
 *
 * IGLView.mm
 * Author: Alexandru Naiman
 *
 * iOS OpenGL|ES Renderer Implementation
 *
 * ----------------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2016, Alexandru Naiman <alexandru dot naiman at icloud dot com>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list
 * of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or other
 * materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be
 * used to endorse or promote products derived from this software without specific prior
 * written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL ALEXANDRU NAIMAN
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#import "IGLView.h"

#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>

@interface IGLView ()
{
	EAGLContext* _context;
	NSInteger _animationFrameInterval;
	CADisplayLink* _displayLink;
	GLuint _defaultFbo, _colorBuffer, _depthBuffer;
	GLint _width, _height;
}
@end

@implementation IGLView

+ (Class) layerClass
{
	return [CAEAGLLayer class];
}

- (instancetype)initWithFrame:(CGRect)frame
{
	if((self = [super initWithFrame:frame]) == nil)
		return nil;
	
	CAEAGLLayer *layer = (CAEAGLLayer*)self.layer;
	layer.opaque = true;
	layer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithBool:false], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
	
	_context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
	
	if(!_context || ![EAGLContext setCurrentContext:_context])
		return nil;
	
	glClearColor(1.f, 0.f, .0f, 1.f);
	
	if(![self createBuffers])
		return nil;
	
	_inputDelegate = nil;
	
	return self;
}

- (bool) createBuffers
{
	glGenFramebuffers(1, &_defaultFbo);
	glGenRenderbuffers(1, &_colorBuffer);
	
	glBindFramebuffer(GL_FRAMEBUFFER, _defaultFbo);
	glBindRenderbuffer(GL_RENDERBUFFER, _colorBuffer);
	
	[_context renderbufferStorage:GL_RENDERBUFFER fromDrawable:(id<EAGLDrawable>)self.layer];
	
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _colorBuffer);
	
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &_width);
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &_height);
	
	glGenRenderbuffers(1, &_depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, _depthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, _width, _height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthBuffer);
	
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		NSLog(@"failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
		return false;
	}
	
	NSLog(@"%s %s renderer initialized.", glGetString(GL_RENDERER), glGetString(GL_VERSION));
	
	return true;
}

- (void)drawView:(id)sender
{
	[EAGLContext setCurrentContext:_context];
}

- (void)bindDrawable
{
	glBindFramebuffer(GL_FRAMEBUFFER, _defaultFbo);
}

- (void)swapBuffers
{
	glBindRenderbuffer(GL_RENDERBUFFER, _colorBuffer);
	[_context presentRenderbuffer:GL_RENDERBUFFER];
}

- (void)dealloc
{
	glDeleteFramebuffers(1, &_defaultFbo);
	glDeleteRenderbuffers(1, &_colorBuffer);
	glDeleteRenderbuffers(1, &_depthBuffer);
}

#pragma mark EngineViewProtocol

- (void)setInputDelegate:(id<EngineInputDelegateProtocol>)delegate
{
	_inputDelegate = delegate;
}

#pragma mark Events

- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
	if(_inputDelegate)
		[_inputDelegate touchesBegan:touches withEvent:event];
}

- (void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
	if(_inputDelegate)
		[_inputDelegate touchesMoved:touches withEvent:event];
}

- (void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
	if(_inputDelegate)
		[_inputDelegate touchesEnded:touches withEvent:event];
}

- (void)touchesCancelled:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event
{
	if(_inputDelegate)
		[_inputDelegate touchesCancelled:touches withEvent:event];
}

@end
