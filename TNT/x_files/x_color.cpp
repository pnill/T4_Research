#include "x_files.hpp"
#include "x_color.hpp"


//=====================================================================================================================================
void color::Saturate( f32 Percentage, xbool bSaturateToWhiteIfNeccessary )
{
    if( bSaturateToWhiteIfNeccessary )
    {
        u8 TestR, TestG, TestB;

        // Increase the color components by a certain percentage.
        TestR = R + (u8)(R * Percentage);
        TestG = G + (u8)(G * Percentage);
        TestB = B + (u8)(B * Percentage);

        // Make sure to test to see if the newer modified color is actually more rich than before we adjusted it.
        // If this color is not more rich ( higher number ), then it means that we have rolled around the u8 and
        // have made the color more faded.  In a case where we have rolled over, just saturate the color towards white.
        if( TestR >= R ) R = TestR;
        else             R = 255;
        if( TestG >= G ) G = TestG;
        else             G = 255;
        if( TestB >= B ) B = TestB;
        else             B = 255;
    }
    // Otherwise, cap the color when a single component hits full brightness ( single component hits 255 )
    else
    {
        xbool RIsHighest;
        xbool GIsHighest;
        u8    MaxAdjustmentValue;
        u8    TestValue;
        u8    TestColor;

        ASSERT( Percentage >= 0.0f && Percentage <= 1.0f );

        RIsHighest = R >= G && R >= B;
        GIsHighest = G > R  && G >= B;

        if(      RIsHighest ) TestColor = R;
        else if( GIsHighest ) TestColor = G;
        else                  TestColor = B;

        TestValue = TestColor + (u8)(TestColor * Percentage);

        if( TestValue > TestColor )
        {
            MaxAdjustmentValue = TestValue - TestColor;
        }
        else
        {
            MaxAdjustmentValue = 255 - TestColor;
        }

        R += MaxAdjustmentValue;
        G += MaxAdjustmentValue;
        B += MaxAdjustmentValue;
    }
}

//=====================================================================================================================================
void color::FadeToBlack( f32 Percentage )
{
    u8 TestR, TestG, TestB;

    ASSERT( Percentage >= 0.0f && Percentage <= 1.0f );

    // Decrease the color components by a certain percentage.
    TestR = R - (u8)(R * Percentage);
    TestG = G - (u8)(G * Percentage);
    TestB = B - (u8)(B * Percentage);

    // Make sure that the colors have been diluted.  If you are trying to dilute the color and a component
    // happens to be larger than before, it means that we have wraped around the u8.  In these cases
    // just set that color component to 0.
    if( TestR <= R ) R = TestR;
    else             R = 0;
    if( TestG <= G ) G = TestG;
    else             G = 0;
    if( TestB <= B ) B = TestB;
    else             B = 0;
}
