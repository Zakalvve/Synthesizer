#ifndef TESTNODE_H
#define TESTNODE_H

#include <vector>
#include <map>
#include <concepts>
#include <iostream>

template<typename... Ts>
class Sink {
    public:
        virtual void push(Ts... args) = 0;
        virtual ~Sink() = default;
};

template<typename... Ts>
class Source {
    public:
        virtual void produce(Sink<Ts...>* consumer) = 0;
        virtual ~Source() = default;
};

template<typename In, typename Out>
class Process : Sink<In> {
    public:
        virtual void process(Source<In>* source, Sink<Out>* sink){
            s = sink;
            source->produce(this);
        }
        virtual ~Process() = default;

    protected:
        Sink<Out>* s;
};

template<typename In, typename Out>
class Node : public Sink<In>, public Source<In> {
    public:
        using InType = In;

        Node() : process(nullptr) {}
        Node(Process<In, Out>* process) : process(process) {}

        virtual void push (In value) override {
            inputBuffer = value;
            std::cout << value << std::endl;

            if (process == nullptr) return;

            for(auto* node : nextNodes){
                process->process(this, node);
            }
        }

        virtual void produce(Sink<In>* consumer){
            consumer->push(inputBuffer);
        }

        template<typename T>
        void connect(T* nextNode) requires std::same_as<typename T::InType, Out> {
            nextNodes.push_back(nextNode);
        }

        In getData() { return inputBuffer; }

    private:
        In inputBuffer;
        Process<In, Out>* process;
        std::vector<Sink<Out>*> nextNodes;
};

template<typename Out>
class OutputNode : public Node<Out, void*> {
    public:
        OutputNode() = default;
};

class ToDoubleProcess : public Process<int, double> {
    public:
        virtual void push(int value) override {
            s->push(static_cast<double>(value));
        }
};

class HalfProcess : public Process<double, double> {
    public:
        virtual void push(double value) override {
            s->push(value/2);
        }
};

#endif //TESTNODE_H