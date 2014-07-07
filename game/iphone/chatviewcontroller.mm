#import "game/iphone/chatviewcontroller.h"
#import "game/iphone/chatcell.h"

@implementation ChatViewController

#define TEXTFIELDWIDTH_LANDSCAPE 397
#define TEXTFIELDWIDTH_PORTRAIT 238

#define TEXTFIELDHEIGHT_LANDSCAPE 24
#define TEXTFIELDHEIGHT_PORTRAIT 29

#define TEXTFIELDFONTSIZE_LANDSCAPE 16
#define TEXTFIELDFONTSIZE_PORTRAIT 20

#define TITLELABELWIDTH_LANDSCAPE 335

#define CHAT_HISTORY 100
#define CHAT_QUEUE 100

- (id)init:(id<ChatViewControllerDelegate>)delegate parent:(UIView *)parent {
    parent_ = parent;
    [parent_ retain];
    delegate_ = delegate;
    [delegate_ retain];
    view_ = nil;
    tableView_ = nil;
    toolbar_ = nil;
    textField_ = nil;
    keyboardShown_ = false;
    chatQueue_ = [[NSMutableArray alloc] initWithCapacity:CHAT_QUEUE];
    suspended_ = NO;
    chatEntries_ = [[NSMutableArray alloc] initWithCapacity:CHAT_HISTORY];
    entryFont_ = [UIFont systemFontOfSize:12];
    CGSize size10Spaces = [@"          " sizeWithFont:entryFont_];
    width10Spaces_ = size10Spaces.width;
    chatc_ = NULL;
    title_ = @"Chat";
    [title_ retain];
    titleLabel_ = nil;
    toolbar2_ = nil;
    clear_ = YES;

    [self registerNotifications];

    [self loadView];

    return self;
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [tableView_ release];
    [toolbar_ release];
    [textField_ release];
    [chatEntries_ release];
    [chatQueue_ release];
    [title_ release];
    [toolbar2_ release];
    [titleLabel_ release];
    [view_ release];
    [delegate_ release];
    [parent_ release];

    [super dealloc];
}

- (void)testChat
{
#if 0
    [self addChat:@"it is raining today" user:@"scott"];
    [self addChat:@"it will be sunny tomorrow" user:@"valerie"];
    [self addChat:@"the lazy fox jumped over the brown cow's back and sang the pledge of allegiance more words please word wrap yes please do it now asdf asdf asdf asdf asdf asdf asdf asdf asdf asdf asdf asdf asdf asdf asdf asdf asdf asdf asdf asdf asdf asdf asdf asdf asdf asdf asdf asdf asdf asdf asdf asdf asdf asdf asdf asdf asdf" user:@"scott"];
    [self addChat:@"I don't believe you." user:@"valerie"];
#endif
}

- (void)registerNotifications
{
    [[NSNotificationCenter defaultCenter] addObserver:self
            selector:@selector(onKeyboardShown:)
            name:UIKeyboardDidShowNotification object:nil];
 
    [[NSNotificationCenter defaultCenter] addObserver:self
            selector:@selector(onKeyboardHidden:)
            name:UIKeyboardDidHideNotification object:nil];

    keyboardShown_ = false;
}

