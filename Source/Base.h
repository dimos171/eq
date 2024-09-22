//
//  Base.h
//  SimpleEQ
//
//  Created by Dmitry Dyadin on 22/09/2024.
//

#ifndef Base_h
#define Base_h

enum Channel
{
    Right, //effectively 0
    Left //effectively 1
};

enum Slope
{
    Slope_12,
    Slope_24,
    Slope_36,
    Slope_48
};

enum ChainPositions
{
    LowCut,
    Peak,
    HighCut
};

struct ChainSettings
{
    float peakFreq { 0 }, peakGainInDecibels{ 0 }, peakQuality {1.f};
    float lowCutFreq { 0 }, highCutFreq { 0 };
    
    Slope lowCutSlope { Slope::Slope_12 }, highCutSlope { Slope::Slope_12 };
    
    bool lowCutBypassed {false}, peakBypassed {false}, highCutBypassed {false};
};

enum FFTOrder
{
    order2048 = 11,
    order4096 = 12,
    order8192 = 13
};


#endif /* Base_h */
