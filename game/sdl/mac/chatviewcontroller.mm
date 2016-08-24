#import "game/sdl/mac/chatViewController.h"
#import "game/sdl/mac/chatcell.h"

@implementation ChatViewController

#define TEXTFIELDWIDTH_LANDSCAPE view_.bounds.size.width - 81
#define TEXTFIELDHEIGHT_LANDSCAPE 24
#define TITLELABELWIDTH_LANDSCAPE 100
#define TOOLBAR_HEIGHT 30

#define CHAT_HISTORY 100
#define CHAT_QUEUE 100

- (id)init {
    wi::HostHelpers::GetSurfaceProperties(&pprops_);
    
    NSRect windowFrame = NSMakeRect(window_.screen.frame.size.width/2 - pprops_.cxWidth/2, window_.screen.frame.size.height/2 - pprops_.cyHeight/2, pprops_.cxWidth, pprops_.cyHeight);
    window_ = [[NSWindow alloc] initWithContentRect:windowFrame
                    styleMask:NSTitledWindowMask
                    backing:NSBackingStoreBuffered
                    defer:YES];
    
    view_ = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, pprops_.cxWidth, pprops_.cyHeight)];
    tableView_ = nil;
    toolbar2_ = nil;
    toolbar_ = nil;
    chatEntries_ = [[NSMutableArray alloc] initWithCapacity:CHAT_HISTORY];
    userEntries_ = [[NSMutableArray alloc] initWithCapacity:CHAT_HISTORY];
    title_ = @"Chat";
    titleLabel_ = nil;
    textField_ = nil;
    entryFont_ = [NSFont systemFontOfSize:12];
    NSSize size10Spaces = [@"          " sizeWithAttributes: @{NSFontAttributeName:entryFont_}];
    width10Spaces_ = size10Spaces.width;
    chatc_ = NULL;    clear_ = YES;
    
    [self loadView];

    return self;
}

- (void)loadView {
    // Window
    [window_ setMovable:YES];
    [window_ setLevel:NSNormalWindowLevel];
    [window_.contentView addSubview:view_];
    [window_ setTitle:@"Hostile Takeover - Chat"];
    
    // View
    [view_ setWantsLayer:YES];
    
    // Toolbar (bottom toolbar)
    toolbar_ = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, view_.frame.size.width, TOOLBAR_HEIGHT)];
    toolbar_.autoresizingMask = NSViewWidthSizable | NSViewMaxYMargin;
    toolbar_.autoresizesSubviews = YES;
    [view_ addSubview:toolbar_];
    
    // Textfield
    // TODO: Considder using NSTextView instead of NSTextField to allow vertically resizing the view
    // when typing longer chats
    textField_ = [[NSTextField alloc] initWithFrame:NSZeroRect];
    textField_.textColor = [NSColor blackColor];
    textField_.placeholderString = @"";
    [textField_ setTarget:self];
    [textField_ setAction:@selector(onSend)];
    [textField_ setFocusRingType:NSFocusRingTypeNone];
    [toolbar_ addSubview:textField_];
    
    // Toolbar2 (top toolbar)
    toolbar2_ = [[NSView alloc] initWithFrame:NSMakeRect(0, view_.frame.size.height - TOOLBAR_HEIGHT, view_.frame.size.width, TOOLBAR_HEIGHT)];
    toolbar2_.autoresizingMask = NSViewWidthSizable | NSViewMaxYMargin;
    toolbar2_.autoresizesSubviews = YES;
    [view_ addSubview:toolbar2_];
    
    // Title Label
    titleLabel_ = [[NSTextView alloc] initWithFrame:NSZeroRect];
    [titleLabel_ setFont:[NSFont boldSystemFontOfSize:18]];
    [titleLabel_ setTextColor:[NSColor whiteColor]];
    [titleLabel_ setString:title_];
    [titleLabel_ setAlignment:NSCenterTextAlignment];
    [titleLabel_ setSelectable:NO];
    [titleLabel_ setBackgroundColor:[NSColor clearColor]];
    
    // Tableview -- also see [self reCreateTableView]
    tableView_ = [[NSTableView alloc] initWithFrame:NSZeroRect];
    // tableView_ gets set as the document for scrollView and then scrollView is added
    // as a subview of view_. So tableView_ doesnt need to be added as a subview of view_.
    // [view_ addSubview:tableView_];
    
    [self layoutViews];
}

