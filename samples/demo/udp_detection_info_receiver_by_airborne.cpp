#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>

#define PORT 12346       // UDP端口号

// 枚举定义消息类型
enum MessageType {
    TIMESTAMP = 0x00,
    CLICK_COORDINATES = 0x01,
    TARGET_BOX_COORDINATES = 0x02,
    CLICK_COORDINATES_FRAME_ID = 0x03,
    TARGET_BOX_COORDINATES_FRAME_ID = 0x04
};

// 定义消息结构体
#pragma pack(1) // 使用1字节对齐方式
struct Message {
    unsigned char header[2];   // 帧头
    unsigned char type;        // 消息类型
    unsigned char reserved;    // 保留字段
    unsigned int requestID;    // 请求ID
    unsigned int dataLength;   // 数据长度
    unsigned char* data;       // 数据段
};
#pragma pack() // 恢复默认对齐方式

// 解析时间戳消息
void parseTimestampMessage(unsigned char* data) {
    unsigned short year = ntohs(*reinterpret_cast<unsigned short*>(data));
    unsigned char month = *(data + 2);
    unsigned char day = *(data + 3);
    unsigned char hour = *(data + 4);
    unsigned char minute = *(data + 5);
    unsigned char second = *(data + 6);
    unsigned short millisecond = ntohs(*reinterpret_cast<unsigned short*>(data + 7));

    std::cout << "Timestamp: " << static_cast<int>(year) << "-" << static_cast<int>(month)
              << "-" << static_cast<int>(day) << " " << static_cast<int>(hour) << ":"
              << static_cast<int>(minute) << ":" << static_cast<int>(second) << "."
              << static_cast<int>(millisecond) << std::endl;
}

// 解析点击坐标消息
void parseClickCoordinatesMessage(unsigned char* data) {
    float x = *reinterpret_cast<float*>(data);
    float y = *reinterpret_cast<float*>(data + 4);

    std::cout << "Click coordinates: (" << x << ", " << y << ")" << std::endl;
}

// 解析目标框坐标消息
void parseTargetBoxCoordinatesMessage(unsigned char* data) {
    float x1 = *reinterpret_cast<float*>(data);
    float y1 = *reinterpret_cast<float*>(data + 4);
    float x2 = *reinterpret_cast<float*>(data + 8);
    float y2 = *reinterpret_cast<float*>(data + 12);

    std::cout << "Target box coordinates: (" << x1 << ", " << y1 << ") - ("
              << x2 << ", " << y2 << ")" << std::endl;
}

// 解析UDP数据包
void parseUDPData(unsigned char* buffer, int length) {
    if (length < 13) {
        std::cout << "Invalid UDP data format" << std::endl;
        return;
    }

    Message message;
    memcpy(&message, buffer, sizeof(Message));

    // 解析消息类型
    MessageType messageType = static_cast<MessageType>(message.type);

    // 根据消息类型处理数据
    switch (messageType) {
        case TIMESTAMP:
            if (length != 9) {
                std::cout << "Invalid timestamp message length" << std::endl;
                return;
            }
            parseTimestampMessage(message.data);
            break;
        case CLICK_COORDINATES:
            if (length != 13) {
                std::cout << "Invalid click coordinates message length" << std::endl;
                return;
            }
            parseClickCoordinatesMessage(message.data);
            break;
        case TARGET_BOX_COORDINATES:
            if (length != 21) {
                std::cout << "Invalid target box coordinates message length" << std::endl;
                return;
            }
            parseTargetBoxCoordinatesMessage(message.data);
            break;
        default:
            std::cout << "Unsupported message type" << std::endl;
            break;
    }
}

int main() {
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    unsigned char buffer[4096];

    // 创建UDP套接字
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    // 绑定UDP套接字到端口
    bind(sockfd, (const struct sockaddr*)&servaddr, sizeof(servaddr));

    while (true) {
        socklen_t len = sizeof(cliaddr);

        // 接收UDP数据包
        ssize_t n = recvfrom(sockfd, buffer, sizeof(buffer), MSG_WAITALL,
                             (struct sockaddr*)&cliaddr, &len);

        // 解析UDP数据包
        parseUDPData(buffer, n);
    }

    close(sockfd);

    return 0;
}
