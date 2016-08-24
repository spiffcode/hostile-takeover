#import "game/sdl/ios/webview.h"

@implementation Webview

- (id)init {
    if (!(self == [super init])) {
        return nil;
    }
    // Setup view
    self.frame = CGRectMake(0, 0, 0, 0);
    self.autoresizesSubviews = YES;
    self.autoresizingMask = UIViewAutoresizingFlexibleHeight | UIViewAutoresizingFlexibleWidth;

    // Setup content view
    contentView_ = [[UIWebView alloc] init];
    [contentView_ setDelegate:self];
    [self addSubview:contentView_];
    
    [self layoutViews];
    
    return self;
}

- (void)dealloc {
    activityView_ = nil;
    backButton_ = nil;
    forwardButton_ = nil;
}

- (void)layoutViews {
    [self createButtons];
    [self createToolbar];

    // Need to set bounds for these or they won't hittest
    UIImage *backImage = [UIImage imageNamed:@"NavBackSmall.png"];
    UIImage *forwardImage = [UIImage imageNamed:@"NavForwardSmall.png"];
    backButton_.bounds = CGRectMake(0, 0, backImage.size.width, toolbar_.frame.size.height);
    forwardButton_.bounds = CGRectMake(0, 0, forwardImage.size.width, toolbar_.frame.size.height);

    // Position toolbar
    CGRect toolbar_frame = toolbar_.frame;
    CGRect parent_frame = self.bounds;
    toolbar_frame.origin.y = parent_frame.size.height -
            toolbar_frame.size.height;
    toolbar_.frame = toolbar_frame;

    [contentView_ setFrame:CGRectMake(0, 0, self.frame.size.width, self.frame.size.height - toolbar_.frame.size.height)];
}

- (void)createToolbar {
    // Create toolbar
    UIToolbar *toolbar = [[UIToolbar alloc] initWithFrame:CGRectZero];
    toolbar.autoresizingMask = UIViewAutoresizingFlexibleTopMargin |
            UIViewAutoresizingFlexibleWidth;
    toolbar.autoresizesSubviews = YES;
    [toolbar setBackgroundColor:[UIColor lightGrayColor]];
    [toolbar setBarStyle:UIBarStyleDefault];
    [toolbar setAlpha:1.0f];

    // Add buttons to toolbar
    UIBarButtonItem *spacer0 = [[UIBarButtonItem alloc]
            initWithBarButtonSystemItem:UIBarButtonSystemItemFixedSpace
            target:nil action:nil];
    spacer0.width = 10;
    UIBarButtonItem *backBarButton = [[UIBarButtonItem alloc]
            initWithCustomView:backButton_];
    UIBarButtonItem *spacer1 = [[UIBarButtonItem alloc]
            initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace
            target:nil action:nil];
    UIBarButtonItem *forwardBarButton = [[UIBarButtonItem alloc]
            initWithCustomView:forwardButton_];
    UIBarButtonItem *spacer2 = [[UIBarButtonItem alloc]
            initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace
            target:nil action:nil];
    UIBarButtonItem *activityBarButton = [[UIBarButtonItem alloc]
            initWithCustomView:activityView_];
    UIBarButtonItem *spacer3 = [[UIBarButtonItem alloc]
            initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace
            target:nil action:nil];
    UIBarButtonItem *doneBarButton = [[UIBarButtonItem alloc]
            initWithBarButtonSystemItem:UIBarButtonSystemItemDone
            target:self action:@selector(onDone)];
    NSArray *array = [NSArray arrayWithObjects: spacer0, backBarButton, spacer1,
            forwardBarButton, spacer2, activityBarButton, spacer3,
            doneBarButton, nil];
    [toolbar setItems:array animated:NO];
    [toolbar sizeToFit];

    // Set the new toolbar
    [toolbar_ removeFromSuperview];
    toolbar_ = toolbar;
    [self addSubview:toolbar_];
}

- (void)createButtons {
    UIImage *backImage = [UIImage imageNamed:@"NavBackSmall.png"];
    UIButton *backButton = [UIButton buttonWithType:UIButtonTypeCustom];
    [backButton setImage:backImage forState:UIControlStateNormal];
    [backButton addTarget:self action:@selector(onBack)
            forControlEvents:UIControlEventTouchUpInside];
    backButton.enabled = NO;
    backButton.showsTouchWhenHighlighted = YES;

    UIImage *forwardImage = [UIImage imageNamed:@"NavForwardSmall.png"];
    UIButton *forwardButton = [UIButton buttonWithType:UIButtonTypeCustom];
    [forwardButton setImage:forwardImage forState:UIControlStateNormal];
    [forwardButton addTarget:self action:@selector(onForward)
            forControlEvents:UIControlEventTouchUpInside];
    forwardButton.enabled = NO;
    forwardButton.showsTouchWhenHighlighted = YES;

    UIActivityIndicatorView *activityView = [[UIActivityIndicatorView alloc]
            initWithActivityIndicatorStyle: UIActivityIndicatorViewStyleWhite];

    backButton_ = backButton;
    forwardButton_ = forwardButton;
    activityView_ = activityView;
}

- (void)setFrame:(CGRect)frame {
    [super setFrame:frame];
    [self layoutViews];
}

- (void)setupViewWithURL:(NSString *)url title:(NSString *)title {
    [self loadDocument:url withTitle:title];
}

- (void)onDone {
    [self removeFromSuperview];
}

- (void)loadDocument:(NSString *)url withTitle:(NSString *)title {
    NSURLRequest *req = [NSURLRequest requestWithURL:
            [NSURL URLWithString:url]];
    [contentView_ loadRequest:req];
}

- (void)setDocument:(NSString *)doc {
    [contentView_ loadHTMLString:doc baseURL:nil];
}

- (void)updateNavButtons {
    backButton_.enabled = contentView_.canGoBack;
    forwardButton_.enabled = contentView_.canGoForward;
}

- (void)webViewDidStartLoad:(UIWebView *)webview {
    [activityView_ startAnimating];
    [self updateNavButtons];
}

- (void)webViewDidFinishLoad:(UIWebView *)webview {
    [activityView_ stopAnimating];
    [self updateNavButtons];
}

- (void)onBack {
    [contentView_ goBack];
}

- (void)onForward {
    [contentView_ goForward];
}

- (void)webView:(UIWebView *)webview didFailLoadWithError:(NSError *)error {
    [contentView_ loadHTMLString:[NSString stringWithFormat:@"<html><body style=\"font-size:25px;\"><b>Error loading the page:<br>%@</b></body></html>", [error localizedDescription]] baseURL:nil];
}
@end
