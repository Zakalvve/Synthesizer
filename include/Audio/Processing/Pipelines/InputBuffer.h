#ifndef INPUTBUFFER_H
#define INPUTBUFFER_H

#include <optional>
#include <vector>
#include <stdexcept>
#include <type_traits>
#include <concepts>

namespace Audio {
    namespace Processing {
        namespace Pipelines {
            template<typename T>
            class InputBuffer {
                public:
                    InputBuffer() = default;
                    virtual void insert(const T& data) = 0;
                    virtual bool isFilled() const = 0;
                    virtual T getBuffer() const = 0;
                    virtual ~InputBuffer() = default;
                protected:
                    T buffer;
            };

            template<typename T>
            class SimpleBuffer : public InputBuffer<std::optional<T>>{
                public:
                    void insert(const T& data) override {
                        this->buffer = std::optional<T>(data);
                    }

                    bool isFilled() const override {
                        return this->buffer.has_value();
                    }

                    T getBuffer() const override {
                        if (!isFilled()){
                            // throw error
                        }
                        return this->buffer.value_or(T());
                    }
            };

            template<typename T>
            concept IsVector = requires {
                typename T::value_type;
                requires std::same_as<T, std::vector<typename T::value_type>>;
            };

            template<IsVector T>
            class MultiBuffer : public InputBuffer<T>{
                public: 
                    MultiBuffer() : parentCount(0) {}
                    void insert(const T& data) override {
                        this->buffer.push_back(data);
                    }

                    bool isFilled() const override {
                        return this->buffer.size() == parentCount;
                    }

                    T getBuffer() const override {
                        if (!isFilled()){
                            // throw error
                        }
                        return this->buffer;
                    }

                    virtual void addParent() {
                        ++parentCount;
                    }
                protected:
                    int parentCount;
            };
        }
    }
}

#endif //INPUTBUFFER_H