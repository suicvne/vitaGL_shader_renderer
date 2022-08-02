#import "vgl3d_metal_private.h"
#import <Foundation/Foundation.h>
#import <CoreFoundation/CoreFoundation.h>
#import <AppKit/AppKit.h>
#import <Cocoa/Cocoa.h>
#import <AppKit/NSColor.h>

@import Metal;
@import simd;
@import QuartzCore.CAMetalLayer;

#define SELF MulStruct* pSelf

int                 MulStruct_InitBackend(SELF);
void                MulStruct_DestroySelf(SELF);
void                MulStruct_PollEvents(SELF);
@interface DemoView : NSView
{
}
- (void)doSomethingElse;
@end

@interface DemoDelegate: NSObject<NSApplicationDelegate, NSWindowDelegate>
@end


MulStruct* MulStruct_CreateHeap() {
    MulStruct* newMulStruct = malloc(sizeof(MulStruct));

    *newMulStruct = MulStruct_Create();

    return newMulStruct;
}

inline MulStruct MulStruct_Create() {
    return (MulStruct) {
        .InitBackend = MulStruct_InitBackend,
        .DestroySelf = MulStruct_DestroySelf,
        .PollEvents =  MulStruct_PollEvents,
        .private.metalLayer = NULL,
        .private.myWindow =   NULL,
    };
}

// Implementing class functions

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
    [newWindow setContentView:demoView];
    [newWindow setDelegate:(id<NSWindowDelegate>)demoView];
    [newWindow makeKeyAndOrderFront: nil];
    newWindow.level = NSMainMenuWindowLevel;

    [NSApp activateIgnoringOtherApps:YES];
    NSApplicationMain(0, "");
    // [NSApp setDelegate:demoView];
    // [newWindo]


    NSLog(@"Initialized! Window: %p", newWindow);

    pSelf->PollEvents(pSelf);
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
    // if(pSelf->private.myWindow == NULL) return;

    // Poll for an event from the event stack.
    NSEvent *event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                        untilDate:[NSDate distantFuture]
                                           inMode:NSDefaultRunLoopMode
                                          dequeue:YES];

    NSEventType et = event.type;
    char* nsEtStr = NULL;
    NS_EVENT_TOSTR(et, nsEtStr);

    // NSLog(@"NS Event: %s (%lu)", nsEtStr, et);

    // Send it upward.
    [NSApp sendEvent:event];
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
@end


@implementation DemoDelegate
@end