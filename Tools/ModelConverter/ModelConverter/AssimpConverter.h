//
//  AssimpConverter.h
//  ModelConverter
//
//  Created by Alexandru Naiman on 23/04/16.
//  Copyright © 2016 Alexandru Naiman. All rights reserved.
//

#import <Foundation/Foundation.h>

#include <assimp/Importer.hpp>

@interface AssimpConverter : NSObject
{
	Assimp::Importer _importer;
	aiMatrix4x4 _globalInverseTransform;
}

- (id)init;
- (bool)convert:(NSString *)srcFile output:(NSString *)nmeshFile;

@end
