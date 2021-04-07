#pragma once
#include <cstdint>
#include <functional>
namespace Animation
{
    using SetLedFn = std::function<void( int, uint8_t, uint8_t, uint8_t )>;
    using GetLedFn = std::function<void( int, uint8_t&, uint8_t&, uint8_t& )>;

    struct AnimationSettings
    {
        int mCount;
        int mIndex;
    };

    class Animation
    {
      public:
        Animation();
        virtual ~Animation();
        virtual void Reset( const AnimationSettings& settings );
        virtual void Render( SetLedFn set_led, GetLedFn get_led, uint32_t ms ) = 0;

      protected:
        AnimationSettings mSettings;
    };

    class FnAnimation : public Animation
    {
      public:
        using AnimationFn = std::function<void( SetLedFn&, GetLedFn&, const AnimationSettings&, uint32_t )>;
        FnAnimation( AnimationFn fn );
        ~FnAnimation() override;
        void Reset( const AnimationSettings& settings ) override;
        void Render( SetLedFn set_led, GetLedFn get_led, uint32_t ms ) override;

      protected:
        AnimationFn mFn;
    };

    extern FnAnimation WhiteAnimation;
    extern FnAnimation DaylightAnimation;
}