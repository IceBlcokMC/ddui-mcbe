#pragma once


struct DDUI_TEST {
    static void init();

#ifdef DDUI_WITH_CAPTURE_PACKET
    static void capture_packet();
#endif
};