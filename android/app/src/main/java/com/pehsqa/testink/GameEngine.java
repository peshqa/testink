package com.peshqa.testink;

import android.graphics.Canvas;

public class GameEngine
{
	public static GameEngine instance = null;
	
	public static GameEngine getInstance() {
        if (instance == null) {
            instance = new GameEngine();
        }
        return instance;
    }
	
	public boolean Update()
	{
		return true;
	}
	
	public boolean Draw(Canvas canvas)
	{
		return true;
	}
}