- (void)loadView
{
    // Create parent view that subviews go into
    CGRect frame = CGRectMake(0, 0, parent_.frame.size.height,
            parent_.frame.size.width);
    view_ = [[UIView alloc] initWithFrame:frame];
    view_.autoresizesSubviews = YES;
    view_.autoresizingMask = UIViewAutoresizingFlexibleHeight |
            UIViewAutoresizingFlexibleWidth;

    // Create toolbar, add to parent

    toolbar_ = [[UIToolbar alloc] initWithFrame:CGRectZero];
    toolbar_.autoresizingMask = UIViewAutoresizingFlexibleTopMargin |
            UIViewAutoresizingFlexibleWidth;
    toolbar_.autoresizesSubviews = YES;
    [view_ addSubview: toolbar_];

    // Create text field - will be added to toolbar in layout because
    // it is the only way the size custom toolbar views

    textField_ = [[UITextField alloc] initWithFrame:CGRectZero];
    textField_.borderStyle = UITextBorderStyleRoundedRect;
    textField_.textColor = [UIColor blackColor];
    textField_.placeholder = @"";
    textField_.autocorrectionType = UITextAutocorrectionTypeNo;
    //textField_.keyboardType = UIKeyboardTypeDefault;
    textField_.keyboardType = UIKeyboardTypeASCIICapable;
    textField_.returnKeyType = UIReturnKeyDefault;
    textField_.clearButtonMode = UITextFieldViewModeWhileEditing;
    textField_.delegate = self;

    // toolbar2 is the navigation header

    toolbar2_ = [[UIToolbar alloc] initWithFrame:CGRectZero];
    toolbar2_.autoresizingMask = UIViewAutoresizingFlexibleTopMargin |
            UIViewAutoresizingFlexibleWidth;
    toolbar2_.autoresizesSubviews = YES;
    [view_ addSubview: toolbar2_];

    // titleLabel that will be added to toolbar2 later

    titleLabel_ = [[UILabel alloc] initWithFrame:CGRectZero];
    titleLabel_.font = [UIFont boldSystemFontOfSize:18];
    titleLabel_.textAlignment = UITextAlignmentCenter;
    titleLabel_.textColor = [UIColor whiteColor];
    titleLabel_.backgroundColor = [UIColor clearColor];
    titleLabel_.shadowColor = [UIColor darkGrayColor];
    titleLabel_.opaque = NO;
    titleLabel_.text = title_;

    // Layout views. Repositions and resizes appropriately.

    [self layoutViews];
}

- (void)reCreateTableView {
    [tableView_ removeFromSuperview];
    [tableView_ release];
    tableView_ = nil;

    // Create tableview and add to parent

    tableView_ = [[UITableView alloc] initWithFrame:CGRectZero
            style:UITableViewStylePlain];
    tableView_.delegate = self;
    tableView_.dataSource = self;
    tableView_.separatorStyle = UITableViewCellSeparatorStyleNone;
    tableView_.autoresizesSubviews = YES;
    tableView_.autoresizingMask = UIViewAutoresizingFlexibleHeight |
            UIViewAutoresizingFlexibleWidth;
    [view_ addSubview: tableView_];
}

- (void)layoutTableView {
    CGRect rcToolbar = toolbar_.frame;
    CGRect rcToolbar2 = toolbar2_.frame;
    CGRect rcTableView = tableView_.frame;
    rcTableView.origin.y = rcToolbar2.size.height;
    rcTableView.size.height = rcToolbar.origin.y - rcTableView.origin.y;
    rcTableView.size.width = rcToolbar.size.width;
    tableView_.frame = rcTableView;
}

