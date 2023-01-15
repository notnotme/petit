#pragma once

namespace PetitAnim
{

//-----------------------------------------------------------------------------
// [SECTION] Animation
//-----------------------------------------------------------------------------

namespace Anim
{

    //-----------------------------------------------------------------------------
    // [SECTION] Anim - Forward declarations and basic types
    //-----------------------------------------------------------------------------

    template<typename T>    struct Animation;
    template<typename T>    struct LerpAnimation;
                            struct EaseInAnimation;
                            struct EaseOutAnimation;

} // namespace Anim

} // namespace PetitAnim

//-----------------------------------------------------------------------------
// [SECTION] Anim - Public declarations and basic types
//-----------------------------------------------------------------------------

template<typename T>
struct PetitAnim::Anim::Animation
{
    Animation(float duration, T startValue, T endValue) :
    duration(duration),
    currentTime(0.0f),
    terminated(false),
    startValue(startValue),
    endValue(endValue),
    currentValue(startValue)
    {
    }

    virtual ~Animation()
    {
    }
            
    void reset()
    {
        terminated = false;
        currentTime = 0.0f;
        currentValue = startValue;
    }        
    
    void revert()
    {
        auto temp = startValue;
        startValue = endValue;
        endValue = temp;
    }

    bool isTerminated()
    {
        return terminated;
    }

    void setDuration(float millis)
    {
        duration = millis;
    }

    void setStartValue(T value)
    {
        startValue = value;
    }

    void setEndValue(T value)
    {
        endValue = value;
    }
    
    T getValue()
    {
        return currentValue;
    }
            
    void update(float dt)
    {
        if (!terminated)
        {
            currentTime += dt;
            if (currentTime <= 0.0f)
            {
                currentValue = startValue;
            }
            else if (currentTime >= duration)
            {
                currentValue = endValue;
                terminated = true;
            }
            else
            {
                currentValue = updateValue();
            }
        }
    }
    
    virtual T updateValue() = 0;
    
protected:
    float   duration;
    float   currentTime;
    bool    terminated;
    T       startValue;
    T       endValue;
    T       currentValue;
};


template<typename T>
struct PetitAnim::Anim::LerpAnimation :
PetitAnim::Anim::Animation<T>
{
    LerpAnimation(float duration, T startValue, T endValue) :
    Animation<T>(duration, startValue, endValue)
    {
    }
    
    virtual ~LerpAnimation()
    {
    }
    
    T updateValue() override {
        auto t = this->currentTime / this->duration;
        return this->startValue + t * (this->endValue - this->startValue);
    };
};

struct PetitAnim::Anim::EaseInAnimation :
PetitAnim::Anim::Animation<float>
{
    EaseInAnimation(float duration, float startValue, float endValue) :
    Animation(duration, startValue, endValue)
    {
    }
    
    virtual ~EaseInAnimation()
    {
    }
    
    float updateValue() override {
        auto t = this->currentTime / this->duration;
	    return this->endValue * t * t + this->startValue;
    }
};

struct PetitAnim::Anim::EaseOutAnimation :
PetitAnim::Anim::Animation<float>
{
    EaseOutAnimation(float duration, float startValue, float endValue) :
    Animation(duration, startValue, endValue)
    {
    }
    
    virtual ~EaseOutAnimation()
    {
    }
    
    float updateValue() override {
        auto t = this->currentTime / this->duration;
	    return -this->endValue * t * (t - 2) + this->startValue;
    }
};
