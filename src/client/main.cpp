#include <atomic>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <functional>
#include <iostream>
#include <semaphore.h>
#include <string>
#include <thread>
#include <unistd.h>
#include <unordered_map>
#include <vector>

#include <arpa/inet.h>
#include <sys/socket.h>

#include "Group.h"
#include "Public.h"
#include "User.h"
#include "json.hpp"
using json = nlohmann::json;

// Controlling the main menu page program
bool isMainMenuRunning = false;

// Record current login user info
User g_currentUser;
// Record the friend list information of the currently logged-in user
std::vector<User> g_currentUserFriendList;
// Record the group list information of the currently logged-in user
std::vector<Group> g_currentUserGroupList;

// For communication between read and write threads
sem_t rwsem;
// record the login status
std::atomic_bool g_isLoginSuccess{false};

// receive thread
void readTaskHandler(int clientfd);

// Get system time (chat messages need to add time information)
std::string getCurrentTime();

// Main Chat Page Program
void mainMenu(int);

// Display the basic information of the current user who has successfully logged in
void showCurrentUserData();


int main(int argc, char **argv) {
    if (argc < 3) {
        std::cerr << "Command ia invalid! Example: ./ChatClient 127.0.0.1 6000" << std::endl;
        exit(-1);
    }

    // Parsing ip and port passed with command line arguments
    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);

    // Create client-side socket
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd == -1) {
        std::cerr << "Socket creation error" << std::endl;
        exit(-1);
    }

    // Fill the server ip and port information that client needs to connect to
    sockaddr_in server;
    memset(&server, 0, sizeof(sockaddr_in));

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);

    // Connect client to server
    if (connect(clientfd, (sockaddr *) &server, sizeof(sockaddr_in)) == -1) {
        std::cerr << "Connect to server error" << std::endl;
        close(clientfd);
        exit(-1);
    }

    // Initialize semaphores for read/write thread communication
    sem_init(&rwsem, 0, 0);

    // Connect to server successful, start the reception sub-thread
    std::thread readTask(readTaskHandler, clientfd);
    readTask.detach();

    // The main thread is used to receive user input and is responsible for sending data
    while (true) {
        // Show Home Menu Login, Register, Logout
        std::cout << "========================" << std::endl;
        std::cout << "1. Login" << std::endl;
        std::cout << "2. Register" << std::endl;
        std::cout << "3. Quit" << std::endl;
        std::cout << "========================" << std::endl;
        std::cout << "Choice: ";
        int choice = 0;
        std::cin >> choice;
        std::cin.get();// Read out the carriage returns left in the buffer

        switch (choice) {
            case 1: {// Login service
                int id = 0;
                char pwd[50] = {0};
                std::cout << "Input user id: ";
                std::cin >> id;
                std::cin.get();// Read out the carriage returns left in the buffer
                std::cout << "Input user password: ";
                std::cin.getline(pwd, 50);

                json js;
                js["msgid"] = LOGIN_MSG;
                js["id"] = id;
                js["password"] = pwd;
                std::string request = js.dump();

                g_isLoginSuccess = false;

                int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
                if (len == -1) {
                    std::cerr << "Send login message error: " << request << std::endl;
                }

                // Waiting semaphore: notified here by the child thread after processing the login response message
                sem_wait(&rwsem);

                if (g_isLoginSuccess) {
                    // Go to the main chat menu page
                    isMainMenuRunning = true;
                    mainMenu(clientfd);
                }
            } break;
            case 2: {// Register service
                char name[50] = {0};
                char pwd[50] = {0};
                std::cout << "Input username: ";
                std::cin.getline(name, 50);
                std::cout << "Input user password: ";
                std::cin.getline(pwd, 50);

                json js;
                js["msgid"] = REG_MSG;
                js["name"] = name;
                js["password"] = pwd;
                std::string request = js.dump();

                int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
                if (len == -1) {
                    std::cerr << "Send registration message error:" << request << std::endl;
                }

                sem_wait(&rwsem);
            } break;
            case 3:// Quit service
                close(clientfd);
                sem_destroy(&rwsem);
                exit(0);
            default:
                std::cerr << "invalid input!" << std::endl;
                break;
        }
    }

    return 0;
}

// Handle response after registration
void doRegResponse(json &responsejs) {
    if (responsejs["errno"].get<int>() != 0) {// registration failure
        std::cerr << "Name already exists, registration error!" << std::endl;
    } else {// registration success
        std::cout << "Name registration success, userid is " << responsejs["id"] << ", do not forget!" << std::endl;
    }
}