- (void)layoutViews {
    // Send button
    NSButton *sendButton = [[NSButton alloc] initWithFrame:NSZeroRect];
    [sendButton setTitle:@"Send"];
    [sendButton setTarget:self];
    [sendButton setAction:@selector(onSend)];
    [sendButton sizeToFit];
    [toolbar_ addSubview:sendButton];
    
    // Players button
    NSButton *playersButton = [[NSButton alloc] initWithFrame:NSZeroRect];
    [playersButton setTitle:@"Players"];
    [playersButton setTarget:self];
    [playersButton setAction:@selector(onPlayers)];
    [playersButton sizeToFit];
    [playersButton setFrameOrigin:NSMakePoint(toolbar2_.frame.size.width - playersButton.frame.size.width - 5, toolbar2_.frame.size.height/2 - playersButton.frame.size.height/2)];
    [toolbar2_ addSubview:playersButton];
    
    // Done button
    NSButton *doneButton = [[NSButton alloc] initWithFrame:NSZeroRect];
    [doneButton setTitle:@"Done"];
    [doneButton setTarget:self];
    [doneButton setAction:@selector(onDone)];
    [doneButton sizeToFit];
    [doneButton setFrameOrigin:NSMakePoint(5, toolbar2_.frame.size.height/2 - doneButton.frame.size.height/2)];
    [toolbar2_ addSubview:doneButton];
    
    // Textfield & send button (subviews of toolbar_)
    [textField_ setFrame:NSMakeRect(5, toolbar_.frame.size.height/2 - sendButton.frame.size.height/2, toolbar_.frame.size.width - sendButton.frame.size.width - 15, sendButton.frame.size.height)];
    [sendButton setFrameOrigin:NSMakePoint(textField_.frame.size.width + 10, textField_.frame.origin.y)];
    
    // Title label
    [titleLabel_ setFrameSize:NSMakeSize(toolbar2_.frame.size.width/2, toolbar2_.frame.size.height)];
    [titleLabel_ setFrameOrigin:NSMakePoint(toolbar2_.frame.size.width/2 - titleLabel_.frame.size.width/2, toolbar2_.frame.size.height/2 - titleLabel_.frame.size.height/2)];
    [toolbar2_ addSubview:titleLabel_];
    
    // Add some color
    [toolbar_.layer setBackgroundColor:[[NSColor darkGrayColor] CGColor]];
    [toolbar2_.layer setBackgroundColor:[[NSColor darkGrayColor] CGColor]];
    [titleLabel_ setBackgroundColor:[NSColor clearColor]];
}

- (void)reCreateTableView {
    // Table Column
    tableColumn_ = [[NSTableColumn alloc] initWithIdentifier:@"chatColumn"];
    [tableColumn_ setWidth:view_.frame.size.width];
    
    // Table View
    tableView_ = [[NSTableView alloc] initWithFrame:NSMakeRect(0,
        toolbar_.frame.size.height,
        view_.frame.size.width,
        view_.frame.size.height - toolbar_.frame.size.height - toolbar2_.frame.size.height)];
    [tableView_ setDelegate:self];
    [tableView_ setDataSource:self];
    [tableView_ setFocusRingType: NSFocusRingTypeNone];
    [tableView_ addTableColumn:tableColumn_];
    [tableView_ setHeaderView:nil];
    [tableView_ setSelectionHighlightStyle:NSTableViewSelectionHighlightStyleNone];
    
    // Scroll view
    NSScrollView *scrollView = [[NSScrollView alloc] initWithFrame:NSMakeRect(0,
        toolbar_.frame.size.height,
        view_.frame.size.width,
        view_.frame.size.height - toolbar_.frame.size.height - toolbar2_.frame.size.height)];
    [scrollView setHasVerticalScroller:YES];
    [scrollView setHasHorizontalScroller:NO];
    [scrollView setDocumentView:tableView_];
    [view_ addSubview:scrollView];

    // Both adding tableColumn_ to tableView_ and setting tableView as scroll view's document
    // seems to change tableView_'s height. So let's change that back to view_'s width.
    [tableView_ setFrameSize:NSMakeSize(view_.frame.size.width, tableView_.frame.size.height)];
}

- (void)show {
    BOOL created = NO;
    if (clear_ || tableView_ == nil) {
        // Create the table view. It will automagically reload the model
        [self reCreateTableView];
        clear_ = NO;
        created = YES;
    }
    
    // Present the chat window
    [window_ setFrameOrigin:[[[NSApplication sharedApplication] mainWindow] frame].origin];
    [window_ makeKeyAndOrderFront:self];

    [textField_ becomeFirstResponder];

    // Only scroll to the bottom if the table view existed already.
    // Otherwise it will crash, because data model loading is
    // asynchronous.
    if (created == NO) {
        [self scrollToBottom];
    }
}

- (void)onDone {
    [textField_ resignFirstResponder];
    [window_ orderOut:self];
    
    if (chatc_ != NULL) {
        chatc_->thread().Post(kidmOnDismissed, chatc_);
    }
}

- (void)onSend {
    if (chatc_ == NULL) {
        return;
    }

    NSString *text = textField_.stringValue;
    int count = (int)text.length;
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
    [textField_ setStringValue:@""];
}

- (void)onPlayers {
    if (chatc_ != NULL) {
        chatc_->thread().Post(kidmOnPlayers, chatc_);
    }
}

