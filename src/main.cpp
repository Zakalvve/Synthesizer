#include <iostream>
#include "PipelineCore.h"

class AddOne : public Processor<double, double> {
    double process(const double& x) override { return x + 1; }
};

class MultiplyByTwo : public Processor<double, double> {
    double process(const double& x) override { return x * 2; }
};

class MinusOne : public Processor<double, double> {
    double process(const double& x) override { return x - 1; }
};

class HalfInput : public Processor<double, double> {
    double process(const double& x) override { return x / 2.0; }
};

class ToInteger : public Processor<double, int> {
    int process(const double& x) override { return static_cast<int>(x); }
};

template<typename T>
class PassThrough : public Processor<T, T> {
    T process(const T& x) override { return x; }
};

class ConditionalDouble : public Processor<std::tuple<bool, int>, int> {
public:
    int process(const std::tuple<bool, int>& input) override {
        bool condition = std::get<0>(input);
        int value = std::get<1>(input);
        return condition ? value * 2 : value;
    }
};


int main() {
    try {
        Node<double, double> start(std::make_unique<AddOne>());
        Node<double, double> second(std::make_unique<MultiplyByTwo>());
        Node<double, double> third(std::make_unique<MultiplyByTwo>());
        Node<double, double> fourth(std::make_unique<MinusOne>());
        Node<double, double> fifth(std::make_unique<HalfInput>());
        Node<double, int>    sixth(std::make_unique<ToInteger>());

        // Define ports
        start.addInput<double>("in");
        start.addOutput<double>("out");

        second.addInput<double>("in");
        second.addOutput<double>("out");

        third.addInput<double>("in");
        third.addOutput<double>("out");

        fourth.addInput<double>("in");
        fourth.addOutput<double>("out");

        fifth.addInput<double>("in");
        fifth.addOutput<double>("out");

        sixth.addInput<double>("in");
        sixth.addOutput<int>("out");

        // Connect the pipeline
        start.connectOutputTo<double>("out", second, "in");
        second.connectOutputTo<double>("out", third, "in");
        third.connectOutputTo<double>("out", fourth, "in");
        fourth.connectOutputTo<double>("out", fifth, "in");
        fifth.connectOutputTo<double>("out", sixth, "in");

        // Feed initial value
        start.feedInput<double>("in", 1.0);

        // Read final output
        int result = sixth.getOutputValue<int>("out");
        std::cout << "Final result: " << result << std::endl; // Should print 3

        std::cout << std::endl;

        // === Second test: conditional double based on bool ===
        Node<std::tuple<bool, int>, int> logicNode(std::make_unique<ConditionalDouble>());
        logicNode.addInput<bool>("flag");    // instead of "flag"
        logicNode.addInput<int>("number");     // instead of "number"
        logicNode.addOutput<int>("out");

        // Feed inputs
        logicNode.feedInput<int>("number", 7);
        logicNode.feedInput<bool>("flag", false);

        // Read result
        int logicResult = logicNode.getOutputValue<int>("out");
        std::cout << "Conditional logic result (should be 7): " << logicResult << std::endl;
    }
    catch (const std::exception& ex) {
        std::cerr << "[EXCEPTION] " << ex.what() << std::endl;
    }

    std::cout << "\nPress Enter to exit...";
    std::cin.get();
    return 0;
}
