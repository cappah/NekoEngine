//
//  AppDelegate.m
//  ModelConverter
//
//  Created by Alexandru Naiman on 23/04/16.
//  Copyright © 2016 Alexandru Naiman. All rights reserved.
//

#import "AppDelegate.h"
#import "NMesh.h"

@interface AppDelegate ()

@property (weak) IBOutlet NSWindow *window;
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
	_assimpConverter = [[AssimpConverter alloc] init];
}

- (void)applicationWillTerminate:(NSNotification *)aNotification
{
	
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
	return TRUE;
}

- (IBAction)convertModel:(id)sender
{
	NSString *inFile = [inPath stringValue];
	
	if([inFile containsString:@"zfg"])
	{
		NMesh *m = [[NMesh alloc] init];
		[m loadZfg:inFile];
		[m writeFile:[outPath stringValue]];
	}
	else
		[_assimpConverter convert:[inPath stringValue] output:[outPath stringValue]];
		
}

@end
