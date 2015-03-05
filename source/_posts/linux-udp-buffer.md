title: linux中udp缓冲区大小的查看与设置
tags: 
- linux
- udp
- buffer
category: coding
date: 2015-02-13 21:37
---
鉴于tcp有重传机制，更多的时候udp对收发缓冲区的大小可能更加敏感一点。

udp缓冲区的大小主要和以下几个值有关：
1. /proc/sys/net/core/rmem_max ------ udp缓冲区的最大值，单位字节，下同
2. /proc/sys/net/core/rmem_default ------- udp缓冲区的默认值，如果不更改的话程序的udp缓冲区默认值就是这个。

查看方法可以直接`cat`以上两个文件进行查看，也可以通过`sysctl`查看。
<!-- more -->
```
sysctl -a | grep rmem_max
```
其实sysctl信息来源就是`proc`下的文件。

##更改udp缓冲区大小
###程序中进行更改
程序中可以使用setsockopt函数与SO_RCVBUF选项对udp缓冲区的值进行更改，但是要注意不管设置的值有多大，超过rmem_max的部分都会被无视。
```
int a = value_wanted;
if (setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &a, sizeof(int)) == -1) {
    ...
}
```
###更改系统值
如果确实要把udp缓冲区改到一个比较大的值，那就需要更改rmem_max的值。
编辑/etc/rc.local文件添加以下代码可使系统在每次启动的时候自动更改系统缓冲区的最大值。
```
echo value_wanted > /proc/sys/net/core/rmem_default
```
或者在/etc/sysctl.conf添加以下代码即可在重启后永久生效。
```
rmem_max=MAX
```
不想重启的话使用命令`sysctl -p`即可。

可以顺便看下setsockopt在linux下的相关实现
```
...

case SO_SNDBUF:
    if (val > sysctl_wmem_max)
        val = sysctl_wmem_max;

    if ((val * 2) < SOCK_MIN_SNDBUF)
        sk->sk_sndbuf = SOCK_MIN_SNDBUF;
    else
        sk->sk_sndbuf = val * 2; //当然缓冲区在系统中的实际值要大一点，因为udp报头以及IP报头等都是需要空间的。
...
```
