#include <MouseStateTracker.hpp>

#include <cstring> // memset

using namespace sr;

// clang-format off
#define UPDATE_BUTTON_STATE( field ) field = static_cast<ButtonState>( ( !!state.field ) | ( ( !!state.field ^ !!lastState.field ) << 1 ) )
// clang-format on

bool MouseStateTracker::update( const MouseState& state ) noexcept
{
    UPDATE_BUTTON_STATE( leftButton );
    UPDATE_BUTTON_STATE( middleButton );
    UPDATE_BUTTON_STATE( rightButton );
    UPDATE_BUTTON_STATE( xButton1 );
    UPDATE_BUTTON_STATE( xButton2 );

    if (lastState != state) {
        lastState = state;
        return true;
    }

    return false;
}

void MouseStateTracker::reset() noexcept
{
    memset( this, 0, sizeof( MouseStateTracker ) );
}