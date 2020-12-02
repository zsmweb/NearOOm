package me.nobody.droid.nearoom;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import com.test.ems.hook.HookArt;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

public class MainActivity extends AppCompatActivity {
    private List<byte[]> heap=new ArrayList<>();
    private TextView dashboard;
    public static final float UNIT_M = 1024 * 1024;
    private static Handler H = new Handler(Looper.getMainLooper());
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        HookArt.hprofPath = new File(this.getCacheDir(), "oom.hprof").getAbsolutePath();

        HookArt.initHook();

        Button btn = findViewById(R.id.button);
        dashboard = findViewById(R.id.textView);
        btn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                new Thread(){
                    @Override
                    public void run() {
                        super.run();
                        for (int i = 0; i < 2; i++) {
                            heap.add(new byte[50000000]);
                            H.post(new Runnable() {
                                @Override
                                public void run() {
                                    StringBuilder stringBuilder=new StringBuilder();
                                    stringBuilder.append("Java Heap Max : ").append(Runtime.getRuntime().maxMemory()/UNIT_M).append(" MB\r\n");
                                    stringBuilder.append("Current used  : ").append((Runtime.getRuntime().totalMemory()-Runtime.getRuntime().freeMemory())/UNIT_M).append(" MB\r\n");
                                    dashboard.setText(stringBuilder.toString());
                                }
                            });
                            try {
                                Thread.sleep(1000);
                            } catch (InterruptedException e) {
                                e.printStackTrace();
                            }
                        }
                        int i=0;
                        while (true) {
                            i ++;
                            heap.add(new byte[10000]);
                            if (i%1000 == 0) {
                                H.post(new Runnable() {
                                    @Override
                                    public void run() {
                                        StringBuilder stringBuilder=new StringBuilder();
                                        stringBuilder.append("Java Heap Max : ").append(Runtime.getRuntime().maxMemory()/UNIT_M).append(" MB\r\n");
                                        stringBuilder.append("Current used  : ").append((Runtime.getRuntime().totalMemory()-Runtime.getRuntime().freeMemory())/UNIT_M).append(" MB\r\n");
                                        dashboard.setText(stringBuilder.toString());
                                    }
                                });
                            }
                            try {
                                Thread.sleep(0, 10);
                            } catch (InterruptedException e) {
                                e.printStackTrace();
                            }
                        }
                    }
                }.start();
            }
        });
    }
}