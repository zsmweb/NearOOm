# NearOOm
监控OOM，临近OOM

Android app 因为往往在临近OOM时会打印一下log：
zu.test.testap: Clamp target GC heap from 248MB to 152MB

从而hook 住 SetIdealFootprint函数，在第一次打印上述log的时候抓一个hprof，用来分析oom问题