<img src="https://pic.imgdb.cn/item/64e42356661c6c8e549bab4f.jpg" alt="SpireCV logo" align="right" height="90" />

# SpireCV 智能感知算法库

## 项目概况

SpireCV是一个专为**智能无人系统**打造的**边缘实时感知SDK**，主要功能包括**相机/吊舱控制**、**视频保存与推流**、**目标探测识别与跟踪**、**边缘数据管理迭代**等。旨在为移动机器人开发者提供高性能、高可靠、接口简洁、功能丰富的视觉感知能力。

 - Github：https://github.com/amov-lab/SpireCV
 - Gitee：https://gitee.com/amovlab/SpireCV
 - **开源项目，维护不易，还烦请点一个star收藏，谢谢支持！**

## 快速入门

 - 安装及使用：[SpireCV使用手册](https://docs.amovlab.com/Spire_CV_Amov/#/)、[SpireCV开发者套件指南](https://docs.amovlab.com/spirecvkit/#/)
    - 需掌握C++语言基础、CMake编译工具基础。
    - 需要掌握OpenCV视觉库基础，了解CUDA、OpenVINO、RKNN和CANN等计算库。
    - 需要了解ROS基本概念及基本操作。

 - 答疑及交流：
    - 答疑论坛（官方定期答疑，推荐）：[阿木社区-SpireCV问答专区](https://bbs.amovlab.com/)
    - 添加微信yinyue199506（备注消息：SpireCV）进入SpireCV智能感知算法库交流群。
    - B站搜索并关注“阿木社区”，开发团队定期直播答疑。

## 项目框架

#### 主要框架如图所示：

<img width="640" src="https://pic.imgdb.cn/item/64802a0d1ddac507cca5223c.jpg"/>

#### 目前支持情况：
 - **功能层**：
    - [x] 视频算法模块（提供接口统一、性能高效、功能多样的感知算法）
    - [x] 视频输入、保存与推流模块（提供稳定、跨平台的视频读写能力）
    - [x] 相机、吊舱控制模块（针对典型硬件生态打通接口，易使用）
    - [x] 感知信息交互模块（提供UDP通信协议）
    - [x] [ROS接口](https://gitee.com/amovlab1/spirecv-ros.git)
 - **平台层**：
    - [x] X86+Nvidia GPU（推荐10系、20系、30系显卡）
    - [x] Jetson（AGX Orin/Xavier、Orin NX/Nano、Xavier NX）
    - [ ] Intel CPU（推进中）
    - [ ] Rockchip（推进中）
    - [ ] HUAWEI Ascend（推进中）

## 功能展示
 - **二维码检测**
 
    <img width="400" src="https://pic.imgdb.cn/item/64f700f3661c6c8e54adcb83.gif"/>
 - **起降标志检测**
  
    <img width="400" src="https://pic.imgdb.cn/item/64f700f0661c6c8e54adc964.gif"/>
 - **椭圆检测**
  
    <img width="400" src="https://pic.imgdb.cn/item/64f700f2661c6c8e54adca9f.gif"/>
 - **目标点击跟踪（含目标检测、目标跟踪）**
  
    <img width="400" src="https://pic.imgdb.cn/item/64f700f1661c6c8e54adc9ae.gif"/>
 - **低延迟推流**
  
    <img width="400" src="https://pic.imgdb.cn/item/64f700f2661c6c8e54adcad2.gif"/>

## 版权声明

 - 本项目受 Apache License 2.0 协议保护。
 - 本项目仅限个人使用，请勿用于商业用途。
 - 如利用本项目进行营利活动，阿木实验室将追究侵权行为。
