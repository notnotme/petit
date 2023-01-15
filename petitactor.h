#pragma once

#include "petit2d.h"

#include <vector>
#include <list>

namespace PetitActor
{

//-----------------------------------------------------------------------------
// [SECTION] Actor
//-----------------------------------------------------------------------------

namespace Actor
{

    //-----------------------------------------------------------------------------
    // [SECTION] Actor - Forward declarations and basic types
    //-----------------------------------------------------------------------------

    template<typename R>                struct  Actor;
    template<typename T, typename R>    struct  TypedActor;

    struct  SpriteActor;
    struct  SpriteVectorActor;
    struct  SpriteListActor;
    struct  VertexActor;
    struct  VertexVectorActor;
    struct  VertexListActor;

} // namespace Actor

//-----------------------------------------------------------------------------
// [SECTION] Layer
//-----------------------------------------------------------------------------

namespace Layer
{

    //-----------------------------------------------------------------------------
    // [SECTION] Layer - Forward declarations and basic types
    //-----------------------------------------------------------------------------

    template <typename A> struct  Layer;
    struct SpriteLayer;
    struct VertexLayer;

} // namespace Layer

} // namespace PetitActor

//-----------------------------------------------------------------------------
// [SECTION] Actor - Public declarations and basic types
//-----------------------------------------------------------------------------
template<typename R>
struct PetitActor::Actor::Actor
{
    bool isAlive = false;
    bool isVisible = false;
  
    Actor()
    {
    }

    virtual ~Actor()
    {
    }

    virtual void update(float dt) = 0;
    virtual void render() = 0;
};

template<typename T, typename R>
struct PetitActor::Actor::TypedActor :
public PetitActor::Actor::Actor<R>
{
    TypedActor(T target) :
    Actor<R>(),
    target(target)
    {
    }

    virtual ~TypedActor()
    {
    }
protected:
    T target;
};

struct PetitActor::Actor::SpriteActor :
public PetitActor::Actor::TypedActor<Petit2D::Sprite::Sprite, Petit2D::Sprite::Sprite>
{
    SpriteActor() :
    TypedActor(Petit2D::Sprite::Sprite())
    {
    }

    virtual ~SpriteActor()
    {
    }

    virtual void render() override
    {
        Petit2D::Sprite::Add(target);
    }
};

struct PetitActor::Actor::SpriteVectorActor :
public PetitActor::Actor::TypedActor<std::vector<Petit2D::Sprite::Sprite>, Petit2D::Sprite::Sprite>
{
    SpriteVectorActor() :
    TypedActor(std::vector<Petit2D::Sprite::Sprite>())
    {
    }

    virtual ~SpriteVectorActor()
    {
        target.clear();
    }

    virtual void render()
    {
        for (const auto& sprite : target)
        {
            Petit2D::Sprite::Add(sprite);
        }
    }
};

struct PetitActor::Actor::SpriteListActor :
public PetitActor::Actor::TypedActor<std::list<Petit2D::Sprite::Sprite>, Petit2D::Sprite::Sprite>
{
    SpriteListActor() :
    TypedActor(std::list<Petit2D::Sprite::Sprite>())
    {
    }

    virtual ~SpriteListActor()
    {
        target.clear();
    }

    virtual void render()
    {
        for (const auto& sprite : target)
        {
            Petit2D::Sprite::Add(sprite);
        }
    }
};

struct PetitActor::Actor::VertexActor :
public PetitActor::Actor::TypedActor<Petit2D::Shape::Vertex, Petit2D::Shape::Vertex>
{
    VertexActor() :
    TypedActor(Petit2D::Shape::Vertex())
    {
    }

    virtual ~VertexActor()
    {
    }

    virtual void render()
    {
        Petit2D::Shape::Add(target);
    }
};

struct PetitActor::Actor::VertexVectorActor :
public PetitActor::Actor::TypedActor<std::vector<Petit2D::Shape::Vertex>, Petit2D::Shape::Vertex>
{
    VertexVectorActor() :
    TypedActor(std::vector<Petit2D::Shape::Vertex>())
    {
    }

    virtual ~VertexVectorActor()
    {
        target.clear();
    }

    virtual void render()
    {
        for (const auto& vertex : target)
        {
            Petit2D::Shape::Add(vertex);
        }
    }
};

struct PetitActor::Actor::VertexListActor :
public PetitActor::Actor::TypedActor<std::list<Petit2D::Shape::Vertex>, Petit2D::Shape::Vertex>
{
    VertexListActor() :
    TypedActor(std::list<Petit2D::Shape::Vertex>())
    {
    }

    virtual ~VertexListActor()
    {
        target.clear();
    }

    virtual void render()
    {
        for (const auto& vertex : target)
        {
            Petit2D::Shape::Add(vertex);
        }
    }
};

//-----------------------------------------------------------------------------
// [SECTION] Layer
//-----------------------------------------------------------------------------

template <typename A>
struct PetitActor::Layer::Layer
{
    std::vector<A> actors = std::vector<A>();

    Layer()
    {
    }

    virtual ~Layer()
    {
        actors.clear();
    }


    virtual void update(float dt)
    {
        for(const auto actor : actors)
        {
            if (actor->isAlive)
            {
                actor->update(dt);
            }
        }
    }

    virtual void render()
    {
        for(const auto actor : actors)
        {
            if (actor->isVisible)
            {
                actor->render();
            }
        }
    }
};

struct PetitActor::Layer::SpriteLayer :
public PetitActor::Layer::Layer<PetitActor::Actor::Actor<Petit2D::Sprite::Sprite>*>
{
    SpriteLayer() :
    Layer()
    {
    }

    virtual ~SpriteLayer()
    {
    }
};

struct PetitActor::Layer::VertexLayer :
public PetitActor::Layer::Layer<PetitActor::Actor::Actor<Petit2D::Shape::Vertex>*>
{
    VertexLayer() :
    Layer()
    {
    }

    virtual ~VertexLayer()
    {
    }
};
