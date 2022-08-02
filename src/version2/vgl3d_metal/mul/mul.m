/**
 * @file mul.m
 * @author Michael Santiago (admin@ignoresolutions.xyz)
 * @brief MUL - Metal Utility Library.
 * Designed to be the modern replacement for glew/glfw on macOS.
 * 
 * The purpose of MUL is the following:
 * 1. Setup a simple NSWindow with a CAMetalLayer in its view. 
 * 2. Setup a Metal device and sane defaults.
 * 3. Accept draw commands in similar syntax to OpenGL calls.
 * 4. Translate draw commands to Metal commands.
 * 5. Allow access to this through the C ABI in a sane, object oriented way.
 * 
 * @version 0.1
 * @date 2022-08-02
 * 
 * @copyright Copyright Michael Santiago (c) 2022
 * 
 */
#import "mul.h"
#import <Foundation/Foundation.h>
#import <CoreFoundation/CoreFoundation.h>
#import <AppKit/AppKit.h>
#import <Cocoa/Cocoa.h>
#import <AppKit/NSColor.h>

// #include "../../common.h"

#define SELF MulStruct* pSelf

static float quadVertexData[] =
{
     0.5, -0.5, 0.0, 1.0,     1.0, 0.0, 0.0, 1.0,
    -0.5, -0.5, 0.0, 1.0,     0.0, 1.0, 0.0, 1.0,
    -0.5,  0.5, 0.0, 1.0,     0.0, 0.0, 1.0, 1.0,
    
     0.5,  0.5, 0.0, 1.0,     1.0, 1.0, 0.0, 1.0,
     0.5, -0.5, 0.0, 1.0,     1.0, 0.0, 0.0, 1.0,
    -0.5,  0.5, 0.0, 1.0,     0.0, 0.0, 1.0, 1.0,
};

// Hmmmmm.
typedef struct
{
    matrix_float4x4 rotation_matrix;
} Uniforms;

// Forward declare instance functions.
int                 MulStruct_InitBackend(SELF);
void                MulStruct_DestroySelf(SELF);
void                MulStruct_PollEvents(SELF);
int                 MulStruct_private_RenderTest(SELF);
void                MulStruct_private_SetWindowTitle(SELF, const char* newWinTitle);
void                MulStruct_private_SetUnifiedTitleBar(SELF, uint8_t unifiedTitleBarEnabled);

void MulStruct_NSLog(SELF, NSString* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    NSLogv(fmt, args);
    va_end(args);
}

MulStruct* MulStruct_CreateHeap() {
    MulStruct* newMulStruct = malloc(sizeof(MulStruct));

    *newMulStruct = MulStruct_Create();

    return newMulStruct;
}


inline MulStruct MulStruct_Create() {
    return (MulStruct) {
        .InitBackend =                      MulStruct_InitBackend,
        .DestroySelf =                      MulStruct_DestroySelf,
        .PollEvents =                       MulStruct_PollEvents,
        .RenderTest =                       MulStruct_private_RenderTest,
        .Log =                              MulStruct_NSLog,
        .SetWindowTitle =                   MulStruct_private_SetWindowTitle,
        .SetWindowUnifiedTitleBar =         MulStruct_private_SetUnifiedTitleBar,
        .private.windowDidResize =          0,
        .private.metalLayer =               NULL,
        .private.metalDevice =              NULL,
        .private.metalCommandQueue =        NULL,
        .private.metalDefLibrary =          NULL,
        .private.metalPipelineState =       NULL,
        .private.metalUniBuffer =           NULL,
        .private.metalVertexBuffer =        NULL,
        .private.metalPipelineDescriptor =  NULL,
        .private.myWindow =                 NULL,
        .private.winTitle =                 [[NSString alloc] initWithCString:"MUL - Metal Utility Library" encoding:NSUTF8StringEncoding],
    };
}

// Set NSWindow Title
void MulStruct_private_SetWindowTitle(SELF, const char* newWinTitle) {

    pSelf->private.winTitle = [NSString stringWithUTF8String:newWinTitle];
    pSelf->private.myWindow.title = pSelf->private.winTitle;
    pSelf->private.myWindow.subtitle = @"MUL - Metal Utility Library";
}

