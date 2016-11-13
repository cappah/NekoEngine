/* NekoEngine - ModelImporter
 *
 * AnimationClip.cpp
 * Author: Alexandru Naiman
 *
 * AnimationClip implementation
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2016, Alexandru Naiman
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <zlib.h>

#include "AnimationClip.h"

void AnimationClip::Export(const char *file)
{
	uint32_t num{0};
	gzFile fp = gzopen(file, "wb");

	gzwrite(fp, ANIM_HEADER, 7);

	gzwrite(fp, &num, sizeof(uint32_t));
	gzwrite(fp, _name.c_str(), num);

	gzwrite(fp, &_duration, sizeof(double));
	gzwrite(fp, &_ticksPerSecond, sizeof(double));

	num = (uint32_t)_channels.size();
	gzwrite(fp, &num, sizeof(uint32_t));

	for (AnimationNode &channel : _channels)
	{
		num = (uint32_t)strlen(channel.name.c_str());
		gzwrite(fp, &num, sizeof(uint32_t));
		gzwrite(fp, channel.name.c_str(), num);

		num = (uint32_t)channel.positionKeys.size();
		gzwrite(fp, &num, sizeof(uint32_t));
		for(VectorKey &vk : channel.positionKeys)
		{
			gzwrite(fp, &vk.value.x, sizeof(double) * 3);
			gzwrite(fp, &vk.time, sizeof(double));
		}

		num = (uint32_t)channel.rotationKeys.size();
		gzwrite(fp, &num, sizeof(uint32_t));
		for(QuatKey &qk : channel.rotationKeys)
		{
			gzwrite(fp, &qk.value.x, sizeof(double) * 4);
			gzwrite(fp, &qk.time, sizeof(double));
		}

		num = (uint32_t)channel.scalingKeys.size();
		gzwrite(fp, &num, sizeof(uint32_t));
		for(VectorKey &vk : channel.scalingKeys)
		{
			gzwrite(fp, &vk.value.x, sizeof(double) * 3);
			gzwrite(fp, &vk.time, sizeof(double));
		}
	}

	gzwrite(fp, ANIM_FOOTER, 7);

	gzclose(fp);
}
