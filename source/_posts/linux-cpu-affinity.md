title: Linux中程序的CPU亲和性(affinity)
tags: 
- linux
- cpu
- affinity
- nginx
category: coding
date: 2015-03-15 14:26
---
之前在nginx的配置文件中看到有一个配置选项`worker_cpu_affinity`用来配置nginx worker process的cpu亲和性，cpu亲和性就是进程或者线程在某个给定的cpu上尽量长时间地运行而不被迁移到其它处理器上的特性。这个没什么问题，我好奇的是这东西到底有多大作用。上网看了下相关的文档，发现设置CPU亲和性的API也就两三种，分别是针对线程和进程的，这里就用线程来测试一下。
<!-- more -->
##pthread cpu affinity
线程中设置cpu亲和性的api和变量主要有以下几个：

* cpu_set_t： cpu的掩码集合
* CPU_ZERO： cpu掩码清零
* CPU_SET： 设置cpu掩码
* pthread_setaffinity_np： 设置线程亲和性，成功返回0。
* pthread_getaffinity_np： 查看亲和性设置结果，成功返回0。

注：这些api仅在FreeBSD和linux下可用，且在使用前须在所有头文件的前面定义宏`_GNU_SOURCE`。

利用这几个api，我写了个小程序开启了和cpu核心数相等的线程数，分别在默认情况下和设置了cpu亲和性的情况下进行等量的计算，最后分别计算出它们的消耗时间。

经过多次的测试，两组数据相当接近，差距基本在误差范围内，不过设置了cpu亲和性的一组线程消耗的时间总是要多一点。

需要查看代码的童鞋可以点[这里](https://github.com/mafagan/blog-code/blob/master/code/thread_cpu.c)。

##nginx的cpu affinity测试
既然自己的测试结果不太给力，那还是拿nginx来测吧。我是用的是nginx的默认配置文件，只是对worker
process的数量还有worker_cpu_affinity配置项进行了改动，然后用apache的ab程序来进行请求测试:

```
ab -n 50000 -c 500 http://10.211.55.4/
```

测试的结果总体上和我写得程序的测试结果相近，两组配置程序的表现差距都在误差范围内，但是数值上设置了cpu亲和性的一组总是要偏低。


##总结
后来我又在网上看了很多的相关资料，发现linux的内核进程调度器天生具有被称为软cpu亲和性的特性，这意味着进程通常不会在处理器之间频繁迁移。并且，设置cpu亲和性和提高性能两者并没有直接的关系，它只能尽可能地提高cpu的利用率，就算是提高cpu利用率这一点也是无法保证的。也许就像一位linux内核开发者说的：“猜测内核的行为是一件非常困难的事情。”

而根据网上的优化案例，nginx的cpu亲和性设置确实在某些时候和某些环境下能够提升程序的性能，程序的优化很多时候也并不仅仅只是改一个参数那么简单，还是要在具体的硬件和软件环境下进行测试才能够找出最优的方案。
