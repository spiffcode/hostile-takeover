#import "game/iphone/chatview.h"

@implementation ChatView

- (id)initWithFrame:(CGRect)rect {

    rect_ = CGRectMake(0, 0, rect.size.height, rect.size.width);

    self = [super initWithFrame:rect_];
    if (self == nil) {
        return nil;
    }

#if 0
    // Rotate this view
    self.center = CGPointMake(rect.size.height / 2, rect.size.width / 2);
    CGAffineTransform transform = self.transform;
    self.transform = CGAffineTransformRotate(transform, (M_PI / 2.0));
#endif

#if 0
    CGRect rcNav = CGRectMake(0, 0, rect_.size.width, 48);
    navBar_ = [[UINavigationBar alloc] initWithFrame: rcNav];
    [navBar_ setDelegate: self];
    navItem_ = [[UINavigationItem alloc] initWithTitle:@"Chat: <room>"];
    [navBar_ pushNavigationItem: navItem_ animated:NO];
    //[navBar_ showButtonsWithRightTitle:@"" rightTitle:@"Dismiss" leftBack: NO];

    [self addSubview: navBar_];
#endif

    //CGRect rcField;
    //textField_ = [[UITextField alloc] initWithFrame:rcField];

    return self;
}

- (void)dealloc {
    //[navBar_ release];
    //[textField_ release];
    [super dealloc];
}

- (void)touchesEnded:(NSSet*)touches withEvent:(UIEvent*)event {

}
@end