int MulStruct_private_CheckIfWindowUpdated(SELF) {
    #define PSELF pSelf->private
    if (PSELF.windowDidResize) {
        if((PSELF.myWindow.styleMask & NSWindowStyleMaskFullSizeContentView) == NSWindowStyleMaskFullSizeContentView)
            PSELF.metalLayer.frame = NSMakeRect(0,0,PSELF.myWindow.frame.size.width,PSELF.myWindow.frame.size.height);
        else
            PSELF.metalLayer.frame = PSELF.myWindow.contentLayoutRect;
        PSELF.windowDidResize = 0;
    }
    #undef PSELF
    return 0;
}

// Implementing class functions
int MulStruct_private_RenderTest(SELF) {

    MulStruct_private_CheckIfWindowUpdated(pSelf);
    
    #define MTLCMDQUEUE pSelf->private.metalCommandQueue

    // Drawable: [pSelf->private.metalLayer nextDrawable];
    id<CAMetalDrawable> currentDrawable = [pSelf->private.metalLayer nextDrawable];
    id<MTLTexture> texture = currentDrawable.texture;
    MTLRenderPassDescriptor *renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
    renderPassDescriptor.colorAttachments[0].texture =      texture;
    renderPassDescriptor.colorAttachments[0].clearColor =   MTLClearColorMake(0.0, 0.0, 1.0, 1.0);
    renderPassDescriptor.colorAttachments[0].loadAction =   MTLLoadActionClear;
    renderPassDescriptor.colorAttachments[0].storeAction =  MTLStoreActionStore;

    // Begin
    // Grab reference to current command buffer. from MTLCmdQueue
    id<MTLCommandBuffer> commandBuffer = [MTLCMDQUEUE commandBuffer];

    // Prepare render command encoder: [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
    id<MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];

    // Setup render pass.
    // [renderEncoder setRenderPipelineState:pSelf->private.metalPipelineState];
    // This is where most of the GL functions for drawing will be able to be translated.

    // Call [renderEncoder endEncoding];
    [renderEncoder endEncoding];

    // Issue draw calls using renderEncoder.
    // For this below, you need a drawable. We don't currently have one. 
    // Render/present: [commandBuffer presentDrawable:drawable];
    [commandBuffer presentDrawable:currentDrawable];


    // End [commandBuffer commit]
    [commandBuffer commit];


    #undef MTLCMDQUEUE
    return 0;
}

int MulStruct_private_BuildInitialPipeline(SELF) {
    #define PRIV_SELF pSelf->private

    #define defLib PRIV_SELF.metalDefLibrary
    pSelf->Log(pSelf, @"Building initial pipeline with default library %u....", defLib);

    // Create VBO for unit quad.
    pSelf->Log(pSelf, @"Building buffers.");
    PRIV_SELF.metalVertexBuffer = [PRIV_SELF.metalDevice newBufferWithBytes:quadVertexData length:sizeof(quadVertexData) options:MTLResourceOptionCPUCacheModeDefault];
    PRIV_SELF.metalUniBuffer = [PRIV_SELF.metalDevice newBufferWithLength:sizeof(Uniforms) options:MTLResourceOptionCPUCacheModeDefault];
    // Create a Uniform buffer for holding mvp matrices.
    pSelf->Log(pSelf, @"Building buffers completed!");

    // "Fetch" (create) the vertex and fragment "functions" (shaders)

    pSelf->Log(pSelf, @"Retrieving programs....");
    NSError* err = nil;

    pSelf->Log(pSelf, @"Func Names: %@", [defLib functionNames]);

    id<MTLFunction> fragProgram = [defLib newFunctionWithName:[defLib functionNames][0]];
    id<MTLFunction> vertProgram = [defLib newFunctionWithName:[defLib functionNames][1]];
    pSelf->Log(pSelf, @"Programs retrieved! %@, %@", fragProgram, vertProgram);


    // Build a render pipeline descriptor. Don't forget to free this later.
    pSelf->Log(pSelf, @"Alloc initing MTLRenderPipelineDescriptor");
    PRIV_SELF.metalPipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    pSelf->Log(pSelf, @"PIPELINE DESCRIPTOR ALLOCATED!");
    
    pSelf->Log(pSelf, @"Setting functions....");
    // Set the vertex and fragment shaders.
    [PRIV_SELF.metalPipelineDescriptor setVertexFunction:vertProgram];
    [PRIV_SELF.metalPipelineDescriptor setFragmentFunction:fragProgram];

    PRIV_SELF.metalPipelineDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;

    // Compile the pipeline!
    NSError* error = NULL;
    PRIV_SELF.metalPipelineState = 
        [PRIV_SELF.metalDevice newRenderPipelineStateWithDescriptor:PRIV_SELF.metalPipelineDescriptor error:&error];

    if(!PRIV_SELF.metalPipelineState) {
        pSelf->Log(pSelf, @"FAILED TO CREATE PIPELINE STATE!");
        return -1;
    }

    // [pipelineDescriptor release];

    #undef defLib
    #undef PRIV_SELF

    return 0;
}

