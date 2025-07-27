#ifndef IWAVEFORM_H_INCLUDED
#define IWAVEFORM_H_INCLUDED

namespace Audio {
    class IWaveform {
        public:
            IWaveform(int sampleRate) : sample_rate(sampleRate) {}
            virtual ~IWaveform() = default;
            virtual double sample(int t, double f) const = 0;

        protected:
            int getSampleRate() const {
                return sample_rate;
            }

        private:
            int sample_rate;
    };
}

#endif // IWAVEFORM_H_INCLUDED
