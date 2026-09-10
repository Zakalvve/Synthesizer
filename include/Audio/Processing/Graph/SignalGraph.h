#ifndef AUDIO_PROCESSING_SIGNALGRAPH_H
#define AUDIO_PROCESSING_SIGNALGRAPH_H

#include <memory>
#include <vector>

#include "SignalProcessor.h"
#include "AudioPipeline.h"

namespace Audio::Processing {
    class SignalGraph {
    public:
        using NodeId = int;

        NodeId add(std::unique_ptr<SignalProcessor> processor);

        void connect(NodeId from, int outPort, NodeId to, int inPort);

        void connectDelayed(NodeId from, int outPort, NodeId to, int inPort);

        void setOutput(NodeId node, int outPort);

        AudioPipeline compile(int maxInstances, int blockSize = 1);

    private:
        struct NodeSpec {
            std::unique_ptr<SignalProcessor> processor;
            int inCount;
            int outCount;
        };

        struct Edge {
            NodeId from;
            int outPort;
            NodeId to;
            int inPort;
            bool delayed;
        };

        struct Terminal {
            NodeId node;
            int port;
        };

        std::vector<NodeSpec> _nodes;
        std::vector<Edge> _edges;
        std::vector<Terminal> _outputs;
    };
}

#endif
