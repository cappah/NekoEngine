//
//  NAnim.m
//  ModelConverter
//
//  Created by Alexandru Naiman on 21/05/16.
//  Copyright © 2016 Alexandru Naiman. All rights reserved.
//

#import "NAnim.h"

#include <zlib.h>
#include <string.h>

#include <sstream>

using namespace std;

@implementation NAnim

- (id)initWithName:(std::string)name duration:(double)duration ticksPerSecond:(double)ticksPerSecond;
{
	if((self = [super init]) == nil)
		return nil;
	
	_name = name;
	
	if(_name.size() == 0)
	{
		static int index = 0;
		char file[256];
		snprintf(file, 256, "unnamed_%d", index++);
		_name = file;
	}
	
	_duration = duration;
	_ticksPerSecond = ticksPerSecond;
	
	return self;
}

- (std::string)name
{
	return _name;
}

- (void)addChannel:(AnimationNode)channel
{
	_channels.push_back(channel);
}

- (bool)writeFile:(NSString *)path
{
	stringstream ss("", ios_base::app | ios_base::out);
	
	ss << "name:" << _name << endl;
	ss << "duration:" << _duration << endl;
	ss << "tickspersecond:" << _ticksPerSecond << endl;
	ss << "channels:" << _channels.size() << endl;
	
	for(AnimationNode &n : _channels)
	{
		ss << "Channel=" << n.name << endl;
		
		for(VectorKey &k : n.positionKeys)
			ss << "poskey[" << k.time << "|" << k.value.x << "," << k.value.y << "," << k.value.z << "]" << endl;
		
		for(QuatKey &k : n.rotationKeys)
		{
			ss << "rotkey[" << k.time << "|" << k.value.x << "," << k.value.y << "," << k.value.z << "," << k.value.w << "];" << endl;
		}
		
		for(VectorKey &k : n.scalingKeys)
			ss << "scalekey[" << k.time << "|" << k.value.x << "," << k.value.y << "," << k.value.z << "]" << endl;
		
		ss << "EndChannel" << endl;
	}
	
	gzFile fp = gzopen([path UTF8String], "wb");
	gzbuffer(fp, 1048576);
	gzwrite(fp, ss.str().c_str(), (unsigned int)ss.str().size());
	gzclose(fp);
	
	return true;
}

@end