- (void)layoutViews
{
    // Resize the textField appropriately. Note the toolbar is a different
    // height in portrait vs. landscape. Also note, custom view button
    // bar items can only be resized by re-added them to the toolbar.

    CGRect rcTextField = textField_.frame;
    if (view_.bounds.size.width <= 320) {
        rcTextField.size.height = TEXTFIELDHEIGHT_PORTRAIT;
        rcTextField.size.width = TEXTFIELDWIDTH_PORTRAIT;
        textField_.font = [UIFont systemFontOfSize:
                TEXTFIELDFONTSIZE_PORTRAIT];
    } else {
        rcTextField.size.height = TEXTFIELDHEIGHT_LANDSCAPE;
        rcTextField.size.width = TEXTFIELDWIDTH_LANDSCAPE;
        textField_.font = [UIFont systemFontOfSize:
                TEXTFIELDFONTSIZE_LANDSCAPE];
    }
    textField_.frame = rcTextField;

    UIBarButtonItem *textFieldButton = [[[UIBarButtonItem alloc]
            initWithCustomView:textField_] autorelease];
    UIBarButtonItem *sendButton = [[[UIBarButtonItem alloc]
            initWithTitle:@"Send" style:UIBarButtonItemStyleBordered
            target:self action:@selector(onSend)] autorelease];
    NSArray *array = [NSArray arrayWithObjects: textFieldButton, sendButton,
            (char *)NULL];
    [toolbar_ setItems:array animated:NO];
    [toolbar_ sizeToFit];

    // The toolbar is taller when in portrait mode, so other things
    // need to be layed out.

    int cyToolbar = rcTextField.size.height + 8;

    CGRect rcToolbar = toolbar_.frame;
    CGRect rcParent = view_.bounds;
    rcToolbar.size.height = cyToolbar;
    rcToolbar.origin.y = rcParent.size.height - rcToolbar.size.height;
    toolbar_.frame = rcToolbar;

    // Size the label width
    [titleLabel_ sizeToFit];
    CGRect frame = titleLabel_.frame;
    frame.size.width = TITLELABELWIDTH_LANDSCAPE;
    titleLabel_.frame = frame;

    // Toolbar2 goes at the top
    UIBarButtonItem *playersButton = [[[UIBarButtonItem alloc]
            initWithTitle:@"Players"
            style:UIBarButtonItemStyleDone
            target:self action:@selector(onPlayers)] autorelease];
    UIBarButtonItem *doneButton = [[[UIBarButtonItem alloc]
            initWithTitle:@"Done"
            style:UIBarButtonItemStyleDone
            target:self action:@selector(onDone)] autorelease];
    UIBarButtonItem *titleItem = [[[UIBarButtonItem alloc]
            initWithCustomView:titleLabel_] autorelease];
    array = [NSArray arrayWithObjects: doneButton, titleItem,
            playersButton, (char *)NULL];
    [toolbar2_ setItems:array animated:NO];
    [toolbar2_ sizeToFit];

    // Toolbar2 goes at the top

    CGRect rcToolbar2 = toolbar2_.frame;
    rcToolbar2.origin.y = 0;
    rcToolbar2.size.height = cyToolbar;
    toolbar2_.frame = rcToolbar2;
}

- (void)onSend {
    if (chatc_ == NULL) {
        return;
    }

    // For some currently unknown reason, sometimes there are leading zeros
    // in this UITextField text. Remove them, and remove whitespace.

    NSString *text = textField_.text;
    int count = text.length;
    int index = 0;
    for (; index < count; index++) {
        unichar ch = [text characterAtIndex:index];
        if (ch != 0) {
            break;
        }
    }
    text = [[text substringFromIndex:index] stringByTrimmingCharactersInSet:
            [NSCharacterSet whitespaceCharacterSet]];

    if (text.length != 0) {
        NSData *data = [text
                dataUsingEncoding:[NSString defaultCStringEncoding]
                allowLossyConversion:YES];
        std::string s((const char *)[data bytes], [data length]);
        wi::ChatParams *params = new wi::ChatParams(s);
        chatc_->thread().Post(kidmOnSend, chatc_, params);
    }
    textField_.text = nil;
}

- (BOOL)textFieldShouldReturn:(UITextField *)textField {
    [self onSend];
    return NO;
}

- (void)scrollToBottom {
    [tableView_ beginUpdates];

    // Scroll to the bottom
    int index = [chatEntries_ count] - 1;
    if (index >= 0) {
        NSIndexPath *indexPath = [NSIndexPath indexPathForRow:index
                inSection:0];
        [tableView_ scrollToRowAtIndexPath:indexPath
                atScrollPosition:UITableViewScrollPositionBottom
                animated:NO];
    }

    [tableView_ endUpdates];
}

- (void)onPlayers
{
    if (chatc_ != NULL) {
        chatc_->thread().Post(kidmOnPlayers, chatc_);
    }
}

- (void)show {
    BOOL created = NO;
    if (clear_ || tableView_ == nil) {
        // Put queued up chat into the model
        suspended_ = NO;
        [self updateChatModel];

        // Create the table view. It will automagically reload the model
        [self reCreateTableView];
        [self layoutTableView];
        clear_ = NO;
        created = YES;
    }
    [parent_ addSubview:view_];

    [textField_ becomeFirstResponder];

    // Only scroll to the bottom if the table view existed already.
    // Otherwise it will crash, because data model loading is
    // asynchronous.
    if (created == NO) {
        [self scrollToBottom];
    }
}

