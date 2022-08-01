#ifndef __VGL3D_METAL_PRIVATE_H__
#define __VGL3D_METAL_PRIVATE_H__

#include "../common.h"

#ifdef __OBJC__
#include <Foundation/Foundation.h>
#include <CoreFoundation/CoreFoundation.h>
#include <AppKit/AppKit.h>
#include <Cocoa/Cocoa.h>

@import QuartzCore.CAMetalLayer;
#define OBJC_STRONG_PROP(x) __strong __unsafe_unretained x 
#else
typedef void NSWindow;
typedef void CAMetalLayer;

#define OBJC_STRONG_PROP(x) x
#endif

#define SELF struct MulStruct* pSelf

// Mul - Metal Utility Lite
typedef struct MulStruct {

    int      (*InitBackend)(SELF);
    void     (*DestroySelf)(SELF);
    void     (*PollEvents)(SELF);


    C_PRIVATE_BEGIN(MulStruct)
    OBJC_STRONG_PROP(NSWindow*)       myWindow;
    OBJC_STRONG_PROP(CAMetalLayer*)   metalLayer;
    C_PRIVATE_END;

} MulStruct;

MulStruct  MulStruct_Create();
MulStruct* MulStruct_CreateHeap();

#undef SELF

#endif
