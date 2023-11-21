package com.peshqa.testink;

import android.app.Activity;
import android.os.Bundle;
import android.widget.TextView;
import android.view.SurfaceView;
import android.view.SurfaceHolder;

import android.view.Window;
import android.view.WindowManager;

//import android.widget.Toast;

public class MainActivity extends Activity
{
	
	// Used to load the 'testink' library on application startup.
    static {
        System.loadLibrary("testink");
    }
	
    @Override
    public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		// TODO: get fullscreen to work!
		//requestWindowFeature(Window.FEATURE_NO_TITLE);
        //getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
        //        WindowManager.LayoutParams.FLAG_FULLSCREEN);
		
		SurfaceView view = new GameView(this/*, AppConstants.GetEngine()*/);

		setContentView(view);
		getActionBar().hide();
        /*TextView label = new TextView(this);
        label.setText(stringFromJNI());

        setContentView(label);*/
    }
	
	/**
     * A native method that is implemented by the 'testink' native library,
     * which is packaged with this application.
     */
	public native String stringFromJNI();
}