- (void)onDone
{
    [view_ removeFromSuperview];
    if (delegate_ != nil) {
        [delegate_ onDone:self];
    }
    if (chatc_ != NULL) {
        chatc_->thread().Post(kidmOnDismissed, chatc_);
    }
}

- (void)onKeyboardShown:(NSNotification *)notification
{
    if (keyboardShown_) {
        return;
    }
    keyboardShown_ = true;

    NSDictionary *info = [notification userInfo];
    NSValue *value = [info objectForKey:UIKeyboardBoundsUserInfoKey];
    CGSize sizeKeyboard = [value CGRectValue].size;

    CGRect rcToolbar = toolbar_.frame;
    rcToolbar = CGRectOffset(rcToolbar, 0, -sizeKeyboard.height);
    toolbar_.frame = rcToolbar;

    CGRect rcTableView = tableView_.frame;
    rcTableView.size.height -= sizeKeyboard.height;
    tableView_.frame = rcTableView;

    [self scrollToBottom];
}

- (void)onKeyboardHidden:(NSNotification *)notification
{
    if (!keyboardShown_) {
        return;
    }
    keyboardShown_ = false;

    NSDictionary *info = [notification userInfo];
    NSValue *value = [info objectForKey:UIKeyboardBoundsUserInfoKey];
    CGSize sizeKeyboard = [value CGRectValue].size;

    CGRect rcToolbar = toolbar_.frame;
    rcToolbar = CGRectOffset(rcToolbar, 0, sizeKeyboard.height);
    toolbar_.frame = rcToolbar;

    CGRect rcTableView = tableView_.frame;
    rcTableView.size.height += sizeKeyboard.height;
    tableView_.frame = rcTableView;
}

#if 0
// Fires on the simulator, but not the device. Go figure.
- (void)viewDidAppear:(BOOL)animated
{
    [self scrollToBottom];
}
#endif

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView
        numberOfRowsInSection:(NSInteger)section {
    return [chatEntries_ count];
}

- (void)initializeChatCell:(int)rowIndex cell:(ChatCell *)cell
{
    CGSize size;
    size.width = view_.bounds.size.width;
    size.height = [self getRowHeight:rowIndex forWidth:size.width];

    NSMutableDictionary *entry = (NSMutableDictionary *)
            [chatEntries_ objectAtIndex:rowIndex];
    if (entry == nil) {
        [cell setup:@"error" user:@"error" size:size];
        return;
    }
    NSString *user = [entry objectForKey:@"user"];
    NSString *chat = [entry objectForKey:@"chat"];
    [cell setup:chat user:user size:size];
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:
        (NSIndexPath *)indexPath {
    ChatCell *cell = (ChatCell *)[tableView_
            dequeueReusableCellWithIdentifier:@"chatcell"];
    if (cell == nil) {
        cell = [[[ChatCell alloc] initWithFrame:CGRectZero
                reuseIdentifier:@"chatcell"] autorelease];
    }
    [self initializeChatCell:indexPath.row cell:cell];
    return cell;
}

- (CGFloat)tableView:(UITableView *)tableView
        heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
    NSMutableDictionary *entry = (NSMutableDictionary *)
            [chatEntries_ objectAtIndex:indexPath.row];
    if (entry == nil) {
        return 0;
    }
    return [self getRowHeight:indexPath.row
            forWidth:view_.bounds.size.width];
}

