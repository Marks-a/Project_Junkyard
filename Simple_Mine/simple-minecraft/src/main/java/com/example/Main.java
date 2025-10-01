package com.example;

import com.example.engine.Window;
/**
 * Main class for just starting the program.
 *
 */
public class Main 
{
    public static void main( String[] args )
    {
        System.out.println( "Hello from Main.java, starting window()" );
        Window window = new Window(800, 600, "Simple Minecraft");
        window.run();
    }
}
