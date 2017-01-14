/* NekoEngine
 *
 * NullAudio.h
 * Author: Alexandru Naiman
 *
 * NekoEngine NullAudio System
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

#include "NullAudio.h"

using namespace glm;

NullAudioBuffer::NullAudioBuffer(size_t size) : AudioBuffer(size) { }
void NullAudioBuffer::SetData(AudioFormat format, size_t frequency, size_t size, void *data) { (void)format; (void)frequency, (void)size, (void)data; }
NullAudioBuffer::~NullAudioBuffer() { }

NullAudioSource::NullAudioSource() noexcept { }
void NullAudioSource::SetPitch(float p) noexcept { (void)p; }
void NullAudioSource::SetGain(float g) noexcept { (void)g; }
void NullAudioSource::SetConeInnerAngle(float a) noexcept { (void)a; }
void NullAudioSource::SetConeOuterAngle(float a) noexcept { (void)a; }
void NullAudioSource::SetConeOuterGain(float g) noexcept { (void)g; }
void NullAudioSource::SetDirection(vec3 &dir) noexcept { (void)dir; }
void NullAudioSource::SetPosition(vec3 &pos) noexcept { (void)pos; }
void NullAudioSource::SetVelocity(vec3 &v) noexcept { (void)v; }
void NullAudioSource::SetLooping(bool looping) noexcept { (void)looping; }
void NullAudioSource::SetMaxDistance(float maxDistance) noexcept { (void)maxDistance; }
void NullAudioSource::SetReferenceDistance(float referenceDistance) noexcept { (void)referenceDistance; }
int NullAudioSource::SetClip(AudioClip *clip) noexcept { (void)clip; return ENGINE_OK; }
bool NullAudioSource::Play() noexcept { return true; }
void NullAudioSource::Pause() noexcept { }
void NullAudioSource::Stop() noexcept { }
void NullAudioSource::Rewind() noexcept { }
bool NullAudioSource::IsPlaying() noexcept { return false; }
NullAudioSource::~NullAudioSource() { }

int NullAudio::Initialize() { return ENGINE_OK; }
const char *NullAudio::GetName() { return "NullAudio"; }
const char *NullAudio::GetVersion() { return "0.4.0.1"; }
AudioBuffer *NullAudio::CreateBuffer(size_t size) { return (AudioBuffer *)new NullAudioBuffer(size); }
AudioSource *NullAudio::CreateSource() { return (AudioSource *)new NullAudioSource(); }
void NullAudio::SetDistanceModel(AudioDistanceModel model) { (void)model; }
void NullAudio::SetListenerPosition(glm::vec3 &position) { (void)position; }
void NullAudio::SetListenerVelocity(glm::vec3 &velocity) { (void)velocity; }
void NullAudio::SetListenerOrientation(glm::vec3 &front, glm::vec3 &up) { (void)front; (void)up; }
void NullAudio::Update(double deltaTime) { (void)deltaTime; }
void NullAudio::Release() { }
NullAudio::~NullAudio() { }

#if defined(_WIN32) || defined(_WIN64)
	#define EXPORT __declspec(dllexport)
#else
	#define EXPORT
#endif

extern "C" EXPORT AudioSystem *createAudioSystem() { return (AudioSystem *)new NullAudio(); }