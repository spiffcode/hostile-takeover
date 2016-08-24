#import <UIKit/UIKit.h>
#import "game/sdl/ios/chatview.h"
#import "game/sdl/ios/chatcell.h"
#import "game/sdl/ios/iphone.h"

@implementation ChatView

#define TEXTFIELDWIDTH_LANDSCAPE self.bounds.size.width - 81
// #define TEXTFIELDWIDTH_PORTRAIT 238

#define TEXTFIELDHEIGHT_LANDSCAPE 24
// #define TEXTFIELDHEIGHT_PORTRAIT 29

#define TEXTFIELDFONTSIZE_LANDSCAPE 16
// #define TEXTFIELDFONTSIZE_PORTRAIT 20

#define TITLELABELWIDTH_LANDSCAPE 100

#define CHAT_HISTORY 100
#define CHAT_QUEUE 100

#define DEVICE_OS [[[UIDevice currentDevice] systemVersion] intValue]

- (id)init {
    if (!(self == [super init])) {
        return nil;
    }
    tableView_ = nil;
    toolbar_ = nil;
    textField_ = nil;
    chatQueue_ = [[NSMutableArray alloc] initWithCapacity:CHAT_QUEUE];
    suspended_ = NO;
    chatEntries_ = [[NSMutableArray alloc] initWithCapacity:CHAT_HISTORY];
    entryFont_ = [UIFont systemFontOfSize:12];
    CGSize size10Spaces = [@"          " sizeWithFont:entryFont_];
    width10Spaces_ = size10Spaces.width;
    chatc_ = NULL;
    title_ = @"Chat";
    titleLabel_ = nil;
    toolbar2_ = nil;
    clear_ = YES;

    [self loadView];

    return self;
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)registerNotifications
{
    [[NSNotificationCenter defaultCenter] addObserver:self
        selector:@selector(keyboardShown:)
        name:UIKeyboardDidShowNotification
        object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
        selector:@selector(keyboardHidden:)
        name:UIKeyboardDidHideNotification
        object:nil];

    [[NSNotificationCenter defaultCenter] addObserver:self
        selector:@selector(keyboardWillChangeFrame:)
        name:UIKeyboardWillChangeFrameNotification
        object:nil];
}

- (void)loadView
{
    // Create parent view that subviews go into
    CGRect frame = CGRectMake(0, 0, [UIScreen mainScreen].bounds.size.width, [UIScreen mainScreen].bounds.size.height);
    [self setFrame:frame];
    self.autoresizesSubviews = YES;
    self.autoresizingMask = UIViewAutoresizingFlexibleHeight | UIViewAutoresizingFlexibleWidth;

    // Create toolbar, add to parent

    toolbar_ = [[UIToolbar alloc] initWithFrame:CGRectZero];
    toolbar_.autoresizingMask = UIViewAutoresizingFlexibleTopMargin |
            UIViewAutoresizingFlexibleWidth;
    toolbar_.autoresizesSubviews = YES;
    toolbar_.backgroundColor = [UIColor darkGrayColor];
    [self addSubview: toolbar_];

    // Create text field - will be added to toolbar in layout because
    // it is the only way the size custom toolbar views

    textField_ = [[UITextField alloc] initWithFrame:CGRectZero];
    textField_.borderStyle = UITextBorderStyleRoundedRect;
    textField_.textColor = [UIColor blackColor];
    textField_.placeholder = @"";
    textField_.autocorrectionType = UITextAutocorrectionTypeDefault;
    textField_.keyboardType = UIKeyboardTypeDefault;
    textField_.returnKeyType = UIReturnKeySend;
    textField_.clearButtonMode = UITextFieldViewModeWhileEditing;
    textField_.delegate = self;

    // toolbar2 is the navigation header

    toolbar2_ = [[UIToolbar alloc] initWithFrame:CGRectZero];
    toolbar2_.autoresizingMask = UIViewAutoresizingFlexibleTopMargin |
            UIViewAutoresizingFlexibleWidth;
    toolbar2_.autoresizesSubviews = YES;
    toolbar2_.backgroundColor = [UIColor darkGrayColor];
    [self addSubview: toolbar2_];

    // titleLabel that will be added to toolbar2 later

    titleLabel_ = [[UILabel alloc] initWithFrame:CGRectZero];
    titleLabel_.font = [UIFont boldSystemFontOfSize:18];
    titleLabel_.textAlignment = NSTextAlignmentCenter;
    titleLabel_.textColor = [UIColor whiteColor];
    titleLabel_.backgroundColor = [UIColor clearColor];
    //titleLabel_.shadowColor = [UIColor darkGrayColor];
    titleLabel_.opaque = NO;
    titleLabel_.text = title_;

    // Layout views. Repositions and resizes appropriately.

    [self layoutViews];
    [self registerNotifications];
}

