package com.shz.ffmpeg;


import android.content.Context;
import android.graphics.PixelFormat;
import android.util.AttributeSet;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

/**
 * projectName: 	    ffmpeg
 * packageName:	        com.shz.ffmpeg
 * className:	        MyVideoView
 * author:	            SHZ
 * time:	            2017/12/20	17:40
 * desc:	            TODO
 *
 * svnVersion:	        $Rev
 * upDateAuthor:	    Administrator
 * upDate:	            2017/12/20
 * upDateDesc:	        TODO
 */
public class MyVideoView
        extends SurfaceView
{
    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("avutil-54");
        System.loadLibrary("swresample-1");
        System.loadLibrary("swscale-3");
        System.loadLibrary("avcodec-56");
        System.loadLibrary("avformat-56");
        System.loadLibrary("avfilter-5");
        System.loadLibrary("avdevice-56");

        System.loadLibrary("native-lib");
    }

    public MyVideoView(Context context) {
        this(context , null);
    }

    public MyVideoView(Context context, AttributeSet attrs) {
        this(context, attrs , 0);
    }

    public MyVideoView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init();
    }

    private void init(){
        SurfaceHolder surfaceHolder = getHolder();
        surfaceHolder.setFormat(PixelFormat.RGBA_8888);;
    }

    public void player(final String input){
        new Thread(new Runnable() {
            @Override
            public void run() {
                render(input,MyVideoView.this.getHolder().getSurface());
            }
        }).start();
    }

    public native void render(String input, Surface surface);
}
