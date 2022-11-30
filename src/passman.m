/* passman.m -- https://github.com/takeiteasy/passman
 
 The MIT License (MIT)

 Copyright (c) 2022 George Watson

 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge,
 publish, distribute, sublicense, and/or sell copies of the Software,
 and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:

 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include "spectre.h"
#import <Cocoa/Cocoa.h>
#import <Security/Security.h>

#define PHI 1.618033988749895

@interface Label : NSTextField
@end

@implementation Label
-(id)initWithFrame:(NSRect)frame andString:(NSString*)string {
    if (self = [super initWithFrame:frame]) {
        if (string)
            [self setStringValue:string];
        [self setBezeled:NO];
        [self setDrawsBackground:NO];
        [self setEditable:NO];
        [self setSelectable:NO];
    }
    return self;
}
@end

@interface PopoverView : NSView <NSTextFieldDelegate> {
    NSButton *quitButton;
    NSTextField *userField;
    NSSecureTextField *passField;
    Label *userFieldLabel, *passFieldLabel;
    NSImage *image;
}
@end

@implementation PopoverView
-(BOOL)control:(NSControl *)control textView:(NSTextView *)textView doCommandBySelector:(SEL)commandSelector {
//    if (commandSelector == @selector(insertTab:) || commandSelector == @selector(deleteBackward:) || commandSelector == @selector(deleteForward:))
//        return NO;
    if (commandSelector == @selector(insertNewline:)) {
        // Enter pressed ...
        return YES;
    }
    if (commandSelector == @selector(cancelOperation:)) {
        [textView setString:@""];
        return YES;
    }
    return NO;
}

-(id)initWithFrame:(NSRect)frame {
    if (self = [super initWithFrame:frame]) {
        quitButton = [[NSButton alloc] initWithFrame:NSMakeRect(5.f, 5.f, 25.f * PHI, 25.f)];
        [quitButton setBordered:NO];
        [quitButton setTitle:@"Quit"];
        [quitButton setAction:@selector(terminate:)];
        [self addSubview:quitButton];
        
        passField = [[NSSecureTextField alloc] initWithFrame:NSMakeRect(60, 50, 200, 20)];
        [passField setDelegate:self];
        [passField setNextKeyView:userField];
        [self addSubview:passField];
        
        passFieldLabel = [[Label alloc] initWithFrame:NSMakeRect(60, 70, 200, 20)
                                            andString:@"Password:"];
        [self addSubview:passFieldLabel];
        
        userField = [[NSTextField alloc] initWithFrame:NSMakeRect(60, 100, 200, 20)];
        [userField setDelegate:self];
        [userField setNextKeyView:passField];
        [self addSubview:userField];
        
        userFieldLabel = [[Label alloc] initWithFrame:NSMakeRect(60, 120, 200, 20)
                                            andString:@"Username:"];
        [self addSubview:userFieldLabel];
        
        image = [NSImage imageWithSystemSymbolName:@"lock.fill"
                          accessibilityDescription:nil];
    }
    return self;
}

-(void)drawRect:(NSRect)dirtyRect {
    [[NSColor clearColor] set];
    NSRectFill([self bounds]);
    
    NSGraphicsContext *gctx = [NSGraphicsContext currentContext];
#if __MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_9
    CGContextRef ctx = (CGContextRef)[gctx graphicsPort];
#else
    CGContextRef ctx = [gctx CGContext];
#endif
    
    NSRect imageRect = NSMakeRect(135.f, 160.f, 50.f, 50.f);
    [gctx saveGraphicsState];
    CGContextBeginTransparencyLayerWithRect(ctx, imageRect, nil);
    CGContextSetBlendMode(ctx, kCGBlendModeNormal);
    
    CGImageRef cgImage = [image CGImageForProposedRect:&imageRect context:NULL hints:nil];
    CGContextDrawImage(ctx, imageRect, cgImage);
    CGImageRelease(cgImage);
    
    CGContextSetBlendMode(ctx, kCGBlendModeSourceIn);
    CGContextSetFillColor(ctx, &NSWhite);
    CGContextFillRect(ctx, imageRect);
    CGContextEndTransparencyLayer(ctx);
    
    [[NSGraphicsContext currentContext] flushGraphics];
}
@end

@interface AppDelegate : NSObject <NSApplicationDelegate, NSWindowDelegate> {
    NSPopover *popover;
    PopoverView *view;
    NSRect popoverRect;
}
@property (strong, nonatomic) NSStatusItem* statusItem;
@end

@implementation AppDelegate : NSObject
-(void)togglePopover:(NSButton*)sender {
    if ([popover isShown]) {
        [popover performClose:sender];
    } else {
        [popover showRelativeToRect:popoverRect
                             ofView:sender
                      preferredEdge:NSRectEdgeMinY];
    }
}

-(id)init {
    if (self = [super init]) {
        NSNotificationCenter* nc = [NSNotificationCenter defaultCenter];
        [nc addObserver:self
               selector:@selector(terminate:)
                   name:NSApplicationWillTerminateNotification
                 object:nil];
        
        _statusItem = [[NSStatusBar systemStatusBar] statusItemWithLength:NSVariableStatusItemLength];
        _statusItem.button.image = [NSImage imageWithSystemSymbolName:@"key"
                                             accessibilityDescription:nil];
#if __MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_4
        _statusItem.highlightMode = YES;
#endif
        [[_statusItem button] setAction:@selector(togglePopover:)];
        
        popoverRect = NSMakeRect(0.f, 0.f, 320.f, 240.f);
        view = [[PopoverView alloc] initWithFrame:popoverRect];
        popover = [[NSPopover alloc] init];
        [popover setBehavior:NSPopoverBehaviorTransient];
        [popover setAnimates:YES];
        [popover setContentSize:popoverRect.size];
        NSViewController *viewController = [[NSViewController alloc] init];
        [viewController setView:view];
        [popover setContentViewController:viewController];
    }
    return self;
}
@end

int main(int argc, const char *argv[]) {
    @autoreleasepool {
        [NSApplication sharedApplication];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyAccessory];
        [NSApp setDelegate:[AppDelegate new]];
        [NSApp activateIgnoringOtherApps:YES];
        [NSApp run];
    }
    return 0;
}
