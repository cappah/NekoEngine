/* NekoEngine
 *
 * GZipFile.h
 * Author: Alexandru Naiman
 *
 * Virtual File System - GZip compressed file
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

#ifdef ENGINE_INTERNAL
	#include <zlib.h>
#endif

#include <Engine/Engine.h>
#include <Runtime/Runtime.h>
#include <System/VFS/VFSFile.h>

class GZipFile : public VFSFile
{
public:
	ENGINE_API GZipFile();
	
	ENGINE_API virtual bool IsOpen() override;
	ENGINE_API virtual bool IsReadonly() override;

	ENGINE_API virtual int Open() override;
	ENGINE_API virtual int Create() override;

	ENGINE_API virtual size_t Read(void *buffer, size_t size, size_t count) override;
	ENGINE_API virtual char *Gets(char *str, int num) override;

	ENGINE_API virtual size_t Write(void *buffer, size_t size, size_t count) override;
	
	ENGINE_API virtual int Seek(size_t offset, int origin) override;
	ENGINE_API virtual size_t Tell() override;
	ENGINE_API virtual bool EoF() override;
	ENGINE_API virtual void Close() override;

	ENGINE_API virtual ~GZipFile();

private:
#ifdef ENGINE_INTERNAL
	gzFile _fp;
#endif
};