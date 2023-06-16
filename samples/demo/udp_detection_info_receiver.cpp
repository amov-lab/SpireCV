#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#define SERV_PORT 20166

typedef unsigned char byte;
using namespace std;


int main(int argc, char *argv[]) {
  sockaddr_in servaddr;
  int sockfd;


  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(SERV_PORT);
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));


  int upd_msg_len = 1024 * 6;  // max_objects = 100
  byte upd_msg[upd_msg_len];
  int msg_queue_len = 1024 * 1024;  // 1M
  byte msg_queue[msg_queue_len];

  int addr_len = sizeof(struct sockaddr_in);
  int start_index = 0, end_index = 0;

  while (1)
  {
    int n = recvfrom(sockfd, upd_msg, upd_msg_len, 0, (struct sockaddr *)&servaddr, reinterpret_cast<socklen_t*>(&addr_len));

    if (end_index + n > msg_queue_len)
    {
      int m = end_index - start_index;
      memcpy((void*) &msg_queue[0], (const void*) &msg_queue[start_index], (size_t) m);
      start_index = 0;
      end_index = m;
    }

    memcpy((void*) &msg_queue[end_index], (const void*) upd_msg, (size_t) n);
    end_index += n;

cout << n << ", " << start_index << ", " << end_index << endl;

    // processing
    while (start_index < end_index)
    {
      int i = start_index;
      if (i > 0 && msg_queue[i-1] == 0xFA && msg_queue[i] == 0xFC)  // frame start
      {
        cout << "FOUND 0xFAFC" << endl;
        i++;
        if (end_index - i >= 2)  // read length
        {
          unsigned short* len = reinterpret_cast<unsigned short*>(&msg_queue[i]);
          int ilen = (int) (*len);
          cout << "LEN: " << ilen << endl;
          if (end_index - i >= ilen + 2 && msg_queue[i+ilen] == 0xFB && msg_queue[i+ilen+1] == 0xFD)
          {
            cout << "FOUND 0xFAFC & 0xFBFD" << endl;
            byte* msg_type = reinterpret_cast<byte*>(&msg_queue[i+2]);
            cout << "Type: " << (int) *msg_type << endl;
            unsigned short* year = reinterpret_cast<unsigned short*>(&msg_queue[i+7]);
            byte* month = reinterpret_cast<byte*>(&msg_queue[i+9]);
            byte* day = reinterpret_cast<byte*>(&msg_queue[i+10]);
            byte* hour = reinterpret_cast<byte*>(&msg_queue[i+11]);
            byte* minute = reinterpret_cast<byte*>(&msg_queue[i+12]);
            byte* second = reinterpret_cast<byte*>(&msg_queue[i+13]);
            unsigned short* millisecond = reinterpret_cast<unsigned short*>(&msg_queue[i+14]);
            cout << "Time: " << *year << "-" << (int) *month << "-" << (int) *day << " " << (int) *hour << ":" << (int) *minute << ":" << (int) *second << " " << *millisecond << endl;
            
            byte* index_d1 = reinterpret_cast<byte*>(&msg_queue[i+16]);
            byte* index_d2 = reinterpret_cast<byte*>(&msg_queue[i+17]);
            byte* index_d3 = reinterpret_cast<byte*>(&msg_queue[i+18]);
            byte* index_d4 = reinterpret_cast<byte*>(&msg_queue[i+19]);
            int mp = i+20;
            if ((*index_d4) & 0x01 == 0x01)
            {
              int* frame_id = reinterpret_cast<int*>(&msg_queue[mp]);
              mp += 4;
              cout << "FrameID: " << *frame_id << endl;
            }
            if ((*index_d4) & 0x02 == 0x02 && (*index_d4) & 0x04 == 0x04)
            {
              int* width = reinterpret_cast<int*>(&msg_queue[mp]);
              mp += 4;
              int* height = reinterpret_cast<int*>(&msg_queue[mp]);
              mp += 4;
              cout << "FrameSize: (" << *width << ", " << *height << ")" << endl;
            }
            int n_objects = 0;
            if ((*index_d4) & 0x08 == 0x08)
            {
              n_objects = *reinterpret_cast<int*>(&msg_queue[mp]);
              mp += 4;
              cout << "N_Objects: " << n_objects << endl;
            }
            if ((*index_d4) & 0x10 == 0x10)
            {
              float* fps = reinterpret_cast<float*>(&msg_queue[mp]);
              mp += 4;
              cout << "FPS: " << *fps << endl;
            }
            if ((*index_d4) & 0x20 == 0x20 && (*index_d4) & 0x40 == 0x40)
            {
              float* fov_x = reinterpret_cast<float*>(&msg_queue[mp]);
              mp += 4;
              float* fov_y = reinterpret_cast<float*>(&msg_queue[mp]);
              mp += 4;
              cout << "FOV: (" << *fov_x << ", " << *fov_y << ")" << endl;
            }
            if ((*index_d4) & 0x80 == 0x80 && (*index_d3) & 0x01 == 0x01 && (*index_d3) & 0x02 == 0x02)
            {
              float* pod_patch = reinterpret_cast<float*>(&msg_queue[mp]);
              mp += 4;
              float* pod_roll = reinterpret_cast<float*>(&msg_queue[mp]);
              mp += 4;
              float* pod_yaw = reinterpret_cast<float*>(&msg_queue[mp]);
              mp += 4;
              cout << "POD-Angles: (" << *pod_patch << ", " << *pod_roll << ", " << *pod_yaw << ")" << endl;
            }
            if ((*index_d3) & 0x04 == 0x04 && (*index_d3) & 0x08 == 0x08 && (*index_d3) & 0x10 == 0x10)
            {
              float* longitude = reinterpret_cast<float*>(&msg_queue[mp]);
              mp += 4;
              float* latitude = reinterpret_cast<float*>(&msg_queue[mp]);
              mp += 4;
              float* altitude = reinterpret_cast<float*>(&msg_queue[mp]);
              mp += 4;
              cout << "UAV-Position: (" << *longitude << ", " << *latitude << ", " << *altitude << ")" << endl;
            }
            if ((*index_d3) & 0x20 == 0x20 && (*index_d3) & 0x40 == 0x40 && (*index_d3) & 0x80 == 0x80)
            {
              float* uav_vx = reinterpret_cast<float*>(&msg_queue[mp]);
              mp += 4;
              float* uav_vy = reinterpret_cast<float*>(&msg_queue[mp]);
              mp += 4;
              float* uav_vz = reinterpret_cast<float*>(&msg_queue[mp]);
              mp += 4;
              cout << "UAV-Speed: (" << *uav_vx << ", " << *uav_vy << ", " << *uav_vz << ")" << endl;
            }
            if ((*index_d2) & 0x01 == 0x01)
            {
              float* illumination = reinterpret_cast<float*>(&msg_queue[mp]);
              mp += 4;
              cout << "Illumination: " << *illumination << endl;
            }
            for (int j=0; j<n_objects; j++)
            {
              byte* index_f1 = reinterpret_cast<byte*>(&msg_queue[mp]);
              mp++;
              byte* index_f2 = reinterpret_cast<byte*>(&msg_queue[mp]);
              mp++;
              byte* index_f3 = reinterpret_cast<byte*>(&msg_queue[mp]);
              mp++;
              byte* index_f4 = reinterpret_cast<byte*>(&msg_queue[mp]);
              mp++;
              if ((*index_f4) & 0x01 == 0x01 && (*index_f4) & 0x02 == 0x02)
              {
                float* cx = reinterpret_cast<float*>(&msg_queue[mp]);
                mp += 4;
                float* cy = reinterpret_cast<float*>(&msg_queue[mp]);
                mp += 4;
                cout << "  Object-[" << j+1 << "]-CXCY: (" << *cx << ", " << *cy << ")" << endl;
              }
              if ((*index_f4) & 0x04 == 0x04 && (*index_f4) & 0x08 == 0x08)
              {
                float* w = reinterpret_cast<float*>(&msg_queue[mp]);
                mp += 4;
                float* h = reinterpret_cast<float*>(&msg_queue[mp]);
                mp += 4;
                cout << "  Object-[" << j+1 << "]-WH: (" << *w << ", " << *h << ")" << endl;
              }
              if ((*index_f4) & 0x10 == 0x10)
              {
                float* score = reinterpret_cast<float*>(&msg_queue[mp]);
                mp += 4;
                cout << "  Object-[" << j+1 << "]-Score: " << *score << endl;
              }
              if ((*index_f4) & 0x20 == 0x20)
              {
                int* category_id = reinterpret_cast<int*>(&msg_queue[mp]);
                mp += 4;
                cout << "  Object-[" << j+1 << "]-CateID: " << *category_id << endl;
              }
              if ((*index_f4) & 0x40 == 0x40)
              {
                int* tracked_id = reinterpret_cast<int*>(&msg_queue[mp]);
                mp += 4;
                cout << "  Object-[" << j+1 << "]-TrackID: " << *tracked_id << endl;
              }
              if ((*index_f4) & 0x80 == 0x80 && (*index_f3) & 0x01 == 0x01 && (*index_f3) & 0x02 == 0x02)
              {
                float* px = reinterpret_cast<float*>(&msg_queue[mp]);
                mp += 4;
                float* py = reinterpret_cast<float*>(&msg_queue[mp]);
                mp += 4;
                float* pz = reinterpret_cast<float*>(&msg_queue[mp]);
                mp += 4;
                cout << "  Object-[" << j+1 << "]-Position: (" << *px << ", " << *py << ", " << *pz << ")" << endl;
              }
              if ((*index_f3) & 0x04 == 0x04 && (*index_f3) & 0x08 == 0x08)
              {
                float* los_ax = reinterpret_cast<float*>(&msg_queue[mp]);
                mp += 4;
                float* los_ay = reinterpret_cast<float*>(&msg_queue[mp]);
                mp += 4;
                cout << "  Object-[" << j+1 << "]-LOS: (" << *los_ax << ", " << *los_ay << ")" << endl;
              }
              if ((*index_f3) & 0x10 == 0x10)
              {
                float* yaw_a = reinterpret_cast<float*>(&msg_queue[mp]);
                mp += 4;
                cout << "  Object-[" << j+1 << "]-YAW: " << *yaw_a << endl;
              }
            }

            start_index += ilen + 4;
          }
          else if (end_index - i < ilen + 2)
          {
            break;
          }
          else
          {
            start_index++;
          }
        }
        else
        {
          break;
        }
      }
      else
      {
        start_index++;
      }
    }
  
  }

  return 0;
}
