#import "game/iphone/chatcell.h"

@implementation ChatCell
- (id)initWithFrame:(CGRect)frame reuseIdentifier:(NSString *)reuseIdentifier {
    self = [super initWithFrame:frame reuseIdentifier:reuseIdentifier];
    if (self == nil) {
        return nil;
    }

    self.selectionStyle = UITableViewCellSelectionStyleNone;

    chatLabel_ = [[UILabel alloc] initWithFrame:CGRectZero];
    chatLabel_.backgroundColor = [UIColor whiteColor];
    chatLabel_.opaque = YES;
    chatLabel_.font = [UIFont systemFontOfSize:12];
    chatLabel_.lineBreakMode = UILineBreakModeWordWrap;
    chatLabel_.numberOfLines = 0;

    [self addSubview:chatLabel_];

    nameLabel_ = [[UILabel alloc] initWithFrame:CGRectZero];
    nameLabel_.backgroundColor = [UIColor whiteColor];
    nameLabel_.opaque = YES;
    nameLabel_.textColor = [UIColor blueColor];
    nameLabel_.font = [UIFont systemFontOfSize:12];
    nameLabel_.lineBreakMode = UILineBreakModeWordWrap;
    nameLabel_.numberOfLines = 0;

    [self addSubview:nameLabel_];

    return self;
}

- (void)setup:(NSString *)chat user:(NSString *)user size:(CGSize)size
{
    chatLabel_.text = chat;
    CGRect frame = chatLabel_.frame;
    frame.size = size;
    chatLabel_.frame = frame;
    nameLabel_.text = user;
    frame = nameLabel_.frame;
    frame.size = [user sizeWithFont:nameLabel_.font];
    nameLabel_.frame  = frame;

    // length 0 means system message
    if ([user length] == 0) {
        chatLabel_.textColor = [UIColor grayColor];
    } else {
        chatLabel_.textColor = [UIColor blackColor];
    }

    // Invalidate the cached image
    [self setNeedsLayout];
    [self setNeedsDisplay];
}
@end
