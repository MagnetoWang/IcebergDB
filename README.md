# IcebergDB
IcebergDB was created for managing deep learning data

## 单机版本

### 环境

- Linux和Mac
- 调用tablet接口支持本机运行

### 功能

- put
- get
- delete
- update

## 集群版本

### 环境

- Linux和Mac
- 搭建zk集群和配置好ip和端口
- 调用ibdb的接口

### 功能

- put
- get

## 参考

LevelDB的跳表

Kafka的日志存储

protobuf的通信协议

brpc的通信框架

zookeeper的分布式锁

## 注意

```
只有Base库部分代码是基于leveldb修改
在此声明原代码的copyright以及本人修改部分代码的copyright
leveldb 声明
Copyright (c) 2011 The LevelDB Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file. See the AUTHORS file for names of contributors.

其余库所有代码，本人使用github用户名作为声明的Copyright
Copyright (C) 2019 MagnetoWang. All rights reserved.
Use of this source code is governed by a GPL-style license that can be
found in the LICENSE file.
Author MagnetoWang
Date 2019-03-02
```

