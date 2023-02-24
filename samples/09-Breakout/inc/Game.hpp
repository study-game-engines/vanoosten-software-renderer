#pragma once

#include "State.hpp"

#include <Events.hpp>
#include <Font.hpp>

#include <memory>

class Game
{
public:
    enum class GameState
    {
        None,
        MainMenu,
        Playing,
        Pause,
        GameOver,
    };

    Game( uint32_t screenWidth, uint32_t screenHeight );

    // Delete copy and move constructors and assign operators
    Game( const Game& )            = delete;
    Game( Game&& )                 = delete;
    Game& operator=( const Game& ) = delete;
    Game& operator=( Game&& )      = delete;

    void update( float deltaTime );

    void processEvent( const sr::Event& event );

    const sr::Image& getImage() const;

private:
    void setState( GameState newState );

    GameState              currentState = GameState::None;
    std::unique_ptr<State> state;

    sr::Image image;
    // Fonts.
    sr::Font arial20;
    sr::Font arial24;
};