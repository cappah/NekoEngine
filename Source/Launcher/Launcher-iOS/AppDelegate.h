//
//  AppDelegate.h
//  Launcher-iOS
//
//  Created by Alexandru Naiman on 06/05/16.
//  Copyright © 2016 Alexandru Naiman. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface AppDelegate : UIResponder <UIApplicationDelegate>
{
	UIStoryboard *_storyboard;
	UIViewController *_mainMenuViewController, *_inGameMenuViewController;
	UIViewController *_engineViewController;
}

@property (strong, nonatomic) UIWindow *window;

- (void)showMenu;
- (void)showEngine;

@end

