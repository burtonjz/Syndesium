#ifndef __DSP_MATH_HPP_
#define __DSP_MATH_HPP_

namespace dsp {
    inline float fastAtan(float inp){
        return inp / (1.0f + 0.28f * inp * inp);
    }
}

#endif // __DSP_MATH_HPP_