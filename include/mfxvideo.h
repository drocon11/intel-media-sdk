/* ****************************************************************************** *\

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2007-2010 Intel Corporation. All Rights Reserved.

File Name: mfxvideo.h

\* ****************************************************************************** */
#ifndef __MFXVIDEO_H__
#define __MFXVIDEO_H__
#include "mfxstructures.h"

/* This is the external include file for the Intel(R) Media Sofware Development Kit product */
#define MFX_VERSION_MAJOR 1
#define MFX_VERSION_MINOR 1

#ifdef __cplusplus
extern "C"
{
#endif

/* MFXVideoCORE */
typedef struct {
    mfxU32      reserved[4];
    mfxHDL      pthis;
    mfxStatus   (*Alloc)    (mfxHDL pthis, mfxU32 nbytes, mfxU16 type, mfxMemId *mid);
    mfxStatus   (*Lock)     (mfxHDL pthis, mfxMemId mid, mfxU8 **ptr);
    mfxStatus   (*Unlock)   (mfxHDL pthis, mfxMemId mid);
    mfxStatus   (*Free)     (mfxHDL pthis, mfxMemId mid);
} mfxBufferAllocator;

typedef struct {
    mfxU32      reserved[4];
    mfxHDL      pthis;

    mfxStatus   (*Alloc)    (mfxHDL pthis, mfxFrameAllocRequest *request, mfxFrameAllocResponse *response);
    mfxStatus   (*Lock)     (mfxHDL pthis, mfxMemId mid, mfxFrameData *ptr);
    mfxStatus   (*Unlock)   (mfxHDL pthis, mfxMemId mid, mfxFrameData *ptr);
    mfxStatus   (*GetHDL)   (mfxHDL pthis, mfxMemId mid, mfxHDL *handle);
    mfxStatus   (*Free)     (mfxHDL pthis, mfxFrameAllocResponse *response);
} mfxFrameAllocator;

/* Library Common */
typedef struct _mfxSession *mfxSession;
mfxStatus MFXInit(mfxIMPL impl, mfxVersion *ver, mfxSession *session);
mfxStatus MFXClose(mfxSession session);

mfxStatus MFXQueryIMPL(mfxSession session, mfxIMPL *impl);
mfxStatus MFXQueryVersion(mfxSession session, mfxVersion *version);

mfxStatus MFXJoinSession(mfxSession session, mfxSession child);
mfxStatus MFXDisjoinSession(mfxSession session);
mfxStatus MFXCloneSession(mfxSession session, mfxSession *clone);
mfxStatus MFXSetPriority(mfxSession session, mfxPriority priority);
mfxStatus MFXGetPriority(mfxSession session, mfxPriority *priority);

/* VideoCORE: */
mfxStatus MFXVideoCORE_SetBufferAllocator(mfxSession session, mfxBufferAllocator *allocator);
mfxStatus MFXVideoCORE_SetFrameAllocator(mfxSession session, mfxFrameAllocator *allocator);
mfxStatus MFXVideoCORE_SetHandle(mfxSession session, mfxHandleType type, mfxHDL hdl);
mfxStatus MFXVideoCORE_GetHandle(mfxSession session, mfxHandleType type, mfxHDL *hdl);

typedef struct _mfxSyncPoint *mfxSyncPoint;
mfxStatus MFXVideoCORE_SyncOperation(mfxSession session, mfxSyncPoint syncp, mfxU32 wait);

/* VideoENCODE */
mfxStatus MFXVideoENCODE_Query(mfxSession session, mfxVideoParam *in, mfxVideoParam *out);
mfxStatus MFXVideoENCODE_QueryIOSurf(mfxSession session, mfxVideoParam *par, mfxFrameAllocRequest *request);
mfxStatus MFXVideoENCODE_Init(mfxSession session, mfxVideoParam *par);
mfxStatus MFXVideoENCODE_Reset(mfxSession session, mfxVideoParam *par);
mfxStatus MFXVideoENCODE_Close(mfxSession session);

mfxStatus MFXVideoENCODE_GetVideoParam(mfxSession session, mfxVideoParam *par);
mfxStatus MFXVideoENCODE_GetEncodeStat(mfxSession session, mfxEncodeStat *stat);
mfxStatus MFXVideoENCODE_EncodeFrameAsync(mfxSession session, mfxEncodeCtrl *ctrl, mfxFrameSurface1 *surface, mfxBitstream *bs, mfxSyncPoint *syncp);

/* VideoDECODE */
mfxStatus MFXVideoDECODE_Query(mfxSession session, mfxVideoParam *in, mfxVideoParam *out);
mfxStatus MFXVideoDECODE_DecodeHeader(mfxSession session, mfxBitstream *bs, mfxVideoParam *par);
mfxStatus MFXVideoDECODE_QueryIOSurf(mfxSession session, mfxVideoParam *par, mfxFrameAllocRequest *request);
mfxStatus MFXVideoDECODE_Init(mfxSession session, mfxVideoParam *par);
mfxStatus MFXVideoDECODE_Reset(mfxSession session, mfxVideoParam *par);
mfxStatus MFXVideoDECODE_Close(mfxSession session);

mfxStatus MFXVideoDECODE_GetVideoParam(mfxSession session, mfxVideoParam *par);
mfxStatus MFXVideoDECODE_GetDecodeStat(mfxSession session, mfxDecodeStat *stat);
mfxStatus MFXVideoDECODE_SetSkipMode(mfxSession session, mfxSkipMode mode);
mfxStatus MFXVideoDECODE_GetPayload(mfxSession session, mfxU64 *ts, mfxPayload *payload);
mfxStatus MFXVideoDECODE_DecodeFrameAsync(mfxSession session, mfxBitstream *bs, mfxFrameSurface1 *surface_work, mfxFrameSurface1 **surface_out, mfxSyncPoint *syncp);

/* VideoVPP */
mfxStatus MFXVideoVPP_Query(mfxSession session, mfxVideoParam *in, mfxVideoParam *out);
mfxStatus MFXVideoVPP_QueryIOSurf(mfxSession session, mfxVideoParam *par, mfxFrameAllocRequest request[2]);
mfxStatus MFXVideoVPP_Init(mfxSession session, mfxVideoParam *par);
mfxStatus MFXVideoVPP_Reset(mfxSession session, mfxVideoParam *par);
mfxStatus MFXVideoVPP_Close(mfxSession session);

mfxStatus MFXVideoVPP_GetVideoParam(mfxSession session, mfxVideoParam *par);
mfxStatus MFXVideoVPP_GetVPPStat(mfxSession session, mfxVPPStat *stat);
mfxStatus MFXVideoVPP_RunFrameVPPAsync(mfxSession session, mfxFrameSurface1 *in, mfxFrameSurface1 *out, mfxExtVppAuxData *aux, mfxSyncPoint *syncp);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
