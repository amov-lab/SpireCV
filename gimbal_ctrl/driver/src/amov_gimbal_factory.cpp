/*
 * @Description: 
 * @Author: L LC @amov
 * @Date: 2023-11-24 15:48:47
 * @LastEditors: L LC @amov
 * @LastEditTime: 2023-12-05 16:27:10
 * @FilePath: /SpireCV/gimbal_ctrl/driver/src/amov_gimbal_factory.cpp
 */

#include "amov_gimbal_private.h"
#include "g1_gimbal_driver.h"
#include "Q10f_gimbal_driver.h"
#include "AT10_gimbal_driver.h"
#include "GX40_gimbal_driver.h"

#include <map>
#include <iterator>

namespace amovGimbalFactory
{
    typedef amovGimbal::amovGimbalBase *(*createCallback)(amovGimbal::IOStreamBase *_IO);
    typedef std::map<std::string, createCallback> callbackMap;

    callbackMap amovGimbals =
        {
            {"G1", g1GimbalDriver::creat},
            {"Q10f", Q10fGimbalDriver::creat},
            {"AT10", AT10GimbalDriver::creat},
            {"GX40", GX40GimbalDriver::creat}};

    /* The amovGimbalCreator class is a factory class that creates an instance of the amovGimbal class */
    // Factory used to create the gimbal instance
    class amovGimbalCreator
    {
    public:
        static amovGimbal::amovGimbalBase *createAmovGimbal(const std::string &type, amovGimbal::IOStreamBase *_IO)
        {
            callbackMap::iterator temp = amovGimbals.find(type);

            if (temp != amovGimbals.end())
            {
                return (temp->second)(_IO);
            }
            std::cout << type << " is Unsupported device type!" << std::endl;
            return NULL;
        }

    private:
        amovGimbalCreator()
        {
        }
        static amovGimbalCreator *pInstance;
        static amovGimbalCreator *getInstance()
        {
            if (pInstance == NULL)
            {
                pInstance = new amovGimbalCreator();
            }
            return pInstance;
        }

        ~amovGimbalCreator();
    };
} // namespace amovGimbalFactory

/**
 * The function creates a new gimbal object, which is a pointer to a new amovGimbal object, which is a
 * pointer to a new Gimbal object, which is a pointer to a new IOStreamBase object
 *
 * @param type the type of the device, which is the same as the name of the class
 * @param _IO The IOStreamBase object that is used to communicate with the device.
 * @param _self the node ID of the device
 * @param _remote the node ID of the remote device
 */
amovGimbal::gimbal::gimbal(const std::string &type, IOStreamBase *_IO,
                           uint32_t _self, uint32_t _remote)
{
    typeName = type;

    devHandle = amovGimbalFactory::amovGimbalCreator::createAmovGimbal(typeName, _IO);

    ((amovGimbalBase *)(devHandle))->nodeSet(_self, _remote);
}

amovGimbal::gimbal::~gimbal()
{
    // 先干掉请求线程
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    delete ((amovGimbalBase *)(devHandle));
}
