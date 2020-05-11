package com.example.mymediaplayer;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import androidx.constraintlayout.widget.ConstraintLayout;
import androidx.core.app.ActivityCompat;

import android.Manifest;
import android.media.MediaPlayer;
import android.os.Bundle;
import android.os.Environment;
import android.provider.ContactsContract;
import android.util.Log;
import android.util.SparseArray;
import android.view.PointerIcon;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.ViewGroup;
import android.widget.FrameLayout;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.Map;
import java.util.Queue;
import java.util.Set;

import static android.content.pm.PackageManager.PERMISSION_GRANTED;

public class MainActivity extends AppCompatActivity {
    private MediaPlayer player;
    private FrameLayout mContainer;
    private Runnable mRunnable;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        //下面开始实例化MediaPlayer对象
        player = new MediaPlayer();

        //只有当播放器准备好了之后才能够播放，所以播放的出发只能在触发了prepare之后
        player.setOnPreparedListener(new MediaPlayer.OnPreparedListener() {
            @Override
            public void onPrepared(MediaPlayer mp) {
                mp.start();
            }
        });

        /*
            向player中设置dispay，也就是SurfaceHolder。
            但此时有可能SurfaceView还没有创建成功，所以需要监听SurfaceView的创建事件
         */
        SurfaceView surfaceView = (SurfaceView)this.findViewById(R.id.surface_view);
        surfaceView.getHolder().addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder holder) {
                //将播放器和SurfaceView关联起来
                player.setDisplay(holder);
            }

            @Override
            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
                Log.d("++++++++",  "width" + width + " height" + height);
            }

            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {

            }
        });

        ActivityCompat.requestPermissions(MainActivity.this, new String[]{Manifest.permission.READ_EXTERNAL_STORAGE, Manifest.permission.WRITE_EXTERNAL_STORAGE}, 1);
        mContainer = findViewById(R.id.container);
        mRunnable = new Runnable() {
            @Override
            public void run() {
                int width = mContainer.getWidth();
                int height = mContainer.getHeight();
                ViewGroup.LayoutParams params = mContainer.getLayoutParams();
                params.width = width + 50;
                params.height = height + 50;
                mContainer.setLayoutParams(params);
                mContainer.postDelayed(mRunnable, 2000);
                Log.d("+++++++++++", "111111111");
            }
        };
        mContainer.postDelayed(mRunnable, 2000);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == 1) {
            for (int i = 0; i < permissions.length; i++) {
                if (grantResults[i] == PERMISSION_GRANTED) {
                    //设置数据数据源，也就播放文件地址，可以是网络地址
                    String dataPath = "http://192.168.10.104:8080/vod.html?src=rtmp://192.168.10.104/vod/test.mp4";
                    player.reset();
                    try {
                        player.setDataSource(dataPath);
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                    //异步缓冲当前视频文件，也有一个同步接口
                    player.prepareAsync();
                } else {
                    // 申请失败
                }
            }
        }
    }

    static class Solution {
        private static class Point {
            int x;
            int y;

            private Point(int x, int y) {
                this.x = x;
                this.y = y;
            }

            @Override
            public int hashCode() {
                return super.hashCode();
            }

            @Override
            public boolean equals(@Nullable Object obj) {
                return super.equals(obj);
            }
        }
        public  void solve(char[][] board) {
            int row = board.length;
            int col = board[0].length;
            Queue<Point> queue = new LinkedList<>();
            Set<Point> pointList = new HashSet<>();
            for (int i = 0; i < col; i++) {
                if (board[0][i] == 'O') {
                    queue.offer(new Point(0, i));
                }
                if (board[row - 1][i] == 'O') {
                    queue.offer(new Point(row-1, i));
                }
            }
            for (int i = 0; i < row; i++) {
                if (board[i][0] == 'O') {
                    queue.offer(new Point(i, 0));
                }
                if (board[i][col - 1] == 'O') {
                    queue.offer(new Point(i, col - 1));
                }
            }
            while (!queue.isEmpty()) {
                Point point = queue.poll();
                pointList.add(point);
                if (point.x - 1 >= 0 && board[point.x - 1][point.y] == 'O') {

                }
            }
        }
    }
}
