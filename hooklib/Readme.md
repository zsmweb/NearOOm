拷贝了https://github.com/iqiyi/xHook代码
拷贝了https://github.com/KwaiAppTeam/KOOM/tree/master/kwai-linker代码
组装成一个库，并稍加改动，添加了hook SetIdealFootprint中的Prettysize函数调用
一般临近OOM的时候都会打印
zu.test.testap: Clamp target GC heap from 248MB to 152MB
类似上面这种log
所以正对这个现象做的一个简单监控