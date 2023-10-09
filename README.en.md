<img src="https://pic.imgdb.cn/item/64e42356661c6c8e549bab4f.jpg" alt="SpireCV logo" align="right" height="90" />

# SpireCV Intelligent perception algorithm library

## Overview

SpireCV is an **real-time edge perception SDK** built for **intelligent unmanned systems**, with main functions including **camera/pod control**, **video saving and push streaming**, **target detection and tracking**, and **edge data management**. It aims to provide mobile robot developers with high-performance, high-reliability, simple interface and feature-rich visual perception capabilities.

 - Github：https://github.com/amov-lab/SpireCV
 - Gitee：https://gitee.com/amovlab/SpireCV
 - **Maintaining an open-source project is not easy, please click star to support us, thanks! **

## Quick start

 - Installation and use: [SpireCV user manual](https://docs.amovlab.com/Spire_CV_Amov/#/)、[SpireCV developer kit guide](https://docs.amovlab.com/spirecvkit/#/)
    - Basic knowledge of C++ language and CMake compilation tool is required
    - Need to master the foundation of OpenCV vision library and understand the computational libraries such as CUDA, OpenVINO, RKNN and CANN
    - Need to understand the basic concepts and basic operation of ROS

 - Q&A and communication:
    - Q&A forum (official regular Q&A, recommended): [Amovlab Community - SpireCV Q&A Zone](https://bbs.amovlab.com/)
    - Add WeChat yinyue199506 (note: SpireCV) into the SpireCV intelligent perception algorithm library exchange group
    - Search and subscribe "Amovlab" on YouTube.com, we will update the video from time to time

## Framework

#### The main framework is shown in the figure:

<img width="640" src="https://pic.imgdb.cn/item/64f7112c661c6c8e54b25974.webp"/>

#### Current support:
 - **Functional level**：
    - [x] Video algorithm module (providing perceptual algorithms with unified interfaces, efficient performance and diverse functions)
    - [x] Video input, save and push stream module (to provide stable, cross-platform video reading and writing capabilities)
    - [x] Camera and pod control module (for the typical hardware ecology to open the interface, easy to use)
    - [x] Sensing information interaction module (providing UDP communication protocol)
    - [x] [ROS interface](https://gitee.com/amovlab1/spirecv-ros.git)
 - **Platform level**：
    - [x] X86 + Nvidia GPUs (10 series, 20 series, and 30 series graphics cards recommended)
    - [x] Jetson (AGX Orin/Xavier、Orin NX/Nano、Xavier NX)
    - [ ] Intel CPU (coming soon)
    - [ ] Rockchip (coming soon)
    - [ ] HUAWEI Ascend (coming soon)

## Demos
 - **QR code detection**
 
    <img width="400" src="https://pic.imgdb.cn/item/64f700f3661c6c8e54adcb83.gif"/>
 - **Landing mark detection**
  
    <img width="400" src="https://pic.imgdb.cn/item/64f700f0661c6c8e54adc964.gif"/>
 - **Ellipse detection**
  
    <img width="400" src="https://pic.imgdb.cn/item/64f700f2661c6c8e54adca9f.gif"/>
 - **Target click tracking (including target detection and tracking)**
  
    <img width="400" src="https://pic.imgdb.cn/item/64f700f1661c6c8e54adc9ae.gif"/>
 - **Low latency push streaming**
  
    <img width="400" src="https://pic.imgdb.cn/item/64f700f2661c6c8e54adcad2.gif"/>

## Copyright statement

 - SpireCV is protected under the Apache License 2.0.
 - SpireCV is for personal use only, please do NOT use it for commercial purposes.
 - If this project is used for profit, Amovlab will pursue infringement.
