/* Neko Engine
 *
 * MessageBoxX11.cpp
 * Author: Alexandru Naiman
 *
 * Message box function for X11. Based on David Oberhollenzer's X11MessageBox
 *
 * Changes:
 * 	- Multiple message box types
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

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include "MessageBoxX11.h"

#include <string.h>
#include <stdlib.h>

typedef struct
{
	int x, y;
	unsigned int width, height;
	int textx, texty;
	int mouseover;
	int clicked;
	const char* text;
}
button;

static void draw_button(button* b, int fg, int bg,
                        Display* dpy, Window w, GC gc)
{
	if(b->mouseover)
	{
		XFillRectangle(dpy, w, gc, b->clicked+b->x, b->clicked+b->y, b->width, b->height);
		XSetForeground(dpy, gc, bg);
		XSetBackground(dpy, gc, fg);
	}
	else
	{
		XSetForeground(dpy, gc, fg);
		XSetBackground(dpy, gc, bg);
		XDrawRectangle(dpy, w, gc, b->x, b->y, b->width, b->height);
	}

	XDrawString(dpy, w, gc, b->clicked+b->textx, b->clicked+b->texty,
			b->text, strlen(b->text));
	XSetForeground(dpy, gc, fg);
	XSetBackground(dpy, gc, bg);
}

static int is_point_inside( button* b, int px, int py )
{
	return px>=b->x && px<=(b->x+(int)b->width-1) &&
		py>=b->y && py<=(b->y+(int)b->height-1);
}

/**************************************************************************
 * A "small" and "simple" function that creates a message box with an OK  *
 * button, using ONLY Xlib.                                               *
 * The function does not return until the user closes the message box,    *
 * using the OK button, the escape key, or the close button what means    *
 * that you can't do anything in the mean time(in the same thread).       *
 * The code may look very ugly, because I pieced it together from         *
 * tutorials and manuals and I use an awfull lot of magic values and      *
 * unexplained calculations.                                              *
 *                                                                        *
 * title: The title of the message box.                                   *
 * text:  The contents of the message box. Use '\n' as a line terminator. *
 **************************************************************************/
