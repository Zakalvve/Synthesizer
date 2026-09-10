#include "SignalGraph.h"

#include <stdexcept>
#include <utility>

namespace Audio::Processing {
    SignalGraph::NodeId SignalGraph::add(std::unique_ptr<SignalProcessor> processor) {
        const int inCount = processor->inputCount();
        const int outCount = processor->outputCount();
        _nodes.push_back(NodeSpec{std::move(processor), inCount, outCount});
        return static_cast<NodeId>(_nodes.size() - 1);
    }

    void SignalGraph::connect(NodeId from, int outPort, NodeId to, int inPort) {
        _edges.push_back(Edge{from, outPort, to, inPort, false});
    }

    void SignalGraph::connectDelayed(NodeId from, int outPort, NodeId to, int inPort) {
        _edges.push_back(Edge{from, outPort, to, inPort, true});
    }

    void SignalGraph::setOutput(NodeId node, int outPort) {
        _outputNode = node;
        _outputPort = outPort;
    }

    AudioPipeline SignalGraph::compile(int maxInstances, int blockSize) {
        if (_outputNode < 0) throw std::runtime_error("SignalGraph::compile: no output node set");

        const int n = static_cast<int>(_nodes.size());

        std::vector<int> outBase(n, 0);
        int numSlots = 0;
        for (int i = 0; i < n; ++i) {
            outBase[i] = numSlots;
            numSlots += _nodes[i].outCount;
        }
        const int zeroSlot = numSlots++;

        std::vector<std::vector<int> > inSlots(n);
        for (int i = 0; i < n; ++i) inSlots[i].assign(_nodes[i].inCount, zeroSlot);

        std::vector<std::vector<int> > adj(n);
        std::vector<int> indeg(n, 0);
        for (const Edge &e: _edges) {
            if (e.delayed) continue;
            inSlots[e.to][e.inPort] = outBase[e.from] + e.outPort;
            adj[e.from].push_back(e.to);
            indeg[e.to]++;
        }

        std::vector<int> order;
        order.reserve(n);
        std::vector<int> ready;
        for (int i = 0; i < n; ++i) if (indeg[i] == 0) ready.push_back(i);
        while (!ready.empty()) {
            const int u = ready.back();
            ready.pop_back();
            order.push_back(u);
            for (int v: adj[u]) if (--indeg[v] == 0) ready.push_back(v);
        }
        if (static_cast<int>(order.size()) != n)
            throw std::runtime_error("SignalGraph::compile: cycle without a delayed edge");

        std::vector<std::size_t> stateOffset(n, 0);
        std::size_t totalState = 0;
        for (int i = 0; i < n; ++i) {
            stateOffset[i] = totalState;
            totalState += _nodes[i].processor->stateSize();
        }

        AudioPipeline p;
        p._maxInstances = maxInstances;
        p._blockSize = blockSize;
        p._instanceStateSize = totalState;
        p._terminalSlot = outBase[_outputNode] + _outputPort;

        p._nodes.reserve(n);
        for (const int idx: order) {
            AudioPipeline::NodeRec rec;
            rec.processor = _nodes[idx].processor.get();
            rec.inputSlots = inSlots[idx];
            rec.outputSlots.resize(_nodes[idx].outCount);
            for (int k = 0; k < _nodes[idx].outCount; ++k) rec.outputSlots[k] = outBase[idx] + k;
            rec.stateOffset = stateOffset[idx];
            p._nodes.push_back(std::move(rec));
        }

        p._owned.resize(n);
        for (int i = 0; i < n; ++i) p._owned[i] = std::move(_nodes[i].processor);

        p._scratch.assign(static_cast<std::size_t>(numSlots) * blockSize, 0.0);
        p._stateArena.assign(static_cast<std::size_t>(maxInstances) * totalState, 0.0);
        return p;
    }
}
