#!/bin/sh

mkdir -p Tools/bin
cc -I/usr/local/include -L/usr/local/lib -o Tools/bin/png2argb Tools/png2argb.c -lpng

Tools/bin/png2argb Launcher/X11/icon_16.png Launcher/X11/icon_32.png Launcher/X11/icon_64.png Launcher/X11/icon_128.png Launcher/X11/icon_256.png Launcher/X11/icon_512.png > Engine/Platform/X11/x11_icon.h
