#ifndef PIPELINECORE_H
#define PIPELINECORE_H

#include <vector>
#include <unordered_map>
#include <memory>
#include <typeindex>
#include <string>
#include <stdexcept>
#include <typeinfo>
#include <iostream>
#include <tuple>
#include <functional>
#include <sstream>
#include <array>

constexpr bool ENABLE_PIPELINE_LOGGING = true;

#define PIPELINE_LOG(msg) \
    do { if (ENABLE_PIPELINE_LOGGING) std::cout << msg << std::endl; } while (0)

class FillableBuffer {
public:
    virtual bool isFilled() const = 0;
    virtual void flush() = 0;
    virtual ~FillableBuffer() = default;
};

template<typename T>
class Sink {
public:
    virtual void consume(const T& value) = 0;
    virtual ~Sink() = default;
};

template<typename T>
class Source {
public:
    virtual void produce(Sink<T>* consumer) = 0;
    virtual ~Source() = default;
};

template<typename T>
std::string to_string_fallback(const T& val) {
    std::ostringstream oss;
    oss << val;
    return oss.str();
}

inline std::string to_string_fallback(const std::vector<double>& vals) {
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < vals.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << vals[i];
    }
    oss << "]";
    return oss.str();
}

template<typename... Ts>
std::string tuple_to_string(const std::tuple<Ts...>& tpl) {
    std::ostringstream oss;
    oss << "(";
    std::apply([&oss](const Ts&... args) {
        std::size_t i = 0;
        ((oss << (i++ ? ", " : "") << to_string_fallback(args)), ...);
    }, tpl);
    oss << ")";
    return oss.str();
}

template<typename TIn, typename TOut>
class Processor : public Sink<TIn> {
public:
    virtual TOut process(const TIn& value) = 0;

    void consume(const TIn& value) override {
        if constexpr (requires { std::tuple_size<TIn>::value; }) {
            PIPELINE_LOG("Processor consuming: " << tuple_to_string(value));
        } else {
            PIPELINE_LOG("Processor consuming: " << to_string_fallback(value));
        }
        if (sink)
            sink->consume(process(value));
    }

    void flow(Source<TIn>* source, Sink<TOut>* sink) {
        this->sink = sink;
        source->produce(this);
    }

protected:
    Sink<TOut>* sink = nullptr;
};

class IPortOwner {
public:
    virtual void onPortFilled() = 0;
    virtual ~IPortOwner() = default;
};

class PortBase : public FillableBuffer {
public:
    virtual ~PortBase() = default;
};

template<typename T>
class Port : public PortBase, public Sink<T>, public Source<T> {
public:
    explicit Port(IPortOwner* owner) : owner(owner) {
        if (!owner) throw std::runtime_error("Port must have an owner");
    }

    void consume(const T& val) override {
        PIPELINE_LOG("Port consuming: " << to_string_fallback(val));
        if (filled) return;
        buffer = val;
        filled = true;

        if (owner) owner->onPortFilled();

        for (auto* sink : sinks) {
            PIPELINE_LOG("Propagating to sink...");
            sink->consume(val);
        }
    }

    void produce(Sink<T>* consumer) override {
        if (filled)
            consumer->consume(buffer);
    }

    void connect(Sink<T>* sink) {
        PIPELINE_LOG("Connecting sink to port");
        sinks.push_back(sink);
    }

    bool isFilled() const override { return filled; }
    void flush() override {
        PIPELINE_LOG("Flushing port");
        filled = false;
    }
    const T& get() const { return buffer; }

private:
    T buffer{};
    bool filled = false;
    std::vector<Sink<T>*> sinks;
    IPortOwner* owner;
};

template<typename T>
class MultiPort : public PortBase, public Sink<T>, public Source<std::vector<T>> {
public:
    explicit MultiPort(IPortOwner* owner) : owner(owner) {
        if (!owner) throw std::runtime_error("MultiPort must have an owner");
    }

    void consume(const T& val) override {
        PIPELINE_LOG("MultiPort consuming: " << to_string_fallback(val));
        buffer.push_back(val);
        if (buffer.size() == connections && owner)
            owner->onPortFilled();
    }

    void produce(Sink<std::vector<T>>* consumer) override {
        if (!buffer.empty())
            consumer->consume(buffer);
    }

    void connect(Sink<std::vector<T>>* sink) {
        PIPELINE_LOG("Connecting vector sink to multiport");
        sinks.push_back(sink);
    }

    void registerInput() { ++connections; }

    bool isFilled() const override { return buffer.size() == connections; }
    void flush() override {
        PIPELINE_LOG("Flushing multiport");
        buffer.clear();
    }

    const std::vector<T>& getAll() const { return buffer; }

private:
    std::vector<T> buffer;
    int connections = 0;
    std::vector<Sink<std::vector<T>>*> sinks;
    IPortOwner* owner;
};

class PortBank : public IPortOwner {
public:
    explicit PortBank(IPortOwner* owner) : owner(owner) {}

    template<typename T>
    void addPort(const std::string& name) {
        PIPELINE_LOG("Adding port: " << name);
        auto port = std::make_unique<Port<T>>(this);
        ports.emplace(name, std::move(port));
        types.emplace(name, std::type_index(typeid(T)));
        orderedNames.push_back(name);
    }

    template<typename T>
    void addMultiPort(const std::string& name) {
        PIPELINE_LOG("Adding multiport: " << name);
        auto port = std::make_unique<MultiPort<T>>(this);
        ports.emplace(name, std::move(port));
        types.emplace(name, std::type_index(typeid(std::vector<T>)));
        orderedNames.push_back(name);
    }

