#include <iostream>
#include <tuple>
#include <vector>
#include <memory>
#include <numeric>
#include <algorithm>

#include "InputNode.h"
#include "MultiCompositeNode.h"
#include "Processor.h"

using namespace Audio::Processing::Pipelines;
using namespace Audio::Processing::Pipelines::Processors;

// --- Processor: Multiply average int by count of true bools ---
class BoolIntAggregator : public Processor<std::tuple<std::vector<bool>, std::vector<int>>, double> {
protected:
    double process(const std::tuple<std::vector<bool>, std::vector<int>>& input) override {
        const auto& bools = std::get<0>(input);
        const auto& ints = std::get<1>(input);

        int trueCount = static_cast<int>(std::count(bools.begin(), bools.end(), true));
        double avgInt = std::accumulate(ints.begin(), ints.end(), 0.0) / ints.size();
        double result = trueCount * avgInt;

        std::cout << "[Aggregator] " << trueCount << " true(s), avg int = " << avgInt
                  << " => Output: " << result << "\n";

        return result;
    }
};

// --- Output processor ---
class PrintProcessor : public Processor<double, double> {
protected:
    double process(const double& value) override {
        std::cout << "[Output] Received: " << value << "\n";
        return value;
    }
};

int main() {
    InputNode<bool> b1, b2;
    InputNode<int> i1, i2;

    MultiCompositeNode<bool, int, double> composite(std::make_unique<BoolIntAggregator>());
    Node<double, double> output(std::make_unique<PrintProcessor>());

    b1.connectToObjectSink<bool>(&composite);
    b2.connectToObjectSink<bool>(&composite);
    i1.connectToObjectSink<int>(&composite);
    i2.connectToObjectSink<int>(&composite);
    composite.connect(&output);

    std::cout << "[Main] Feeding 2 bools and 2 ints\n";
    b1.consume(true);
    i1.consume(10);
    b2.consume(false);
    i2.consume(14); // triggers processing

    // std::cout << "\n[Main] Feeding 2 bools and 2 ints again\n";
    // b1.consume(true);
    // i1.consume(20);
    // b2.consume(true);
    // i2.consume(12); // triggers processing

    std::cout << "\nPress Enter to exit...";
    std::cin.get();
    return 0;
}
