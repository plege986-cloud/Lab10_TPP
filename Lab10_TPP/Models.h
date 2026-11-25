#pragma once
#include <string>
struct EventSource { int id=0; std::string name; std::string category; std::string status; };
struct LogEntry { int id=0; int sourceId=0; std::string timestamp; std::string level; std::string message; int durationMs=0; };
struct EntryState { int id=0; int entryId=0; std::string status; };
struct StateHistory { int id=0; int entryId=0; std::string previous; std::string next; };
