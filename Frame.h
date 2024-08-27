/*
  ==============================================================================

    Frame.h
    Created: 22 Aug 2024 3:37:26pm
    Author:  benny

  ==============================================================================
*/

#pragma once
const int numSteps = 4;
class Frame 
{
public:
    Frame() : data()
    {

    }

    float* operator[](int index)
    {
        return data.data()[index].data();
    }
    
    
private:
    std::array<std::array<float, 2048>, 128 / numSteps> data;
};