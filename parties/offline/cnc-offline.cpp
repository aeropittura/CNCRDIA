/*
 * cnc-offline.cpp
 *
 */

#include "he/offline/cut_and_choose_machine.h"

int main(int argc, const char** argv)
{
    CutAndChooseMachine(argc, argv).run();
    return 0;
}
