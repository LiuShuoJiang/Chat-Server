# Chat Server

## Brief Introduction

A clustered chat server and command line client that can work in a Nginx TCP load balanced environment. 

Functions such as new user registration, user login, adding friends, adding groups, friend communication, group chat, and keeping offline messages are implemented.

- Developed the core network module based on Muduo network library, achieving efficient communication. 
- Utilized a third-party JSON library for serialization and deserialization of communication data. 
- Employed Nginx's TCP load balancing to distribute client requests across multiple servers, enhancing concurrent processing capabilities. 
- Resolved cross-server communication challenges using a publish-subscribe server middleware, Redis message queue. 
- Encapsulated the MySQL interface to store user data on disk, achieving data persistence.

## Requirements

- System requirements: Ubuntu 22.04.3 LTS
- Required software/packages:
  - cmake >= v3.22
  - [Muduo](https://github.com/chenshuo/muduo) == v2.0.2
  - MySQL >= v8.0.0
  - Redis
  - Nginx >= 1.24.0

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

To configure databases, see [Database Design](#mysql-database-table-design) section.

### Install Redis

```shell
curl -fsSL https://packages.redis.io/gpg | sudo gpg --dearmor -o /usr/share/keyrings/redis-archive-keyring.gpg
echo "deb [signed-by=/usr/share/keyrings/redis-archive-keyring.gpg] https://packages.redis.io/deb $(lsb_release -cs) main" | sudo tee /etc/apt/sources.list.d/redis.list

sudo apt-get update
sudo apt-get install redis
sudo apt-get install libhiredis-dev
```

### Install and Configure Nginx

#### Installation from Source

```shell
sudo apt-get update
sudo apt install libpcre3 libpcre3-dev
wget https://nginx.org/download/nginx-1.24.0.tar.gz
tar zxvf nginx-1.24.0.tar.gz
cd nginx-1.24.0/
./configure --with-stream
make
sudo make install
```

#### Configure Load Balance

```shell
cd /usr/local/nginx/conf
```

Then add following contents to `nginx.conf` file:

```
#nginx TCP load balance config
stream {
    upstream MyServer {
        server 127.0.0.1:6000 weight=1 max_fails=3 fail_timeout=30s;
        server 127.0.0.1:6002 weight=1 max_fails=3 fail_timeout=30s;
    }

    server {
        proxy_connect_timeout 1s;
        #proxy timeout 3s;
        listen 8000;
        proxy_pass MyServer;
        tcp_nodelay on;
    }
}
```

You may change `127.0.0.1:6000` and `127.0.0.1:6002` above to the actual IPs and ports.

### Build Project

```shell
git clone https://github.com/LiuShuoJiang/Chat-Server.git
cd Chat-Server
./build.sh
```

### (Example Test) Start the Server and Client

Change the current directory of shell to `{PROJECT_SOURCE_DIR}/bin` first.

Terminal 1:

```shell
./Chat_Server 127.0.0.1 6000
```

Terminal 2:

```shell
./Chat_Server 127.0.0.1 6002
```

Terminal 3:

```shell
./Chat_Client 127.0.0.1 8000
```

Then start the chat.

Terminal 4:

```shell
./Chat_Client 127.0.0.1 8000
```

Then start the chat.

## MySQL Database Table Design

Create a database named `chat` first. Then use this database.

**User Table**

|  Field Name  |         Field Type          |      Description       |         Constraints          |
|:------------:|:---------------------------:|:----------------------:|:----------------------------:|
|      id      |             INT             |        user id         |  PRIMARY KEY、AUTO_INCREMENT  |
|     name     |         VARCHAR(50)         |       user name        |       NOT NULL, UNIQUE       |
|   password   |         VARCHAR(50)         |     user password      |           NOT NULL           |
|    state     |  ENUM('online', 'offline')  |  current login status  |      DEFAULT 'offline'       |

**Friend Table**

|  Field Name  |  Field Type  |  Description  |          Constraints          |
|:------------:|:------------:|:-------------:|:-----------------------------:|
|    userid    |     INT      |    user id    |  NOT NULL, joint primary key  |
|   friendid   |     INT      |   friend id   |  NOT NULL, joint primary key  |

**AllGroup Table**

|  Field Name  |   Field Type   |     Description     |         Constraints          |
|:------------:|:--------------:|:-------------------:|:----------------------------:|
|      id      |      INT       |      group id       |  PRIMARY KEY、AUTO_INCREMENT  |
|  groupname   |  VARCHAR(50)   |     group name      |       NOT NULL, UNIQUE       |
|  groupdesc   |  VARCHAR(200)  |  group description  |          DEFAULT ''          |

**GroupUser Table**

|  Field Name  |         Field Type          |     Description     |          Constraints          |
|:------------:|:---------------------------:|:-------------------:|:-----------------------------:|
|   groupid    |             INT             |      group id       |  NOT NULL, joint primary key  |
|    userid    |             INT             |   group member id   |  NOT NULL, joint primary key  |
|  grouprole   |  ENUM('creator', 'normal')  |  role within group  |       DEFAULT 'normal'        |

**OfflineMessage Table**

|  Field Name  |  Field Type   |               Description               |  Constraints  |
|:------------:|:-------------:|:---------------------------------------:|:-------------:|
|    userid    |      INT      |                 user id                 |   NOT NULL    |
|   message    |  VARCHAR(50)  |  offline messages (store Json strings)  |   NOT NULL    |