- (NSArray *)updateChatModel {
    NSMutableArray *indexPaths = [NSMutableArray arrayWithCapacity:10];
    while ([chatQueue_ count] != 0) {
        NSDictionary *dict = (NSDictionary *)[chatQueue_ objectAtIndex:0];
        NSString *chat = [dict objectForKey:@"chat"];
        NSString *user = [dict objectForKey:@"player"];

        if ([user length] == 0) {
            // System message.
            NSMutableDictionary *entry = [NSMutableDictionary
                    dictionaryWithObjectsAndKeys: @"", @"user",
                    chat, @"chat", (char *)NULL];
            [chatEntries_ addObject:entry];
        } else {
            // Regular chat message. Add ':' to the end of user
            NSString *newUser = [NSString stringWithFormat:@"%@:", user];

            // Insert spaces into chat so that user's name can draw into that
            // space with a different UILabel. Hack-o-rama.

            CGSize sizeUser = [newUser sizeWithFont:entryFont_];
            CGFloat ratio = sizeUser.width / width10Spaces_;
            int countSpaces = ceil(ratio * 10.0) + 2;

            char szSpaces[256];
            if (countSpaces > sizeof(szSpaces)) {
                countSpaces = sizeof(szSpaces);
            }
            memset(szSpaces, ' ', sizeof(szSpaces));
            szSpaces[countSpaces - 1] = 0;
            NSString *newChat = [NSString stringWithFormat:@"%s%@", szSpaces,
                    chat];
            NSMutableDictionary *entry = [NSMutableDictionary
                    dictionaryWithObjectsAndKeys: newUser, @"user",
                    newChat, @"chat", (char *)NULL];
            [chatEntries_ addObject:entry];
        }
        [chatQueue_ removeObjectAtIndex:0];
        NSIndexPath *indexPath = [NSIndexPath indexPathForRow:
                [chatEntries_ count] - 1 inSection:0];
        [indexPaths addObject:indexPath];
    }
    return indexPaths;
}

- (void)updateChatRows:(BOOL)scroll {
    if (suspended_ == YES) {
        return;
    }
    NSArray *indexPaths = [self updateChatModel];
    if ([indexPaths count] != 0) {
        [tableView_ beginUpdates];
        [tableView_ insertRowsAtIndexPaths:indexPaths
                withRowAnimation:UITableViewRowAnimationBottom];
        [tableView_ endUpdates];
    }
    if (scroll == YES) {
        NSIndexPath *indexPath = [NSIndexPath indexPathForRow:
                [chatEntries_ count] - 1 inSection:0];
        [tableView_ scrollToRowAtIndexPath:indexPath
                atScrollPosition:UITableViewScrollPositionBottom
                animated:NO];
    }
}

- (void)suspendUpdates:(BOOL)suspend {
    if (suspend == suspended_) {
        return;
    }
    if (suspended_ == NO & suspend == YES) {
        suspended_ = suspend;
        return;
    }
    if (suspended_ == YES && suspend == NO) {
        suspended_ = suspend;
        [self updateChatRows:NO];
    }
}

- (int)getRowHeight:(int)rowIndex forWidth:(int)width
{
    NSMutableDictionary *entry = (NSMutableDictionary *)
            [chatEntries_ objectAtIndex:rowIndex];
    if (entry == nil) {
        return 0;
    }

    // See if the height is cached; if so return it
    NSString *key = [NSString stringWithFormat:@"%d", width];
    NSString *value = [entry objectForKey:key];
    if (value != nil) {
        return [value intValue];
    }

    // Calculate what the height is, cache it, return it
    NSString *chat = (NSString *)[entry objectForKey:@"chat"];

    CGSize sizeBox = CGSizeMake(width, 2800);
    CGSize size = [chat sizeWithFont:entryFont_ constrainedToSize:sizeBox
            lineBreakMode:UILineBreakModeWordWrap];
    value = [NSString stringWithFormat:@"%d", (int)size.height];
    [entry setObject:value forKey:key];
    return size.height;
}

- (void)doClear
{
    [chatEntries_ removeAllObjects];
    [chatQueue_ removeAllObjects];

    // Clear occurs when switching between chat contexts (room, game, etc).
    // It would be nice to be able to call reloadData here. Unfortunately,
    // it causes "stuck chat cells" when the TableView isn't visible. As
    // a hack workaround, destroy the tableView_ and re-created it when
    // showing. During this period, queue up chat.
    [tableView_ removeFromSuperview];
    [tableView_ release];
    tableView_ = nil;
    clear_ = YES;
    [self suspendUpdates:YES];
}

- (void)doAddChat:(NSDictionary *)dict
{
    [chatQueue_ addObject:dict];
    [self updateChatRows:YES];
}