    template<typename T>
    Port<T>* getPort(const std::string& name) {
        auto typeIt = types.find(name);
        if (typeIt == types.end())
            throw std::runtime_error("Port not found: " + name);

        if (typeIt->second != std::type_index(typeid(T)))
            throw std::runtime_error("Type mismatch on port: " + name);

        return static_cast<Port<T>*>(ports.at(name).get());
    }

    template<typename T>
    const Port<T>* getPort(const std::string& name) const {
        auto typeIt = types.find(name);
        if (typeIt == types.end())
            throw std::runtime_error("Port not found: " + name);

        if (typeIt->second != std::type_index(typeid(T)))
            throw std::runtime_error("Type mismatch on port: " + name);

        return static_cast<const Port<T>*>(ports.at(name).get());
    }

    template<typename T>
    MultiPort<T>* getMultiPort(const std::string& name) {
        auto typeIt = types.find(name);
        if (typeIt == types.end())
            throw std::runtime_error("MultiPort not found: " + name);

        if (typeIt->second != std::type_index(typeid(std::vector<T>)))
            throw std::runtime_error("Type mismatch on multiport: " + name);

        return static_cast<MultiPort<T>*>(ports.at(name).get());
    }

    template<typename T>
    const MultiPort<T>* getMultiPort(const std::string& name) const {
        auto typeIt = types.find(name);
        if (typeIt == types.end())
            throw std::runtime_error("MultiPort not found: " + name);

        if (typeIt->second != std::type_index(typeid(std::vector<T>)))
            throw std::runtime_error("Type mismatch on multiport: " + name);

        return static_cast<const MultiPort<T>*>(ports.at(name).get());
    }

    bool allFilled() const {
        for (const auto& [_, port] : ports)
            if (!static_cast<FillableBuffer*>(port.get())->isFilled())
                return false;
        return true;
    }

    void flushAll() {
        for (const auto& [_, port] : ports)
            static_cast<FillableBuffer*>(port.get())->flush();
    }

    void onPortFilled() override {
        if (owner)
            owner->onPortFilled();
    }

    const std::vector<std::string>& orderedPortNames() const {
        return orderedNames;
    }

    const std::unordered_map<std::string, std::unique_ptr<PortBase>>& allPorts() const {
        return ports;
    }

private:
    IPortOwner* owner;
    mutable std::unordered_map<std::string, std::unique_ptr<PortBase>> ports;
    mutable std::unordered_map<std::string, std::type_index> types;
    std::vector<std::string> orderedNames;
};

namespace detail {
    template<typename T>
    auto getPortValue(const PortBank& bank, const std::string& name) {
        if constexpr (requires { typename T::value_type; }) {
            return bank.getMultiPort<typename T::value_type>(name)->getAll();
        } else {
            return bank.getPort<T>(name)->get();
        }
    }

    template<typename Tuple, std::size_t... Is>
    Tuple unpackPorts(const PortBank& bank, std::index_sequence<Is...>) {
        const auto& names = bank.orderedPortNames();
        return std::make_tuple(getPortValue<std::tuple_element_t<Is, Tuple>>(bank, names[Is])...);
    }
}

template<typename TIn, typename TOut>
class Node : public IPortOwner {
public:
    explicit Node(std::unique_ptr<Processor<TIn, TOut>> processor)
        : processor(std::move(processor)),
          inputBank(this),
          outputBank(this) {}

    template<typename T>
    void addInput(const std::string& name) {
        inputBank.addPort<T>(name);
    }

    template<typename T>
    void addOutput(const std::string& name) {
        outputBank.addPort<T>(name);
    }

    template<typename T>
    void connectOutputTo(const std::string& outputName, Node& target, const std::string& targetInputName) {
        outputBank.getPort<T>(outputName)->connect(target.inputBank.getPort<T>(targetInputName));
    }

    template<typename T, typename TargetNode>
    void connectOutputTo(const std::string& outputName, TargetNode& target, const std::string& targetInputName) {
        outputBank.getPort<T>(outputName)->connect(target.template inputs().template getPort<T>(targetInputName));
    }

    template<typename T>
    void feedInput(const std::string& name, const T& value) {
        PIPELINE_LOG("Feeding input " << name << ": " << to_string_fallback(value));
        inputBank.getPort<T>(name)->consume(value);
    }

    template<typename T>
    T getOutputValue(const std::string& name) const {
        return outputBank.getPort<T>(name)->get();
    }

    PortBank& inputs() { return inputBank; }
    const PortBank& inputs() const { return inputBank; }

    PortBank& outputs() { return outputBank; }
    const PortBank& outputs() const { return outputBank; }

    void onPortFilled() override {
        if (!inputBank.allFilled() || !processor) return;

        PIPELINE_LOG("All inputs filled. Executing processor...");
        TIn in = readInputs();
        TOut out = processor->process(in);
        writeOutputs(out);
        inputBank.flushAll();
    }

private:
    PortBank inputBank;
    PortBank outputBank;
    std::unique_ptr<Processor<TIn, TOut>> processor;

    TIn readInputs();
    void writeOutputs(const TOut&);
};

template<typename TIn, typename TOut>
TIn Node<TIn, TOut>::readInputs() {
    if constexpr (std::is_same_v<TIn, std::tuple<>>) {
        return {};
    } else if constexpr (requires { std::tuple_size<TIn>::value; }) {
        return detail::unpackPorts<TIn>(inputBank, std::make_index_sequence<std::tuple_size<TIn>::value>{});
    } else {
        return inputBank.template getPort<TIn>("in")->get();
    }
}

template<typename TIn, typename TOut>
void Node<TIn, TOut>::writeOutputs(const TOut& value) {
    outputBank.template getPort<TOut>("out")->consume(value);
}

#endif // PIPELINECORE_H