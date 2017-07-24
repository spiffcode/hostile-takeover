package com.spiffcode.ht;

import com.spiffcode.ht.R;

import android.app.Activity;
import android.app.Dialog;
import android.os.Bundle;
import android.view.View;
import android.view.WindowManager;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.Button;
import android.widget.ProgressBar;

public class WIWebDialog extends Dialog implements android.view.View.OnClickListener {

	private String loadUrl;
	private Button btn_done, btn_back, btn_forward;
	private WebView webView;
	private ProgressBar progress;

	public WIWebDialog(Activity a, String url) {
		super(a, android.R.style.Theme_Black_NoTitleBar_Fullscreen);
		this.loadUrl = url;
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		// Grab the content view from the xml
		setContentView(R.layout.wi_web_dialog);
		progress = (ProgressBar)findViewById(R.id.progress);
		
		// Setup immersive mode
		setSystemUiVisibilityMode();
		
		// Let's keep the screen on
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
		
		// Setup the web view
		webView = (WebView)findViewById(R.id.webview);
		webView.loadUrl(loadUrl);
		webView.setWebViewClient(new WebViewClient() {
		    @Override
		    public boolean shouldOverrideUrlLoading(WebView view, String url) {
		    	progress.setVisibility(View.VISIBLE);
		        view.loadUrl(url);
		        return true;
		    }
		    
		    @Override
		    public void onPageFinished(WebView view, String url) {
		        super.onPageFinished(view, url);
		        progress.setVisibility(View.GONE);
		    }
		});
		
		// Set the listener for the buttons
		btn_done = (Button)findViewById(R.id.btn_done);
		btn_back = (Button)findViewById(R.id.btn_back);
		btn_forward = (Button)findViewById(R.id.btn_forward);
		btn_done.setOnClickListener(this);
	    btn_back.setOnClickListener(this);
	    btn_forward.setOnClickListener(this);

        // Enable JavaScript
        webView.getSettings().setJavaScriptEnabled(true);
	}
	
	private void setSystemUiVisibilityMode() {
		View v = getWindow().getDecorView();
		GameActivity.setSystemUiVisibilityMode(v);
	}
	
	@Override
	public void onClick(View v) {
		// Deal with button clicks
	    switch (v.getId()) {
	    case R.id.btn_done:
	    	dismiss();
	    	break;
	    case R.id.btn_back:
	    	if (webView.canGoBack()) {
	    		webView.goBack();
	    	}
	    	break;
	    case R.id.btn_forward:
	    	if (webView.canGoForward()) {
	    		webView.goForward();
	    	}
	    	break;
	    default:
	    	break;
	    }
	}
}
