//
//  MenuController.h
//  Launcher
//
//  Created by Alexandru Naiman on 31/01/17.
//  Copyright © 2017 Alexandru Naiman. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

@interface MenuController : NSObject

// Main Menu
- (IBAction)startGame:(id)sender;

// In-Game Menu, End Game Menu
- (IBAction)resumeGame:(id)sender;
- (IBAction)endGame:(id)sender;

// Settings Menu
- (IBAction)saveSettings:(id)sender;

@end
