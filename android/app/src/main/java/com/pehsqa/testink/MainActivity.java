package com.peshqa.testink;

import android.app.Activity;
import android.os.Bundle;
import android.widget.TextView;

//import com.peshqa.testink.databinding.ActivityMainBinding;

public class MainActivity extends Activity
{
	// Used to load the 'testink' library on application startup.
    static {
        System.loadLibrary("testink");
    }
	
	//private ActivityMainBinding binding;
	
    @Override
    public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

        /*binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        // Example of a call to a native method
        TextView tv = binding.sampleText;
        tv.setText(stringFromJNI());*/

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