- (void)reCreateTableView {
    [tableView_ removeFromSuperview];
    tableView_ = nil;

    // Create tableview and add to parent

    tableView_ = [[UITableView alloc] initWithFrame:CGRectZero style:UITableViewStylePlain];
    tableView_.delegate = self;
    tableView_.dataSource = self;
    tableView_.autoresizesSubviews = YES;
    tableView_.autoresizingMask = UIViewAutoresizingFlexibleHeight | UIViewAutoresizingFlexibleWidth;
    tableView_.separatorStyle = UITableViewCellSeparatorStyleNone;
    [self layoutTableView];
    [self addSubview: tableView_];
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
    rcTextField.size.height = TEXTFIELDHEIGHT_LANDSCAPE;
    rcTextField.size.width = TEXTFIELDWIDTH_LANDSCAPE;
    textField_.font = [UIFont systemFontOfSize: TEXTFIELDFONTSIZE_LANDSCAPE];
    textField_.frame = rcTextField;

    UIBarButtonItem *textFieldButton = [[UIBarButtonItem alloc]
            initWithCustomView:textField_];
    UIBarButtonItem *sendButton = [[UIBarButtonItem alloc]
            initWithTitle:@"Send" style:UIBarButtonItemStyleBordered
            target:self action:@selector(onSend)];
    NSArray *array = [NSArray arrayWithObjects: textFieldButton, sendButton,
            (char *)NULL];
    [toolbar_ setItems:nil];
    [toolbar_ setItems:array animated:NO];
    [toolbar_ sizeToFit];

    // The toolbar is taller when in portrait mode, so other things
    // need to be layed out.

    int cyToolbar = rcTextField.size.height + 8;

    CGRect rcToolbar = toolbar_.frame;
    CGRect rcParent = self.bounds;
    rcToolbar.size.height = cyToolbar;
    rcToolbar.origin.y = rcParent.size.height - rcToolbar.size.height;
    toolbar_.frame = rcToolbar;

    // Size the label width
    [titleLabel_ sizeToFit];
    CGRect frame = titleLabel_.frame;
    frame.size.width = TITLELABELWIDTH_LANDSCAPE;
    titleLabel_.frame = frame;

    // Toolbar2 goes at the top
    UIBarButtonItem *playersButton = [[UIBarButtonItem alloc]
            initWithTitle:@"Players"
            style:UIBarButtonItemStyleDone
            target:self action:@selector(onPlayers)];
    UIBarButtonItem *doneButton = [[UIBarButtonItem alloc]
            initWithTitle:@"Done"
            style:UIBarButtonItemStyleDone
            target:self action:@selector(onDone)];
    keyboardButton_ = [[UIBarButtonItem alloc]
            initWithTitle:@"Keyboard"
            style:UIBarButtonItemStyleDone
            target:self action:@selector(onKeyboard)];
            [keyboardButton_ setEnabled:NO];
    UIBarButtonItem *titleItem = [[UIBarButtonItem alloc]
            initWithCustomView:titleLabel_];
    UIBarButtonItem *sizingItem = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace target:self action:nil];
    array = [NSArray arrayWithObjects: doneButton, sizingItem, titleItem, sizingItem,
            keyboardButton_, playersButton, (char *)NULL];
    [toolbar2_ setItems:nil];
    [toolbar2_ setItems:array animated:NO];
    [toolbar2_ sizeToFit];

    // Toolbar2 goes at the top

    CGRect rcToolbar2 = toolbar2_.frame;
    rcToolbar2.origin.y = 0;
    rcToolbar2.size.height = cyToolbar;
    toolbar2_.frame = rcToolbar2;

    // TableView
    //[self layoutTableView];
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
        clear_ = NO;
        created = YES;
    }

    [IPhone presentView:self];
    [tableView_ setSeparatorStyle:UITableViewCellSeparatorStyleNone];

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
    [textField_ resignFirstResponder];
    [self removeFromSuperview];
    
    if (chatc_ != NULL) {
        chatc_->thread().Post(kidmOnDismissed, chatc_);
    }
}

- (void)onKeyboard
{
    [keyboardButton_ setEnabled:NO];
    if (textField_.isFirstResponder) {
        [textField_ resignFirstResponder];
    } else {
        [textField_ becomeFirstResponder];
    }
    
}

- (void)keyboardShown:(NSNotification *)notification {
    if (!self.superview) {
        return;
    }
    [self scrollToBottom];
    [keyboardButton_ setEnabled:YES];
}

