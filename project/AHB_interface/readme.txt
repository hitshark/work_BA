设计中需要注意的事项：
1、接口代码中除了异常（地址出错或其他情况）之外，没有其他的打印信息。如果要判断一次传输是否完成，可以从主设备读写数据是否相等来进行判断；
2、LT和AT的从设备实现的很简单，字节使能（对应于AHB协议中数据通道的选择）和大小端问题都没有考虑；周期精确模型的实现也很简单；
3、对事务级周期精确的理解，是否应该带有时钟？
4、大小端问题？
5、诸如总线宽度等参数的可配置？