// Handle login response
void doLoginResponse(json &responsejs) {
    if (responsejs["errno"].get<int>() != 0) {// login failure
        std::cerr << responsejs["errmsg"] << std::endl;
        g_isLoginSuccess = false;
    } else {// login success
        // record current user id and name
        g_currentUser.setId(responsejs["id"].get<int>());
        g_currentUser.setName(responsejs["name"]);

        // record the current user's friend list information
        if (responsejs.contains("friends")) {
            // initialize
            g_currentUserFriendList.clear();

            std::vector<std::string> vec = responsejs["friends"];
            for (std::string &str: vec) {
                json js = json::parse(str);
                User user;
                user.setId(js["id"].get<int>());
                user.setName(js["name"]);
                user.setState(js["state"]);
                g_currentUserFriendList.push_back(user);
            }
        }

        // record the current user's group information
        if (responsejs.contains("groups")) {
            // Initialize
            g_currentUserGroupList.clear();

            std::vector<std::string> vec1 = responsejs["groups"];
            for (std::string &groupstr: vec1) {
                json grpjs = json::parse(groupstr);
                Group group;
                group.setId(grpjs["id"].get<int>());
                group.setName(grpjs["groupname"]);
                group.setDesc(grpjs["groupdesc"]);

                std::vector<std::string> vec2 = grpjs["users"];
                for (std::string &userstr: vec2) {
                    GroupUser user;
                    json js = json::parse(userstr);
                    user.setId(js["id"].get<int>());
                    user.setName(js["name"]);
                    user.setState(js["state"]);
                    user.setRole(js["role"]);
                    group.getUsers().push_back(user);
                }

                g_currentUserGroupList.push_back(group);
            }
        }

        // show basic info of login user
        showCurrentUserData();

        // Displays offline messages of the current user: personal chat messages or group messages
        if (responsejs.contains("offlinemsg")) {
            std::vector<std::string> vec = responsejs["offlinemsg"];
            for (std::string &str: vec) {
                json js = json::parse(str);

                // time + [id] + name + " said: " + xxx
                if (ONE_CHAT_MSG == js["msgid"].get<int>()) {
                    std::cout << js["time"].get<std::string>()
                              << " [" << js["id"] << "]"
                              << js["name"].get<std::string>()
                              << " said: " << js["msg"].get<std::string>() << std::endl;
                } else {
                    std::cout << "Group Message[" << js["groupid"]
                              << "]: " << js["time"].get<std::string>()
                              << " [" << js["id"] << "]"
                              << js["name"].get<std::string>()
                              << " said: " << js["msg"].get<std::string>() << std::endl;
                }
            }
        }

        g_isLoginSuccess = true;
    }
}

// Receive message handler
void readTaskHandler(int clientfd) {
    for (;;) {
        char buffer[1024] = {0};
        int len = recv(clientfd, buffer, 1024, 0);// blocked
        if (-1 == len || 0 == len) {
            close(clientfd);
            exit(-1);
        }

        // Receive the data forwarded by ChatServer and deserialize it to generate a json data object
        json js = json::parse(buffer);
        int msgtype = js["msgid"].get<int>();

        if (ONE_CHAT_MSG == msgtype) {
            std::cout << js["time"].get<std::string>()
                      << " [" << js["id"] << "]"
                      << js["name"].get<std::string>()
                      << " said: " << js["msg"].get<std::string>() << std::endl;
            continue;
        }

        if (GROUP_CHAT_MSG == msgtype) {
            std::cout << "Group Message[" << js["groupid"]
                      << "]: " << js["time"].get<std::string>() << " ["
                      << js["id"] << "]" << js["name"].get<std::string>()
                      << " said: " << js["msg"].get<std::string>() << std::endl;
            continue;
        }

        if (LOGIN_MSG_ACK == msgtype) {
            doLoginResponse(js);// Business for Handling Login Responses
            sem_post(&rwsem);   // Notify the main thread that the login result processing is complete
            continue;
        }

        if (REG_MSG_ACK == msgtype) {
            doRegResponse(js);
            sem_post(&rwsem);// Notify the main thread that the registration result processing is complete
            continue;
        }
    }
}

// Display the basic information of the current user who has successfully logged in
void showCurrentUserData() {
    std::cout << "===========================LOGIN USER===========================" << std::endl;
    std::cout << "Current Login User => ID:" << g_currentUser.getId() << " Name:" << g_currentUser.getName() << std::endl;
    std::cout << "---------------------------FRIEND LIST--------------------------" << std::endl;
    if (!g_currentUserFriendList.empty()) {
        for (User &user: g_currentUserFriendList) {
            std::cout << user.getId() << " " << user.getName() << " " << user.getState() << std::endl;
        }
    }
    std::cout << "---------------------------GROUP LIST---------------------------" << std::endl;
    if (!g_currentUserGroupList.empty()) {
        for (Group &group: g_currentUserGroupList) {
            std::cout << "Group ID: "<< group.getId() << " Group Name: " << group.getName() << " Group Description: " << group.getDesc() << std::endl;
            std::cout << "User information of this group:" << std::endl;
            for (GroupUser &user: group.getUsers()) {
                std::cout << user.getId() << " " << user.getName()
                          << " " << user.getState()
                          << " " << user.getRole() << std::endl;
            }
        }
    }
    std::cout << "================================================================" << std::endl;
}

// "help" command handler
void help(int fd = 0, std::string str = "");

// "chat" command handler
void chat(int, std::string);

// "addfriend" command handler
void addfriend(int, std::string);

// "creategroup" command handler
void creategroup(int, std::string);

// "addgroup" command handler
void addgroup(int, std::string);

// "groupchat" command handler
void groupchat(int, std::string);

