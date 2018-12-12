//#import "MetalView.h"
#import "MetalLayer.h"

@interface MetalView:NSView
    -(id)init:(MetalLayer *)ml;
@end
@implementation MetalView
    +(Class)layerClass { return [CAMetalLayer class]; }
    -(BOOL)wantsUpdateLayer { return YES; }
    -(void)updateLayer { [super updateLayer]; }
    -(id)init:(MetalLayer *)ml {
        self = [super initWithFrame:ml->frame()];
        if(self) {
            self.layer = ml->layer();
            self.wantsLayer = YES;
        }
        return self;
    }
@end

class Metal {
  
    private:
    
        NSWindow *win;
        MetalView *metalview;
    
        MetalLayer *horizontalBlur;
        MetalLayer *verticalBlur;
    
        dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
    
    public:
    
        Metal() {
        
            this->win = [[NSWindow alloc] initWithContentRect:CGRectMake(0,0,W,H) styleMask:1 backing:NSBackingStoreBuffered defer:NO];
            
            this->horizontalBlur = new MetalLayer(@"default",@"horizontalBlur",CGRectMake(0,0,W,H),true);
            this->verticalBlur = new MetalLayer(@"default",@"verticalBlur",CGRectMake(0,0,W,H),false);
            this->metalview = [[MetalView alloc] init:this->verticalBlur];
            
            [[this->win contentView] addSubview:this->metalview];
            [this->win center];
        }
        
        void render(unsigned int *buffer) {
                        
            this->horizontalBlur->cleanup();
            [this->horizontalBlur->texture() replaceRegion:MTLRegionMake2D(0,0,W,H) mipmapLevel:0 withBytes:buffer bytesPerRow:W<<2];
            this->horizontalBlur->update(^(id<MTLCommandBuffer> commandBuffer){
                 dispatch_semaphore_signal(this->semaphore);
            });
            
            dispatch_semaphore_wait(this->semaphore,DISPATCH_TIME_FOREVER);
            
            this->verticalBlur->cleanup();
            this->verticalBlur->texture(this->horizontalBlur->drawableTexture());
            this->verticalBlur->update();
            
            static dispatch_once_t predicate;
            dispatch_once(&predicate,^{
                dispatch_async(dispatch_get_main_queue(),^{
                    [this->win makeKeyAndOrderFront:nil];
                });
            });
        }
    
        ~Metal() {
            [this->metalview removeFromSuperview];
        }
};
