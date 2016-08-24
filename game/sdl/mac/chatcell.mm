#import "game/sdl/mac/chatcell.h"

@implementation ChatCell

#define FONT [NSFont systemFontOfSize:12]

+ (id)cellWithRect:(NSRect)rect user:(NSString *)user chat:(NSString *)chat {
    ChatCell *cell = [[ChatCell alloc] initWithFrame:rect];
    
    NSTextView *chatLabel_, *nameLabel_;
    
    chatLabel_ = [[NSTextView alloc] initWithFrame:rect];
    chatLabel_.backgroundColor = [NSColor clearColor];
    chatLabel_.font = FONT;
    [chatLabel_ setSelectable:NO];
    [chatLabel_ setString:chat];
    [cell addSubview:chatLabel_];
    
    NSSize nameLabelSize = [user sizeWithAttributes: @{NSFontAttributeName:FONT}];
    nameLabel_ = [[NSTextView alloc] initWithFrame:NSMakeRect(0, rect.size.height - nameLabelSize.height, rect.size.width, nameLabelSize.height)];
    nameLabel_.backgroundColor = [NSColor clearColor];
    nameLabel_.textColor = [NSColor blueColor];
    nameLabel_.font = FONT;
    [nameLabel_ setSelectable:NO];
    [nameLabel_ setString:user];
    [cell addSubview:nameLabel_];

    // length 0 means system message
    if ([user length] == 0) {
        chatLabel_.textColor = [NSColor grayColor];
    } else {
        [chatLabel_ setTextColor:[NSColor blackColor]];
    }

    return cell;
}
@end
