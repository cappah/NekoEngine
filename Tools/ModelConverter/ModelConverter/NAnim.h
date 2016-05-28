//
//  NAnim.h
//  ModelConverter
//
//  Created by Alexandru Naiman on 21/05/16.
//  Copyright © 2016 Alexandru Naiman. All rights reserved.
//

#import <Foundation/Foundation.h>

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct VectorKey
{
	glm::dvec3 value;
	double time;
};

struct QuatKey
{
	glm::dquat value;
	double time;
};

typedef struct ANIMATION_NODE
{
	std::string name;
	std::vector<VectorKey> positionKeys;
	std::vector<QuatKey> rotationKeys;
	std::vector<VectorKey> scalingKeys;
} AnimationNode;

@interface NAnim : NSObject
{
	std::string _name;
	double _duration, _ticksPerSecond;
	std::vector<AnimationNode> _channels;
}

- (std::string)name;

- (id)initWithName:(std::string)name duration:(double)duration ticksPerSecond:(double)ticksPerSecond;
- (void)addChannel:(AnimationNode)channel;

- (bool)writeFile:(NSString *)path;

@end
