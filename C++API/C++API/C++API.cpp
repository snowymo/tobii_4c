/*
COPYRIGHT 2018-2019  - PROPERTY OF TOBII AB
-------------------------------------
2018-2019 TOBII AB - KARLSROVAGEN 2D, DANDERYD 182 53, SWEDEN - All Rights Reserved.

NOTICE:  All information contained herein is, and remains, the property of Tobii AB and its suppliers, if any.
The intellectual and technical concepts contained herein are proprietary to Tobii AB and its suppliers and may be
covered by U.S.and Foreign Patents, patent applications, and are protected by trade secret or copyright law.
Dissemination of this information or reproduction of this material is strictly forbidden unless prior written
permission is obtained from Tobii AB.
*/
#include "../GazeExample/tcpConnection.h"

#include <iostream>
#include <interaction_lib/InteractionLib.h>
#include <interaction_lib/misc/InteractionLibPtr.h>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

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

    // setup ids and rectangles that define the interactors we want to use (one in each corner of the screen),
    // then add them to the interaction library
    //constexpr IL::InteractorId idA = 0;
    //constexpr IL::InteractorId idB = 2;
    //constexpr IL::InteractorId idC = 3;
    //constexpr IL::InteractorId idD = 4;
    //constexpr IL::InteractorId idE = 5;

    constexpr float size = 350.0f;
    
    // create an image based on the area here, with color
    cv::Mat focusIndicatorImage(1200, 1920, CV_8UC3);
    //IL::Rectangle rects[] = { rectA, rectB, rectC, rectD, rectE };
    IL::Rectangle rects[9];
    IL::InteractorId ids[9];
    for (int i = 0; i < 9; i++) {
        ids[i] = (IL::InteractorId)i;
        rects[i] = { i % 3 * width / 3, i / 3 * height / 3, width / 3*0.8, height / 3 * 0.8 };
    }
    cv::Scalar colors[] = { 
        cv::Scalar(255, 128, 128) , 
        cv::Scalar(128,128,255) ,
        cv::Scalar(255,191,128) ,
        cv::Scalar(191,128,255) ,
        cv::Scalar(255,255,255) ,
        cv::Scalar(128,229,255),
        cv::Scalar(191,255,128),
        cv::Scalar(255,128,208),
        cv::Scalar(255,255,128)        
    };
    for (int i = 0; i < 9; i++) {
        cv::rectangle(focusIndicatorImage, cv::Rect(rects[i].x, rects[i].y, rects[i].w, rects[i].h), colors[i], -1);
    }
    
    cv::imshow("eyetracker", focusIndicatorImage);
    cv::waitKey(0);
    cv::imwrite("eyetracker.jpg", focusIndicatorImage);

    constexpr float z = 0.0f;


    intlib->BeginInteractorUpdates();

    for (int i = 0; i < 9; i++) {
        intlib->AddOrUpdateInteractor(ids[i], rects[i], z);
    }


    intlib->CommitInteractorUpdates();

    // this struct is used to maintain a focus count (see below)
    struct Focus
    {
        IL::InteractorId id = IL::EmptyInteractorId();
        size_t           count = 0;
    };
    Focus focus;

    // subscribe to gaze focus events
    // print event data to std out when called and count the number of consecutive focus events
    intlib->SubscribeGazeFocusEvents([](IL::GazeFocusEvent evt, void* context)
    {
        Focus& focus = *static_cast<Focus*>(context);
        std::cout
            << "Interactor: " << evt.id
            << ", focused: " << std::boolalpha << evt.hasFocus
            << ", timestamp: " << evt.timestamp_us << " us"
            << "\n";

        if (evt.hasFocus)
        {
            focus.count = focus.id == evt.id ? focus.count + 1 : 1;
            focus.id = evt.id;
            // zhenyi
            /*byte buffer[8];
            memcpy_s(buffer, 4, &(evt.x), 4);
            memcpy_s(buffer+4, 4, &(evt.y), 4);*/
            // skip the center
            //if (evt.id != 4) {
                std::string gazeMsg = std::to_string(evt.id);
                tcpSendMsg("onfocus:" + gazeMsg);
            //}            
        }
    }, &focus);

    // setup and maintain device connection, wait for device data between events and 
    // update interaction library to trigger all callbacks
    // stop after 3 consecutive focus events on the same interactor
    std::cout << "Starting interaction library update loop.\n";

    constexpr size_t max_focus_count = 3;

    // zhenyi
    tcpClientSetup();

    while (true)
    {
        intlib->WaitAndUpdate();
    }

    std::cout << "Interactor " << focus.id << " got focused " << focus.count << " times\n";
}
