//
//  MenuController.m
//  Launcher
//
//  Created by Alexandru Naiman on 31/01/17.
//  Copyright © 2017 Alexandru Naiman. All rights reserved.
//

#import "MenuController.h"

#include <Engine/Engine.h>
#include <Engine/SceneManager.h>

@implementation MenuController

- (IBAction)startGame:(id)sender
{
	[[[UIApplication sharedApplication] delegate] performSelector:@selector(showEngine)];
	Engine::Pause(false);
	SceneManager::LoadScene(0);
}

- (IBAction)resumeGame:(id)sender
{
	[[[UIApplication sharedApplication] delegate] performSelector:@selector(showEngine)];
	Engine::Pause(false);
}

- (IBAction)endGame:(id)sender
{
	SceneManager::LoadScene(1);
}

- (IBAction)saveSettings:(id)sender
{
	// TODO: Implement WriteConfigFloat
}

@end