// "logout" command handler
void logout(int, std::string);

// List of client commands supported by the system
std::unordered_map<std::string, std::string> commandMap = {
        {"help", "List all supported commands. Format is: help"},
        {"chat", "One to one chat. Format is: chat:friendid:message"},
        {"addfriend", "Add friend. Format is: addfriend:friendid"},
        {"creategroup", "Create group. Format is: creategroup:groupname:groupdesc"},
        {"addgroup", "Join in a group. Format is: addgroup:groupid"},
        {"groupchat", "Group chat. Format is: groupchat:groupid:message"},
        {"logout", "Logout. Format is: logout"}};

// Registering system-supported client-side command processing
std::unordered_map<std::string, std::function<void(int, std::string)>> commandHandlerMap = {
        {"help", help},
        {"chat", chat},
        {"addfriend", addfriend},
        {"creategroup", creategroup},
        {"addgroup", addgroup},
        {"groupchat", groupchat},
        {"logout", logout}};

// main menu program
void mainMenu(int clientfd) {
    help();

    char buffer[1024] = {0};
    while (isMainMenuRunning) {
        std::cin.getline(buffer, 1024);
        std::string commandbuf(buffer);
        std::string command;// store the command
        int idx = commandbuf.find(':');
        if (idx == -1) {
            command = commandbuf;
        } else {
            command = commandbuf.substr(0, idx);
        }

        auto it = commandHandlerMap.find(command);
        if (it == commandHandlerMap.end()) {
            std::cerr << "Invalid input command!" << std::endl;
            continue;
        }

        // Calls the event handler callback for the corresponding command
        // mainMenu is closed to modification,
        // adding new functionality does not require modification of this function
        it->second(clientfd, commandbuf.substr(idx + 1, commandbuf.size() - idx));
    }
}

// "help" command handler
void help(int, std::string) {
    std::cout << "Show Command List >>> " << std::endl;
    for (auto &p: commandMap) {
        std::cout << p.first << " : " << p.second << std::endl;
    }
    std::cout << std::endl;
}

// "addfriend" command handler
void addfriend(int clientfd, std::string str) {
    int friendid = atoi(str.c_str());
    json js;
    js["msgid"] = ADD_FRIEND_MSG;
    js["id"] = g_currentUser.getId();
    js["friendid"] = friendid;
    std::string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len) {
        std::cerr << "Send addfriend message error -> " << buffer << std::endl;
    }
}

// "chat" command handler
void chat(int clientfd, std::string str) {
    int idx = str.find(':');// friendid:message
    if (-1 == idx) {
        std::cerr << "Chat command is invalid!" << std::endl;
        return;
    }

    int friendid = atoi(str.substr(0, idx).c_str());
    std::string message = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msgid"] = ONE_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["toid"] = friendid;
    js["msg"] = message;
    js["time"] = getCurrentTime();
    std::string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len) {
        std::cerr << "Send chat message error -> " << buffer << std::endl;
    }
}

// "creategroup" command handler  groupname:groupdesc
void creategroup(int clientfd, std::string str) {
    int idx = str.find(':');
    if (-1 == idx) {
        std::cerr << "creategroup command is invalid!" << std::endl;
        return;
    }

    std::string groupname = str.substr(0, idx);
    std::string groupdesc = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msgid"] = CREATE_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupname"] = groupname;
    js["groupdesc"] = groupdesc;
    std::string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len) {
        std::cerr << "Send creategroup message error -> " << buffer << std::endl;
    }
}

// "addgroup" command handler
void addgroup(int clientfd, std::string str) {
    int groupid = atoi(str.c_str());
    json js;
    js["msgid"] = ADD_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupid"] = groupid;
    std::string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len) {
        std::cerr << "Send addgroup message error -> " << buffer << std::endl;
    }
}

// "groupchat" command handler   groupid:message
void groupchat(int clientfd, std::string str) {
    int idx = str.find(':');
    if (-1 == idx) {
        std::cerr << "groupchat command is invalid!" << std::endl;
        return;
    }

    int groupid = atoi(str.substr(0, idx).c_str());
    std::string message = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msgid"] = GROUP_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["groupid"] = groupid;
    js["msg"] = message;
    js["time"] = getCurrentTime();
    std::string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len) {
        std::cerr << "send groupchat msg error -> " << buffer << std::endl;
    }
}

// "logout" command handler
void logout(int clientfd, std::string) {
    json js;
    js["msgid"] = LOGOUT_MSG;
    js["id"] = g_currentUser.getId();
    std::string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len) {
        std::cerr << "Send logout msg error -> " << buffer << std::endl;
    } else {
        isMainMenuRunning = false;
    }
}

// Get system time (time information needs to be added to chat messages)
std::string getCurrentTime() {
    auto timeInfo = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm *ptm = localtime(&timeInfo);

    char date[60] = {0};
    sprintf(date, "%d-%02d-%02d %02d:%02d:%02d",
            (int) ptm->tm_year + 1900, (int) ptm->tm_mon + 1, (int) ptm->tm_mday,
            (int) ptm->tm_hour, (int) ptm->tm_min, (int) ptm->tm_sec);

    return std::string(date);
}
