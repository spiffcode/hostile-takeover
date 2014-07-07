#import "game/iphone/webviewcontroller.h"

@implementation WebViewController

- (id)init:(id<WebViewControllerDelegate>)delegate parent:(UIView *)parent {
    parent_ = parent;
    [parent_ retain];
    delegate_ = delegate;
    [delegate_ retain];
    view_ = nil;
    webView_ = nil;
    activityView_ = nil;
    backButton_ = nil;
    forwardButton_ = nil;

    [self loadView];

    return self;
}

- (void)dealloc {
    [webView_ release];
    [activityView_ release];
    [backButton_ release];
    [forwardButton_ release];
    [view_ release];
    [delegate_ release];
    [parent_ release];
    [super dealloc];
}

- (void)loadView {
    // Create parent view for web view and toolbar
    CGRect frame = CGRectMake(0, 0, parent_.frame.size.height,
            parent_.frame.size.width);
    UIView *parentView = [[[UIView alloc] initWithFrame:frame] autorelease];
    parentView.autoresizesSubviews = YES;
    parentView.autoresizingMask = UIViewAutoresizingFlexibleHeight |
            UIViewAutoresizingFlexibleWidth;

    // Create toolbar
    UIToolbar *toolbar = [[[UIToolbar alloc] initWithFrame:CGRectZero] autorelease];
    toolbar.autoresizingMask = UIViewAutoresizingFlexibleTopMargin |
            UIViewAutoresizingFlexibleWidth;
    toolbar.autoresizesSubviews = YES;
    [parentView addSubview: toolbar];

    // Add buttons to toolbar
    UIBarButtonItem *spacer0 = [[[UIBarButtonItem alloc]
            initWithBarButtonSystemItem:UIBarButtonSystemItemFixedSpace
            target:nil action:nil] autorelease];
    spacer0.width = 10;
    UIImage *backImage = [UIImage imageNamed:@"NavBackSmall.png"];
    UIButton *backButton = [UIButton buttonWithType:UIButtonTypeCustom];
    [backButton setImage:backImage forState:UIControlStateNormal];
    [backButton addTarget:self action:@selector(onBack)
            forControlEvents:UIControlEventTouchUpInside];
    backButton.enabled = NO;
    backButton.showsTouchWhenHighlighted = YES;
    UIBarButtonItem *backBarButton = [[[UIBarButtonItem alloc]
            initWithCustomView:backButton] autorelease];
    UIBarButtonItem *spacer1 = [[[UIBarButtonItem alloc]
            initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace
            target:nil action:nil] autorelease];
    UIImage *forwardImage = [UIImage imageNamed:@"NavForwardSmall.png"];
    UIButton *forwardButton = [UIButton buttonWithType:UIButtonTypeCustom];
    [forwardButton setImage:forwardImage forState:UIControlStateNormal];
    [forwardButton addTarget:self action:@selector(onForward)
            forControlEvents:UIControlEventTouchUpInside];
    forwardButton.enabled = NO;
    forwardButton.showsTouchWhenHighlighted = YES;
    UIBarButtonItem *forwardBarButton = [[[UIBarButtonItem alloc]
            initWithCustomView:forwardButton] autorelease];
    UIBarButtonItem *spacer2 = [[[UIBarButtonItem alloc]
            initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace
            target:nil action:nil] autorelease];
    UIActivityIndicatorView *activityView = [[[UIActivityIndicatorView alloc]
            initWithActivityIndicatorStyle: UIActivityIndicatorViewStyleWhite]
            autorelease];
    UIBarButtonItem *activityBarButton = [[[UIBarButtonItem alloc]
            initWithCustomView:activityView] autorelease];
    UIBarButtonItem *spacer3 = [[[UIBarButtonItem alloc]
            initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace
            target:nil action:nil] autorelease];
    UIBarButtonItem *doneBarButton = [[[UIBarButtonItem alloc]
            initWithBarButtonSystemItem:UIBarButtonSystemItemDone
            target:self action:@selector(onDone)] autorelease];
    NSArray *array = [NSArray arrayWithObjects: spacer0, backBarButton, spacer1,
            forwardBarButton, spacer2, activityBarButton, spacer3,
            doneBarButton, nil];
    [toolbar setItems:array animated:NO];
    [toolbar sizeToFit];

    // Need to set bounds for these or they won't hittest
    backButton.bounds = CGRectMake(0, 0, backImage.size.width,
            toolbar.frame.size.height);
    forwardButton.bounds = CGRectMake(0, 0, forwardImage.size.width,
            toolbar.frame.size.height);

    // Create web view
    frame.size.height -= toolbar.frame.size.height;
    UIWebView *webView = [[[UIWebView alloc] initWithFrame:frame] autorelease];
    webView.backgroundColor = [UIColor whiteColor];
    webView.scalesPageToFit = YES;
    webView.autoresizingMask = (UIViewAutoresizingFlexibleWidth |
            UIViewAutoresizingFlexibleHeight);
    webView.delegate = self;
    [parentView addSubview: webView];

    // Position toolbar
    CGRect toolbar_frame = toolbar.frame;
    CGRect parent_frame = parentView.bounds;
    toolbar_frame.origin.y = parent_frame.size.height -
            toolbar_frame.size.height;
    toolbar.frame = toolbar_frame;

    // Keep these around
    view_ = parentView;
    [view_ retain];
    webView_ = webView;
    [webView_ retain];
    activityView_ = activityView;
    [activityView_ retain];
    backButton_ = backButton;
    [backButton_ retain];
    forwardButton_ = forwardButton;
    [forwardButton_ retain];
}

- (void)show {
    [parent_ addSubview:view_];
}

- (void)onDone {
    [view_ removeFromSuperview];
    [delegate_ onDone:self];
}

- (void)loadDocument:(NSString *)url withTitle:(NSString *)title {
    NSURLRequest *req = [NSURLRequest requestWithURL:
            [NSURL URLWithString:url]];
    [webView_ loadRequest:req];
}

- (void)setDocument:(NSString *)doc {
    [webView_ loadHTMLString:doc baseURL:nil];
}

- (void)updateNavButtons {
    backButton_.enabled = webView_.canGoBack;
    forwardButton_.enabled = webView_.canGoForward;
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
    [webView_ goBack];
}

- (void)onForward {
    [webView_ goForward];
}

- (void)webView:(UIWebView *)webview didFailLoadWithError:(NSError *)error {
    [webView_ loadHTMLString:@"<html><body style=\"font-size:25px;\"><b>Timed out loading page.</b></body></html>" baseURL:nil];
}
@end
