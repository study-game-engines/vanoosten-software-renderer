#pragma once

#include "Player.hpp"

#include <Image.hpp>
#include <Math/AABB.hpp>

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
    Level( const ldtk::World& world, const ldtk::Level& level );

    void update( float deltaTime );

    // Reset level.
    void reset();

    void draw( sr::Image& image ) const;

private:
    const ldtk::World* world = nullptr;
    const ldtk::Level* level = nullptr;

    // Level colliders.
    std::vector<Collider> colliders;

    Player    player;
    glm::vec2 playerStart;
};