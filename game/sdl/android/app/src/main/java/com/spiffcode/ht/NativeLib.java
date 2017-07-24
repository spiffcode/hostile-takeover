package com.spiffcode.ht;

import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.AssetManager;
import android.net.Uri;
import android.provider.Settings.Secure;
import android.text.InputFilter;
import android.text.InputType;
import android.widget.EditText;

public class NativeLib {
	
	// TODO: Make these private and have getter/setter functions
	public static String gameDataPath; 			// data/user/0/com.spiffcode.ht/files
	public static int screenWidth; 				// in pixels
	public static int screenHeight; 			// in pixels
	public static int screenDPI; 				// in dpi
	public static AssetManager assetManager;
	public static String askString;				// string from user input dialog
	public static GameActivity gameActivity; 	// A link back to our main gameActivity class instance
	public static ChatController chatc;
	
	// Native methods
	private native static void nativePostAskStringEvent();
	
	// Gets called from Java
	public static void postAskStringEvent() {
		nativePostAskStringEvent();
	}
	
	// Methods that get called from C++
	
	static String getDataDir() {
		return gameDataPath;
	}
	
	static int screenWidth() {
		return screenWidth;
	}
	
	static int screenHeight() {
		return screenHeight;
	}
	
	static int screenDPI() {
		return screenDPI;
	}
	
	static AssetManager getAssetManager() {
		return assetManager;
	}
	
	static String getAndroidID() {
		return Secure.getString(GameActivity.getContext().getContentResolver(), Secure.ANDROID_ID);
	}
	
	static void openUrl(String url) {
		Intent browserIntent = new Intent(Intent.ACTION_VIEW, Uri.parse(url));
		browserIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
		GameActivity.getContext().startActivity(browserIntent);
	}
	
	static void initiateAsk(final String title, final int max, final String def, final int keyboard, final int secure) {
		// Do the work on the UI thread
		gameActivity.runOnUiThread(new Runnable() {
	        @Override
	        public void run() {
                AlertDialog.Builder alertBuilder = new AlertDialog.Builder(GameActivity.getContext());
                alertBuilder.setTitle(title);

                // Setup an EditText view
                final EditText editText = new EditText(GameActivity.getContext());
                editText.setText(def);
                // Is this a password EditText?
                if (secure == 1) {
                    editText.setInputType(InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_VARIATION_PASSWORD);
                } else {
                    editText.setInputType(InputType.TYPE_CLASS_TEXT);
                }
                
                // Set max characters
                if (max != -1) {
                    editText.setFilters(new InputFilter[] {new InputFilter.LengthFilter(max)});
                }
                
                // Add to alert
                alertBuilder.setView(editText);

                // Setup the alert buttons
                alertBuilder.setPositiveButton("Ok", new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int whichButton) {
                    String value = (editText.getText()).toString();
                        // Save this value so the game can grab it				
                        NativeLib.askString = value;
                        NativeLib.postAskStringEvent();
                    }
                });
                alertBuilder.setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        // Canceled
                    }
                });

                // Show the alert and set the cursor position
	        	alertBuilder.create().show();
	        	editText.setSelection(editText.getText().length());
	        }
	    });
	}
	
	static void initiateWebView(final String url) {
		// Create and show the web dialog from the UI thread
		gameActivity.runOnUiThread(new Runnable() {
	        @Override
	        public void run() {
	        	WIWebDialog webDialog = new WIWebDialog(gameActivity, url);
	        	webDialog.show();
	        }
	    });
	}
	
	static String getAskString() {
		return askString;
	}
	
	static String getPlatformString() {
		return "Android " + android.os.Build.VERSION.RELEASE;
	}
}