static id<MTLLibrary> defaultMtlLib = nil;

int MulStruct_private_InitMetal(SELF, DemoView* viewPtr) {

    #define PRIV_SELF pSelf->private

    // Setup Metal.
    pSelf->Log(pSelf, @"Init Metal.");

    // Create the metal device.
    PRIV_SELF.metalDevice = MTLCreateSystemDefaultDevice();

    // Log the Metal Device information.
    pSelf->Log(pSelf, @"Metal Device: %p", pSelf->private.metalDevice);
    NSString* name = pSelf->private.metalDevice.name;
    MTLDeviceLocation location = pSelf->private.metalDevice.location;
    pSelf->Log(pSelf, @"Metal Device Name: %@", name);
    pSelf->Log(pSelf, @"Metal Device Location: %lu", location);

    // Create the Metal layer.
    PRIV_SELF.metalLayer =              [CAMetalLayer layer];
    PRIV_SELF.metalLayer.pixelFormat =  MTLPixelFormatBGRA8Unorm;
    PRIV_SELF.metalLayer.device =       PRIV_SELF.metalDevice;
    PRIV_SELF.metalLayer.frame =        PRIV_SELF.myWindow.contentLayoutRect;

    // Add the layer to the view.
    [viewPtr.layer addSublayer:pSelf->private.metalLayer];

    // Log
    pSelf->Log(pSelf, @"Metal Layer added to DemoView.. CAMetalLayer: %p", pSelf->private.metalLayer);

    // Create a new command queue.
    // This is what we load up to tell Metal what the fuck to do.
    PRIV_SELF.metalCommandQueue = [PRIV_SELF.metalDevice newCommandQueue];

    // Get the default library (shader cache)
    NSString* defNsString = [[NSString alloc] initWithCString:Mul_private_DefaultShaders encoding:NSUTF8StringEncoding];

    // Define a capturing Lambda so we can set pSelf->private.metalDefLibrary
    void(^metalLibComplete)(id<MTLLibrary> library, NSError *error) = ^(id<MTLLibrary> library, NSError *error){
        if(error) {
            pSelf->Log(pSelf, @"ERROR!!!!! %@", error);
            return;
        }

        pSelf->Log(pSelf, @"MTLLibrary completed!");
        pSelf->Log(pSelf, @"Func Names: %@", [library functionNames]);

        defaultMtlLib = library;
        pSelf->private.metalDefLibrary = (id<MTLLibrary>)library;
        [pSelf->private.metalDefLibrary retain]; // This is necessary, otherwise ARC ruins everyone's day.
    };

    pSelf->Log(pSelf, @"newLibraryWithSource:\n%@\n\n", defNsString);
    PRIV_SELF.metalDefLibrary = nil;
    
    PRIV_SELF.metalDefLibrary = [PRIV_SELF.metalDevice newDefaultLibrary];
    if(PRIV_SELF.metalDefLibrary == nil) {
        pSelf->Log(pSelf, @"METAL DEFAULT LIBRARY IS NULL!!!! Loading from C-String instead.");
        [PRIV_SELF.metalDevice newLibraryWithSource:defNsString options:nil completionHandler:metalLibComplete];
    }

    // TODO: Do I need to set viewPtr.contentScaleFactor..?

    #undef PRIV_SELF

    return 0;
}

