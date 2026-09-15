#include "GraphCompiler.h"

#include <stdexcept>
#include <unordered_map>

#include "SignalGraph.h"
#include "NodeRegistry.h"

namespace Audio::Synth {
    using namespace Audio::Processing;

    CompiledGraph compileGraph(const GraphDef &def, int maxInstances, int blockSize, int sampleRate) {
        SignalGraph graph;
        std::unordered_map<std::string, SignalGraph::NodeId> ids;
        std::vector<std::pair<std::string, SignalProcessor *> > nodes;
        nodes.reserve(def.nodes.size());

        for (const NodeDef &n: def.nodes) {
            if (ids.count(n.name))
                throw std::runtime_error("compileGraph: duplicate node name '" + n.name + "'");
            auto proc = NodeRegistry::instance().create(n, sampleRate);
            SignalProcessor *raw = proc.get();
            SignalGraph::NodeId id = graph.add(std::move(proc));
            ids[n.name] = id;
            nodes.emplace_back(n.name, raw);
        }

        for (const ConnectionDef &c: def.connections) {
            auto f = ids.find(c.from);
            auto t = ids.find(c.to);
            if (f == ids.end())
                throw std::runtime_error("compileGraph: unknown connection source '" + c.from + "'");
            if (t == ids.end())
                throw std::runtime_error("compileGraph: unknown connection target '" + c.to + "'");
            if (c.delayed) graph.connectDelayed(f->second, c.outPort, t->second, c.inPort);
            else graph.connect(f->second, c.outPort, t->second, c.inPort);
        }

        for (const OutputDef &o: def.outputs) {
            auto it = ids.find(o.node);
            if (it == ids.end())
                throw std::runtime_error("compileGraph: unknown output node '" + o.node + "'");
            graph.setOutput(it->second, o.port);
        }

        CompiledGraph result;
        result.pipeline = graph.compile(maxInstances, blockSize);
        result.nodes = std::move(nodes);
        return result;
    }
}
