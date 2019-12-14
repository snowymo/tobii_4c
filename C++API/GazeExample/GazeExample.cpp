// GazeExample.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "tcpConnection.h"

#include <iostream>
#include <interaction_lib/InteractionLib.h>
#include <interaction_lib/misc/InteractionLibPtr.h>
int main()
{
    // create the interaction library
    IL::UniqueInteractionLibPtr intlib(IL::CreateInteractionLib(IL::FieldOfUse::Interactive));
    // assume single screen with size 2560x1440 and use full screen (not window local) coordinates
    constexpr float width = 1920.0f;// 2560.0f;
    constexpr float height = 1200.0f;// 1440.0f;
    constexpr float offset = 0.0f;
    intlib->CoordinateTransformAddOrUpdateDisplayArea(width, height);
    intlib->CoordinateTransformSetOriginOffset(offset, offset);
    // subscribe to gaze point data; print data to stdout when called
    intlib->SubscribeGazePointData([](IL::GazePointData evt, void* context)
    {
        std::cout
            << "x: " << evt.x
            << ", y: " << evt.y
            << ", validity: " << (evt.validity == IL::Validity::Valid ? "valid" : "invalid")
            << ", timestamp: " << evt.timestamp_us << " us"
            << "\n";

        // zhenyi
        /*byte buffer[8];
        memcpy_s(buffer, 4, &(evt.x), 4);
        memcpy_s(buffer+4, 4, &(evt.y), 4);*/
        std::string gazeMsg = evt.validity == IL::Validity::Valid ? std::to_string(evt.x) + " " + std::to_string(evt.y) : "";
        tcpSendMsg(gazeMsg);
    }, nullptr);
    std::cout << "Starting interaction library update loop.\n";


    // zhenyi
    tcpClientSetup();

    // setup and maintain device connection, wait for device data between events and 
    // update interaction library to trigger all callbacks, stop after 200 cycles
    constexpr size_t max_cycles = 20000;
    size_t cycle = 0;
    while (true)
    {
        intlib->WaitAndUpdate();
    }
}