void MulStruct_private_SetUnifiedTitleBar(SELF, uint8_t unifiedTitleBarEnabled) {
    NSWindowStyleMask winStyleMask = pSelf->private.myWindow.styleMask;

    if(unifiedTitleBarEnabled) {
        winStyleMask |= NSWindowStyleMaskFullSizeContentView;
        pSelf->private.myWindow.titlebarAppearsTransparent = 1;
        pSelf->private.myWindow.titleVisibility = NSWindowTitleHidden;
    } else {
        winStyleMask ^= NSWindowStyleMaskFullSizeContentView;
        pSelf->private.myWindow.titlebarAppearsTransparent = 0;
        pSelf->private.myWindow.titleVisibility = NSWindowTitleVisible;
    }
    
    [pSelf->private.myWindow setStyleMask:winStyleMask];
}

int MulStruct_InitBackend(SELF) {


    // app object
    NSApp = [NSApplication sharedApplication];

    //                              x     y     w      h
    NSRect myContentRect = NSMakeRect(0.f, 0.f, 960.f, 544.f);

    NSWindowStyleMask winStyleMask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable;
    NSBackingStoreType backStoreType = NSBackingStoreBuffered;

    // Create NSWindow
    NSWindow *newWindow = [[NSWindow alloc] initWithContentRect:myContentRect styleMask:winStyleMask backing:backStoreType defer:false];
    [newWindow setTitle:@"VGL3D Metal"];
    pSelf->private.myWindow = newWindow;

    DemoView* demoView = [[DemoView alloc] initWithFrame: myContentRect];
    [demoView retain];
    demoView.ActiveMulContext = pSelf;
    [newWindow setContentView:demoView];
    [newWindow setDelegate:(id<NSWindowDelegate>)demoView];
    [newWindow makeKeyAndOrderFront: nil];
    newWindow.level = NSMainMenuWindowLevel;
    pSelf->private.demoView = demoView;

    [NSApp activateIgnoringOtherApps:YES];
    NSLog(@"Initialized! Window: %p", newWindow);

    pSelf->PollEvents(pSelf);
    // Init Metal with our view we have control of.
    MulStruct_private_InitMetal(pSelf, demoView);
    do {} while(defaultMtlLib == nil);
    pSelf->private.metalDefLibrary = defaultMtlLib;
    pSelf->Log(pSelf, @"DONE! Building initial pipeline....");
    MulStruct_private_BuildInitialPipeline(pSelf);

    // Redraw?!
    [demoView setNeedsDisplay:YES];

    return 0;
}

void MulStruct_DestroySelf(SELF) {
    NSLog(@"Releasing Cocoa resources");
    [pSelf->private.myWindow release];
    [NSApp release];
}

