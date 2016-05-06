//
//  AppDelegate.m
//  Launcher-iOS
//
//  Created by Alexandru Naiman on 06/05/16.
//  Copyright © 2016 Alexandru Naiman. All rights reserved.
//

#import "AppDelegate.h"

#include <Engine/Engine.h>

@interface AppDelegate ()

@end

@implementation AppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
	UIWindow *window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
	[self setWindow:window];
	[window makeKeyAndVisible];
	
	std::string args("");
	
	args.append("--data=");
	args.append([[[NSBundle mainBundle] resourcePath] UTF8String]);
	args.append("/Data --ini=");
	args.append([[[NSBundle mainBundle] resourcePath] UTF8String]);
	args.append("/Engine_Low.ini --log=");
	
	NSArray *urls = [[NSFileManager defaultManager] URLsForDirectory:NSLibraryDirectory inDomains:NSUserDomainMask];
	NSURL *logUrl = [[urls lastObject] URLByAppendingPathComponent:@"Logs"];
	logUrl = [logUrl URLByAppendingPathComponent:@"NekoEngine.log"];
	args.append([[logUrl path] UTF8String]);
	
	if (Engine::Initialize(args, false) != ENGINE_OK)
	{
		printf("Failed to initialize engine.\n");
		Platform::MessageBox("Fatal error", "Failed to initialize engine", MessageBoxButtons::OK, MessageBoxIcon::Error);
		return false;
	}
	
	Engine::Run();
	
	return true;
}

- (void)applicationWillResignActive:(UIApplication *)application {
	// Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
	// Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
}

- (void)applicationDidEnterBackground:(UIApplication *)application {
	// Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
	// If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
}

- (void)applicationWillEnterForeground:(UIApplication *)application {
	// Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
	// Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
}

- (void)applicationWillTerminate:(UIApplication *)application {
	// Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
}

@end
