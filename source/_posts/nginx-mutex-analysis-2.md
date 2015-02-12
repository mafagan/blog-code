title: nginx中的accept-mutex源码分析(二)
tags: 
- nginx
- mutex
- algorithm
category: coding
date: 2015-02-05 22:24
---
在上篇文章里我们了解了nginx使用accept-mutex解决『惊群』问题的方法，其实在nginx的架构中，accept-mutex还起到了另外一个作用，负载均衡。
###worker process的负载均衡
在nginx抢夺accept-mutex之前有一个判断条件
```
if (ngx_accept_disabled > 0) {
    ngx_accept_disabled--;                
} else {...}
```
很显然，如果ngx_use_accept_mutex为正，这个worker会放弃这一次抢夺mutex的机会。而在每次抢夺mutex之前(或者说抢夺mutex之后)，worker都会对ngx_accept_disabled这个变量的值进行计算，算法如下
```
ngx_accept_disabled = ngx_cycle->connection_n / 8
                        - ngx_cycle->free_connection_n;

```
connection_n表示的是最大连接数，free_connection_n表示的就是剩余连接数，因此当剩余连接少于总连接数的八分之一，也就是已用连接数多于总连接数的八分之七，worker就会放弃这一次抢夺mutex的机会。那如果所有的worker的connection都超过八分之七哪怎么办？会不会导致整个nginx停止工作？这个你可以放心，注意代码里有这么一行`ngx_accept_disabled--;`，很明显worker不会一直放弃抢夺mutex的机会，因为每放弃一次，该变量的值都会减少一次，直至小于0.                
注：最大连接数在nginx中是一个可配置选项，名为worker connections，下面是官方的解释，
> Sets the maximum number of simultaneous connections that can be opened
> by a worker process.

###结语
之前对这一部分的代码一直很好奇很期待，现在了解了之后倒是觉得有一点点失望。不过，这里的负载均衡指的是nginx作为服务器时使用的平衡各个worker负载的方法，至于我们用得更多的nginx作为反向代理服务器使用的负载均衡是完全不同的方法，有空再研究下里面的代码。