#define STR_VALUE(x) #x
#define NS_EVENT_CASE(x, STOR) case x: (STOR) = STR_VALUE((x)); break;
#define NS_EVENT_TOSTR(X, STORAGE)\
switch(X){\
    NS_EVENT_CASE(NSEventTypeLeftMouseDown, STORAGE    ) \
    NS_EVENT_CASE(NSEventTypeLeftMouseUp, STORAGE      ) \
    NS_EVENT_CASE(NSEventTypeRightMouseDown, STORAGE   ) \
    NS_EVENT_CASE(NSEventTypeRightMouseUp, STORAGE     ) \
    NS_EVENT_CASE(NSEventTypeMouseMoved, STORAGE       ) \
    NS_EVENT_CASE(NSEventTypeLeftMouseDragged, STORAGE ) \
    NS_EVENT_CASE(NSEventTypeRightMouseDragged, STORAGE) \
    NS_EVENT_CASE(NSEventTypeMouseEntered, STORAGE     ) \
    NS_EVENT_CASE(NSEventTypeMouseExited, STORAGE      ) \
    NS_EVENT_CASE(NSEventTypeKeyDown, STORAGE          ) \
    NS_EVENT_CASE(NSEventTypeKeyUp, STORAGE            ) \
    NS_EVENT_CASE(NSEventTypeFlagsChanged, STORAGE     ) \
    NS_EVENT_CASE(NSEventTypeAppKitDefined, STORAGE    ) \
    NS_EVENT_CASE(NSEventTypeSystemDefined, STORAGE    ) \
    NS_EVENT_CASE(NSEventTypePeriodic, STORAGE         ) \
    NS_EVENT_CASE(NSEventTypeCursorUpdate, STORAGE     ) \
    NS_EVENT_CASE(NSEventTypeScrollWheel, STORAGE      ) \
    NS_EVENT_CASE(NSEventTypeTabletPoint, STORAGE      ) \
    NS_EVENT_CASE(NSEventTypeTabletProximity, STORAGE  ) \
    NS_EVENT_CASE(NSEventTypeOtherMouseDown, STORAGE   ) \
    NS_EVENT_CASE(NSEventTypeOtherMouseUp, STORAGE     ) \
    NS_EVENT_CASE(NSEventTypeOtherMouseDragged, STORAGE) \
    NS_EVENT_CASE(NSEventTypeGesture, STORAGE )          \
    NS_EVENT_CASE(NSEventTypeMagnify, STORAGE )          \
    NS_EVENT_CASE(NSEventTypeSwipe, STORAGE   )          \
    NS_EVENT_CASE(NSEventTypeRotate, STORAGE  )          \
    NS_EVENT_CASE(NSEventTypeBeginGesture, STORAGE)      \
    NS_EVENT_CASE(NSEventTypeEndGesture, STORAGE)        \
    NS_EVENT_CASE(NSEventTypeApplicationDefined, STORAGE)\
    NS_EVENT_CASE(NSEventTypeSmartMagnify, STORAGE)      \
    NS_EVENT_CASE(NSEventTypeQuickLook, STORAGE)         \
    NS_EVENT_CASE(NSEventTypePressure, STORAGE)          \
    NS_EVENT_CASE(NSEventTypeDirectTouch, STORAGE)       \
    NS_EVENT_CASE(NSEventTypeChangeMode, STORAGE)        \
    default: STORAGE = "UNKNOWN"; break;                 \
}\

// 

void MulStruct_PollEvents(SELF) {

    // Poll for an event from the event stack.
    NSEvent *event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                        untilDate:[NSDate distantFuture]
                                           inMode:NSDefaultRunLoopMode
                                          dequeue:YES];

    // NSEventType et = event.type;
    // char* nsEtStr = NULL;
    // NS_EVENT_TOSTR(et, nsEtStr);
    // NSLog(@"NS Event: %s (%lu)", nsEtStr, et);
    // Send it upward.
    [NSApp sendEvent:event];

    // Call render test
    // [pSelf->private.demoView setNeedsDisplay:YES];
}

// End Implementing Class Functions

#undef SELF

@implementation DemoView
-(void)doSomethingElse{
    NSLog(@"doSomethingElse called.");
}
-(void)windowWillClose:(NSNotification*)notification {
    NSLog(@"DemoView Terminating self.");
    [NSApp terminate:self];
}
- (void)viewDidLoad {
    // [super viewDidLoad];
    NSLog(@"Yes, it did load.");
}
-(void)windowDidResize:(NSNotification *)notification {

    // Notify mul that we need to resize the Metal context.
    if(self.ActiveMulContext != nil) {
        // NSLog(@"WINDOW DID RESIZE. Active mul: %p", self.ActiveMulContext);
        self.ActiveMulContext->private.windowDidResize = 1;
    }

    
}
@end