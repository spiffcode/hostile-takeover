package com.spiffcode.ht;

import java.io.File;
import org.libsdl.app.SDLActivity; 
import com.spiffcode.ht.R;
import com.spiffcode.ht.NativeLib;
import android.annotation.SuppressLint;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.view.View;
import android.view.View.OnSystemUiVisibilityChangeListener;
import android.view.Window;
import android.view.WindowManager;

// A wrapper class to call SDLActivity and pass some data onto NativeLib
public class GameActivity extends SDLActivity {	
	
	@Override
	public void onCreate(Bundle savedInstanceState)
	{		
		// Make the activity fullscreen
		requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
        WindowManager.LayoutParams.FLAG_FULLSCREEN);
		        
		// Setup immersive mode
        setSystemUiVisibilityMode(getWindow().getDecorView());
		
        setContentView(R.layout.main);
        
        // Let's keep the screen on
     	getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        
		setupNativeLib();
		
		String gamePath = getApplication().getApplicationContext().getFilesDir().getAbsolutePath();
		NativeLib.gameDataPath = gamePath;
		File dataPathFile = new File(gamePath);
		dataPathFile.mkdir();
		
		super.onCreate(savedInstanceState);
	}
	
	@Override
	public void onResume() {
	    super.onResume();
	    setSystemUiVisibilityMode(getWindow().getDecorView());
	}
	
	void setupNativeLib() {
		// Screen information
		DisplayMetrics metrics = getApplicationContext().getResources().getDisplayMetrics();
		NativeLib.screenWidth = metrics.widthPixels;
		NativeLib.screenHeight = metrics.heightPixels;
		NativeLib.screenDPI = metrics.densityDpi;
		
		NativeLib.assetManager = getResources().getAssets();
		
		// App data path
		NativeLib.gameDataPath = getApplication().getApplicationContext().getFilesDir().getAbsolutePath();
		
		// Give NativeLib access to this activity
		NativeLib.gameActivity = this;
	}
	
	@SuppressLint("NewApi")
	public static void setSystemUiVisibilityMode(final View v) {
		// Api 11 is needed to override setOnSystemUiVisibilityChangeListener()
		if (android.os.Build.VERSION.SDK_INT >= 11) {
			// Set visibility options now
		    v.setSystemUiVisibility(getVisibilityOptions());
		    
		    // Override visibility options for the future
		    v.setOnSystemUiVisibilityChangeListener(new OnSystemUiVisibilityChangeListener() {
		    
		    	@Override
		    	public void onSystemUiVisibilityChange(int visibility) {
				    v.setSystemUiVisibility(getVisibilityOptions());
		    	}
		    });
		}
	}

	@SuppressLint("NewApi")
	private static int getVisibilityOptions() {

		int options = 0;
		
		if (android.os.Build.VERSION.SDK_INT >= 16) {
			options = options |
				View.SYSTEM_UI_FLAG_LAYOUT_STABLE
				| View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
				| View.SYSTEM_UI_FLAG_FULLSCREEN;
		}
		
		if (android.os.Build.VERSION.SDK_INT >= 19) {
			options = options | 
				View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
				| View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
				| View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION;
		}
	
		return options;
	}
}
