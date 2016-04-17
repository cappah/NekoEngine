#-------------------------------------------------
#
# Project created by QtCreator 2016-02-11T04:48:06
#
#-------------------------------------------------

TEMPLATE = subdirs 
CONFIG += ordered
SUBDIRS = Engine \
	  Editor \
	  Utilities \
	  TestGame

Engine.depends = Utilities TestGame
Editor.depends = Engine
