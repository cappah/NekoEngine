//
//  AppDelegate.h
//  ModelConverter
//
//  Created by Alexandru Naiman on 23/04/16.
//  Copyright © 2016 Alexandru Naiman. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "AssimpConverter.h"

@interface AppDelegate : NSObject <NSApplicationDelegate>
{
	IBOutlet NSTextField* inPath;
	IBOutlet NSTextField* outPath;
	
	AssimpConverter *_assimpConverter;
}

- (IBAction)convertModel:(id)sender;

@end

