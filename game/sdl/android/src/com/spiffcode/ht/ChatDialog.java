package com.spiffcode.ht;

import com.spiffcode.ht.R;

import android.app.Activity;
import android.app.Dialog;
import android.content.DialogInterface;
import android.graphics.Color;
import android.graphics.Rect;
import android.os.Bundle;
import android.text.InputType;
import android.text.SpannableString;
import android.text.SpannableStringBuilder;
import android.text.style.ForegroundColorSpan;
import android.view.KeyEvent;
import android.view.View;
import android.view.ViewTreeObserver;
import android.view.Window;
import android.view.WindowManager;
import android.view.inputmethod.EditorInfo;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.TextView.OnEditorActionListener;

public class ChatDialog extends Dialog implements android.view.View.OnClickListener {
	
	private ListView listView_;
	private ArrayAdapter<Object> listViewAdapter_;
	private Button btn_done, btn_players, btn_send;
	private EditText editText_;
	private RelativeLayout layout_;
	private boolean everyOtherOnGlobalLayout_;

	public ChatDialog(Activity a) {
		super(a, android.R.style.Theme_Black_NoTitleBar_Fullscreen);
		listViewAdapter_ = new ArrayAdapter<Object>(GameActivity.getContext(), R.layout.chat_listview_row);
	}
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		// Setup immersive mode
		setSystemUiVisibilityMode();
		
		// Grab the content view from the xml
		setContentView(R.layout.chat_dialog);
		
		// Let's keep the screen on
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
		
		// Enable the hardware back button to close the dialog and let the
		// game know that the dialog was closed
		final ChatDialog dialog = (ChatDialog)this;
		dialog.setOnKeyListener(new Dialog.OnKeyListener() {			
            @Override
            public boolean onKey(DialogInterface arg0, int keyCode, KeyEvent event) {          	
                if (keyCode == KeyEvent.KEYCODE_BACK) {
                	dialog.dismiss();
                	return true;
                }
                return false;
            }
        });
		
		// Set the listener for the buttons
		btn_done = (Button)findViewById(R.id.btn_done);
	    btn_players = (Button)findViewById(R.id.btn_players);
	    btn_send = (Button)findViewById(R.id.btn_send);
	    btn_done.setOnClickListener(this);
	    btn_players.setOnClickListener(this);
	    btn_send.setOnClickListener(this);
		
	    // Setup the ListView
		listView_ = (ListView)findViewById(R.id.listView);    
		listView_.setAdapter(listViewAdapter_);
		listView_.setTranscriptMode(ListView.TRANSCRIPT_MODE_ALWAYS_SCROLL);
	    
	    // Setup layout
	    layout_ = (RelativeLayout)findViewById(R.id.rlayout);
	    
