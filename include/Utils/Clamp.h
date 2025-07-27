#ifndef CLAMP_H_INCLUDED
#define CLAMP_H_INCLUDED

template <typename T>
T clamp(T value, T min, T max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

#endif // CLAMP_H_INCLUDED
