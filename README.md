# domain_parser
提取全域名中的根域名，例如www.best.com.cn 提取根域名为 best.com.cn

需求来源：

    最近因为工作需要判断一个域名是否备案，实际提取的域名就是HTTP报文中的Host的内容，而判断一个域名是否是根据根域名进行的。例如访问www.qq.com,提取Host的内容为www.qq.com，而判断这个域名是否备案，是通过qq.com进行，因此需要从Host内容中提取出根域名。
    
遇到的问题

1、顶级域名的种类存在以下不同情况，例如 www.google.com    www.google.com.cn 顶级域名分别是.com 和.com.cn提取顶级名分别为google.com  goolge.com.cn

2、Host的长度不一，例如 api.best.com   upload.api.best.com 提取的根域名都为best.com

解决思路：
    由于程序是用C语言实现，所以就写一个C语言的lib库了。首先顶级域名是公开的，可以参考维基百科https://zh.wikipedia.org/wiki/%E4%BA%92%E8%81%94%E7%BD%91%E9%A1%B6%E7%BA%A7%E5%9F%9F%E5%88%97%E8%A1%A8
使用hash表将顶级域名存储起来，方便后面查找顶级域名在O（1）时间内找出来。

解析Host， 例如 api.upload.qq.com  大概的思路如下：

1、先计算出域名中每个点(.)在字符串中的位置

2、然后根据Host中点个个数提取出顶级域名，判断顶级域名是否在hash表

3、找到顶级域名后，再提取顶级域名的根域名，组合起来就组成了最终的结果

