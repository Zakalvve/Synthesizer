#ifndef NODE_H
#define NODE_H

#include <vector>
#include <map>
#include <concepts>
#include <iostream>

#include "Sink.h"
#include "Source.h"
#include "Process.h"
#include "InputBuffer.h"

namespace Audio {
    namespace Processing {
        namespace Pipelines {
            template<typename Buffer, typename In, typename Out>
            requires std::derived_from<Buffer, InputBuffer<In>>
            class Node : public Sink<In>, public Source<In> {
                public:
                    using InType = In;

                    Node() : process(nullptr) {}
                    Node(Process<In, Out>* process) : process(process) {}

                    virtual void consume (In value) override {
                        inputBuffer.insert(value);
                        std::cout << value << std::endl;

                        if (process == nullptr) return;
                        
                        if (inputBuffer.isFilled()){
                            for(auto* node : nextNodes){
                                process->process(this, node);
                            }
                        }
                    }

                    virtual void produce(Sink<In>* consumer){
                        consumer->consume(inputBuffer);
                    }

                    template<typename T>
                    void connect(T* nextNode) requires std::same_as<typename T::InType, Out> {
                        nextNodes.push_back(nextNode);
                    }

                    In getData() { return inputBuffer.getBuffer(); }

                private:
                    Buffer inputBuffer;
                    Process<In, Out>* process;
                    std::vector<Sink<Out>*> nextNodes;
            };

            template<typename Out>
            class OutputNode : public Node<Out, void*> {
                public:
                    OutputNode() = default;
            };
        }
    }
}

#endif //NODE_H