- (void)doShow:(UINavigationController *)vcwi
{
    [self show];
    titleLabel_.text = title_;
}

- (void)doHide
{
    [self onDone];
}

- (void)doSetTitle:(NSString *)title
{
    [title_ release];
    title_ = title;
    [title_ retain];
}

- (void)setController:(wi::ChatController *)chatc
{
    chatc_ = chatc;
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)way {
    return way == UIInterfaceOrientationLandscapeRight;
}

// UIScrollViewDelegate
// Inserting items while scrolling causes UITableView to crash. Track
// when scrolling happens an cache inserts until scrolling stops.

- (BOOL)scrollViewShouldScrollToTop:(UIScrollView *)scrollView {
    [self suspendUpdates:YES];
    return YES;
}

- (void)scrollViewDidScrollToTop:(UIScrollView *)scrollView {
    [self suspendUpdates:NO];
}

- (void)scrollViewWillBeginDragging:(UIScrollView *)scrollView {
    [self suspendUpdates:YES];
}

- (void)scrollViewDidEndDragging:(UIScrollView *)scrollView
        willDecelerate:(BOOL)decelerate {
    if (decelerate == NO) {
        [self suspendUpdates:NO];
    }
}

- (void)scrollViewDidEndDecelerating:(UIScrollView *)scrollView {
    [self suspendUpdates:NO];
}
@end

// ++++
// This is the game thread callable class for controlling chat

namespace wi {

// These get called on the game thread, and do the real work on the main
// thread.

ChatController::ChatController(WiViewController *vcwi,
        ChatViewController *vcchat) : vcwi_(vcwi), vcchat_(vcchat),
        pcccb_(NULL) {
    [vcchat_ setController: this];
}

void ChatController::Clear() {
    [vcchat_ performSelectorOnMainThread:@selector(doClear)
            withObject:nil waitUntilDone: NO];
}

void ChatController::AddChat(const char *player, const char *chat) {
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSString *player_s = [NSString stringWithCString:player
            encoding:[NSString defaultCStringEncoding]];
    NSString *chat_s = [NSString stringWithCString:chat
            encoding:[NSString defaultCStringEncoding]];
    NSDictionary *dict = [NSDictionary dictionaryWithObjectsAndKeys:
            player_s, @"player", chat_s, @"chat", (char *)NULL];
    [vcchat_ performSelectorOnMainThread:@selector(doAddChat:)
            withObject:dict waitUntilDone: NO];
    [pool release];
}

void ChatController::Show(bool fShow) {
    if (fShow) {
        [vcchat_ performSelectorOnMainThread:@selector(doShow:)
                withObject:vcwi_ waitUntilDone: NO];
    } else {
        [vcchat_ performSelectorOnMainThread:@selector(doHide)
                withObject:nil waitUntilDone: NO];
    }
}

void ChatController::SetTitle(const char *title) {
    title_ = title;
    NSString *title_s = [NSString stringWithCString:title
            encoding:[NSString defaultCStringEncoding]];
    [vcchat_ performSelectorOnMainThread:@selector(doSetTitle:)
            withObject:title_s waitUntilDone: NO];
}

const char *ChatController::GetTitle() {
    return title_.c_str();
}

IChatControllerCallback *ChatController::SetCallback(
        IChatControllerCallback *pcccb) {
    IChatControllerCallback *old = pcccb_;
    pcccb_ = pcccb;
    return old;
}

void ChatController::OnMessage(base::Message *pmsg) {
    switch (pmsg->id) {
    case kidmOnDismissed:
        [vcwi_ becomeFirstResponder];
        if (pcccb_ != NULL) {
            pcccb_->OnChatDismissed();
        }
        break;

    case kidmOnSend:
        if (pcccb_ != NULL) {
            ChatParams *params = (ChatParams *)pmsg->data;
            pcccb_->OnChatSend(params->chat.c_str());
            delete params;
        }
        break;

    case kidmOnPlayers:
        if (pcccb_ != NULL) {
            pcccb_->OnPlayers();
        }
        break;
    }
}

} // namespace wi