- (void)scrollToBottom {
    [tableView_ beginUpdates];

    // Scroll to the bottom
    int index = (int)[chatEntries_ count] - 1;
    if (index >= 0) {
        [tableView_ scrollRowToVisible:[tableView_ numberOfRows] - 1];
    }

    [tableView_ endUpdates];
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView {
    return [chatEntries_ count];
}

- (CGFloat)tableView:(NSTableView *)tableView heightOfRow:(NSInteger)row {
    return [self heightForString:[chatEntries_ objectAtIndex:row] width:view_.frame.size.width];
}

- (NSView *)tableView:(NSTableView *)tableView
   viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row {
    ChatCell *cell = [ChatCell cellWithRect:NSMakeRect(0, 0,
        view_.frame.size.width,
        [self heightForString:[chatEntries_ objectAtIndex:row] width:view_.frame.size.width])
        user:[userEntries_ objectAtIndex:row]
        chat:[chatEntries_ objectAtIndex:row]];

    return cell;
}

- (float)heightForString:(NSString *)string width:(float)width {
    return [self sizeForString:string maxWidth:width].height;
}

- (NSSize)sizeForString:(NSString *)string maxWidth:(float)width {
    NSFont* font = [NSFont systemFontOfSize:12];
    // Calculate the size for the text to be able to display correctly
    NSString* text = string;
    NSInteger maxWidth = width;
    NSInteger maxHeight = 2000;
    NSSize constraint = NSMakeSize(maxWidth, maxHeight);
    NSDictionary* attrs = [NSDictionary dictionaryWithObjectsAndKeys:NSFontAttributeName, font, nil];
    NSRect newBounds = [text boundingRectWithSize:constraint
        options:NSLineBreakByWordWrapping | NSStringDrawingUsesLineFragmentOrigin
        attributes:attrs];

    return newBounds.size;
}

- (void)updateChatRows:(BOOL)scroll {
    [tableView_ reloadData];
    [self scrollToBottom];
}

- (void)setController:(wi::ChatController *)chatc {
    chatc_ = chatc;
}

- (wi::ChatController *)controller {
    return chatc_;
}

- (void)doClear
{
    [chatEntries_ removeAllObjects];
    [userEntries_ removeAllObjects];
    
    [self updateChatRows:NO];
}

- (void)doAddChat:(NSDictionary *)dict
{
    NSString *chat = [dict objectForKey:@"chat"];
    NSString *user = [dict objectForKey:@"player"];
    
    if ([user length] == 0) {
            // System message.
            [chatEntries_ addObject:chat];
            [userEntries_ addObject:user];
        } else {
            // Regular chat message. Add ':' to the end of user
            NSString *newUser = [NSString stringWithFormat:@"%@: ", user];

            // Insert spaces into chat so that user's name can draw into that
            // space with a different UILabel. Hack-o-rama.

            NSSize sizeUser = [self sizeForString:newUser maxWidth:200];
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
            [chatEntries_ addObject:newChat];
            [userEntries_ addObject:newUser];
        }
    
    [self updateChatRows:YES];
}

- (void)doShow
{
    [self show];
    [titleLabel_ setString:title_];
}

- (void)doHide
{
    [self onDone];
}

- (void)doSetTitle:(NSString *)title
{
    title_ = title;
}
@end

// ++++
// This is the game thread callable class for controlling chat
namespace wi {

// These get called on the game thread, and do the real work on the main
// thread.

ChatController::ChatController(ChatViewController *vcchat) : vcchat_(vcchat), pcccb_(NULL) {
    vcchat_ = vcchat;
    [vcchat_ setController:this];
}

void ChatController::Clear() {
    [vcchat_ performSelectorOnMainThread:@selector(doClear) withObject:nil waitUntilDone: NO];
}

void ChatController::AddChat(const char *player, const char *chat) {
    printf("ChatController::AddChat %s\n", chat);
    @autoreleasepool {
        NSString *player_s = [NSString stringWithCString:player
                encoding:[NSString defaultCStringEncoding]];
        NSString *chat_s = [NSString stringWithCString:chat
                encoding:[NSString defaultCStringEncoding]];
        NSDictionary *dict = [NSDictionary dictionaryWithObjectsAndKeys:
                player_s, @"player", chat_s, @"chat", (char *)NULL];
        [vcchat_ performSelectorOnMainThread:@selector(doAddChat:)
                withObject:dict waitUntilDone: NO];
    }
}

void ChatController::Show(bool fShow) {
    printf("ChatController::Show %s\n", fShow ? "True" : "False");
    if (fShow) {
        [vcchat_ performSelectorOnMainThread:@selector(doShow)
                withObject:nil waitUntilDone: NO];
    } else {
        [vcchat_ performSelectorOnMainThread:@selector(doHide)
                withObject:nil waitUntilDone: NO];
    }
}

void ChatController::SetTitle(const char *title) {
    printf("ChatController::SetTitle %s\n", title);
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
    printf("ChatController::OnMessage\n");
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