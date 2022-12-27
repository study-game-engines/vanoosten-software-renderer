#pragma once

#include "Config.hpp"
#include "GamePadState.hpp"

namespace sr
{
/// <summary>
/// Dead-zone mode.
/// </summary>
enum class DeadZone
{
    IndependentAxis = 0,
    Circular,
    None,
};

class SR_API GamePad
{
public:
    static const int MAX_PLAYERS;

    static GamePadState getState( int      player,
                                  DeadZone deadZoneMode = DeadZone::IndependentAxis );

    static bool setVibration( int player, float leftMotor, float rightMotor,
                              float leftTrigger = 0.0f, float rightTrigger = 0.0f );

    // Static class, delete constructors and assignment operators.
    GamePad()                 = delete;
    GamePad( const GamePad& ) = delete;
    GamePad( GamePad&& )      = delete;
    ~GamePad()                = delete;

    const GamePad& operator=( const GamePad& ) const = delete;
    const GamePad& operator=( GamePad&& ) const      = delete;
};
}  // namespace sr