- (void)keyboardHidden:(NSNotification *)notification {
    if (!self.superview) {
        return;
    }
    [self scrollToBottom];
    [keyboardButton_ setEnabled:YES];
}

- (void)keyboardWillChangeFrame:(NSNotification *)notification {
    NSDictionary *info = [notification userInfo];
    CGRect keyboardFrameEnd = [[info objectForKey:UIKeyboardFrameEndUserInfoKey] CGRectValue];
    // Move the toolbar
    CGRect rcToolbar = [toolbar_ frame];
    if (DEVICE_OS <= 7) {
        // Don't ask... you dont even want to know about this calculation...
        // The keyboardFrameEnd rect is based on portrait, but self.frame and its subviews' frames
        // are landscape. So calculating there toolbar_ should be based on keyboardFrameEnd is a pain.
        // Lets hope we don't ever need to modify/maintain this <= iOS 7 code.
        if ([[UIApplication sharedApplication] statusBarOrientation] == UIDeviceOrientationLandscapeRight) {
            // Strangely enough, this seems to fire when my device is physically in landscapeLeft mode
            rcToolbar.origin.y = keyboardFrameEnd.origin.x - toolbar_.frame.size.height;
        }
        if ([[UIApplication sharedApplication] statusBarOrientation] == UIDeviceOrientationLandscapeLeft) {
            // Strangely enough, this seems to fire when my device is physically in landscapeRight mode
            rcToolbar.origin.y = self.frame.size.height - keyboardFrameEnd.origin.x - keyboardFrameEnd.size.width - toolbar_.frame.size.height;
        }
    } else { // DEVICE_OS <= 8
        rcToolbar.origin.y = keyboardFrameEnd.origin.y - rcToolbar.size.height;
    }
    [toolbar_ setFrame:rcToolbar];
    [self bringSubviewToFront:toolbar_];

    // Resize the table view
    [self layoutTableView];
}

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
    size.width = self.bounds.size.width;
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
    ChatCell *cell = (ChatCell *)[tableView_ dequeueReusableCellWithIdentifier:@"chatcell"];
    if (cell == nil) {
        cell = [[ChatCell alloc] initWithFrame:CGRectZero reuseIdentifier:@"chatcell"];
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
    return [self getRowHeight:indexPath.row forWidth:self.bounds.size.width];
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

- (void)updateChatRows {
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
        [self updateChatRows];
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
            lineBreakMode:NSLineBreakByWordWrapping];
    value = [NSString stringWithFormat:@"%d", (int)ceilf(size.height)];
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
    tableView_ = nil;
    clear_ = YES;
    [self suspendUpdates:YES];
}

- (void)doAddChat:(NSDictionary *)dict
{
    [chatQueue_ addObject:dict];
    [self updateChatRows];
    [self scrollToBottom];
}

- (void)doShow
{
    [self show];
    titleLabel_.text = title_;
}

- (void)doHide
{
    if ([self superview])
        [self onDone];
}

- (void)doSetTitle:(NSString *)title
{
    title_ = title;
}

- (void)setController:(wi::ChatController *)chatc
{
    chatc_ = chatc;
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)way {
    return UIInterfaceOrientationLandscapeRight;
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

- (void)setFrame:(CGRect)frame {
    [super setFrame:frame];
    [self layoutViews];
    [self layoutTableView];
}
@end

// ++++
// This is the game thread callable class for controlling chat

namespace wi {

// These get called on the game thread, and do the real work on the main
// thread.

ChatController::ChatController(ChatView *vcchat) {
    vcchat_ = vcchat;
    [vcchat_ setController: this];
}

void ChatController::Clear() {
    [vcchat_ performSelectorOnMainThread:@selector(doClear)
            withObject:nil waitUntilDone: NO];
}

void ChatController::AddChat(const char *player, const char *chat) {
    NSString *player_s = [NSString stringWithCString:player
            encoding:[NSString defaultCStringEncoding]];
    NSString *chat_s = [NSString stringWithCString:chat
            encoding:[NSString defaultCStringEncoding]];
    NSDictionary *dict = [NSDictionary dictionaryWithObjectsAndKeys:
            player_s, @"player", chat_s, @"chat", (char *)NULL];
    [vcchat_ performSelectorOnMainThread:@selector(doAddChat:)
            withObject:dict waitUntilDone: NO];
}

void ChatController::Show(bool fShow) {
    if (fShow) {
        [vcchat_ performSelectorOnMainThread:@selector(doShow) withObject:nil waitUntilDone: NO];
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
