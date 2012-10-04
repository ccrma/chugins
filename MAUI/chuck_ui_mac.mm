/*----------------------------------------------------------------------------
 ChucK Concurrent, On-the-fly Audio Programming Language
 Compiler and Virtual Machine
 
 Copyright (c) 2004 Ge Wang and Perry R. Cook.  All rights reserved.
 http://chuck.cs.princeton.edu/
 http://soundlab.cs.princeton.edu/
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 U.S.A.
 -----------------------------------------------------------------------------*/

//-----------------------------------------------------------------------------
// file: chuck_ui_mac.mm
// desc: ui boilerplate for mac os x 
//
// author: Spencer Salazar (spencer@ccrma.stanford.edu)
// date: October 2011
//-----------------------------------------------------------------------------

#include "chuck_ui.h"

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>
#import "chuck_vm.h"
#import "chuck_globals.h"
#import "util_icon.h"


@class Chuck_UI_Controller;


static Chuck_UI_Manager * g_ui_manager = NULL;
static Chuck_UI_Controller * g_ui_controller = nil;


@interface Chuck_UI_Controller : NSObject<NSApplicationDelegate>
{
@private
    Chuck_UI_Manager * uiManager;
}

- (void)quit;

@end

Chuck_UI_Manager * Chuck_UI_Manager::instance()
{
    if(g_ui_manager == NULL)
        g_ui_manager = new Chuck_UI_Manager;
    return g_ui_manager;
}

Chuck_UI_Manager::Chuck_UI_Manager()
{
    init();
    
    g_ui_controller = [Chuck_UI_Controller new];
}

void Chuck_UI_Manager::init()
{
    m_hasStarted = false;
    m_doStart = false;
    m_doShutdown = false;
    m_hookActivated = false;
    m_hook = NULL;
}

void Chuck_UI_Manager::run()
{
    while( !m_doStart && !m_doShutdown )
    {
        // TODO: semaphore?
        usleep(10000);
    }
    
    if( m_doShutdown )
        return;
    
    m_doStart = false;
    
    // TODO: handle case where VM shuts down before this function finishes,
    // which seems to cause crashes
    
    NSAutoreleasePool * arpool = [NSAutoreleasePool new];
    
    NSAutoreleasePool * temp_pool = [NSAutoreleasePool new];
    
    NSApplication * app = [NSApplication sharedApplication];
    
    [app setDelegate:g_ui_controller];
    [app setActivationPolicy:NSApplicationActivationPolicyRegular];
    
    NSImage * application_image = [[[NSImage alloc] initWithData:[NSData dataWithBytesNoCopy:(void *)miniAudicle_png 
                                                                                length:miniAudicle_png_length
                                                                                freeWhenDone:NO]]
                                   autorelease];
    [app setApplicationIconImage:application_image];
    
    NSMenu * mainMenu = [[[NSMenu alloc] initWithTitle:@"main menu"] autorelease];
    NSMenu * chuckMenu = [[[NSMenu alloc] initWithTitle:@"chuck"] autorelease];
    
    NSMenuItem * menuItem;
    
    menuItem = [[[NSMenuItem new] initWithTitle:@"chuck" 
                                          action:NULL
                                   keyEquivalent:@""]
                autorelease];
    [menuItem setSubmenu:chuckMenu];
    [mainMenu addItem:menuItem];
    
    menuItem = [[[NSMenuItem new] initWithTitle:@"About chuck" 
                                         action:NULL
                                  keyEquivalent:@""]
                autorelease];
    [chuckMenu addItem:menuItem];
    
    [chuckMenu addItem:[NSMenuItem separatorItem]];
    
    menuItem = [[[NSMenuItem new] initWithTitle:@"Quit" 
                                         action:NULL
                                  keyEquivalent:@"q"]
                autorelease];
    [menuItem setTarget:g_ui_controller];
    [menuItem setAction:@selector(quit)];
    [chuckMenu addItem:menuItem];
    
    [app setMainMenu:mainMenu];
    
    [app activateIgnoringOtherApps:YES];
    
    [temp_pool release];
    
    m_hasStarted = true;
    m_doStart = false;
    
    [app run];
    
    [arpool release];
    
    m_hook->deactivate(m_hook);
}


void Chuck_UI_Manager::start()
{
    if( !m_hookActivated )
    {
        m_hook->activate(m_hook);
        m_hookActivated = true;
    }
    
    if( !m_hasStarted )
        m_doStart = true;
}

void Chuck_UI_Manager::shutdown()
{
    m_doShutdown = true;
    
    if( m_hasStarted || m_doStart )
    {
        NSAutoreleasePool * arpool = [NSAutoreleasePool new];
        
        [[NSApplication sharedApplication] performSelectorOnMainThread:@selector(terminate:)
                                                            withObject:NSApp
                                                         waitUntilDone:NO];
        
        [arpool release];
    }
}



@implementation Chuck_UI_Controller

- (void)quit
{
    if(g_vm) g_vm->stop(); // termination of vm will cause application exit
}

@end


