#pragma once

#include "Effect.hpp"
#include "Pickup.hpp"
#include "Player.hpp"

#include <Image.hpp>
#include <Math/AABB.hpp>
#include <TileMap.hpp>

#include <LDtkLoader/Level.hpp>
#include <LDtkLoader/World.hpp>

enum class ColliderType
{
    Default,
    Sand,
    Slime,
};

struct Collider
{
    ColliderType type = ColliderType::Default;
    Math::AABB   aabb;
    bool         isOneWay = false;
};

class Level
{
public:
    Level() = default;
    Level( const ldtk::Project& project, const ldtk::World& world, const ldtk::Level& level );

    void update( float deltaTime );

    // Reset level.
    void reset();

    void draw( sr::Image& image ) const;

private:
    void updateCollisions( float deltaTime );
    void updatePickups( float deltaTime );
    void updateEffects( float deltaTime );

    const ldtk::World* world = nullptr;
    const ldtk::Level* level = nullptr;

    // Level colliders.
    std::vector<Collider> colliders;

    // Fruit sprites.
    std::map<std::string, sr::SpriteSheet> fruitSprites;

    // All pickups
    std::vector<Pickup> allPickups;
    // Remaining pickups.
    std::vector<Pickup> availablePickups;
    // Sprite animation to play when a pickup is collected.
    sr::SpriteAnim pickupCollected;
    // Currently playing effects.
    std::vector<Effect> effects;

        // Tile map.
        sr::TileMap tileMap;
    // Sprite sheet for the tile map.
    sr::SpriteSheet spriteSheet;

    Player    player;
    glm::vec2 playerStart { 0 };
};