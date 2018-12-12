#import <Cocoa/Cocoa.h>

#define FrameRate 30
#define W 1280
#define H 720

#import "VideoPlayer.h"
#import "Metal.h"

class App {
    
    private:
    
        dispatch_source_t timer;
    
        Metal *metal = nullptr;
        VideoPlayer *videoplayer = nullptr;
    
        unsigned int *buffer = nullptr;
    
        void initialize(NSString *fileURL) {
            
            this->buffer = new unsigned int[W*H];
            
            this->videoplayer = new VideoPlayer(fileURL);
            this->metal = new Metal();
            
            this->timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER,0,0,dispatch_queue_create("setInterval",0));
            dispatch_source_set_timer(this->timer,dispatch_time(0,0),(1.0/FrameRate)*1000000000,0);
            dispatch_source_set_event_handler(this->timer,^(){
                
                this->videoplayer->copy(this->buffer,W,H,W);
                this->metal->render(this->buffer);
                
            });
            if(this->timer) dispatch_resume(this->timer);
        }
    
    public:
    
        App() {
            
            NSString *debugURL = [[NSBundle mainBundle] pathForResource:@"trailer_720p" ofType:@"mov"];
            
            if([[NSFileManager defaultManager] fileExistsAtPath:debugURL]) {
                this->initialize(debugURL);
            }
            
        }
    
        ~App() {
            
            if(this->timer) {
                dispatch_source_cancel(this->timer);
                this->timer = nullptr;
            }
            
            usleep(100000);
            
            if(this->videoplayer) delete this->videoplayer;
            if(this->metal) delete this->metal;
            if(this->buffer) delete[] this->buffer;
        }
};

@interface AppDelegate:NSObject <NSApplicationDelegate> {
    App *m;
}
@end

@implementation AppDelegate
-(void)applicationDidFinishLaunching:(NSNotification*)aNotification {
    m = new App();
}
-(void)applicationWillTerminate:(NSNotification *)aNotification {
    if(m) delete m;
}
@end

int main (int argc, const char * argv[]) {
    @autoreleasepool {
        srand(CFAbsoluteTimeGetCurrent());
        srandom(CFAbsoluteTimeGetCurrent());
        id app = [NSApplication sharedApplication];
        id delegat = [AppDelegate alloc];
        [app setDelegate:delegat];
        
        id menu = [[NSMenu alloc] init];
        id rootMenuItem = [[NSMenuItem alloc] init];
        [menu addItem:rootMenuItem];
        id appMenu = [[NSMenu alloc] init];
        id quitMenuItem = [[NSMenuItem alloc] initWithTitle:@"Quit" action:@selector(terminate:) keyEquivalent:@"q"];
        [appMenu addItem:quitMenuItem];
        [rootMenuItem setSubmenu:appMenu];
        [NSApp setMainMenu:menu];
        [app run];
        return 0;
    }
}