	    // Setup editText
	    editText_ = (EditText)findViewById(R.id.edittext);
	    editText_.requestFocus();
	    editText_.setInputType(InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_FLAG_CAP_SENTENCES);
	    editText_.setOnEditorActionListener(new OnEditorActionListener() {
	        @Override
	    	public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
	            boolean handled = false;
	            if (actionId == EditorInfo.IME_ACTION_SEND) {
	            	sendChat();
	                handled = true;
	            }
	            return handled;
	        }
	    });
	}
	
	@Override
	public void show() {
		// This listener "expires" when the dialog is dismissed. Hence, it is set here
		// instead of being set in onCreate()
		final Window rootWindow = getWindow();
	    View rootView = rootWindow.getDecorView().findViewById(android.R.id.content);
	    rootView.getViewTreeObserver().addOnGlobalLayoutListener(
	        new ViewTreeObserver.OnGlobalLayoutListener() {
	        public void onGlobalLayout() {
	        	// This method *should* only get called when the soft keyboard is shown/hidden.
	        	// So we can use to to resize the chatroom appropriately when that happens.
	        	// However, calling resizing the chatroom layout in this method will cause the system
	        	// to call this method again -- creating an infinite loop. To avoid this, we will
	        	// immediately return the method if it is being called because we just changed the
	        	// layout's size. This is achieved by immediately returning the method every other
	        	// time it is called.
	        	if (everyOtherOnGlobalLayout_) {
	        		everyOtherOnGlobalLayout_ = false;
	        		return;
	        	}
	        	everyOtherOnGlobalLayout_ = true;

	        	// Get the rect that we can display on; this take's into account the presence
	        	// of the soft keyboard.
	            Rect displayRect = new Rect();
	            View view = rootWindow.getDecorView();
	            view.getWindowVisibleDisplayFrame(displayRect);
	            
	            // Set the layout's height and width to the visible display room.
	            LinearLayout.LayoutParams params = (LinearLayout.LayoutParams)layout_.getLayoutParams();
	    		params.height = displayRect.height();
	    		params.width = displayRect.width();
	    		layout_.setLayoutParams(params);
	        }
	    });

        super.show();
	    editText_.requestFocus();
	    
	    listViewAdapter_.notifyDataSetChanged();
	    scrollToBottom();
	}
	
	@Override
	public void dismiss() {
		NativeLib.chatc.nativeOnDone();
		editText_.clearFocus();
		super.dismiss();
	}
	
	@Override
	public void onClick(View v) {
		// Deal with button clicks
	    switch (v.getId()) {
	    case R.id.btn_done:
	    	dismiss();
	    	break;
	    case R.id.btn_players:
	    	NativeLib.chatc.nativeOnPlayers();
	    	break;
	    case R.id.btn_send:
	    	sendChat();
	    	break;
	    default:
	    	break;
	    }
	}
	
	public void addChat(String player, String chat, int sig) {	
		// Change "+" to "\xa0" but only do this if we know the player is a mod with a sig
		if (sig == 1) {
			if (player.contains("+")) {
				player = player.replace("+", "†");
			}
			
			// Server message
			if (player.isEmpty()) {
				chat = chat.replace("+", "†");
			}
		}
				
		SpannableStringBuilder text = new SpannableStringBuilder();
		
		if (player.isEmpty()) { // Server message			
			// Color the server message
			SpannableString messageSpan = new SpannableString(chat);
			messageSpan.setSpan(new ForegroundColorSpan(Color.GRAY), 0, chat.length(), 0);
			text.append(messageSpan);
		} else { // Player chat
			// Add the ": "
			player = player + ": ";
			
			// Color the player's name
			SpannableString playerSpan = new SpannableString(player);
			playerSpan.setSpan(new ForegroundColorSpan(Color.BLUE), 0, player.length(), 0);
			text.append(playerSpan);
			
			// Color the chat
			SpannableString chatSpan = new SpannableString(chat);
			chatSpan.setSpan(new ForegroundColorSpan(Color.BLACK), 0, chat.length(), 0);
			text.append(chatSpan);	
		}
		
		listViewAdapter_.add(text);
		listViewAdapter_.notifyDataSetChanged();
		scrollToBottom();
	}
	
	public void clear() {
		listViewAdapter_.clear();
		listViewAdapter_.notifyDataSetChanged();
	}
	
	public void setTitle(String title) {
		TextView titleView = (TextView)findViewById(R.id.title);
		titleView.setText(title);
	}
	
	private void sendChat() {
		// Get the chat and trim whitespace
		String chat = (editText_.getText()).toString().trim();
		
		// Replace non ASCII characters
		chat = chat.replaceAll("[^\\x00-\\x7F]", "?");
		
		// Send the chat if it's not empty
		if (!chat.isEmpty()) {
			NativeLib.chatc.nativeOnSend(chat);
		}
		
		// Remove any text from the editText
		editText_.setText("");
	}
	
	private void scrollToBottom() {		
		listView_.post(new Runnable() {
	        @Override
	        public void run() {
	        	listView_.setSelection(listViewAdapter_.getCount() - 1);
	        }
	    });
	}
	
	private void setSystemUiVisibilityMode() {
		View v = getWindow().getDecorView();
		GameActivity.setSystemUiVisibilityMode(v);
	}
}
