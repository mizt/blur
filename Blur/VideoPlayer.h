#import <AVFoundation/AVFoundation.h>

class VideoPlayer {
	
	private:
	
		double percentage = 0;
	
		AVPlayerItemVideoOutput *output;
		AVPlayerItem *playerItem;
		AVPlayer *player;
	
		dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
	
	public:
	
		VideoPlayer(NSString *filePath) {
			
			NSURL *fileURL = [NSURL fileURLWithPath:filePath];
			NSDictionary *options = @{(id)AVURLAssetPreferPreciseDurationAndTimingKey:@(YES)};
			AVURLAsset *asset = [AVURLAsset URLAssetWithURL:fileURL options:options];
			
			if(asset == nil) {
				NSLog(@"Error loading asset : %@", [fileURL description]);
			}
			else {
				
				[asset loadValuesAsynchronouslyForKeys:[NSArray arrayWithObject:@"tracks"] completionHandler:^{
					
					NSError *error = nil;
					AVKeyValueStatus status = [asset statusOfValueForKey:@"tracks" error:&error];
					if(status==AVKeyValueStatusLoaded) {
						NSDictionary *settings = @{(id)kCVPixelBufferPixelFormatTypeKey:[NSNumber numberWithInt:kCVPixelFormatType_32BGRA]};
						this->output = [[AVPlayerItemVideoOutput alloc] initWithPixelBufferAttributes:settings];
						this->playerItem = [AVPlayerItem playerItemWithAsset:asset];
						[this->playerItem addOutput:this->output];
						this->player = [AVPlayer playerWithPlayerItem:this->playerItem];
						[this->player setMuted:YES];
						[this->player pause];
					}
					else {
						NSLog(@"Failed to load the tracks : %ld (%@)",status,error);
					}
                    [this->player seekToTime:kCMTimeZero toleranceBefore:kCMTimeZero toleranceAfter:kCMTimeZero completionHandler:^(BOOL finished) {
                        dispatch_semaphore_signal(this->semaphore);
                    }];
				}];
				
				dispatch_semaphore_wait(this->semaphore,DISPATCH_TIME_FOREVER);
			}
		}
	
		~VideoPlayer() {
		}
	
	
		void copy(unsigned int *ptr,int w, int h,int rb) {
						
			CMTime current;
			CMTime duration;
			CVPixelBufferRef buffer = nullptr;
			
			while(true) {
				
				current = [this->output itemTimeForHostTime:CACurrentMediaTime()];
				duration = [this->playerItem duration];
				this->percentage = ((float)current.value/(float)current.timescale)/((float)duration.value/(float)duration.timescale);
				
				if(this->percentage>=1.0-(1.0/(float)duration.timescale)) {
					[this->player seekToTime:kCMTimeZero toleranceBefore:kCMTimeZero toleranceAfter:kCMTimeZero completionHandler:^(BOOL finished) {
						dispatch_semaphore_signal(this->semaphore);
					}];
					dispatch_semaphore_wait(this->semaphore,DISPATCH_TIME_FOREVER);
				}
				else {
					
                    [this->output hasNewPixelBufferForItemTime:current];
					
					buffer = [this->output copyPixelBufferForItemTime:current itemTimeForDisplay:nil];
					
					if(!buffer) {
						[this->player.currentItem stepByCount:1];
						usleep(10000.0/120.0);
					}
					else {
						break;
					}
				}
			}
			
			if(buffer) {
				
				CVPixelBufferLockBaseAddress(buffer,0);
				
				unsigned int *baseAddress = (unsigned int *)CVPixelBufferGetBaseAddress(buffer);
				size_t width = CVPixelBufferGetWidth(buffer);
				size_t height = CVPixelBufferGetHeight(buffer);
				
				if(w==width&&h==height) {
					
					for(int i=0; i<h; i++) {
						
						unsigned int *src = baseAddress+i*width;
						unsigned int *dst = ptr+i*rb;
						
						for(int j=0; j<w; j++) {
							
							unsigned int argb = *src++;
							
							*dst++ = (0xFF000000|(argb&0xFF)<<16|(argb&0xFF00)|((argb>>16)&0xFF));
							
						}
					}
					
				}
			}
			
			CVPixelBufferUnlockBaseAddress(buffer,0);
			CVPixelBufferRelease(buffer);
			
			[this->player.currentItem stepByCount:1];
			
		}
};
