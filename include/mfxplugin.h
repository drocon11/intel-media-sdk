/******************************************************************************* *\

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2007-2010 Intel Corporation. All Rights Reserved.

File Name: mfxplugin.h

*******************************************************************************/
#ifndef __MFXPLUGIN_H__
#define __MFXPLUGIN_H__
#include "mfxvideo.h"

#pragma warning(disable: 4201)

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef enum {
    MFX_THREADPOLICY_SERIAL    = 0,
    MFX_THREADPOLICY_PARALLEL    = 1
} mfxThreadPolicy;

typedef struct mfxPluginParam {
    mfxU32  reserved[14];
    mfxThreadPolicy ThreadPolicy;
    mfxU32  MaxThreadNum;
} mfxPluginParam;

typedef struct mfxCoreParam{
    mfxU32  reserved[13];
    mfxIMPL Impl;
    mfxVersion Version;
    mfxU32  NumWorkingThread;
} mfxCoreParam;

typedef struct mfxCoreInterface {
    mfxHDL pthis;

    mfxHDL reserved1[2];
    mfxFrameAllocator reserved2;
    mfxBufferAllocator reserved3;

    mfxStatus (*GetCoreParam)(mfxHDL pthis, mfxCoreParam *par);
    mfxStatus (*GetHandle) (mfxHDL pthis, mfxHandleType type, mfxHDL *handle);
    mfxStatus (*IncreaseReference) (mfxHDL pthis, mfxFrameData *fd);
    mfxStatus (*DecreaseReference) (mfxHDL pthis, mfxFrameData *fd);
    mfxStatus (*CopyFrame) (mfxHDL pthis, mfxFrameSurface1 *dst, mfxFrameSurface1 *src);
    mfxStatus (*CopyBuffer)(mfxHDL pthis, mfxU8 *dst, mfxU32 size, mfxFrameSurface1 *src);

    mfxHDL reserved4[8];
} mfxCoreInterface;

typedef struct mfxPlugin{
    mfxHDL pthis;

    mfxStatus (*PluginInit) (mfxHDL pthis, mfxCoreInterface *core);
    mfxStatus (*PluginClose) (mfxHDL pthis);

    mfxStatus (*GetPluginParam)(mfxHDL pthis, mfxPluginParam *par);

    mfxStatus (*Submit)(mfxHDL pthis, const mfxHDL *in, mfxU32 in_num, const mfxHDL *out, mfxU32 out_num, mfxThreadTask *task);
    mfxStatus (*Execute)(mfxHDL pthis, mfxThreadTask task, mfxU32 uid_p, mfxU32 uid_a);
    mfxStatus (*FreeResources)(mfxHDL pthis, mfxThreadTask task, mfxStatus sts);

    mfxHDL reserved[9];
} mfxPlugin;



mfxStatus MFXVideoUSER_Register(mfxSession session, mfxU32 type, const mfxPlugin *par);
mfxStatus MFXVideoUSER_Unregister(mfxSession session, mfxU32 type);

mfxStatus MFXVideoUSER_ProcessFrameAsync(mfxSession session, const mfxHDL *in, mfxU32 in_num, const mfxHDL *out, mfxU32 out_num, mfxSyncPoint *syncp);

#ifdef __cplusplus
} // extern "C" 
#endif /* __cplusplus */

#endif /* __MFXPLUGIN_H__ */
