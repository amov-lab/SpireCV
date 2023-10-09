#include <iostream>
#include <string>
// 包含SpireCV SDK头文件
#include <sv_world.h>
#include <chrono>

// yaw roll pitch
double gimbalEulerAngle[3];
bool revFlag = false;
void gimableCallback(double &frame_ang_r, double &frame_ang_p, double &frame_ang_y,
                     double &imu_ang_r, double &imu_ang_p, double &imu_ang_y,
                     double &fov_x, double &fov_y)
{
    revFlag = true;
    gimbalEulerAngle[0] = imu_ang_r;
    gimbalEulerAngle[1] = imu_ang_p;
    gimbalEulerAngle[2] = frame_ang_y;
}

int main(int argc, char *argv[])
{
    std::cout << "start " << argv[0] << " ...." << std::endl;

    if (argc != 3)
    {
        std::cout << "param error" << std::endl;
        return -1;
    }

    sv::Gimbal gimbal(sv::GimbalType::G1, sv::GimbalLink::SERIAL);

    gimbal.setSerialPort(argv[1]);
    if (!gimbal.open(gimableCallback))
    {
        std::cout << "IO open error" << std::endl;
        return -1;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    if (!revFlag)
    {
        std::cout << "IO error,without data.failed !!!!!" << std::endl;
        return -1;
    }

    std::cout << " start set home test " << std::endl;
    gimbal.setHome();

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    for (uint8_t i = 0; i < 2; i++)
    {
        if (fabs(gimbalEulerAngle[i]) > 0.1f)
        {
            std::cout << " gimbal set home error , failed !!!!!" << std::endl;
            std::cout << "YRP:" << gimbalEulerAngle[0] << std::endl
                      << gimbalEulerAngle[1] << std::endl
                      << gimbalEulerAngle[2] << std::endl;
            return -1;
        }
    }
    if (fabs(gimbalEulerAngle[2]) > 3.0f)
    {
        std::cout << " gimbal set angle error , failed !!!!!" << std::endl;
        std::cout << "YRP:" << gimbalEulerAngle[0] << std::endl
                  << gimbalEulerAngle[1] << std::endl
                  << gimbalEulerAngle[2] << std::endl;
        return -1;
    }

    std::cout << " pass... " << std::endl;
    std::cout << " start set angle 1 test " << std::endl;

    double angleSet[3] = {30, 90, 45};
    gimbal.setAngleEuler(angleSet[0], angleSet[1], angleSet[2], 0, 0, 0);

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    for (uint8_t i = 0; i < 3; i++)
    {
        if (fabs(gimbalEulerAngle[i] - angleSet[i]) > 1.0f)
        {
            std::cout << " gimbal set angle error , failed !!!!!" << std::endl;
            std::cout << "YRP:" << gimbalEulerAngle[0] << std::endl
                      << gimbalEulerAngle[1] << std::endl
                      << gimbalEulerAngle[2] << std::endl;
            return -1;
        }
    }

    std::cout << " pass... " << std::endl;
    std::cout << " start set angle 2 test " << std::endl;

    angleSet[0] = -angleSet[0];
    angleSet[1] = -30;
    angleSet[2] = -angleSet[2];
    gimbal.setAngleEuler(angleSet[0], angleSet[1], angleSet[2], 0, 0, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    for (uint8_t i = 0; i < 3; i++)
    {
        if (fabs(gimbalEulerAngle[i] - angleSet[i]) > 1.0f)
        {
            std::cout << " gimbal set angle error , failed !!!!!" << std::endl;
            std::cout << "YRP:" << gimbalEulerAngle[0] << std::endl
                      << gimbalEulerAngle[1] << std::endl
                      << gimbalEulerAngle[2] << std::endl;
            return -1;
        }
    }

    std::cout << " pass... " << std::endl;
    std::cout << " start set angle rate test " << std::endl;

    gimbal.setHome();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    angleSet[0] = 20;
    angleSet[1] = 20;
    angleSet[2] = 20;

    for (uint8_t i = 0; i < 51; i++)
    {
        gimbal.setAngleRateEuler(angleSet[0], angleSet[1], angleSet[2]);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    gimbal.setAngleRateEuler(0, 0, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    for (uint8_t i = 0; i < 3; i++)
    {
        if (fabs(gimbalEulerAngle[i] - angleSet[i]) > 0.7f)
        {
            std::cout << " gimbal set angle rate error , failed !!!!!" << std::endl;
            std::cout << "YRP:" << gimbalEulerAngle[0] << std::endl
                      << gimbalEulerAngle[1] << std::endl
                      << gimbalEulerAngle[2] << std::endl;
            return -1;
        }
    }

    gimbal.setHome();
    std::cout << " pass... " << std::endl;
    std::cout << " start image test " << std::endl;

    sv::Camera cap;
    cap.setIp(argv[2]);
    cap.setWH(1280, 720);
    cap.setFps(30);

    cap.open(sv::CameraType::G1);

    if (!cap.isRunning())
    {
        std::cout << " gimbal image error , failed !!!!!" << std::endl;
        return -1;
    }

    cv::Mat img;
    uint16_t count = 0;

    for (uint16_t i = 0; i < 300; i++)
    {
        if (cap.read(img))
        {
            count++;
            if (count > 10)
            {
                std::cout << " pass... " << std::endl;
                return 0;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    std::cout << " gimbal image error , failed !!!!!" << std::endl;
    return -1;
}