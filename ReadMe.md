# TransFile

文件传输工具，实现SOCKET文件传输功能，适用于局域网之间的文件传输，解决特殊端口被封或不支持剪切板复制文件时的数据传输。

## 功能特性

* 支持文件传输
* 支持消息发送
* 支持文件拖拽
* 支持数据互传

## 环境

Visual Studio 2010

## 使用方法

本程序自实现客户端和服务端，需配合使用。使用时，一方作为客户端，一方作为服务端，服务端先启动并监听端口，客户端再连上来。
然后相互之间可以互相发送数据，右键左边的主机列表或拖拽文件即可发送对应的文件，支持多文件同时发送。

## 运行截图示例

![启动图](https://github.com/arbboter/TransportFile/blob/master/Image//start.jpg)

![文件传输图](https://github.com/arbboter/TransportFile/blob/master/Image//file.jpg)