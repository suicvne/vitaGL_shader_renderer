/**
 * @file mul.h
 * @author Michael Santiago (admin@ignoresolutions.xyz)
 * @brief MUL - Metal Utility Library.
 * Designed to be the modern replacement for glew/glfw on macOS.
 * 
 * The purpose of MUL is the following:
 * 1. Setup a simple NSWindow with a CAMetalLayer in its view. 
 * 2. Setup a Metal device and sane defaults.
 * 3. Accept draw commands in similar syntax to OpenGL calls.
 * 4. Translate draw commands to Metal commands.
 * 
 * @version 0.1
 * @date 2022-08-02
 * 
 * @copyright Copyright Michael Santiago (c) 2022
 * 
 */

#ifndef __MUL_H__
#define __MUL_H__

#include "mul_default_shader.h"

// Thank you Falco Girgis for this excellent idea.
#ifndef C_PRIVATE_BEGIN
#define C_PRIVATE_BEGIN(N)  struct N##_private {
#endif
#ifndef C_PRIVATE_END
#define C_PRIVATE_END       } private;
#endif

#ifdef __OBJC__
#import <Foundation/Foundation.h>
#import <CoreFoundation/CoreFoundation.h>
#import <AppKit/AppKit.h>
#import <Cocoa/Cocoa.h>

// @import CoreGraphics;
// @import Metal;
// @import simd;
// @import QuartzCore.CAMetalLayer;

#import <CoreGraphics/CoreGraphics.h>
#import <Metal/Metal.h>
#import <Metal/MTLDevice.h>
#import <simd/simd.h>
#import <QuartzCore/CAMetalLayer.h>

#define OBJC_STRONG_PROP(x) __strong x

@interface DemoView : NSView
@property struct MulStruct* ActiveMulContext;
@end

#define MTLDEVICETYPE   id<MTLDevice>
#define MTLLIBTYPE      id<MTLLibrary>
#define MTLRPSTATETYPE  id<MTLRenderPipelineState>
#define MTLBUFFERTYPE   id<MTLBuffer>
#define MTLCMDQUEUETYPE id<MTLCommandQueue>
#define MTLRPDESCTYPE   MTLRenderPipelineDescriptor*
#define NSSTRINGTYPE    NSString*
#define DVIEWTYPE       DemoView*
#define NSSTRTYPE       NSString*
#else

// These types are declared for Obj C headers.
// They are NOT defined for C headers, so let's just define them.
typedef void NSWindow;
typedef void CAMetalLayer;
typedef void MTLDevice;
typedef void MTLLibrary;
typedef void MTLRenderPipelineState;
typedef void MTLBuffer;

#define MTLDEVICETYPE   uint32_t
#define MTLLIBTYPE      uint32_t
#define MTLRPSTATETYPE  uint32_t
#define MTLBUFFERTYPE   uint32_t
#define MTLCMDQUEUETYPE uint32_t
#define NSSTRINGTYPE    uint32_t
#define MTLRPDESCTYPE   void*
#define DVIEWTYPE       void*
#define NSSTRTYPE       void*

#define OBJC_STRONG_PROP(x) x

#endif

#define SELF struct MulStruct* pSelf

// Mul - Metal Utility Library
typedef struct MulStruct {

    int      (*InitBackend)(SELF);
    void     (*DestroySelf)(SELF);
    void     (*PollEvents)(SELF);
    void     (*Log)(SELF, NSSTRINGTYPE, ...);
    int      (*RenderTest)(SELF);
    void     (*SetWindowTitle)(SELF, const char*);
    void     (*SetWindowUnifiedTitleBar)(SELF, uint8_t);

    C_PRIVATE_BEGIN(MulStruct)
    OBJC_STRONG_PROP(NSSTRTYPE)                    winTitle;
    OBJC_STRONG_PROP(DVIEWTYPE)                    demoView;

    OBJC_STRONG_PROP(NSWindow*)                    myWindow;

    // Metal Stuff.
    OBJC_STRONG_PROP(CAMetalLayer*)                metalLayer;
    OBJC_STRONG_PROP(MTLDEVICETYPE)                metalDevice;
    
    OBJC_STRONG_PROP(MTLCMDQUEUETYPE)              metalCommandQueue;
    OBJC_STRONG_PROP(MTLLIBTYPE)                   metalDefLibrary;
    OBJC_STRONG_PROP(MTLRPSTATETYPE)               metalPipelineState;
    OBJC_STRONG_PROP(MTLBUFFERTYPE)                metalUniBuffer;
    OBJC_STRONG_PROP(MTLBUFFERTYPE)                metalVertexBuffer;
    OBJC_STRONG_PROP(MTLRPDESCTYPE)                metalPipelineDescriptor;
    uint8_t                                        windowDidResize:1;
    
    C_PRIVATE_END;

} MulStruct;

MulStruct  MulStruct_Create();
MulStruct* MulStruct_CreateHeap();

#undef SELF

#undef OBJ_STRONG_PROP
#undef MTLDEVICETYPE
#undef MTLRPSTATETYPE
#undef MTLBUFFERTYPE

#endif //__MUL_H__