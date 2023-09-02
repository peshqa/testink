package com.peshqa.testink;

import android.app.Activity;
import android.os.Bundle;
import android.widget.TextView;

public class MainActivity extends Activity
{
	// Used to load the 'testink' library on application startup.
    static {
        System.loadLibrary("testink");
    }
	
    @Override
    public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

        TextView label = new TextView(this);
        label.setText(stringFromJNI());

        setContentView(label);
    }
	
	/**
     * A native method that is implemented by the 'testink' native library,
     * which is packaged with this application.
     */
	public native String stringFromJNI();
}