int MessageBoxX11(const char* title, const char* text, unsigned int flags)
{
	const char* wmDeleteWindow = "WM_DELETE_WINDOW";
	int black, white, height = 0, direction, ascent, descent, X, Y, W = 0, H;
	size_t i, lines = 0;
	char *atom;
	const char *end, *temp;
	button primaryBtn, secondaryBtn;
	Display* dpy;
	Window w;
	Atom wmDelete;
	GC gc;
	XFontStruct* font;
	XCharStruct overall;
	XSizeHints hints;
	XEvent e;
	int ret = X11_MSG_BOX_RET_CANCEL;
	
	memset(&primaryBtn, 0x0, sizeof(button));
	memset(&secondaryBtn, 0x0, sizeof(button));

	/* Open a display */
	if(!(dpy = XOpenDisplay(0)))
		return X11_MSG_BOX_ERROR;

	/* Get us a white and black color */
	black = BlackPixel(dpy, DefaultScreen(dpy));
	white = WhitePixel(dpy, DefaultScreen(dpy));

	/* Create a window with the specified title */
	w = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 0, 0, 100, 100,
					0, black, black);

	XSelectInput(dpy, w, ExposureMask | StructureNotifyMask |
				KeyReleaseMask | PointerMotionMask |
				ButtonPressMask | ButtonReleaseMask);

	XMapWindow(dpy, w);
	XStoreName(dpy, w, title);

	wmDelete = XInternAtom(dpy, wmDeleteWindow, True);
	XSetWMProtocols(dpy, w, &wmDelete, 1);

	/* Create a graphics context for the window */
	gc = XCreateGC(dpy, w, 0, 0);

	XSetForeground(dpy, gc, white);
	XSetBackground(dpy, gc, black);

	/* Compute the printed width and height of the text */
	if(!(font = XQueryFont(dpy, XGContextFromGC(gc))))
		goto cleanup;

	for(temp = text; temp; temp = end ? (end+1) : NULL, ++lines)
	{
		end = strchr(temp, '\n');

		XTextExtents(font, temp, end ? (unsigned int)(end-temp):strlen(temp),
				&direction, &ascent, &descent, &overall );

		W = overall.width>W ? overall.width : W;
		height = (ascent+descent)>height ? (ascent+descent) : height;
	}

	/* Compute the shape of the window and adjust the window accordingly */
	W += 30;
	H = lines * height + height + 40;
	X = DisplayWidth (dpy, DefaultScreen(dpy))/2 - W/2;
	Y = DisplayHeight(dpy, DefaultScreen(dpy))/2 - H/2;

	XMoveResizeWindow(dpy, w, X, Y, W, H);

	if((flags & X11_MSG_BOX_BTN_OK) == X11_MSG_BOX_BTN_OK)
	{
		/* Compute the shape of the OK button */
		XTextExtents(font, "OK", 2, &direction, &ascent, &descent, &overall);

		primaryBtn.width = overall.width + 30;
		primaryBtn.height = ascent + descent + 5;
		primaryBtn.x = W/2 - primaryBtn.width/2;
		primaryBtn.y = H   - height - 15;
		primaryBtn.textx = primaryBtn.x + 15;
		primaryBtn.texty = primaryBtn.y + primaryBtn.height - 3;
		primaryBtn.mouseover = 0;
		primaryBtn.clicked = 0;
		primaryBtn.text = "OK";

		secondaryBtn.clicked = 0;
	}
	else if((flags & X11_MSG_BOX_BTN_YESNO) == X11_MSG_BOX_BTN_YESNO)
	{
		XTextExtents(font, "YES", 3, &direction, &ascent, &descent, &overall);

		primaryBtn.width = overall.width + 30;
		primaryBtn.height = ascent + descent + 5;
		primaryBtn.y = H - height - 15;
		primaryBtn.texty = primaryBtn.y + primaryBtn.height - 3;
		primaryBtn.mouseover = 0;
		primaryBtn.clicked = 0;
		primaryBtn.text = "YES";

		XTextExtents(font, "NO", 2, &direction, &ascent, &descent, &overall);

		secondaryBtn.width = overall.width + 30;
		secondaryBtn.height = ascent + descent + 5;
		secondaryBtn.y = H - height - 15;
		secondaryBtn.texty = secondaryBtn.y + secondaryBtn.height - 3;
		secondaryBtn.mouseover = 0;
		secondaryBtn.clicked = 0;
		secondaryBtn.text = "NO";

		primaryBtn.x = W / 2 - ((primaryBtn.width + secondaryBtn.width + 10) / 2);
		primaryBtn.textx = primaryBtn.x + 15;
		secondaryBtn.x = primaryBtn.x + primaryBtn.width + 10;
		secondaryBtn.textx = secondaryBtn.x + 15;
	}

	XFreeFontInfo(NULL, font, 1); /* We don't need that anymore */

	/* Make the window non resizeable */
	XUnmapWindow(dpy, w);

	hints.flags      = PSize | PMinSize | PMaxSize;
	hints.min_width  = hints.max_width  = hints.base_width  = W;
	hints.min_height = hints.max_height = hints.base_height = H;

	XSetWMNormalHints(dpy, w, &hints);
	XMapRaised(dpy, w);
	XFlush(dpy);

	/* Event loop */
	while(1)
	{
		XNextEvent(dpy, &e);
		primaryBtn.clicked = 0;
		secondaryBtn.clicked = 0;

		if(e.type == MotionNotify)
		{
			if(is_point_inside(&primaryBtn, e.xmotion.x, e.xmotion.y))
			{
				if(!primaryBtn.mouseover)
					e.type = Expose;

				primaryBtn.mouseover = 1;

				if(secondaryBtn.mouseover)
					secondaryBtn.mouseover = 0;
			}
			else if(is_point_inside(&secondaryBtn, e.xmotion.x, e.xmotion.y))
			{
				if(!secondaryBtn.mouseover)
					e.type = Expose;

				secondaryBtn.mouseover = 1;

				if(primaryBtn.mouseover)
					primaryBtn.mouseover = 0;
			}
			else
			{
				if(primaryBtn.mouseover || secondaryBtn.mouseover)
					e.type = Expose;

				primaryBtn.mouseover = 0;
				primaryBtn.clicked = 0;
				secondaryBtn.mouseover = 0;
				secondaryBtn.clicked = 0;
			}
		}

		switch(e.type)
		{
			case ButtonPress:
			case ButtonRelease:
			{
				if(e.xbutton.button != Button1)
					break;

				if(primaryBtn.mouseover)
				{
					primaryBtn.clicked = e.type == ButtonPress ? 1 : 0;

					if(!primaryBtn.clicked)
					{
						ret = (flags & X11_MSG_BOX_BTN_OK) == X11_MSG_BOX_BTN_OK ? X11_MSG_BOX_RET_OK : X11_MSG_BOX_RET_YES;
						goto cleanup;
					}
				}
				else if(secondaryBtn.mouseover)
				{
					secondaryBtn.clicked = e.type == ButtonPress ? 1 : 0;

					if(!secondaryBtn.clicked)
					{
						ret = X11_MSG_BOX_RET_NO;
						goto cleanup;
					}
				}
				else
				{
					primaryBtn.clicked = 0;
					secondaryBtn.clicked = 0;
				}
			}
			break;
			case Expose:
			case MapNotify:
			{
				XClearWindow(dpy, w);
				
				/* Draw text lines */
				for( i=0, temp=text; temp; temp=end ? (end+1) : NULL, i+=height )
				{
					end = strchr(temp, '\n');

					XDrawString(dpy, w, gc, 10, 10+height+i, temp,
							end ? (unsigned int)(end-temp) : strlen(temp) );
				}

				/* Draw buttons */
				draw_button(&primaryBtn, white, black, dpy, w, gc);
				
				if((flags & X11_MSG_BOX_BTN_YESNO) == X11_MSG_BOX_BTN_YESNO)
					draw_button(&secondaryBtn, white, black, dpy, w, gc);

				XFlush(dpy);
			}            
			break;
			case KeyRelease:
			{
				if(XLookupKeysym(&e.xkey, 0) == XK_Escape)
                			goto cleanup;
			}
			break;
			case ClientMessage:
			{
				atom = XGetAtomName(dpy, e.xclient.message_type);

				if(*atom == *wmDeleteWindow)
				{
					XFree(atom);
					goto cleanup;
				}
				
				XFree(atom);
			}
			break;
		}
	}

cleanup:
	XFreeGC(dpy, gc);
	XDestroyWindow(dpy, w);
	XCloseDisplay(dpy);

	return ret;
}
