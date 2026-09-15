#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/error/en.h>
#include <cstdio>
#include <iostream>
#include <memory>
#include <vector>
#include <queue>

#include "MidiJsonFile.h"
#include "MidiSequence.h"
#include "MidiTrack.h"
#include "MidiMessage.h"
#include "MidiNoteMessage.h"
#include "MidiChannelOnMessage.h"

namespace Audio {
    namespace Midi {
        MidiJsonFile::MidiJsonFile() : sequence(nullptr) {};
        MidiJsonFile::~MidiJsonFile() {};
        bool MidiJsonFile::load(const char* filePath) {
            FILE* fp = fopen(filePath, "r");

            if (!fp) {
                std::cerr << "Could not open file " << filePath << std::endl;
                return false;
            }

            char buffer[bufferSize];

            rapidjson::FileReadStream is(fp, buffer, sizeof(buffer));

            rapidjson::Document document;
            document.ParseStream(is);

            fclose(fp);

            if (document.HasParseError()){
                std::cerr << "JSON parse error: " << rapidjson::GetParseError_En(document.GetParseError())
                    << " at offset " << document.GetErrorOffset() << std::endl;
                return false;
            }

            int ticksPerQuarterNote = 0;
            int microsecondsPerQuarterNote = 0;
            int tracksCount = 0;

            // Read header
            if (document.HasMember("header") && document["header"].IsObject()){
                const rapidjson::Value& header = document["header"];

                if (header.HasMember("ticksPerQuarterNote") && header["ticksPerQuarterNote"].IsInt()){
                    ticksPerQuarterNote = header["ticksPerQuarterNote"].GetInt();
                }

                if (header.HasMember("microsecondsPerQuarterNote") && header["microsecondsPerQuarterNote"].IsInt()){
                    microsecondsPerQuarterNote = header["microsecondsPerQuarterNote"].GetInt();
                }

                if (header.HasMember("tracks") && header["tracks"].IsInt()){
                    tracksCount = header["tracks"].GetInt();
                }
            }

            // Read tracks
            if (document.HasMember("tracks") && document["tracks"].IsArray()) {
                const rapidjson::Value& jTracks = document["tracks"].GetArray();
                std::vector<std::unique_ptr<MidiTrack>> tracks;

                for(rapidjson::SizeType i = 0; i < jTracks.Size(); i++){
                    const rapidjson::Value& track = jTracks[i];
                    std::string trackName = "";
                    std::queue<std::unique_ptr<MidiMessage>> messages;

                    if (track.HasMember("trackName") && track["trackName"].IsString()){
                        trackName = track["trackName"].GetString();
                    }

                    if (track.HasMember("events") && track["events"].IsArray()){
                        const rapidjson::Value& events = track["events"].GetArray();

                        for (rapidjson::SizeType j = 0; j < events.Size(); j++){
                            const rapidjson::Value& event = events[j];
                            int deltaTime = 0;
                            std::string type = "";
                            std::string subType = "";
                            int channel = -1;

                            if (event.HasMember("deltaTime") && event["deltaTime"].IsInt()){
                                deltaTime = event["deltaTime"].GetInt();
                            }

                            if (event.HasMember("type") && event["type"].IsString()){
                                type = event["type"].GetString();
                            }

                            if (event.HasMember("subType") && event["subType"].IsString()){
                                subType = event["subType"].GetString();
                            }

                            if (event.HasMember("channel") && event["channel"].IsInt()){
                                channel = event["channel"].GetInt();
                            }

                            std::unique_ptr<MidiMessage> message = nullptr;

                            if (type == "channel"){
                                if (subType == "keyOn" || subType == "keyOff"){
                                    std::string note = "";
                                    int velocity = 0;

                                    if (event.HasMember("note") && event["note"].IsString()){
                                        note = event["note"].GetString();
                                    }
                                    if (event.HasMember("velocity") && event["velocity"].IsInt()){
                                        velocity = event["velocity"].GetInt();
                                    }

                                    message = std::make_unique<MidiNoteMessage>(deltaTime, type, subType, channel, note, velocity);

                                }
                            } else if (type == "meta") {
                                if (subType == "channelOn"){
                                    message = std::make_unique<MidiChannelOnMessage>(deltaTime, type, subType, channel);
                                }
                            }

                            if (message == nullptr){
                                message = std::make_unique<MidiMessage>(deltaTime, type, subType, channel);
                            }

                            if (message) {
                                messages.push(std::move(message));
                            }
                        }
                    }

                    std::unique_ptr<MidiTrack> midiTrack = std::make_unique<MidiTrack>(trackName, std::move(messages));

                    tracks.push_back(std::move(midiTrack));
                }

                sequence = std::make_unique<MidiSequence>(ticksPerQuarterNote, microsecondsPerQuarterNote, tracksCount, std::move(tracks));
            }

            return sequence != nullptr;
        }
    }
}
