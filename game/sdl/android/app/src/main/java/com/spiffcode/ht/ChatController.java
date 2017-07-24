package com.spiffcode.ht;

import com.spiffcode.ht.ChatDialog;
import com.spiffcode.ht.NativeLib;

public class ChatController {
	
	// Native methods
	public native void nativeOnSend(String chat);
	public native void nativeOnDone();
	public native void nativeOnPlayers();
	
	// Private variables
	private String title_;
	private ChatDialog chatDialog_;
	
	// Methods that get called from JNI
	// Note: Any of these methods that want to modify the dialog UI will need to 
	// be run on the UI thread.
	
	public ChatController() {
		super();
		
		// Save this in NativeLib
		NativeLib.chatc = this;
		
		// Create the dialog
		NativeLib.gameActivity.runOnUiThread(new Runnable() {
	        @Override
	        public void run() {
	        	chatDialog_ = new ChatDialog(NativeLib.gameActivity);
	        	
	        	// Show and hide the dialog so dialog.onCreate gets called.
	        	// TODO: We shouldn't have to do this
	        	chatDialog_.show();
	        	chatDialog_.dismiss();
	        }
	    });
	}
	
	public void clear() {
		NativeLib.gameActivity.runOnUiThread(new Runnable() {
	        @Override
	        public void run() {
	        	chatDialog_.clear();
	        }
	    });
	}
	
	public void addChat(final String player, final String chat, final int sig) {
		NativeLib.gameActivity.runOnUiThread(new Runnable() {
	        @Override
	        public void run() {
	        	chatDialog_.addChat(player, chat, sig);
	        }
	    });
	}

	public void show() {
		NativeLib.gameActivity.runOnUiThread(new Runnable() {
	        @Override
	        public void run() {
	        	chatDialog_.show();
	        }
	    });
	}
	
	public void hide() {
        NativeLib.gameActivity.runOnUiThread(new Runnable() {
	        @Override
	        public void run() {
	        	chatDialog_.dismiss();
	        }
	    });
	}
	
	public void setTitle(String title) {
		title_ = title;
		chatDialog_.setTitle(title);
	}
	
	public String getTitle() {
		return title_;
	}
}
