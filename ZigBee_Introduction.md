
Table of Contents
=================

* [ZigBee介绍](#zigbee介绍)
* [ZigBee和IEEE 802.15.4 的关系](https://github.com/QuLeo/ZigBee/blob/master/ZigBee_Introduction.md#zigbee%E7%9A%84%E7%89%B9%E7%82%B9)
* [ZigBee的特点](#ZigBee的特点)
* [ZigBee无线网络通信信道分析](#ZigBee无线网络通信信道分析)
* [ZigBee的网络拓扑模型]()
* [ZigBee应用范围]()
* [CC2530芯片（入门级）]()

Created by [gh-md-toc](https://github.com/ekalinin/github-markdown-toc)

#### ZigBee介绍
&emsp;&emsp;近年来，由于无线接入技术的需求日益增大，无线通信和无线网络均呈现出指数增加的趋势。这有力的推动力无线通信向高速通信方向的发展。然而，工业、农业、车载电子系统、家用网络、医疗传感器和伺服执行机构等都是无线通信还未涉足或者刚刚涉足的领域。这些领域对数据吞吐量的要求很低，功率消耗也比现有标准提供的功率消耗低。此外，为了促使简单方便的、可以随意使用的无线装置大量涌现，需要在未来的个人活动空间内布置大量的无线接入点，因而低廉的价格将起到关键的作用。为了降低元器件的价格，以便于这些装置批量生产，有必要发展出一个标准的解决方案。这个标准要解决的问题是，设计一个**维持最小流量的通信链路**和**低复杂度的无线收发信机**；要考虑的核心问题是低功耗和低价格的设计。这就要求该标准应**提供低带宽低数据传输速率的应用**。 
#### ZigBee和IEEE 802.15.4 的关系
IEEE 802.15.4标准的优点:     
&emsp;&emsp;A：低功耗        
&emsp;&emsp;B：低价格        
&emsp;&emsp;C：低数据传输率 
IEEE 802.15.4标准制定小组的任务:        
&emsp;&emsp;A：物理层 (DSSS)：数据的调制发送和接收解调，介质选择，信道选择。        
&emsp;&emsp;B：MAC 层 (CSMA/CA)：产生网络信标，支持设备的安全性等。
 
&emsp;&emsp;ZigBee是建立在 IEEE 802.15.4 标准之上，由于 IEEE 802.15.4标准只定义了**物理层协议**和**MAC 层协议**，于是成立了zigbee联盟，ZigBee联盟对其**网络层协议**和 **API** 进行了标准化，还开发了**安全层**。经过ZigBee联盟对 IEEE 802.15.4 的改进，这才真正形成了**ZigBee协议栈**(Zstack)。
![在这里插入图片描述](https://img-blog.csdnimg.cn/20190320005944866.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3dlaXhpbl80MTg5MDk3MQ==,size_10,color_FFFFFF,t_70)
#### ZigBee的特点
数据传输速率低：10KB/秒~250KB /秒，专注于低传输应用。
功耗低：在低功耗待机模式下，两节普通5号电池可使用6~24个月。  
成本低：ZigBee数据传输速率低，协议简单，所以大大降低了成本。  
网络容量大：网络可容纳 65,000 个设备。 
时延短：通常时延都在 15ms~30ms。 
安全： ZigBee提供了数据完整性检查和鉴权功能，采用AES-128加密算法（美国新加密算法，是目前最好的文本加密算法之一） 
![在这里插入图片描述](https://img-blog.csdnimg.cn/20190320005916693.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3dlaXhpbl80MTg5MDk3MQ==,size_12,color_FFFFFF,t_70)
#### ZigBee无线网络通信信道分析 
&emsp;&emsp;天线对于无线通信系统来说至关重要，在日常生活中可以看到各式各样的天线，如手机天线、电视接收天线等，天线的主要功能可以概括为:完成无线电波的发射与接收。发射时，把高频电流转换为电磁波发射出去；接收时，将电磁波转换为高频电流。如何区分不同的电波呢？ 一般情况，不同的电波具有不同的频谱，无线通信系统的频谱有几十兆赫兹到几千兆赫兹，包括了收音机、手机、卫星电视等使用的波段，这些电波都使用空气作为传输介质来传播，为了防止不同的应用之间相互干扰，就需要对无线通信系统的通信信道进行必要的管理。各个国家都有自己的无线管理结构，如美国的联邦通信委员会（FCC）、欧洲的典型标准委 员会（ETSI）。我国的无线电管理机构为中国无线电管理委员会，其主要职责是负责无线电频率的划分、分配与指配、卫星轨道位置协调和管理、无线电监测、检测、干扰查处，协调处理电磁干扰事宜和维护空中电波秩序等。一般情况，使用某一特定的频段需要得到无限电管理部门的许可，当然，各国的无线电管理部门也规定了一部分频段是对公众开放的，不需要许可使用，以满足不同的应用需求， 这些频段包括ISM（Industrial、Scientific and Medical——工业、科学和医疗）频带。除了 ISM 频带外，在我国，低于135KHz，在北美、日本等地，低于 400KHz 的频带也是免费频段。各国对无线电频谱的管理不仅规定了 ISM 频带的频率，同时也规定了在这些频 带上所使用的发射功率，在项目开发过程中，需要查阅相关的手册，如我国信息产业部发布 的《微功率（短距离）无线电设备管理规定》。 
&emsp;&emsp;IEEE 802.15.4（ZigBee）工作在 ISM 频带，定义了两个频段，2.4GHz频段和896/915MHz频带。在IEEE 802.15.4中共规定了27个信道：  
&emsp;&emsp;在2.4GHz频段，共有16个信道，信道通信速率为250kbps；  
&emsp;&emsp;在915MHz频段，共有10个信道，信道通信速率为40kbps；  
&emsp;&emsp;在896MHz频段，有1个信道，信道通信速率为20kbps。
![在这里插入图片描述](https://img-blog.csdnimg.cn/20190320092443717.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3dlaXhpbl80MTg5MDk3MQ==,size_16,color_FFFFFF,t_70)
 #### ZigBee的网络拓扑模型 
 
 &emsp;&emsp;ZigBee网络拓扑结构主要有**星形网络**和**网型网络**。不同的网络拓扑对应于不同的应用领域，在ZigBee无线网络中，不同的网络拓扑结构对网络节点的配置也不同，网络节点的类型：**协调器**、**路由器**和**终端节点**。
 &emsp;&emsp;MESH网状网络拓扑结构的网络具有强大的功能，网络可以通过多级跳的方式来通信；该拓扑结构还可以组成极为复杂的网络；网络还具备自组织、自愈功能。 
 ![在这里插入图片描述](https://img-blog.csdnimg.cn/20190320092540786.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3dlaXhpbl80MTg5MDk3MQ==,size_16,color_FFFFFF,t_70)
#### ZigBee应用范围 
&emsp;&emsp;ZigBee已广泛应用于物联网产业链中的M2M行业，如智能电网、智能交通、智能家 居、金融、移动 POS 终端、供应链自动化、工业自动化、智能建筑、消防、公共安全、环境保护、气象、数字化医疗、遥感勘测、农业、林业、水务、煤矿、石化等领域。 
![在这里插入图片描述](https://img-blog.csdnimg.cn/20190320092818928.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3dlaXhpbl80MTg5MDk3MQ==,size_16,color_FFFFFF,t_70)
#### CC2530芯片（入门级）
CC2530芯片主要特点：  
&emsp;&emsp;高性能、低功耗的8051微控制器内核；  
&emsp;&emsp;适应2.4GHz IEEE 802.15.4的RF收发器；  
&emsp;&emsp;电源电压范围宽（2.0～3.6V）；  
&emsp;&emsp;看门狗、电池监视器和温度传感器；  
&emsp;&emsp;具有8路输入8～14位ADC；  
&emsp;&emsp;2个串行, 1个红外发生电路； 
&emsp;&emsp;1个通用的16位和2个8位定时器；  
&emsp;&emsp;高级加密标准（AES）协处理器；  
&emsp;&emsp;21个通用 I/O 引脚，2个具有20mA的电流吸收或电流供给能力； 
&emsp;&emsp;小尺寸QLP-40封装，6mm×6mm。 

&emsp;&emsp;最后，楼主是在深圳安联德购买开发板进行学习和研究工作的。大家不要觉得入门难。入门级ZigBee控制核心仅仅是一个增强型8051芯片，学过51郭天祥的都可以上手。
