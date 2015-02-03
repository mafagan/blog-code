title: 小丑的蝙蝠侠——YouCompleteMe
tags: vim
category: coding
date: 2015-01-06 15:36
---
YouCompleteMe(以下简称YCM), 目前vim上最优秀的补全插件。

###简介
vim上有很多补全插件，像ctags, cscope, neocomplete等。这些插件或多或少我都用过，总体感觉就是在瞎XX猜。相比之下，YCM与这些插件有着本质的区别，它基于语法分析，补全精准快速，即使是在百兆源码文件中使用也不会有显著的的效率下降，支持C家族全系语言，亲测还支持python、js等，可以说很大程度上解决了vim补全上的问题。
<!-- more -->
YCM由谷歌工程师Strahinja Val Markovic开发,使用的是C/S架构，在vim启动的时候服务端就会自动运行，正常情况下是感受不到YCM的存在的。YCM的语义分析使用的是clang(就是mac上的默认编译器)，C端使用C++开发，据说是为了提高效率，再由python进行封装，是不是觉得有点晕菜？没关系，其实你只要记住YCM很好用就行了。附一张效果图，btw，YCM还有补全路径、跳转到定义声明这些貌似不太起眼但是非常实用的功能，用起来感觉各种惊喜。
![ycm-demo](http://ccxcu.img43.wal8.com/img43/507748_20150118041318/142245596241.gif)

###mac下的安装
YCM的语义分析用的是clang，因此在YCM在mac的安装也是最简单直接的。
使用vundle安装YCM，在.vimrc文件中添加以下代码：
``` bash
Bundle 'Valloric/YouCompleteMe' 
```
打开vim输入
``` bash
:BundleInstall
```	
	
等待YCM下载完成。然后进入文件夹进行编译：
``` bash
cd ~/.vim/bundle/YouCompleteMe
./install --clang-completer --system-libclang
```
这里要注意一下**--system-libclang**这个参数，之前我编译的时候没有加入这个参数，YCM会下载默认的clang包导致编译出错，原因我也不清楚，不过加上之后YCM就会使用mac自带的clang，可以正常编译。

YCM是基于macvim开发的，作者也是建议使用macvim以保证script的正确执行，不过貌似直接使用vim也不会有很大的问题，如果实在不放心，可以安装macvim之后在.bash_profile下加入以下代码:
``` bash	
alias vim='/Applications/MacVim.app/Contents/MacOS/Vim -v'
```
这样在命令行中使用的vim都会是macvim的vim。记住不要使用软连接，这样子macvim的vim没有办法正常启动。

###配置
YCM的默认配置文件是在.vim文件夹中的.ycm_extra_conf.py,在项目根目录下放同名文件就可以对项目进行特定的配置，其实最要就是配置头文件的路径，至于其它更详细的配置，可以参阅YCM的[项目主页](https://github.com/Valloric/YouCompleteMe)。

###体验
* 流畅，在大项目中尤其明显
* 可对路径进行补全，这个大爱
* 语法检测，说实话我觉得这个有点烦
* 定义/声明跳转，YCM的声明跳转没有问题，不过定义跳转会时不时出现失败的情况，作者的解释是如果定义不在一个翻译单元内确实是无法补全的，这个也没有办法了

###后记
之前一直觉得YouCompleteMe这句话有点耳熟，直到前几天一不小心在Acfun上看到，终于知道是神马回事了。

![joker](http://ccxcu.img43.wal8.com/img43/507748_20150118041318/142245596287.jpg)





