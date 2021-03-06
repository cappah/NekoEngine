/* NekoEngine
 *
 * AnimatorComponent.h
 * Author: Alexandru Naiman
 *
 * AnimatorComponent class definition 
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2017, Alexandru Naiman
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

#pragma once

#include <Engine/Engine.h>
#include <Scene/ObjectComponent.h>
#include <Animation/AnimationClip.h>
#include <Animation/Skeleton.h>

class AnimatorComponent : public ObjectComponent
{
public:
	ENGINE_API AnimatorComponent(ComponentInitializer *initializer);

	ENGINE_API virtual int Load() override;

	ENGINE_API bool IsPlaying() { return _playing; }
	ENGINE_API bool IsOneShot() { return _oneShot; }
	
	ENGINE_API void PlayDefaultAnimation(bool loop = true) noexcept;
	ENGINE_API void PlayAnimation(AnimationClip *clip, bool loop = true) noexcept;
	ENGINE_API void PlayOneShot(AnimationClip *clip) noexcept
	{
		if (_skeleton->GetAnimationClip() == clip)
			return;
		
		_prevClip = _skeleton->GetAnimationClip();
		_prevLoop = _loop;
		PlayAnimation(clip, false);
		_oneShot = true;
	}
	
	ENGINE_API virtual void Update(double deltaTime) noexcept override;
	ENGINE_API void UpdateData(VkCommandBuffer commandBuffer) noexcept override;

	ENGINE_API virtual bool Unload() override;

	ENGINE_API virtual ~AnimatorComponent() { }

	Buffer *GetSkeletonBuffer() const noexcept { return _skeleton->GetBuffer(); }	
	
protected:
	std::string _defaultAnimId;
	std::string _targetMesh;
	double _currentTime;
	bool _playing;
	bool _loop;
	bool _prevLoop;
	bool _oneShot;
	
	Skeleton *_skeleton;
	AnimationClip *_defaultAnim;
	AnimationClip *_prevClip;
};
