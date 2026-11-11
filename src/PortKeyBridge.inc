// Keyboard shortcuts only. No direct-touch pointer, mouse buttons or scroll state.
static NSMutableDictionary<NSNumber *,NSNumber *> *gPortKeySources;
static NSMutableDictionary<NSNumber *,NSString *> *gPortKeyCharacters;
static NSEvent *PortKeyEvent(NSEventType type) {
    NSEvent *e=[NSEvent new];e.type=type;e.window=gWindows.lastObject;
    e.timestamp=NSProcessInfo.processInfo.systemUptime;e.modifierFlags=gModifiers|gPortModifiers;return e;
}
__attribute__((visibility("default"))) void PortPostShortcutKey(unsigned slot,unsigned short code,BOOL held,NSString *characters) {
    if(slot>1)return;
    if(!gPortKeySources){gPortKeySources=[NSMutableDictionary new];gPortKeyCharacters=[NSMutableDictionary new];}
    NSNumber *key=@(code);uint32_t old=[gPortKeySources[key] unsignedIntValue];
    uint32_t next=PortInputSources(old,20+slot,held);gPortKeySources[key]=@(next);
    if((old!=0)==(next!=0))return;
    if(held)gPortKeyCharacters[key]=characters?:@"";
    if(code==56)gPortModifiers=next?(1UL<<17):0;
    NSEvent *e=PortKeyEvent(code==56?NSEventTypeFlagsChanged:(next?NSEventTypeKeyDown:NSEventTypeKeyUp));
    e.keyCode=code;e.charactersIgnoringModifiers=gPortKeyCharacters[key]?:@"";
    e.characters=gPortModifiers?[e.charactersIgnoringModifiers uppercaseString]:e.charactersIgnoringModifiers;
    static unsigned logged;if(logged++<16)NSLog(@"PORTPAD shortcut key=%u down=%d",code,next!=0);
    ShimDispatch(e);
}
__attribute__((visibility("default"))) void PortCancelShortcutKeys(void) {
    for(NSNumber *key in gPortKeySources.allKeys) {
        if(![gPortKeySources[key] unsignedIntValue])continue;
        gPortKeySources[key]=@0;unsigned short code=key.unsignedShortValue;if(code==56)gPortModifiers=0;
        NSEvent *e=PortKeyEvent(code==56?NSEventTypeFlagsChanged:NSEventTypeKeyUp);
        e.keyCode=code;e.charactersIgnoringModifiers=gPortKeyCharacters[key]?:@"";e.characters=e.charactersIgnoringModifiers;ShimDispatch(e);
    }
    gPortModifiers=0;
}
