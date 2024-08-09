/*
 * @Description:
 * @Author: jario-jin @amov
 * @Date: 2023-04-12 09:12:52
 * @LastEditors: L LC @amov
 * @LastEditTime: 2023-12-05 17:25:40
 * @FilePath: /SpireCV/include/sv_gimbal.h
 */
#ifndef __SV_GIMBAL__
#define __SV_GIMBAL__

#include <string>
#include <map>
#include <iterator>
#include <thread>
#include <mutex>

namespace sv
{

  typedef void (*PStateInvoke)(double &frame_ang_r, double &frame_ang_p, double &frame_ang_y,
                               double &imu_ang_r, double &imu_ang_p, double &imu_ang_y,
                               double &fov_x, double &fov_y);

  enum class GimbalType
  {
    G1,
    Q10f,
    AT10,
    GX40,
  };
  enum class GimbalLink : int
  {
    NONE = 0x00,
    SERIAL = 0x01,
    ETHERNET_TCP = 0x02,
    ETHERNET_UDP = 0x04,
  };

  constexpr GimbalLink operator|(GimbalLink a, GimbalLink b)
  {
    return static_cast<GimbalLink>(static_cast<int>(a) | static_cast<int>(b));
  }

  constexpr GimbalLink operator&(GimbalLink a, GimbalLink b)
  {
    return static_cast<GimbalLink>(static_cast<int>(a) & static_cast<int>(b));
  }

  enum class GimablSerialByteSize
  {
    FIVE_BYTES = 5,
    SIX_BYTES = 6,
    SEVEN_BYTES = 7,
    EIGHT_BYTES = 8,
  };

  enum class GimablSerialParity
  {
    PARITY_NONE = 0,
    PARITY_ODD = 1,
    PARITY_EVEN = 2,
    PARITY_MARK = 3,
    PARITY_SPACE = 4,
  };

  enum class GimablSerialStopBits
  {
    STOPBITS_ONE = 1,
    STOPBITS_TWO = 2,
    STOPBITS_ONE_POINT_FIVE = 3,
  };

  enum class GimablSerialFlowControl
  {
    FLOWCONTROL_NONE = 0,
    FLOWCONTROL_SOFTWARE = 1,
    FLOWCONTROL_HARDWARE = 2,
  };

  typedef struct
  {
    double q0;
    double q1;
    double q2;
    double q3;
  } GimbalQuaternionT;

  typedef struct
  {
    double x; // or N
    double y; // or E
    double z; // or UP
  } GimbalVelocityT;

  typedef struct
  {
    double yaw;
    double roll;
    double pitch;
  } GimbalPosT;

  static inline void emptyCallback(double &frameAngleRoll, double &frameAnglePitch, double &frameAngleYaw,
                                   double &imuAngleRoll, double &imuAnglePitch, double &imuAngleYaw,
                                   double &fovX, double &fovY)
  {
  }

  //! A gimbal control and state reading class.
  /*!
    A common gimbal control class for vary type of gimbals.
    e.g. AMOV G1
  */
  class Gimbal
  {
  private:
    // Device pointers
    void *dev;
    void *IO;
    PStateInvoke m_callback;

    // Generic serial interface parameters list & default parameters
    std::string m_serial_port = "/dev/ttyUSB0";
    int m_serial_baud_rate = 115200;
    int m_serial_byte_size = (int)GimablSerialByteSize::EIGHT_BYTES;
    int m_serial_parity = (int)GimablSerialParity::PARITY_NONE;
    int m_serial_stopbits = (int)GimablSerialStopBits::STOPBITS_ONE;
    int m_serial_flowcontrol = (int)GimablSerialFlowControl::FLOWCONTROL_NONE;
    int m_serial_timeout = 500;

    // Ethernet interface parameters list & default parameters
    std::string m_net_ip = "192.168.144.121";
    int m_net_port = 2332;
    int m_net_recv_port = 2338;
    int m_net_send_port = 2337;

    GimbalType m_gimbal_type;
    GimbalLink m_gimbal_link;

    static std::map<std::string, void *> IOList;
    static std::mutex IOListMutex;
    static void *creatIO(Gimbal *dev);
    static void removeIO(Gimbal *dev);

    static void gimbalUpdataCallback(double frameAngleRoll, double frameAnglePitch, double frameAngleYaw,
                                     double imuAngleRoll, double imuAnglePitch, double imuAngleYaw,
                                     double fovX, double fovY, void *handle)
    {
      ((Gimbal *)(handle))->m_callback(frameAngleRoll, frameAnglePitch, frameAngleYaw, imuAngleRoll, imuAnglePitch, imuAngleYaw, fovX, fovY);
    }

  public:
    //! Constructor
    /*!
      \param serial_port: string like '/dev/ttyUSB0' in linux sys.
      \param baud_rate: serial baud rate, e.g. 115200
    */
    Gimbal(GimbalType gtype = GimbalType::G1, GimbalLink ltype = GimbalLink::SERIAL)
    {
      m_gimbal_type = gtype;
      m_gimbal_link = ltype;
    }
    ~Gimbal();
    // set Generic serial interface parameters
    void setSerialPort(const std::string &port);
    void setSerialPort(const int baud_rate);
    void setSerialPort(GimablSerialByteSize byte_size, GimablSerialParity parity,
                       GimablSerialStopBits stop_bits, GimablSerialFlowControl flowcontrol,
                       int time_out = 500);

    // set Ethernet interface parameters
    void setNetIp(const std::string &ip);
    // set tcp port
    void setTcpNetPort(const int &port);
    // set udp port
    void setUdpNetPort(const int &recvPort, const int &sendPort);

    // Create a device instance
    void setStateCallback(PStateInvoke callback);
    bool open(PStateInvoke callback = emptyCallback);

    // Funtions
    bool setHome();
    bool setZoom(double x);
    bool setAutoZoom(int state);
    bool setAutoFocus(int state);
    bool takePhoto();
    bool takeVideo(int state);
    int getVideoState();
    void attitudeCorrection(const GimbalQuaternionT &quaterion,
                                    const GimbalVelocityT &speed,
                                    const GimbalVelocityT &acc, void *extenData);
    void attitudeCorrection(const GimbalPosT &pos,
                                    const GimbalVelocityT &speed,
                                    const GimbalVelocityT &acc, void *extenData);

    //! Set gimbal angles
    /*!
      \param roll: eular roll angle (-60, 60) degree
      \param pitch: eular pitch angle (-135, 135) degree
      \param yaw: eular yaw angle (-150, 150) degree
      \param roll_rate: roll angle rate, degree/s
      \param pitch_rate: pitch angle rate, degree/s
      \param yaw_rate: yaw angle rate, degree/s
    */
    void setAngleEuler(
        double roll,
        double pitch,
        double yaw,
        double roll_rate = 0,
        double pitch_rate = 0,
        double yaw_rate = 0);
    //! Set gimbal angle rates
    /*!
      \param roll_rate: roll angle rate, degree/s
      \param pitch_rate: pitch angle rate, degree/s
      \param yaw_rate: yaw angle rate, degree/s
    */
    void setAngleRateEuler(
        double roll_rate,
        double pitch_rate,
        double yaw_rate);
  };
}
#endif
