# Chat Server

## Requirements

- System requirements: Ubuntu 22.04.3 LTS
- Required software/packages:
  - cmake >= v3.22
  - [Muduo](https://github.com/chenshuo/muduo) == v2.0.2
  - MySQL >= v8.0.0
  - Redis
  - Nginx

## Setup Guide

### Install Muduo

Install required packages:

```shell
sudo apt-get install build-essential cmake
sudo apt-get install libboost-dev
sudo apt-get install libprotobuf-dev protobuf-compiler libprotoc-dev libgoogle-perftools-dev
sudo apt-get install libboost-test-dev libboost-program-options-dev libboost-system-dev
sudo apt-get install libc-ares-dev libcurl4-openssl-dev
sudo apt-get install zlib1g-dev libgd-dev
```

Build Muduo:

```shell
git clone https://github.com/chenshuo/muduo.git
cd muduo
./build.sh
./build.sh install
```

Install Muduo to the system:

```shell
cd ..
cd build
cd release-install-cpp11
cd include
mv muduo /usr/include
cd ..
cd lib
mv * /usr/local/lib
```

### Install MySQL Server

```shell
wget https://dev.mysql.com/get/mysql-apt-config_0.8.29-1_all.deb
sudo dpkg -i mysql-apt-config_0.8.29-1_all.deb
sudo apt-get update
sudo apt-get install mysql-server libmysqlclient-dev
```
