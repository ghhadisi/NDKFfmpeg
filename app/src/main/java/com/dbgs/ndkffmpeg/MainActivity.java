package com.dbgs.ndkffmpeg;

import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.SurfaceView;
import android.view.View;
import android.widget.TextView;


//import com.dbgs.ffmpeglib2.VideoView;

//import com.dbgs.ffmpegaudiolib1.AudioPlayer;

//import com.dbgs.ffmpegopensles.AudioPlayer;

import com.dbgs.videoaudiolib.MyPlayer;

import java.io.File;

public class MainActivity extends AppCompatActivity {
    TextView tv_name;
//    VideoView surface;
SurfaceView surfaceView;
    MyPlayer myPlayer;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        tv_name = (TextView) findViewById(R.id.tv_name);
//        surface = (VideoView) findViewById(R.id.surface);
        tv_name.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                load();
            }
        });
        surfaceView = (SurfaceView) findViewById(R.id.surfaceView);
         myPlayer = new MyPlayer();
        myPlayer.setSurfaceView(surfaceView);
    }
    void load(){
//        VideoUtil  util = new VideoUtil();
//        String input = new File(Environment.getExternalStorageDirectory(), "input.mp4").getAbsolutePath();
//        String  output= new File(Environment.getExternalStorageDirectory(), "output.yuv").getAbsolutePath();
//        util.open(input, output);

//        String input = new File(Environment.getExternalStorageDirectory(), "input.mp4").getAbsolutePath();
//        surface.player(input);

//        AudioPlayer audioPlayer = new AudioPlayer();
//        String input = new File(Environment.getExternalStorageDirectory(),"input.mp3").getAbsolutePath();
//        String output = new File(Environment.getExternalStorageDirectory(),"output.pcm").getAbsolutePath();
//        audioPlayer.sound(input,output);


//        AudioPlayer audioPlayer = new AudioPlayer();
//        String input = new File(Environment.getExternalStorageDirectory(),"input.mp3").getAbsolutePath();
//        audioPlayer.sound();
        File file = new File(Environment.getExternalStorageDirectory(), "input.mp4");
//        myPlayer.playJava("rtmp://live.hkstv.hk.lxdns.com/live/hks");
        myPlayer.playJava(file.getAbsolutePath());
    }
}
