#ifndef MIDIMESSAGEVISITOR_H
#define MIDIMESSAGEVISITOR_H

namespace Audio {
    namespace Midi {
        class MidiMessage;
        class MidiNoteMessage;
        class MidiChannelOnMessage;

        class MidiMessageVisitor {
        public:
            virtual void visit(MidiMessage& message) = 0;
            virtual void visit(MidiNoteMessage& message) = 0;
            virtual void visit(MidiChannelOnMessage& message) = 0;
        };
    }
}

#endif // MIDIMESSAGEVISITOR_H
