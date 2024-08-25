#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/error/en.h>
#include <cstdio>
#include <iostream>
#include <memory>
#include <vector>
#include <queue>

#include "MidiJsonFile.h"
#include "MidiRoll.h"
#include "MidiRollTrack.h"
#include "MidiRollEvent.h"
#include "MidiRollKeyEvent.h"
#include "MidiRollChannelOnEvent.h"

namespace Audio {
    namespace Midi {
        MidiJsonFile::MidiJsonFile() : roll(nullptr) {};
        MidiJsonFile::~MidiJsonFile() {};
        bool MidiJsonFile::load(const char* filePath, int sampleRate) {
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
                std::vector<std::unique_ptr<MidiRollTrack>> mjTracks;

                for(rapidjson::SizeType i = 0; i < jTracks.Size(); i++){
                    const rapidjson::Value& track = jTracks[i];
                    std::string trackName = "";
                    std::queue<std::unique_ptr<MidiRollEvent>> mjEvents;

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

                            std::unique_ptr<MidiRollEvent> mjEvent = nullptr;

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

                                    mjEvent = std::make_unique<MidiRollKeyEvent>(deltaTime, type, subType, channel, note, velocity);

                                }
                            } else if (type == "meta") {
                                if (subType == "channelOn"){
                                    mjEvent = std::make_unique<MidiRollChannelOnEvent>(deltaTime, type, subType, channel);
                                }
                            }

                            if (mjEvent == nullptr){
                                mjEvent = std::make_unique<MidiRollEvent>(deltaTime, type, subType, channel);
                            }

                            if (mjEvent) {
                                mjEvents.push(std::move(mjEvent));
                            }
                        }
                    }

                    std::unique_ptr<MidiRollTrack> mjTrack = std::make_unique<MidiRollTrack>(trackName, std::move(mjEvents));

                    mjTracks.push_back(std::move(mjTrack));
                }

                roll = std::make_unique<MidiRoll>(sampleRate, ticksPerQuarterNote, microsecondsPerQuarterNote, tracksCount, std::move(mjTracks));
            }

            return roll != nullptr;
        }
    }
}