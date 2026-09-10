//
// Created by david on 29/07/2025.
//

#ifndef PORTBANK_H
#define PORTBANK_H

#include <iostream>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>

#include "IPortOwner.h"
#include "MultiPort.h"
#include "PipelinesUtils.h"
#include "Port.h"

namespace Audio::Processing::Pipelines {
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
            const auto typeIt = types.find(name);
            if (typeIt == types.end())
                throw std::runtime_error("Port not found: " + name);

            if (typeIt->second != std::type_index(typeid(T)))
                throw std::runtime_error("Type mismatch on port: " + name);

            return static_cast<Port<T>*>(ports.at(name).get());
        }

        template<typename T>
        const Port<T>* getPort(const std::string& name) const {
            const auto typeIt = types.find(name);
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

#endif //PORTBANK_H
