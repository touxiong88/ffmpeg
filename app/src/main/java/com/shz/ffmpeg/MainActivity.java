package com.shz.ffmpeg;

import android.app.Activity;
import android.os.Bundle;
import android.os.Environment;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Spinner;

import java.io.File;

public class MainActivity
        extends Activity
{
    String[] videos = {"demo.mp4","demo.avi","demo.mkv","demo.flv"};
    private Spinner sp_video;
    MyVideoView videoView;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        videoView = (MyVideoView) findViewById(R.id.surface);
        sp_video = (Spinner) findViewById(R.id.sp_video);

        //多种格式的视频列表
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_list_item_1,android.R.id.text1,videos);
        sp_video.setAdapter(adapter);


    }

    public void mPlay(View view){
        String video = sp_video.getSelectedItem().toString();
        String input = new File(Environment.getExternalStorageDirectory(),video).getAbsolutePath();
        videoView.player(input);
    }
}
